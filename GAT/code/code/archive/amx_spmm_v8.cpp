/**
 * V8: 交叉调度 — AMX panel 和孤儿行按行号交叉执行
 *
 * 核心改动:
 *   1. 把 AMX panels 和 orphan rows 统一编排成 "work chunks"
 *   2. 按起始行号排序, 相邻 chunk 共享 B cache
 *   3. 单个 parallel region 内交替执行 AMX 和 AVX
 *   4. 保留: 连续A, BF16 B, 多轮分组
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <omp.h>

constexpr int TILE_R = 16;
constexpr int TILE_C = 32;
constexpr int K      = 32;
constexpr int KC     = 16;
constexpr float FILL_THR = 0.0625f;
constexpr int MAX_NTILES = 64;

typedef struct __attribute__((aligned(64))) {
    uint8_t palette_id, start_row, reserved0[14];
    uint16_t colsb[16]; uint8_t rows[16];
} tile_config_t;

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4);
    u += 0x7FFF + ((u >> 16) & 1);
    return (uint16_t)(u >> 16);
}

struct CSR {
    std::vector<uint32_t> indptr, indices;
    std::vector<float> values;
    int M, N; int64_t nnz;
};
CSR read_csrbin(const char* path) {
    CSR c; FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERR: %s\n", path); exit(1); }
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr,4,3,f); fread(dims,8,3,f);
    c.M=(int)dims[0]; c.N=(int)dims[1]; c.nnz=(int64_t)dims[2];
    c.indptr.resize(c.M+1); c.indices.resize(c.nnz); c.values.resize(c.nnz);
    fread(c.indptr.data(),4,c.M+1,f);
    fread(c.indices.data(),4,c.nnz,f);
    fread(c.values.data(),4,c.nnz,f);
    fclose(f); return c;
}
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

struct PackedNZ { uint8_t row, tile_idx; uint16_t tile_col, bf16_val; };

struct Panel {
    int rows[TILE_R];
    int nrows, U, ntiles, total_deg;
    int64_t A_offset;
    int min_row;  // ★ 用于排序
    std::vector<int> b_rows;
};

// ★ 统一工作单元
struct WorkChunk {
    int type;  // 0 = AMX panel, 1 = AVX orphan group
    int idx;   // panel index or orphan group index
    int sort_key;  // min row number for sorting
};

struct PrecompData {
    std::vector<Panel> panels;
    std::vector<int> avx_rows;
    uint16_t* A_all;
    int64_t total_tiles, amx_nnz;

    // ★ 孤儿行分组 (每组 ~64 行, 按行号连续)
    std::vector<std::pair<int,int>> orphan_groups; // [start_idx, count] into avx_rows

    // ★ 排序后的工作队列
    std::vector<WorkChunk> work_queue;
};

bool try_build_panel(int col, int min_rows,
    const CSR& csr, const CSC& csc,
    const std::vector<int>& deg,
    std::vector<bool>& assigned,
    std::vector<int>& col_map,
    Panel& panel, std::vector<PackedNZ>& packed_nz,
    int64_t& tile_offset)
{
    std::vector<int> avail;
    avail.reserve(std::min((int)csc.col_counts[col], TILE_R*4));
    for(int64_t p=csc.col_ptr[col];p<csc.col_ptr[col+1];p++){
        int r=csc.col_rows[p];
        if(!assigned[r]){
            avail.push_back(r);
            if((int)avail.size()>=TILE_R*4) break;
        }
    }
    if((int)avail.size()<min_rows) return false;

    int nrows=std::min((int)avail.size(),TILE_R);
    std::vector<int> cols; int td=0;
    for(int i=0;i<nrows;i++){
        int r=avail[i]; td+=deg[r];
        for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++)
            cols.push_back(csr.indices[p]);
    }
    std::sort(cols.begin(),cols.end());
    cols.erase(std::unique(cols.begin(),cols.end()),cols.end());
    int U=(int)cols.size(),nt=(U+TILE_C-1)/TILE_C;
    float fill=(float)td/(TILE_R*TILE_C*nt);
    if(fill<FILL_THR||nt>MAX_NTILES) return false;

    for(int j=0;j<(int)cols.size();j++) col_map[cols[j]]=j;
    panel.nrows=nrows; panel.U=U; panel.ntiles=nt; panel.total_deg=td;
    panel.A_offset=tile_offset*TILE_R*TILE_C;
    panel.min_row=avail[0];
    for(int i=1;i<nrows;i++) panel.min_row=std::min(panel.min_row,avail[i]);

    for(int i=0;i<TILE_R;i++) panel.rows[i]=(i<nrows)?avail[i]:avail[0];

    packed_nz.clear(); packed_nz.reserve(td);
    for(int i=0;i<nrows;i++){
        int r=avail[i];
        for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){
            int j=col_map[csr.indices[p]];
            packed_nz.push_back({(uint8_t)i,(uint8_t)(j/TILE_C),
                (uint16_t)(j%TILE_C),f32_to_bf16(csr.values[p])});
        }
    }
    panel.b_rows.assign(nt*32,-1);
    for(int t=0;t<nt;t++) for(int p=0;p<16;p++){
        int je=t*TILE_C+p*2,jo=je+1;
        if(je<U) panel.b_rows[t*32+p*2]=cols[je];
        if(jo<U) panel.b_rows[t*32+p*2+1]=cols[jo];
    }
    for(int j=0;j<(int)cols.size();j++) col_map[cols[j]]=-1;
    for(int i=0;i<nrows;i++) assigned[avail[i]]=true;
    tile_offset+=nt;
    return true;
}

PrecompData build_all(const CSR& csr, const CSC& csc) {
    PrecompData pd;
    int M=csr.M, N=csr.N;
    std::vector<int> deg(M);
    for(int i=0;i<M;i++) deg[i]=csr.indptr[i+1]-csr.indptr[i];
    std::vector<int> col_order;
    for(int j=0;j<N;j++) if(csc.col_counts[j]>=2) col_order.push_back(j);
    std::sort(col_order.begin(),col_order.end(),
        [&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});
    std::vector<bool> assigned(M,false);
    std::vector<int> col_map(N,-1);

    struct PB { Panel panel; std::vector<PackedNZ> packed_nz; };
    std::vector<PB> builds;
    int64_t tile_offset=0;
    int r1=0,r2=0,r3=0;

    for(int col:col_order){
        if(csc.col_counts[col]<TILE_R) break;
        PB pb;
        if(try_build_panel(col,TILE_R,csr,csc,deg,assigned,col_map,
            pb.panel,pb.packed_nz,tile_offset)){ builds.push_back(std::move(pb)); r1++; }
    }
    for(int col:col_order){
        if(csc.col_counts[col]<8) break;
        PB pb;
        if(try_build_panel(col,8,csr,csc,deg,assigned,col_map,
            pb.panel,pb.packed_nz,tile_offset)){ builds.push_back(std::move(pb)); r2++; }
    }
    for(int col:col_order){
        if(csc.col_counts[col]<4) break;
        PB pb;
        if(try_build_panel(col,4,csr,csc,deg,assigned,col_map,
            pb.panel,pb.packed_nz,tile_offset)){ builds.push_back(std::move(pb)); r3++; }
    }
    printf("    Round1(16+): %d  Round2(8+): %d  Round3(4+): %d\n",r1,r2,r3);

    // 连续 A
    pd.total_tiles=tile_offset;
    int64_t A_size=tile_offset*TILE_R*TILE_C;
    pd.A_all=(uint16_t*)aligned_alloc(64,std::max(A_size,(int64_t)1)*sizeof(uint16_t));
    memset(pd.A_all,0,A_size*sizeof(uint16_t));
    pd.amx_nnz=0;
    for(auto&pb:builds){
        uint16_t*base=pd.A_all+pb.panel.A_offset;
        for(auto&nz:pb.packed_nz)
            base[(int64_t)nz.tile_idx*TILE_R*TILE_C+nz.row*TILE_C+nz.tile_col]=nz.bf16_val;
        pd.amx_nnz+=pb.panel.total_deg;
        pd.panels.push_back(std::move(pb.panel));
    }
    for(int i=0;i<M;i++) if(!assigned[i]) pd.avx_rows.push_back(i);
    // avx_rows 已经按行号递增（因为遍历顺序）

    printf("    Total tiles: %ld (%.1f MB)\n",tile_offset,(double)A_size*2/1e6);

    // ★ 孤儿行分组: 每组 64 行
    constexpr int ORPHAN_GROUP_SIZE = 64;
    int nr=(int)pd.avx_rows.size();
    for(int s=0;s<nr;s+=ORPHAN_GROUP_SIZE){
        int cnt=std::min(ORPHAN_GROUP_SIZE, nr-s);
        pd.orphan_groups.push_back({s, cnt});
    }

    // ★ 构建排序工作队列
    for(int i=0;i<(int)pd.panels.size();i++){
        pd.work_queue.push_back({0, i, pd.panels[i].min_row});
    }
    for(int i=0;i<(int)pd.orphan_groups.size();i++){
        int first_row = pd.avx_rows[pd.orphan_groups[i].first];
        pd.work_queue.push_back({1, i, first_row});
    }
    std::sort(pd.work_queue.begin(), pd.work_queue.end(),
        [](const WorkChunk& a, const WorkChunk& b){ return a.sort_key < b.sort_key; });

    printf("    Work chunks: %d (%d AMX + %d orphan groups)\n",
        (int)pd.work_queue.size(), (int)pd.panels.size(), (int)pd.orphan_groups.size());

    return pd;
}

// ==================== AVX-512 standalone ====================
void avx512_all(const CSR& csr, const float* B, float* C) {
    int nr=csr.M;
    #pragma omp parallel for schedule(dynamic,256)
    for(int i=0;i<nr;i++){
        float*Cr=C+(int64_t)i*K;
        __m512 c0=_mm512_setzero_ps(),c1=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            c0=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br),c0);
            c1=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+16),c1);
        }
        _mm512_storeu_ps(Cr,c0); _mm512_storeu_ps(Cr+16,c1);
    }
}

// ==================== V8: 交叉调度 ====================
void amx_v8_interleaved(const CSR& csr, const float* B,
    const uint16_t* B_bf16, float* C, const PrecompData& pd)
{
    int nw=(int)pd.work_queue.size();
    const uint16_t* A_all=pd.A_all;

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        tile_config_t cfg; memset(&cfg,0,64);
        cfg.palette_id=1;
        cfg.rows[0]=16;cfg.colsb[0]=KC*4;
        cfg.rows[1]=16;cfg.colsb[1]=64;
        cfg.rows[2]=16;cfg.colsb[2]=KC*4;
        _tile_loadconfig(&cfg);

        alignas(64) uint8_t Bb[16*64];
        alignas(64) float Cb[16][KC];

        #pragma omp for schedule(dynamic,4)
        for(int wi=0;wi<nw;wi++){
            const WorkChunk& wc=pd.work_queue[wi];

            if(wc.type==0){
                // ===== AMX panel =====
                const Panel& P=pd.panels[wc.idx];
                const uint16_t* preA=A_all+P.A_offset;
                int nrows=P.nrows;

                for(int kc=0;kc<K;kc+=KC){
                    _tile_zero(0);
                    for(int t=0;t<P.ntiles;t++){
                        const int*br=&P.b_rows[t*32];
                        for(int p=0;p<16;p++){
                            int re=br[p*2],ro=br[p*2+1];
                            uint32_t*pr=(uint32_t*)(Bb+p*64);
                            if(re>=0&&ro>=0){
                                __m256i ve=_mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)re*K+kc]);
                                __m256i vo=_mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)ro*K+kc]);
                                __m512i ie=_mm512_cvtepu16_epi32(ve);
                                __m512i io=_mm512_cvtepu16_epi32(vo);
                                _mm512_store_si512((__m512i*)pr,
                                    _mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
                            } else if(re>=0){
                                __m256i ve=_mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)re*K+kc]);
                                _mm512_store_si512((__m512i*)pr,_mm512_cvtepu16_epi32(ve));
                            } else {
                                _mm512_store_si512((__m512i*)pr,_mm512_setzero_si512());
                            }
                        }
                        _tile_loadd(1, preA+(size_t)t*TILE_R*TILE_C, 64);
                        _tile_loadd(2, Bb, 64);
                        _tile_dpbf16ps(0,1,2);
                    }
                    _tile_stored(0, Cb, KC*4);
                    for(int i=0;i<nrows;i++){
                        float*dst=C+(int64_t)P.rows[i]*K+kc;
                        for(int k=0;k<KC;k++) dst[k]=Cb[i][k];
                    }
                }
            } else {
                // ===== AVX orphan group =====
                auto [start, cnt] = pd.orphan_groups[wc.idx];
                for(int g=0;g<cnt;g++){
                    int i=pd.avx_rows[start+g];
                    float*Cr=C+(int64_t)i*K;
                    __m512 c0=_mm512_setzero_ps(),c1=_mm512_setzero_ps();
                    for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
                        __m512 a=_mm512_set1_ps(csr.values[p]);
                        const float*Br=B+(int64_t)csr.indices[p]*K;
                        c0=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br),c0);
                        c1=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+16),c1);
                    }
                    _mm512_storeu_ps(Cr,c0); _mm512_storeu_ps(Cr+16,c1);
                }
            }
        }
        _tile_release();
    }
}

// ==================== Naive ====================
void naive_spmm(const CSR& csr, const float* B, float* C) {
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    for(int i=0;i<csr.M;i++)
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            float v=csr.values[p];
            const float*Br=B+(int64_t)csr.indices[p]*K;
            float*Cr=C+(int64_t)i*K;
            for(int k=0;k<K;k++) Cr[k]+=v*Br[k];
        }
}

int main(int argc, char** argv) {
    if(argc<2){printf("Usage: %s <mat> [threads]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl,0x1023,18);

    printf("=== AMX V8 (Interleaved Scheduling) ===\n");
    printf("K=%d threads=%d\n\n", K, nt);

    CSR csr=read_csrbin(argv[1]);
    printf("[1] M=%d N=%d NNZ=%ld avg=%.1f\n", csr.M,csr.N,csr.nnz,(double)csr.nnz/csr.M);
    CSC csc=build_csc(csr);

    printf("[2] Build...\n");
    auto T0=std::chrono::high_resolution_clock::now();
    PrecompData pd=build_all(csr,csc);
    auto T1=std::chrono::high_resolution_clock::now();
    printf("    AMX: %d panels, %ld NNZ (%.1f%%)\n",
        (int)pd.panels.size(),pd.amx_nnz,100.0*pd.amx_nnz/csr.nnz);
    printf("    AVX: %d rows, %ld NNZ (%.1f%%)\n",
        (int)pd.avx_rows.size(),csr.nnz-pd.amx_nnz,100.0*(csr.nnz-pd.amx_nnz)/csr.nnz);
    printf("    Prep: %.2fs\n\n",std::chrono::duration<double>(T1-T0).count());

    float* B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;
    uint16_t* B_bf16=(uint16_t*)aligned_alloc(64,(size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B_bf16[i]=f32_to_bf16(B[i]);

    float* C_ref=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    printf("[3] Naive (1T)...\n");
    T0=std::chrono::high_resolution_clock::now();
    naive_spmm(csr,B,C_ref);
    T1=std::chrono::high_resolution_clock::now();
    double naive_ms=std::chrono::duration<double,std::milli>(T1-T0).count();
    printf("    %.1fms\n\n",naive_ms);

    float* C=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));

    // Pure AVX-512
    printf("[4] Pure AVX-512 (%dT)...\n",nt);
    avx512_all(csr,B,C); // warmup
    double best_avx=1e9;
    for(int t=0;t<5;t++){
        T0=std::chrono::high_resolution_clock::now();
        avx512_all(csr,B,C);
        T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<best_avx) best_avx=ms;
    }
    printf("    Best: %.2f ms\n\n",best_avx);

    // V8 Interleaved
    printf("[5] AMX V8 Interleaved (%dT)...\n",nt);
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_v8_interleaved(csr,B,B_bf16,C,pd); // warmup

    double best_v8=1e9;
    for(int t=0;t<5;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        T0=std::chrono::high_resolution_clock::now();
        amx_v8_interleaved(csr,B,B_bf16,C,pd);
        T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        printf("    [%d] %.2f ms\n",t,ms);
        if(ms<best_v8) best_v8=ms;
    }
    printf("    Best: %.2f ms\n",best_v8);
    printf("    ★ vs Naive(1T): %.2fx\n",naive_ms/best_v8);
    printf("    ★ vs AVX-512(%dT): %.2fx\n\n",nt,best_avx/best_v8);

    // Correctness
    printf("[6] Correctness...\n");
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_v8_interleaved(csr,B,B_bf16,C,pd);
    int bad=0; float max_abs=0;
    for(int64_t i=0;i<(int64_t)csr.M*K;i++){
        float ae=fabsf(C_ref[i]-C[i]);
        if(ae>max_abs)max_abs=ae;
        float re=(fabsf(C_ref[i])>1e-6f)?ae/fabsf(C_ref[i]):ae;
        if(re>0.05f&&fabsf(C_ref[i])>0.01f) bad++;
    }
    printf("    Max abs: %.3e  Bad: %d (%.4f%%)\n",max_abs,bad,100.0*bad/(csr.M*K));
    printf("    %s\n",100.0*bad/(csr.M*K)<1.0?"✅ OK":"❌ FAIL");

    printf("\n============ SUMMARY ============\n");
    printf("Naive (1T):          %.1f ms\n",naive_ms);
    printf("Pure AVX-512 (%dT):  %.2f ms\n",nt,best_avx);
    printf("AMX V8 (%dT):        %.2f ms (%.2fx AVX)\n",nt,best_v8,best_avx/best_v8);

    free(B);free(B_bf16);free(C);free(C_ref);free(pd.A_all);
    printf("\n=== Done ===\n");
    return 0;
}
