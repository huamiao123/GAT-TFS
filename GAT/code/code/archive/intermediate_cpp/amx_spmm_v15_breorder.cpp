/**
 * V15: B-Row Degree Reordering + Panel Ordering
 * 
 * Insight: AMX gather bottleneck = random B row access. Two orthogonal attacks:
 * 
 * (1) B-row reordering: sort B rows by column degree (descending).
 *     Hot B rows (high-degree columns) packed at beginning of array.
 *     These fit in L2 and are reused across thousands of panels.
 *     Physical layout: B_new[rank[j], :] = B_old[j, :]
 *     Column indices remapped: A_new.col = rank[A_old.col]
 *
 * (2) Panel ordering: sort panels by "centroid" of their B-row indices.
 *     Consecutive panels access similar B regions → L2 temporal reuse.
 *
 * Test: V12 (baseline) vs V15a (B-reorder) vs V15b (panel-order) vs V15c (both)
 * K=128, soc-Pokec + web-Google
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

constexpr int TILE_R=16, TILE_C=32, K=128, KC=16;
constexpr float FILL_THR=0.0625f;
constexpr int MAX_NTILES=64;

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

// ============ B-row degree reordering ============
// Returns permutation: new_index -> old_index (for B row access)
// and rank: old_index -> new_index (for column remapping)
struct ReorderInfo {
    std::vector<int> perm;  // perm[new] = old
    std::vector<int> rank;  // rank[old] = new
};

ReorderInfo build_reorder(const CSR& csr) {
    int N = csr.N;
    // Count column degrees
    std::vector<int> col_deg(N, 0);
    for(int64_t i = 0; i < csr.nnz; i++) col_deg[csr.indices[i]]++;
    
    // Sort by degree descending
    ReorderInfo ri;
    ri.perm.resize(N);
    std::iota(ri.perm.begin(), ri.perm.end(), 0);
    std::sort(ri.perm.begin(), ri.perm.end(), 
        [&](int a, int b){ return col_deg[a] > col_deg[b]; });
    
    // Build reverse mapping
    ri.rank.resize(N);
    for(int i = 0; i < N; i++) ri.rank[ri.perm[i]] = i;
    
    // Stats
    int64_t top1k_deg = 0;
    for(int i = 0; i < std::min(1000, N); i++) top1k_deg += col_deg[ri.perm[i]];
    printf("  Top-1000 cols cover %.1f%% of NNZ\n", 100.0*top1k_deg/csr.nnz);
    printf("  Top-1000 B footprint: %.1f KB (L2=2MB)\n", 1000.0*K*2/1024);
    
    return ri;
}

// Create reordered CSR (column indices remapped)
CSR remap_csr(const CSR& csr, const ReorderInfo& ri) {
    CSR out = csr;
    for(int64_t i = 0; i < csr.nnz; i++)
        out.indices[i] = ri.rank[csr.indices[i]];
    return out;
}

// Create reordered B matrix
void reorder_B(const float* B_old, float* B_new, const uint16_t* Bbf16_old, uint16_t* Bbf16_new,
               const ReorderInfo& ri, int N) {
    #pragma omp parallel for
    for(int new_idx = 0; new_idx < N; new_idx++) {
        int old_idx = ri.perm[new_idx];
        memcpy(B_new + (int64_t)new_idx*K, B_old + (int64_t)old_idx*K, K*sizeof(float));
        memcpy(Bbf16_new + (int64_t)new_idx*K, Bbf16_old + (int64_t)old_idx*K, K*sizeof(uint16_t));
    }
}

// ============ Standard tiling (same as V12) ============
struct CSC {
    std::vector<int64_t> col_ptr;
    std::vector<int> col_rows, col_counts;
};
CSC build_csc(const CSR& csr) {
    CSC sc; int N=csr.N;
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
    int rows[TILE_R];
    int nrows,U,ntiles,total_deg;
    int64_t A_offset;
    std::vector<int> b_rows;
    double centroid; // average B-row index (for panel ordering)
};
struct PrecompData {
    std::vector<Panel> panels;
    std::vector<int> avx_rows;
    uint16_t*A_all;
    int64_t total_tiles,amx_nnz;
};
struct PackedNZ{uint8_t row,tile_idx;uint16_t tile_col,bf16_val;};

PrecompData build_all(const CSR&csr, const CSC&csc, bool sort_panels){
    PrecompData pd;int M=csr.M,N=csr.N;
    std::vector<int>deg(M);
    for(int i=0;i<M;i++)deg[i]=csr.indptr[i+1]-csr.indptr[i];
    std::vector<int>col_order;
    for(int j=0;j<N;j++)if(csc.col_counts[j]>=TILE_R)col_order.push_back(j);
    std::sort(col_order.begin(),col_order.end(),[&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});
    std::vector<bool>assigned(M,false);
    std::vector<int>col_map(N,-1);
    struct PB{Panel panel;std::vector<PackedNZ>packed_nz;};
    std::vector<PB>builds;
    
    for(int col:col_order){
        if(csc.col_counts[col]<TILE_R)break;
        std::vector<int>avail;
        for(int64_t p=csc.col_ptr[col];p<csc.col_ptr[col+1];p++){int r=csc.col_rows[p];if(!assigned[r])avail.push_back(r);}
        if((int)avail.size()<TILE_R)continue;
        if((int)avail.size()>TILE_R)
            std::partial_sort(avail.begin(),avail.begin()+TILE_R,avail.end(),[&](int a,int b){return deg[a]<deg[b];});
        std::vector<int>cols;int td=0;
        for(int i=0;i<TILE_R;i++){int r=avail[i];td+=deg[r];for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++)cols.push_back(csr.indices[p]);}
        std::sort(cols.begin(),cols.end());cols.erase(std::unique(cols.begin(),cols.end()),cols.end());
        int U=(int)cols.size(),nt=(U+TILE_C-1)/TILE_C;
        float fill=(float)td/(TILE_R*TILE_C*nt);
        if(fill<FILL_THR||nt>MAX_NTILES)continue;
        for(int j=0;j<(int)cols.size();j++)col_map[cols[j]]=j;
        Panel panel;panel.nrows=TILE_R;panel.U=U;panel.ntiles=nt;panel.total_deg=td;
        panel.A_offset=0; // will be set after ordering
        for(int i=0;i<TILE_R;i++)panel.rows[i]=avail[i];
        std::vector<PackedNZ>pnz;pnz.reserve(td);
        for(int i=0;i<TILE_R;i++){int r=avail[i];for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){int j=col_map[csr.indices[p]];pnz.push_back({(uint8_t)i,(uint8_t)(j/TILE_C),(uint16_t)(j%TILE_C),f32_to_bf16(csr.values[p])});}}
        panel.b_rows.assign(nt*32,-1);
        // Compute centroid of B-row indices
        double sum=0; int cnt=0;
        for(int t=0;t<nt;t++)for(int p=0;p<16;p++){
            int je=t*TILE_C+p*2,jo=je+1;
            if(je<U){panel.b_rows[t*32+p*2]=cols[je]; sum+=cols[je]; cnt++;}
            if(jo<U){panel.b_rows[t*32+p*2+1]=cols[jo]; sum+=cols[jo]; cnt++;}
        }
        panel.centroid = cnt>0 ? sum/cnt : 0;
        for(int j=0;j<(int)cols.size();j++)col_map[cols[j]]=-1;
        for(int i=0;i<TILE_R;i++)assigned[avail[i]]=true;
        builds.push_back({std::move(panel),std::move(pnz)});
    }
    
    // Optionally sort panels by centroid for B-locality
    if(sort_panels){
        std::sort(builds.begin(),builds.end(),
            [](const PB&a,const PB&b){return a.panel.centroid < b.panel.centroid;});
    }
    
    // Assign A offsets and pack
    int64_t tile_offset=0;
    for(auto&pb:builds){
        pb.panel.A_offset=tile_offset*TILE_R*TILE_C;
        tile_offset+=pb.panel.ntiles;
    }
    pd.total_tiles=tile_offset;
    int64_t A_size=tile_offset*TILE_R*TILE_C;
    pd.A_all=(uint16_t*)aligned_alloc(64,std::max(A_size,(int64_t)1)*sizeof(uint16_t));
    memset(pd.A_all,0,A_size*sizeof(uint16_t));pd.amx_nnz=0;
    for(auto&pb:builds){
        uint16_t*base=pd.A_all+pb.panel.A_offset;
        for(auto&nz:pb.packed_nz)
            base[(int64_t)nz.tile_idx*TILE_R*TILE_C+nz.row*TILE_C+nz.tile_col]=nz.bf16_val;
        pd.amx_nnz+=pb.panel.total_deg;
        pd.panels.push_back(std::move(pb.panel));
    }
    for(int i=0;i<M;i++)if(!assigned[i])pd.avx_rows.push_back(i);
    printf("  Panels=%d Tiles=%ld AMX_NNZ=%.1f%%\n",(int)pd.panels.size(),tile_offset,100.0*pd.amx_nnz/csr.nnz);
    return pd;
}

// ============ KC-Fusion kernel (same as V12) ============
static inline void gather_fused_4kc(const uint16_t*B_bf16, const int*br, int kc_base,
    uint8_t*Bb0, uint8_t*Bb1, uint8_t*Bb2, uint8_t*Bb3) {
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        uint32_t*p0=(uint32_t*)(Bb0+p*64);
        uint32_t*p1=(uint32_t*)(Bb1+p*64);
        uint32_t*p2=(uint32_t*)(Bb2+p*64);
        uint32_t*p3=(uint32_t*)(Bb3+p*64);
        if(re>=0&&ro>=0){
            const uint16_t*be=&B_bf16[(int64_t)re*K+kc_base];
            const uint16_t*bo=&B_bf16[(int64_t)ro*K+kc_base];
            __m512i fe0=_mm512_loadu_si512(be);__m512i fe1=_mm512_loadu_si512(be+32);
            __m512i fo0=_mm512_loadu_si512(bo);__m512i fo1=_mm512_loadu_si512(bo+32);
            __m512i ie,io;
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe0));
            io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fo0));
            _mm512_store_si512((__m512i*)p0,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe0,1));
            io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fo0,1));
            _mm512_store_si512((__m512i*)p1,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe1));
            io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fo1));
            _mm512_store_si512((__m512i*)p2,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe1,1));
            io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fo1,1));
            _mm512_store_si512((__m512i*)p3,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
        } else if(re>=0){
            const uint16_t*be=&B_bf16[(int64_t)re*K+kc_base];
            __m512i fe0=_mm512_loadu_si512(be);__m512i fe1=_mm512_loadu_si512(be+32);
            _mm512_store_si512((__m512i*)p0,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe0)));
            _mm512_store_si512((__m512i*)p1,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe0,1)));
            _mm512_store_si512((__m512i*)p2,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe1)));
            _mm512_store_si512((__m512i*)p3,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe1,1)));
        } else {
            __m512i z=_mm512_setzero_si512();
            _mm512_store_si512((__m512i*)p0,z);_mm512_store_si512((__m512i*)p1,z);
            _mm512_store_si512((__m512i*)p2,z);_mm512_store_si512((__m512i*)p3,z);
        }
    }
}

void amx_kernel(const uint16_t*B_bf16, float*C, const PrecompData&pd){
    int np=(int)pd.panels.size();
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        tile_config_t cfg;memset(&cfg,0,64);cfg.palette_id=1;
        cfg.rows[0]=16;cfg.colsb[0]=KC*4;cfg.rows[1]=16;cfg.colsb[1]=64;
        cfg.rows[2]=16;cfg.colsb[2]=KC*4;cfg.rows[3]=16;cfg.colsb[3]=KC*4;
        cfg.rows[4]=16;cfg.colsb[4]=KC*4;cfg.rows[5]=16;cfg.colsb[5]=KC*4;
        _tile_loadconfig(&cfg);
        alignas(64) uint8_t Bb0[1024],Bb1[1024],Bb2[1024],Bb3[1024];
        alignas(64) float Cb0[16][KC],Cb1[16][KC],Cb2[16][KC],Cb3[16][KC];
        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){
            const Panel&P=pd.panels[pi];
            const uint16_t*preA=pd.A_all+P.A_offset;
            for(int pass=0;pass<2;pass++){
                int kc_base=pass*64;
                _tile_zero(0);_tile_zero(3);_tile_zero(4);_tile_zero(5);
                for(int t=0;t<P.ntiles;t++){
                    gather_fused_4kc(B_bf16,&P.b_rows[t*32],kc_base,Bb0,Bb1,Bb2,Bb3);
                    _tile_loadd(1,preA+(size_t)t*TILE_R*TILE_C,64);
                    _tile_loadd(2,Bb0,64);_tile_dpbf16ps(0,1,2);
                    _tile_loadd(2,Bb1,64);_tile_dpbf16ps(3,1,2);
                    _tile_loadd(2,Bb2,64);_tile_dpbf16ps(4,1,2);
                    _tile_loadd(2,Bb3,64);_tile_dpbf16ps(5,1,2);
                }
                _tile_stored(0,Cb0,KC*4);_tile_stored(3,Cb1,KC*4);
                _tile_stored(4,Cb2,KC*4);_tile_stored(5,Cb3,KC*4);
                for(int i=0;i<TILE_R;i++){
                    float*dst=C+(int64_t)P.rows[i]*K+kc_base;
                    for(int k=0;k<KC;k++)dst[k]=Cb0[i][k];
                    for(int k=0;k<KC;k++)dst[k+KC]=Cb1[i][k];
                    for(int k=0;k<KC;k++)dst[k+2*KC]=Cb2[i][k];
                    for(int k=0;k<KC;k++)dst[k+3*KC]=Cb3[i][k];
                }
            }
        }
        _tile_release();
    }
}

void avx512_all(const CSR&csr,const float*B,float*C){
    #pragma omp parallel for schedule(dynamic,256)
    for(int i=0;i<csr.M;i++){
        float*Cr=C+(int64_t)i*K;
        __m512 c[8]; for(int j=0;j<8;j++) c[j]=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            for(int j=0;j<8;j++) c[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),c[j]);
        }
        for(int j=0;j<8;j++) _mm512_storeu_ps(Cr+j*16,c[j]);
    }
}
void avx512_rows(const CSR&csr,const float*B,float*C,const std::vector<int>&rows){
    int nr=(int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx];float*Cr=C+(int64_t)i*K;
        __m512 c[8]; for(int j=0;j<8;j++) c[j]=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            for(int j=0;j<8;j++) c[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),c[j]);
        }
        for(int j=0;j<8;j++) _mm512_storeu_ps(Cr+j*16,c[j]);
    }
}

int main(int argc,char**argv){
    if(argc<2){printf("Usage: %s <mat> [thr]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl,0x1023,18);
    printf("=== V15 B-Reorder + Panel-Order K=128 ===\nthreads=%d\n\n",nt);

    // Load original matrix
    CSR csr_orig=read_csrbin(argv[1]);
    printf("M=%d N=%d NNZ=%ld avg=%.1f\n",csr_orig.M,csr_orig.N,csr_orig.nnz,(double)csr_orig.nnz/csr_orig.M);
    
    // Build B reorder permutation
    printf("\n[1] B-row reordering...\n");
    ReorderInfo ri = build_reorder(csr_orig);
    CSR csr_reord = remap_csr(csr_orig, ri);
    
    // Allocate B (original and reordered)
    int N = csr_orig.N;
    float*B_orig=(float*)aligned_alloc(64,(size_t)N*K*sizeof(float));
    uint16_t*Bbf_orig=(uint16_t*)aligned_alloc(64,(size_t)N*K*sizeof(uint16_t));
    srand(12345);
    for(int64_t i=0;i<(int64_t)N*K;i++){B_orig[i]=(rand()%200-100)/100.0f;Bbf_orig[i]=f32_to_bf16(B_orig[i]);}
    
    float*B_reord=(float*)aligned_alloc(64,(size_t)N*K*sizeof(float));
    uint16_t*Bbf_reord=(uint16_t*)aligned_alloc(64,(size_t)N*K*sizeof(uint16_t));
    reorder_B(B_orig, B_reord, Bbf_orig, Bbf_reord, ri, N);
    
    float*C=(float*)aligned_alloc(64,(size_t)csr_orig.M*K*sizeof(float));
    float*C_ref=(float*)aligned_alloc(64,(size_t)csr_orig.M*K*sizeof(float));
    
    // AVX baseline (on original data, for reference)
    printf("\n[2] AVX-512 baseline...\n");
    avx512_all(csr_orig,B_orig,C);
    double best_avx=1e9;
    for(int t=0;t<5;t++){
        auto T0=std::chrono::high_resolution_clock::now();
        avx512_all(csr_orig,B_orig,C);
        auto T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<best_avx)best_avx=ms;
    }
    printf("  AVX-512: %.2f ms\n",best_avx);
    memcpy(C_ref,C,(size_t)csr_orig.M*K*sizeof(float));

    // Also time AVX on reordered data (should be similar or slightly better)
    double best_avx_reord=1e9;
    for(int t=0;t<5;t++){
        auto T0=std::chrono::high_resolution_clock::now();
        avx512_all(csr_reord,B_reord,C);
        auto T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<best_avx_reord)best_avx_reord=ms;
    }
    printf("  AVX-512 (reord): %.2f ms\n",best_avx_reord);

    auto run_config = [&](const char*name, const CSR&csr, const uint16_t*Bbf, const float*B, bool panel_sort) {
        CSC csc=build_csc(csr);
        printf("\n[%s] building...\n",name);
        PrecompData pd=build_all(csr,csc,panel_sort);
        // warmup
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        amx_kernel(Bbf,C,pd); avx512_rows(csr,B,C,pd.avx_rows);
        double best=1e9,ba=1e9,bf=1e9;
        for(int t=0;t<5;t++){
            memset(C,0,(size_t)csr.M*K*sizeof(float));
            auto T0=std::chrono::high_resolution_clock::now();
            amx_kernel(Bbf,C,pd);
            auto T1=std::chrono::high_resolution_clock::now();
            avx512_rows(csr,B,C,pd.avx_rows);
            auto T2=std::chrono::high_resolution_clock::now();
            double a=std::chrono::duration<double,std::milli>(T1-T0).count();
            double f=std::chrono::duration<double,std::milli>(T2-T1).count();
            if(a+f<best){best=a+f;ba=a;bf=f;}
        }
        printf("  %-25s amx=%7.2f fb=%7.2f e2e=%7.2f (%.2fx)\n",name,ba,bf,best,best_avx/best);
        free(pd.A_all);
        return ba; // return AMX time
    };

    printf("\n===== TESTING 4 CONFIGURATIONS =====\n");
    double a0 = run_config("V12-baseline",      csr_orig,  Bbf_orig,  B_orig,  false);
    double a1 = run_config("V15a-B-reorder",     csr_reord, Bbf_reord, B_reord, false);
    double a2 = run_config("V15b-panel-order",   csr_orig,  Bbf_orig,  B_orig,  true);
    double a3 = run_config("V15c-both",          csr_reord, Bbf_reord, B_reord, true);

    printf("\n========== SUMMARY ==========\n");
    printf("AVX-512:              %.2f ms\n", best_avx);
    printf("AVX-512 (reord):      %.2f ms\n", best_avx_reord);
    printf("\nAMX kernel speedup (vs V12 baseline):\n");
    printf("  V12-baseline:       %.2f ms (1.000x)\n", a0);
    printf("  V15a-B-reorder:     %.2f ms (%.3fx)\n", a1, a0/a1);
    printf("  V15b-panel-order:   %.2f ms (%.3fx)\n", a2, a0/a2);
    printf("  V15c-both:          %.2f ms (%.3fx)\n", a3, a0/a3);

    free(B_orig);free(Bbf_orig);free(B_reord);free(Bbf_reord);free(C);free(C_ref);
    printf("\n=== Done ===\n");return 0;
}
