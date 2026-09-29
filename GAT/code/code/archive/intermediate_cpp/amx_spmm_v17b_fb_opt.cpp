/**
 * V17b: Fallback Optimization Experiment
 * 
 * Tests 5 configurations on each matrix:
 *   1. V16-style single-pass (baseline)
 *   2. Two-pass naive (V17)
 *   3. Two-pass + sorted fallback
 *   4. Single-pass + sorted fallback (isolate FB sort effect)
 *   5. Interleaved: AMX on threads 0..15, FB on threads 16..31
 * 
 * K=128 only. Prefetch dist=3.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <omp.h>

constexpr int TILE_C=32, K=128, KC=16;
constexpr float FILL_THR=0.0625f;
constexpr int MAX_NTILES=64;
constexpr int PF_DIST=3;

typedef struct __attribute__((aligned(64))) {
    uint8_t palette_id, start_row, reserved0[14];
    uint16_t colsb[16]; uint8_t rows[16];
} tile_config_t;

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u,&f,4);
    u+=0x7FFF+((u>>16)&1);
    return (uint16_t)(u>>16);
}

struct CSR {
    std::vector<uint32_t> indptr,indices;
    std::vector<float> values;
    int M,N; int64_t nnz;
};
CSR read_csrbin(const char* path) {
    CSR c; FILE*f=fopen(path,"rb");
    if(!f){fprintf(stderr,"ERR: %s\n",path);exit(1);}
    uint32_t hdr[3];uint64_t dims[3];
    fread(hdr,4,3,f);fread(dims,8,3,f);
    c.M=(int)dims[0];c.N=(int)dims[1];c.nnz=(int64_t)dims[2];
    c.indptr.resize(c.M+1);c.indices.resize(c.nnz);c.values.resize(c.nnz);
    fread(c.indptr.data(),4,c.M+1,f);
    fread(c.indices.data(),4,c.nnz,f);
    fread(c.values.data(),4,c.nnz,f);
    fclose(f);return c;
}
struct CSC {
    std::vector<int64_t> col_ptr;
    std::vector<int> col_rows,col_counts;
};
CSC build_csc(const CSR& csr) {
    CSC sc;int N=csr.N;
    sc.col_counts.assign(N,0);
    for(int64_t i=0;i<csr.nnz;i++) sc.col_counts[csr.indices[i]]++;
    sc.col_ptr.assign(N+1,0);
    for(int j=0;j<N;j++) sc.col_ptr[j+1]=sc.col_ptr[j]+sc.col_counts[j];
    sc.col_rows.resize(csr.nnz);
    std::vector<int64_t> pos(N,0);
    for(int i=0;i<csr.M;i++)
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            int c=csr.indices[p];
            sc.col_rows[sc.col_ptr[c]+pos[c]++]=i;
        }
    return sc;
}

struct Panel {
    int rows[16]; int nrows,U,ntiles,total_deg;
    int64_t A_offset;
    std::vector<int> b_rows;
};
struct PrecompData {
    std::vector<Panel> panels_16, panels_8;
    std::vector<int> avx_rows;
    uint16_t *A_all_16, *A_all_8;
    int64_t total_tiles_16, total_tiles_8, amx_nnz_16, amx_nnz_8;
};
struct PackedNZ{uint8_t row,tile_idx;uint16_t tile_col,bf16_val;};

void build_panels(const CSR&csr,const CSC&csc,int TILE_R,int min_deg,
    std::vector<bool>&assigned,std::vector<int>&col_map,
    std::vector<Panel>&out,uint16_t*&A,int64_t&tiles,int64_t&nnz){
    int M=csr.M,N=csr.N;
    std::vector<int>deg(M);
    for(int i=0;i<M;i++)deg[i]=csr.indptr[i+1]-csr.indptr[i];
    std::vector<int>co;
    for(int j=0;j<N;j++)if(csc.col_counts[j]>=min_deg)co.push_back(j);
    std::sort(co.begin(),co.end(),[&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});
    struct PB{Panel p;std::vector<PackedNZ>nz;};
    std::vector<PB>builds;int64_t toff=0;
    for(int col:co){
        if(csc.col_counts[col]<min_deg)break;
        std::vector<int>av;
        for(int64_t p=csc.col_ptr[col];p<csc.col_ptr[col+1];p++){int r=csc.col_rows[p];if(!assigned[r])av.push_back(r);}
        if((int)av.size()<TILE_R)continue;
        if((int)av.size()>TILE_R)
            std::partial_sort(av.begin(),av.begin()+TILE_R,av.end(),[&](int a,int b){return deg[a]<deg[b];});
        std::vector<int>cols;int td=0;
        for(int i=0;i<TILE_R;i++){int r=av[i];td+=deg[r];for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++)cols.push_back(csr.indices[p]);}
        std::sort(cols.begin(),cols.end());cols.erase(std::unique(cols.begin(),cols.end()),cols.end());
        int U=(int)cols.size(),nt=(U+TILE_C-1)/TILE_C;
        float fill=(float)td/(TILE_R*TILE_C*nt);
        if(fill<FILL_THR||nt>MAX_NTILES)continue;
        for(int j=0;j<(int)cols.size();j++)col_map[cols[j]]=j;
        Panel panel;panel.nrows=TILE_R;panel.U=U;panel.ntiles=nt;panel.total_deg=td;
        panel.A_offset=toff*TILE_R*TILE_C;
        for(int i=0;i<TILE_R;i++)panel.rows[i]=av[i];
        std::vector<PackedNZ>pnz;pnz.reserve(td);
        for(int i=0;i<TILE_R;i++){int r=av[i];for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){int j=col_map[csr.indices[p]];pnz.push_back({(uint8_t)i,(uint8_t)(j/TILE_C),(uint16_t)(j%TILE_C),f32_to_bf16(csr.values[p])});}}
        panel.b_rows.assign(nt*32,-1);
        for(int t=0;t<nt;t++)for(int p=0;p<16;p++){int je=t*TILE_C+p*2,jo=je+1;if(je<U)panel.b_rows[t*32+p*2]=cols[je];if(jo<U)panel.b_rows[t*32+p*2+1]=cols[jo];}
        for(int j=0;j<(int)cols.size();j++)col_map[cols[j]]=-1;
        for(int i=0;i<TILE_R;i++)assigned[av[i]]=true;
        toff+=nt;builds.push_back({std::move(panel),std::move(pnz)});
    }
    tiles=toff;int64_t sz=toff*TILE_R*TILE_C;
    A=(uint16_t*)aligned_alloc(64,std::max(sz,(int64_t)1)*2);
    memset(A,0,std::max(sz,(int64_t)1)*2);nnz=0;
    for(auto&pb:builds){uint16_t*base=A+pb.p.A_offset;for(auto&nz:pb.nz)base[(int64_t)nz.tile_idx*TILE_R*TILE_C+nz.row*TILE_C+nz.tile_col]=nz.bf16_val;nnz+=pb.p.total_deg;out.push_back(std::move(pb.p));}
}

PrecompData build_single(const CSR&csr,const CSC&csc){
    PrecompData pd;int M=csr.M,N=csr.N;
    std::vector<bool>asgn(M,false);std::vector<int>cm(N,-1);
    build_panels(csr,csc,16,16,asgn,cm,pd.panels_16,pd.A_all_16,pd.total_tiles_16,pd.amx_nnz_16);
    pd.A_all_8=nullptr;pd.total_tiles_8=0;pd.amx_nnz_8=0;
    for(int i=0;i<M;i++)if(!asgn[i])pd.avx_rows.push_back(i);
    return pd;
}
PrecompData build_two(const CSR&csr,const CSC&csc){
    PrecompData pd;int M=csr.M,N=csr.N;
    std::vector<bool>asgn(M,false);std::vector<int>cm(N,-1);
    build_panels(csr,csc,16,16,asgn,cm,pd.panels_16,pd.A_all_16,pd.total_tiles_16,pd.amx_nnz_16);
    build_panels(csr,csc,8,8,asgn,cm,pd.panels_8,pd.A_all_8,pd.total_tiles_8,pd.amx_nnz_8);
    for(int i=0;i<M;i++)if(!asgn[i])pd.avx_rows.push_back(i);
    return pd;
}

// Sort fallback rows by their median column index → B access locality
std::vector<int> sort_fb_rows(const CSR& csr, const std::vector<int>& rows) {
    std::vector<int> sorted = rows;
    std::sort(sorted.begin(), sorted.end(), [&](int a, int b) {
        // Sort by first column index (proxy for B-row access locality)
        uint32_t ca = (csr.indptr[a] < csr.indptr[a+1]) ? csr.indices[csr.indptr[a]] : 0;
        uint32_t cb = (csr.indptr[b] < csr.indptr[b+1]) ? csr.indices[csr.indptr[b]] : 0;
        return ca < cb;
    });
    return sorted;
}

// ==================== Kernels ====================
static inline void prefetch_tile_B(const uint16_t*B,const int*br,int kb){
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        if(re>=0){const char*a=(const char*)&B[(int64_t)re*K+kb];_mm_prefetch(a,_MM_HINT_T1);_mm_prefetch(a+64,_MM_HINT_T1);}
        if(ro>=0){const char*a=(const char*)&B[(int64_t)ro*K+kb];_mm_prefetch(a,_MM_HINT_T1);_mm_prefetch(a+64,_MM_HINT_T1);}
    }
}
static inline void gather4(const uint16_t*B,const int*br,int kb,uint8_t*b0,uint8_t*b1,uint8_t*b2,uint8_t*b3){
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        uint32_t*p0=(uint32_t*)(b0+p*64);uint32_t*p1=(uint32_t*)(b1+p*64);
        uint32_t*p2=(uint32_t*)(b2+p*64);uint32_t*p3=(uint32_t*)(b3+p*64);
        if(re>=0&&ro>=0){
            const uint16_t*be=&B[(int64_t)re*K+kb];const uint16_t*bo=&B[(int64_t)ro*K+kb];
            __m512i f0=_mm512_loadu_si512(be);__m512i f1=_mm512_loadu_si512(be+32);
            __m512i g0=_mm512_loadu_si512(bo);__m512i g1=_mm512_loadu_si512(bo+32);
            __m512i ie,io;
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f0));io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(g0));
            _mm512_store_si512((__m512i*)p0,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f0,1));io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(g0,1));
            _mm512_store_si512((__m512i*)p1,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f1));io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(g1));
            _mm512_store_si512((__m512i*)p2,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f1,1));io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(g1,1));
            _mm512_store_si512((__m512i*)p3,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
        }else if(re>=0){
            const uint16_t*be=&B[(int64_t)re*K+kb];
            __m512i f0=_mm512_loadu_si512(be);__m512i f1=_mm512_loadu_si512(be+32);
            _mm512_store_si512((__m512i*)p0,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f0)));
            _mm512_store_si512((__m512i*)p1,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f0,1)));
            _mm512_store_si512((__m512i*)p2,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f1)));
            _mm512_store_si512((__m512i*)p3,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f1,1)));
        }else{
            __m512i z=_mm512_setzero_si512();
            _mm512_store_si512((__m512i*)p0,z);_mm512_store_si512((__m512i*)p1,z);
            _mm512_store_si512((__m512i*)p2,z);_mm512_store_si512((__m512i*)p3,z);
        }
    }
}

void amx_kern(const uint16_t*B,float*C,const std::vector<Panel>&panels,const uint16_t*A,int tr){
    int np=(int)panels.size();if(!np)return;
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        tile_config_t cfg;memset(&cfg,0,64);cfg.palette_id=1;
        cfg.rows[0]=tr;cfg.colsb[0]=KC*4;cfg.rows[1]=tr;cfg.colsb[1]=64;
        cfg.rows[2]=16;cfg.colsb[2]=KC*4;
        cfg.rows[3]=tr;cfg.colsb[3]=KC*4;cfg.rows[4]=tr;cfg.colsb[4]=KC*4;cfg.rows[5]=tr;cfg.colsb[5]=KC*4;
        _tile_loadconfig(&cfg);
        alignas(64)uint8_t b0[1024],b1[1024],b2[1024],b3[1024];
        alignas(64)float c0[16][KC],c1[16][KC],c2[16][KC],c3[16][KC];
        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){
            const Panel&P=panels[pi];const uint16_t*pA=A+P.A_offset;
            for(int pass=0;pass<2;pass++){
                int kb=pass*64;
                _tile_zero(0);_tile_zero(3);_tile_zero(4);_tile_zero(5);
                for(int pf=0;pf<PF_DIST&&pf<P.ntiles;pf++)prefetch_tile_B(B,&P.b_rows[pf*32],kb);
                for(int t=0;t<P.ntiles;t++){
                    if(t+PF_DIST<P.ntiles)prefetch_tile_B(B,&P.b_rows[(t+PF_DIST)*32],kb);
                    gather4(B,&P.b_rows[t*32],kb,b0,b1,b2,b3);
                    _tile_loadd(1,pA+(size_t)t*tr*TILE_C,64);
                    _tile_loadd(2,b0,64);_tile_dpbf16ps(0,1,2);
                    _tile_loadd(2,b1,64);_tile_dpbf16ps(3,1,2);
                    _tile_loadd(2,b2,64);_tile_dpbf16ps(4,1,2);
                    _tile_loadd(2,b3,64);_tile_dpbf16ps(5,1,2);
                }
                _tile_stored(0,c0,KC*4);_tile_stored(3,c1,KC*4);_tile_stored(4,c2,KC*4);_tile_stored(5,c3,KC*4);
                for(int i=0;i<P.nrows;i++){
                    float*d=C+(int64_t)P.rows[i]*K+kb;
                    for(int k=0;k<KC;k++)d[k]=c0[i][k];
                    for(int k=0;k<KC;k++)d[k+KC]=c1[i][k];
                    for(int k=0;k<KC;k++)d[k+2*KC]=c2[i][k];
                    for(int k=0;k<KC;k++)d[k+3*KC]=c3[i][k];
                }
            }
        }
        _tile_release();
    }
}

// AMX kernel that only runs on a subset of threads
void amx_kern_partial(const uint16_t*B,float*C,const std::vector<Panel>&panels,const uint16_t*A,int tr, int thread_start, int thread_end){
    int np=(int)panels.size();if(!np)return;
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if(tid >= thread_start && tid < thread_end) {
            syscall(SYS_arch_prctl,0x1023,18);
            tile_config_t cfg;memset(&cfg,0,64);cfg.palette_id=1;
            cfg.rows[0]=tr;cfg.colsb[0]=KC*4;cfg.rows[1]=tr;cfg.colsb[1]=64;
            cfg.rows[2]=16;cfg.colsb[2]=KC*4;
            cfg.rows[3]=tr;cfg.colsb[3]=KC*4;cfg.rows[4]=tr;cfg.colsb[4]=KC*4;cfg.rows[5]=tr;cfg.colsb[5]=KC*4;
            _tile_loadconfig(&cfg);
            alignas(64)uint8_t b0[1024],b1[1024],b2[1024],b3[1024];
            alignas(64)float c0[16][KC],c1[16][KC],c2[16][KC],c3[16][KC];
            // Manual work distribution among [thread_start, thread_end)
            int my_threads = thread_end - thread_start;
            int my_id = tid - thread_start;
            int chunk = (np + my_threads - 1) / my_threads;
            int pi_start = my_id * chunk;
            int pi_end = std::min(pi_start + chunk, np);
            for(int pi=pi_start;pi<pi_end;pi++){
                const Panel&P=panels[pi];const uint16_t*pA=A+P.A_offset;
                for(int pass=0;pass<2;pass++){
                    int kb=pass*64;
                    _tile_zero(0);_tile_zero(3);_tile_zero(4);_tile_zero(5);
                    for(int pf=0;pf<PF_DIST&&pf<P.ntiles;pf++)prefetch_tile_B(B,&P.b_rows[pf*32],kb);
                    for(int t=0;t<P.ntiles;t++){
                        if(t+PF_DIST<P.ntiles)prefetch_tile_B(B,&P.b_rows[(t+PF_DIST)*32],kb);
                        gather4(B,&P.b_rows[t*32],kb,b0,b1,b2,b3);
                        _tile_loadd(1,pA+(size_t)t*tr*TILE_C,64);
                        _tile_loadd(2,b0,64);_tile_dpbf16ps(0,1,2);
                        _tile_loadd(2,b1,64);_tile_dpbf16ps(3,1,2);
                        _tile_loadd(2,b2,64);_tile_dpbf16ps(4,1,2);
                        _tile_loadd(2,b3,64);_tile_dpbf16ps(5,1,2);
                    }
                    _tile_stored(0,c0,KC*4);_tile_stored(3,c1,KC*4);_tile_stored(4,c2,KC*4);_tile_stored(5,c3,KC*4);
                    for(int i=0;i<P.nrows;i++){
                        float*d=C+(int64_t)P.rows[i]*K+kb;
                        for(int k=0;k<KC;k++)d[k]=c0[i][k];
                        for(int k=0;k<KC;k++)d[k+KC]=c1[i][k];
                        for(int k=0;k<KC;k++)d[k+2*KC]=c2[i][k];
                        for(int k=0;k<KC;k++)d[k+3*KC]=c3[i][k];
                    }
                }
            }
            _tile_release();
        }
    }
}

void avx_all(const CSR&c,const float*B,float*C){
    #pragma omp parallel for schedule(dynamic,256)
    for(int i=0;i<c.M;i++){
        float*Cr=C+(int64_t)i*K;
        __m512 v[8];for(int j=0;j<8;j++)v[j]=_mm512_setzero_ps();
        for(uint32_t p=c.indptr[i];p<c.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(c.values[p]);const float*Br=B+(int64_t)c.indices[p]*K;
            for(int j=0;j<8;j++)v[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),v[j]);
        }
        for(int j=0;j<8;j++)_mm512_storeu_ps(Cr+j*16,v[j]);
    }
}
void avx_rows(const CSR&c,const float*B,float*C,const std::vector<int>&rows){
    int nr=(int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx];float*Cr=C+(int64_t)i*K;
        __m512 v[8];for(int j=0;j<8;j++)v[j]=_mm512_setzero_ps();
        for(uint32_t p=c.indptr[i];p<c.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(c.values[p]);const float*Br=B+(int64_t)c.indices[p]*K;
            for(int j=0;j<8;j++)v[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),v[j]);
        }
        for(int j=0;j<8;j++)_mm512_storeu_ps(Cr+j*16,v[j]);
    }
}

// AVX fallback on subset of threads
void avx_rows_partial(const CSR&c,const float*B,float*C,const std::vector<int>&rows,int thr_start,int thr_end){
    int nr=(int)rows.size();
    #pragma omp parallel
    {
        int tid=omp_get_thread_num();
        if(tid>=thr_start&&tid<thr_end){
            int my_threads=thr_end-thr_start;
            int my_id=tid-thr_start;
            int chunk=(nr+my_threads-1)/my_threads;
            int start=my_id*chunk;
            int end=std::min(start+chunk,nr);
            for(int idx=start;idx<end;idx++){
                int i=rows[idx];float*Cr=C+(int64_t)i*K;
                __m512 v[8];for(int j=0;j<8;j++)v[j]=_mm512_setzero_ps();
                for(uint32_t p=c.indptr[i];p<c.indptr[i+1];p++){
                    __m512 a=_mm512_set1_ps(c.values[p]);const float*Br=B+(int64_t)c.indices[p]*K;
                    for(int j=0;j<8;j++)v[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),v[j]);
                }
                for(int j=0;j<8;j++)_mm512_storeu_ps(Cr+j*16,v[j]);
            }
        }
    }
}

int main(int argc,char**argv){
    if(argc<2){printf("Usage: %s <mat> [thr]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl,0x1023,18);
    printf("=== V17b Fallback Optimization K=128 ===\nthreads=%d\n\n",nt);

    CSR csr=read_csrbin(argv[1]);
    printf("M=%d N=%d NNZ=%ld avg=%.1f\n\n",csr.M,csr.N,csr.nnz,(double)csr.nnz/csr.M);
    CSC csc=build_csc(csr);

    // Build both configurations
    printf("[Build single-pass]...\n");
    PrecompData pd1=build_single(csr,csc);
    printf("  P16=%d T16=%ld AMX=%.1f%% FB=%d\n",
        (int)pd1.panels_16.size(),pd1.total_tiles_16,100.0*pd1.amx_nnz_16/csr.nnz,(int)pd1.avx_rows.size());

    printf("[Build two-pass]...\n");
    PrecompData pd2=build_two(csr,csc);
    printf("  P16=%d T16=%ld P8=%d T8=%ld AMX=%.1f%% FB=%d\n",
        (int)pd2.panels_16.size(),pd2.total_tiles_16,
        (int)pd2.panels_8.size(),pd2.total_tiles_8,
        100.0*(pd2.amx_nnz_16+pd2.amx_nnz_8)/csr.nnz,(int)pd2.avx_rows.size());

    // Sort fallback rows
    std::vector<int> fb1_sorted = sort_fb_rows(csr, pd1.avx_rows);
    std::vector<int> fb2_sorted = sort_fb_rows(csr, pd2.avx_rows);

    // Count FB NNZ for diagnostics
    auto count_nnz = [&](const std::vector<int>& rows) -> int64_t {
        int64_t s=0; for(int r:rows) s+=csr.indptr[r+1]-csr.indptr[r]; return s;
    };
    printf("  FB1 NNZ=%ld (%.1f%%) FB2 NNZ=%ld (%.1f%%)\n",
        count_nnz(pd1.avx_rows),100.0*count_nnz(pd1.avx_rows)/csr.nnz,
        count_nnz(pd2.avx_rows),100.0*count_nnz(pd2.avx_rows)/csr.nnz);

    // Allocate
    float*B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);for(int64_t i=0;i<(int64_t)csr.N*K;i++)B[i]=(rand()%200-100)/100.0f;
    uint16_t*Bb=(uint16_t*)aligned_alloc(64,(size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0;i<(int64_t)csr.N*K;i++)Bb[i]=f32_to_bf16(B[i]);
    float*C=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));

    // AVX baseline
    avx_all(csr,B,C);
    double best_avx=1e9;
    for(int t=0;t<5;t++){auto T0=std::chrono::high_resolution_clock::now();avx_all(csr,B,C);auto T1=std::chrono::high_resolution_clock::now();double ms=std::chrono::duration<double,std::milli>(T1-T0).count();if(ms<best_avx)best_avx=ms;}
    printf("\nAVX-512: %.2f ms\n\n",best_avx);

    // Benchmark helper
    auto bench = [&](const char*name, auto run_fn) {
        // warmup
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        run_fn();
        double best=1e9;
        for(int t=0;t<5;t++){
            memset(C,0,(size_t)csr.M*K*sizeof(float));
            auto T0=std::chrono::high_resolution_clock::now();
            run_fn();
            auto T1=std::chrono::high_resolution_clock::now();
            double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
            if(ms<best)best=ms;
        }
        printf("  %-35s e2e=%7.2f ms (%.2fx)\n",name,best,best_avx/best);
    };

    // Detailed benchmark with phase timing
    auto bench_detail = [&](const char*name, auto amx_fn, auto fb_fn) {
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        amx_fn(); fb_fn();
        double best=1e9,ba=1e9,bf=1e9;
        for(int t=0;t<5;t++){
            memset(C,0,(size_t)csr.M*K*sizeof(float));
            auto T0=std::chrono::high_resolution_clock::now();
            amx_fn();
            auto T1=std::chrono::high_resolution_clock::now();
            fb_fn();
            auto T2=std::chrono::high_resolution_clock::now();
            double a=std::chrono::duration<double,std::milli>(T1-T0).count();
            double f=std::chrono::duration<double,std::milli>(T2-T1).count();
            if(a+f<best){best=a+f;ba=a;bf=f;}
        }
        printf("  %-35s amx=%7.2f fb=%7.2f e2e=%7.2f (%.2fx)\n",name,ba,bf,best,best_avx/best);
    };

    printf("[Config 1] Single-pass, unsorted FB:\n");
    bench_detail("1P-unsorted",
        [&](){ amx_kern(Bb,C,pd1.panels_16,pd1.A_all_16,16); },
        [&](){ avx_rows(csr,B,C,pd1.avx_rows); });

    printf("[Config 2] Single-pass, SORTED FB:\n");
    bench_detail("1P-sorted",
        [&](){ amx_kern(Bb,C,pd1.panels_16,pd1.A_all_16,16); },
        [&](){ avx_rows(csr,B,C,fb1_sorted); });

    printf("[Config 3] Two-pass, unsorted FB:\n");
    bench_detail("2P-unsorted",
        [&](){ amx_kern(Bb,C,pd2.panels_16,pd2.A_all_16,16);
               amx_kern(Bb,C,pd2.panels_8,pd2.A_all_8,8); },
        [&](){ avx_rows(csr,B,C,pd2.avx_rows); });

    printf("[Config 4] Two-pass, SORTED FB:\n");
    bench_detail("2P-sorted",
        [&](){ amx_kern(Bb,C,pd2.panels_16,pd2.A_all_16,16);
               amx_kern(Bb,C,pd2.panels_8,pd2.A_all_8,8); },
        [&](){ avx_rows(csr,B,C,fb2_sorted); });

    printf("[Config 5] Single-pass, INTERLEAVED:\n");
    bench("1P-interleaved", [&](){
        // AMX on threads 0..15, Fallback on threads 16..31, simultaneously
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            if(tid < 16) {
                // AMX work
                syscall(SYS_arch_prctl,0x1023,18);
                tile_config_t cfg;memset(&cfg,0,64);cfg.palette_id=1;
                cfg.rows[0]=16;cfg.colsb[0]=KC*4;cfg.rows[1]=16;cfg.colsb[1]=64;
                cfg.rows[2]=16;cfg.colsb[2]=KC*4;
                cfg.rows[3]=16;cfg.colsb[3]=KC*4;cfg.rows[4]=16;cfg.colsb[4]=KC*4;cfg.rows[5]=16;cfg.colsb[5]=KC*4;
                _tile_loadconfig(&cfg);
                alignas(64)uint8_t b0[1024],b1[1024],b2[1024],b3[1024];
                alignas(64)float c0[16][KC],c1[16][KC],c2[16][KC],c3[16][KC];
                int np=(int)pd1.panels_16.size();
                int chunk=(np+16-1)/16;
                int pi_s=tid*chunk, pi_e=std::min(pi_s+chunk,np);
                for(int pi=pi_s;pi<pi_e;pi++){
                    const Panel&P=pd1.panels_16[pi];const uint16_t*pA=pd1.A_all_16+P.A_offset;
                    for(int pass=0;pass<2;pass++){
                        int kb=pass*64;
                        _tile_zero(0);_tile_zero(3);_tile_zero(4);_tile_zero(5);
                        for(int pf=0;pf<PF_DIST&&pf<P.ntiles;pf++)prefetch_tile_B(Bb,&P.b_rows[pf*32],kb);
                        for(int t=0;t<P.ntiles;t++){
                            if(t+PF_DIST<P.ntiles)prefetch_tile_B(Bb,&P.b_rows[(t+PF_DIST)*32],kb);
                            gather4(Bb,&P.b_rows[t*32],kb,b0,b1,b2,b3);
                            _tile_loadd(1,pA+(size_t)t*16*TILE_C,64);
                            _tile_loadd(2,b0,64);_tile_dpbf16ps(0,1,2);
                            _tile_loadd(2,b1,64);_tile_dpbf16ps(3,1,2);
                            _tile_loadd(2,b2,64);_tile_dpbf16ps(4,1,2);
                            _tile_loadd(2,b3,64);_tile_dpbf16ps(5,1,2);
                        }
                        _tile_stored(0,c0,KC*4);_tile_stored(3,c1,KC*4);_tile_stored(4,c2,KC*4);_tile_stored(5,c3,KC*4);
                        for(int i=0;i<16;i++){
                            float*d=C+(int64_t)P.rows[i]*K+kb;
                            for(int k=0;k<KC;k++)d[k]=c0[i][k];
                            for(int k=0;k<KC;k++)d[k+KC]=c1[i][k];
                            for(int k=0;k<KC;k++)d[k+2*KC]=c2[i][k];
                            for(int k=0;k<KC;k++)d[k+3*KC]=c3[i][k];
                        }
                    }
                }
                _tile_release();
            } else {
                // Fallback work on threads 16..31
                int fb_threads = nt - 16;
                int my_id = tid - 16;
                int nr = (int)fb1_sorted.size();
                int chunk = (nr + fb_threads - 1) / fb_threads;
                int start = my_id * chunk;
                int end = std::min(start + chunk, nr);
                for(int idx=start;idx<end;idx++){
                    int i=fb1_sorted[idx];float*Cr=C+(int64_t)i*K;
                    __m512 v[8];for(int j=0;j<8;j++)v[j]=_mm512_setzero_ps();
                    for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
                        __m512 a=_mm512_set1_ps(csr.values[p]);const float*Br=B+(int64_t)csr.indices[p]*K;
                        for(int j=0;j<8;j++)v[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),v[j]);
                    }
                    for(int j=0;j<8;j++)_mm512_storeu_ps(Cr+j*16,v[j]);
                }
            }
        }
    });

    // Fallback-only benchmark (to measure pure FB cost)
    printf("\n[FB-only timings]:\n");
    bench("FB-only unsorted (all threads)",
        [&](){ avx_rows(csr,B,C,pd1.avx_rows); });
    bench("FB-only SORTED (all threads)",
        [&](){ avx_rows(csr,B,C,fb1_sorted); });

    free(B);free(Bb);free(C);
    if(pd1.A_all_16)free(pd1.A_all_16);
    if(pd2.A_all_16)free(pd2.A_all_16);
    if(pd2.A_all_8)free(pd2.A_all_8);
    printf("\n=== Done ===\n");return 0;
}
