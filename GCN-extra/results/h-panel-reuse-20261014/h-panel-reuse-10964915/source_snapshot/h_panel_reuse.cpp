#define main frozen_gcn_e2e_main
#include "original_gcn_e2e_v3.cpp"
#undef main

#include <array>
#include <chrono>
#include <vector>
#include <string>
#include <numeric>
#include <limits>
#include <type_traits>

// A0 is the unmodified tfs_v3_* function from the included original source.
// A1 is window=1; A2-A5 use window=8,16,32,64. Duplicate-gather at 16
// is a matched window/staging control, not a proposed optimization.
struct Choice { const char *name; int window; bool duplicate_gather; };
static const Choice choices[] = {
    {"A0_original", 0, false}, {"A1_step", 1, false},
    {"A2_window8", 8, false}, {"A3_window16", 16, false},
    {"A4_window32", 32, false}, {"A5_window64", 64, false},
    {"control_window16_duplicate_gather", 16, true}
};

template<bool BF16_OUT>
static void scatter_panel(const float *ctmp, int tile, int panel,
                          const int *rows, int batch,
                          typename std::conditional<BF16_OUT,uint16_t,float>::type *out) {
    const int col = panel*64 + tile*16;
    for (int n=0; n<batch; ++n) {
        auto *dst = out + (size_t)rows[n]*128 + col;
        const float *src = ctmp + n*16;
        if constexpr (BF16_OUT) {
            for (int k=0;k<16;k++) dst[k]=f32_to_bf16(src[k]);
        } else memcpy(dst,src,64);
    }
}

template<bool BF16_OUT>
static void window_kernel(const uint32_t *indptr,const uint32_t *indices,
                          const uint16_t *h,const uint16_t *w,
                          typename std::conditional<BF16_OUT,uint16_t,float>::type *out,
                          const int *perm,int N,int window,bool duplicate_gather) {
    if(window<1 || window>64) abort();
    #pragma omp parallel
    {
        if(syscall(SYS_arch_prctl,0x1023,18)!=0){perror("arch_prctl");abort();}
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);
        alignas(64) uint16_t hbuf[64*TR*K_DIM];
        alignas(64) float cstate[NP][4][TR*16];
        alignas(64) float ctmp[TR*16];
        uint32_t base[TR],degree[TR];int rows[TR];
        #pragma omp for schedule(dynamic,1) nowait
        for(int rg=0;rg<N;rg+=64) {
            int rg_end=std::min(N,rg+64);
            for(int i=rg;i<rg_end;i+=TR) {
                int batch=std::min(TR,rg_end-i),max_degree=0;
                for(int n=0;n<batch;n++) {
                    rows[n]=perm[i+n];base[n]=indptr[rows[n]];
                    degree[n]=indptr[rows[n]+1]-base[n];
                    max_degree=std::max(max_degree,(int)degree[n]);
                }
                if(max_degree==0) {
                    for(int n=0;n<batch;n++)
                        memset(out+(size_t)rows[n]*128,0,128*sizeof(*out));
                    continue;
                }
                for(int ws=0;ws<max_degree;ws+=window) {
                    int count=std::min(window,max_degree-ws);
                    for(int panel=0;panel<NP;panel++) {
                        if(panel==0 || duplicate_gather) {
                            for(int q=0;q<count;q++) {
                                int s=ws+q;
                                uint16_t *slot=hbuf+(size_t)q*TR*K_DIM;
                                // Preserve original T0 prefetch distance during fill.
                                if(s+1<max_degree) for(int n=0;n<batch;n++)
                                    if((uint32_t)(s+1)<degree[n]) {
                                        uint32_t next=indices[base[n]+s+1];
                                        const char *p=(const char*)(h+(size_t)next*K_DIM);
                                        _mm_prefetch(p,_MM_HINT_T0);
                                        _mm_prefetch(p+64,_MM_HINT_T0);
                                        _mm_prefetch(p+128,_MM_HINT_T0);
                                        _mm_prefetch(p+192,_MM_HINT_T0);
                                    }
                                for(int n=0;n<batch;n++) {
                                    uint16_t *dst=slot+n*K_DIM;
                                    if((uint32_t)s<degree[n]) {
                                        uint32_t j=indices[base[n]+s];
                                        memcpy(dst,h+(size_t)j*K_DIM,K_DIM*2);
                                    } else memset(dst,0,K_DIM*2);
                                }
                                if(batch<TR) memset(slot+batch*K_DIM,0,(TR-batch)*K_DIM*2);
                            }
                        }
                        if(ws==0) {
                            _tile_zero(TC0);_tile_zero(TC1);
                            _tile_zero(TC2);_tile_zero(TC3);
                        } else {
                            _tile_loadd(TC0,cstate[panel][0],64);
                            _tile_loadd(TC1,cstate[panel][1],64);
                            _tile_loadd(TC2,cstate[panel][2],64);
                            _tile_loadd(TC3,cstate[panel][3],64);
                        }
                        for(int q=0;q<count;q++) {
                            const uint16_t *slot=hbuf+(size_t)q*TR*K_DIM;
                            for(int kb=0;kb<KB;kb++) {
                                _tile_loadd(TA,slot+kb*32,K_DIM*2);
                                int ob0=panel*4;
                                _tile_loadd(TB0,w+((kb*NB+ob0+0)*16)*32,64);
                                _tile_loadd(TB1,w+((kb*NB+ob0+1)*16)*32,64);
                                _tile_dpbf16ps(TC0,TA,TB0);
                                _tile_dpbf16ps(TC1,TA,TB1);
                                _tile_loadd(TB0,w+((kb*NB+ob0+2)*16)*32,64);
                                _tile_loadd(TB1,w+((kb*NB+ob0+3)*16)*32,64);
                                _tile_dpbf16ps(TC2,TA,TB0);
                                _tile_dpbf16ps(TC3,TA,TB1);
                            }
                        }
                        if(ws+count<max_degree) {
                            _tile_stored(TC0,cstate[panel][0],64);
                            _tile_stored(TC1,cstate[panel][1],64);
                            _tile_stored(TC2,cstate[panel][2],64);
                            _tile_stored(TC3,cstate[panel][3],64);
                        } else {
                            _tile_stored(TC0,ctmp,64);scatter_panel<BF16_OUT>(ctmp,0,panel,rows,batch,out);
                            _tile_stored(TC1,ctmp,64);scatter_panel<BF16_OUT>(ctmp,1,panel,rows,batch,out);
                            _tile_stored(TC2,ctmp,64);scatter_panel<BF16_OUT>(ctmp,2,panel,rows,batch,out);
                            _tile_stored(TC3,ctmp,64);scatter_panel<BF16_OUT>(ctmp,3,panel,rows,batch,out);
                        }
                    }
                }
            }
        }
        _tile_release();
    }
}

template<bool BF16_OUT>
static void one(const Choice &c,const csr_t &g,const uint16_t *h,const uint16_t *w,
                typename std::conditional<BF16_OUT,uint16_t,float>::type *out,const int *perm) {
    if(c.window==0) {
        if constexpr(BF16_OUT) tfs_v3_bf16out(g.indptr,g.indices,h,w,out,perm,g.N,64);
        else tfs_v3_fp32out(g.indptr,g.indices,h,w,out,perm,g.N,64);
    } else window_kernel<BF16_OUT>(g.indptr,g.indices,h,w,out,perm,g.N,c.window,c.duplicate_gather);
}

static bool equal_bits(const void *a,const void *b,size_t nbytes) {return memcmp(a,b,nbytes)==0;}
static double now_ms(){return omp_get_wtime()*1000.0;}
struct Times {double l1,relu,convert,l2,total;};

int main(int argc,char **argv) {
    if(argc<3) {fprintf(stderr,"Usage: %s <dataset_root> <graph> [repeats]\n",argv[0]);return 2;}
    int repeats=argc>3?atoi(argv[3]):5;
    char path[1024];snprintf(path,sizeof(path),"%s/%s/%s.csrbin",argv[1],argv[2],argv[2]);
    csr_t g=load_csrbin(path);int N=g.N;size_t sz=(size_t)N*128;
    srand(12345);
    float *w1=(float*)mkl_malloc(128*128*4,64),*w2=(float*)mkl_malloc(128*128*4,64);
    float *h0=(float*)mkl_malloc(sz*4,64);
    for(int i=0;i<128*128;i++){w1[i]=.01f*((rand()%200)-100);w2[i]=.01f*((rand()%200)-100);}
    for(size_t i=0;i<sz;i++)h0[i]=.01f*((rand()%200)-100);
    int *perm=make_degree_perm(g.indptr,N);
    uint16_t *wv1=(uint16_t*)aligned_alloc(64,128*128*2),*wv2=(uint16_t*)aligned_alloc(64,128*128*2);
    make_W_vnni(w1,wv1);make_W_vnni(w2,wv2);
    uint16_t *h0b=(uint16_t*)aligned_alloc(64,sz*2);
    convert_f32_to_bf16(h0,h0b,sz);
    float *h1=(float*)aligned_alloc(64,sz*4),*ref1=(float*)aligned_alloc(64,sz*4);
    uint16_t *h1b=(uint16_t*)aligned_alloc(64,sz*2),*ref1b=(uint16_t*)aligned_alloc(64,sz*2);
    uint16_t *h2=(uint16_t*)aligned_alloc(64,sz*2),*ref2=(uint16_t*)aligned_alloc(64,sz*2);
    one<false>(choices[0],g,h0b,wv1,ref1,perm);
    relu_f32(ref1,sz);convert_f32_to_bf16(ref1,ref1b,sz);
    one<true>(choices[0],g,ref1b,wv2,ref2,perm);
    uint64_t sum_max=0,nonempty_batches=0,windows[65]={};
    for(int rg=0;rg<N;rg+=64)for(int i=rg;i<std::min(N,rg+64);i+=16){
        int md=0;for(int n=i;n<std::min(N,i+16);n++)md=std::max(md,(int)(g.indptr[perm[n]+1]-g.indptr[perm[n]]));
        sum_max+=md;if(md)nonempty_batches++;
        for(auto &c:choices)if(c.window)windows[c.window]+=(md+c.window-1)/c.window;
    }
    printf("CONFIG graph=%s N=%d E=%u threads=%d repeats=%d seed=12345 R=64 TR=16 D=128 F=128\n",argv[2],N,g.nnz,omp_get_max_threads(),repeats);
    for(auto &c:choices){
        uint64_t windows_count=c.window?windows[c.window]:0;
        uint64_t gather=(c.window==0||c.duplicate_gather)?2ull*g.nnz:g.nnz;
        // Per layer: 4 TDP per kb, 4 kb, 2 panels, sum of batch max degrees.
        printf("WORK method=%s gathered_neighbors=%llu h_source_logical_bytes=%llu hbuf_copy_bytes=%llu h_tile_loads=%llu w_tile_loads=%llu tdp=%llu windows=%llu c_state_load_tiles=%llu c_state_store_tiles=%llu\n",
          c.name,(unsigned long long)gather,(unsigned long long)(gather*256),
          (unsigned long long)(gather*256),(unsigned long long)(sum_max*8),
          (unsigned long long)(sum_max*32),(unsigned long long)(sum_max*32),
          (unsigned long long)windows_count,
          (unsigned long long)(c.window?8*(windows_count-nonempty_batches):0),
          (unsigned long long)(c.window?8*(windows_count-nonempty_batches):0));
    }
    auto forward=[&](const Choice &c,Times *ts){
        double t0=now_ms();one<false>(c,g,h0b,wv1,h1,perm);double t1=now_ms();
        relu_f32(h1,sz);double t2=now_ms();
        convert_f32_to_bf16(h1,h1b,sz);double t3=now_ms();
        one<true>(c,g,h1b,wv2,h2,perm);double t4=now_ms();
        if(ts)*ts={t1-t0,t2-t1,t3-t2,t4-t3,t4-t0};
    };
    for(auto &c:choices){
        one<false>(c,g,h0b,wv1,h1,perm);
        one<false>(choices[0],g,h0b,wv1,ref1,perm);
        bool l1=equal_bits(h1,ref1,sz*4);
        relu_f32(h1,sz);convert_f32_to_bf16(h1,h1b,sz);
        bool inter=equal_bits(h1b,ref1b,sz*2);
        one<true>(c,g,h1b,wv2,h2,perm);
        bool l2=equal_bits(h2,ref2,sz*2);
        printf("CHECK method=%s l1_fp32_bits=%d interlayer_bf16_bits=%d l2_bf16_bits=%d pass=%d\n",c.name,l1,inter,l2,l1&&inter&&l2);fflush(stdout);
        if(!(l1&&inter&&l2))return 3;
    }
    for(auto &c:choices)forward(c,nullptr);
    int nm=sizeof(choices)/sizeof(choices[0]);
    for(int rep=0;rep<repeats;rep++)for(int pos=0;pos<nm;pos++){
        int idx=(rep%2)?nm-1-pos:pos;Times t;forward(choices[idx],&t);
        printf("TIME graph=%s method=%s rep=%d order=%d layer1_ms=%.9f relu_ms=%.9f conversion_ms=%.9f layer2_ms=%.9f total_ms=%.9f\n",
          argv[2],choices[idx].name,rep,pos,t.l1,t.relu,t.convert,t.l2,t.total);fflush(stdout);
    }
    printf("COMPLETE graph=%s checks=%d times=%d pass=1\n",argv[2],nm,nm*repeats);
    return 0;
}
