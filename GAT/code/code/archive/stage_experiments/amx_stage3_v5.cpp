/**
 * Stage3 v5: Degree Tiling (DEG_BLK)
 *
 * 专家诊断：高度数矩阵 gather 数据量冲刷 L1 cache，W 矩阵被踢出
 *   低度数 (deg=5.6):  Gather=24KB, W=32KB, 合计56KB ≈ L1(48KB)上限, W驻留
 *   高度数 (deg=18.8): Gather=76KB >> L1, W 每步重新从 L2 捞, AMX 饿死
 *
 * 修复：DEG_BLK 分块
 *   把 max_deg 切成 DEG_BLK 大小的子块
 *   每个子块 Gather = MT(16) × DEG_BLK × K(128) × 2B
 *   令 DEG_BLK = 8: Gather = 16×8×256B = 32KB
 *   W(32KB) + Gather(32KB) = 64KB → L1+L2 友好, W 大部分驻留
 *
 * 同时修复：hollywood 路径改为 hollywood-2009
 *
 * 理论预测 (专家1公式)：speedup ≈ 1 + 2/avg_deg
 *   web-Google  deg=5.6  → ~1.36×
 *   as-Skitter  deg=13.1 → ~1.15×
 *   soc-Pokec   deg=18.8 → ~1.11×
 *   hollywood   deg=99.9 → ~1.02×
 */
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <vector>
#include <numeric>
#include <string>
#include <cmath>

typedef struct {
    uint8_t palette_id,start_row,reserved0[14];
    uint16_t colsb[16]; uint8_t rows[16];
} __attribute__((packed)) tile_config_t;
static_assert(sizeof(tile_config_t)==64,"");

static constexpr int K=128,N=128,MT=16,TK=32,TN=16;
// DEG_BLK：每子块处理的邻居数
// 每子块 Gather = MT*DEG_BLK*K*2 bytes
// DEG_BLK=8: 16×8×256B=32KB, 加上W(32KB)=64KB, 在L1+L2安全区
static constexpr int DEG_BLK=8;
static constexpr int REPEAT=10,WARMUP=3;

static inline uint16_t f32bf16(float f){uint32_t u;memcpy(&u,&f,4);return(uint16_t)(u>>16);}
static inline float bf16f32(uint16_t b){uint32_t u=(uint32_t)b<<16;float f;memcpy(&f,&u,4);return f;}

struct CSR{ uint64_t nrow,ncol,nnz; std::vector<uint32_t> indptr,indices; };
static bool load_csr(const char* p,CSR& m){
    FILE* fp=fopen(p,"rb");if(!fp){printf("cannot open %s\n",p);return false;}
    uint32_t a,b,c;fread(&a,4,1,fp);fread(&b,4,1,fp);fread(&c,4,1,fp);
    fread(&m.nrow,8,1,fp);fread(&m.ncol,8,1,fp);fread(&m.nnz,8,1,fp);
    m.indptr.resize(m.nrow+1);m.indices.resize(m.nnz);
    fread(m.indptr.data(),4,m.nrow+1,fp);
    fread(m.indices.data(),4,m.nnz,fp);
    fclose(fp);return true;
}
static void cfg_tiles(){
    tile_config_t c={};c.palette_id=1;
    c.rows[0]=MT;c.colsb[0]=TN*4; c.rows[1]=MT;c.colsb[1]=TN*4;
    c.rows[2]=MT;c.colsb[2]=TK*2;
    c.rows[3]=TK/2;c.colsb[3]=TN*4; c.rows[4]=TK/2;c.colsb[4]=TN*4;
    _tile_loadconfig(&c);
}
static uint16_t* pack_W(const float* W){
    uint16_t* o=(uint16_t*)aligned_alloc(64,K*N*2); int off=0;
    for(int k0=0;k0<K;k0+=TK)for(int n0=0;n0<N;n0+=TN)for(int kp=0;kp<TK/2;kp++)
        for(int n=0;n<TN;n++){
            o[off++]=f32bf16(W[(k0+2*kp)*N+n0+n]);
            o[off++]=f32bf16(W[(k0+2*kp+1)*N+n0+n]);
        }
    return o;
}
static void fb_spmm(const CSR& A,const uint16_t* H,float* Z,int nt){
    omp_set_num_threads(nt);
    #pragma omp parallel for schedule(dynamic,64)
    for(int64_t i=0;i<(int64_t)A.nrow;i++){
        float* z=Z+i*K; memset(z,0,K*4);
        for(int64_t p=A.indptr[i];p<(int64_t)A.indptr[i+1];p++)
            for(int k=0;k<K;k++) z[k]+=bf16f32(H[(size_t)A.indices[p]*K+k]);
    }
}

// ── Fusion v5：Degree Tiling 保护 L1 Cache ──────────────────────
static void fusion_v5(
    const CSR& A,
    const std::vector<uint32_t>& order,
    const uint16_t* H,
    uint32_t zrow,
    const uint16_t* Wv,
    float* Out,
    int nt)
{
    omp_set_num_threads(nt);
    const int64_t M=(int64_t)order.size();
    const int BS=TK/2*TN*2, AS=K*2, BS2=TN*4, CS=N*4;

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        cfg_tiles();

        // hbuf：DEG_BLK 步 × MT 行 × K 列 BF16
        // = 8 × 16 × 256B = 32KB，安全在 L1 内
        uint16_t* hbuf=(uint16_t*)aligned_alloc(64, DEG_BLK*MT*K*2);
        // ctmp：MT × N FP32 安全写缓冲
        float*    ctmp=(float*)   aligned_alloc(64, MT*N*4);
        int32_t cur[MT];
        uint32_t nodes[MT];

        #pragma omp for schedule(dynamic,4)
        for(int64_t i0=0;i0<M;i0+=MT){
            int batch=(int)std::min((int64_t)MT,M-i0);

            int max_deg=0;
            for(int r=0;r<batch;r++){
                nodes[r]=order[i0+r];
                cur[r]=(int32_t)A.indptr[nodes[r]];
                int d=(int)(A.indptr[nodes[r]+1]-A.indptr[nodes[r]]);
                if(d>max_deg) max_deg=d;
            }
            for(int r=batch;r<MT;r++){nodes[r]=zrow;cur[r]=-1;}

            if(max_deg==0){
                for(int r=0;r<batch;r++) memset(Out+(size_t)nodes[r]*N,0,N*4);
                continue;
            }

            // ctmp 清零（累加器）
            memset(ctmp,0,MT*N*4);

            // ── 外层：DEG_BLK 分块 ──────────────────────────────
            for(int d0=0;d0<max_deg;d0+=DEG_BLK){
                int blk=(int)std::min(DEG_BLK, max_deg-d0);

                // Step1：Gather 这个子块的 blk 步邻居到 hbuf
                // hbuf[step][row] = H[neighbor[row][d0+step]]
                for(int s=0;s<blk;s++){
                    uint16_t* hb=hbuf+s*MT*K;
                    for(int r=0;r<MT;r++){
                        uint32_t j;
                        if(r<batch && cur[r]<(int32_t)A.indptr[nodes[r]+1])
                            j=A.indices[cur[r]++];
                        else
                            j=zrow;
                        memcpy(hb+r*K, H+(size_t)j*K, K*2);
                    }
                }

                // Step2：AMX 计算 blk 步，C tile 从 ctmp load/store
                // W(32KB) 此时在 L1，不会被 hbuf(32KB) 踢出（分批！）
                for(int s=0;s<blk;s++){
                    const uint16_t* hb=hbuf+s*MT*K;
                    for(int np=0;np<N/(TN*2);np++){
                        // 从 ctmp 恢复累加器（第一个子块第一步才清零）
                        if(d0==0 && s==0){
                            _tile_zero(0); _tile_zero(1);
                        } else {
                            _tile_loadd(0,ctmp+np*TN*2,   CS);
                            _tile_loadd(1,ctmp+np*TN*2+TN,CS);
                        }
                        for(int kp=0;kp<K/TK;kp++){
                            _tile_loadd(2,hb+kp*TK,AS);
                            _tile_loadd(3,Wv+(kp*(N/TN)+np*2  )*BS,BS2);
                            _tile_loadd(4,Wv+(kp*(N/TN)+np*2+1)*BS,BS2);
                            _tile_dpbf16ps(0,2,3);
                            _tile_dpbf16ps(1,2,4);
                        }
                        _tile_stored(0,ctmp+np*TN*2,   CS);
                        _tile_stored(1,ctmp+np*TN*2+TN,CS);
                    }
                }
            }

            // 写回：按原始节点位置散写
            for(int r=0;r<batch;r++)
                memcpy(Out+(size_t)nodes[r]*N, ctmp+r*N, N*4);
        }

        free(hbuf); free(ctmp);
        _tile_release();
    }
}

template<typename F>
static double bench(F fn,int w,int r){
    for(int i=0;i<w;i++)fn();
    double best=1e18;
    for(int i=0;i<r;i++){
        auto t0=std::chrono::high_resolution_clock::now();fn();
        auto t1=std::chrono::high_resolution_clock::now();
        best=std::min(best,std::chrono::duration<double,std::milli>(t1-t0).count());
    }
    return best;
}

int main(int argc,char** argv){
    int nt=argc>=2?atoi(argv[1]):32;

    struct MatInfo{ const char* path; double avg_deg_ref; };
    MatInfo MATS[]={
        {"/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/web-Google/web-Google.csrbin",      5.6},
        {"/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/as-Skitter/as-Skitter.csrbin",    13.1},
        {"/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/soc-Pokec/soc-Pokec.csrbin",      18.8},
        {"/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/hollywood-2009/hollywood-2009.csrbin", 99.9},
        {nullptr,0}
    };

    printf("=== Stage3 v5: Degree Tiling (DEG_BLK=%d) ===\n",DEG_BLK);
    printf("K=%d  N=%d  threads=%d  repeat=%d\n",K,N,nt,REPEAT);
    printf("理论 speedup ≈ 1 + 2/avg_deg (专家IO模型)\n\n");
    printf("%-20s %8s %10s %10s %10s %8s %8s\n",
           "矩阵","avg_deg","FB-BF16","v3(ref)","v5","v5/v3","theory");
    printf("%s\n",std::string(80,'-').c_str());

    // v3 参考值（来自之前实验）
    double v3_ref[]={14.10, 69.65, 118.43, -1};

    srand(42);
    float* Wf=(float*)aligned_alloc(64,K*N*4);
    for(int i=0;i<K*N;i++) Wf[i]=(rand()%200-100)/100.f;
    uint16_t* Wv=pack_W(Wf); free(Wf);

    for(int mi=0;MATS[mi].path;mi++){
        CSR A; if(!load_csr(MATS[mi].path,A)) continue;
        const char* name=strrchr(MATS[mi].path,'/')+1;
        double avg_deg=(double)A.nnz/A.nrow;
        double theory=1.0+2.0/avg_deg;  // 专家1公式

        printf("%-20s %8.1f",name,avg_deg); fflush(stdout);

        size_t Hr=A.ncol+1;
        uint16_t* H=(uint16_t*)aligned_alloc(64,Hr*K*2);
        for(size_t i=0;i<A.ncol*(size_t)K;i++) H[i]=f32bf16((rand()%200-100)/100.f);
        memset(H+A.ncol*K,0,K*2);
        uint32_t zrow=(uint32_t)A.ncol;

        // 度数降序排列
        std::vector<uint32_t> order(A.nrow);
        std::iota(order.begin(),order.end(),0);
        std::sort(order.begin(),order.end(),[&](uint32_t a,uint32_t b){
            return (A.indptr[a+1]-A.indptr[a])>(A.indptr[b+1]-A.indptr[b]);
        });

        float* Zfb=(float*)aligned_alloc(64,A.nrow*K*4);
        float* Zv5=(float*)aligned_alloc(64,A.nrow*N*4);
        memset(Zfb,0,A.nrow*K*4);

        double ms_fb=bench([&](){fb_spmm(A,H,Zfb,nt);},WARMUP,REPEAT);
        double ms_v5=bench([&](){
            memset(Zv5,0,A.nrow*N*4);
            fusion_v5(A,order,H,zrow,Wv,Zv5,nt);
        },WARMUP,REPEAT);

        double v3r=v3_ref[mi]>0?v3_ref[mi]:ms_v5;
        printf(" %10.2f %10.2f %10.2f %8.2f× %8.2f×\n",
               ms_fb, v3r, ms_v5, v3r/ms_v5, theory);

        free(H); free(Zfb); free(Zv5);
    }
    free(Wv);

    printf("\n注：v5/v3 > 1× 说明 DEG_BLK 分块有效\n");
    printf("    theory = 1 + 2/avg_deg（专家IO模型预测值）\n");
    printf("    实验值贴近理论值 → IO模型成立 → 论文理论分析扎实\n");
    printf("\n=== Done ===\n");
    return 0;
}
