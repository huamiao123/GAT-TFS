#include "preload_checks.hpp"
#include "icpp_heads_preload.hpp"
#include <iomanip>
#include <iostream>
#include <random>

namespace gat::icpp_heads_preload {
namespace {
namespace old=gat::icpp_heads;
constexpr const char* method="amx_hi_lo";
struct Shape {int n,D,K,d;};
struct Fixture {Graph g;Param p;std::vector<float> left,right;};
struct Snapshot {
    std::vector<float> numerator,maximum,den;
    std::vector<uint64_t> blocks,updates,rescales;
    Stats stats;
};
void require(bool ok,const std::string& why) {
    if(!ok)throw std::runtime_error("ICPP preload regression: "+why);
}
void count(uint64_t ref,uint64_t got,const std::string& field) {
    if(ref!=got) {
        std::cout<<"ICPP_PRELOAD_COUNT_MISMATCH field="<<field<<" expected="<<ref<<" actual="<<got<<'\n';
        require(false,field+" work counter mismatch");
    }
}
void exact(const std::vector<float>& ref,const std::vector<float>& got,
           const std::string& label,bool allow_infinity=false) {
    require(ref.size()==got.size(),label+" shape");
    for(size_t i=0;i<ref.size();++i) {
        bool equal=std::memcmp(&ref[i],&got[i],sizeof(float))==0;
        bool finite=std::isfinite(ref[i])&&std::isfinite(got[i]);
        bool allowed=allow_infinity&&equal&&std::isinf(ref[i]);
        if(!equal || (!finite&&!allowed)) {
            uint32_t a,b;std::memcpy(&a,&ref[i],4);std::memcpy(&b,&got[i],4);
            std::cout<<std::setprecision(17)<<"ICPP_PRELOAD_BIT_MISMATCH label="<<label
              <<" index="<<i<<" expected="<<ref[i]<<" actual="<<got[i]
              <<" expected_bits="<<a<<" actual_bits="<<b<<'\n';
            require(false,label+" must remain bitwise equal; no tolerance relaxation");
        }
    }
}
void same_counts(const Stats& ref,const Stats& got,const std::string& label) {
    require(ref.optimized==got.optimized,label+" shape dispatch");
#define B(field) count(ref.baseline.field,got.baseline.field,label+".baseline."#field)
    B(blocks);B(max_updates);B(rescales);B(rescaled_feature_elements);
    B(rescaled_padded_feature_elements);B(spill_bytes);B(reload_bytes);B(rescale_events);
    B(amx_calls);B(neighbor_steps);B(executed_fma);B(sampled_tiles);B(total_tiles);
#undef B
#define C(field) count(ref.field,got.field,label+"."#field)
    C(ph_calls);C(ph_physical_fma);C(ph_useful_fma);C(raw_H_bytes);C(packed_H_bytes);
    C(projection_calls);C(head_switch_store_bytes);C(head_switch_reload_bytes);C(ub_scatter_bytes);
#undef C
    // Timers measure different spans after probability tile preload. They
    // must be finite/nonnegative but are not numeric work invariants.
}
void valid_timers(const Stats& got,const std::string& label) {
    auto valid=[&](double value,const char* name){require(std::isfinite(value)&&value>=0,label+" timer "+name);};
#define B(field) valid(got.baseline.field,#field)
    B(score_generation);B(block_max);B(exp_den);B(rescale_store);B(rescale_vector);
    B(rescale_reload);B(source_gather);B(weighted_convert);B(tile_load);B(amx_compute);
    B(final_store);B(output_write);B(scheduling);B(tile_config);
#undef B
#define C(field) valid(got.field,#field)
    C(source_index);C(H_pack);C(P_pack);C(PH_load);C(PH_compute);C(PH_store_scatter);
    C(UB_convert);C(UW_load);C(UW_compute);C(V_store);C(V_reload);C(row_rescale);C(transition);
#undef C
}
Fixture fixture(const Shape& s,uint32_t seed,bool equal=false) {
    Fixture f;auto& g=f.g;auto& p=f.p;
    g.n=s.n;g.din=s.D;g.classes=s.d;p.in=s.D;p.heads=s.K;p.dim=s.d;
    std::mt19937 rng(seed);std::uniform_real_distribution<float> uniform(-1.f,1.f);
    g.x.resize(checked(g.n,p.in));p.w.resize(checked(p.in,p.width()));
    p.al.resize(p.width());p.ar.resize(p.width());
    for(auto& x:g.x)x=.37f*uniform(rng);
    for(auto& x:p.w)x=.41f*uniform(rng)/std::sqrt(float(p.in));
    for(auto& x:p.al)x=.33f*uniform(rng)/std::sqrt(float(p.dim));
    for(auto& x:p.ar)x=.29f*uniform(rng)/std::sqrt(float(p.dim));
    f.left.resize(checked(g.n,p.heads));f.right.resize(f.left.size());
    for(uint32_t n=0;n<g.n;++n)for(int h=0;h<p.heads;++h) {
        size_t at=size_t(n)*p.heads+h;float position=float(n)/float(g.n-1);
        f.left[at]=equal?0.f:.12f*uniform(rng)+.019f*h;
        f.right[at]=equal?0.f:(h==0?1.1f*position-.55f:
            (h==1?-1.f*position+.5f:.61f*uniform(rng)+.027f*h));
    }
    constexpr uint64_t degree_set[]={0,1,15,16,17,31,32,33,63,64,65,96,97,129,130,257,273};
    g.row.push_back(0);
    for(uint32_t n=0;n<g.n;++n) {
        uint64_t degree=degree_set[(n*7)%17];
        if(n==0||n==6)degree=0;
        if(n==2||n==4||n+1==g.n)degree=129;
        for(uint64_t e=0;e<degree;++e) {
            uint32_t source;
            if(n==2||n+1==g.n||n%4==1)source=uint32_t(std::min<uint64_t>(g.n-1,e/8));
            else if(n==4||n%4==2)source=uint32_t(g.n-1-std::min<uint64_t>(g.n-1,e/8));
            else source=uint32_t((e*13+n*5+(e/17)*3)%g.n);
            g.col.push_back(source);
        }
        g.row.push_back(g.col.size());
    }
    g.e=g.col.size();g.validate();p.validate();return f;
}
void initialize(const Fixture& f,Workspace& w) {
    auto& o=w.fallback.original;auto& b=o.base;
    convert(f.g.x.data(),b.xbf.data(),f.g.x.size());b.left=f.left;b.right=f.right;
    float poison=std::numeric_limits<float>::quiet_NaN();
    std::fill(b.out.begin(),b.out.end(),poison);std::fill(b.max.begin(),b.max.end(),poison);
    std::fill(b.den.begin(),b.den.end(),poison);
    std::fill(w.ubuf.begin(),w.ubuf.end(),poison);std::fill(w.vbuf.begin(),w.vbuf.end(),poison);
    std::fill(w.bbuf.begin(),w.bbuf.end(),BF16(0xffff));
    uint64_t bad=std::numeric_limits<uint64_t>::max();
    std::fill(o.row_blocks.begin(),o.row_blocks.end(),bad);
    std::fill(o.row_max_updates.begin(),o.row_max_updates.end(),bad);
    std::fill(o.row_rescales.begin(),o.row_rescales.end(),bad);
    require(b.z.empty()&&b.score.empty()&&b.p.empty()&&b.alpha.empty(),"new preload allocated global projection/edge arrays");
}
Snapshot capture(const Workspace& w,const Stats& stats) {
    const auto& o=w.fallback.original;
    return {o.base.out,o.base.max,o.base.den,o.row_blocks,o.row_max_updates,o.row_rescales,stats};
}
void compare_state(const Snapshot& ref,const Workspace& w,const std::string& label,bool counters) {
    const auto& o=w.fallback.original;
    exact(ref.numerator,o.base.out,label+".output");exact(ref.maximum,o.base.max,label+".maximum",true);
    exact(ref.den,o.base.den,label+".denominator");
    if(counters) {
        require(ref.blocks==o.row_blocks,label+" per-row blocks");
        require(ref.updates==o.row_max_updates,label+" independent-head maximum updates");
        require(ref.rescales==o.row_rescales,label+" independent-head rescales");
    }
}
Stats expected_counts(const Stats& baseline,int period,bool counters) {
    Stats expected=baseline;
    if(!counters) {expected=Stats{};expected.optimized=baseline.optimized;}
    expected.baseline.total_tiles=(counters||period)?baseline.baseline.total_tiles:0;
    expected.baseline.sampled_tiles=period?baseline.baseline.total_tiles:0;
    return expected;
}
void check_case(const Shape& shape,uint32_t seed,int block,bool equal=false) {
    Fixture f=fixture(shape,seed,equal);Prepared q=prepare(f.p,true,true,true);Schedule sched=degree_schedule(f.g);
    Workspace ref;ref.allocate(f.g,f.p,true);initialize(f,ref);Stats ref_stats;
    old::aggregate(f.g,f.p,q,sched,ref,block,32,ref_stats,0,true,method);
    const Snapshot raw=capture(ref,ref_stats);
    if(!equal&&shape.K==8&&shape.d==32) {
        require(raw.rescales[size_t(2)*shape.K]>0,"fixture lacks later running-max updates");
        count(0,raw.rescales[size_t(4)*shape.K],"descending head0 rescale");
        count(0,raw.rescales[size_t(2)*shape.K+1],"ascending source but head1 descending rescale");
    }
    if(equal)count(0,ref_stats.baseline.rescales,"equal scores never rescale");
    gat::Times ft;finish(f.g,f.p,ref.fallback.original.base,false,ft);
    const auto normalized=ref.fallback.original.base.out;
    for(int period:{0,1})for(bool enabled:{false,true}) {
        Workspace actual;actual.allocate(f.g,f.p,enabled);initialize(f,actual);Stats stats;
        gat::icpp_heads_preload::aggregate(f.g,f.p,q,sched,actual,block,32,stats,period,enabled,method);
        std::string label="D"+std::to_string(shape.D)+"_N"+std::to_string(shape.n)+"_K"+
            std::to_string(shape.K)+"_B"+std::to_string(block)+"_profile"+std::to_string(period)+
            "_counters"+std::to_string(int(enabled));
        compare_state(raw,actual,label,enabled);
        same_counts(expected_counts(ref_stats,period,enabled),stats,label);
        valid_timers(stats,label);
        gat::Times t;finish(f.g,f.p,actual.fallback.original.base,false,t);
        exact(normalized,actual.fallback.original.base.out,label+".normalized");
    }
    std::cout<<"ICPP_PRELOAD_OPERATOR_PASS N="<<shape.n<<" D="<<shape.D<<" K="<<shape.K
      <<" d="<<shape.d<<" block="<<block<<" equal_scores="<<equal
      <<" optimized="<<ref_stats.optimized<<" profile_counter_variants=4"
      <<" raw_numerator_max_den_bitwise_equal=1 work_counts_equal=1 timings_equal_required=0\n";
}
void layer_case(const Shape& shape,uint32_t seed,int block) {
    Fixture f=fixture(shape,seed);Prepared q=prepare(f.p,true,true,true);Schedule sched=degree_schedule(f.g);
    for(bool hidden:{false,true}) {
        Workspace ref;ref.allocate(f.g,f.p,true);Timing reference_time;Stats reference_stats;
        old::layer(f.g,f.p,q,sched,f.g.x,ref,hidden,block,32,reference_time,reference_stats,0,true,"fp32",method);
        const auto reference=capture(ref,reference_stats);
        const auto left=ref.fallback.original.base.left,right=ref.fallback.original.base.right;
        for(int period:{0,1})for(bool enabled:{false,true}) {
            Workspace actual;actual.allocate(f.g,f.p,enabled);initialize(f,actual);Timing t;Stats s;
            gat::icpp_heads_preload::layer(f.g,f.p,q,sched,f.g.x,actual,hidden,block,32,t,s,period,enabled,"fp32",method);
            std::string label="layer_K"+std::to_string(shape.K)+"_d"+std::to_string(shape.d)+
                "_hidden"+std::to_string(int(hidden))+"_profile"+std::to_string(period)+
                "_counters"+std::to_string(int(enabled));
            compare_state(reference,actual,label,enabled);
            exact(left,actual.fallback.original.base.left,label+".contracted_L");
            exact(right,actual.fallback.original.base.right,label+".contracted_R");
            same_counts(expected_counts(reference_stats,period,enabled),s,label);valid_timers(s,label);
            require(std::isfinite(t.total)&&t.total>=0,"layer wall timer");
        }
        std::cout<<"ICPP_PRELOAD_LAYER_PASS N="<<shape.n<<" D="<<shape.D<<" K="<<shape.K
          <<" d="<<shape.d<<" block="<<block<<" hidden_ELU="<<hidden
          <<" contracted_LR_FP32=1 output_max_den_bitwise_equal=1 profile_counter_variants=4\n";
    }
}
}
void run_preload_smoke() {
    std::cout<<std::setprecision(17)<<"ICPP_PRELOAD_SMOKE_BEGIN prior_independent_heads_checks_required=1"
      <<" regression=bitwise_no_arithmetic_change PH_probability_tiles=hi_TMM6_lo_TMM2"
      <<" timers_are_not_work_counts=true unchanged_master_abs_gate=.003"
      <<" master_precision_certification=0 model_accuracy=UNVERIFIED\n";
    constexpr Shape shapes[]={{17,17,8,32},{35,33,8,32},{17,100,8,32},
                              {35,128,8,32},{17,256,8,32},{35,256,8,32}};
    size_t cases=0;
    for(size_t i=0;i<sizeof(shapes)/sizeof(shapes[0]);++i)
        for(int block:{16,32}) {check_case(shapes[i],0x5052454cu+uint32_t(i),block);++cases;}
    for(int block:{16,32}) {check_case({35,100,8,32},0x50524545u,block,true);++cases;}
    for(int block:{16,32}) {
        check_case({17,256,1,47},0x46414c4cu,block);++cases;
        check_case({35,33,2,7},0x46414c32u,block);++cases;
    }
    layer_case({17,100,8,32},0x4c505231u,16);
    layer_case({35,256,8,32},0x4c505232u,32);
    layer_case({17,256,1,47},0x4c505233u,32);
    std::cout<<"ICPP_PRELOAD_SMOKE_CONTROL_PASS operator_cases="<<cases
      <<" layer_controls=6 profile_counter_variants_per_case=4"
      <<" old_new_numerator_max_den_bitwise_equal=1 all_work_counts_equal=1"
      <<" PH_load_timer_numeric_equality_required=0 precision_semantics_unchanged=1"
      <<" master_precision_certification=0 trained_accuracy=UNVERIFIED\n";
}
}
