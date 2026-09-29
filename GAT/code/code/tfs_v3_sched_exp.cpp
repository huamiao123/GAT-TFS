/*
 * tfs_v3_sched_exp.cpp -- TFS V3 Scheduling Experiment
 *
 * Identical kernel to amx_tfs_v3.cpp. Only the schedule clause varies.
 *
 * BUILD:
 *   icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 \
 *        -qopenmp -o tfs_v3_sched_exp code/tfs_v3_sched_exp.cpp
 *
 * USAGE:
 *   OMP_NUM_THREADS=32 ./tfs_v3_sched_exp <data_dir> <matrix> <sched> [R=64]
 *   sched: 0=static 1=dynamic,1 2=dynamic,4 3=guided 4=dynamic,1(V3)
 */

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <omp.h>

#define K_IN   128
#define K_OUT  128
#define KB     4
#define NB     8
#define TR     16
#define NP     2
#define TC0 0
#define TC1 1
#define TC2 2
#define TC3 3
#define TA  4
#define TB0 5
#define TB1 6

struct __attribute__((aligned(64))) tilecfg_t {
    uint8_t  palette;
    uint8_t  start_row;
    uint8_t  reserved[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
};

static void setup_tilecfg(tilecfg_t *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->palette = 1;
    for (int t = 0; t < 7; t++) { cfg->rows[t] = TR; cfg->colsb[t] = 64; }
}

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return (uint16_t)(u >> 16);
}
static inline float bf16_to_f32(uint16_t b) {
    uint32_t u = (uint32_t)b << 16; float f; memcpy(&f, &u, 4); return f;
}

static void make_W_vnni(const float *W, uint16_t *Wv) {
    for (int kb = 0; kb < KB; kb++)
        for (int ob = 0; ob < NB; ob++)
            for (int kp = 0; kp < 16; kp++)
                for (int n = 0; n < 16; n++) {
                    int k0 = kb*32+kp*2, k1 = k0+1, col = ob*16+n;
                    uint16_t v0 = f32_to_bf16(W[k0*K_OUT+col]);
                    uint16_t v1 = f32_to_bf16(W[k1*K_OUT+col]);
                    int base = ((kb*NB+ob)*16+kp)*32+n*2;
                    memcpy(&Wv[base],   &v0, 2);
                    memcpy(&Wv[base+1], &v1, 2);
                }
}

static void convert_H_bf16(const float *H, uint16_t *Hb, int N) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++)
        for (int k = 0; k < K_IN; k++)
            Hb[(size_t)i*K_IN+k] = f32_to_bf16(H[(size_t)i*K_IN+k]);
}

static int* make_degree_perm(const uint32_t *indptr, int N) {
    int *perm = (int*)malloc((size_t)N*sizeof(int));
    for (int i = 0; i < N; i++) perm[i] = i;
    const uint32_t *ip = indptr;
    std::sort(perm, perm+N, [ip](int a, int b) {
        return (ip[a+1]-ip[a]) < (ip[b+1]-ip[b]);
    });
    return perm;
}

static void print_deg_stats(const uint32_t *indptr, int N) {
    uint32_t mn=~0u, mx=0; double sum=0;
    for (int i=0;i<N;i++) {
        uint32_t d=indptr[i+1]-indptr[i];
        if(d<mn)mn=d; if(d>mx)mx=d; sum+=d;
    }
    printf("  deg: min=%u max=%u avg=%.1f ratio=%.0f\n",mn,mx,sum/N,(double)mx/(sum/N));
}

/* Per-thread stats */
struct __attribute__((aligned(64))) tstats_t {
    double wall_time; uint64_t total_nb; uint64_t num_grp; uint32_t max_deg;
};
static tstats_t g_ts[256];

/* ================================================================
 * Kernel body macro — identical to amx_tfs_v3.cpp
 * ================================================================ */
#define KERNEL_BODY(rg, R, N, indptr, indices, Hb, Wv, C, perm, tid) \
do { \
    int rg_end = ((rg)+(R)<(N)) ? ((rg)+(R)) : (N); \
    for (int i=(rg); i<rg_end; i+=TR) { \
        int batch = ((i+TR)<=rg_end) ? TR : (rg_end-i); \
        int max_deg = 0; \
        for (int n=0; n<batch; n++) { \
            int row = (perm)[i+n]; \
            orig_row[n]=row; base_local[n]=(indptr)[row]; \
            deg_local[n]=(indptr)[row+1]-(indptr)[row]; \
            if((int)deg_local[n]>max_deg) max_deg=(int)deg_local[n]; \
        } \
        { uint64_t gnb=0; for(int n=0;n<batch;n++) gnb+=deg_local[n]; \
          g_ts[tid].total_nb+=gnb; g_ts[tid].num_grp++; \
          if((uint32_t)max_deg>g_ts[tid].max_deg) g_ts[tid].max_deg=(uint32_t)max_deg; } \
        for (int obp=0; obp<NP; obp++) { \
            _tile_zero(TC0); _tile_zero(TC1); _tile_zero(TC2); _tile_zero(TC3); \
            memset(Hbuf, 0, TR*K_IN*sizeof(uint16_t)); \
            int active_from = 0; \
            for (int s=0; s<max_deg; s++) { \
                if (s+1<max_deg) { \
                    for (int nn=active_from; nn<batch; nn++) { \
                        if ((uint32_t)(s+1)<deg_local[nn]) { \
                            uint32_t jn=(indices)[base_local[nn]+s+1]; \
                            const char *a=(const char*)&(Hb)[(size_t)jn*K_IN]; \
                            _mm_prefetch(a,_MM_HINT_T0); _mm_prefetch(a+64,_MM_HINT_T0); \
                            _mm_prefetch(a+128,_MM_HINT_T0); _mm_prefetch(a+192,_MM_HINT_T0); \
                        } \
                    } \
                } \
                while (active_from<batch && (uint32_t)s>=deg_local[active_from]) { \
                    memset(&Hbuf[active_from*K_IN],0,K_IN*sizeof(uint16_t)); active_from++; \
                } \
                if (active_from>=batch) break; \
                for (int nn=active_from; nn<batch; nn++) { \
                    uint32_t j=(indices)[base_local[nn]+s]; \
                    memcpy(&Hbuf[nn*K_IN],&(Hb)[(size_t)j*K_IN],K_IN*sizeof(uint16_t)); \
                } \
                for (int kb=0; kb<KB; kb++) { \
                    _tile_loadd(TA,(const uint8_t*)Hbuf+kb*64,K_IN*2); \
                    int ob0=obp*4; const uint16_t *Wb=(Wv); \
                    int w0=((kb)*NB+(ob0+0))*16*32, w1=((kb)*NB+(ob0+1))*16*32; \
                    int w2=((kb)*NB+(ob0+2))*16*32, w3=((kb)*NB+(ob0+3))*16*32; \
                    _tile_loadd(TB0,&Wb[w0],64); _tile_loadd(TB1,&Wb[w1],64); \
                    _tile_dpbf16ps(TC0,TA,TB0); _tile_dpbf16ps(TC1,TA,TB1); \
                    _tile_loadd(TB0,&Wb[w2],64); _tile_loadd(TB1,&Wb[w3],64); \
                    _tile_dpbf16ps(TC2,TA,TB0); _tile_dpbf16ps(TC3,TA,TB1); \
                } \
            } \
            int col=obp*64; \
            _tile_stored(TC0,Ctmp,64); for(int nn=0;nn<batch;nn++) memcpy(&(C)[(size_t)orig_row[nn]*K_OUT+col+0],&Ctmp[nn*16],64); \
            _tile_stored(TC1,Ctmp,64); for(int nn=0;nn<batch;nn++) memcpy(&(C)[(size_t)orig_row[nn]*K_OUT+col+16],&Ctmp[nn*16],64); \
            _tile_stored(TC2,Ctmp,64); for(int nn=0;nn<batch;nn++) memcpy(&(C)[(size_t)orig_row[nn]*K_OUT+col+32],&Ctmp[nn*16],64); \
            _tile_stored(TC3,Ctmp,64); for(int nn=0;nn<batch;nn++) memcpy(&(C)[(size_t)orig_row[nn]*K_OUT+col+48],&Ctmp[nn*16],64); \
        } \
    } \
} while(0)

/* ================================================================
 * Kernel with configurable schedule
 * ================================================================ */
static void tfs_v3_sched(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb, const uint16_t *Wv,
    float *C, const int *perm, int N, int R, int sm)
{
    memset(C, 0, (size_t)N*K_OUT*sizeof(float));
    int nt=omp_get_max_threads();
    for(int t=0;t<nt;t++) { g_ts[t]={0,0,0,0}; }

    #pragma omp parallel
    {
        if (syscall(SYS_arch_prctl,0x1023,18)!=0) { perror("arch_prctl"); exit(1); }
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);
        uint16_t Hbuf[TR*K_IN] __attribute__((aligned(64)));
        float Ctmp[TR*16] __attribute__((aligned(64)));
        uint32_t base_local[TR], deg_local[TR]; int orig_row[TR];
        int tid=omp_get_thread_num();
        double t0=omp_get_wtime();

        switch(sm) {
        case 0: {
            #pragma omp for schedule(static) nowait
            for(int rg=0;rg<N;rg+=R) { KERNEL_BODY(rg,R,N,indptr,indices,Hb,Wv,C,perm,tid); }
        } break;
        case 1: case 4: {
            #pragma omp for schedule(dynamic,1) nowait
            for(int rg=0;rg<N;rg+=R) { KERNEL_BODY(rg,R,N,indptr,indices,Hb,Wv,C,perm,tid); }
        } break;
        case 2: {
            #pragma omp for schedule(dynamic,4) nowait
            for(int rg=0;rg<N;rg+=R) { KERNEL_BODY(rg,R,N,indptr,indices,Hb,Wv,C,perm,tid); }
        } break;
        case 3: {
            #pragma omp for schedule(guided) nowait
            for(int rg=0;rg<N;rg+=R) { KERNEL_BODY(rg,R,N,indptr,indices,Hb,Wv,C,perm,tid); }
        } break;
        }

        double t1=omp_get_wtime();
        g_ts[tid].wall_time=t1-t0;
        _tile_release();
    }
}

/* CSR loader */
struct csr_t { uint32_t *indptr,*indices; int N; uint32_t nnz; };
static csr_t load_csrbin(const char *path) {
    FILE *f=fopen(path,"rb");
    if(!f){fprintf(stderr,"Cannot open %s\n",path);exit(1);}
    uint8_t hdr[36]; fread(hdr,1,36,f);
    uint32_t type; uint64_t nr,nc,nz;
    memcpy(&type,&hdr[8],4); memcpy(&nr,&hdr[12],8);
    memcpy(&nc,&hdr[20],8); memcpy(&nz,&hdr[28],8);
    int N=(int)nr; uint32_t nnz=(uint32_t)nz;
    printf("  csrbin: N=%d NNZ=%u\n",N,nnz);
    uint32_t *indptr=(uint32_t*)malloc((size_t)(N+1)*4);
    uint32_t *indices=(uint32_t*)malloc((size_t)nnz*4);
    fread(indptr,4,N+1,f); fread(indices,4,nnz,f); fclose(f);
    return {indptr,indices,N,nnz};
}

static void print_thread_stats(int nt) {
    double mx=0,mn=1e30,sum=0;
    for(int t=0;t<nt;t++){double ms=g_ts[t].wall_time*1000;if(ms>mx)mx=ms;if(ms<mn)mn=ms;sum+=ms;}
    double mean=sum/nt;
    printf("  Thread: max=%.2f min=%.2f mean=%.2f ms  imbal=%.4f waste=%.1f%%\n",
           mx,mn,mean,mx/mean,(1-mean/mx)*100);
    printf("  %-4s %9s %8s %12s %8s\n","T","Time(ms)","Groups","Neighbors","MaxDeg");
    for(int t=0;t<nt;t++){
        double ms=g_ts[t].wall_time*1000;
        printf("  %-4d %9.2f %8lu %12lu %8u",t,ms,g_ts[t].num_grp,g_ts[t].total_nb,g_ts[t].max_deg);
        if(ms>=mx-0.001)printf(" <-MAX");
        if(ms<=mn+0.001)printf(" <-MIN");
        printf("\n");
    }
}

static void benchmark(const char *dir, const char *name, int R, int sm) {
    const char *sn[]={"static","dynamic,1","dynamic,4","guided","dynamic,1(V3)"};
    char path[512]; snprintf(path,512,"%s/%s/%s.csrbin",dir,name,name);
    printf("Loading: %s\n",path);
    csr_t csr=load_csrbin(path);
    int N=csr.N; uint32_t nnz=csr.nnz;
    printf("Matrix: %s N=%d NNZ=%u avg_deg=%.1f\n",name,N,nnz,(double)nnz/N);
    print_deg_stats(csr.indptr,N);
    printf("Schedule: %s R=%d\n",sn[sm],R);

    int *perm=make_degree_perm(csr.indptr,N);
    srand(12345);
    float *H=(float*)malloc((size_t)N*K_IN*4);
    float *W=(float*)malloc((size_t)K_IN*K_OUT*4);
    for(size_t i=0;i<(size_t)N*K_IN;i++) H[i]=0.01f*((rand()%200)-100);
    for(int i=0;i<K_IN*K_OUT;i++) W[i]=0.01f*((rand()%200)-100);
    uint16_t *Hb=(uint16_t*)aligned_alloc(64,(size_t)N*K_IN*2);
    uint16_t *Wv=(uint16_t*)aligned_alloc(64,(size_t)KB*NB*16*32*2);
    convert_H_bf16(H,Hb,N); make_W_vnni(W,Wv);
    float *C=(float*)calloc((size_t)N*K_OUT,4);
    int nt=omp_get_max_threads();

    tfs_v3_sched(csr.indptr,csr.indices,Hb,Wv,C,perm,N,R,sm); /* warmup */
    const int RUNS=5; double times[5];
    for(int r=0;r<RUNS;r++){
        double t0=omp_get_wtime();
        tfs_v3_sched(csr.indptr,csr.indices,Hb,Wv,C,perm,N,R,sm);
        double t1=omp_get_wtime();
        times[r]=(t1-t0)*1000; printf("  run %d: %.2f ms\n",r,times[r]);
    }
    double st[5]; memcpy(st,times,sizeof(times)); std::sort(st,st+RUNS);
    double median=st[RUNS/2];
    printf("  MEDIAN: %.2f ms\n\n",median);

    printf("--- Per-Thread Stats (last run) ---\n");
    print_thread_stats(nt);

    double mx=0,sum=0;
    for(int t=0;t<nt;t++){double ms=g_ts[t].wall_time*1000;if(ms>mx)mx=ms;sum+=ms;}
    double mean=sum/nt;
    printf("\nSUMMARY|%s|%s|R=%d|%d|%.2f|%.4f|%.1f\n",
           name,sn[sm],R,nt,median,mx/mean,(1-mean/mx)*100);
    printf("========================================\n\n");

    free(csr.indptr);free(csr.indices);
    free(H);free(W);free(Hb);free(Wv);free(C);free(perm);
}

int main(int argc, char **argv) {
    printf("TFS V3 Scheduling Experiment\nThreads: %d\n\n",omp_get_max_threads());
    if(argc<4){
        fprintf(stderr,"Usage: %s <data_dir> <matrix> <sched> [R=64]\n"
            "  sched: 0=static 1=dynamic,1 2=dynamic,4 3=guided 4=V3baseline\n",argv[0]);
        return 1;
    }
    benchmark(argv[1],argv[2],(argc>=5)?atoi(argv[4]):64,atoi(argv[3]));
    return 0;
}
