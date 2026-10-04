#pragma once
#include <array>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <type_traits>
namespace gcn_extra_paper {
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
template<bool B16> inline void scatter_output(std::conditional_t<B16,uint16_t,float>* dst,const float* src,int count) {
    if constexpr(B16) { for(int k=0;k<count;k++)dst[k]=::f32_to_bf16(src[k]); }
    else memcpy(dst,src,size_t(count)*sizeof(float));
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
template<bool P,bool B16=false> void block_kernel(const SourceGraphView& g,const uint16_t* hb,const uint16_t* w,std::conditional_t<B16,uint16_t,float>* c,const int* perm,
                                  const Method& method,Profile* result) {
    const int R=64;std::vector<Profile> profiles(omp_get_max_threads());
    double z=tick<P>(true);if(!method.nozero)memset(c,0,size_t(g.n)*DIM*sizeof(*c));
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
                    #define STORE_REPLAY(T,COL) { t=tick<P>(sample);_tile_stored(T,tmp,64);tock<P>(pr,TILE_STORE,t,sample);t=tick<P>(sample);for(int n=0;n<batch;n++)scatter_output<B16>(c+size_t(rows[n])*DIM+obp*64+COL,tmp+n*16,16);tock<P>(pr,SCATTER,t,sample); }
                    STORE_REPLAY(TC0,0);STORE_REPLAY(TC1,16);STORE_REPLAY(TC2,32);STORE_REPLAY(TC3,48);
                    #undef STORE_REPLAY
                }
            } else {
                if(!maxdeg){for(int n=0;n<batch;n++)memset(c+size_t(rows[n])*DIM,0,DIM*sizeof(*c));continue;}
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
                t=tick<P>(sample);for(int n=0;n<batch;n++)scatter_output<B16>(c+size_t(rows[n])*DIM,ctile+n*DIM,DIM);tock<P>(pr,SCATTER,t,sample);
            }
        }
        _tile_release();
    }
    if constexpr(P)for(const auto& p:profiles){result->tiles+=p.tiles;for(int s=0;s<PHASES;s++)result->sec[s]+=p.sec[s];}
}

}
