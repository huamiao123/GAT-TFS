/**
 * Stage3 v4: Pre-Gather + N-Pass Outer Loop
 *
 * v3 的剩余问题：
 *   每个 step 都要存取 C tile（4次 tileload + 4次 tilestore）
 *   avg_deg=5.6 → C tile 流量 = 90KB，实际输出只有 8KB，11× 开销
 *
 * v4 的修复：
 *   1. Pre-gather：先把所有邻居全部 gather 到 pre_buf
 *      avg_deg≈6 → 6×16×256B = 24KB，全在 L1 cache
 *   2. N-pass 提到外层：C tile 在寄存器里驻留整个 max_deg 循环
 *      C tile 存取：4次 tilestore（仅写一次）vs v3 的 90KB
 *
 * 预期：C tile 流量减少 11×，整体加速 2-3×
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

typedef struct {
    uint8_t palette_id,start_row,reserved0[14];
    uint16_t colsb[16]; uint8_t rows[16];
} __attribute__((packed)) tile_config_t;
static_assert(sizeof(tile_config_t)==64,"");

static constexpr int K=128,N=128,MT=16,TK=32,TN=16;
static constexpr int REPEAT=10,WARMUP=3;

static inline uint16_t f32bf16(float f){uint32_t u;memcpy(&u,&f,4);return(uint16_t)(u>>16);}
static inline float bf16f32(uint16_t b){uint32_t u=(uint32_t)b<<16;float f;memcpy(&f,&u,4);return f;}

struct CSR{
    uint64_t nrow,ncol,nnz;
    std::vector<uint32_t> indptr,indices;
};
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
    uint16_t* o=(uint16_t*)aligned_alloc(64,K*N*2);
    int off=0;
    for(int k0=0;k0<K;k0+=TK)
        for(int n0=0;n0<N;n0+=TN)
            for(int kp=0;kp<TK/2;kp++)
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
            for(int k=0;k<K;k++)
                z[k]+=bf16f32(H[(size_t)A.indices[p]*K+k]);
    }
}

// ── Fusion v4：Pre-Gather + N-Pass 外层 ─────────────────────────
static void fusion_v4(
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
    // B block size，A stride，B stride，C stride
    const int BS=TK/2*TN*2, AS=K*2, BS2=TN*4, CS=N*4;

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        cfg_tiles();

        // pre_buf：最多 MAX_DEG 步 × MT 行 × K 列 BF16
        // web-Google 排序后 max_deg_in_batch ≈ 6-10，取 256 足够
        static constexpr int MAX_DEG=256;
        uint16_t* pre_buf=(uint16_t*)aligned_alloc(64,MAX_DEG*MT*K*2);
        // ctmp：MT × N FP32，尾批次安全缓冲
        float* ctmp=(float*)aligned_alloc(64,MT*N*4);

        #pragma omp for schedule(dynamic,4)
        for(int64_t i0=0;i0<M;i0+=MT){
            int batch=(int)std::min((int64_t)MT,M-i0);

            // 取出这批节点，计算 max_deg
            uint32_t nodes[MT];
            int32_t  ptr[MT];   // 每节点当前指针
            int max_deg=0;
            for(int r=0;r<batch;r++){
                nodes[r]=order[i0+r];
                ptr[r]=(int32_t)A.indptr[nodes[r]];
                int d=(int)(A.indptr[nodes[r]+1]-A.indptr[nodes[r]]);
                if(d>max_deg) max_deg=d;
            }
            for(int r=batch;r<MT;r++){nodes[r]=zrow;ptr[r]=-1;}

            if(max_deg==0){
                for(int r=0;r<batch;r++) memset(Out+(size_t)nodes[r]*N,0,N*4);
                continue;
            }

            // 限制 max_deg 不超过缓冲区
            int actual_deg=std::min(max_deg,MAX_DEG);

            // ── Step1：Pre-Gather 所有邻居 ───────────────────────
            // pre_buf[step][row] = H[neighbor[row][step]] BF16
            // 内存布局：[step × MT × K]，step 为最外层
            for(int step=0;step<actual_deg;step++){
                uint16_t* buf_step=pre_buf+(size_t)step*MT*K;
                for(int r=0;r<MT;r++){
                    uint32_t j;
                    if(r<batch && ptr[r]<(int32_t)A.indptr[nodes[r]+1])
                        j=A.indices[ptr[r]++];
                    else
                        j=zrow;
                    memcpy(buf_step+r*K, H+(size_t)j*K, K*2);
                }
            }

            // ── Step2：N-Pass 外层，C tile 驻留寄存器 ────────────
            // 对于 N=128，有 N/(TN*2)=4 个 n_pass
            // 每个 n_pass：C tile 清零，遍历所有 step，最后一次 tilestore
            for(int np=0;np<N/(TN*2);np++){
                // C tile 清零（只做一次）
                _tile_zero(0);   // cols np*TN*2
                _tile_zero(1);   // cols np*TN*2+TN

                // 遍历所有步（C tile 始终在寄存器里）
                for(int step=0;step<actual_deg;step++){
                    const uint16_t* buf_step=pre_buf+(size_t)step*MT*K;
                    // K-pass
                    for(int kp=0;kp<K/TK;kp++){
                        // A tile：从 pre_buf 加载
                        _tile_loadd(2, buf_step+kp*TK, AS);
                        // B tiles：W 的对应块（驻留 L2/L1）
                        _tile_loadd(3,Wv+(kp*(N/TN)+np*2  )*BS,BS2);
                        _tile_loadd(4,Wv+(kp*(N/TN)+np*2+1)*BS,BS2);
                        // 累加（C tile 在寄存器，无需 tileload）
                        _tile_dpbf16ps(0,2,3);
                        _tile_dpbf16ps(1,2,4);
                    }
                }

                // 只做一次 tilestore（C tile 写回）
                if(batch==MT){
                    // 完整批次：直接写 Out
                    _tile_stored(0,Out+(size_t)(i0  )*N+np*TN*2,   CS);
                    _tile_stored(1,Out+(size_t)(i0  )*N+np*TN*2+TN,CS);
                } else {
                    // 尾批次：写到 ctmp 再按节点散写
                    _tile_stored(0,ctmp+np*TN*2,   CS);
                    _tile_stored(1,ctmp+np*TN*2+TN,CS);
                }
            }

            // 尾批次：把 ctmp 按原始节点位置写回 Out
            if(batch<MT){
                for(int r=0;r<batch;r++)
                    memcpy(Out+(size_t)nodes[r]*N, ctmp+r*N, N*4);
            }
        }

        free(pre_buf); free(ctmp);
        _tile_release();
    }
}

template<typename F>
static double bench(F fn,int w,int r){
    for(int i=0;i<w;i++) fn();
    double best=1e18;
    for(int i=0;i<r;i++){
        auto t0=std::chrono::high_resolution_clock::now();
        fn();
        auto t1=std::chrono::high_resolution_clock::now();
        best=std::min(best,std::chrono::duration<double,std::milli>(t1-t0).count());
    }
    return best;
}

int main(int argc,char** argv){
    int nt=argc>=2?atoi(argv[1]):32;
    const char* MATS[]={
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/web-Google/web-Google.csrbin",
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/amazon0601/amazon0601.csrbin",
        nullptr
    };

    printf("=== Stage3 v4: Pre-Gather + N-Pass Outer Loop ===\n");
    printf("K=%d  N=%d  threads=%d  repeat=%d\n\n",K,N,nt,REPEAT);

    srand(42);
    float* Wf=(float*)aligned_alloc(64,K*N*4);
    for(int i=0;i<K*N;i++) Wf[i]=(rand()%200-100)/100.f;
    uint16_t* Wv=pack_W(Wf); free(Wf);

    // 参考值（来自之前实验）
    double ref_v3[]  ={47.32, 25.07};
    double ref_2step[]={22.0,  10.0};

    printf("%-20s %10s %10s %10s %10s %10s\n",
           "矩阵","FB-BF16","Fusion_v3","Fusion_v4","v4/v3","vs 2step");
    printf("%s\n",std::string(74,'-').c_str());

    for(int mi=0;MATS[mi];mi++){
        CSR A; if(!load_csr(MATS[mi],A)) continue;
        const char* name=strrchr(MATS[mi],'/')+1;

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
        float* Zv4=(float*)aligned_alloc(64,A.nrow*N*4);
        memset(Zfb,0,A.nrow*K*4);

        double ms_fb=bench([&](){fb_spmm(A,H,Zfb,nt);},WARMUP,REPEAT);
        double ms_v4=bench([&](){
            memset(Zv4,0,A.nrow*N*4);
            fusion_v4(A,order,H,zrow,Wv,Zv4,nt);
        },WARMUP,REPEAT);

        printf("%-20s %10.2f %10.2f %10.2f %10.2f %10.2f×\n",
               name, ms_fb, ref_v3[mi], ms_v4,
               ref_v3[mi]/ms_v4,
               ref_2step[mi]/ms_v4);

        free(H); free(Zfb); free(Zv4);
    }

    free(Wv);
    printf("\n注：vs 2step = (SpMM+GeMM两步) / Fusion_v4\n");
    printf("    >1.0 表示融合比两步更快\n");
    printf("\n=== Done ===\n");
    return 0;
}
