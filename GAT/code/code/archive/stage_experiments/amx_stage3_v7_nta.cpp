/**
 * Stage3 v7: NTA Prefetch（Non-Temporal Access）
 *
 * 专家2诊断：H特征是 streaming data（每条只用一次），
 * 默认 cache 行为会把 H 塞满 L1，把 W(32KB) 踢出去。
 *
 * 修复：用 _MM_HINT_NTA 预取 H，告诉硬件
 *   "这些数据我只用一次，请不要污染 L1/L2，保住 W 矩阵"
 *
 * 原理：
 *   正常 prefetch → 数据进 L1/L2，参与 cache 竞争
 *   NTA prefetch  → 数据只进 L1（只占一路），用完立刻失效
 *                   不挤占 W 的 cache 空间
 *
 * 预期收益：高度数矩阵（as-Skitter, soc-Pokec, hollywood）
 *   W 不再被踢出 L1 → AMX 计算时 W cache hit 率恢复
 *   可能从 0.57×-0.84× 提升到 1.0×+
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
static constexpr int REPEAT=10,WARMUP=3;
// NTA 预取距离：提前 PREFETCH_DIST 步预取
static constexpr int PREFETCH_DIST=4;

static inline uint16_t f32bf16(float f){uint32_t u;memcpy(&u,&f,4);return(uint16_t)(u>>16);}
static inline float bf16f32(uint16_t b){uint32_t u=(uint32_t)b<<16;float f;memcpy(&f,&u,4);return f;}

struct CSR{ uint64_t nrow,ncol,nnz; std::vector<uint32_t> indptr,indices; };
static bool load_csr(const char* p,CSR& m){
    FILE* fp=fopen(p,"rb");if(!fp){printf("SKIP: %s\n",p);return false;}
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

// ── Fusion v3（无NTA，对照组）────────────────────────────────────
static void fusion_v3(const CSR& A,const std::vector<uint32_t>& order,
    const uint16_t* H,uint32_t zrow,const uint16_t* Wv,float* Out,int nt){
    omp_set_num_threads(nt);
    const int64_t M=(int64_t)order.size();
    const int BS=TK/2*TN*2,AS=K*2,BS2=TN*4,CS=N*4;
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18); cfg_tiles();
        uint16_t* hbuf=(uint16_t*)aligned_alloc(64,MT*K*2);
        float* ctmp=(float*)aligned_alloc(64,MT*N*4);
        int32_t cur[MT]; uint32_t nodes[MT];
        #pragma omp for schedule(dynamic,4)
        for(int64_t i0=0;i0<M;i0+=MT){
            int batch=(int)std::min((int64_t)MT,M-i0);
            int max_deg=0;
            for(int r=0;r<batch;r++){
                nodes[r]=order[i0+r]; cur[r]=(int32_t)A.indptr[nodes[r]];
                int d=(int)(A.indptr[nodes[r]+1]-A.indptr[nodes[r]]);
                if(d>max_deg) max_deg=d;
            }
            for(int r=batch;r<MT;r++){nodes[r]=zrow;cur[r]=-1;}
            if(max_deg==0){for(int r=0;r<batch;r++)memset(Out+(size_t)nodes[r]*N,0,N*4);continue;}
            memset(ctmp,0,MT*N*4);
            for(int step=0;step<max_deg;step++){
                for(int r=0;r<MT;r++){
                    uint32_t j=(r<batch&&cur[r]<(int32_t)A.indptr[nodes[r]+1])
                               ?A.indices[cur[r]++]:zrow;
                    memcpy(hbuf+r*K,H+(size_t)j*K,K*2);
                }
                for(int np=0;np<N/(TN*2);np++){
                    if(step>0){_tile_loadd(0,ctmp+np*TN*2,CS);_tile_loadd(1,ctmp+np*TN*2+TN,CS);}
                    else{_tile_zero(0);_tile_zero(1);}
                    for(int kp=0;kp<K/TK;kp++){
                        _tile_loadd(2,hbuf+kp*TK,AS);
                        _tile_loadd(3,Wv+(kp*(N/TN)+np*2)*BS,BS2);
                        _tile_loadd(4,Wv+(kp*(N/TN)+np*2+1)*BS,BS2);
                        _tile_dpbf16ps(0,2,3); _tile_dpbf16ps(1,2,4);
                    }
                    _tile_stored(0,ctmp+np*TN*2,CS); _tile_stored(1,ctmp+np*TN*2+TN,CS);
                }
            }
            for(int r=0;r<batch;r++) memcpy(Out+(size_t)nodes[r]*N,ctmp+r*N,N*4);
        }
        free(hbuf);free(ctmp);_tile_release();
    }
}

// ── Fusion v7：v3 + NTA prefetch ────────────────────────────────
// _MM_HINT_NTA：Non-Temporal Access
//   告诉硬件：这条数据只用一次，用完不要留在 cache
//   效果：H 特征不污染 L1/L2，W 矩阵持续驻留
static void fusion_v7_nta(const CSR& A,const std::vector<uint32_t>& order,
    const uint16_t* H,uint32_t zrow,const uint16_t* Wv,float* Out,int nt){
    omp_set_num_threads(nt);
    const int64_t M=(int64_t)order.size();
    const int BS=TK/2*TN*2,AS=K*2,BS2=TN*4,CS=N*4;
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18); cfg_tiles();
        uint16_t* hbuf=(uint16_t*)aligned_alloc(64,MT*K*2);
        float* ctmp=(float*)aligned_alloc(64,MT*N*4);
        int32_t cur[MT]; uint32_t nodes[MT];
        // 预取游标（超前 PREFETCH_DIST 步）
        int32_t pcur[MT];

        #pragma omp for schedule(dynamic,4)
        for(int64_t i0=0;i0<M;i0+=MT){
            int batch=(int)std::min((int64_t)MT,M-i0);
            int max_deg=0;
            for(int r=0;r<batch;r++){
                nodes[r]=order[i0+r];
                cur[r]=(int32_t)A.indptr[nodes[r]];
                pcur[r]=(int32_t)A.indptr[nodes[r]];
                int d=(int)(A.indptr[nodes[r]+1]-A.indptr[nodes[r]]);
                if(d>max_deg) max_deg=d;
            }
            for(int r=batch;r<MT;r++){nodes[r]=zrow;cur[r]=-1;pcur[r]=-1;}
            if(max_deg==0){for(int r=0;r<batch;r++)memset(Out+(size_t)nodes[r]*N,0,N*4);continue;}

            // 初始化预取游标：提前 PREFETCH_DIST 步
            for(int pre=0;pre<PREFETCH_DIST && pre<max_deg;pre++){
                for(int r=0;r<MT;r++){
                    uint32_t j;
                    if(r<batch && pcur[r]<(int32_t)A.indptr[nodes[r]+1])
                        j=A.indices[pcur[r]++];
                    else
                        j=zrow;
                    // NTA 预取：每隔 64B（一个 cache line）预取一次
                    const uint16_t* src=H+(size_t)j*K;
                    for(int kb=0;kb<K*2;kb+=64)
                        _mm_prefetch((const char*)src+kb, _MM_HINT_NTA);
                }
            }

            memset(ctmp,0,MT*N*4);
            for(int step=0;step<max_deg;step++){
                // 发出下一步的 NTA 预取（超前 PREFETCH_DIST 步）
                int pre_step=step+PREFETCH_DIST;
                if(pre_step<max_deg){
                    for(int r=0;r<MT;r++){
                        uint32_t j;
                        if(r<batch && pcur[r]<(int32_t)A.indptr[nodes[r]+1])
                            j=A.indices[pcur[r]++];
                        else
                            j=zrow;
                        const uint16_t* src=H+(size_t)j*K;
                        for(int kb=0;kb<K*2;kb+=64)
                            _mm_prefetch((const char*)src+kb, _MM_HINT_NTA);
                    }
                }

                // Gather（数据已在路上，NTA 保证不污染 L1）
                for(int r=0;r<MT;r++){
                    uint32_t j=(r<batch&&cur[r]<(int32_t)A.indptr[nodes[r]+1])
                               ?A.indices[cur[r]++]:zrow;
                    memcpy(hbuf+r*K,H+(size_t)j*K,K*2);
                }

                // AMX 计算（与 v3 完全相同）
                for(int np=0;np<N/(TN*2);np++){
                    if(step>0){_tile_loadd(0,ctmp+np*TN*2,CS);_tile_loadd(1,ctmp+np*TN*2+TN,CS);}
                    else{_tile_zero(0);_tile_zero(1);}
                    for(int kp=0;kp<K/TK;kp++){
                        _tile_loadd(2,hbuf+kp*TK,AS);
                        _tile_loadd(3,Wv+(kp*(N/TN)+np*2)*BS,BS2);
                        _tile_loadd(4,Wv+(kp*(N/TN)+np*2+1)*BS,BS2);
                        _tile_dpbf16ps(0,2,3); _tile_dpbf16ps(1,2,4);
                    }
                    _tile_stored(0,ctmp+np*TN*2,CS); _tile_stored(1,ctmp+np*TN*2+TN,CS);
                }
            }
            for(int r=0;r<batch;r++) memcpy(Out+(size_t)nodes[r]*N,ctmp+r*N,N*4);
        }
        free(hbuf);free(ctmp);_tile_release();
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
    const char* BASE="/home/huangjianqiang_group/hdacp1/data/SpMM_project/data";
    // 重点测高度数矩阵：NTA 对高度数最有帮助
    const char* MATS[]={
        "cit-Patents/cit-Patents.csrbin",       // deg=4.4  低度数（对照）
        "web-Google/web-Google.csrbin",          // deg=5.6
        "as-Skitter/as-Skitter.csrbin",          // deg=13.1 中度数
        "soc-Pokec/soc-Pokec.csrbin",            // deg=18.8 高度数
        "hollywood-2009/hollywood-2009.csrbin",  // deg=99.9 超高度数
        nullptr
    };

    printf("=== Stage3 v7: NTA Prefetch vs v3 ===\n");
    printf("PREFETCH_DIST=%d  K=%d  N=%d  threads=%d\n\n",PREFETCH_DIST,K,N,nt);
    printf("%-22s %8s %10s %10s %10s %8s\n",
           "矩阵","avg_deg","v3(ms)","v7_nta(ms)","提升","theory");
    printf("%s\n",std::string(72,'-').c_str());

    srand(42);
    float* Wf=(float*)aligned_alloc(64,K*N*4);
    for(int i=0;i<K*N;i++) Wf[i]=(rand()%200-100)/100.f;
    uint16_t* Wv=pack_W(Wf); free(Wf);

    for(int mi=0;MATS[mi];mi++){
        char path[512]; snprintf(path,512,"%s/%s",BASE,MATS[mi]);
        CSR A; if(!load_csr(path,A)) continue;
        const char* name=strrchr(MATS[mi],'/')+1;
        double avg_deg=(double)A.nnz/A.nrow;

        size_t Hr=A.ncol+1;
        uint16_t* H=(uint16_t*)aligned_alloc(64,Hr*K*2);
        for(size_t i=0;i<A.ncol*(size_t)K;i++) H[i]=f32bf16((rand()%200-100)/100.f);
        memset(H+A.ncol*K,0,K*2);
        uint32_t zrow=(uint32_t)A.ncol;

        std::vector<uint32_t> order(A.nrow);
        std::iota(order.begin(),order.end(),0);
        std::sort(order.begin(),order.end(),[&](uint32_t a,uint32_t b){
            return (A.indptr[a+1]-A.indptr[a])>(A.indptr[b+1]-A.indptr[b]);
        });

        float* Zv3=(float*)aligned_alloc(64,A.nrow*N*4);
        float* Zv7=(float*)aligned_alloc(64,A.nrow*N*4);

        double ms_v3=bench([&](){
            memset(Zv3,0,A.nrow*N*4);
            fusion_v3(A,order,H,zrow,Wv,Zv3,nt);
        },WARMUP,REPEAT);

        double ms_v7=bench([&](){
            memset(Zv7,0,A.nrow*N*4);
            fusion_v7_nta(A,order,H,zrow,Wv,Zv7,nt);
        },WARMUP,REPEAT);

        printf("%-22s %8.1f %10.2f %10.2f %10.2f× %8.2f×\n",
               name,avg_deg,ms_v3,ms_v7,ms_v3/ms_v7,1.0+2.0/avg_deg);

        free(H);free(Zv3);free(Zv7);
    }
    free(Wv);
    printf("\n提升 >1.0× 表示 NTA 有效（W 驻留率提升）\n");
    printf("预期：高度数矩阵改善更明显（原来 cache eviction 更严重）\n");
    printf("\n=== Done ===\n");
    return 0;
}
