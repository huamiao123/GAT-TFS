#include "checks.hpp"
#include "icpp_online.hpp"
#include <iomanip>
#include <iostream>
#include <random>

// Correctness harness only: the audited icpx build lowers the production
// intrinsic to this oneAPI internal entry. Matching an internal ABI is
// deliberately confined to checks.cpp; it is not a portable kernel API.
extern "C" __m512 __svml_expf16_z0(__m512);
extern "C" __m512 gat_checks_plain_expf16(__m512) __asm__("__svml_expf16");

namespace gat::icpp_online {
namespace {
constexpr double fp32_abs_gate=1e-5;
constexpr double fp32_rel_gate=1e-5;
constexpr double master_abs_gate=.003; // Existing gate; never enlarged here.
constexpr double unit_roundoff=0x1p-24;

struct Shape {int n,D,K,d;};
struct Fixture {
    Graph graph;
    Param param;
    std::vector<float> left,right;
};
struct Simulation {
    // The only running weighted state is V[d]. No U[D] aggregation is used.
    std::vector<float> numerator,output,max,den;
    std::vector<uint64_t> blocks,updates,rescales;
    std::vector<std::vector<unsigned char>> rescale_history;
    std::vector<double> rounding_budget;
    std::vector<double> den_shadow,den_rounding_budget;
};
struct Metric {
    double max_abs=0,mean_abs=0,relative_l2=0;
    bool finite=true;
};
template<class Ref> Metric measure(const std::vector<Ref>& ref,
                                 const std::vector<float>& actual) {
    if(ref.size()!=actual.size() || ref.empty())
        throw std::runtime_error("ICPP smoke comparison shape mismatch");
    Metric result;long double sum=0,delta2=0,ref2=0;
    for(size_t k=0;k<ref.size();++k) {
        double a=double(ref[k]),b=double(actual[k]);
        if(!std::isfinite(a) || !std::isfinite(b)){result.finite=false;continue;}
        double delta=b-a,absolute=std::abs(delta);
        result.max_abs=std::max(result.max_abs,absolute);sum+=absolute;
        delta2+=static_cast<long double>(delta)*delta;
        ref2+=static_cast<long double>(a)*a;
    }
    if(!result.finite){result.max_abs=result.mean_abs=result.relative_l2=INFINITY;return result;}
    result.mean_abs=double(sum/ref.size());
    result.relative_l2=ref2>0?double(std::sqrt(delta2/ref2)):(delta2==0?0:INFINITY);
    return result;
}
void print_metric(const char* phase,const Shape& shape,int block,const Metric& m,
                  const char* gate,bool passed) {
    std::cout<<std::setprecision(12)<<"ICPP_ONLINE_SMOKE phase="<<phase
      <<" N="<<shape.n<<" D="<<shape.D<<" K="<<shape.K<<" d="<<shape.d
      <<" block="<<block<<" finite="<<m.finite<<" max_abs_error="<<m.max_abs
      <<" mean_abs_error="<<m.mean_abs<<" relative_L2_error="<<m.relative_l2
      <<" gate="<<gate<<" pass="<<passed<<'\n';
}
void require(bool condition,const std::string& explanation) {
    if(!condition)throw std::runtime_error("ICPP Online smoke failed: "+explanation);
}
float score(const Fixture& f,uint32_t row,uint32_t source,int head) {
    const int K=f.param.heads;
    // One FP32 score definition is shared by every oracle and simulation.
    return leak(f.left[size_t(row)*K+head]+f.right[size_t(source)*K+head]);
}
float exp_value(float value,bool svml) {
    if(!svml)return std::exp(value);
    alignas(64) float lanes[16];
    _mm512_store_ps(lanes,__svml_expf16_z0(_mm512_set1_ps(value)));
    return lanes[0];
}
void diagnose_exp_entries() {
    std::vector<float> plain,matched;plain.reserve(1024);matched.reserve(1024);
    uint64_t max_ulps=0,differences=0;float argument_at_max=0,plain_at_max=0,matched_at_max=0;
    for(int base=0;base<1024;base+=16) {
        alignas(64) float inputs[16],first[16],second[16];
        for(int lane=0;lane<16;++lane)inputs[lane]=-10.f+10.f*float(base+lane)/1023.f;
        const __m512 x=_mm512_load_ps(inputs);
        _mm512_store_ps(first,gat_checks_plain_expf16(x));
        _mm512_store_ps(second,__svml_expf16_z0(x));
        for(int lane=0;lane<16;++lane) {
            plain.push_back(first[lane]);matched.push_back(second[lane]);
            uint32_t a=0,b=0;std::memcpy(&a,first+lane,4);std::memcpy(&b,second+lane,4);
            const uint64_t ulps=a>b?uint64_t(a-b):uint64_t(b-a);
            if(ulps)++differences;
            if(ulps>max_ulps){max_ulps=ulps;argument_at_max=inputs[lane];plain_at_max=first[lane];matched_at_max=second[lane];}
        }
    }
    const Metric error=measure(plain,matched);
    std::cout<<std::setprecision(17)<<"ICPP_ONLINE_EXP_ENTRY_DIAGNOSTIC"
      <<" samples=1024 interval=[-10,0] plain_entry=__svml_expf16"
      <<" kernel_matched_entry=__svml_expf16_z0 differing_values="<<differences
      <<" max_abs_error="<<error.max_abs<<" mean_abs_error="<<error.mean_abs
      <<" relative_L2_error="<<error.relative_l2<<" max_ULP_distance="<<max_ulps
      <<" argument_at_max_ULP="<<argument_at_max<<" plain_at_max="<<plain_at_max
      <<" matched_at_max="<<matched_at_max<<" internal_ABI_scope=checks_only"
      <<" matching_entry_does_not_certify_FP32_master_precision=true\n";
}
float rounded_multiply(float a,float b) {
    // Commit this FP32 rounding before a subsequent addition. Inlining must
    // not turn the scalar denominator recurrence into multiply-add contraction.
    volatile float rounded=a*b;
    return rounded;
}
float rounded_add(float a,float b) {
    volatile float rounded=a+b;
    return rounded;
}
double gamma(uint64_t operations) {
    const double product=double(operations)*unit_roundoff;
    require(product<.1,"rounding budget too large for this tiny fixture");
    return product/(1-product);
}

Fixture fixture(Shape shape,int seed) {
    Fixture f;Graph& g=f.graph;Param& p=f.param;
    g.n=shape.n;g.din=shape.D;g.classes=shape.d;
    p.in=shape.D;p.heads=shape.K;p.dim=shape.d;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> random(-1.f,1.f);
    g.x.resize(size_t(g.n)*p.in);
    for(auto& v:g.x)v=.35f*random(rng);
    p.w.resize(size_t(p.in)*p.width());
    for(auto& v:p.w)v=.35f*random(rng)/std::sqrt(float(p.in));
    p.al.resize(p.width());p.ar.resize(p.width());
    for(auto& v:p.al)v=.4f*random(rng)/std::sqrt(float(p.dim));
    for(auto& v:p.ar)v=.4f*random(rng)/std::sqrt(float(p.dim));
    f.left.resize(size_t(g.n)*p.heads);f.right.resize(f.left.size());
    for(uint32_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h) {
        const float position=float(i)/float(g.n-1);
        f.left[size_t(i)*p.heads+h]=.09f*random(rng)+.017f*h;
        if(h==0)f.right[size_t(i)*p.heads+h]=.9f*position-.45f;
        else if(h==1)f.right[size_t(i)*p.heads+h]=-.8f*position+.4f;
        else f.right[size_t(i)*p.heads+h]=.5f*random(rng)+.021f*h;
    }
    // Duplicate edges are intentional. Degrees cross every requested block
    // boundary; source maxima can rise only in later blocks, including B=64.
    const uint64_t degrees[]={0,1,15,16,17,31,32,33,63,64,65,96,97,129,130,257,273};
    g.row.push_back(0);
    for(uint32_t i=0;i<g.n;++i) {
        uint64_t degree=degrees[(i*7)%17];
        if(i==0 || i==6)degree=0;
        if(i==2 || i==4 || i==g.n-1)degree=129;
        for(uint64_t edge=0;edge<degree;++edge) {
            uint32_t source=0;
            if(i==2 || i==g.n-1 || i%4==1)
                source=uint32_t(std::min<uint64_t>(g.n-1,edge/8));
            else if(i==4 || i%4==2)
                source=uint32_t(g.n-1-std::min<uint64_t>(g.n-1,edge/8));
            else source=uint32_t((edge*13+i*5+(edge/17)*3)%g.n);
            g.col.push_back(source);
        }
        g.row.push_back(g.col.size());
    }
    g.e=g.col.size();g.validate();p.validate();return f;
}

std::vector<double> stable_fp64(const Fixture& f,bool quantized_inputs=false) {
    const Graph& g=f.graph;const Param& p=f.param;
    // Reference-only tiny Z materialization. The optimized path must not do it.
    std::vector<double> projected(size_t(g.n)*p.width(),0),out(projected.size(),0);
    for(uint32_t node=0;node<g.n;++node)for(int h=0;h<p.heads;++h)
        for(int d=0;d<p.dim;++d) {
            double value=0;
            for(int k=0;k<p.in;++k) {
                float x=g.x[size_t(node)*p.in+k];
                float w=p.w[size_t(k)*p.width()+h*p.dim+d];
                if(quantized_inputs){x=expand(rne(x));w=expand(rne(w));}
                value+=double(x)*double(w);
            }
            projected[size_t(node)*p.width()+h*p.dim+d]=value;
        }
    for(uint32_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h) {
        if(g.row[i]==g.row[i+1])continue;
        double maximum=-INFINITY,denominator=0;
        for(uint64_t e=g.row[i];e<g.row[i+1];++e)
            maximum=std::max(maximum,double(score(f,i,g.col[e],h)));
        for(uint64_t e=g.row[i];e<g.row[i+1];++e) {
            double weight=std::exp(double(score(f,i,g.col[e],h))-maximum);
            denominator+=weight;
            for(int d=0;d<p.dim;++d)
                out[size_t(i)*p.width()+h*p.dim+d]+=
                    weight*projected[size_t(g.col[e])*p.width()+h*p.dim+d];
        }
        for(int d=0;d<p.dim;++d)out[size_t(i)*p.width()+h*p.dim+d]/=denominator;
    }
    return out;
}

Simulation simulate(const Fixture& f,int block,bool bf16,bool svml) {
    const Graph& g=f.graph;const Param& p=f.param;
    const int kblocks=(p.in+31)/32;
    Simulation s;const size_t nk=size_t(g.n)*p.heads,nw=size_t(g.n)*p.width();
    s.numerator.assign(nw,0);s.output.assign(nw,0);s.rounding_budget.assign(nw,0);
    s.max.assign(nk,-INFINITY);s.den.assign(nk,0);
    s.den_shadow.assign(nk,0);s.den_rounding_budget.assign(nk,0);
    s.blocks.assign(nk,0);s.updates.assign(nk,0);s.rescales.assign(nk,0);
    s.rescale_history.resize(nk);
    std::vector<float> x=g.x,w=p.w;
    if(bf16){for(auto& v:x)v=expand(rne(v));for(auto& v:w)v=expand(rne(v));}
    for(uint32_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h) {
        const size_t state=size_t(i)*p.heads+h,output=size_t(i)*p.width()+h*p.dim;
        std::vector<double> envelope(p.dim,0);
        std::vector<float> weighted(p.in,0);
        for(uint64_t begin=g.row[i];begin<g.row[i+1];begin+=uint64_t(block)) {
            uint64_t end=std::min(g.row[i+1],begin+uint64_t(block));
            float maximum=-INFINITY;
            for(uint64_t e=begin;e<end;++e)maximum=std::max(maximum,score(f,i,g.col[e],h));
            const float next=std::max(s.max[state],maximum);
            const bool update=next>s.max[state],rescale=update && s.den[state]>0;
            ++s.blocks[state];if(update)++s.updates[state];if(rescale)++s.rescales[state];
            s.rescale_history[state].push_back(rescale);
            if(rescale) {
                const float r=exp_value(s.max[state]-next,svml);
                s.den[state]=bf16?rounded_multiply(s.den[state],r):s.den[state]*r;
                s.den_shadow[state]*=double(r);
                for(int d=0;d<p.dim;++d){s.numerator[output+d]*=r;envelope[d]*=double(r);}
            }
            s.max[state]=next;
            for(uint64_t e=begin;e<end;++e) {
                const float probability=exp_value(score(f,i,g.col[e],h)-next,svml);
                s.den[state]=bf16?rounded_add(s.den[state],probability):s.den[state]+probability;
                s.den_shadow[state]+=double(probability);
                for(int k=0;k<p.in;++k) {
                    float value=probability*x[size_t(g.col[e])*p.in+k];
                    weighted[k]=bf16?expand(rne(value)):value;
                }
                for(int d=0;d<p.dim;++d) {
                    if(bf16) {
                        // Intel TDPBF16PS pseudocode: each 32-wide kb forms
                        // separate even and odd chains from zero, then adds
                        // the two FP32 chains and the previous FP32 V.
                        // Fixtures exclude subnormals; DAZ/FTZ is immaterial.
                        for(int kb=0;kb<kblocks;++kb) {
                            float even=0,odd=0;
                            for(int pair=0;pair<16;++pair) {
                                int k=kb*32+2*pair;
                                if(k<p.in)even=std::fma(weighted[k],w[size_t(k)*p.width()+h*p.dim+d],even);
                                if(k+1<p.in)odd=std::fma(weighted[k+1],w[size_t(k+1)*p.width()+h*p.dim+d],odd);
                            }
                            float pair_sum=even+odd;
                            s.numerator[output+d]+=pair_sum;
                        }
                    } else {
                        // FP32 control retains the same per-neighbor pH -> W
                        // projection and V[d] state as the original ICPP flow.
                        for(int k=0;k<p.in;++k)
                            s.numerator[output+d]=std::fma(weighted[k],
                                w[size_t(k)*p.width()+h*p.dim+d],s.numerator[output+d]);
                    }
                    for(int k=0;k<p.in;++k)
                        envelope[d]+=std::abs(double(weighted[k])*double(w[size_t(k)*p.width()+h*p.dim+d]));
                }
            }
        }
        const float inverse=s.den[state]>0?1.f/s.den[state]:0.f;
        const uint64_t degree=g.row[i+1]-g.row[i];
        // All p/r are positive. A same-p/r FP32 recurrence has a forward
        // rounding bound gamma(edge additions + rescale multiplications)*l.
        // Two rounded implementations use twice that bound. The FP64 shadow
        // uses the SAME FP32 p/r, rather than a different FP64-exp attention.
        s.den_rounding_budget[state]=2*gamma(degree+s.rescales[state])*s.den_shadow[state];
        // Conservative, cancellation-safe FP32 operation-order budget for TWO
        // rounded implementations: 16 FMA chain, pair-add, kb/edge accumulation,
        // row rescaling and output normalization. It depends on operand
        // magnitudes and actual work, never on observed master BF16 error.
        const double chain=gamma(17),acc=gamma(degree*uint64_t(kblocks)+s.rescales[state]+3);
        for(int d=0;d<p.dim;++d) {
            s.output[output+d]=s.numerator[output+d]*inverse;
            s.rounding_budget[output+d]=2*(chain+acc+chain*acc+4*unit_roundoff)*
                                          envelope[d]*double(inverse);
        }
    }
    return s;
}

std::vector<float> replay_simd_denominator(const Fixture& f,const Schedule& schedule,int block) {
    const auto& g=f.graph;const auto& p=f.param;
    std::vector<float> result(size_t(g.n)*p.heads,0);
    for(size_t base=0;base<g.n;base+=16)for(int h=0;h<p.heads;++h) {
        const int batch=int(std::min<uint64_t>(16,g.n-base));
        uint32_t rows[16]={};uint64_t starts[16]={},degrees[16]={},max_degree=0;
        alignas(64) float left[16]={},den[16]={},maximum[16];
        for(int n=0;n<16;++n)maximum[n]=-INFINITY;
        for(int n=0;n<batch;++n) {
            rows[n]=schedule.perm[base+n];starts[n]=g.row[rows[n]];
            degrees[n]=g.row[rows[n]+1]-starts[n];max_degree=std::max(max_degree,degrees[n]);
            left[n]=f.left[size_t(rows[n])*p.heads+h];
        }
        for(uint64_t begin=0;begin<max_degree;begin+=uint64_t(block)) {
            const int count=int(std::min<uint64_t>(block,max_degree-begin));
            alignas(64) float scores[64][16],block_max[16];
            for(int n=0;n<16;++n)block_max[n]=-INFINITY;
            for(int b=0;b<count;++b) {
                alignas(64) float right[16]={};unsigned valid=0;
                for(int n=0;n<batch;++n)if(begin+uint64_t(b)<degrees[n]) {
                    valid|=1u<<n;
                    uint32_t source=g.col[starts[n]+begin+uint64_t(b)];
                    right[n]=f.right[size_t(source)*p.heads+h];
                }
                __m512 e=leaky(_mm512_add_ps(_mm512_load_ps(left),_mm512_load_ps(right)));
                e=_mm512_mask_mov_ps(_mm512_set1_ps(-INFINITY),__mmask16(valid),e);
                _mm512_store_ps(scores[b],e);
                _mm512_store_ps(block_max,_mm512_max_ps(_mm512_load_ps(block_max),e));
            }
            alignas(64) float previous[16];std::memcpy(previous,maximum,sizeof(previous));
            _mm512_store_ps(maximum,_mm512_max_ps(_mm512_load_ps(maximum),_mm512_load_ps(block_max)));
            unsigned changed=0;
            for(int n=0;n<batch;++n)if(begin<degrees[n] && den[n]>0 && maximum[n]>previous[n])changed|=1u<<n;
            __m512 shift=_mm512_maskz_sub_ps(__mmask16(changed),_mm512_load_ps(previous),_mm512_load_ps(maximum));
            _mm512_store_ps(den,_mm512_mul_ps(_mm512_load_ps(den),__svml_expf16_z0(shift)));
            for(int b=0;b<count;++b) {
                unsigned valid=0;
                for(int n=0;n<batch;++n)if(begin+uint64_t(b)<degrees[n])valid|=1u<<n;
                __m512 shifted=_mm512_maskz_sub_ps(__mmask16(valid),_mm512_load_ps(scores[b]),_mm512_load_ps(maximum));
                __m512 weight=_mm512_maskz_mov_ps(__mmask16(valid),__svml_expf16_z0(shifted));
                _mm512_store_ps(den,_mm512_add_ps(_mm512_load_ps(den),weight));
            }
        }
        for(int n=0;n<batch;++n)result[size_t(rows[n])*p.heads+h]=den[n];
    }
    return result;
}
bool denominator_diagnostics(const Shape& shape,int block,const Fixture& f,
                             const Simulation& reference,const std::vector<float>& actual) {
    require(reference.den.size()==actual.size(),"denominator diagnostic shape");
    bool budget_ok=true;size_t differences=0,printed=0;
    double max_budget=0,max_fraction=0;
    for(size_t idx=0;idx<actual.size();++idx) {
        const double error=std::abs(double(actual[idx])-reference.den[idx]);
        const double budget=reference.den_rounding_budget[idx];
        max_budget=std::max(max_budget,budget);
        const double fraction=budget>0?error/budget:(error==0?0:INFINITY);
        max_fraction=std::max(max_fraction,fraction);
        if(!std::isfinite(actual[idx]) || error>budget)budget_ok=false;
        if(error==0)continue;
        ++differences;
        if(printed++>=16)continue;
        const uint32_t node=uint32_t(idx/size_t(shape.K));const int head=int(idx%size_t(shape.K));
        uint32_t expected_bits=0,actual_bits=0;
        std::memcpy(&expected_bits,&reference.den[idx],sizeof(expected_bits));
        std::memcpy(&actual_bits,&actual[idx],sizeof(actual_bits));
        const uint64_t ulps=expected_bits>actual_bits?uint64_t(expected_bits-actual_bits):uint64_t(actual_bits-expected_bits);
        const double ulp_size=double(std::nextafter(reference.den[idx],std::numeric_limits<float>::infinity()))-reference.den[idx];
        std::cout<<std::setprecision(17)<<"ICPP_ONLINE_DEN_DIAGNOSTIC N="<<shape.n<<" D="<<shape.D
          <<" K="<<shape.K<<" d="<<shape.d<<" block="<<block<<" node="<<node<<" head="<<head
          <<" degree="<<(f.graph.row[node+1]-f.graph.row[node])<<" rescales="<<reference.rescales[idx]
          <<" expected="<<reference.den[idx]<<" actual="<<actual[idx]<<" ULP_distance="<<ulps
          <<" expected_ULP_size="<<ulp_size<<" abs_error="<<error<<" FP64_same_p_r_shadow="<<reference.den_shadow[idx]
          <<" arithmetic_budget="<<budget<<" within_budget="<<(std::isfinite(actual[idx]) && error<=budget)
          <<" cause=scalar_contract_or_exp_lane_difference_not_yet_attributed\n";
    }
    std::cout<<"ICPP_ONLINE_DEN_BUDGET N="<<shape.n<<" D="<<shape.D<<" K="<<shape.K
      <<" d="<<shape.d<<" block="<<block<<" differing_states="<<differences
      <<" omitted_differing_states="<<(differences>16?differences-16:0)
      <<" max_absolute_budget="<<max_budget<<" max_error_to_budget="<<max_fraction
      <<" basis=2_gamma_degree_plus_rescales_times_FP64_same_p_r_shadow"
      <<" master_gate_independent=true arithmetic_budget_pass="<<budget_ok<<'\n';
    return budget_ok;
}

bool budget_gate(const Simulation& reference,const std::vector<float>& actual,
                 double& max_budget,double& max_fraction) {
    require(reference.output.size()==actual.size(),"instruction gate shape mismatch");
    bool passed=true;max_budget=max_fraction=0;
    for(size_t k=0;k<actual.size();++k) {
        double error=std::abs(double(actual[k])-reference.output[k]);
        double budget=reference.rounding_budget[k];max_budget=std::max(max_budget,budget);
        if(!std::isfinite(actual[k]) || !std::isfinite(error) || error>budget)passed=false;
        const double fraction=budget>0?error/budget:(error==0?0:INFINITY);
        max_fraction=std::max(max_fraction,fraction);
    }
    return passed;
}
std::vector<float> normalize(const Graph& g,const Param& p,const gat::Workspace& w) {
    std::vector<float> output=w.out;
    for(uint32_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h) {
        const float denominator=w.den[size_t(i)*p.heads+h];
        const float inverse=denominator>0?1.f/denominator:0.f;
        for(int d=0;d<p.dim;++d)output[size_t(i)*p.width()+h*p.dim+d]*=inverse;
    }
    return output;
}
void fill_workspace(const Fixture& f,Workspace& w) {
    w.allocate(f.graph,f.param,true);
    require(w.base.z.empty() && w.base.score.empty() && w.base.p.empty() && w.base.alpha.empty(),
            "optimized workspace materialized Z, edge scores, or edge probabilities");
    convert(f.graph.x.data(),w.base.xbf.data(),f.graph.x.size());
    w.base.left=f.left;w.base.right=f.right;
    // Poison outputs and state so missing/scattered rows cannot pass silently.
    std::fill(w.base.out.begin(),w.base.out.end(),std::numeric_limits<float>::quiet_NaN());
    std::fill(w.base.den.begin(),w.base.den.end(),std::numeric_limits<float>::quiet_NaN());
    std::fill(w.base.max.begin(),w.base.max.end(),std::numeric_limits<float>::quiet_NaN());
}
void counters(const Fixture& f,const Schedule& schedule,const Simulation& simulation,
              const Workspace& actual,const Stats& stats,int block) {
    const auto& g=f.graph;const auto& p=f.param;
    require(actual.row_blocks==simulation.blocks,"per-row/head blocks or original-row scatter");
    require(actual.row_max_updates==simulation.updates,"per-row/head maximum update count");
    require(actual.row_rescales==simulation.rescales,"initial maximum counted as a rescale");
    uint64_t blocks=0,updates=0,rescales=0,steps=0,events=0;
    for(size_t i=0;i<simulation.blocks.size();++i) {
        blocks+=simulation.blocks[i];updates+=simulation.updates[i];rescales+=simulation.rescales[i];
        const float maximum=actual.base.max[i],expected=simulation.max[i];
        require(maximum==expected,"running maximum or empty-row -INF mismatch");
    }
    for(size_t tile=0;tile<g.n;tile+=16) {
        const size_t end=std::min<size_t>(g.n,tile+16);
        uint64_t max_degree=0;
        for(size_t n=tile;n<end;++n) {
            uint32_t row=schedule.perm[n];max_degree=std::max(max_degree,g.row[row+1]-g.row[row]);
        }
        steps+=max_degree*uint64_t(p.heads);
        for(int h=0;h<p.heads;++h)for(uint64_t b=0;b<(max_degree+block-1)/block;++b) {
            bool changed=false;
            for(size_t n=tile;n<end;++n) {
                const auto& history=simulation.rescale_history[size_t(schedule.perm[n])*p.heads+h];
                if(b<history.size() && history[size_t(b)])changed=true;
            }
            if(changed)++events;
        }
    }
    const uint64_t obs=(p.dim+15)/16,dp=(p.in+31)/32*32;
    require(stats.blocks==blocks && stats.max_updates==updates && stats.rescales==rescales,"global state counters");
    require(stats.rescaled_feature_elements==rescales*uint64_t(p.dim),"logical rescaled feature count");
    require(stats.rescaled_padded_feature_elements==rescales*obs*16,"padded row scaling count");
    require(stats.rescale_events==events,"TR16 whole-tile spill event count");
    require(stats.spill_bytes==events*obs*1024 && stats.reload_bytes==events*obs*1024,"rescale-only spill/reload bytes");
    require(stats.neighbor_steps==steps && stats.amx_calls==steps*(dp/32)*obs,"synchronized neighbor coverage or kb/ob AMX call count");
    require(stats.executed_fma==steps*16*dp*obs*16,"executed padded FMA count");
    require(stats.total_tiles==uint64_t(p.heads)*((g.n+15)/16),"head-independent destination tile count");
}
void check_schedule(const Graph& g,const Schedule& schedule) {
    require(schedule.perm.size()==g.n,"Degree Sort permutation shape");
    std::vector<unsigned char> seen(g.n,0);uint64_t prior=0;
    for(uint32_t row:schedule.perm) {
        require(row<g.n && !seen[row],"Degree Sort permutation must be bijective");seen[row]=1;
        uint64_t degree=g.row[row+1]-g.row[row];require(degree>=prior,"ascending Degree Sort order");prior=degree;
    }
}
void report_master(const Shape& shape,int block,const std::vector<double>& master,
                   const std::vector<float>& value,const char* phase) {
    Metric m=measure(master,value);
    print_metric(phase,shape,block,m,"master_absolute_.003_small_fixture_only",m.finite && m.max_abs<=master_abs_gate);
}
void equal_scores_original_lifecycle_check() {
    // A controlled attention fixture, not a change to model semantics: zero
    // L/R makes every valid edge score zero, p=1, and all later r=1. Original
    // B1 and Online must therefore execute the same neighbor-step BF16(H)W
    // accumulation with no online spill/reload or row scaling.
    const Shape shape{35,33,8,47};
    Fixture f=fixture(shape,1709);
    std::fill(f.left.begin(),f.left.end(),0.f);
    std::fill(f.right.begin(),f.right.end(),0.f);
    const auto master=stable_fp64(f);
    for(int block:{16,32,64}) {
        const auto control=simulate(f,block,false,false);
        const Metric error=measure(master,control.output);
        const bool passed=error.finite && error.max_abs<=fp32_abs_gate && error.relative_l2<=fp32_rel_gate;
        print_metric("ALL_EQUAL_SCORES_FP32_CONTROL",shape,block,error,
                     "abs_1e-5_and_relative_L2_1e-5_before_AMX",passed);
        require(passed,"all-equal FP32 V[d] control must pass before original/new AMX comparison");
    }
    const auto prepared=prepare(f.param,true,true,true);
    const auto schedule=degree_schedule(f.graph);
    check_schedule(f.graph,schedule);
    const int panel=32;
    gat::Workspace original;
    original.allocate(f.graph,f.param,"tfs_bf16");
    convert(f.graph.x.data(),original.xbf.data(),f.graph.x.size());
    original.left=f.left;original.right=f.right;
    // This is the exact stable maximum for the all-equal nonempty rows, and
    // the original baseline's prescribed maximum for empty rows as well.
    std::fill(original.max.begin(),original.max.end(),0.f);
    std::fill(original.out.begin(),original.out.end(),std::numeric_limits<float>::quiet_NaN());
    std::fill(original.den.begin(),original.den.end(),std::numeric_limits<float>::quiet_NaN());
    Times original_times;
    tfs_aggregate(f.graph,f.param,prepared,schedule,original,panel,original_times);
    const auto original_numerator=original.out;
    finish(f.graph,f.param,original,false,original_times);
    for(int block:{16,32,64}) {
        Workspace online;fill_workspace(f,online);Stats stats;
        require(online.base.xbf==original.xbf,"equal-score B1/Online quantized inputs differ");
        aggregate(f.graph,f.param,prepared,schedule,online,block,panel,stats,0,true);
        const auto online_numerator=online.base.out;
        Times online_times;
        finish(f.graph,f.param,online.base,false,online_times);
        const Metric error=measure(original.out,online.base.out);
        // memcmp deliberately checks bits, including signed zero. Numeric
        // vector equality alone would not be a bitwise identity assertion.
        const bool identical=error.finite &&
            std::memcmp(original.out.data(),online.base.out.data(),original.out.size()*sizeof(float))==0;
        print_metric("ALL_EQUAL_SCORES_ORIGINAL_B1_VS_ONLINE",shape,block,error,
                     "bitwise_equal_after_finish_without_activation",identical);
        require(identical,"p=1/no-rescale Online changed original B1 output tile lifecycle");
        require(std::memcmp(original_numerator.data(),online_numerator.data(),
                            original_numerator.size()*sizeof(float))==0,
                "equal-score pre-normalization AMX numerators differ");
        require(std::memcmp(original.den.data(),online.base.den.data(),original.den.size()*sizeof(float))==0,
                "equal-score B1/Online denominators differ");
        require(stats.rescales==0 && stats.rescale_events==0 && stats.spill_bytes==0 && stats.reload_bytes==0,
                "equal scores caused a rescale or spill/reload");
        for(uint32_t row=0;row<f.graph.n;++row)for(int h=0;h<f.param.heads;++h) {
            const size_t idx=size_t(row)*f.param.heads+h;
            const uint64_t degree=f.graph.row[row+1]-f.graph.row[row];
            require(online.row_max_updates[idx]==uint64_t(degree>0) && online.row_rescales[idx]==0,
                    "equal scores must update once per nonempty row/head and never rescale");
        }
        std::cout<<"ICPP_ONLINE_SMOKE_EQUAL_SCORES block="<<block
          <<" p=1 r=1 activation=false identical_quantized_H=true identical_packed_W=true"
          <<" numerator_bitwise_equal=true denominator_bitwise_equal=true output_bitwise_equal=true"
          <<" rescale_count=0 spill_bytes=0 reload_bytes=0\n";
    }
}
} // namespace

void run_smoke() {
    std::cout<<"ICPP_ONLINE_SMOKE_BEGIN fp32_max_abs_gate="<<fp32_abs_gate
      <<" fp32_relative_L2_gate="<<fp32_rel_gate<<" original_master_abs_gate="<<master_abs_gate
      <<" scope=small_operators_not_model_accuracy"
      <<" bf16_rounding=H_W_RNE_then_RNE_pH"
      <<" emulator=TDPBF16PS_even_odd_FP32_FMA_chains"
      <<" scalar_exp_vs_SVML=reported_separately"
      <<" matched_SVML_entry=__svml_expf16_z0 internal_entry_scope=checks_only"
      <<" subnormals=excluded_DAZ_FTZ_not_tested"
      <<" instruction_gate_does_not_certify_master_gate=true\n";
    diagnose_exp_entries();
    equal_scores_original_lifecycle_check();
    const Shape shapes[]={
        {17,17,1,7},{35,17,2,32},{17,17,8,47},{35,33,1,64},
        {17,33,2,7},{35,33,8,32},{17,128,1,47},{35,128,2,64},
        {17,128,8,7},{35,256,1,32},{17,256,2,47},{35,256,8,64},
        {17,256,8,32},{35,128,8,32},{17,33,8,64},{35,17,8,7}
    };
    size_t cases=0;
    for(size_t case_id=0;case_id<sizeof(shapes)/sizeof(shapes[0]);++case_id) {
        const Shape shape=shapes[case_id];Fixture f=fixture(shape,739+int(case_id)*41);
        const auto master=stable_fp64(f),quantized_master=stable_fp64(f,true);
        const auto prepared=prepare(f.param,true,true,true);
        const auto schedule=degree_schedule(f.graph);check_schedule(f.graph,schedule);
        for(int block:{16,32,64}) {
            const int panel=(case_id%3==0?16:case_id%3==1?32:64);
            auto control=simulate(f,block,false,false);
            Metric fp32_error=measure(master,control.output);
            const bool fp32_ok=fp32_error.finite && fp32_error.max_abs<=fp32_abs_gate &&
                                fp32_error.relative_l2<=fp32_rel_gate;
            print_metric("FP32_V_ONLINE_VS_FP64_STABLE_SAME_LR",shape,block,fp32_error,
                         "abs_1e-5_and_relative_L2_1e-5",fp32_ok);
            // Do not call the AMX kernel for a case whose FP32 control failed.
            require(fp32_ok,"FP32 V[d] online control vs stable same-attention FP64");
            require(control.rescales[size_t(2)*shape.K]>0,"forced increasing maximum must rescale");
            require(control.rescales[size_t(4)*shape.K]==0,"forced nonincreasing maximum must not rescale");
            for(int h=0;h<shape.K;++h)require(control.blocks[size_t(6)*shape.K+h]==0,"empty row state");

            auto scalar=simulate(f,block,true,false);
            auto vector_exp=simulate(f,block,true,true);
            print_metric("BF16_EMULATION_STD_EXP_VS_SVML_EXP",shape,block,
                         measure(scalar.output,vector_exp.output),"diagnostic_RNE_threshold_sensitive",true);
            report_master(shape,block,master,scalar.output,"BF16_EMULATION_VS_FP32_MASTER");
            print_metric("BF16_EMULATION_VS_QUANTIZED_HW_STABLE",shape,block,
                         measure(quantized_master,scalar.output),"diagnostic_additional_pH_RNE",true);

            Workspace actual;fill_workspace(f,actual);Stats stats;
            aggregate(f.graph,f.param,prepared,schedule,actual,block,panel,stats,0,true);
            auto normalized=normalize(f.graph,f.param,actual.base);
            double max_budget=0,max_fraction=0;
            bool instruction_ok=budget_gate(vector_exp,normalized,max_budget,max_fraction);
            print_metric("AMX_VS_BF16_TDP_EMULATION_SAME_SVML_EXP",shape,block,
                         measure(vector_exp.output,normalized),"operand_based_FP32_rounding_budget",instruction_ok);
            std::cout<<"ICPP_ONLINE_SMOKE_BUDGET N="<<shape.n<<" D="<<shape.D<<" K="<<shape.K
              <<" d="<<shape.d<<" block="<<block<<" max_absolute_budget="<<max_budget
              <<" max_error_to_budget="<<max_fraction<<" basis=gamma17_plus_gamma_edge_kb_rescales"
              <<" independent_of_master_BF16_error=true\n";
            // std::exp changes may cross BF16 pH rounding boundaries. Both raw
            // errors are visible; only matched-exp emulation isolates AMX math.
            print_metric("AMX_VS_BF16_EMULATION_STD_EXP",shape,block,
                         measure(scalar.output,normalized),"diagnostic_exp_and_RNE_difference",instruction_ok);
            require(instruction_ok,"AMX instruction math vs BF16 pair-chain emulation");
            Metric denominator=measure(vector_exp.den,actual.base.den);
            const bool denominator_budget_ok=denominator_diagnostics(shape,block,f,vector_exp,actual.base.den);
            print_metric("AMX_DENOMINATOR_VS_SVML_SIMULATION",shape,block,denominator,
                         "same_p_r_gamma_budget_and_relative_L2_1e-5",denominator.finite && denominator_budget_ok && denominator.relative_l2<=1e-5);
            // Independent TR16 replay uses exactly the vector exp lane layout
            // and separate vector mul/add order, without any AMX operations.
            // Its bitwise gate distinguishes a kernel denominator error from
            // scalar-oracle floating point differences. Do not label a scalar
            // one-ULP discrepancy as a proven contraction without checks.s.
            const auto replay=replay_simd_denominator(f,schedule,block);
            const Metric replay_error=measure(replay,actual.base.den);
            const bool replay_ok=replay_error.finite &&
                std::memcmp(replay.data(),actual.base.den.data(),replay.size()*sizeof(float))==0;
            print_metric("AMX_DENOMINATOR_VS_TR16_SIMD_REPLAY",shape,block,replay_error,
                         "bitwise_same_lane_layout_and_mul_add_order",replay_ok);
            require(replay_ok,"kernel denominator differs from deterministic TR16 SIMD replay");
            require(denominator.finite && denominator_budget_ok && denominator.relative_l2<=1e-5,"online denominator rounding budget");
            counters(f,schedule,vector_exp,actual,stats,block);
            report_master(shape,block,master,normalized,"AMX_VS_FP32_MASTER");

            Workspace profiled;fill_workspace(f,profiled);Stats profile_stats;
            aggregate(f.graph,f.param,prepared,schedule,profiled,block,panel,profile_stats,1,true);
            auto profiled_normalized=normalize(f.graph,f.param,profiled.base);
            Metric profile_error=measure(normalized,profiled_normalized);
            print_metric("PROFILE_ON_VS_OFF",shape,block,profile_error,"bitwise_arithmetic_invariance",profile_error.finite && profile_error.max_abs==0);
            require(profile_error.finite && profile_error.max_abs==0 && actual.base.out==profiled.base.out &&
                    actual.base.den==profiled.base.den,"profiling changed output or denominator");
            counters(f,schedule,vector_exp,profiled,profile_stats,block);
            require(profile_stats.sampled_tiles==profile_stats.total_tiles,"period-one must profile every tile");
            // Exercise the benchmark specialization with BOTH clocks and
            // counters disabled on the full 8-head D256/d32 shape.
            if(case_id==12 && block==32) {
                Workspace speed;fill_workspace(f,speed);Stats speed_stats;
                aggregate(f.graph,f.param,prepared,schedule,speed,block,panel,speed_stats,0,false);
                const auto speed_output=normalize(f.graph,f.param,speed.base);
                const Metric speed_error=measure(normalized,speed_output);
                print_metric("COUNTERS_AND_CLOCKS_OFF_VS_PROFILE",shape,block,speed_error,
                             "bitwise_benchmark_specialization_invariance",speed_error.finite && speed_error.max_abs==0);
                require(speed_error.finite && speed_error.max_abs==0 && speed.base.out==actual.base.out &&
                        speed.base.den==actual.base.den,"counter-free benchmark specialization changed math");
            }
            // Actual normalization and ELU use the production finish function.
            Times finishing;finish(f.graph,f.param,profiled.base,true,finishing);
            std::vector<float> expected_elu=normalized;
            for(auto& value:expected_elu)value=elu(value);
            Metric activation_error=measure(expected_elu,profiled.base.out);
            print_metric("FINISH_NORMALIZATION_AND_ELU",shape,block,activation_error,
                         "abs_1e-5_SVML_activation_difference",activation_error.finite && activation_error.max_abs<=1e-5);
            require(activation_error.finite && activation_error.max_abs<=1e-5,"normalization or ELU semantics");
            std::cout<<"ICPP_ONLINE_SMOKE_COUNTERS N="<<shape.n<<" D="<<shape.D<<" K="<<shape.K
              <<" d="<<shape.d<<" block="<<block<<" edges="<<f.graph.e
              <<" blocks="<<stats.blocks<<" max_updates="<<stats.max_updates
              <<" rescales="<<stats.rescales<<" rescale_events="<<stats.rescale_events
              <<" spill_bytes="<<stats.spill_bytes<<" reload_bytes="<<stats.reload_bytes
              <<" neighbor_steps="<<stats.neighbor_steps<<" amx_calls="<<stats.amx_calls
              <<" original_row_scatter=true complete_neighborhoods=true independent_heads=true\n";
            ++cases;
        }
        // Two real-attention operator cases also exercise the layer wrapper,
        // contracted FP32 L/R, H conversion, normalization, and activation.
        if(case_id==0 || case_id==12) {
            Workspace actual;actual.allocate(f.graph,f.param,true);
            Timing timing;Stats stats;
            layer(f.graph,f.param,prepared,schedule,f.graph.x,actual,false,32,32,timing,stats,0,true,"fp32");
            f.left=actual.base.left;f.right=actual.base.right;
            auto reference=simulate(f,32,true,true);
            double max_budget=0,max_fraction=0;
            const bool passed=budget_gate(reference,actual.base.out,max_budget,max_fraction);
            print_metric("LAYER_WRAPPER_WITH_CONTRACTED_REAL_ATTENTION",shape,32,
                         measure(reference.output,actual.base.out),"same_LR_instruction_budget",passed);
            require(passed,"layer wrapper with independent Vanilla GAT attention");
            counters(f,schedule,reference,actual,stats,32);
            report_master(shape,32,stable_fp64(f),actual.base.out,"REAL_ATTENTION_AMX_VS_FP32_MASTER");
        }
    }
    std::cout<<"ICPP_ONLINE_SMOKE_PASS operator_cases="<<cases
      <<" D=17,33,128,256 K=1,2,8 d=7,32,47,64 N=17,35 block=16,32,64"
      <<" state=V_head_dim no_local_U=true master_gate_scope=small_fixtures_only"
      <<" trained_model_accuracy=UNVERIFIED\n";
}
} // namespace gat::icpp_online
