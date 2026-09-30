#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <immintrin.h>
#include <mkl.h>
#include <omp.h>

namespace gat {
using BF16 = MKL_BF16;
using Clock = std::chrono::steady_clock;
inline double seconds(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double>(b-a).count();
}
#ifdef GAT_PROFILE
inline auto tick() { return Clock::now(); }
inline constexpr bool profiling=true;
#else
inline Clock::time_point tick() { return {}; }
inline constexpr bool profiling=false;
#endif
size_t checked(uint64_t a,uint64_t b);
void require_finite(const std::vector<float>& a,const std::string& name);
struct Graph {
    uint64_t n=0,e=0;
    int din=0,classes=0;
    std::vector<uint64_t> row;
    std::vector<uint32_t> col;
    std::vector<float> x;
    void validate() const;
};
Graph load_graph(const std::string& path);
void save_graph(const Graph&,const std::string&);
struct Param {
    int in=0,heads=0,dim=0;
    std::vector<float> w,al,ar; // W[in][heads*dim]; heads are independent.
    int width() const { return heads*dim; }
    void validate() const;
};
using Model=std::vector<Param>;
Model random_model(int din,int classes,int seed);
Model load_model(const std::string& path,int din,int classes);
void save_model(const Model&,const std::string&);
inline float leak(float x) { return x>=0?x:0.2f*x; }
inline float elu(float x) { return x>=0?x:std::expm1(x); }
inline __m512 leaky(__m512 x) {
    return _mm512_mask_mul_ps(x,_mm512_cmp_ps_mask(x,_mm512_setzero_ps(),_CMP_LT_OQ),x,_mm512_set1_ps(0.2f));
}
inline __m512 activation(__m512 x) {
    auto mask=_mm512_cmp_ps_mask(x,_mm512_setzero_ps(),_CMP_LT_OQ);
    // Positive lanes must not evaluate exp(large_positive).
    __m512 neg=_mm512_maskz_mov_ps(mask,x);
    return _mm512_mask_mov_ps(x,mask,_mm512_sub_ps(_mm512_exp_ps(neg),_mm512_set1_ps(1)));
}
inline __mmask16 tail_mask(int count) { return count>=16?__mmask16(0xffff):__mmask16((1u<<count)-1); }
BF16 rne(float x);
float expand(BF16 x);
void convert(const float* input,BF16* output,size_t count);
std::vector<float> quantized(const std::vector<float>&);
struct Times {
    double convert=0,projection=0,lr=0,max_prescan=0,aggregate=0,normalize=0,activation=0,total=0;
    // Profile-only worker sums, nested within aggregate; not additive wall times.
    double score_exp_worker=0,gather_weight_convert_worker=0,tile_load_worker=0;
    double gather_load_worker=0,p_times_x_convert_worker=0,weighted_spmm_worker=0;
    double amx_compute_worker=0,tile_store_worker=0,normalize_worker=0,activation_worker=0;
    double tile_config_worker=0,scheduling_worker=0,output_write_worker=0;
    uint64_t neighbor_steps=0,executed_fma=0;
};
struct Trace {
    std::vector<float> z,left,right,max,den,score,p,alpha;
};
struct Prepared {
    std::vector<BF16> w; // oneMKL all-head matrix
    std::vector<float> blr; // FP32 [D][2K], computed from master W/a
    std::vector<BF16> blr_bf16;
    std::vector<float> blr_dot; // [2K][D] contiguous FP32 vectors for AVX dots
    std::vector<BF16> packed; // [head][kb][output block][k-pair][column*2]
    int padded_in=0,output_blocks=0;
    double weight_prepare_s=0;
};
Prepared prepare(const Param&,bool attention=true,bool tfs_pack=true,bool bf16_weights=true);
struct Schedule {
    std::vector<uint32_t> perm;
    uint64_t neighbor_steps=0; // sum max degree over consecutive TR=16 sorted tiles
    double degree_sort_s=0;
};
Schedule degree_schedule(const Graph&);
struct Workspace {
    std::vector<BF16> xbf;
    std::vector<float> z,left,right,max,den,out,lr;
    std::vector<float> score,p,alpha; // R0 only
    std::vector<BF16> thread_hbuf;
    std::vector<float> thread_cbuf;
    std::vector<float> lr_sgemm_weights,lr_sgemm_tmp;
    void allocate(const Graph&,const Param&,const std::string& path);
    void prepare_lr_sgemm(const Graph&,const Param&);
    size_t bytes() const;
};
void projection(const Graph&,const Param&,const Prepared&,const std::vector<float>&,Workspace&,bool bf16);
void lr_from_z(const Graph&,const Param&,Workspace&,bool scalar=false);
void lr_from_z_mkl(const Graph&,const Param&,Workspace&);
void lr_reordered(const Graph&,const Param&,const Prepared&,const std::vector<float>&,Workspace&,const std::string& policy);
void max_prescan(const Graph&,const Param&,Workspace&);
void standard_aggregate(const Graph&,const Param&,Workspace&,const float* fixed_p=nullptr,const float* fixed_den=nullptr,Times* times=nullptr);
void finish(const Graph&,const Param&,Workspace&,bool hidden,Times&);
void ref_layer(const Graph&,const Param&,const std::vector<float>&,Workspace&,bool hidden,Times&);
void standard_layer(const Graph&,const Param&,const Prepared&,const std::vector<float>&,Workspace&,bool hidden,Times&,bool fp32=false,const std::string& lr_policy="z_avx");
void tfs_aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,Workspace&,int panel,Times&,const float* fixed_p=nullptr,const float* fixed_den=nullptr);
void tfs_layer(const Graph&,const Param&,const Prepared&,const Schedule&,const std::vector<float>&,Workspace&,bool hidden,int panel,Times&,const std::string& lr_policy);
Trace capture(const Workspace&);
struct Error {
    double max_abs=0,mean_abs=0,relative_l2=0,rmse=0,p50=0,p90=0,p99=0;
    bool finite=true;
};
Error errors(const std::vector<float>& ref,const std::vector<float>& actual);
void print_error(const std::string& path,int layer,const std::string& stage,const Error&);
void print_times(const std::string& path,const std::string& mode,int layer,int rep,const Graph&,const Param&,const Workspace&,const Times&,double static_s,int panel);
struct Fixture {
    std::vector<float> input,p,den;
};
void save_fixture(const Graph&,const Param&,const Fixture&,const std::string&);
Fixture load_fixture(const Graph&,const Param&,const std::string&);
}
