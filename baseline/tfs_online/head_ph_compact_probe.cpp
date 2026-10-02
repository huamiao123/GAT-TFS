#include "head_ph_compact_probe.hpp"
#include <array>
#include <atomic>
#include <iomanip>
#include <iostream>
#include <sys/syscall.h>
#include <unistd.h>

namespace gat::icpp_ph_compact {
namespace {
constexpr int D=256,K=8,B=32,configured_rows=8,threads=16;
constexpr size_t max_samples=512;
enum class Mode {AVX=0,High=1,HighLow=2};
const char* name(Mode mode) {
    return mode==Mode::AVX?"AVX_SHARED_H_FP32_PH":
           mode==Mode::High?"AMX_PHI_BF16_PH_COMPACT8":"AMX_PHI_PLO_BF16_PH_COMPACT8";
}
struct alignas(64) Config {
    uint8_t palette=1,start=0,reserved[14]={};
    uint16_t cols[16]={};uint8_t rows[16]={};
    Config(){for(int t:{0,4,6}){cols[t]=64;rows[t]=8;}cols[5]=64;rows[5]=16;}
};
static_assert(sizeof(Config)==64);
struct alignas(64) Scratch {
    alignas(64) float probability[configured_rows*B]={};
    alignas(64) BF16 phi[configured_rows*B]={},plo[configured_rows*B]={};
    alignas(64) BF16 hrow[B*D]={},hpacked[(D/16)*512]={};
    alignas(64) float cbuf[configured_rows*16]={},den[16]={};
    uint32_t source[B]={};
};
struct Output {
    std::vector<float> value,den;
    void allocate(size_t samples){value.resize(samples*K*D);den.resize(samples*K);}
};
struct Stages {
    double score_exp=0,h_gather_pack=0,p_pack=0,ph_compute_scatter=0,amx_init=0;
};
struct Run {
    double wall=0;Stages worker;
    int actual_threads=0;
};

// A single shared generator ensures all three methods consume exactly the
// same FP32 exp/denominator definition. This is one stable first-neighbor-block
// attention evaluation, not a separate softmax proposal or a full-row output.
__attribute__((noinline)) void make_probability(const Graph& g,const Param& p,
                                               const gat::Workspace& base,
                                               uint32_t row,Scratch& w) {
    std::fill(w.probability,w.probability+configured_rows*B,0.f);
    alignas(64) float score[B][16];
    __m512 left=_mm512_maskz_loadu_ps(__mmask16(0xff),base.left.data()+size_t(row)*p.heads);
    __m512 maximum=_mm512_set1_ps(-std::numeric_limits<float>::infinity());
    for(int b=0;b<B;++b) {
        w.source[b]=g.col[g.row[row]+uint64_t(b)];
        __m512 right=_mm512_maskz_loadu_ps(__mmask16(0xff),
                                        base.right.data()+size_t(w.source[b])*p.heads);
        __m512 e=leaky(_mm512_add_ps(left,right));
        e=_mm512_mask_mov_ps(_mm512_set1_ps(-std::numeric_limits<float>::infinity()),__mmask16(0xff),e);
        _mm512_store_ps(score[b],e);maximum=_mm512_max_ps(maximum,e);
    }
    __m512 den=_mm512_setzero_ps();
    for(int b=0;b<B;++b) {
        __m512 shifted=_mm512_maskz_sub_ps(__mmask16(0xff),_mm512_load_ps(score[b]),maximum);
        __m512 weight=_mm512_maskz_mov_ps(__mmask16(0xff),_mm512_exp_ps(shifted));
        alignas(64) float value[16];_mm512_store_ps(value,weight);
        for(int h=0;h<K;++h)w.probability[h*B+b]=value[h];
        den=_mm512_add_ps(den,weight);
    }
    _mm512_store_ps(w.den,den);
}
inline __m512 expand_vector(__m256i v) {
    return _mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(v),16));
}
template<bool Low> void pack_probability(Scratch& w) {
    for(int at=0;at<configured_rows*B;at+=16) {
        __m512 p=_mm512_load_ps(w.probability+at);
        __m256bh hi=_mm512_cvtneps_pbh(p);
        _mm256_store_si256(reinterpret_cast<__m256i*>(w.phi+at),(__m256i)hi);
        if constexpr(Low) {
            __m512 residual=_mm512_sub_ps(p,expand_vector((__m256i)hi));
            __m256bh lo=_mm512_cvtneps_pbh(residual);
            _mm256_store_si256(reinterpret_cast<__m256i*>(w.plo+at),(__m256i)lo);
        }
    }
}
void gather_pack_H(const gat::Workspace& base,Scratch& w) {
    // Native BF16 pair interleave, directly from the two source rows.
    // No float conversion and no intermediate row-major H copy.
    for(int kp=0;kp<B/2;++kp) {
        const BF16* a=base.xbf.data()+size_t(w.source[2*kp])*D;
        const BF16* b=base.xbf.data()+size_t(w.source[2*kp+1])*D;
        for(int ob=0;ob<D/16;++ob) {
            __m256i raw_a=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a+ob*16));
            __m256i raw_b=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b+ob*16));
            __m512i lo=_mm512_cvtepu16_epi32(raw_a);
            __m512i hi=_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw_b),16);
            _mm512_store_si512(w.hpacked+size_t(ob)*512+kp*32,_mm512_or_si512(lo,hi));
        }
    }
}

void avx_PH(const gat::Workspace& base,const Scratch& w,float* output) {
    for(int feature=0;feature<D;feature+=16) {
        std::array<__m512,K> accumulator;
        #pragma unroll
        for(int h=0;h<K;++h)accumulator[h]=_mm512_setzero_ps();
        for(int b=0;b<B;++b) {
            const BF16* src=base.xbf.data()+size_t(w.source[b])*D+feature;
            __m512 value=expand_vector(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src)));
            #pragma unroll
            for(int h=0;h<K;++h)
                accumulator[h]=_mm512_fmadd_ps(_mm512_set1_ps(w.probability[h*B+b]),value,accumulator[h]);
        }
        #pragma unroll
        for(int h=0;h<K;++h)_mm512_storeu_ps(output+h*D+feature,accumulator[h]);
    }
}
template<bool Low> void amx_PH(Scratch& w,float* output) {
    _tile_loadd(4,w.phi,B*sizeof(BF16));
    if constexpr(Low)_tile_loadd(6,w.plo,B*sizeof(BF16));
    for(int ob=0;ob<D/16;++ob) {
        _tile_zero(0);
        _tile_loadd(5,w.hpacked+size_t(ob)*512,64);
        _tile_dpbf16ps(0,4,5);
        if constexpr(Low)_tile_dpbf16ps(0,6,5);
        _tile_stored(0,w.cbuf,64);
        for(int h=0;h<K;++h)
            std::memcpy(output+h*D+ob*16,w.cbuf+h*16,16*sizeof(float));
    }
}

Run execute(Mode mode,const Graph& g,const Param& p,const gat::Workspace& base,
            const std::vector<uint32_t>& sample,std::vector<Scratch>& scratch,
            Output& output) {
    std::array<Stages,threads> worker{};
    Run result;std::atomic<bool> failed{false};
    auto begin=Clock::now();
    #pragma omp parallel num_threads(threads)
    {
        int thread=omp_get_thread_num();Scratch& w=scratch[size_t(thread)];
        auto& stage=worker[size_t(thread)];
        #pragma omp single
        {result.actual_threads=omp_get_num_threads();}
        if(mode!=Mode::AVX) {
            auto a=Clock::now();
            if(syscall(SYS_arch_prctl,0x1023,18)!=0)failed.store(true);
            #pragma omp barrier
            if(!failed.load()){Config cfg;_tile_loadconfig(&cfg);}
            stage.amx_init=seconds(a,Clock::now());
        }
        if(!failed.load()) {
            #pragma omp for schedule(static)
            for(size_t at=0;at<sample.size();++at) {
                auto a=Clock::now();make_probability(g,p,base,sample[at],w);
                stage.score_exp+=seconds(a,Clock::now());
                std::memcpy(output.den.data()+at*K,w.den,K*sizeof(float));
                if(mode==Mode::AVX) {
                    a=Clock::now();avx_PH(base,w,output.value.data()+at*K*D);
                    stage.ph_compute_scatter+=seconds(a,Clock::now());
                } else {
                    a=Clock::now();gather_pack_H(base,w);
                    stage.h_gather_pack+=seconds(a,Clock::now());
                    a=Clock::now();
                    if(mode==Mode::High)pack_probability<false>(w);
                    else pack_probability<true>(w);
                    stage.p_pack+=seconds(a,Clock::now());
                    a=Clock::now();
                    if(mode==Mode::High)amx_PH<false>(w,output.value.data()+at*K*D);
                    else amx_PH<true>(w,output.value.data()+at*K*D);
                    stage.ph_compute_scatter+=seconds(a,Clock::now());
                }
            }
            if(mode!=Mode::AVX)_tile_release();
        }
    }
    result.wall=seconds(begin,Clock::now());
    if(failed.load())throw std::runtime_error("head PH probe AMX permission failed");
    if(result.actual_threads!=threads)throw std::runtime_error("head PH probe requires 16 actual OpenMP threads");
    for(const auto& s:worker) {
        result.worker.score_exp+=s.score_exp;result.worker.h_gather_pack+=s.h_gather_pack;
        result.worker.p_pack+=s.p_pack;result.worker.ph_compute_scatter+=s.ph_compute_scatter;
        result.worker.amx_init+=s.amx_init;
    }
    return result;
}

std::vector<uint32_t> select_samples(const Graph& g,const Schedule& schedule,uint64_t& eligible) {
    eligible=0;
    for(uint32_t row:schedule.perm)if(g.row[row+1]-g.row[row]>=B)++eligible;
    size_t count=size_t(std::min<uint64_t>(max_samples,eligible));
    std::vector<uint32_t> selected;selected.reserve(count);
    if(!count)return selected;
    uint64_t rank=0;size_t next=0;
    for(uint32_t row:schedule.perm)if(g.row[row+1]-g.row[row]>=B) {
        uint64_t target=count==1?0:uint64_t(next)*(eligible-1)/(count-1);
        if(rank==target){selected.push_back(row);++next;if(next==count)break;}
        ++rank;
    }
    if(selected.size()!=count)throw std::runtime_error("head PH sample selection mismatch");
    return selected;
}
double operand(BF16 b) {
    // TDPBF16PS treats BF16 subnormal inputs as signed zero independently of
    // MXCSR. Native H and both packed P components are interpreted accordingly.
    return (uint16_t(b)&0x7f80u)==0?0.0:double(expand(b));
}
struct Oracle {
    std::array<std::vector<double>,3> value,envelope;
    uint64_t H_subnormal_operands=0,P_subnormal_components=0;
};
Oracle scalar_oracle(const Graph& g,const Param& p,const gat::Workspace& base,
                     const std::vector<uint32_t>& sample,const Output& expected) {
    Oracle o;size_t size=sample.size()*K*D;
    for(int mode=0;mode<3;++mode){o.value[mode].resize(size);o.envelope[mode].resize(size);}
    Scratch w;
    for(size_t at=0;at<sample.size();++at) {
        make_probability(g,p,base,sample[at],w);pack_probability<true>(w);
        if(std::memcmp(w.den,expected.den.data()+at*K,K*sizeof(float)))
            throw std::runtime_error("head PH probability replay denominator changed");
        for(int b=0;b<B;++b)for(int feature=0;feature<D;++feature) {
            BF16 bH=base.xbf[size_t(w.source[b])*D+feature];
            if((uint16_t(bH)&0x7f80u)==0 && (uint16_t(bH)&0x7fu)!=0)++o.H_subnormal_operands;
        }
        for(int h=0;h<K;++h) {
            for(int b=0;b<B;++b)for(BF16 component:{w.phi[h*B+b],w.plo[h*B+b]})
                if((uint16_t(component)&0x7f80u)==0 && (uint16_t(component)&0x7fu)!=0)++o.P_subnormal_components;
            for(int feature=0;feature<D;++feature) {
                double exact=0,high=0,low=0,env_exact=0,env_high=0,env_low=0;
                for(int b=0;b<B;++b) {
                    BF16 bH=base.xbf[size_t(w.source[b])*D+feature];
                    double original_H=double(expand(bH)),amx_H=operand(bH);
                    double product=double(w.probability[h*B+b])*original_H;
                    double hi=operand(w.phi[h*B+b])*amx_H;
                    double lo=operand(w.plo[h*B+b])*amx_H;
                    exact+=product;high+=hi;low+=lo;
                    env_exact+=std::abs(product);env_high+=std::abs(hi);env_low+=std::abs(lo);
                }
                size_t idx=at*K*D+h*D+feature;
                o.value[0][idx]=exact;o.value[1][idx]=high;o.value[2][idx]=high+low;
                o.envelope[0][idx]=env_exact;o.envelope[1][idx]=env_high;
                o.envelope[2][idx]=env_high+env_low;
            }
        }
    }
    return o;
}
struct Metric {double max_abs=0,mean_abs=0,relative_l2=0;bool finite=true;};
Metric metric(const std::vector<double>& reference,const Output& actual,bool normalize) {
    Metric m;double square=0,reference_square=0;
    for(size_t i=0;i<reference.size();++i) {
        double denominator=normalize?double(actual.den[(i/(K*D))*K+(i/D)%K]):1.0;
        double ref=reference[i]/denominator,value=double(actual.value[i])/denominator;
        if(!std::isfinite(ref)||!std::isfinite(value)||!(denominator>0)){m.finite=false;continue;}
        double delta=value-ref;m.max_abs=std::max(m.max_abs,std::abs(delta));
        m.mean_abs+=std::abs(delta);square+=delta*delta;reference_square+=ref*ref;
    }
    m.mean_abs/=reference.size();
    m.relative_l2=reference_square>0?std::sqrt(square/reference_square):square==0?0:INFINITY;
    return m;
}
bool instruction_gate(const std::vector<double>& reference,const std::vector<double>& envelope,
                      const Output& actual,double& max_budget,double& max_fraction) {
    constexpr double unit=0x1p-24;
    // Covers 32 AVX FMA additions, or two AMX even/odd 16-term chains plus
    // their horizontal/add-to-C operations (at most two passes). Quantization
    // error is absent from this gate because its oracle uses the packed operands.
    constexpr double gamma40=40*unit/(1-40*unit);
    constexpr double underflow_floor=128*double(std::numeric_limits<float>::min());
    bool pass=true;max_budget=max_fraction=0;
    for(size_t i=0;i<reference.size();++i) {
        double budget=gamma40*envelope[i]+underflow_floor;
        double error=std::abs(double(actual.value[i])-reference[i]);
        max_budget=std::max(max_budget,budget);
        if(!std::isfinite(actual.value[i])||!std::isfinite(error)||error>budget)pass=false;
        max_fraction=std::max(max_fraction,budget>0?error/budget:error==0?0:INFINITY);
    }
    for(float den:actual.den)if(!std::isfinite(den)||den<=0)pass=false;
    return pass;
}
void print_metric(const char* mode,const char* reference,const Metric& m,bool normalize) {
    std::cout<<"HEAD_PH_ERROR method="<<mode<<" reference="<<reference
      <<" output="<<(normalize?"U_over_original_FP32_den_reported_in_FP64":"unnormalized_U")
      <<" max_abs="<<m.max_abs<<" mean_abs="<<m.mean_abs<<" relative_L2="<<m.relative_l2
      <<" finite="<<m.finite<<" diagnostic_not_full_model_gate=1\n";
}
double median(std::array<double,3> a){std::sort(a.begin(),a.end());return a[1];}
}

void probe_head_PH_compact(const Graph& g,const Param& p,const gat::Workspace& base,const Schedule& schedule) {
    if(p.in!=D || p.heads!=K) {
        std::cout<<"HEAD_PH_PROBE_SKIP reason=requires_L2_D256_K8 D="<<p.in<<" K="<<p.heads<<'\n';return;
    }
    if(g.row.size()!=g.n+1 || g.col.size()!=g.e || schedule.perm.size()!=g.n ||
       base.xbf.size()!=checked(g.n,D) || base.left.size()!=checked(g.n,K) || base.right.size()!=checked(g.n,K))
        throw std::runtime_error("head PH probe graph/workspace shape mismatch");
    uint64_t eligible=0;auto sample=select_samples(g,schedule,eligible);
    if(sample.empty()){std::cout<<"HEAD_PH_PROBE_SKIP reason=no_degree_ge_32_rows\n";return;}
    uint64_t fingerprint=1469598103934665603ull;
    for(uint32_t row:sample){fingerprint^=row;fingerprint*=1099511628211ull;}
    std::cout<<std::setprecision(17)<<"HEAD_PH_PROBE_BEGIN N="<<g.n<<" E="<<g.e
      <<" D="<<D<<" K="<<K<<" B="<<B<<" eligible_degree_ge_B="<<eligible<<" samples="<<sample.size()
      <<" selection=systematic_even_ranks_in_DegreeSort_eligible_order"
      <<" population_estimate=0 neighbors=first_32_only full_row_attention=0"
      <<" sample_first_row="<<sample.front()<<" sample_last_row="<<sample.back()
      <<" sample_row_fingerprint="<<fingerprint<<" configured_AMX_rows="<<configured_rows
      <<" useful_AMX_rows="<<K<<" head_row_occupancy="<<double(K)/configured_rows
      <<" requested_threads="<<threads<<" current_OMP_max_threads="<<omp_get_max_threads()
      <<" omp_proc_bind="<<int(omp_get_proc_bind())<<" warmups=1 reps=3"
      <<" allocation=outside_timers AMX_permission_config_release=in_full_microtimer"
      <<" H_packing=direct_vector_BF16_pair_interleave intermediate_Hrow_copy=0"
      <<" score_exp=shared_FP32_stable_firstblock independent_head_attention=1"
      <<" denominator=original_FP32_unchanged output=unnormalized_U"
      <<" normalization_error_reporting=FP64_outside_timer"
      <<" coefficients=BF16_multiplication_quantization_not_approximate_softmax"
      <<" full_model_speedup=NOT_MEASURED removes_8x_FMA=0"
      <<" full_model_master_gate=UNCHANGED_0.003_NOT_EVALUATED\n";

    std::vector<Scratch> scratch(threads);
    std::array<Output,3> output;for(auto& o:output)o.allocate(sample.size());
    std::array<std::array<double,3>,3> times{};
    for(Mode mode:{Mode::AVX,Mode::High,Mode::HighLow})
        execute(mode,g,p,base,sample,scratch,output[int(mode)]);
    constexpr Mode order[3][3]={{Mode::AVX,Mode::High,Mode::HighLow},
                              {Mode::HighLow,Mode::High,Mode::AVX},
                              {Mode::High,Mode::AVX,Mode::HighLow}};
    for(int rep=0;rep<3;++rep)for(int position=0;position<3;++position) {
        Mode mode=order[rep][position];Run r=execute(mode,g,p,base,sample,scratch,output[int(mode)]);
        times[int(mode)][rep]=r.wall;
        std::cout<<"HEAD_PH_TIME method="<<name(mode)<<" rep="<<rep<<" order_position="<<position
          <<" total_ms="<<r.wall*1000<<" actual_threads="<<r.actual_threads
          <<" score_exp_worker_ms="<<r.worker.score_exp*1000
          <<" H_gather_VNNI_pack_worker_ms="<<r.worker.h_gather_pack*1000
          <<" P_pack_worker_ms="<<r.worker.p_pack*1000
          <<" PH_compute_scatter_worker_ms="<<r.worker.ph_compute_scatter*1000
          <<" AMX_init_worker_ms="<<r.worker.amx_init*1000
          <<" worker_stage_times_are_sums_not_wall_decomposition=1"
          <<" AVX_PH_bracket_includes_shared_H_gather_FMA_output=1"
          <<" AMX_PH_bracket_includes_tile_load_compute_store_scatter=1\n";
    }
    bool same_den=true;
    for(int mode=1;mode<3;++mode)
        same_den=same_den && std::memcmp(output[0].den.data(),output[mode].den.data(),output[0].den.size()*sizeof(float))==0;
    std::cout<<"HEAD_PH_DEN_CONTROL original_FP32_den_bitwise_all_methods="<<same_den<<'\n';
    if(!same_den)throw std::runtime_error("head PH methods changed FP32 denominator");

    // All reference/error work is outside every measured region. The FP64
    // oracle does scalar products/sums over the input operands, never AMX/AVX PH.
    Oracle oracle=scalar_oracle(g,p,base,sample,output[0]);
    std::cout<<"HEAD_PH_ORACLE arithmetic=independent_scalar_FP64_dot"
      <<" quantized_operand_source=RNE_P_component_replay_native_H_bits"
      <<" AMX_BF16_DAZ_H_subnormal_operands="<<oracle.H_subnormal_operands
      <<" AMX_BF16_DAZ_P_subnormal_components="<<oracle.P_subnormal_components
      <<" MXCSR="<<_mm_getcsr()<<" budget=gamma40_abs_products_plus_128_FLT_MIN"
      <<" BF16_quant_error_not_in_instruction_budget=1\n";
    bool gates=true;
    double avx_median=median(times[0]);
    for(int mode=0;mode<3;++mode) {
        Mode method=Mode(mode);double budget=0,fraction=0;
        bool pass=instruction_gate(oracle.value[mode],oracle.envelope[mode],output[mode],budget,fraction);
        gates=gates && pass;
        std::cout<<"HEAD_PH_INSTRUCTION_GATE method="<<name(method)<<" pass="<<pass
          <<" max_absolute_budget="<<budget<<" max_error_to_budget="<<fraction
          <<" reference=same_operand_scalar_FP64 not_full_model_master_gate=1\n";
        print_metric(name(method),"same_quantized_operands_FP64",metric(oracle.value[mode],output[mode],false),false);
        print_metric(name(method),"same_quantized_operands_FP64",metric(oracle.value[mode],output[mode],true),true);
        if(mode) {
            print_metric(name(method),"FP32_P_native_BF16_H_FP64",metric(oracle.value[0],output[mode],false),false);
            print_metric(name(method),"FP32_P_native_BF16_H_FP64",metric(oracle.value[0],output[mode],true),true);
        }
        uint64_t count=sample.size(),passes=mode==2?2:1;
        uint64_t useful_FMA=count*K*B*D;
        uint64_t physical_FMA=mode?count*configured_rows*B*D*passes:useful_FMA;
        uint64_t rawH_bytes=count*B*D*sizeof(BF16);
        auto bounds=std::minmax_element(times[mode].begin(),times[mode].end());
        double med=median(times[mode]);
        std::cout<<"HEAD_PH_SUMMARY method="<<name(method)<<" median_ms="<<med*1000
          <<" min_ms="<<*bounds.first*1000<<" max_ms="<<*bounds.second*1000
          <<" micro_speed_vs_AVX="<<avx_median/med<<" scope=sampled_hot_repeated_first_blocks"
          <<" useful_FMA="<<useful_FMA<<" physical_issued_FMA="<<physical_FMA
          <<" physical_to_useful_FMA="<<double(physical_FMA)/useful_FMA
          <<" AMX_passes="<<(mode?passes:0)<<" TDP_calls="<<(mode?count*(D/16)*passes:0)
          <<" raw_H_logical_read_bytes="<<rawH_bytes
          <<" H_local_gather_write_bytes="<<0
          <<" H_VNNI_pack_read_bytes="<<(mode?rawH_bytes:0)
          <<" H_VNNI_pack_write_bytes="<<(mode?rawH_bytes:0)
          <<" P_pack_BF16_write_bytes="<<(mode?count*configured_rows*B*sizeof(BF16)*passes:0)
          <<" H_tile_load_bytes="<<(mode?rawH_bytes:0)
          <<" P_tile_load_bytes="<<(mode?count*configured_rows*B*sizeof(BF16)*passes:0)
          <<" C_tile_store_bytes="<<(mode?count*configured_rows*D*sizeof(float):0)
          <<" useful_U_output_bytes="<<count*K*D*sizeof(float)
          <<" no_DRAM_measurement=1 full_model_speedup=NOT_MEASURED\n";
    }
    std::cout<<"HEAD_PH_PROBE_END instruction_gates="<<gates
      <<" full_model_master_gate=NOT_EVALUATED task_accuracy=NOT_EVALUATED\n";
    if(!gates)throw std::runtime_error("head PH same-operand instruction gate failed");
}
}
