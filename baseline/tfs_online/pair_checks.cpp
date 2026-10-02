#include "pair_checks.hpp"
#include "icpp_pair.hpp"
#include <iomanip>
#include <iostream>
#include <random>

namespace gat::icpp_pair {
namespace {
struct Shape { int nodes,input,heads,dim; };
struct TestFixture {
    Graph graph;
    Param param;
    std::vector<float> left,right;
};
struct Snapshot {
    std::vector<float> out,den,maximum;
    std::vector<uint64_t> blocks,updates,rescales;
};

bool optimized(const Param& p) { return p.heads%2==0 && p.dim<=32; }

TestFixture fixture(const Shape& s,uint32_t seed,bool equal=false) {
    TestFixture f;
    auto& g=f.graph;auto& p=f.param;
    g.n=uint64_t(s.nodes);g.din=s.input;g.classes=s.dim;
    p.in=s.input;p.heads=s.heads;p.dim=s.dim;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> random(-1.f,1.f);
    g.x.resize(checked(g.n,p.in));
    p.w.resize(checked(p.in,p.width()));
    p.al.resize(p.width());p.ar.resize(p.width());
    for(auto& x:g.x)x=.35f*random(rng);
    for(auto& x:p.w)x=.35f*random(rng)/std::sqrt(float(p.in));
    for(auto& x:p.al)x=.4f*random(rng)/std::sqrt(float(p.dim));
    for(auto& x:p.ar)x=.4f*random(rng)/std::sqrt(float(p.dim));
    f.left.resize(checked(g.n,p.heads));f.right.resize(f.left.size());
    for(uint64_t n=0;n<g.n;++n)for(int h=0;h<p.heads;++h) {
        const size_t at=size_t(n)*p.heads+h;
        float position=float(n)/float(g.n-1);
        f.left[at]=equal?0.f:.09f*random(rng)+.017f*h;
        if(equal)f.right[at]=0.f;
        else if(h==0)f.right[at]=.9f*position-.45f;
        else if(h==1)f.right[at]=-.8f*position+.4f;
        else f.right[at]=.5f*random(rng)+.021f*h;
    }
    // Duplicate edges are intentional. Long sorted/decreasing source rows give
    // opposite running-max histories on two independent attention heads.
    static constexpr uint64_t degrees[]={0,1,15,16,17,31,32,33,63,64,65,96,97,129,130,257,273};
    g.row.push_back(0);
    for(uint64_t n=0;n<g.n;++n) {
        uint64_t degree=degrees[(n*7)%17];
        if(n==0 || n==6)degree=0;
        if(n==2 || n==4 || n+1==g.n)degree=129;
        for(uint64_t e=0;e<degree;++e) {
            uint32_t source;
            if(n==2 || n+1==g.n || n%4==1)
                source=uint32_t(std::min<uint64_t>(g.n-1,e/8));
            else if(n==4 || n%4==2)
                source=uint32_t(g.n-1-std::min<uint64_t>(g.n-1,e/8));
            else source=uint32_t((e*13+n*5+(e/17)*3)%g.n);
            g.col.push_back(source);
        }
        g.row.push_back(g.col.size());
    }
    g.e=g.col.size();g.validate();p.validate();
    return f;
}

void schedule_check(const Graph& g,const Schedule& s) {
    if(s.perm.size()!=g.n)throw std::runtime_error("pair smoke: schedule length");
    std::vector<unsigned char> seen(size_t(g.n),0);
    uint64_t previous=0;
    for(size_t at=0;at<s.perm.size();++at) {
        uint32_t n=s.perm[at];
        if(n>=g.n || seen[n]++)throw std::runtime_error("pair smoke: nonbijective row permutation");
        uint64_t degree=g.row[n+1]-g.row[n];
        if(at && degree<previous)throw std::runtime_error("pair smoke: missing DegreeSort");
        previous=degree;
    }
}

void initialize(const TestFixture& f,icpp_online::Workspace& w) {
    auto& b=w.base;
    convert(f.graph.x.data(),b.xbf.data(),f.graph.x.size());
    b.left=f.left;b.right=f.right;
    const float poison=std::numeric_limits<float>::quiet_NaN();
    std::fill(b.out.begin(),b.out.end(),poison);
    std::fill(b.den.begin(),b.den.end(),poison);
    std::fill(b.max.begin(),b.max.end(),poison);
    const uint64_t bad=std::numeric_limits<uint64_t>::max();
    std::fill(w.row_blocks.begin(),w.row_blocks.end(),bad);
    std::fill(w.row_max_updates.begin(),w.row_max_updates.end(),bad);
    std::fill(w.row_rescales.begin(),w.row_rescales.end(),bad);
    if(!b.z.empty() || !b.score.empty() || !b.p.empty() || !b.alpha.empty())
        throw std::runtime_error("pair smoke: unexpected full Z/edge-weight allocation");
}

Snapshot capture(const icpp_online::Workspace& w) {
    return {w.base.out,w.base.den,w.base.max,w.row_blocks,
            w.row_max_updates,w.row_rescales};
}

void exact_float(const std::vector<float>& expected,const std::vector<float>& actual,
                 const std::string& label,bool allow_matching_infinity=false) {
    bool identical=expected.size()==actual.size();
    double max_abs=0,absolute=0,squared=0,reference_squared=0;
    size_t count=0,first=std::numeric_limits<size_t>::max();
    if(identical)for(size_t i=0;i<expected.size();++i) {
        bool same=std::memcmp(&expected[i],&actual[i],sizeof(float))==0;
        if(!same && first==std::numeric_limits<size_t>::max())first=i;
        identical=identical && same;
        if(!std::isfinite(expected[i]) || !std::isfinite(actual[i])) {
            if(allow_matching_infinity && same && std::isinf(expected[i]))continue;
            identical=false;
            if(first==std::numeric_limits<size_t>::max())first=i;
            continue;
        }
        double delta=double(actual[i])-expected[i];
        max_abs=std::max(max_abs,std::abs(delta));absolute+=std::abs(delta);
        squared+=delta*delta;reference_squared+=double(expected[i])*expected[i];++count;
    }
    double relative=reference_squared>0?std::sqrt(squared/reference_squared):
                    (squared==0?0:std::numeric_limits<double>::infinity());
    std::cout<<"ICPP_PAIR_SMOKE phase="<<label<<" max_abs_error="<<max_abs
             <<" mean_abs_error="<<(count?absolute/count:0)
             <<" relative_L2_error="<<relative<<" bitwise="<<(identical?"PASS":"FAIL")<<'\n';
    if(!identical) {
        if(first<expected.size() && first<actual.size()) {
            uint32_t e=0,a=0;std::memcpy(&e,&expected[first],4);std::memcpy(&a,&actual[first],4);
            std::cout<<"ICPP_PAIR_SMOKE_MISMATCH phase="<<label<<" index="<<first
                     <<" expected="<<expected[first]<<" actual="<<actual[first]
                     <<" expected_bits="<<e<<" actual_bits="<<a<<'\n';
        }
        throw std::runtime_error("pair smoke bitwise float failure: "+label);
    }
}

void exact_count(uint64_t expected,uint64_t actual,const std::string& label) {
    if(expected!=actual) {
        std::cout<<"ICPP_PAIR_SMOKE_COUNTER field="<<label<<" expected="<<expected
                 <<" actual="<<actual<<" pass=FAIL\n";
        throw std::runtime_error("pair smoke counter failure: "+label);
    }
}

void exact_rows(const std::vector<uint64_t>& expected,const std::vector<uint64_t>& actual,
                const std::string& label) {
    exact_count(expected.size(),actual.size(),label+".size");
    for(size_t i=0;i<expected.size();++i)exact_count(expected[i],actual[i],label+"["+std::to_string(i)+"]");
}

void exact_stats(const icpp_online::Stats& a,const icpp_online::Stats& b,
                 const std::string& label,bool compare_samples=true) {
#define COUNT(field) exact_count(a.field,b.field,label+"."#field)
    COUNT(blocks);COUNT(max_updates);COUNT(rescales);
    COUNT(rescaled_feature_elements);COUNT(rescaled_padded_feature_elements);
    COUNT(spill_bytes);COUNT(reload_bytes);COUNT(rescale_events);
    COUNT(amx_calls);COUNT(neighbor_steps);COUNT(executed_fma);COUNT(total_tiles);
    if(compare_samples){COUNT(sampled_tiles);}
#undef COUNT
    // Profile fields are sampled worker intervals with different coarse
    // brackets in the pair path, and cannot be compared as wall times.
}

void compare_state(const Snapshot& a,const icpp_online::Workspace& b,
                   const std::string& label,bool counters) {
    exact_float(a.out,b.base.out,label+".numerator");
    exact_float(a.den,b.base.den,label+".denominator");
    exact_float(a.maximum,b.base.max,label+".maximum",true);
    if(counters) {
        exact_rows(a.blocks,b.row_blocks,label+".row_blocks");
        exact_rows(a.updates,b.row_max_updates,label+".row_max_updates");
        exact_rows(a.rescales,b.row_rescales,label+".row_rescales");
    }
}

void read_counts(const Graph& g,const Param& p,const Stats& stats,bool enabled,
                 const std::string& label) {
    const uint64_t groups=uint64_t(optimized(p)?p.heads/2:p.heads);
    const uint64_t edge_groups=uint64_t(checked(g.e,groups));
    exact_count(enabled?uint64_t(checked(edge_groups,(p.in+15)/16)):0,
                stats.raw_H_vectors,label+".raw_H_vectors");
    exact_count(enabled?uint64_t(checked(edge_groups,uint64_t(p.in)*sizeof(BF16))):0,
                stats.logical_raw_H_bytes,label+".logical_raw_H_bytes");
    exact_count(enabled?edge_groups:0,stats.source_index_loads,label+".source_index_loads");
}

void row_invariants(const TestFixture& f,const icpp_online::Workspace& w,
                    const icpp_online::Stats& stats,int block,bool equal) {
    uint64_t blocks=0,updates=0,rescales=0;
    const auto& g=f.graph;const auto& p=f.param;
    for(uint64_t n=0;n<g.n;++n)for(int h=0;h<p.heads;++h) {
        size_t at=size_t(n)*p.heads+h;
        uint64_t degree=g.row[n+1]-g.row[n];
        uint64_t expected_blocks=(degree+uint64_t(block)-1)/uint64_t(block);
        exact_count(expected_blocks,w.row_blocks[at],"row_invariant.blocks");
        if(!degree) {
            exact_count(0,w.row_max_updates[at],"empty.updates");
            exact_count(0,w.row_rescales[at],"empty.rescales");
            if(w.base.den[at]!=0.f || w.base.max[at]!=-std::numeric_limits<float>::infinity())
                throw std::runtime_error("pair smoke: empty row state");
        } else {
            if(w.row_max_updates[at]<1 || w.row_max_updates[at]>expected_blocks ||
               w.row_rescales[at]+1!=w.row_max_updates[at])
                throw std::runtime_error("pair smoke: initial maximum incorrectly counted as rescale");
            if(equal) {
                exact_count(1,w.row_max_updates[at],"equal.updates");
                exact_count(0,w.row_rescales[at],"equal.rescales");
                if(w.base.den[at]!=float(degree))throw std::runtime_error("pair smoke: equal-score denominator");
            }
        }
        blocks+=w.row_blocks[at];updates+=w.row_max_updates[at];rescales+=w.row_rescales[at];
    }
    exact_count(blocks,stats.blocks,"sum.blocks");
    exact_count(updates,stats.max_updates,"sum.max_updates");
    exact_count(rescales,stats.rescales,"sum.rescales");
    exact_count(rescales*uint64_t(p.dim),stats.rescaled_feature_elements,"sum.rescaled_elements");
    exact_count(rescales*uint64_t((p.dim+15)/16)*16,stats.rescaled_padded_feature_elements,"sum.padded_elements");
    exact_count(uint64_t(p.heads)*((g.n+15)/16),stats.total_tiles,"sum.total_tiles");
    if(equal) {
        exact_count(0,stats.rescale_events,"equal.events");
        exact_count(0,stats.spill_bytes,"equal.spill");
        exact_count(0,stats.reload_bytes,"equal.reload");
    } else {
        // Original IDs survive DegreeSort: ascending row 2 must rescale on h0,
        // descending row 4 never does. Head 1 has the opposite source ranking.
        if(w.row_rescales[size_t(2)*p.heads]==0)
            throw std::runtime_error("pair smoke: fixture did not force running-max increase");
        exact_count(0,w.row_rescales[size_t(4)*p.heads],"descending.no_rescale");
        if(p.heads>1)exact_count(0,w.row_rescales[size_t(2)*p.heads+1],"independent_head.no_rescale");
    }
}

void profile_control(const TestFixture& f,const Prepared& q,const Schedule& schedule,
                     const Snapshot& raw,const std::vector<float>& normalized,
                     const icpp_online::Stats& plain_stats,int block) {
    const auto& g=f.graph;const auto& p=f.param;
    icpp_online::Workspace original;original.allocate(g,p,true);initialize(f,original);
    Workspace paired;paired.allocate(g,p,true);initialize(f,paired.original);
    icpp_online::Stats os;Stats ps;
    icpp_online::aggregate(g,p,q,schedule,original,block,32,os,1,true);
    aggregate(g,p,q,schedule,paired,block,32,ps,1,true);
    compare_state(raw,original,"profile.original",true);
    compare_state(raw,paired.original,"profile.pair",true);
    exact_stats(os,ps.baseline,"profile.original_vs_pair");
    exact_stats(plain_stats,ps.baseline,"profile_vs_plain",false);
    exact_count(ps.baseline.total_tiles,ps.baseline.sampled_tiles,"profile.all_tiles_sampled");
    read_counts(g,p,ps,true,"profile");
    gat::Times ft;finish(g,p,paired.original.base,false,ft);
    exact_float(normalized,paired.original.base.out,"profile.normalized");

    // The timing path selects a different template without statistics or
    // clocks. No output gate is relaxed for that production specialization.
    Workspace speed;speed.allocate(g,p,false);initialize(f,speed.original);
    Stats ss;aggregate(g,p,q,schedule,speed,block,32,ss,0,false);
    compare_state(raw,speed.original,"no_profile_no_counters",false);
    read_counts(g,p,ss,false,"no_profile_no_counters");
    exact_stats(icpp_online::Stats{},ss.baseline,"no_profile_no_counters.stats");
    gat::Times sf;finish(g,p,speed.original.base,false,sf);
    exact_float(normalized,speed.original.base.out,"no_profile_no_counters.normalized");

    // Exercise the fourth specialization as well: profile clocks enabled,
    // exact arithmetic counters disabled. Only the profile tile count remains.
    Workspace profile_only;profile_only.allocate(g,p,false);initialize(f,profile_only.original);
    Stats po;aggregate(g,p,q,schedule,profile_only,block,32,po,1,false);
    compare_state(raw,profile_only.original,"profile_no_counters",false);
    read_counts(g,p,po,false,"profile_no_counters");
    icpp_online::Stats expected_profile_only;
    expected_profile_only.total_tiles=plain_stats.total_tiles;
    expected_profile_only.sampled_tiles=plain_stats.total_tiles;
    exact_stats(expected_profile_only,po.baseline,"profile_no_counters.stats");
    gat::Times pf;finish(g,p,profile_only.original.base,false,pf);
    exact_float(normalized,profile_only.original.base.out,"profile_no_counters.normalized");
}

void operator_case(const Shape& shape,int block,uint32_t seed,bool equal,bool profile) {
    TestFixture f=fixture(shape,seed,equal);
    const auto& g=f.graph;const auto& p=f.param;
    Prepared q=prepare(p,true,true,true);Schedule s=degree_schedule(g);schedule_check(g,s);
    icpp_online::Workspace original;original.allocate(g,p,true);initialize(f,original);
    Workspace pair;pair.allocate(g,p,true);initialize(f,pair.original);
    // Scratch is thread-local, not a second graph-sized aggregation matrix.
    size_t expected_scratch=optimized(p)?checked(pair.original.thread_capacity,
                                      checked(32,q.padded_in)):0;
    exact_count(expected_scratch,pair.paired_hbuf.size(),"bounded_pair_hbuf");
    icpp_online::Stats os;Stats ps;
    std::cout<<"ICPP_PAIR_SMOKE_CASE N="<<g.n<<" D="<<p.in<<" K="<<p.heads
             <<" d="<<p.dim<<" E="<<g.e<<" block="<<block
             <<" optimized="<<int(optimized(p))<<" equal_scores="<<int(equal)<<'\n';
    icpp_online::aggregate(g,p,q,s,original,block,32,os,0,true);
    aggregate(g,p,q,s,pair,block,32,ps,0,true);
    Snapshot raw=capture(original);
    compare_state(raw,pair.original,"operator",true);
    exact_stats(os,ps.baseline,"operator.stats");read_counts(g,p,ps,true,"operator");
    row_invariants(f,original,os,block,equal);
    row_invariants(f,pair.original,ps.baseline,block,equal);
    gat::Times of,pf;finish(g,p,original.base,false,of);finish(g,p,pair.original.base,false,pf);
    exact_float(original.base.out,pair.original.base.out,"operator.normalized");
    if(profile)profile_control(f,q,s,raw,original.base.out,os,block);
    std::cout<<"ICPP_PAIR_SMOKE_CASE_RESULT pass=PASS raw_H_vectors="<<ps.raw_H_vectors
             <<" logical_raw_H_bytes="<<ps.logical_raw_H_bytes
             <<" source_index_loads="<<ps.source_index_loads
             <<" counters_unit=head logical_bytes_are_DRAM_measurement=0\n";
}

void layer_case(const Shape& shape,uint32_t seed) {
    TestFixture f=fixture(shape,seed);
    const auto& g=f.graph;const auto& p=f.param;
    Prepared q=prepare(p,true,true,true);Schedule s=degree_schedule(g);schedule_check(g,s);
    icpp_online::Workspace original;original.allocate(g,p,true);
    Workspace pair;pair.allocate(g,p,true);
    icpp_online::Timing ot;Timing pt;icpp_online::Stats os;Stats ps;
    std::cout<<"ICPP_PAIR_SMOKE_LAYER N="<<g.n<<" D="<<p.in<<" K="<<p.heads
             <<" d="<<p.dim<<" block=32 lr_policy=fp32 hidden=1\n";
    icpp_online::layer(g,p,q,s,g.x,original,true,32,32,ot,os,0,true,"fp32");
    layer(g,p,q,s,g.x,pair,true,32,32,pt,ps,0,true,"fp32");
    exact_float(original.base.left,pair.original.base.left,"layer.left");
    exact_float(original.base.right,pair.original.base.right,"layer.right");
    exact_float(original.base.out,pair.original.base.out,"layer.normalized_ELU");
    exact_float(original.base.den,pair.original.base.den,"layer.denominator");
    exact_float(original.base.max,pair.original.base.max,"layer.maximum",true);
    exact_rows(original.row_blocks,pair.original.row_blocks,"layer.row_blocks");
    exact_rows(original.row_max_updates,pair.original.row_max_updates,"layer.row_updates");
    exact_rows(original.row_rescales,pair.original.row_rescales,"layer.row_rescales");
    exact_stats(os,ps.baseline,"layer.stats");read_counts(g,p,ps,true,"layer");
    std::cout<<"ICPP_PAIR_SMOKE_LAYER_RESULT pass=PASS\n";
}
}

void run_pair_smoke() {
    std::cout<<std::setprecision(17);
    std::cout<<"ICPP_PAIR_SMOKE_START oracle=original_icpp_online bitwise_required=1 "
                "master_precision_certification=0 untrained_model_accuracy_certification=0 "
                "profile_intervals=sampled_worker_seconds logical_bytes_are_DRAM_measurement=0\n";
    static constexpr Shape shapes[]={
        {17,17,1,7},{35,17,2,7},{17,17,3,16},
        {35,33,4,16},{17,33,8,32},{35,33,2,47},
        {17,100,4,7},{35,100,8,16},{17,100,3,32},
        {35,128,2,32},{17,128,4,47},{35,128,1,16},
        {17,256,8,32},{35,256,4,32},{17,256,3,47},
        {35,17,8,7},{17,33,2,16},{35,100,8,32}
    };
    int cases=0,paired_cases=0,fallback_cases=0;
    for(size_t i=0;i<sizeof(shapes)/sizeof(shapes[0]);++i)for(int block:{16,32,64}) {
        // Identical fixture across neighbor-block sizes; independent params
        // and attention vectors across shapes and heads.
        operator_case(shapes[i],block,0x50414952u+uint32_t(i),false,i==12 && block==32);
        ++cases;if(shapes[i].heads%2==0 && shapes[i].dim<=32)++paired_cases;else ++fallback_cases;
    }
    for(int block:{16,32,64}) {
        operator_case({35,33,8,32},block,0x45515541u,true,false);++cases;++paired_cases;
    }
    layer_case({17,128,8,32},0x4c415931u);
    layer_case({35,100,3,47},0x4c415932u);
    std::cout<<"ICPP_PAIR_SMOKE_RESULT pass=PASS operator_cases="<<cases
             <<" optimized_cases="<<paired_cases<<" fallback_cases="<<fallback_cases
             <<" layer_wrapper_cases=2 comparison=EXACT_original_online "
                "original_master_gate_unchanged=1\n";
}
}
