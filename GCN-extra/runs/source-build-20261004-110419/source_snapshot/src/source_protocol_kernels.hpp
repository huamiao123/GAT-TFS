// Generated from the frozen original and previously validated candidate.
// No CSR copy or new memory-placement policy is introduced.
#include <array>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
namespace gcn_extra_source {
constexpr int DIM=128;
enum Phase { ZERO, SCHEDULE, REDUCE_ZERO, CSR_PREFETCH, GATHER_DECODE, REDUCE_ADD,
             CONVERT, TILE_LOAD, TILE_COMPUTE, TILE_STORE, SCATTER, SETUP, PHASES };
const char* phase_name[]={"output_zero","row_schedule","partial_zero","csr_prefetch",
 "feature_gather_decode","fp32_neighbor_reduction","partial_bf16_conversion",
 "tile_load","tile_compute","tile_store","output_scatter","thread_amx_setup"};
struct Profile { std::array<double,PHASES> sec{}; uint64_t tiles=0; };
template<bool P> inline double tick(bool sample) { if constexpr(P) { if(sample) return omp_get_wtime(); } return 0; }
template<bool P> inline void tock(Profile& p,Phase s,double t,bool sample) { if constexpr(P) { if(sample) p.sec[s]+=omp_get_wtime()-t; } }
constexpr int PROFILE_STRIDE=256;

struct SourceArrayView {
    const uint32_t* p;
    uint32_t operator[](size_t i) const { return p[i]; }
};
struct SourceGraphView { int n; uint64_t e; SourceArrayView row,col; };
static void tfs_v3_kernel_nozero(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb, const uint16_t *Wv,
    float *C, const int *perm, int N, int R)
{
    // Only the redundant output memset is omitted.

    #pragma omp parallel
    {
        if (syscall(SYS_arch_prctl, 0x1023, 18) != 0) { perror("arch_prctl"); exit(1); }
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);

        uint16_t Hbuf[TR * K_DIM] __attribute__((aligned(64)));
        float    Ctmp[TR * 16]    __attribute__((aligned(64)));
        uint32_t base_local[TR], deg_local[TR];
        int      orig_row[TR];

        #pragma omp for schedule(dynamic, 1) nowait
        for (int rg = 0; rg < N; rg += R) {
            int rg_end = (rg+R < N) ? (rg+R) : N;
            for (int i = rg; i < rg_end; i += TR) {
                int batch = ((i+TR) <= rg_end) ? TR : (rg_end-i);
                int max_deg = 0;
                for (int n = 0; n < batch; n++) {
                    int row = perm[i+n];
                    orig_row[n] = row;
                    base_local[n] = indptr[row];
                    deg_local[n] = indptr[row+1]-indptr[row];
                    if ((int)deg_local[n] > max_deg) max_deg = (int)deg_local[n];
                }
                for (int obp = 0; obp < NP; obp++) {
                    _tile_zero(TC0); _tile_zero(TC1); _tile_zero(TC2); _tile_zero(TC3);
                    memset(Hbuf, 0, TR*K_DIM*sizeof(uint16_t));
                    int active_from = 0;
                    for (int s = 0; s < max_deg; s++) {
                        if (s+1 < max_deg) {
                            for (int n = active_from; n < batch; n++) {
                                if ((uint32_t)(s+1) < deg_local[n]) {
                                    uint32_t j_next = indices[base_local[n]+s+1];
                                    const char *addr = (const char*)&Hb[(size_t)j_next*K_DIM];
                                    _mm_prefetch(addr, _MM_HINT_T0);
                                    _mm_prefetch(addr+64, _MM_HINT_T0);
                                    _mm_prefetch(addr+128, _MM_HINT_T0);
                                    _mm_prefetch(addr+192, _MM_HINT_T0);
                                }
                            }
                        }
                        while (active_from < batch && (uint32_t)s >= deg_local[active_from]) {
                            memset(&Hbuf[active_from*K_DIM], 0, K_DIM*sizeof(uint16_t));
                            active_from++;
                        }
                        if (active_from >= batch) break;
                        for (int n = active_from; n < batch; n++) {
                            uint32_t j = indices[base_local[n]+s];
                            memcpy(&Hbuf[n*K_DIM], &Hb[(size_t)j*K_DIM], K_DIM*sizeof(uint16_t));
                        }
                        for (int kb = 0; kb < KB; kb++) {
                            _tile_loadd(TA, (const uint8_t*)Hbuf+kb*64, K_DIM*2);
                            int ob0 = obp*4;
                            const uint16_t *Wb = Wv;
                            #define WV_OFF(kb_, ob_) (((kb_)*NB+(ob_))*16*32)
                            _tile_loadd(TB0, &Wb[WV_OFF(kb,ob0+0)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb,ob0+1)], 64);
                            _tile_dpbf16ps(TC0, TA, TB0);
                            _tile_dpbf16ps(TC1, TA, TB1);
                            _tile_loadd(TB0, &Wb[WV_OFF(kb,ob0+2)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb,ob0+3)], 64);
                            _tile_dpbf16ps(TC2, TA, TB0);
                            _tile_dpbf16ps(TC3, TA, TB1);
                            #undef WV_OFF
                        }
                    }
                    int col = obp*64;
                    _tile_stored(TC0, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+0], &Ctmp[n*16], 64);
                    _tile_stored(TC1, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+16], &Ctmp[n*16], 64);
                    _tile_stored(TC2, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+32], &Ctmp[n*16], 64);
                    _tile_stored(TC3, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+48], &Ctmp[n*16], 64);
                }
            }
        }
        _tile_release();
    }
}
template<bool P> inline void project_panel(const uint16_t* hb,const uint16_t* w,int obp,Profile& pr,bool sample) {
    for(int kb=0;kb<KB;kb++) {
        double t=tick<P>(sample);
        _tile_loadd(TA,(const uint8_t*)hb+kb*64,DIM*2);
        int ob0=obp*4;
        _tile_loadd(TB0,w+((kb*NB+ob0)*16*32),64);
        _tile_loadd(TB1,w+((kb*NB+ob0+1)*16*32),64);
        tock<P>(pr,TILE_LOAD,t,sample);t=tick<P>(sample);
        _tile_dpbf16ps(TC0,TA,TB0);_tile_dpbf16ps(TC1,TA,TB1);
        tock<P>(pr,TILE_COMPUTE,t,sample);t=tick<P>(sample);
        _tile_loadd(TB0,w+((kb*NB+ob0+2)*16*32),64);
        _tile_loadd(TB1,w+((kb*NB+ob0+3)*16*32),64);
        tock<P>(pr,TILE_LOAD,t,sample);t=tick<P>(sample);
        _tile_dpbf16ps(TC2,TA,TB0);_tile_dpbf16ps(TC3,TA,TB1);
        tock<P>(pr,TILE_COMPUTE,t,sample);
    }
}
template<bool P> void reduce_tile(const SourceGraphView& g,const uint16_t* hb,const uint32_t* bases,const uint32_t* degrees,
                int batch,uint32_t start,uint32_t count,float* partial,uint16_t* hi,uint16_t* lo,
                bool accurate,Profile& pr,bool sample) {
    double t=tick<P>(sample);memset(partial,0,TR*DIM*sizeof(float));tock<P>(pr,REDUCE_ZERO,t,sample);
    for(int n=0;n<batch;n++) {
        uint32_t end=std::min(degrees[n],start+count);
        __m512 acc[8];for(auto& v:acc)v=_mm512_setzero_ps();
        for(uint32_t k=start;k<end;k++) {
            t=tick<P>(sample);uint32_t j=g.col[bases[n]+k];
            if(k+1<end) { const char* next=(const char*)(hb+size_t(g.col[bases[n]+k+1])*DIM);for(int o=0;o<256;o+=64)_mm_prefetch(next+o,_MM_HINT_T0); }
            tock<P>(pr,CSR_PREFETCH,t,sample);t=tick<P>(sample);
            __m512 val[8];const uint16_t* src=hb+size_t(j)*DIM;
            for(int f=0;f<8;f++)val[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(src+f*16))),16));
            tock<P>(pr,GATHER_DECODE,t,sample);t=tick<P>(sample);
            for(int f=0;f<8;f++)acc[f]=_mm512_add_ps(acc[f],val[f]);
            tock<P>(pr,REDUCE_ADD,t,sample);
        }
        t=tick<P>(sample);for(int f=0;f<8;f++)_mm512_store_ps(partial+n*DIM+f*16,acc[f]);tock<P>(pr,REDUCE_ADD,t,sample);
    }
    t=tick<P>(sample);
    for(int v=0;v<TR*DIM;v+=16) {
        __m512 a=_mm512_load_ps(partial+v);__m512i bits=_mm512_castps_si512(a);
        __m256i top=_mm512_cvtepi32_epi16(_mm512_srli_epi32(bits,16));_mm256_store_si256((__m256i*)(hi+v),top);
        if(accurate) {
            __m512 quant=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(top),16));
            __m512 residual=_mm512_sub_ps(a,quant);
            _mm256_store_si256((__m256i*)(lo+v),_mm512_cvtepi32_epi16(_mm512_srli_epi32(_mm512_castps_si512(residual),16)));
        }
    }
    tock<P>(pr,CONVERT,t,sample);
}
struct Method { std::string name;int block=1;bool accurate=false,replay=false;int mkl=0;bool nozero=false; };
bool is_original(const Method& m){return m.name=="original" || m.name=="original_nozero";}
template<bool P> void block_kernel(const SourceGraphView& g,const uint16_t* hb,const uint16_t* w,float* c,const int* perm,
                                  const Method& method,Profile* result) {
    const int R=64;std::vector<Profile> profiles(omp_get_max_threads());
    double z=tick<P>(true);if(!method.nozero)memset(c,0,size_t(g.n)*DIM*sizeof(float));
    if constexpr(P) profiles[0].sec[ZERO]=omp_get_wtime()-z;
    #pragma omp parallel
    {
        Profile& pr=profiles[omp_get_thread_num()];double t=tick<P>(true);
        if(syscall(SYS_arch_prctl,0x1023,18)!=0){perror("AMX permission");abort();}
        tilecfg_t cfg;setup_tilecfg(&cfg);_tile_loadconfig(&cfg);tock<P>(pr,SETUP,t,true);
        alignas(64) float partial[TR*DIM],ctile[TR*DIM],tmp[TR*16];
        alignas(64) uint16_t hi[TR*DIM],lo[TR*DIM];
        uint32_t bases[TR],degrees[TR];int rows[TR];
        #pragma omp for schedule(dynamic,1) nowait
        for(int rg=0;rg<g.n;rg+=R)for(int i=rg;i<std::min(rg+R,g.n);i+=TR) {
            bool sample=P && (((i/TR)%PROFILE_STRIDE)==0 || i+TR>=g.n);
            if(sample)pr.tiles++;
            t=tick<P>(sample);int batch=std::min(TR,std::min(rg+R,g.n)-i);uint32_t maxdeg=0;
            for(int n=0;n<batch;n++){rows[n]=perm[i+n];bases[n]=g.row[rows[n]];degrees[n]=g.row[rows[n]+1]-bases[n];maxdeg=std::max(maxdeg,degrees[n]);}
            uint32_t step=method.block==0?std::max(1u,maxdeg):(uint32_t)method.block;
            tock<P>(pr,SCHEDULE,t,sample);
            if(method.replay) {
                for(int obp=0;obp<NP;obp++) {
                    t=tick<P>(sample);_tile_zero(TC0);_tile_zero(TC1);_tile_zero(TC2);_tile_zero(TC3);tock<P>(pr,REDUCE_ZERO,t,sample);
                    for(uint32_t start=0;start<maxdeg;start+=step) {
                        reduce_tile<P>(g,hb,bases,degrees,batch,start,step,partial,hi,lo,method.accurate,pr,sample);
                        project_panel<P>(hi,w,obp,pr,sample);if(method.accurate)project_panel<P>(lo,w,obp,pr,sample);
                    }
                    #define STORE_REPLAY(T,COL) { t=tick<P>(sample);_tile_stored(T,tmp,64);tock<P>(pr,TILE_STORE,t,sample);t=tick<P>(sample);for(int n=0;n<batch;n++)memcpy(c+size_t(rows[n])*DIM+obp*64+COL,tmp+n*16,64);tock<P>(pr,SCATTER,t,sample); }
                    STORE_REPLAY(TC0,0);STORE_REPLAY(TC1,16);STORE_REPLAY(TC2,32);STORE_REPLAY(TC3,48);
                    #undef STORE_REPLAY
                }
            } else {
                if(!maxdeg){for(int n=0;n<batch;n++)memset(c+size_t(rows[n])*DIM,0,DIM*sizeof(float));continue;}
                for(uint32_t start=0;start<maxdeg;start+=step) {
                    reduce_tile<P>(g,hb,bases,degrees,batch,start,step,partial,hi,lo,method.accurate,pr,sample);
                    for(int obp=0;obp<NP;obp++) {
                        t=tick<P>(sample);
                        if(start==0){_tile_zero(TC0);_tile_zero(TC1);_tile_zero(TC2);_tile_zero(TC3);}
                        else{_tile_loadd(TC0,ctile+obp*64,512);_tile_loadd(TC1,ctile+obp*64+16,512);_tile_loadd(TC2,ctile+obp*64+32,512);_tile_loadd(TC3,ctile+obp*64+48,512);}
                        tock<P>(pr,start==0?REDUCE_ZERO:TILE_LOAD,t,sample);
                        project_panel<P>(hi,w,obp,pr,sample);if(method.accurate)project_panel<P>(lo,w,obp,pr,sample);
                        t=tick<P>(sample);
                        _tile_stored(TC0,ctile+obp*64,512);_tile_stored(TC1,ctile+obp*64+16,512);_tile_stored(TC2,ctile+obp*64+32,512);_tile_stored(TC3,ctile+obp*64+48,512);
                        tock<P>(pr,TILE_STORE,t,sample);
                    }
                }
                t=tick<P>(sample);for(int n=0;n<batch;n++)memcpy(c+size_t(rows[n])*DIM,ctile+n*DIM,DIM*sizeof(float));tock<P>(pr,SCATTER,t,sample);
            }
        }
        _tile_release();
    }
    if constexpr(P)for(const auto& p:profiles){result->tiles+=p.tiles;for(int s=0;s<PHASES;s++)result->sec[s]+=p.sec[s];}
}

}
