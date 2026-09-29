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
#include <string>

typedef struct {
    uint8_t palette_id, start_row, reserved0[14];
    uint16_t colsb[16];
    uint8_t rows[16];
} __attribute__((packed)) tile_config_t;
static_assert(sizeof(tile_config_t)==64,"");

static constexpr int K=128, N=128, MT=16, TK=32, TN=16;
static constexpr int REPEAT=10, WARMUP=3;

static inline uint16_t f32bf16(float f){uint32_t u;memcpy(&u,&f,4);return(uint16_t)(u>>16);}
static inline float bf16f32(uint16_t b){uint32_t u=(uint32_t)b<<16;float f;memcpy(&f,&u,4);return f;}

struct CSR{ uint64_t nrow,ncol,nnz; std::vector<uint32_t> indptr,indices; };
static bool load(const char* p, CSR& m){
    FILE* fp=fopen(p,"rb"); if(!fp){printf("cannot open %s\n",p);return false;}
    uint32_t a,b,c; fread(&a,4,1,fp);fread(&b,4,1,fp);fread(&c,4,1,fp);
    fread(&m.nrow,8,1,fp);fread(&m.ncol,8,1,fp);fread(&m.nnz,8,1,fp);
    m.indptr.resize(m.nrow+1);m.indices.resize(m.nnz);
    fread(m.indptr.data(),4,m.nrow+1,fp);fread(m.indices.data(),4,m.nnz,fp);
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

// ── FB-BF16 SpMM baseline ────────────────────────────────────────
static void fb_spmm(const CSR& A, const uint16_t* H, float* Z, int nt){
    omp_set_num_threads(nt);
    #pragma omp parallel for schedule(dynamic,64)
    for(int64_t i=0;i<(int64_t)A.nrow;i++){
        float* z=Z+i*K; memset(z,0,K*4);
        for(int64_t p=A.indptr[i];p<(int64_t)A.indptr[i+1];p++)
            for(int k=0;k<K;k++)
                z[k]+=bf16f32(H[(size_t)A.indices[p]*K+k]);
    }
}

// ── Node-Parallel Fusion Kernel ──────────────────────────────────
// tile 的 16 行 = 16 个目标节点，无 reduction，直接写回
static void fusion_v2(const CSR& A, const uint16_t* H, uint32_t zrow,
                      const uint16_t* Wv, float* Out, int nt){
    omp_set_num_threads(nt);
    const int BS=TK/2*TN*2, AS=K*2, BS2=TN*4, CS=N*4;

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        cfg_tiles();

        uint16_t* hbuf=(uint16_t*)aligned_alloc(64,MT*K*2);
        float*    ctmp=(float*)   aligned_alloc(64,MT*N*4);  // 安全写缓冲
        int32_t   cur[MT];

        #pragma omp for schedule(dynamic,4)
        for(int64_t i0=0;i0<(int64_t)A.nrow;i0+=MT){
            int batch=(int)std::min((int64_t)MT,(int64_t)A.nrow-i0);

            // 初始化游标，计算 max_deg
            int max_deg=0;
            for(int r=0;r<batch;r++){
                cur[r]=(int32_t)A.indptr[i0+r];
                int d=(int)(A.indptr[i0+r+1]-A.indptr[i0+r]);
                if(d>max_deg) max_deg=d;
            }
            for(int r=batch;r<MT;r++) cur[r]=-1;

            // 孤立批
            if(max_deg==0){
                for(int r=0;r<batch;r++) memset(Out+(i0+r)*N,0,N*4);
                continue;
            }

            // C_tmp 清零（MT×N FP32 安全缓冲）
            memset(ctmp,0,MT*N*4);

            // 内层循环：step × MT 次 gather + AMX
            for(int step=0;step<max_deg;step++){
                // Gather：每节点取一个邻居，用完则指向 zero_row
                for(int r=0;r<MT;r++){
                    uint32_t j;
                    if(r<batch && cur[r]<(int32_t)A.indptr[i0+r+1])
                        j=A.indices[cur[r]++];
                    else
                        j=zrow;
                    memcpy(hbuf+r*K, H+(size_t)j*K, K*2);
                }

                // AMX：hbuf(MT×K) × W(K×N) 累加到 ctmp(MT×N)
                for(int np=0;np<N/(TN*2);np++){
                    // 加载累加器（step>0 时从 ctmp 恢复）
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
                    // 写回 ctmp（始终安全）
                    _tile_stored(0,ctmp+np*TN*2,   CS);
                    _tile_stored(1,ctmp+np*TN*2+TN,CS);
                }
            }

            // 把 ctmp 的前 batch 行写回 Out（不越界）
            for(int r=0;r<batch;r++)
                memcpy(Out+(i0+r)*N, ctmp+r*N, N*4);
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

int main(int argc,char** argv){
    int nt=argc>=2?atoi(argv[1]):32;
    const char* MATS[]={
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/web-Google/web-Google.csrbin",
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/amazon0601/amazon0601.csrbin",
        nullptr
    };
    printf("=== Stage3 v2: Node-Parallel Tiling ===\n");
    printf("K=%d  N=%d  threads=%d  repeat=%d\n\n",K,N,nt,REPEAT);
    printf("%-20s %8s %8s %12s %14s %8s\n",
           "矩阵","NNZ(M)","avg_deg","FB-BF16(ms)","Fusion_v2(ms)","加速比");
    printf("%s\n",std::string(76,'-').c_str());

    srand(42);
    float* Wf=(float*)aligned_alloc(64,K*N*4);
    for(int i=0;i<K*N;i++) Wf[i]=(rand()%200-100)/100.f;
    uint16_t* Wv=pack_W(Wf); free(Wf);

    for(int mi=0;MATS[mi];mi++){
        CSR A; if(!load(MATS[mi],A)) continue;
        const char* name=strrchr(MATS[mi],'/')+1;
        printf("%-20s %8.1f %8.1f",name,A.nnz/1e6,(double)A.nnz/A.nrow);
        fflush(stdout);

        // H：ncol+1 行，末尾全零
        size_t Hr=A.ncol+1;
        uint16_t* H=(uint16_t*)aligned_alloc(64,Hr*K*2);
        for(size_t i=0;i<A.ncol*(size_t)K;i++) H[i]=f32bf16((rand()%200-100)/100.f);
        memset(H+A.ncol*K,0,K*2);  // zero row
        uint32_t zrow=(uint32_t)A.ncol;

        float* Zfb =(float*)aligned_alloc(64,A.nrow*K*4);
        float* Zv2 =(float*)aligned_alloc(64,A.nrow*N*4);
        memset(Zfb,0,A.nrow*K*4);
        memset(Zv2,0,A.nrow*N*4);

        double ms_fb=bench([&](){fb_spmm(A,H,Zfb,nt);},WARMUP,REPEAT);
        printf(" %12.2f",ms_fb); fflush(stdout);

        double ms_v2=bench([&](){
            memset(Zv2,0,A.nrow*N*4);
            fusion_v2(A,H,zrow,Wv,Zv2,nt);
        },WARMUP,REPEAT);
        printf(" %14.2f %8.2f×\n",ms_v2,ms_fb/ms_v2);

        free(H); free(Zfb); free(Zv2);
    }
    free(Wv);
    printf("\n注：Fusion_v2 同时完成 SpMM+GeMM\n");
    printf("    FB-BF16 只做 SpMM，公平对比需加上 GeMM(~4ms)\n");
    printf("    两步总计 ≈ 22ms(web-Google) / 10ms(amazon0601)\n");
    printf("\n=== Done ===\n");
    return 0;
}
