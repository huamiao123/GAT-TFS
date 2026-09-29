/**
 * Fusion Kernel 最终测试：全部 7 个基准矩阵
 * 使用 v3（度数降序排序 + node-parallel tiling）
 * 对比：FB-BF16 SpMM（baseline）
 * 输出：完整的 speedup vs avg_deg 曲线数据
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

static inline uint16_t f32bf16(float f){uint32_t u;memcpy(&u,&f,4);return(uint16_t)(u>>16);}
static inline float bf16f32(uint16_t b){uint32_t u=(uint32_t)b<<16;float f;memcpy(&f,&u,4);return f;}

struct CSR{ uint64_t nrow,ncol,nnz; std::vector<uint32_t> indptr,indices; };
static bool load_csr(const char* p,CSR& m){
    FILE* fp=fopen(p,"rb");if(!fp){printf("SKIP(not found): %s\n",p);return false;}
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

// ── Fusion v3（最终版）──────────────────────────────────────────
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
    const char* MATS[]={
        "cit-Patents/cit-Patents.csrbin",
        "web-Google/web-Google.csrbin",
        "amazon0601/amazon0601.csrbin",
        "as-Skitter/as-Skitter.csrbin",
        "indochina/indochina.csrbin",
        "soc-Pokec/soc-Pokec.csrbin",
        "hollywood-2009/hollywood-2009.csrbin",
        nullptr
    };

    printf("=== AMX Fusion Final: 全矩阵测试 ===\n");
    printf("kernel: fusion_v3 (node-parallel + degree sorting)\n");
    printf("K=%d  N=%d  threads=%d  repeat=%d\n\n",K,N,nt,REPEAT);

    printf("%-24s %8s %8s %12s %14s %10s %8s\n",
           "矩阵","NNZ(M)","avg_deg",
           "FB-BF16(ms)","Fusion_v3(ms)","vs_FB","theory");
    printf("%s\n",std::string(90,'-').c_str());

    srand(42);
    float* Wf=(float*)aligned_alloc(64,K*N*4);
    for(int i=0;i<K*N;i++) Wf[i]=(rand()%200-100)/100.f;
    uint16_t* Wv=pack_W(Wf); free(Wf);

    double geomean_num=1.0; int cnt=0;

    for(int mi=0;MATS[mi];mi++){
        char path[512]; snprintf(path,512,"%s/%s",BASE,MATS[mi]);
        CSR A; if(!load_csr(path,A)) continue;
        const char* name=strrchr(MATS[mi],'/')+1;
        double avg_deg=(double)A.nnz/A.nrow;
        double theory=1.0+2.0/avg_deg;

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

        float* Zfb=(float*)aligned_alloc(64,A.nrow*K*4);
        float* Zv3=(float*)aligned_alloc(64,A.nrow*N*4);
        memset(Zfb,0,A.nrow*K*4);

        double ms_fb=bench([&](){fb_spmm(A,H,Zfb,nt);},WARMUP,REPEAT);
        double ms_v3=bench([&](){
            memset(Zv3,0,A.nrow*N*4);
            fusion_v3(A,order,H,zrow,Wv,Zv3,nt);
        },WARMUP,REPEAT);

        double speedup=ms_fb/ms_v3;
        geomean_num*=speedup; cnt++;

        printf("%-24s %8.1f %8.1f %12.2f %14.2f %10.2f× %8.2f×\n",
               name,A.nnz/1e6,avg_deg,ms_fb,ms_v3,speedup,theory);

        free(H);free(Zfb);free(Zv3);
    }
    free(Wv);

    if(cnt>0){
        double geomean=pow(geomean_num,1.0/cnt);
        printf("%s\n",std::string(90,'-').c_str());
        printf("%-24s %8s %8s %12s %14s %10.2f×\n",
               "几何均值","","","","",geomean);
    }

    printf("\n说明：\n");
    printf("  vs_FB  = FB-BF16(仅SpMM) / Fusion_v3(SpMM+GeMM)\n");
    printf("  >1.0×  = 融合后做了更多工作反而更快（HBM流量节省超过计算开销）\n");
    printf("  theory = 1+2/avg_deg（IO节省模型预测值）\n");
    printf("  公平对比需 vs (FB-BF16 + GeMM) / Fusion_v3\n");
    printf("\n=== Done ===\n");
    return 0;
}
