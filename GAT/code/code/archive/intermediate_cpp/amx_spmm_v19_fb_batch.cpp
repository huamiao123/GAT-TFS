/**
 * V17c: BF16 Fallback
 * 
 * Key insight: Fallback is MEMORY-BOUND (loading B rows).
 *   FP32 B row at K=128: 512 bytes = 8 cache lines
 *   BF16 B row at K=128: 256 bytes = 4 cache lines  ← half!
 * 
 * We already have B_bf16. Convert bf16→fp32 on the fly:
 *   fp32 = bf16 << 16  (bf16 is upper 16 bits of fp32)
 * 
 * Trade: 4 extra ALU ops per B row, save 4 cache line loads.
 * Since FB is memory-bound, this should be a big win.
 *
 * Also tests: BF16 fallback + Two-Pass combined.
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
constexpr int MAX_NTILES=64, PF_DIST=3;

typedef struct __attribute__((aligned(64))){
    uint8_t palette_id,start_row,reserved0[14];
    uint16_t colsb[16];uint8_t rows[16];
}tile_config_t;

static inline uint16_t f32_to_bf16(float f){
    uint32_t u;memcpy(&u,&f,4);u+=0x7FFF+((u>>16)&1);return(uint16_t)(u>>16);
}

struct CSR{std::vector<uint32_t>indptr,indices;std::vector<float>values;int M,N;int64_t nnz;};
CSR read_csrbin(const char*path){
    CSR c;FILE*f=fopen(path,"rb");if(!f){fprintf(stderr,"ERR:%s\n",path);exit(1);}
    uint32_t h[3];uint64_t d[3];fread(h,4,3,f);fread(d,8,3,f);
    c.M=(int)d[0];c.N=(int)d[1];c.nnz=(int64_t)d[2];
    c.indptr.resize(c.M+1);c.indices.resize(c.nnz);c.values.resize(c.nnz);
    fread(c.indptr.data(),4,c.M+1,f);fread(c.indices.data(),4,c.nnz,f);fread(c.values.data(),4,c.nnz,f);
    fclose(f);return c;
}
struct CSC{std::vector<int64_t>col_ptr;std::vector<int>col_rows,col_counts;};
CSC build_csc(const CSR&csr){
    CSC sc;int N=csr.N;sc.col_counts.assign(N,0);
    for(int64_t i=0;i<csr.nnz;i++)sc.col_counts[csr.indices[i]]++;
    sc.col_ptr.assign(N+1,0);for(int j=0;j<N;j++)sc.col_ptr[j+1]=sc.col_ptr[j]+sc.col_counts[j];
    sc.col_rows.resize(csr.nnz);std::vector<int64_t>pos(N,0);
    for(int i=0;i<csr.M;i++)for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){int c=csr.indices[p];sc.col_rows[sc.col_ptr[c]+pos[c]++]=i;}
    return sc;
}

struct Panel{int rows[16];int nrows,U,ntiles,total_deg;int64_t A_offset;std::vector<int>b_rows;};
struct PrecompData{
    std::vector<Panel>panels_16,panels_8;std::vector<int>avx_rows;
    uint16_t*A16,*A8;int64_t t16,t8,nnz16,nnz8;
};
struct PackedNZ{uint8_t row,tile_idx;uint16_t tile_col,bf16_val;};

void build_panels(const CSR&csr,const CSC&csc,int TR,int md,
    std::vector<bool>&asgn,std::vector<int>&cm,
    std::vector<Panel>&out,uint16_t*&A,int64_t&tiles,int64_t&nnz){
    int M=csr.M,N=csr.N;std::vector<int>deg(M);
    for(int i=0;i<M;i++)deg[i]=csr.indptr[i+1]-csr.indptr[i];
    std::vector<int>co;for(int j=0;j<N;j++)if(csc.col_counts[j]>=md)co.push_back(j);
    std::sort(co.begin(),co.end(),[&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});
    struct PB{Panel p;std::vector<PackedNZ>nz;};std::vector<PB>builds;int64_t toff=0;
    for(int col:co){if(csc.col_counts[col]<md)break;
        std::vector<int>av;for(int64_t p=csc.col_ptr[col];p<csc.col_ptr[col+1];p++){int r=csc.col_rows[p];if(!asgn[r])av.push_back(r);}
        if((int)av.size()<TR)continue;
        if((int)av.size()>TR)std::partial_sort(av.begin(),av.begin()+TR,av.end(),[&](int a,int b){return deg[a]<deg[b];});
        std::vector<int>cols;int td=0;
        for(int i=0;i<TR;i++){int r=av[i];td+=deg[r];for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++)cols.push_back(csr.indices[p]);}
        std::sort(cols.begin(),cols.end());cols.erase(std::unique(cols.begin(),cols.end()),cols.end());
        int U=(int)cols.size(),nt=(U+TILE_C-1)/TILE_C;
        float fill=(float)td/(TR*TILE_C*nt);if(fill<FILL_THR||nt>MAX_NTILES)continue;
        for(int j=0;j<(int)cols.size();j++)cm[cols[j]]=j;
        Panel panel;panel.nrows=TR;panel.U=U;panel.ntiles=nt;panel.total_deg=td;panel.A_offset=toff*TR*TILE_C;
        for(int i=0;i<TR;i++)panel.rows[i]=av[i];
        std::vector<PackedNZ>pnz;pnz.reserve(td);
        for(int i=0;i<TR;i++){int r=av[i];for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){int j=cm[csr.indices[p]];pnz.push_back({(uint8_t)i,(uint8_t)(j/TILE_C),(uint16_t)(j%TILE_C),f32_to_bf16(csr.values[p])});}}
        panel.b_rows.assign(nt*32,-1);
        for(int t=0;t<nt;t++)for(int p=0;p<16;p++){int je=t*TILE_C+p*2,jo=je+1;if(je<U)panel.b_rows[t*32+p*2]=cols[je];if(jo<U)panel.b_rows[t*32+p*2+1]=cols[jo];}
        for(int j=0;j<(int)cols.size();j++)cm[cols[j]]=-1;
        for(int i=0;i<TR;i++)asgn[av[i]]=true;toff+=nt;builds.push_back({std::move(panel),std::move(pnz)});
    }
    tiles=toff;int64_t sz=toff*TR*TILE_C;A=(uint16_t*)aligned_alloc(64,std::max(sz,(int64_t)1)*2);
    memset(A,0,std::max(sz,(int64_t)1)*2);nnz=0;
    for(auto&pb:builds){uint16_t*base=A+pb.p.A_offset;for(auto&nz:pb.nz)base[(int64_t)nz.tile_idx*TR*TILE_C+nz.row*TILE_C+nz.tile_col]=nz.bf16_val;nnz+=pb.p.total_deg;out.push_back(std::move(pb.p));}
}


// ==================== FB Row Sorting by Primary Column ====================
// 核心思想：对每个 fallback 行，找它访问的"最高度数列"作为主键排序
// 排序后，schedule(dynamic,256) 每次抓的 256 行天然共享大量 B-row
// 复杂度：O(FB_NNZ) 找主列 + O(N log N) 排序，预处理开销极小
void sort_fb_by_primary_col(std::vector<int>& fb_rows, const CSR& csr, const CSC& csc){
    int n=(int)fb_rows.size();
    if(n<=1) return;
    auto t0=std::chrono::high_resolution_clock::now();
    // 对每个 fb 行，找它访问的全局度数最高的列
    std::vector<std::pair<int,int>> key_row(n); // (primary_col, row_id)
    for(int idx=0;idx<n;idx++){
        int r=fb_rows[idx];
        int best_col=-1, best_deg=-1;
        for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){
            int c=csr.indices[p];
            if(csc.col_counts[c]>best_deg){best_deg=csc.col_counts[c];best_col=c;}
        }
        key_row[idx]={best_col, r};
    }
    std::sort(key_row.begin(), key_row.end());
    for(int i=0;i<n;i++) fb_rows[i]=key_row[i].second;
    double ms=std::chrono::duration<double,std::milli>(
        std::chrono::high_resolution_clock::now()-t0).count();
    printf("  [FB-Sort] %d rows sorted by primary col in %.1f ms\n",n,ms);
}

PrecompData build_single(const CSR&c,const CSC&sc){
    PrecompData pd;int M=c.M,N=c.N;std::vector<bool>a(M,false);std::vector<int>cm(N,-1);
    build_panels(c,sc,16,16,a,cm,pd.panels_16,pd.A16,pd.t16,pd.nnz16);
    pd.A8=nullptr;pd.t8=0;pd.nnz8=0;
    for(int i=0;i<M;i++)if(!a[i])pd.avx_rows.push_back(i);return pd;
}
PrecompData build_two(const CSR&c,const CSC&sc){
    PrecompData pd;int M=c.M,N=c.N;std::vector<bool>a(M,false);std::vector<int>cm(N,-1);
    build_panels(c,sc,16,16,a,cm,pd.panels_16,pd.A16,pd.t16,pd.nnz16);
    build_panels(c,sc,8,8,a,cm,pd.panels_8,pd.A8,pd.t8,pd.nnz8);
    for(int i=0;i<M;i++)if(!a[i])pd.avx_rows.push_back(i);return pd;
}

// ==================== AMX Kernel (with prefetch) ====================
static inline void pf_tile(const uint16_t*B,const int*br,int kb){
    for(int p=0;p<16;p++){int re=br[p*2],ro=br[p*2+1];
        if(re>=0){const char*a=(const char*)&B[(int64_t)re*K+kb];_mm_prefetch(a,_MM_HINT_T1);_mm_prefetch(a+64,_MM_HINT_T1);}
        if(ro>=0){const char*a=(const char*)&B[(int64_t)ro*K+kb];_mm_prefetch(a,_MM_HINT_T1);_mm_prefetch(a+64,_MM_HINT_T1);}
    }
}
static inline void gather4(const uint16_t*B,const int*br,int kb,uint8_t*b0,uint8_t*b1,uint8_t*b2,uint8_t*b3){
    for(int p=0;p<16;p++){int re=br[p*2],ro=br[p*2+1];
        uint32_t*p0=(uint32_t*)(b0+p*64);uint32_t*p1=(uint32_t*)(b1+p*64);
        uint32_t*p2=(uint32_t*)(b2+p*64);uint32_t*p3=(uint32_t*)(b3+p*64);
        if(re>=0&&ro>=0){
            const uint16_t*be=&B[(int64_t)re*K+kb];const uint16_t*bo=&B[(int64_t)ro*K+kb];
            __m512i f0=_mm512_loadu_si512(be);__m512i f1=_mm512_loadu_si512(be+32);
            __m512i g0=_mm512_loadu_si512(bo);__m512i g1=_mm512_loadu_si512(bo+32);__m512i ie,io;
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f0));io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(g0));
            _mm512_store_si512((__m512i*)p0,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f0,1));io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(g0,1));
            _mm512_store_si512((__m512i*)p1,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f1));io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(g1));
            _mm512_store_si512((__m512i*)p2,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f1,1));io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(g1,1));
            _mm512_store_si512((__m512i*)p3,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
        }else if(re>=0){
            const uint16_t*be=&B[(int64_t)re*K+kb];__m512i f0=_mm512_loadu_si512(be);__m512i f1=_mm512_loadu_si512(be+32);
            _mm512_store_si512((__m512i*)p0,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f0)));
            _mm512_store_si512((__m512i*)p1,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f0,1)));
            _mm512_store_si512((__m512i*)p2,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f1)));
            _mm512_store_si512((__m512i*)p3,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f1,1)));
        }else{__m512i z=_mm512_setzero_si512();_mm512_store_si512((__m512i*)p0,z);_mm512_store_si512((__m512i*)p1,z);_mm512_store_si512((__m512i*)p2,z);_mm512_store_si512((__m512i*)p3,z);}
    }
}
void amx_kern(const uint16_t*B,float*C,const std::vector<Panel>&panels,const uint16_t*A,int tr){
    int np=(int)panels.size();if(!np)return;
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);tile_config_t cfg;memset(&cfg,0,64);cfg.palette_id=1;
        cfg.rows[0]=tr;cfg.colsb[0]=KC*4;cfg.rows[1]=tr;cfg.colsb[1]=64;cfg.rows[2]=16;cfg.colsb[2]=KC*4;
        cfg.rows[3]=tr;cfg.colsb[3]=KC*4;cfg.rows[4]=tr;cfg.colsb[4]=KC*4;cfg.rows[5]=tr;cfg.colsb[5]=KC*4;
        _tile_loadconfig(&cfg);
        alignas(64)uint8_t b0[1024],b1[1024],b2[1024],b3[1024];
        alignas(64)float c0[16][KC],c1[16][KC],c2[16][KC],c3[16][KC];
        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){const Panel&P=panels[pi];const uint16_t*pA=A+P.A_offset;
            for(int pass=0;pass<2;pass++){int kb=pass*64;
                _tile_zero(0);_tile_zero(3);_tile_zero(4);_tile_zero(5);
                for(int pf=0;pf<PF_DIST&&pf<P.ntiles;pf++)pf_tile(B,&P.b_rows[pf*32],kb);
                for(int t=0;t<P.ntiles;t++){
                    if(t+PF_DIST<P.ntiles)pf_tile(B,&P.b_rows[(t+PF_DIST)*32],kb);
                    gather4(B,&P.b_rows[t*32],kb,b0,b1,b2,b3);
                    _tile_loadd(1,pA+(size_t)t*tr*TILE_C,64);
                    _tile_loadd(2,b0,64);_tile_dpbf16ps(0,1,2);_tile_loadd(2,b1,64);_tile_dpbf16ps(3,1,2);
                    _tile_loadd(2,b2,64);_tile_dpbf16ps(4,1,2);_tile_loadd(2,b3,64);_tile_dpbf16ps(5,1,2);
                }
                _tile_stored(0,c0,KC*4);_tile_stored(3,c1,KC*4);_tile_stored(4,c2,KC*4);_tile_stored(5,c3,KC*4);
                for(int i=0;i<P.nrows;i++){float*d=C+(int64_t)P.rows[i]*K+kb;
                    for(int k=0;k<KC;k++)d[k]=c0[i][k];for(int k=0;k<KC;k++)d[k+KC]=c1[i][k];
                    for(int k=0;k<KC;k++)d[k+2*KC]=c2[i][k];for(int k=0;k<KC;k++)d[k+3*KC]=c3[i][k];}
            }
        }
        _tile_release();
    }
}

// ==================== FP32 Fallback (baseline) ====================
void avx_fb_fp32(const CSR&c,const float*B,float*C,const std::vector<int>&rows){
    int nr=(int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx];float*Cr=C+(int64_t)i*K;
        __m512 v[8];for(int j=0;j<8;j++)v[j]=_mm512_setzero_ps();
        for(uint32_t p=c.indptr[i];p<c.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(c.values[p]);
            const float*Br=B+(int64_t)c.indices[p]*K;
            for(int j=0;j<8;j++)v[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),v[j]);
        }
        for(int j=0;j<8;j++)_mm512_storeu_ps(Cr+j*16,v[j]);
    }
}

// ==================== BF16 Fallback (NEW!) ====================
// Load bf16 B, convert to fp32 on the fly, FMA with fp32 a_ij
// K=128 bf16 = 256 bytes = 4 cache lines (vs 8 for fp32)
void avx_fb_bf16(const CSR&c, const uint16_t*B_bf16, float*C, const std::vector<int>&rows){
    int nr=(int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx]; float*Cr=C+(int64_t)i*K;
        // 8 accumulators × 16 fp32 = 128 values
        __m512 v0=_mm512_setzero_ps(), v1=_mm512_setzero_ps();
        __m512 v2=_mm512_setzero_ps(), v3=_mm512_setzero_ps();
        __m512 v4=_mm512_setzero_ps(), v5=_mm512_setzero_ps();
        __m512 v6=_mm512_setzero_ps(), v7=_mm512_setzero_ps();

        for(uint32_t p=c.indptr[i];p<c.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(c.values[p]);
            const uint16_t*Br=&B_bf16[(int64_t)c.indices[p]*K];
            
            // Load 4 × 64 bytes = 128 bf16 values (4 cache lines)
            // Each 512-bit load gives 32 bf16 → split into 2 × 16 fp32
            __m512i raw0 = _mm512_loadu_si512(Br);      // bf16[0..31]
            __m512i raw1 = _mm512_loadu_si512(Br+32);   // bf16[32..63]
            __m512i raw2 = _mm512_loadu_si512(Br+64);   // bf16[64..95]
            __m512i raw3 = _mm512_loadu_si512(Br+96);   // bf16[96..127]

            // Convert bf16 → fp32: zero-extend to 32-bit, shift left 16
            // raw0 lower 16 bf16 → fp32
            __m512 f0 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_castsi512_si256(raw0)), 16));
            __m512 f1 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(raw0, 1)), 16));
            __m512 f2 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_castsi512_si256(raw1)), 16));
            __m512 f3 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(raw1, 1)), 16));
            __m512 f4 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_castsi512_si256(raw2)), 16));
            __m512 f5 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(raw2, 1)), 16));
            __m512 f6 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_castsi512_si256(raw3)), 16));
            __m512 f7 = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(raw3, 1)), 16));

            v0=_mm512_fmadd_ps(a,f0,v0); v1=_mm512_fmadd_ps(a,f1,v1);
            v2=_mm512_fmadd_ps(a,f2,v2); v3=_mm512_fmadd_ps(a,f3,v3);
            v4=_mm512_fmadd_ps(a,f4,v4); v5=_mm512_fmadd_ps(a,f5,v5);
            v6=_mm512_fmadd_ps(a,f6,v6); v7=_mm512_fmadd_ps(a,f7,v7);
        }
        _mm512_storeu_ps(Cr,    v0); _mm512_storeu_ps(Cr+16, v1);
        _mm512_storeu_ps(Cr+32, v2); _mm512_storeu_ps(Cr+48, v3);
        _mm512_storeu_ps(Cr+64, v4); _mm512_storeu_ps(Cr+80, v5);
        _mm512_storeu_ps(Cr+96, v6); _mm512_storeu_ps(Cr+112,v7);
    }
}

// ==================== Full AVX baseline (for speedup reference) ====================
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

int main(int argc,char**argv){
    if(argc<2){printf("Usage: %s <mat> [thr]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);syscall(SYS_arch_prctl,0x1023,18);
    printf("=== V19 FB Batching K=128 ===\nthreads=%d\n\n",nt);

    CSR csr=read_csrbin(argv[1]);
    printf("M=%d N=%d NNZ=%ld avg=%.1f\n\n",csr.M,csr.N,csr.nnz,(double)csr.nnz/csr.M);
    CSC csc=build_csc(csr);

    // Build
    PrecompData pd1=build_single(csr,csc);
    PrecompData pd2=build_two(csr,csc);
    
    // Check if two-pass is worth it
    auto count_nnz=[&](const std::vector<int>&r)->int64_t{int64_t s=0;for(int i:r)s+=csr.indptr[i+1]-csr.indptr[i];return s;};
    int64_t fb1_nnz=count_nnz(pd1.avx_rows), fb2_nnz=count_nnz(pd2.avx_rows);
    double fb_reduction = 1.0 - (double)fb2_nnz / fb1_nnz;
    bool use_2p = fb_reduction > 0.30;

    printf("1-Pass: AMX=%.1f%% FB=%d (%.1f%% NNZ)\n",
        100.0*pd1.nnz16/csr.nnz,(int)pd1.avx_rows.size(),100.0*fb1_nnz/csr.nnz);
    printf("2-Pass: AMX=%.1f%% FB=%d (%.1f%% NNZ) reduction=%.0f%% %s\n\n",
        100.0*(pd2.nnz16+pd2.nnz8)/csr.nnz,(int)pd2.avx_rows.size(),100.0*fb2_nnz/csr.nnz,
        fb_reduction*100, use_2p?"→ USE 2P":"→ SKIP 2P");

    // Allocate
    float*B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);for(int64_t i=0;i<(int64_t)csr.N*K;i++)B[i]=(rand()%200-100)/100.0f;
    uint16_t*Bb=(uint16_t*)aligned_alloc(64,(size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0;i<(int64_t)csr.N*K;i++)Bb[i]=f32_to_bf16(B[i]);
    float*C=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    float*Cref=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));


    // Sort fallback rows by primary column (FB Batching)
    printf("\n--- FB Batching: sorting fallback rows ---\n");
    sort_fb_by_primary_col(pd1.avx_rows, csr, csc);
    sort_fb_by_primary_col(pd2.avx_rows, csr, csc);

    // AVX baseline
    avx_all(csr,B,C);double best_avx=1e9;
    for(int t=0;t<5;t++){auto T0=std::chrono::high_resolution_clock::now();avx_all(csr,B,C);auto T1=std::chrono::high_resolution_clock::now();double ms=std::chrono::duration<double,std::milli>(T1-T0).count();if(ms<best_avx)best_avx=ms;}
    printf("AVX-512 (FP32 all): %.2f ms\n\n",best_avx);
    memcpy(Cref,C,(size_t)csr.M*K*sizeof(float));

    // Benchmark detailed
    auto bench=[&](const char*name, auto amx_fn, auto fb_fn){
        memset(C,0,(size_t)csr.M*K*sizeof(float));amx_fn();fb_fn();
        double best=1e9,ba=1e9,bf=1e9;
        for(int t=0;t<5;t++){memset(C,0,(size_t)csr.M*K*sizeof(float));
            auto T0=std::chrono::high_resolution_clock::now();amx_fn();
            auto T1=std::chrono::high_resolution_clock::now();fb_fn();
            auto T2=std::chrono::high_resolution_clock::now();
            double a=std::chrono::duration<double,std::milli>(T1-T0).count();
            double f=std::chrono::duration<double,std::milli>(T2-T1).count();
            if(a+f<best){best=a+f;ba=a;bf=f;}}
        printf("  %-32s amx=%7.2f fb=%7.2f e2e=%7.2f (%.2fx)\n",name,ba,bf,best,best_avx/best);
    };

    // === Config 1: 1P + FP32 FB (current baseline) ===
    printf("[1] 1-Pass + FP32 Fallback:\n");
    bench("1P+FP32",
        [&](){amx_kern(Bb,C,pd1.panels_16,pd1.A16,16);},
        [&](){avx_fb_fp32(csr,B,C,pd1.avx_rows);});

    // === Config 2: 1P + BF16 FB (NEW) ===
    printf("[2] 1-Pass + BF16 Fallback:\n");
    bench("1P+BF16",
        [&](){amx_kern(Bb,C,pd1.panels_16,pd1.A16,16);},
        [&](){avx_fb_bf16(csr,Bb,C,pd1.avx_rows);});

    // === Config 3: 2P + FP32 FB ===
    printf("[3] 2-Pass + FP32 Fallback:\n");
    bench("2P+FP32",
        [&](){amx_kern(Bb,C,pd2.panels_16,pd2.A16,16);
              amx_kern(Bb,C,pd2.panels_8,pd2.A8,8);},
        [&](){avx_fb_fp32(csr,B,C,pd2.avx_rows);});

    // === Config 4: 2P + BF16 FB (ultimate combo) ===
    printf("[4] 2-Pass + BF16 Fallback:\n");
    bench("2P+BF16",
        [&](){amx_kern(Bb,C,pd2.panels_16,pd2.A16,16);
              amx_kern(Bb,C,pd2.panels_8,pd2.A8,8);},
        [&](){avx_fb_bf16(csr,Bb,C,pd2.avx_rows);});

    // === Config 5: Auto-select best of {1P,2P} × {FP32,BF16} ===
    // Smart: use 2P only if fb_reduction > 30%, always try BF16
    printf("[5] Smart Auto:\n");
    if(use_2p){
        bench("Auto(2P+BF16)",
            [&](){amx_kern(Bb,C,pd2.panels_16,pd2.A16,16);
                  amx_kern(Bb,C,pd2.panels_8,pd2.A8,8);},
            [&](){avx_fb_bf16(csr,Bb,C,pd2.avx_rows);});
    } else {
        bench("Auto(1P+BF16)",
            [&](){amx_kern(Bb,C,pd1.panels_16,pd1.A16,16);},
            [&](){avx_fb_bf16(csr,Bb,C,pd1.avx_rows);});
    }

    // Correctness (using BF16 FB)
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_kern(Bb,C,pd1.panels_16,pd1.A16,16);
    avx_fb_bf16(csr,Bb,C,pd1.avx_rows);
    int bad=0;float mx=0;
    for(int64_t i=0;i<(int64_t)csr.M*K;i++){
        float ae=fabsf(Cref[i]-C[i]);if(ae>mx)mx=ae;
        float ref=fabsf(Cref[i]);
        float re=(ref>1e-6f)?ae/ref:ae;
        if(re>0.05f&&ref>0.01f)bad++;
    }
    printf("\nCorrect: max=%.3e bad=%.4f%% %s\n",mx,100.0*bad/((double)csr.M*K),(100.0*bad/((double)csr.M*K))<1.0?"OK":"WARN");

    // Isolated FB timing (no AMX cache pollution)
    printf("\n[FB-only timings]:\n");
    auto bench_fb=[&](const char*n,auto fn){
        fn();double best=1e9;
        for(int t=0;t<5;t++){memset(C,0,(size_t)csr.M*K*sizeof(float));
            auto T0=std::chrono::high_resolution_clock::now();fn();
            auto T1=std::chrono::high_resolution_clock::now();
            double ms=std::chrono::duration<double,std::milli>(T1-T0).count();if(ms<best)best=ms;}
        printf("  %-32s %.2f ms\n",n,best);
    };
    bench_fb("FB-only FP32",[&](){avx_fb_fp32(csr,B,C,pd1.avx_rows);});
    bench_fb("FB-only BF16",[&](){avx_fb_bf16(csr,Bb,C,pd1.avx_rows);});


    // ==================== FB BATCHING BENCHMARK ====================
    printf("\n========================================\n");
    printf("  FB BATCHING: SORTED vs UNSORTED\n");
    printf("========================================\n");

    // 使用 Smart Auto 配置
    PrecompData& pd_use = use_2p ? pd2 : pd1;
    const char* cfg_nm = use_2p ? "2P+BF16" : "1P+BF16";

    // 当前结果已经是 sorted 的（上面排序过了），直接测
    double sorted_best = 1e9;
    for(int t=0;t<5;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        auto T0=std::chrono::high_resolution_clock::now();
        amx_kern(Bb,C,pd_use.panels_16,pd_use.A16,16);
        if(use_2p&&pd_use.A8)amx_kern(Bb,C,pd_use.panels_8,pd_use.A8,8);
        avx_fb_bf16(csr,Bb,C,pd_use.avx_rows);
        auto T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<sorted_best)sorted_best=ms;
    }

    // 打乱顺序作为 unsorted 对照组
    std::vector<int> unsorted_rows = pd_use.avx_rows;
    {
        // Fisher-Yates shuffle
        srand(42);
        for(int i=(int)unsorted_rows.size()-1;i>0;i--){
            int j=rand()%(i+1);
            std::swap(unsorted_rows[i],unsorted_rows[j]);
        }
    }
    double unsorted_best = 1e9;
    for(int t=0;t<5;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        auto T0=std::chrono::high_resolution_clock::now();
        amx_kern(Bb,C,pd_use.panels_16,pd_use.A16,16);
        if(use_2p&&pd_use.A8)amx_kern(Bb,C,pd_use.panels_8,pd_use.A8,8);
        avx_fb_bf16(csr,Bb,C,unsorted_rows);
        auto T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<unsorted_best)unsorted_best=ms;
    }

    // 原始顺序（按行号升序，V17c 默认）
    std::vector<int> original_rows = pd_use.avx_rows;
    std::sort(original_rows.begin(), original_rows.end());
    double original_best = 1e9;
    for(int t=0;t<5;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        auto T0=std::chrono::high_resolution_clock::now();
        amx_kern(Bb,C,pd_use.panels_16,pd_use.A16,16);
        if(use_2p&&pd_use.A8)amx_kern(Bb,C,pd_use.panels_8,pd_use.A8,8);
        avx_fb_bf16(csr,Bb,C,original_rows);
        auto T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<original_best)original_best=ms;
    }

    printf("  %s E2E comparison (64 threads):\n", cfg_nm);
    printf("  [Original order]     %7.2f ms  (%.2fx vs AVX)\n", original_best, best_avx/original_best);
    printf("  [Random shuffle]     %7.2f ms  (%.2fx vs AVX)\n", unsorted_best, best_avx/unsorted_best);
    printf("  [Sorted by pri-col]  %7.2f ms  (%.2fx vs AVX)  %+.1f%% vs original\n",
        sorted_best, best_avx/sorted_best, (sorted_best/original_best-1)*100);
    printf("\n");

    // FB-only timing comparison (isolated, no AMX cache pollution)
    printf("  FB-only timing (isolated):\n");
    auto bench_fb_order=[&](const char*name, const std::vector<int>& rows){
        double best=1e9;
        for(int t=0;t<5;t++){
            memset(C,0,(size_t)csr.M*K*sizeof(float));
            auto T0=std::chrono::high_resolution_clock::now();
            avx_fb_bf16(csr,Bb,C,rows);
            auto T1=std::chrono::high_resolution_clock::now();
            double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
            if(ms<best)best=ms;
        }
        printf("    %-22s %7.2f ms\n",name,best);
        return best;
    };
    double fb_orig = bench_fb_order("FB original", original_rows);
    double fb_rand = bench_fb_order("FB random", unsorted_rows);
    double fb_sort = bench_fb_order("FB sorted", pd_use.avx_rows);
    printf("    Sorting improvement: %+.1f%% vs original\n", (fb_sort/fb_orig-1)*100);
    printf("========================================\n");

    free(B);free(Bb);free(C);free(Cref);
    if(pd1.A16)free(pd1.A16);if(pd2.A16)free(pd2.A16);if(pd2.A8)free(pd2.A8);
    printf("\n=== Done ===\n");return 0;
}
