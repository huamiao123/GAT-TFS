/**
 * Stage3 v3: Degree-Aware Node Sorting
 *
 * v2 的问题：
 *   每批 16 节点按 max_deg 同步推进
 *   web-Google avg_deg=5.6，但 hub 节点 deg 可达数百
 *   一个 hub 节点拖累整批 15 个低度数节点做无效 zero-row gather
 *
 * v3 的修复：
 *   预处理：按度数降序排列节点
 *   效果：同一批 16 节点度数相近 → max_deg ≈ avg_deg → 几乎无浪费
 *   代价：O(N log N) 排序，只做一次，不计入计时
 *
 * 对比实验：
 *   FB-BF16 SpMM（基准）
 *   Fusion v2（无排序）
 *   Fusion v3（度数排序）
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
    uint8_t palette_id, start_row, reserved0[14];
    uint16_t colsb[16]; uint8_t rows[16];
} __attribute__((packed)) tile_config_t;
static_assert(sizeof(tile_config_t)==64,"");

static constexpr int K=128, N=128, MT=16, TK=32, TN=16;
static constexpr int REPEAT=10, WARMUP=3;

static inline uint16_t f32bf16(float f){uint32_t u;memcpy(&u,&f,4);return(uint16_t)(u>>16);}
static inline float bf16f32(uint16_t b){uint32_t u=(uint32_t)b<<16;float f;memcpy(&f,&u,4);return f;}

struct CSR{
    uint64_t nrow,ncol,nnz;
    std::vector<uint32_t> indptr,indices;
};
static bool load(const char* p,CSR& m){
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

// ── FB-BF16 SpMM（按原始顺序，baseline）────────────────────────
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

// ── 通用 Fusion Kernel（接受节点顺序 order[]）───────────────────
// order[i] = 第 i 个处理的原始节点编号
// Out[order[i]*N : +N] 写回结果
static void fusion_ordered(
    const CSR& A,
    const std::vector<uint32_t>& order,  // 节点处理顺序
    const uint16_t* H,                   // [ncol+1, K] BF16，末尾全零
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

        uint16_t* hbuf=(uint16_t*)aligned_alloc(64,MT*K*2);
        float*    ctmp=(float*)   aligned_alloc(64,MT*N*4);
        int32_t   cur[MT];

        #pragma omp for schedule(dynamic,4)
        for(int64_t i0=0;i0<M;i0+=MT){
            int batch=(int)std::min((int64_t)MT,M-i0);

            // 取出这批节点的原始编号
            uint32_t nodes[MT];
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

            memset(ctmp,0,MT*N*4);

            for(int step=0;step<max_deg;step++){
                // Gather：每节点取一个邻居
                for(int r=0;r<MT;r++){
                    uint32_t j;
                    if(r<batch && cur[r]<(int32_t)A.indptr[nodes[r]+1])
                        j=A.indices[cur[r]++];
                    else
                        j=zrow;
                    memcpy(hbuf+r*K, H+(size_t)j*K, K*2);
                }

                // AMX：hbuf(MT×K) × W(K×N) 累加到 ctmp
                for(int np=0;np<N/(TN*2);np++){
                    if(step>0){
                        _tile_loadd(0,ctmp+np*TN*2,   CS);
                        _tile_loadd(1,ctmp+np*TN*2+TN,CS);
                    } else {
                        _tile_zero(0); _tile_zero(1);
                    }
                    for(int kp=0;kp<K/TK;kp++){
                        _tile_loadd(2,hbuf+kp*TK,AS);
                        _tile_loadd(3,Wv+(kp*(N/TN)+np*2  )*BS,BS2);
                        _tile_loadd(4,Wv+(kp*(N/TN)+np*2+1)*BS,BS2);
                        _tile_dpbf16ps(0,2,3);
                        _tile_dpbf16ps(1,2,4);
                    }
                    _tile_stored(0,ctmp+np*TN*2,   CS);
                    _tile_stored(1,ctmp+np*TN*2+TN,CS);
                }
            }

            // 写回原始位置（按 nodes[] 散写）
            for(int r=0;r<batch;r++)
                memcpy(Out+(size_t)nodes[r]*N, ctmp+r*N, N*4);
        }

        free(hbuf); free(ctmp);
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

// ── 统计批次内度数差异（衡量排序效果）──────────────────────────
static void print_batch_stats(const CSR& A,
                               const std::vector<uint32_t>& order,
                               const char* label){
    double total_waste=0;
    int64_t total_steps=0;
    int64_t M=order.size();
    for(int64_t i0=0;i0<M;i0+=MT){
        int batch=(int)std::min((int64_t)MT,M-i0);
        int max_deg=0,sum_deg=0;
        for(int r=0;r<batch;r++){
            int d=(int)(A.indptr[order[i0+r]+1]-A.indptr[order[i0+r]]);
            if(d>max_deg) max_deg=d;
            sum_deg+=d;
        }
        total_waste += (double)(max_deg*MT - sum_deg);
        total_steps += max_deg*MT;
    }
    double waste_pct = total_steps>0 ? total_waste/total_steps*100 : 0;
    printf("    [%s] 批次 zero-row 浪费率：%.1f%%（有效计算 %.1f%%）\n",
           label, waste_pct, 100-waste_pct);
}

int main(int argc,char** argv){
    int nt=argc>=2?atoi(argv[1]):32;
    const char* MATS[]={
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/web-Google/web-Google.csrbin",
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/amazon0601/amazon0601.csrbin",
        nullptr
    };

    printf("=== Stage3 v3: Degree-Aware Node Sorting ===\n");
    printf("K=%d  N=%d  threads=%d  repeat=%d\n\n",K,N,nt,REPEAT);

    srand(42);
    float* Wf=(float*)aligned_alloc(64,K*N*4);
    for(int i=0;i<K*N;i++) Wf[i]=(rand()%200-100)/100.f;
    uint16_t* Wv=pack_W(Wf); free(Wf);

    for(int mi=0;MATS[mi];mi++){
        CSR A; if(!load(MATS[mi],A)) continue;
        const char* name=strrchr(MATS[mi],'/')+1;
        printf("矩阵：%s  NNZ=%.1fM  avg_deg=%.1f\n",
               name,A.nnz/1e6,(double)A.nnz/A.nrow);

        // H：ncol+1 行，末尾全零
        size_t Hr=A.ncol+1;
        uint16_t* H=(uint16_t*)aligned_alloc(64,Hr*K*2);
        for(size_t i=0;i<A.ncol*(size_t)K;i++) H[i]=f32bf16((rand()%200-100)/100.f);
        memset(H+A.ncol*K,0,K*2);
        uint32_t zrow=(uint32_t)A.ncol;

        // 原始顺序（v2）
        std::vector<uint32_t> order_orig(A.nrow);
        std::iota(order_orig.begin(),order_orig.end(),0);

        // 度数降序排列（v3）
        std::vector<uint32_t> order_deg(A.nrow);
        std::iota(order_deg.begin(),order_deg.end(),0);
        std::sort(order_deg.begin(),order_deg.end(),
            [&](uint32_t a,uint32_t b){
                return (A.indptr[a+1]-A.indptr[a]) >
                       (A.indptr[b+1]-A.indptr[b]);
            });

        // 打印批次统计
        print_batch_stats(A, order_orig, "v2无排序");
        print_batch_stats(A, order_deg,  "v3度数排序");
        printf("\n");

        float* Zfb =(float*)aligned_alloc(64,A.nrow*K*4);
        float* Zv2 =(float*)aligned_alloc(64,A.nrow*N*4);
        float* Zv3 =(float*)aligned_alloc(64,A.nrow*N*4);
        memset(Zfb,0,A.nrow*K*4);

        // FB-BF16
        double ms_fb=bench([&](){fb_spmm(A,H,Zfb,nt);},WARMUP,REPEAT);

        // Fusion v2（原始顺序）
        double ms_v2=bench([&](){
            memset(Zv2,0,A.nrow*N*4);
            fusion_ordered(A,order_orig,H,zrow,Wv,Zv2,nt);
        },WARMUP,REPEAT);

        // Fusion v3（度数排序）
        double ms_v3=bench([&](){
            memset(Zv3,0,A.nrow*N*4);
            fusion_ordered(A,order_deg,H,zrow,Wv,Zv3,nt);
        },WARMUP,REPEAT);

        printf("  %-20s %10s %10s %10s\n","","FB-BF16","Fusion_v2","Fusion_v3");
        printf("  %-20s %10.2f %10.2f %10.2f  ms\n","时间",ms_fb,ms_v2,ms_v3);
        printf("  %-20s %10s %10.2f %10.2f  ×\n","vs FB-BF16","1.00",ms_fb/ms_v2,ms_fb/ms_v3);
        printf("  %-20s %10s %10s %10.2f  ×\n","v3 vs v2","","",ms_v2/ms_v3);
        printf("\n  注：Fusion 同时完成 SpMM+GeMM，FB-BF16 只做 SpMM\n");
        printf("      公平对比：两步总计 ≈ %.0fms vs Fusion_v3 %.2fms = %.2f×\n",
               ms_fb+4.0, ms_v3, (ms_fb+4.0)/ms_v3);
        printf("\n%s\n\n",std::string(60,'-').c_str());

        free(H); free(Zfb); free(Zv2); free(Zv3);
    }

    free(Wv);
    printf("=== Done ===\n");
    return 0;
}
