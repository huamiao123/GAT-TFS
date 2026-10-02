#include "hybrid_checks.hpp"
#include "icpp_heads_hybrid.hpp"
#include "icpp_heads_preload.hpp"
#include <iomanip>
#include <iostream>
#include <random>

// Checks-only oneAPI internal entry: earlier audited icpx builds lowered the
// production vector intrinsic to z0, while a replay intrinsic could select the
// distinct plain entry. Actual icpp_block assembly must still be audited.
extern "C" __m512 __svml_expf16_z0(__m512);

namespace gat::icpp_heads_hybrid {
namespace {
constexpr double fp32_abs_gate=1e-5,fp32_rel_gate=1e-5;
constexpr double master_abs_gate=.003; // Report separately; never enlarged.
constexpr double unit_roundoff=0x1p-24;
struct Shape {int n,D,K,d;};
struct Fixture {Graph g;Param p;std::vector<float> left,right;};
struct Metric {double max_abs=0,mean_abs=0,relative_l2=0;bool finite=true;};
struct Attention {
    // Reference-only tiny edge materialization is explicitly outside kernels.
    std::vector<float> probability,maximum,den;
    std::vector<uint64_t> blocks,updates,rescales;
    std::vector<std::vector<float>> scale;
    std::vector<std::vector<unsigned char>> changed;
};
struct Simulation {
    std::vector<float> numerator,output;
    std::vector<double> raw_budget,output_budget;
};
struct Snapshot {std::vector<float> numerator,maximum,den;};

void require(bool value,const std::string& why) {
    if(!value)throw std::runtime_error("ICPP hybrid smoke: "+why);
}
template<class T> Metric metric(const std::vector<T>& ref,const std::vector<float>& actual) {
    require(!ref.empty() && ref.size()==actual.size(),"comparison shape");
    Metric m;long double abs_sum=0,squared=0,ref_squared=0;
    for(size_t k=0;k<ref.size();++k) {
        double a=double(ref[k]),b=double(actual[k]);
        if(!std::isfinite(a) || !std::isfinite(b)){m.finite=false;continue;}
        double delta=b-a;
        m.max_abs=std::max(m.max_abs,std::abs(delta));abs_sum+=std::abs(delta);
        squared+=static_cast<long double>(delta)*delta;
        ref_squared+=static_cast<long double>(a)*a;
    }
    if(!m.finite){m.max_abs=m.mean_abs=m.relative_l2=INFINITY;return m;}
    m.mean_abs=double(abs_sum/ref.size());
    m.relative_l2=ref_squared>0?double(std::sqrt(squared/ref_squared)):(squared==0?0:INFINITY);
    return m;
}
void print_metric(const std::string& phase,const Shape& s,int block,const Metric& m,
                  const std::string& gate,bool pass) {
    std::cout<<"ICPP_HYBRID_SMOKE phase="<<phase<<" N="<<s.n<<" D="<<s.D
             <<" K="<<s.K<<" d="<<s.d<<" block="<<block<<" finite="<<m.finite
             <<" max_abs_error="<<m.max_abs<<" mean_abs_error="<<m.mean_abs
             <<" relative_L2_error="<<m.relative_l2<<" gate="<<gate<<" pass="<<pass<<'\n';
}
void exact_float(const std::vector<float>& a,const std::vector<float>& b,
                 const std::string& name,bool permit_equal_infinity=false) {
    require(a.size()==b.size(),name+" size");
    for(size_t i=0;i<a.size();++i) {
        bool same=std::memcmp(&a[i],&b[i],4)==0;
        bool finite=std::isfinite(a[i]) && std::isfinite(b[i]);
        bool allowed=permit_equal_infinity && same && std::isinf(a[i]);
        if(!same || (!finite && !allowed)) {
            uint32_t expected=0,actual=0;std::memcpy(&expected,&a[i],4);std::memcpy(&actual,&b[i],4);
            uint64_t distance=expected>actual?uint64_t(expected-actual):uint64_t(actual-expected);
            std::cout<<"ICPP_HYBRID_SMOKE_BIT_MISMATCH phase="<<name<<" index="<<i
                     <<" expected="<<a[i]<<" actual="<<b[i]<<" expected_bits="<<expected
                     <<" actual_bits="<<actual<<" bit_distance="<<distance
                     <<" positive_denominator_ULP_distance="<<distance<<'\n';
            require(false,name+" bitwise mismatch; inspect score/exp ABI and generated arithmetic, do not relax gate");
        }
    }
    std::cout<<"ICPP_HYBRID_SMOKE_EXACT phase="<<name<<" pass=1\n";
}
void count(uint64_t expected,uint64_t actual,const std::string& name) {
    if(expected!=actual) {
        std::cout<<"ICPP_HYBRID_SMOKE_COUNT_MISMATCH field="<<name<<" expected="<<expected
                 <<" actual="<<actual<<'\n';
        require(false,name+" counter mismatch");
    }
}
double gamma(uint64_t n) {
    double t=double(n)*unit_roundoff;require(t<.1,"invalid tiny-fixture operation budget");
    return t/(1-t);
}
float add(float a,float b) {volatile float value=a+b;return value;}
float multiply(float a,float b) {volatile float value=a*b;return value;}
float edge_score(const Fixture& f,uint32_t row,uint32_t source,int h) {
    float sum=add(f.left[size_t(row)*f.p.heads+h],f.right[size_t(source)*f.p.heads+h]);
    return sum>=0?sum:multiply(.2f,sum);
}

Fixture fixture(Shape s,uint32_t seed,bool equal=false) {
    Fixture f;auto& g=f.g;auto& p=f.p;
    g.n=s.n;g.din=s.D;g.classes=s.d;p.in=s.D;p.heads=s.K;p.dim=s.d;
    std::mt19937 rng(seed);std::uniform_real_distribution<float> random(-1.f,1.f);
    g.x.resize(checked(g.n,p.in));p.w.resize(checked(p.in,p.width()));
    p.al.resize(p.width());p.ar.resize(p.width());
    for(auto& x:g.x)x=.35f*random(rng);
    for(auto& x:p.w)x=.35f*random(rng)/std::sqrt(float(p.in));
    for(auto& x:p.al)x=.4f*random(rng)/std::sqrt(float(p.dim));
    for(auto& x:p.ar)x=.4f*random(rng)/std::sqrt(float(p.dim));
    f.left.resize(checked(g.n,p.heads));f.right.resize(f.left.size());
    for(uint32_t n=0;n<g.n;++n)for(int h=0;h<p.heads;++h) {
        size_t at=size_t(n)*p.heads+h;float position=float(n)/float(g.n-1);
        f.left[at]=equal?0.f:.09f*random(rng)+.017f*h;
        f.right[at]=equal?0.f:(h==0?.9f*position-.45f:
                             (h==1?-.8f*position+.4f:.5f*random(rng)+.021f*h));
    }
    const uint64_t degrees[]={0,1,15,16,17,31,32,33,63,64,65,96,97,129,130,257,273};
    g.row.push_back(0);
    for(uint32_t n=0;n<g.n;++n) {
        uint64_t degree=degrees[(n*7)%17];
        if(n==0 || n==6)degree=0;
        if(n==2 || n==4 || n+1==g.n)degree=129;
        for(uint64_t e=0;e<degree;++e) {
            uint32_t source;
            if(n==2 || n+1==g.n || n%4==1)source=uint32_t(std::min<uint64_t>(g.n-1,e/8));
            else if(n==4 || n%4==2)source=uint32_t(g.n-1-std::min<uint64_t>(g.n-1,e/8));
            else source=uint32_t((e*13+n*5+(e/17)*3)%g.n);
            g.col.push_back(source);
        }
        g.row.push_back(g.col.size());
    }
    g.e=g.col.size();g.validate();p.validate();return f;
}
void check_schedule(const Graph& g,const Schedule& s) {
    require(s.perm.size()==g.n,"DegreeSort length");
    std::vector<unsigned char> seen(size_t(g.n),0);uint64_t previous=0;
    for(uint32_t row:s.perm) {
        require(row<g.n && !seen[row],"DegreeSort not bijective");seen[row]=1;
        uint64_t degree=g.row[row+1]-g.row[row];require(degree>=previous,"DegreeSort not ascending");previous=degree;
    }
}
Attention empty_attention(const Fixture& f) {
    Attention a;size_t nk=checked(f.g.n,f.p.heads);
    a.probability.assign(checked(f.g.e,f.p.heads),0);
    a.maximum.assign(nk,-INFINITY);a.den.assign(nk,0);
    a.blocks.assign(nk,0);a.updates.assign(nk,0);a.rescales.assign(nk,0);
    a.scale.resize(nk);a.changed.resize(nk);return a;
}
Attention scalar_attention(const Fixture& f,int block) {
    Attention a=empty_attention(f);const auto& g=f.g;const auto& p=f.p;
    for(uint32_t row=0;row<g.n;++row)for(int h=0;h<p.heads;++h) {
        size_t at=size_t(row)*p.heads+h;
        for(uint64_t begin=g.row[row];begin<g.row[row+1];begin+=uint64_t(block)) {
            uint64_t end=std::min(g.row[row+1],begin+uint64_t(block));float bm=-INFINITY;
            for(uint64_t e=begin;e<end;++e)bm=std::max(bm,edge_score(f,row,g.col[e],h));
            float next=std::max(a.maximum[at],bm);
            bool update=next>a.maximum[at],rescale=update && a.den[at]>0;
            float r=rescale?std::exp(add(a.maximum[at],-next)):1.f;
            ++a.blocks[at];if(update)++a.updates[at];if(rescale)++a.rescales[at];
            a.scale[at].push_back(r);a.changed[at].push_back(rescale);
            a.den[at]=multiply(a.den[at],r);a.maximum[at]=next;
            for(uint64_t e=begin;e<end;++e) {
                float probability=std::exp(add(edge_score(f,row,g.col[e],h),-next));
                a.probability[size_t(e)*p.heads+h]=probability;
                a.den[at]=add(a.den[at],probability);
            }
        }
    }
    return a;
}
Attention matched_attention(const Fixture& f,const Schedule& schedule,int block) {
    Attention a=empty_attention(f);const auto& g=f.g;const auto& p=f.p;
    std::vector<unsigned char> visited(a.probability.size(),0);
    for(uint64_t first=0;first<g.n;first+=16)for(int h=0;h<p.heads;++h) {
        int batch=int(std::min<uint64_t>(16,g.n-first));
        uint32_t rows[16]={};uint64_t starts[16]={},degrees[16]={},max_degree=0;
        alignas(64) float left[16]={},den[16]={},maximum[16];
        for(int n=0;n<16;++n)maximum[n]=-INFINITY;
        for(int n=0;n<batch;++n) {
            rows[n]=schedule.perm[first+n];starts[n]=g.row[rows[n]];
            degrees[n]=g.row[rows[n]+1]-starts[n];max_degree=std::max(max_degree,degrees[n]);
            left[n]=f.left[size_t(rows[n])*p.heads+h];
        }
        for(uint64_t begin=0;begin<max_degree;begin+=uint64_t(block)) {
            int length=int(std::min<uint64_t>(block,max_degree-begin));
            alignas(64) float score[64][16],bm[16];__mmask16 masks[64];
            for(int n=0;n<16;++n)bm[n]=-INFINITY;
            for(int b=0;b<length;++b) {
                alignas(64) float right[16]={};unsigned valid=0;
                for(int n=0;n<batch;++n)if(begin+uint64_t(b)<degrees[n]) {
                    valid|=1u<<n;uint32_t source=g.col[starts[n]+begin+uint64_t(b)];
                    right[n]=f.right[size_t(source)*p.heads+h];
                }
                masks[b]=__mmask16(valid);
                __m512 e=leaky(_mm512_add_ps(_mm512_load_ps(left),_mm512_load_ps(right)));
                e=_mm512_mask_mov_ps(_mm512_set1_ps(-INFINITY),masks[b],e);
                _mm512_store_ps(score[b],e);
                _mm512_store_ps(bm,_mm512_max_ps(_mm512_load_ps(bm),e));
            }
            alignas(64) float previous[16],rescale[16];std::memcpy(previous,maximum,sizeof(previous));
            _mm512_store_ps(maximum,_mm512_max_ps(_mm512_load_ps(maximum),_mm512_load_ps(bm)));
            unsigned changed=0;
            for(int n=0;n<batch;++n)if(begin<degrees[n]) {
                size_t at=size_t(rows[n])*p.heads+h;bool update=maximum[n]>previous[n];
                bool scale=update && den[n]>0;if(scale)changed|=1u<<n;
                ++a.blocks[at];if(update)++a.updates[at];if(scale)++a.rescales[at];
                a.changed[at].push_back(scale);
            }
            __m512 shift=_mm512_maskz_sub_ps(__mmask16(changed),_mm512_load_ps(previous),_mm512_load_ps(maximum));
            __m512 r=__svml_expf16_z0(shift);_mm512_store_ps(rescale,r);
            _mm512_store_ps(den,_mm512_mul_ps(_mm512_load_ps(den),r));
            for(int n=0;n<batch;++n)if(begin<degrees[n])a.scale[size_t(rows[n])*p.heads+h].push_back(rescale[n]);
            for(int b=0;b<length;++b) {
                __m512 shifted=_mm512_maskz_sub_ps(masks[b],_mm512_load_ps(score[b]),_mm512_load_ps(maximum));
                __m512 weight=_mm512_maskz_mov_ps(masks[b],__svml_expf16_z0(shifted));
                alignas(64) float probability[16];_mm512_store_ps(probability,weight);
                _mm512_store_ps(den,_mm512_add_ps(_mm512_load_ps(den),weight));
                for(int n=0;n<batch;++n)if(unsigned(masks[b])&(1u<<n)) {
                    size_t edge=size_t(starts[n]+begin+uint64_t(b))*p.heads+h;
                    require(!visited[edge],"reference replay visited an edge twice");visited[edge]=1;
                    a.probability[edge]=probability[n];
                }
            }
        }
        for(int n=0;n<batch;++n) {
            size_t at=size_t(rows[n])*p.heads+h;a.maximum[at]=maximum[n];a.den[at]=den[n];
        }
    }
    for(auto seen:visited)require(seen==1,"reference replay omitted an edge/head");
    return a;
}

float oracle_quantize(float value);
std::vector<double> stable_fp64(const Fixture& f,bool quantized=false) {
    const auto& g=f.g;const auto& p=f.p;
    std::vector<double> z(checked(g.n,p.width()),0),out(z.size(),0);
    // Only this correctness oracle materializes tiny Z. Original edge
    // multiplicity, row IDs and independent heads are retained.
    for(uint32_t node=0;node<g.n;++node)for(int h=0;h<p.heads;++h)for(int d=0;d<p.dim;++d)
        for(int k=0;k<p.in;++k) {
            float x=g.x[size_t(node)*p.in+k],w=p.w[size_t(k)*p.width()+h*p.dim+d];
            if(quantized){x=oracle_quantize(x);w=oracle_quantize(w);}
            z[size_t(node)*p.width()+h*p.dim+d]+=double(x)*double(w);
        }
    for(uint32_t row=0;row<g.n;++row)for(int h=0;h<p.heads;++h) {
        if(g.row[row]==g.row[row+1])continue;
        double maximum=-INFINITY,den=0;
        for(uint64_t e=g.row[row];e<g.row[row+1];++e)
            maximum=std::max(maximum,double(edge_score(f,row,g.col[e],h)));
        for(uint64_t e=g.row[row];e<g.row[row+1];++e) {
            double probability=std::exp(double(edge_score(f,row,g.col[e],h))-maximum);den+=probability;
            for(int d=0;d<p.dim;++d)out[size_t(row)*p.width()+h*p.dim+d]+=
                probability*z[size_t(g.col[e])*p.width()+h*p.dim+d];
        }
        for(int d=0;d<p.dim;++d)out[size_t(row)*p.width()+h*p.dim+d]/=den;
    }
    return out;
}
// Scalar bit-level RNE is independent of production BF16 helpers/intrinsics.
float oracle_quantize(float value) {
    require(std::isfinite(value),"oracle BF16 operand finite");
    uint32_t bits;std::memcpy(&bits,&value,sizeof(bits));
    bits=(bits+0x7fffu+((bits>>16)&1u))&0xffff0000u;
    float result;std::memcpy(&result,&bits,sizeof(result));return result;
}
Simulation simulate(const Fixture& f,const Attention& a,int block,bool bf16,
                    const std::string& backend="avx_shared",int threshold=24) {
    const auto& g=f.g;const auto& p=f.p;int kblocks=(p.in+31)/32;
    Simulation s;s.numerator.assign(checked(g.n,p.width()),0);s.output.resize(s.numerator.size());
    s.raw_budget.assign(s.numerator.size(),0);s.output_budget.assign(s.numerator.size(),0);
    std::vector<float> x=g.x,w=p.w;
    if(bf16){for(auto& v:x)v=oracle_quantize(v);for(auto& v:w)v=oracle_quantize(v);}
    for(uint32_t row=0;row<g.n;++row)for(int h=0;h<p.heads;++h) {
        size_t at=size_t(row)*p.heads+h,base=size_t(row)*p.width()+h*p.dim;
        std::vector<float> u(p.in,0);std::vector<double> u_error(p.in,0);
        std::vector<double> envelope(p.dim,0),propagated_ph(p.dim,0);size_t ib=0;
        for(uint64_t begin=g.row[row];begin<g.row[row+1];begin+=uint64_t(block),++ib) {
            uint64_t end=std::min(g.row[row+1],begin+uint64_t(block));
            require(ib<a.scale[at].size(),"simulation block history length");float r=a.scale[at][ib];
            if(a.changed[at][ib])for(int d=0;d<p.dim;++d) {
                s.numerator[base+d]=multiply(s.numerator[base+d],r);
                envelope[d]*=double(r);propagated_ph[d]*=double(r);
            }
            for(int k=0;k<p.in;++k) {
                float value=0;u_error[k]=0;
                if(bf16 && (backend=="amx_hi_lo" || (backend=="hybrid" && end-begin>=uint64_t(threshold)))) {
                    // T7zero -> hi TDP -> lo TDP. Each TDP has two FP32
                    // even/odd FMA chains, then pair-add, then accumulator-add.
                    double absolute_products=0;
                    for(int component=0;component<2;++component) {
                        float even=0,odd=0;
                        for(int pair=0;pair<16;++pair)for(int lane=0;lane<2;++lane) {
                            uint64_t edge=begin+uint64_t(pair*2+lane);
                            float coeff=0,input=0;
                            if(edge<end) {
                                float prob=a.probability[size_t(edge)*p.heads+h];
                                float hi=oracle_quantize(prob);
                                coeff=component?oracle_quantize(add(prob,-hi)):hi;
                                input=x[size_t(g.col[edge])*p.in+k];
                            }
                            if(lane==0)even=std::fma(coeff,input,even);
                            else odd=std::fma(coeff,input,odd);
                            absolute_products+=std::abs(double(coeff)*double(input));
                        }
                        value=add(value,add(even,odd));
                    }
                    // Bound two legal FP32 implementations using fixed
                    // quantized PH operands. Quantization discontinuities are
                    // propagated with monotone RNE endpoints, not a fitted
                    // BF16 or master tolerance. No subnormal operands tested.
                    double e=2*gamma(20)*absolute_products;
                    float below=float(double(value)-e),above=float(double(value)+e);
                    if(double(below)>double(value)-e)below=std::nextafter(below,-INFINITY);
                    if(double(above)<double(value)+e)above=std::nextafter(above,INFINITY);
                    float qvalue=oracle_quantize(value);
                    u_error[k]=std::max(std::abs(double(oracle_quantize(below))-qvalue),
                                        std::abs(double(oracle_quantize(above))-qvalue));
                    u[k]=qvalue;
                } else {
                    // FP32 AVX shared-H preserves the original per-feature
                    // neighbor-order FMA chain and quantizes only fresh U_B.
                    for(uint64_t edge=begin;edge<end;++edge)
                        value=std::fma(a.probability[size_t(edge)*p.heads+h],
                                       x[size_t(g.col[edge])*p.in+k],value);
                    u[k]=bf16?oracle_quantize(value):value;
                }
            }
            for(int d=0;d<p.dim;++d) {
                if(bf16)for(int kb=0;kb<kblocks;++kb) {
                    float even=0,odd=0;
                    for(int pair=0;pair<16;++pair) {
                        int k=kb*32+2*pair;
                        if(k<p.in)even=std::fma(u[k],w[size_t(k)*p.width()+h*p.dim+d],even);
                        if(k+1<p.in)odd=std::fma(u[k+1],w[size_t(k+1)*p.width()+h*p.dim+d],odd);
                    }
                    s.numerator[base+d]=add(s.numerator[base+d],add(even,odd));
                } else for(int k=0;k<p.in;++k)
                    s.numerator[base+d]=std::fma(u[k],w[size_t(k)*p.width()+h*p.dim+d],s.numerator[base+d]);
                for(int k=0;k<p.in;++k) {
                    double weight=std::abs(double(w[size_t(k)*p.width()+h*p.dim+d]));
                    envelope[d]+=std::abs(double(u[k]))*weight;
                    propagated_ph[d]+=u_error[k]*weight;
                }
            }
        }
        require(ib==a.scale[at].size(),"simulation omitted block");
        float inverse=a.den[at]>0?1.f/a.den[at]:0.f;
        double chain=gamma(17),acc=gamma(a.blocks[at]*uint64_t(kblocks)+a.rescales[at]+2);
        double factor=2*(chain+acc+chain*acc+4*unit_roundoff);
        for(int d=0;d<p.dim;++d) {
            s.output[base+d]=multiply(s.numerator[base+d],inverse);
            s.raw_budget[base+d]=factor*envelope[d]+(1+factor)*propagated_ph[d];
            s.output_budget[base+d]=s.raw_budget[base+d]*double(inverse);
        }
    }
    return s;
}
void budget_check(const std::vector<float>& reference,const std::vector<double>& budget,
                  const std::vector<float>& actual,const Shape& shape,int block,const std::string& label) {
    require(reference.size()==actual.size() && budget.size()==actual.size(),"instruction budget shape");
    bool pass=true;double max_budget=0,max_fraction=0;size_t first=actual.size();
    for(size_t i=0;i<actual.size();++i) {
        double error=std::abs(double(actual[i])-reference[i]);max_budget=std::max(max_budget,budget[i]);
        double fraction=budget[i]>0?error/budget[i]:(error==0?0:INFINITY);max_fraction=std::max(max_fraction,fraction);
        if(!std::isfinite(error) || error>budget[i]){pass=false;if(first==actual.size())first=i;}
    }
    print_metric(label,shape,block,metric(reference,actual),"same_PH_UW_quantized_operand_operation_budget",pass);
    std::cout<<"ICPP_HYBRID_SMOKE_BUDGET phase="<<label<<" max_budget="<<max_budget
             <<" max_error_over_budget="<<max_fraction<<" observed_master_error_used=0\n";
    if(first<actual.size())std::cout<<"ICPP_HYBRID_SMOKE_BUDGET_MISMATCH index="<<first
        <<" node="<<first/size_t(shape.K*shape.d)<<" head="<<(first/shape.d)%shape.K
        <<" feature="<<first%shape.d<<" expected="<<reference[first]<<" actual="<<actual[first]
        <<" absolute_budget="<<budget[first]<<'\n';
    require(pass,label+" instruction emulation; this gate does not certify master precision");
}
void master_report(const Shape& shape,int block,const std::vector<double>& master,
                   const std::vector<float>& actual,const std::string& phase) {
    Metric m=metric(master,actual);bool pass=m.finite && m.max_abs<=master_abs_gate;
    print_metric(phase,shape,block,m,"master_absolute_.003_small_fixture_only",pass);
    std::cout<<"ICPP_HYBRID_SMOKE_MASTER_STATUS abs_gate="<<master_abs_gate<<" abs_gate_pass="<<pass
             <<" PH_Ublock_quantization_error_is_not_instruction_error=1 model_precision=UNVERIFIED\n";
}
void initialize(const Fixture& f,icpp_online::Workspace& w) {
    convert(f.g.x.data(),w.base.xbf.data(),f.g.x.size());w.base.left=f.left;w.base.right=f.right;
    float poison=std::numeric_limits<float>::quiet_NaN();
    std::fill(w.base.out.begin(),w.base.out.end(),poison);std::fill(w.base.max.begin(),w.base.max.end(),poison);
    std::fill(w.base.den.begin(),w.base.den.end(),poison);
    uint64_t bad=std::numeric_limits<uint64_t>::max();
    std::fill(w.row_blocks.begin(),w.row_blocks.end(),bad);
    std::fill(w.row_max_updates.begin(),w.row_max_updates.end(),bad);
    std::fill(w.row_rescales.begin(),w.row_rescales.end(),bad);
    require(w.base.z.empty() && w.base.score.empty() && w.base.p.empty() && w.base.alpha.empty(),
            "optimized path allocated full Z/score/probability");
}
void online_state(const Attention& a,const icpp_online::Workspace& w,const std::string& label,bool counters=true) {
    exact_float(a.maximum,w.base.max,label+".maximum",true);exact_float(a.den,w.base.den,label+".denominator");
    if(counters) {
        require(a.blocks==w.row_blocks,label+" row-block/scatter counters");
        require(a.updates==w.row_max_updates,label+" running maxima");
        require(a.rescales==w.row_rescales,label+" initial max counted as rescale");
    }
}
void same_state_counts(const icpp_online::Stats& a,const icpp_online::Stats& b,const std::string& label) {
#define C(field) count(a.field,b.field,label+"."#field)
    C(blocks);C(max_updates);C(rescales);C(rescaled_feature_elements);C(rescaled_padded_feature_elements);
    C(spill_bytes);C(reload_bytes);C(rescale_events);C(total_tiles);C(sampled_tiles);
#undef C
    // AMX calls/FMA/neighbor_steps intentionally differ in block projection.
}
void check_operands(const Fixture& f,const Prepared& q,const gat::Workspace& b) {
    for(size_t i=0;i<f.g.x.size();++i)
        require(expand(b.xbf[i])==oracle_quantize(f.g.x[i]),"H conversion did not use independent RNE");
    int kb_count=(f.p.in+31)/32,obs=(f.p.dim+15)/16;
    for(int h=0;h<f.p.heads;++h)for(int kb=0;kb<kb_count;++kb)
        for(int ob=0;ob<obs;++ob)for(int pair=0;pair<16;++pair)
            for(int d=0;d<16;++d)for(int lane=0;lane<2;++lane) {
                int feature=ob*16+d,k=kb*32+pair*2+lane;
                float expected=(feature<f.p.dim && k<f.p.in)?
                    oracle_quantize(f.p.w[size_t(k)*f.p.width()+h*f.p.dim+feature]):0.f;
                size_t index=(((size_t(h)*kb_count+kb)*obs+ob)*512)+pair*32+d*2+lane;
                require(expand(q.packed[index])==expected,"W VNNI packing/RNE oracle mismatch");
            }
}
void counters(const Fixture& f,const Schedule& schedule,const Attention& a,
              const Workspace& w,const Stats& stats,int block,int threshold) {
    const auto& g=f.g;const auto& p=f.p;const auto& b=stats.baseline;
    uint64_t blocks=0,updates=0,rescales=0,events=0,projection_calls=0,reloads=0;
    for(uint32_t row=0;row<g.n;++row)for(int h=0;h<p.heads;++h) {
        size_t i=size_t(row)*p.heads+h;uint64_t degree=g.row[row+1]-g.row[row];
        count((degree+block-1)/block,a.blocks[i],"row_degree_block_coverage");
        if(degree) {
            require(a.updates[i]>=1 && a.updates[i]<=a.blocks[i],"nonempty row maximum updates");
            count(a.updates[i]-1,a.rescales[i],"first_maximum_not_a_rescale");
        } else {
            count(0,a.updates[i],"empty.updates");count(0,a.rescales[i],"empty.rescales");
            require(a.den[i]==0.f && a.maximum[i]==-INFINITY,"empty row m/l state");
        }
        blocks+=a.blocks[i];updates+=a.updates[i];rescales+=a.rescales[i];
    }
    for(uint64_t tile=0;tile<g.n;tile+=16) {
        uint64_t end=std::min(g.n,tile+16),degree=0;
        for(uint64_t n=tile;n<end;++n) {
            uint32_t row=schedule.perm[n];degree=std::max(degree,g.row[row+1]-g.row[row]);
        }
        uint64_t tile_blocks=(degree+block-1)/block;
        projection_calls+=uint64_t(p.heads)*tile_blocks;
        if(tile_blocks)reloads+=uint64_t(p.heads)*(tile_blocks-1);
        for(int h=0;h<p.heads;++h)for(uint64_t ib=0;ib<tile_blocks;++ib) {
            bool changed=false;
            for(uint64_t n=tile;n<end;++n) {
                const auto& history=a.changed[size_t(schedule.perm[n])*p.heads+h];
                if(ib<history.size() && history[size_t(ib)])changed=true;
            }
            if(changed)++events;
        }
    }
    uint64_t dp=uint64_t((p.in+31)/32)*32,edge_heads=uint64_t(checked(g.e,p.heads));
    require(stats.optimized,"K8/d32 fixture did not take optimized backend");
    count(blocks,b.blocks,"blocks");count(updates,b.max_updates,"max_updates");count(rescales,b.rescales,"rescales");
    count(rescales*32,b.rescaled_feature_elements,"rescaled_features");
    count(rescales*32,b.rescaled_padded_feature_elements,"padded_rescaled_features");
    count(events,b.rescale_events,"rescale_events");
    // V is already in worker-local memory because eight independent heads
    // exceed the TMM register budget. Extra rescale spill/reload is zero.
    count(0,b.spill_bytes,"extra_rescale_spill");count(0,b.reload_bytes,"extra_rescale_reload");
    count(projection_calls,stats.projection_calls,"projection_calls");
    count(projection_calls,b.neighbor_steps,"UW_steps");
    count(projection_calls*(dp/32)*2,b.amx_calls,"UW_TDP_instructions");
    count(projection_calls*16*dp*32,b.executed_fma,"UW_physical_FMA");
    count(projection_calls*2048,stats.head_switch_store_bytes,"head_context_store_bytes");
    count(reloads*2048,stats.head_switch_reload_bytes,"head_context_reload_bytes");
    count(edge_heads*uint64_t(p.in),stats.ph_useful_fma,"PH_valid_FMA");
    count(g.e*uint64_t(p.in)*sizeof(BF16),stats.raw_H_bytes,"shared_source_H_bytes");
    uint64_t amx_blocks=0,avx_blocks=0,avx_edges=0;
    for(uint64_t row=0;row<g.n;++row) {
        uint64_t degree=g.row[row+1]-g.row[row];
        for(uint64_t begin=0;begin<degree;begin+=uint64_t(block)) {
            uint64_t valid=std::min<uint64_t>(block,degree-begin);
            if(valid>=uint64_t(threshold))++amx_blocks;
            else {++avx_blocks;avx_edges+=valid;}
        }
    }
    count(blocks/uint64_t(p.heads),amx_blocks+avx_blocks,"independent_dispatch_rowblocks");
    uint64_t avx_dim=uint64_t((p.in+15)/16)*16;
    count(amx_blocks*(dp/16)*2,stats.ph_calls,"mixed_PH_TDP_instructions");
    count(amx_blocks*8*32*dp*2+avx_edges*8*avx_dim,stats.ph_physical_fma,"mixed_PH_physical_FMA");
    count(amx_blocks*32*dp*sizeof(BF16),stats.packed_H_bytes,"mixed_padded_packed_H_bytes");
    count(amx_blocks*8*dp*sizeof(float)+avx_blocks*8*avx_dim*sizeof(float),
          stats.ub_scatter_bytes,"mixed_U_scatter_bytes");
    std::cout<<"ICPP_HYBRID_DISPATCH threshold="<<threshold<<" amx_rowblocks="<<amx_blocks
             <<" avx_rowblocks="<<avx_blocks<<" avx_edges="<<avx_edges<<" counter_dispatch_independent=1\n";
    count(uint64_t(p.heads)*((g.n+15)/16),b.total_tiles,"head_tiles");
    count(checked(w.fallback.original.thread_capacity,checked(8*16,dp)),w.ubuf.size(),"bounded_U_B");
    count(checked(w.fallback.original.thread_capacity,8*512),w.vbuf.size(),"bounded_V_contexts");
    count(checked(w.fallback.original.thread_capacity,32*dp),w.bbuf.size(),"bounded_packed_H");
    online_state(a,w.fallback.original,"reference_vs_heads");
}
void same_all_counts(const Stats& a,const Stats& b,const std::string& label,bool compare_samples=true) {
    auto aa=a.baseline,bb=b.baseline;
    if(!compare_samples)aa.sampled_tiles=bb.sampled_tiles=0;
    same_state_counts(aa,bb,label);
#define C(field) count(a.field,b.field,label+"."#field)
    C(ph_calls);C(ph_physical_fma);C(ph_useful_fma);C(raw_H_bytes);C(packed_H_bytes);
    C(projection_calls);C(head_switch_store_bytes);C(head_switch_reload_bytes);C(ub_scatter_bytes);
#undef C
    count(aa.amx_calls,bb.amx_calls,label+".amx_calls");
    count(aa.neighbor_steps,bb.neighbor_steps,label+".neighbor_steps");
    count(aa.executed_fma,bb.executed_fma,label+".executed_fma");
    require(a.optimized==b.optimized,label+" backend selection mismatch");
}
void profile_controls(const Fixture& f,const Prepared& q,const Schedule& schedule,int block,
                      const Snapshot& raw,const std::vector<float>& normalized,const Stats& plain,
                      int threshold) {
    for(int period:{0,1})for(bool enabled:{false,true}) {
        Workspace w;w.allocate(f.g,f.p,enabled);initialize(f,w.fallback.original);Stats stats;
        gat::icpp_heads_hybrid::aggregate(f.g,f.p,q,schedule,w,block,32,stats,period,enabled,threshold);
        std::string label="T"+std::to_string(threshold)+"_profile_"+std::to_string(period)+
                          "_counters_"+std::to_string(int(enabled));
        exact_float(raw.numerator,w.fallback.original.base.out,label+".numerator");
        exact_float(raw.den,w.fallback.original.base.den,label+".denominator");
        exact_float(raw.maximum,w.fallback.original.base.max,label+".maximum",true);
        if(enabled) {
            same_all_counts(plain,stats,label,false);
            count(period?plain.baseline.total_tiles:0,stats.baseline.sampled_tiles,label+".sampled_tiles");
        } else {
            Stats expected;expected.optimized=plain.optimized;
            if(period){expected.baseline.total_tiles=plain.baseline.total_tiles;expected.baseline.sampled_tiles=plain.baseline.total_tiles;}
            same_all_counts(expected,stats,label);
        }
        gat::Times t;finish(f.g,f.p,w.fallback.original.base,false,t);
        exact_float(normalized,w.fallback.original.base.out,label+".normalized");
    }
}
void check_case(const Fixture& f,Shape shape,int block,bool forced,bool profile,
                const std::string& label,int threshold) {
    std::string phase="T"+std::to_string(threshold)+"_"+label;
    const auto master=stable_fp64(f);const auto scalar=scalar_attention(f,block);
    const auto fp32=simulate(f,scalar,block,false);Metric math=metric(master,fp32.output);
    bool math_pass=math.finite && math.max_abs<=fp32_abs_gate && math.relative_l2<=fp32_rel_gate;
    print_metric(phase+"_FP32_BLOCK_UW_VS_STABLE_FP64",shape,block,math,"abs_1e-5_relative_L2_1e-5_before_AMX",math_pass);
    require(math_pass,"FP32 block-UW mathematical control before AMX");
    Prepared q=prepare(f.p,true,true,true);Schedule schedule=degree_schedule(f.g);check_schedule(f.g,schedule);
    const auto matched=matched_attention(f,schedule,block);
    // Independent per-original-row, per-neighbor-block branch selection is
    // based exclusively on CSR degree and threshold. No production PH output
    // or branch/statistics array is consumed by this opcode oracle.
    const auto emulator=simulate(f,matched,block,true,"hybrid",threshold);
    icpp_block::Workspace old;old.allocate(f.g,f.p,true);initialize(f,old.original);
    icpp_block::Stats old_stats;icpp_block::aggregate(f.g,f.p,q,schedule,old,block,32,old_stats,0,true);
    online_state(matched,old.original,"matched_SVML_vs_old_block");
    Workspace actual;actual.allocate(f.g,f.p,true);initialize(f,actual.fallback.original);
    std::fill(actual.ubuf.begin(),actual.ubuf.end(),std::numeric_limits<float>::quiet_NaN());
    std::fill(actual.vbuf.begin(),actual.vbuf.end(),std::numeric_limits<float>::quiet_NaN());
    std::fill(actual.bbuf.begin(),actual.bbuf.end(),BF16(0xffff));
    check_operands(f,q,actual.fallback.original.base);
    Stats stats;gat::icpp_heads_hybrid::aggregate(f.g,f.p,q,schedule,actual,block,32,stats,0,true,threshold);
    exact_float(old.original.base.max,actual.fallback.original.base.max,phase+"_old.maximum",true);
    exact_float(old.original.base.den,actual.fallback.original.base.den,phase+"_old.denominator");
    if(threshold>block)
        exact_float(old.original.base.out,actual.fallback.original.base.out,"all_AVX_vs_old_block.numerator");
    if(threshold==1) {
        Workspace amx;amx.allocate(f.g,f.p,true);initialize(f,amx.fallback.original);Stats as;
        gat::icpp_heads_preload::aggregate(f.g,f.p,q,schedule,amx,block,32,as,0,true,"amx_hi_lo");
        exact_float(amx.fallback.original.base.out,actual.fallback.original.base.out,"all_AMX_vs_preload.numerator");
        same_all_counts(as,stats,"all_AMX_vs_preload.counts");
    }
    counters(f,schedule,matched,actual,stats,block,threshold);
    budget_check(emulator.numerator,emulator.raw_budget,actual.fallback.original.base.out,shape,block,phase+"_NUMERATOR_VS_MIXED_EMULATOR");
    Snapshot raw{actual.fallback.original.base.out,actual.fallback.original.base.max,actual.fallback.original.base.den};
    gat::Times t;finish(f.g,f.p,actual.fallback.original.base,false,t);
    budget_check(emulator.output,emulator.output_budget,actual.fallback.original.base.out,shape,block,phase+"_OUTPUT_VS_MIXED_EMULATOR");
    master_report(shape,block,master,actual.fallback.original.base.out,phase+"_OUTPUT_VS_FP32_MASTER");
    if(forced) {
        require(matched.rescales[size_t(2)*f.p.heads]>0,"fixture failed to force later running maximum");
        count(0,matched.rescales[size_t(4)*f.p.heads],"descending.no_rescale");
        count(0,matched.rescales[size_t(2)*f.p.heads+1],"independent_head.no_rescale");
    } else if(label=="ALL_EQUAL_SCORES")count(0,stats.baseline.rescales,"equal.rescales");
    if(profile)profile_controls(f,q,schedule,block,raw,actual.fallback.original.base.out,stats,threshold);
    std::cout<<"ICPP_HYBRID_SMOKE_CASE_CONTROL_PASS label="<<label<<" threshold="<<threshold
             <<" PH_useful_FMA="<<stats.ph_useful_fma<<" PH_physical_FMA="<<stats.ph_physical_fma
             <<" UW_physical_FMA="<<stats.baseline.executed_fma<<" projection_calls="<<stats.projection_calls
             <<" head_context_store_bytes="<<stats.head_switch_store_bytes
             <<" head_context_reload_bytes="<<stats.head_switch_reload_bytes
             <<" all_eight_V_TMM_resident=0 master_precision_certification=0\n";
}
void fallback_case(Shape shape,uint32_t seed,int block,int threshold) {
    Fixture f=fixture(shape,seed);Prepared q=prepare(f.p,true,true,true);Schedule schedule=degree_schedule(f.g);
    icpp_block::Workspace old;old.allocate(f.g,f.p,true);initialize(f,old.original);
    icpp_block::Stats os;icpp_block::aggregate(f.g,f.p,q,schedule,old,block,32,os,0,true);
    Workspace actual;actual.allocate(f.g,f.p,true);initialize(f,actual.fallback.original);
    require(actual.ubuf.empty() && actual.vbuf.empty() && actual.bbuf.empty(),"fallback allocated optimized scratch");
    Stats as;gat::icpp_heads_hybrid::aggregate(f.g,f.p,q,schedule,actual,block,32,as,0,true,threshold);
    require(!as.optimized,"unsupported shape failed to select original block fallback");
    exact_float(old.original.base.out,actual.fallback.original.base.out,"fallback.raw_output");
    exact_float(old.original.base.max,actual.fallback.original.base.max,"fallback.maximum",true);
    exact_float(old.original.base.den,actual.fallback.original.base.den,"fallback.denominator");
    require(old.original.row_blocks==actual.fallback.original.row_blocks,"fallback row blocks");
    require(old.original.row_max_updates==actual.fallback.original.row_max_updates,"fallback row maxima");
    require(old.original.row_rescales==actual.fallback.original.row_rescales,"fallback row rescales");
    same_state_counts(os.baseline,as.baseline,"fallback.state_counts");
    count(os.baseline.amx_calls,as.baseline.amx_calls,"fallback.UW_AMX_calls");
    count(os.baseline.executed_fma,as.baseline.executed_fma,"fallback.UW_FMA");
    count(os.baseline.neighbor_steps,as.baseline.neighbor_steps,"fallback.UW_steps");
    count(os.block_projection_calls,as.projection_calls,"fallback.projection_calls");
    count(0,as.ph_calls,"fallback.new_PH_calls");count(0,as.ph_physical_fma,"fallback.new_PH_FMA");
    count(0,as.head_switch_store_bytes,"fallback.new_head_context_store");
    const auto matched=matched_attention(f,schedule,block);const auto emulator=simulate(f,matched,block,true);
    online_state(matched,actual.fallback.original,"fallback.independent_state");
    budget_check(emulator.numerator,emulator.raw_budget,actual.fallback.original.base.out,shape,block,"FALLBACK_INDEPENDENT_BLOCK_EMULATOR");
    Snapshot raw{actual.fallback.original.base.out,actual.fallback.original.base.max,actual.fallback.original.base.den};
    gat::Times ft;finish(f.g,f.p,actual.fallback.original.base,false,ft);
    master_report(shape,block,stable_fp64(f),actual.fallback.original.base.out,"FALLBACK_MASTER");
    profile_controls(f,q,schedule,block,raw,actual.fallback.original.base.out,as,threshold);
    std::cout<<"ICPP_HYBRID_SMOKE_FALLBACK_CONTROL_PASS N="<<shape.n<<" D="<<shape.D
             <<" K="<<shape.K<<" d="<<shape.d<<" block="<<block<<" threshold="<<threshold<<'\n';
}
void layer_case(Shape shape,uint32_t seed,int threshold) {
    Fixture f=fixture(shape,seed);Prepared q=prepare(f.p,true,true,true);Schedule schedule=degree_schedule(f.g);
    icpp_online::Workspace lr;lr.allocate(f.g,f.p,true);
    lr_reordered(f.g,f.p,q,f.g.x,lr.base,"fp32");f.left=lr.base.left;f.right=lr.base.right;
    check_case(f,shape,32,false,false,"REAL_CONTRACTED_LR",threshold);
    Workspace plain;plain.allocate(f.g,f.p,true);Timing pt;Stats ps;
    gat::icpp_heads_hybrid::layer(f.g,f.p,q,schedule,f.g.x,plain,false,32,32,pt,ps,0,true,"fp32",threshold);
    exact_float(f.left,plain.fallback.original.base.left,"layer.left");exact_float(f.right,plain.fallback.original.base.right,"layer.right");
    const auto attention=matched_attention(f,schedule,32);
    const auto emulator=simulate(f,attention,32,true,"hybrid",threshold);
    budget_check(emulator.output,emulator.output_budget,plain.fallback.original.base.out,shape,32,"PRODUCTION_LAYER_WRAPPER");
    counters(f,schedule,attention,plain,ps,32,threshold);
    Workspace hidden;hidden.allocate(f.g,f.p,true);Timing ht;Stats hs;
    gat::icpp_heads_hybrid::layer(f.g,f.p,q,schedule,f.g.x,hidden,true,32,32,ht,hs,0,true,"fp32",threshold);
    gat::Workspace activated;
    activated.out=plain.fallback.original.base.out;activated.den.assign(checked(f.g.n,f.p.heads),1.f);
    gat::Times at;finish(f.g,f.p,activated,true,at);
    exact_float(activated.out,hidden.fallback.original.base.out,"production.ELU_after_normalization");
    same_all_counts(ps,hs,"hidden_vs_plain.counters");
    std::cout<<"ICPP_HYBRID_SMOKE_LAYER_CONTROL_PASS hidden_activation=ELU lr_policy=fp32 threshold="<<threshold<<'\n';
}
void fallback_layer(Shape shape,uint32_t seed,int threshold) {
    Fixture f=fixture(shape,seed);Prepared q=prepare(f.p,true,true,true);Schedule schedule=degree_schedule(f.g);
    for(bool hidden:{false,true}) {
        icpp_block::Workspace old;old.allocate(f.g,f.p,true);icpp_block::Timing ot;icpp_block::Stats os;
        icpp_block::layer(f.g,f.p,q,schedule,f.g.x,old,hidden,32,32,ot,os,0,true,"fp32");
        Workspace actual;actual.allocate(f.g,f.p,true);Timing t;Stats s;
        gat::icpp_heads_hybrid::layer(f.g,f.p,q,schedule,f.g.x,actual,hidden,32,32,t,s,0,true,"fp32",threshold);
        require(!s.optimized,"fallback layer shape dispatch");
        exact_float(old.original.base.left,actual.fallback.original.base.left,"fallback_layer.left");
        exact_float(old.original.base.right,actual.fallback.original.base.right,"fallback_layer.right");
        exact_float(old.original.base.out,actual.fallback.original.base.out,"fallback_layer.output");
        exact_float(old.original.base.max,actual.fallback.original.base.max,"fallback_layer.maximum",true);
        exact_float(old.original.base.den,actual.fallback.original.base.den,"fallback_layer.den");
        same_state_counts(os.baseline,s.baseline,"fallback_layer.stats");
        std::cout<<"ICPP_HYBRID_FALLBACK_LAYER_PASS K="<<shape.K<<" d="<<shape.d
                 <<" hidden="<<hidden<<" threshold="<<threshold<<'\n';
    }
}
}
void run_hybrid_smoke() {
    std::cout<<std::setprecision(17);
    std::cout<<"ICPP_HYBRID_SMOKE_BEGIN fp32_abs_gate="<<fp32_abs_gate
        <<" fp32_relative_L2_gate="<<fp32_rel_gate<<" unchanged_master_abs_gate="<<master_abs_gate
        <<" branch_oracle=original_row_actual_block_neighbor_count"
        <<" short_PH=FP32_neighbor_order_FMA long_PH=hi_then_lo_TDP"
        <<" BF16_interface=RNE_H_W_then_mixed_PH_then_RNE_Ublock"
        <<" instruction_emulator=TDPBF16PS_even_odd_FP32_FMA_chains"
        <<" quantization_bound=monotone_RNE_opcode_error_interval_endpoints"
        <<" matched_exp_entry=__svml_expf16_z0 internal_ABI_scope=checks_only"
        <<" production_exp_symbol_requires_assembly_audit=1"
        <<" tiny_reference_edge_materialization=checks_only subnormals=excluded"
        <<" DAZ_FTZ_not_tested=1 model_precision=UNVERIFIED trained_accuracy=UNVERIFIED\n";
    constexpr Shape shapes[]={{17,17,8,32},{35,33,8,32},{17,100,8,32},
                              {35,128,8,32},{17,256,8,32},{35,256,8,32}};
    size_t cases=0;
    for(size_t i=0;i<sizeof(shapes)/sizeof(shapes[0]);++i) {
        Fixture f=fixture(shapes[i],0x48594252u+uint32_t(i));
        for(int block:{16,32})for(int threshold:{16,24,32}) {
            check_case(f,shapes[i],block,true,true,"FIXED_INDEPENDENT_LR",threshold);++cases;
        }
    }
    Shape equal_shape{35,100,8,32};Fixture equal=fixture(equal_shape,0x48594551u,true);
    for(int block:{16,32})for(int threshold:{16,24,32}) {
        check_case(equal,equal_shape,block,false,true,"ALL_EQUAL_SCORES",threshold);++cases;
    }
    Shape endpoint_shape{35,256,8,32};Fixture endpoint=fixture(endpoint_shape,0x48594550u);
    for(int block:{16,32})for(int threshold:{1,33}) {
        check_case(endpoint,endpoint_shape,block,true,true,"ENDPOINT_BITWISE_REGRESSION",threshold);++cases;
    }
    for(int threshold:{16,24,32}) {
        fallback_case({17,100,1,47},0x48594631u,32,threshold);
        fallback_case({35,33,2,7},0x48594632u,16,threshold);
        layer_case({17,128,8,32},0x48594c31u,threshold);
    }
    fallback_layer({35,256,1,47},0x48594c32u,24);
    std::cout<<"ICPP_HYBRID_SMOKE_CONTROL_PASS operator_cases="<<cases
        <<" fallback_controls=6 real_LR_layer_controls=3 fallback_layer_controls=2"
        <<" profile_counter_combinations_per_operator=4"
        <<" FP32_math_gate_passed=1 independent_mixed_quantized_instruction_gate_passed=1"
        <<" master_gate_reported_separately=1 master_precision_certification=0"
        <<" 8x_D_wide_aggregation_removed=0 logical_bytes_are_DRAM_measurement=0"
        <<" eight_heads_all_TMM_resident=0 profile_fields=sampled_worker_seconds"
        <<" trained_model_accuracy=UNVERIFIED\n";
}
}
