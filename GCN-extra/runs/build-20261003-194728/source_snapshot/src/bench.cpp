#define main frozen_original_cli_main
#include "../original/amx_tfs_v3.cpp"
#undef main
#include <mkl.h>
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <sys/resource.h>

template<class T> struct AlignedAllocator {
    using value_type=T;
    AlignedAllocator()=default;
    template<class U> AlignedAllocator(const AlignedAllocator<U>&) {}
    T* allocate(size_t n) { void* p=nullptr; if(posix_memalign(&p,64,std::max(size_t(64),n*sizeof(T)))) throw std::bad_alloc(); return (T*)p; }
    void deallocate(T* p,size_t) { free(p); }
};
template<class T,class U> bool operator==(const AlignedAllocator<T>&,const AlignedAllocator<U>&){return true;}
template<class T,class U> bool operator!=(const AlignedAllocator<T>&,const AlignedAllocator<U>&){return false;}
template<class T> using AV=std::vector<T,AlignedAllocator<T>>;

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

struct Graph {
    int n=0; uint64_t e=0, self_loops=0; std::string name;
    std::vector<uint32_t> row,col; double load_ms=0, values_scan_ms=0;
};
uint64_t mix64(uint64_t x) { x+=0x9e3779b97f4a7c15ULL; x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL; x=(x^(x>>27))*0x94d049bb133111ebULL; return x^(x>>31); }
uint64_t hash_bytes(const void* p,size_t n,uint64_t h=1469598103934665603ULL) {
    const uint8_t* a=(const uint8_t*)p; for(size_t i=0;i<n;i++){h^=a[i];h*=1099511628211ULL;} return h;
}
Graph load_graph(const std::string& path,const std::string& name) {
    Graph g; g.name=name; double t=omp_get_wtime(); FILE* f=fopen(path.c_str(),"rb");
    if(!f) throw std::runtime_error("cannot open graph: "+path);
    uint32_t fmt[3]; uint64_t nr,nc,nz;
    if(fread(fmt,4,3,f)!=3 || fread(&nr,8,1,f)!=1 || fread(&nc,8,1,f)!=1 || fread(&nz,8,1,f)!=1) throw std::runtime_error("bad CSR header");
    if(fmt[0]!=0 || fmt[1]!=0 || nr!=nc || nr>INT32_MAX || nz>UINT32_MAX) throw std::runtime_error("unsupported CSR layout/dimensions");
    g.n=(int)nr;g.e=nz;g.row.resize(nr+1);g.col.resize(nz);
    if(fread(g.row.data(),4,nr+1,f)!=nr+1 || fread(g.col.data(),4,nz,f)!=nz) throw std::runtime_error("truncated CSR");
    if(g.row[0]!=0 || g.row[nr]!=nz) throw std::runtime_error("invalid CSR endpoints");
    for(int i=0;i<g.n;i++) {
        if(g.row[i]>g.row[i+1]) throw std::runtime_error("nonmonotone CSR");
        for(uint32_t a=g.row[i];a<g.row[i+1];a++) { if(g.col[a]>=nr) throw std::runtime_error("source out of bounds"); if(g.col[a]==(uint32_t)i) g.self_loops++; }
    }
    g.load_ms=(omp_get_wtime()-t)*1000; t=omp_get_wtime();
    std::vector<float> chunk(1<<20); uint64_t read=0,bad=0; bool absent=false;
    while(read<nz) {
        size_t wanted=(size_t)std::min<uint64_t>(chunk.size(),nz-read), got=fread(chunk.data(),4,wanted,f);
        if(got==0 && read==0){absent=true;break;}
        if(got!=wanted) throw std::runtime_error("partial CSR values array");
        for(size_t k=0;k<got;k++) if(chunk[k]!=1.0f) bad++;
        read+=got;
    }
    fclose(f);g.values_scan_ms=(omp_get_wtime()-t)*1000;
    printf("GRAPH_VALUES name=%s checked=%llu nonunit=%llu implicit_ones=%d\n",name.c_str(),(unsigned long long)read,(unsigned long long)bad,absent);
    if(bad) throw std::runtime_error("nonunit CSR values: reject unweighted Original TFS comparison");
    return g;
}
Graph synthetic(int n,int degree,uint64_t seed,bool smoke) {
    if(n<=0 || degree<0 || degree>n || uint64_t(n)*degree>UINT32_MAX) throw std::runtime_error("invalid synthetic dimensions");
    Graph g;g.n=n;g.name=smoke?"smoke":"regular-n"+std::to_string(n)+"-q"+std::to_string(degree);g.row.resize(n+1);
    const int ds[]={0,1,2,7,8,9,16,31,32,33,37};
    for(int i=0;i<n;i++)g.row[i+1]=g.row[i]+(smoke?std::min(n,ds[i%11]):degree);
    g.e=g.row[n];g.col.resize(g.e);
    for(int i=0;i<n;i++) {
        uint64_t start=mix64(seed+i)%n, stride=(mix64(seed+uint64_t(i)*17+99)%n)|1;
        while(std::gcd(stride,(uint64_t)n)!=1) stride+=2;
        for(uint32_t k=g.row[i];k<g.row[i+1];k++) {
            uint32_t j=(uint32_t)((start+uint64_t(k-g.row[i])*stride)%n);g.col[k]=j;if(j==(uint32_t)i)g.self_loops++;
        }
    }
    return g;
}

struct Params { AV<float> w,wq; AV<uint16_t> wv; double pack_ms=0; };
Params params() {
    Params p;p.w.resize(DIM*DIM);p.wq.resize(DIM*DIM);p.wv.resize(KB*NB*16*32);
    for(float& x:p.w)x=0.01f*((rand()%200)-100);
    double t=omp_get_wtime();make_W_vnni(p.w.data(),p.wv.data());
    for(size_t i=0;i<p.w.size();i++)p.wq[i]=bf16_to_f32(f32_to_bf16(p.w[i]));
    p.pack_ms=(omp_get_wtime()-t)*1000;return p;
}
struct Input { AV<float> h,hq; AV<uint16_t> hb; double generate_ms=0,convert_ms=0; };
Input input(const Graph& g,bool smoke) {
    Input x;size_t z=size_t(g.n)*DIM;x.h.resize(z);x.hq.resize(z);x.hb.resize(z);double t=omp_get_wtime();
    srand(12345);for(float& v:x.h)v=0.01f*((rand()%200)-100);
    if(smoke) for(int i=0;i<g.n;i++)for(int k=0;k<DIM;k++) x.h[size_t(i)*DIM+k]= (i%7==0?0.0f: (k%3==0?((i&1)?-1:1)*0.03125f: x.h[size_t(i)*DIM+k]));
    x.generate_ms=(omp_get_wtime()-t)*1000;t=omp_get_wtime();
    convert_H_bf16(x.h.data(),x.hb.data(),g.n);
    #pragma omp parallel for schedule(static)
    for(size_t i=0;i<z;i++)x.hq[i]=bf16_to_f32(x.hb[i]);
    x.convert_ms=(omp_get_wtime()-t)*1000;return x;
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
template<bool P> void reduce_tile(const Graph& g,const uint16_t* hb,const uint32_t* bases,const uint32_t* degrees,
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
struct Method { std::string name;int block=1;bool accurate=false,replay=false;int mkl=0; };
template<bool P> void block_kernel(const Graph& g,const uint16_t* hb,const uint16_t* w,float* c,const int* perm,
                                  const Method& method,Profile* result) {
    const int R=64;std::vector<Profile> profiles(omp_get_max_threads());
    double z=tick<P>(true);memset(c,0,size_t(g.n)*DIM*sizeof(float));
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
                if(!maxdeg)continue;
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

// Instrumented copy of the original hot loop. The unmodified original remains
// the performance anchor; output equality is checked before accepting profiles.
void original_profile(const Graph& g,const uint16_t* hb,const uint16_t* w,float* c,const int* perm,Profile& result) {
    std::vector<Profile> profiles(omp_get_max_threads());double t=omp_get_wtime();memset(c,0,size_t(g.n)*DIM*4);profiles[0].sec[ZERO]=omp_get_wtime()-t;
    #pragma omp parallel
    {
        Profile& pr=profiles[omp_get_thread_num()];double t=omp_get_wtime();
        if(syscall(SYS_arch_prctl,0x1023,18)!=0)abort();tilecfg_t cfg;setup_tilecfg(&cfg);_tile_loadconfig(&cfg);pr.sec[SETUP]+=omp_get_wtime()-t;
        alignas(64) uint16_t hbuf[TR*DIM];alignas(64) float tmp[TR*16];uint32_t bases[TR],degrees[TR];int rows[TR];
        #pragma omp for schedule(dynamic,1) nowait
        for(int rg=0;rg<g.n;rg+=64)for(int i=rg;i<std::min(rg+64,g.n);i+=TR) {
            bool sample=((i/TR)%PROFILE_STRIDE)==0 || i+TR>=g.n;if(sample)pr.tiles++;
            t=tick<true>(sample);int batch=std::min(TR,std::min(rg+64,g.n)-i),maxdeg=0;
            for(int n=0;n<batch;n++){rows[n]=perm[i+n];bases[n]=g.row[rows[n]];degrees[n]=g.row[rows[n]+1]-bases[n];maxdeg=std::max(maxdeg,(int)degrees[n]);}tock<true>(pr,SCHEDULE,t,sample);
            for(int obp=0;obp<NP;obp++) {
                t=tick<true>(sample);_tile_zero(TC0);_tile_zero(TC1);_tile_zero(TC2);_tile_zero(TC3);memset(hbuf,0,sizeof(hbuf));int active=0;tock<true>(pr,REDUCE_ZERO,t,sample);
                for(int s=0;s<maxdeg;s++) {
                    t=tick<true>(sample);
                    if(s+1<maxdeg)for(int n=active;n<batch;n++)if((uint32_t)(s+1)<degrees[n]){const char* next=(const char*)(hb+size_t(g.col[bases[n]+s+1])*DIM);for(int o=0;o<256;o+=64)_mm_prefetch(next+o,_MM_HINT_T0);}
                    while(active<batch && (uint32_t)s>=degrees[active]){memset(hbuf+active*DIM,0,DIM*2);active++;}
                    tock<true>(pr,CSR_PREFETCH,t,sample);if(active>=batch)break;
                    t=tick<true>(sample);for(int n=active;n<batch;n++)memcpy(hbuf+n*DIM,hb+size_t(g.col[bases[n]+s])*DIM,DIM*2);tock<true>(pr,GATHER_DECODE,t,sample);
                    project_panel<true>(hbuf,w,obp,pr,sample);
                }
                #define STORE_ORIG(T,COL) { t=tick<true>(sample);_tile_stored(T,tmp,64);tock<true>(pr,TILE_STORE,t,sample);t=tick<true>(sample);for(int n=0;n<batch;n++)memcpy(c+size_t(rows[n])*DIM+obp*64+COL,tmp+n*16,64);tock<true>(pr,SCATTER,t,sample); }
                STORE_ORIG(TC0,0);STORE_ORIG(TC1,16);STORE_ORIG(TC2,32);STORE_ORIG(TC3,48);
                #undef STORE_ORIG
            }
        }
        _tile_release();
    }
    for(const auto& p:profiles){result.tiles+=p.tiles;for(int s=0;s<PHASES;s++)result.sec[s]+=p.sec[s];}
}

struct MKLGraph {
    sparse_matrix_t a=nullptr;matrix_descr descr{};std::vector<MKL_INT> rs,re,ci;std::vector<float> values;
    double setup_ms=0;
    MKLGraph(const Graph& g) {
        double t=omp_get_wtime();rs.resize(g.n);re.resize(g.n);ci.resize(g.e);values.assign(g.e,1.0f);
        for(int i=0;i<g.n;i++){rs[i]=g.row[i];re[i]=g.row[i+1];}for(size_t i=0;i<g.e;i++)ci[i]=g.col[i];
        descr.type=SPARSE_MATRIX_TYPE_GENERAL;
        // oneMKL rejects a CSR with zero stored entries. Its mathematical SpMM
        // is zero; retain the ordinary dense GeMM after explicitly zeroing Z.
        if(g.e==0){setup_ms=(omp_get_wtime()-t)*1000;return;}
        if(mkl_sparse_s_create_csr(&a,SPARSE_INDEX_BASE_ZERO,g.n,g.n,rs.data(),re.data(),ci.data(),values.data())!=SPARSE_STATUS_SUCCESS)throw std::runtime_error("MKL create");
        mkl_sparse_set_mm_hint(a,SPARSE_OPERATION_NON_TRANSPOSE,descr,SPARSE_LAYOUT_ROW_MAJOR,DIM,16);mkl_sparse_optimize(a);setup_ms=(omp_get_wtime()-t)*1000;
    }
    ~MKLGraph(){if(a)mkl_sparse_destroy(a);}
    void run(const Graph& g,const float* h,const float* w,float* z,float* c,double* spmm=nullptr,double* gemm=nullptr) {
        double t=omp_get_wtime();sparse_status_t status=SPARSE_STATUS_SUCCESS;
        if(g.e==0)memset(z,0,size_t(g.n)*DIM*sizeof(float));
        else status=mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1,a,descr,SPARSE_LAYOUT_ROW_MAJOR,h,DIM,DIM,0,z,DIM);
        if(status!=SPARSE_STATUS_SUCCESS)throw std::runtime_error("MKL SpMM");double u=omp_get_wtime();
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,g.n,DIM,DIM,1,z,DIM,w,DIM,0,c,DIM);double v=omp_get_wtime();
        if(spmm)*spmm=(u-t)*1000;if(gemm)*gemm=(v-u)*1000;
    }
};
struct Error { double maxabs=0,meanabs=0,l2=0,maxnorm=0;bool finite=true; };
Error error(const float* a,const float* b,size_t n) {
    double sum=0,sq=0,ref=0,mx=0,maxabs=0;int bad=0;
    #pragma omp parallel for reduction(+:sum,sq,ref,bad) reduction(max:mx,maxabs) schedule(static)
    for(size_t k=0;k<n;k++) {
        if(!std::isfinite(a[k]) || !std::isfinite(b[k])){bad++;continue;}
        double d=double(a[k])-b[k];sum+=fabs(d);sq+=d*d;ref+=double(b[k])*b[k];mx=std::max(mx,fabs(double(b[k])));maxabs=std::max(maxabs,fabs(d));
    }
    return {maxabs,sum/std::max(size_t(1),n),sqrt(sq/std::max(ref,1e-300)),maxabs/std::max(mx,1e-30),bad==0};
}
struct SampleRef { std::vector<int> rows;std::vector<double> values; };
SampleRef fp64_sample(const Graph& g,const Input& x,const Params& p,const int* perm) {
    std::set<int> selected;for(int k=0;k<std::min(32,g.n);k++){selected.insert(perm[k]);selected.insert(perm[g.n-1-k]);}
    for(int k=0;k<32;k++)selected.insert((int)(mix64(k+12345)%g.n));
    SampleRef s;s.rows.assign(selected.begin(),selected.end());s.values.resize(s.rows.size()*DIM);
    #pragma omp parallel for schedule(dynamic,1)
    for(size_t r=0;r<s.rows.size();r++) {
        double a[DIM]={};int i=s.rows[r];for(uint32_t e=g.row[i];e<g.row[i+1];e++)for(int k=0;k<DIM;k++)a[k]+=x.hq[size_t(g.col[e])*DIM+k];
        for(int f=0;f<DIM;f++){double v=0;for(int k=0;k<DIM;k++)v+=a[k]*p.wq[k*DIM+f];s.values[r*DIM+f]=v;}
    }
    return s;
}
Error sample_error(const float* c,const SampleRef& s) {
    double sum=0,sq=0,ref=0,mx=0,ma=0;bool finite=true;
    for(size_t r=0;r<s.rows.size();r++)for(int k=0;k<DIM;k++){double a=c[size_t(s.rows[r])*DIM+k],b=s.values[r*DIM+k],d=a-b;if(!std::isfinite(a)){finite=false;continue;}sum+=fabs(d);sq+=d*d;ref+=b*b;mx=std::max(mx,fabs(b));ma=std::max(ma,fabs(d));}
    return{ma,sum/std::max(size_t(1),s.values.size()),sqrt(sq/std::max(ref,1e-300)),ma/std::max(mx,1e-30),finite};
}
void run_method(const Graph& g,const Input& x,const Params& p,const int* perm,const Method& m,MKLGraph& mg,float* scratch,float* c) {
    if(m.mkl)mg.run(g,m.mkl==1?x.h.data():x.hq.data(),m.mkl==1?p.w.data():p.wq.data(),scratch,c);
    else if(m.name=="original")tfs_v3(g.row.data(),g.col.data(),x.hb.data(),p.wv.data(),c,perm,g.n,64);
    else block_kernel<false>(g,x.hb.data(),p.wv.data(),c,perm,m,nullptr);
}
void write_error(std::ofstream& f,const Graph& g,const Method& m,const char* boundary,const char* ref,const Error& e,size_t n,bool pass) {
    f<<g.name<<','<<m.name<<','<<boundary<<','<<ref<<','<<e.maxabs<<','<<e.meanabs<<','<<e.l2<<','<<e.maxnorm<<','<<e.finite<<','<<pass<<','<<n<<'\n';
    printf("CORRECT graph=%s method=%s boundary=%s reference=%s rel_l2=%.9g max_abs=%.9g max_norm=%.9g pass=%d\n",g.name.c_str(),m.name.c_str(),boundary,ref,e.l2,e.maxabs,e.maxnorm,pass);fflush(stdout);
}
void work(std::ofstream& f,const Graph& g,const Method& m,const int* perm) {
    uint64_t useful=0,physical=0,tile_steps=0,groups=0;int b=m.block;uint64_t maxdeg=0;
    for(int i=0;i<g.n;i++){uint64_t d=g.row[i+1]-g.row[i];maxdeg=std::max(maxdeg,d);useful+=b?((d+b-1)/b):(d?1:0);}
    for(int i=0;i<g.n;i+=TR){uint64_t mx=0;for(int n=i;n<std::min(i+TR,g.n);n++){uint64_t d=g.row[perm[n]+1]-g.row[perm[n]];mx=std::max(mx,b?((d+b-1)/b):(d?1:0));}physical+=TR*mx;tile_steps+=mx;if(mx)groups++;}
    int passes=m.replay||m.name=="original"?NP:1;double amx=m.mkl?0:2.0*physical*DIM*DIM*(m.accurate?2:1);
    uint64_t spill=m.mkl?0:(m.replay||m.name=="original"?((g.n+TR-1)/TR)*TR*DIM*4:tile_steps*TR*DIM*4);
    uint64_t reload=m.mkl||m.replay||m.name=="original"?0:(tile_steps-groups)*TR*DIM*4;
    double adds=m.name=="original"?0:double(g.e)*DIM*passes;
    f<<g.name<<','<<m.name<<','<<b<<','<<m.replay<<','<<m.accurate<<','<<g.n<<','<<g.e<<','<<double(g.e)/g.n<<','<<maxdeg<<','<<double(maxdeg)*g.n/std::max(uint64_t(1),g.e)<<','<<useful<<','<<physical<<','<<(physical?double(useful)/physical:1)<<','<<amx<<','<<g.e*DIM*(m.mkl?4:2)*passes<<','<<passes<<','<<spill<<','<<reload<<','<<adds<<'\n';
}
struct E2ETimes { double total=0,l1=0,act=0,l2=0; };
E2ETimes e2e(const Graph& g,const Input& x,const Params& p1,const Params& p2,const int* perm,const Method& m,MKLGraph& mg,
             AV<float>& intermediate,AV<float>& quant,AV<uint16_t>& hb,AV<float>& scratch,AV<float>& c) {
    double t=omp_get_wtime();run_method(g,x,p1,perm,m,mg,scratch.data(),intermediate.data());double t1=omp_get_wtime();
    size_t z=c.size();
    #pragma omp parallel for schedule(static)
    for(size_t k=0;k<z;k++) {
        float a=std::max(0.0f,intermediate[k]);intermediate[k]=a;
        if(m.mkl!=1){uint16_t b=f32_to_bf16(a);hb[k]=b;if(m.mkl==2)quant[k]=bf16_to_f32(b);}
    }
    double t2=omp_get_wtime();
    if(m.mkl)mg.run(g,m.mkl==1?intermediate.data():quant.data(),m.mkl==1?p2.w.data():p2.wq.data(),scratch.data(),c.data());
    else if(m.name=="original")tfs_v3(g.row.data(),g.col.data(),hb.data(),p2.wv.data(),c.data(),perm,g.n,64);
    else block_kernel<false>(g,hb.data(),p2.wv.data(),c.data(),perm,m,nullptr);
    double t3=omp_get_wtime();return{(t3-t)*1000,(t1-t)*1000,(t2-t1)*1000,(t3-t2)*1000};
}
int main(int argc,char** argv) {
    try {
        std::string path,name,out=".",blocks="2,4,8,16,32,64,full",only="";int n=0,q=0,repeats=5,warmups=1;bool smoke=false,skip_e2e=false,replay=true;uint64_t seed=20261003;
        for(int a=1;a<argc;a++) {
            std::string opt=argv[a];auto val=[&](){if(a+1>=argc)throw std::runtime_error("missing argument");return std::string(argv[++a]);};
            if(opt=="--graph")path=val();else if(opt=="--name")name=val();else if(opt=="--out")out=val();else if(opt=="--blocks")blocks=val();
            else if(opt=="--synthetic")n=stoi(val());else if(opt=="--degree")q=stoi(val());else if(opt=="--repeats")repeats=stoi(val());else if(opt=="--warmups")warmups=stoi(val());
            else if(opt=="--seed")seed=stoull(val());else if(opt=="--smoke")smoke=true;else if(opt=="--skip-e2e")skip_e2e=true;else if(opt=="--no-replay")replay=false;else if(opt=="--only")only=val();else throw std::runtime_error("unknown argument: "+opt);
        }
        if(repeats<1 || warmups<0)throw std::runtime_error("invalid repeats");
        mkl_set_dynamic(0);mkl_set_num_threads(omp_get_max_threads());
        Graph g=smoke?synthetic(37,0,seed,true):(!path.empty()?load_graph(path,name):synthetic(n,q,seed,false));
        std::ofstream pre(out+"/preprocess.csv"),tim(out+"/timings.csv"),cor(out+"/correctness.csv"),wrk(out+"/work.csv"),pro(out+"/profiles.csv");
        for(auto* f:{&pre,&tim,&cor,&wrk,&pro}){if(!*f)throw std::runtime_error("output directory missing");f->precision(12);}
        pre<<"graph,phase,ms\n";tim<<"graph,method,kind,repeat,ms,layer1_ms,activation_conversion_ms,layer2_ms\n";
        cor<<"graph,method,boundary,reference,max_abs,mean_abs,relative_l2,normalized_max,finite,pass,numel\n";
        wrk<<"graph,method,block,replay,accurate,N,E,avg_degree,max_degree,degree_ratio,useful_block_rows,physical_block_rows,eta,modeled_amx_flops,logical_feature_bytes,gather_passes,local_output_store_bytes,local_output_reload_bytes,fp32_reduce_adds\n";
        pro<<"graph,method,phase,sampled_thread_ms,sampled_tiles,profile_wall_ms\n";
        pre<<g.name<<",csr_load_validate,"<<g.load_ms<<'\n'<<g.name<<",full_values_scan,"<<g.values_scan_ms<<'\n';
        double t=omp_get_wtime();std::unique_ptr<int,decltype(&free)> perm(make_degree_perm(g.row.data(),g.n),&free);pre<<g.name<<",degree_sort,"<<(omp_get_wtime()-t)*1000<<'\n';
        Input x=input(g,smoke);Params p1=params(),p2=params();MKLGraph mg(g);
        pre<<g.name<<",input_generate,"<<x.generate_ms<<'\n'<<g.name<<",input_bf16_convert_and_reference_decode,"<<x.convert_ms<<'\n'<<g.name<<",weight1_pack,"<<p1.pack_ms<<'\n'<<g.name<<",weight2_pack,"<<p2.pack_ms<<'\n'<<g.name<<",mkl_graph_setup,"<<mg.setup_ms<<'\n';
        size_t z=size_t(g.n)*DIM;AV<float> scratch(z),c(z),matched(z),fp32(z),orig(z);
        mg.run(g,x.hq.data(),p1.wq.data(),scratch.data(),matched.data());mg.run(g,x.h.data(),p1.w.data(),scratch.data(),fp32.data());
        SampleRef s=fp64_sample(g,x,p1,perm.get());Method qm{"mkl_bf16_inputs",1,false,false,2};Error qe=sample_error(matched.data(),s);
        bool qpass=qe.finite && qe.l2<1e-3 && qe.maxnorm<1e-3;write_error(cor,g,qm,"kernel_sample","fp64_bf16_inputs",qe,s.values.size(),qpass);if(!qpass)throw std::runtime_error("reference validation failed");
        std::vector<Method> methods={{"mkl_fp32",1,false,false,1},qm,{"original",1,false,true,0}};
        for(size_t pos=0;pos<blocks.size();) {
            size_t end=blocks.find(',',pos);std::string tok=blocks.substr(pos,end==std::string::npos?end:end-pos);int b=tok=="full"?0:stoi(tok);
            if(b<0 || b>1048576)throw std::runtime_error("bad block size");std::string bn=b?std::to_string(b):"full";
            for(bool accurate:{false,true})methods.push_back({"shared_b"+bn+(accurate?"_accurate":"_fast"),b,accurate,false,0});
            if(replay && (b==8 || b==32 || b==0))methods.push_back({"replay_b"+bn+"_fast",b,false,true,0});
            if(end==std::string::npos)break;pos=end+1;
        }
        if(!only.empty()){methods.erase(std::remove_if(methods.begin(),methods.end(),[&](const Method& m){return m.name!=only;}),methods.end());if(methods.empty())throw std::runtime_error("unknown method selection");}
        printf("CONFIG graph=%s N=%d E=%llu threads=%d R=64 D=128 F=128 repeats=%d warmups=%d\n",g.name.c_str(),g.n,(unsigned long long)g.e,omp_get_max_threads(),repeats,warmups);fflush(stdout);
        bool allpass=true;
        for(const Method& m:methods) {
            run_method(g,x,p1,perm.get(),m,mg,scratch.data(),c.data());
            Error e=error(c.data(),matched.data(),z);double gate=m.mkl==1?0.03:((m.name=="original" || m.accurate || m.mkl)?1e-3:1e-2);
            bool pass=e.finite && e.l2<gate && e.maxnorm<gate;write_error(cor,g,m,"kernel_full","mkl_bf16_inputs",e,z,pass);allpass&=pass;
            Error ef=error(c.data(),fp32.data(),z);write_error(cor,g,m,"kernel_full","mkl_fp32",ef,z,ef.finite);
            Error es=sample_error(c.data(),s);write_error(cor,g,m,"kernel_sample","fp64_bf16_inputs",es,s.values.size(),es.finite && es.l2<gate && es.maxnorm<gate);allpass&=es.finite && es.l2<gate && es.maxnorm<gate;
            work(wrk,g,m,perm.get());if(m.name=="original")orig=c;
            if(m.block==1 && !m.mkl && m.name!="original" && !m.accurate && only.empty()) {
                Error identity=error(c.data(),orig.data(),z);bool same=identity.finite && identity.maxabs==0;
                write_error(cor,g,m,"b1_identity","unchanged_original",identity,z,same);allpass&=same;
            }
        }
        cor.flush();if(!allpass)throw std::runtime_error("kernel correctness gate failed; performance skipped");
        // Rotating order makes the same methods occupy different positions.
        for(int r=-warmups;r<repeats;r++)for(size_t k=0;k<methods.size();k++) {
            const Method& m=methods[(k+size_t(r+warmups))%methods.size()];double sp=0,gm=0;t=omp_get_wtime();
            if(m.mkl)mg.run(g,m.mkl==1?x.h.data():x.hq.data(),m.mkl==1?p1.w.data():p1.wq.data(),scratch.data(),c.data(),&sp,&gm);
            else run_method(g,x,p1,perm.get(),m,mg,scratch.data(),c.data());double ms=(omp_get_wtime()-t)*1000;
            tim<<g.name<<','<<m.name<<','<<(r<0?"kernel_warmup":"kernel")<<','<<r<<','<<ms<<",0,0,0\n";
            if(m.mkl && r>=0){pro<<g.name<<','<<m.name<<",mkl_spmm_wall,"<<sp<<",0,"<<ms<<'\n'<<g.name<<','<<m.name<<",mkl_gemm_wall,"<<gm<<",0,"<<ms<<'\n';}
            if(r>=0)printf("TIME graph=%s method=%s kernel rep=%d ms=%.6f\n",g.name.c_str(),m.name.c_str(),r,ms);fflush(stdout);
        }
        tim.flush();
        for(const Method& m:methods)if(!m.mkl) {
            Profile pr;t=omp_get_wtime();
            if(m.name=="original")original_profile(g,x.hb.data(),p1.wv.data(),c.data(),perm.get(),pr);
            else block_kernel<true>(g,x.hb.data(),p1.wv.data(),c.data(),perm.get(),m,&pr);
            double wall=(omp_get_wtime()-t)*1000;Error pe=error(c.data(),m.name=="original"?orig.data():matched.data(),z);
            double gate=m.name=="original"?0:(m.accurate?1e-3:1e-2);
            bool pass=pe.finite && (m.name=="original"?pe.maxabs==0:(pe.l2<gate && pe.maxnorm<gate));
            write_error(cor,g,m,"instrumented_kernel",m.name=="original"?"unchanged_original":"mkl_bf16_inputs",pe,z,pass);if(!pass)throw std::runtime_error("profile output mismatch");
            for(int a=0;a<PHASES;a++)pro<<g.name<<','<<m.name<<','<<phase_name[a]<<','<<pr.sec[a]*1000<<','<<pr.tiles<<','<<wall<<'\n';
        }
        if(!skip_e2e) {
            AV<float> intermediate(z),quant(z),e2eref(z),e2efp(z);AV<uint16_t> hb(z);
            e2e(g,x,p1,p2,perm.get(),qm,mg,intermediate,quant,hb,scratch,e2eref);
            Method fpm{"mkl_fp32",1,false,false,1};e2e(g,x,p1,p2,perm.get(),fpm,mg,intermediate,quant,hb,scratch,e2efp);
            std::vector<Method> em;
            for(const Method& m:methods)if(smoke || m.mkl || m.name=="original" || m.name=="shared_b8_fast" || m.name=="shared_b32_fast" || m.block==0)em.push_back(m);
            for(const Method& m:em) {
                e2e(g,x,p1,p2,perm.get(),m,mg,intermediate,quant,hb,scratch,c);
                Error e=error(c.data(),e2eref.data(),z);bool pass=e.finite && e.l2<0.03 && e.maxnorm<0.03;
                write_error(cor,g,m,"two_layer_e2e","mkl_bf16_inputs",e,z,pass);
                Error f=error(c.data(),e2efp.data(),z);write_error(cor,g,m,"two_layer_e2e","mkl_fp32",f,z,f.finite);
                if(!pass)throw std::runtime_error("two-layer correctness gate failed");
            }
            for(int r=-warmups;r<repeats;r++)for(size_t k=0;k<em.size();k++) {
                const Method& m=em[(k+size_t(r+warmups))%em.size()];E2ETimes et=e2e(g,x,p1,p2,perm.get(),m,mg,intermediate,quant,hb,scratch,c);
                tim<<g.name<<','<<m.name<<','<<(r<0?"e2e_warmup":"e2e")<<','<<r<<','<<et.total<<','<<et.l1<<','<<et.act<<','<<et.l2<<'\n';
                if(r>=0)printf("TIME graph=%s method=%s e2e rep=%d ms=%.6f L1=%.6f act_convert=%.6f L2=%.6f\n",g.name.c_str(),m.name.c_str(),r,et.total,et.l1,et.act,et.l2);fflush(stdout);
            }
        }
        uint64_t gh=hash_bytes(g.row.data(),g.row.size()*4);gh=hash_bytes(g.col.data(),g.col.size()*4,gh);
        uint64_t hh=hash_bytes(x.hb.data(),x.hb.size()*2),wh=hash_bytes(p1.wv.data(),p1.wv.size()*2);rusage ru{};getrusage(RUSAGE_SELF,&ru);
        std::ofstream info(out+"/info.json");info<<"{\"graph\":\""<<g.name<<"\",\"N\":"<<g.n<<",\"E\":"<<g.e<<",\"self_loops\":"<<g.self_loops<<",\"threads\":"<<omp_get_max_threads()<<",\"seed\":"<<seed<<",\"graph_fnv64\":\""<<gh<<"\",\"H_bf16_fnv64\":\""<<hh<<"\",\"W_packed_fnv64\":\""<<wh<<"\",\"max_rss_kib\":"<<ru.ru_maxrss<<",\"sample_rows\":"<<s.rows.size()<<",\"profile_stride\":"<<PROFILE_STRIDE<<",\"status\":\"PASS\",\"paper_status\":\"UNVERIFIED\"}\n";
        printf("COMPLETE graph=%s status=PASS max_rss_kib=%ld\n",g.name.c_str(),ru.ru_maxrss);return 0;
    } catch(const std::exception& e) {fprintf(stderr,"FATAL %s\n",e.what());return 2;}
}
