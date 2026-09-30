#include "gat.hpp"
#include <iomanip>
#include <iostream>
#include <map>
#include <functional>
#include <sstream>
using namespace gat;
struct Args {
    std::map<std::string,std::string> v;
    Args(int argc,char** argv){for(int i=1;i<argc;++i){std::string k=argv[i];if(k.rfind("--",0)!=0 || i+1==argc)throw std::runtime_error("arguments must be --key value");if(!v.emplace(k,argv[++i]).second)throw std::runtime_error("duplicate argument: "+k);}
        const std::vector<std::string> allowed={"--graph","--weights","--seed","--save-weights","--path","--mode","--threads","--panel-r","--repeats","--warmups","--layer","--lr-policy","--fixture","--max-abs","--rel-l2","--dump-prefix","--output"};
        for(const auto& x:v)if(std::find(allowed.begin(),allowed.end(),x.first)==allowed.end())throw std::runtime_error("unknown option: "+x.first);}
    std::string get(const std::string& k,const std::string& def="")const{auto i=v.find(k);return i==v.end()?def:i->second;}
    int num(const std::string& k,int def)const{return std::stoi(get(k,std::to_string(def)));}
};
static void dump(const std::string& path,const std::vector<float>& v){std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(v.data()),v.size()*4);if(!f)throw std::runtime_error("cannot write dump");}
static void validate_stage(const Workspace& w){
    for(auto item:{std::pair<const char*,const std::vector<float>*>("Z",&w.z),{"left",&w.left},{"right",&w.right},{"row_max",&w.max},{"den",&w.den},{"output",&w.out},{"score",&w.score},{"p",&w.p},{"alpha",&w.alpha}})require_finite(*item.second,item.first);
}
static void run_path(const std::string& path,const Graph& g,const Param& p,const Prepared& q,const Schedule& s,const std::vector<float>& input,Workspace& w,bool hidden,int panel,Times& t,const std::string& policy){
    if(path=="ref_fp32")ref_layer(g,p,input,w,hidden,t);
    else if(path=="tfs_bf16")tfs_layer(g,p,q,s,input,w,hidden,panel,t,policy);
    else standard_layer(g,p,q,input,w,hidden,t,path=="standard_fp32",path=="matched_attention"?policy:policy=="z_mkl"?"z_mkl":"z_avx");
}
static double pct(std::vector<double> x,double q){std::sort(x.begin(),x.end());return x[std::min(x.size()-1,size_t(std::ceil(q*x.size())-1))];}
int main(int argc,char** argv){try{
    Args a(argc,argv);std::string graph=a.get("--graph"),path=a.get("--path","standard_bf16"),mode=a.get("--mode","correctness");
    if(graph.empty())throw std::runtime_error("--graph is required");
    if(path!="ref_fp32"&&path!="standard_bf16"&&path!="standard_fp32"&&path!="tfs_bf16"&&path!="matched_attention")throw std::runtime_error("bad path");
    int threads=a.num("--threads",16),panel=a.num("--panel-r",64),reps=a.num("--repeats",5),warmups=a.num("--warmups",2),layer=a.num("--layer",0);
    if(threads<=0||panel<16||panel%16||reps<1||warmups<0||layer<0||layer>3)throw std::runtime_error("invalid benchmark controls");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_dynamic(0);mkl_set_num_threads(threads);
    MKLVersion version;mkl_get_version(&version);
    std::cout<<"ENV mkl="<<version.MajorVersion<<"."<<version.MinorVersion<<"."<<version.UpdateVersion<<" processor="<<version.Processor<<" threads="<<threads
      <<" source_input="<<graph<<" checkpoint="<<(a.get("--weights").empty()?"random_fixture_not_trained":a.get("--weights"))<<" profile="<<profiling
      <<" vendor_amx_status=UNVERIFIED_UNTIL_DISPATCH_EVIDENCE\n";
    auto cold_start=Clock::now();Graph g=load_graph(graph);
    Model model;
    if(!a.get("--weights").empty())model=load_model(a.get("--weights"),g.din,g.classes);
    else {if(a.get("--seed").empty())throw std::runtime_error("provide --weights checkpoint.bin or explicit --seed for synthetic fixture");model=random_model(g.din,g.classes,a.num("--seed",11));}
    if(!a.get("--save-weights").empty())save_model(model,a.get("--save-weights"));
    Schedule sched;if(path=="tfs_bf16"||mode=="correctness"||mode=="fixed-p")sched=degree_schedule(g);
    bool controls=mode=="correctness"||mode=="fixed-p"||mode=="export-fixture"||mode=="micro";
    std::vector<Prepared> prep;for(const auto& p:model)prep.push_back(prepare(p,controls||path=="tfs_bf16"||path=="matched_attention",controls||path=="tfs_bf16",controls||(path!="ref_fp32"&&path!="standard_fp32")));
    double static_s=sched.degree_sort_s;size_t static_bytes=sched.perm.size()*4;
    for(const auto& p:prep){static_s+=p.weight_prepare_s;static_bytes+=(p.w.size()+p.blr_bf16.size()+p.packed.size())*2+(p.blr.size()+p.blr_dot.size())*4;}
    std::cout<<"STATIC degree_sort_ms="<<sched.degree_sort_s*1000<<" model_prepare_ms="<<(static_s-sched.degree_sort_s)*1000<<" bytes="<<static_bytes<<" graph_policy=as_loaded_no_mutation\n";
    std::string policy=a.get("--lr-policy",path=="tfs_bf16"||path=="matched_attention"?"bf16":"z_avx");
    if(policy!="bf16"&&policy!="fp32"&&policy!="avx"&&policy!="z_avx"&&policy!="z_mkl")throw std::runtime_error("bad LR policy");
    if((path=="tfs_bf16"||path=="matched_attention")&&(policy=="z_avx"||policy=="z_mkl"))throw std::runtime_error("reordered LR path requires bf16/fp32/avx");
    std::cout<<"CONFIG mode="<<mode<<" path="<<path<<" lr_policy="<<policy<<" panel_R="<<panel<<" layer="<<layer<<" warmups="<<warmups<<" repeats="<<reps<<"\n";
    if(mode=="correctness") {
        bool have_abs=!a.get("--max-abs").empty(),have_rel=!a.get("--rel-l2").empty();
        if(have_abs!=have_rel)throw std::runtime_error("set both --max-abs and --rel-l2");
        double maxabs=have_abs?std::stod(a.get("--max-abs")):0,rel=have_rel?std::stod(a.get("--rel-l2")):0;
        if(have_abs&&(!std::isfinite(maxabs)||!std::isfinite(rel)||maxabs<0||rel<0))throw std::runtime_error("invalid accuracy thresholds");
        bool pass=true,finite=true;auto check=[&](const std::string& name,int l,const std::string& stage,const std::vector<float>& r,const std::vector<float>& v,bool gate){auto e=errors(r,v);print_error(name,l,stage,e);pass&=e.finite;finite&=e.finite;if(gate&&have_abs)pass&=e.max_abs<=maxabs&&e.relative_l2<=rel;};
        std::vector<std::string> paths={"standard_fp32","standard_bf16","tfs_bf16","matched_attention"};
        std::vector<float> input=g.x;
        for(int l=0;l<3;++l){const auto& p=model[l];Workspace rw;rw.allocate(g,p,"ref_fp32");Times rt;ref_layer(g,p,input,rw,l<2,rt);validate_stage(rw);
            if(!a.get("--dump-prefix").empty()){
                std::string pre=a.get("--dump-prefix")+"_layer"+std::to_string(l+1);
                for(auto item:{std::pair<const char*,const std::vector<float>*>("output",&rw.out),{"z",&rw.z},{"left",&rw.left},{"right",&rw.right},{"max",&rw.max},{"den",&rw.den},{"score",&rw.score},{"p",&rw.p},{"alpha",&rw.alpha}})dump(pre+"_"+item.first+".f32",*item.second);
            }
            Param qp=p;qp.w=quantized(p.w);auto qx=quantized(input);Workspace qr;qr.allocate(g,p,"ref_fp32");Times qt;ref_layer(g,qp,qx,qr,l<2,qt);
            check("quantized_XW_fp32_accum",l+1,"Z",rw.z,qr.z,false);
            check("quantized_XW_fp32_accum",l+1,"layer",rw.out,qr.out,false);
            for(const auto& test:paths){Workspace w;w.allocate(g,p,test);Times t;
                run_path(test,g,p,prep[l],sched,input,w,l<2,panel,t,test=="tfs_bf16"||test=="matched_attention"?"bf16":"z_avx");validate_stage(w);
                if(!w.z.empty()){check(test,l+1,"Z",rw.z,w.z,false);check(test,l+1,"Z_accum_vs_quantized_fp32",qr.z,w.z,false);}
                check(test,l+1,"L",rw.left,w.left,false);check(test,l+1,"R",rw.right,w.right,false);check(test,l+1,"row_max",rw.max,w.max,false);
                check(test,l+1,"denominator",rw.den,w.den,false);check(test,l+1,"layer",rw.out,w.out,true);
                print_times(test,"correctness",l+1,0,g,p,w,t,static_s,panel);
            }
            // FP32 contraction control isolates algebraic reorder from X/W/B BF16.
            Workspace control;control.allocate(g,p,"tfs_bf16");lr_reordered(g,p,prep[l],input,control,"fp32");
            check("attention_reorder_fp32",l+1,"L",rw.left,control.left,false);check("attention_reorder_fp32",l+1,"R",rw.right,control.right,false);
            input=std::move(rw.out);
        }
        std::vector<Workspace> refs(3);for(int l=0;l<3;++l)refs[l].allocate(g,model[l],"ref_fp32");const std::vector<float>* ri=&g.x;
        for(int l=0;l<3;++l){Times t;ref_layer(g,model[l],*ri,refs[l],l<2,t);ri=&refs[l].out;}
        for(const auto& test:paths){std::vector<Workspace> ws(3);for(int l=0;l<3;++l)ws[l].allocate(g,model[l],test);const std::vector<float>* in=&g.x;
            for(int l=0;l<3;++l){Times t;run_path(test,g,model[l],prep[l],sched,*in,ws[l],l<2,panel,t,test=="tfs_bf16"||test=="matched_attention"?"bf16":"z_avx");in=&ws[l].out;validate_stage(ws[l]);}
            check(test,3,"model_path_specific_chain",refs[2].out,ws[2].out,true);
            if(!a.get("--dump-prefix").empty())dump(a.get("--dump-prefix")+"_model_"+test+".f32",ws[2].out);}
        if(!a.get("--dump-prefix").empty())dump(a.get("--dump-prefix")+"_model_ref_fp32.f32",refs[2].out);
        std::cout<<"CORRECTNESS finite="<<finite<<" accuracy_gate="<<(have_abs?(pass?"PASS":"FAIL"):"UNCONFIGURED_NOT_ACCEPTED")<<" task_metric=UNAVAILABLE\n";
        return pass?0:1;
    }
    if(mode=="export-fixture" || mode=="fixed-p" || mode=="micro") {
        if(layer==0)layer=2;const auto& p=model[layer-1];const auto& q=prep[layer-1];
        std::vector<float> input=g.x;
        for(int l=0;l<layer-1;++l){Workspace rw;rw.allocate(g,model[l],"ref_fp32");Times t;ref_layer(g,model[l],input,rw,true,t);input=std::move(rw.out);}
        if(mode=="micro") {
            Workspace w;w.allocate(g,p,"matched_attention");w.prepare_lr_sgemm(g,p);convert(input.data(),w.xbf.data(),input.size());
            auto bench=[&](const std::string& name,const std::function<void()>& fn){std::vector<double> samples;
                for(int r=-warmups;r<reps;++r){auto t=Clock::now();fn();auto dt=seconds(t,Clock::now());if(r>=0)samples.push_back(dt);}
                std::cout<<"MICRO name="<<name<<" layer="<<layer<<" median_ms="<<pct(samples,.5)*1000<<" p95_ms="<<pct(samples,.95)*1000<<"\n";};
            bench("convert_rne",[&]{convert(input.data(),w.xbf.data(),input.size());});
            bench("projection_bf16",[&]{projection(g,p,q,input,w,true);});
            bench("lr_z_avx",[&]{lr_from_z(g,p,w);});auto avxL=w.left,avxR=w.right;
            bench("lr_z_sgemm_static_workspace",[&]{lr_from_z_mkl(g,p,w);});print_error("micro",layer,"lr_z_mkl_vs_avx_L",errors(avxL,w.left));
            bench("lr_reordered_bf16",[&]{lr_reordered(g,p,q,input,w,"bf16");});
            bench("lr_reordered_fp32",[&]{lr_reordered(g,p,q,input,w,"fp32");});
            bench("lr_reordered_avx",[&]{lr_reordered(g,p,q,input,w,"avx");});
            bench("max_prescan",[&]{max_prescan(g,p,w);});
            bench("weighted_spmm_score_exp",[&]{standard_aggregate(g,p,w);});
            return 0;
        }
        Fixture fixture;
        if(mode=="export-fixture" || a.get("--fixture").empty()){
            Workspace rw;rw.allocate(g,p,"ref_fp32");Times t;ref_layer(g,p,input,rw,false,t);fixture={input,rw.p,rw.den};
        }else fixture=load_fixture(g,p,a.get("--fixture"));
        if(mode=="export-fixture") {if(a.get("--fixture").empty())throw std::runtime_error("--fixture output is required");save_fixture(g,p,fixture,a.get("--fixture"));std::cout<<"FIXTURE saved layer="<<layer<<"\n";return 0;}
        input=std::move(fixture.input);
        // Fixed-p comparison always executes both paths with the identical fixture.
        Workspace sw,tw;sw.allocate(g,p,"standard_bf16");tw.allocate(g,p,"tfs_bf16");
        convert(input.data(),sw.xbf.data(),input.size());convert(input.data(),tw.xbf.data(),input.size());
        auto standard_op=[&](Times& t){auto begin=Clock::now(),x=begin;projection(g,p,q,input,sw,true);t.projection=seconds(x,Clock::now());x=Clock::now();standard_aggregate(g,p,sw,fixture.p.data(),fixture.den.data(),&t);t.aggregate=seconds(x,Clock::now());finish(g,p,sw,false,t);t.total=seconds(begin,Clock::now());};
        auto tfs_op=[&](Times& t){auto begin=Clock::now();tfs_aggregate(g,p,q,sched,tw,panel,t,fixture.p.data(),fixture.den.data());t.aggregate=seconds(begin,Clock::now());finish(g,p,tw,false,t);t.total=seconds(begin,Clock::now());};
        Times st,tt;standard_op(st);tfs_op(tt);require_finite(sw.out,"fixed-p standard");require_finite(tw.out,"fixed-p TFS");
        print_error("tfs_fixed_p",layer,"vs_standard_bf16_extra_pX_quantization_and_reduction",errors(sw.out,tw.out));
        // FP32 fixed-p oracle, then quantized-XW FP32 oracle: separate the pX rounding cost.
        Workspace oracle;oracle.allocate(g,p,"standard_bf16");projection(g,p,q,input,oracle,false);standard_aggregate(g,p,oracle,fixture.p.data(),fixture.den.data());Times ot;finish(g,p,oracle,false,ot);
        print_error("standard_fixed_p",layer,"vs_master_fixed_p",errors(oracle.out,sw.out));print_error("tfs_fixed_p",layer,"vs_master_fixed_p",errors(oracle.out,tw.out));
        // Explicit FP32 emulation of BF16 weighted feature + BF16 W, same neighbor-step order.
        if(g.e*double(p.heads)*p.in*p.dim<=2e7){
            std::vector<float> emulated(tw.out.size(),0);auto xq=quantized(input),wq=quantized(p.w);
            for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h)for(uint64_t e=g.row[i];e<g.row[i+1];++e)for(int k=0;k<p.in;++k){float value=expand(rne(fixture.p[e*p.heads+h]*xq[size_t(g.col[e])*p.in+k]));for(int d=0;d<p.dim;++d)emulated[i*p.width()+h*p.dim+d]+=value*wq[size_t(k)*p.width()+h*p.dim+d];}
            for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h)for(int d=0;d<p.dim;++d){float den=fixture.den[i*p.heads+h];emulated[i*p.width()+h*p.dim+d]=den>0?emulated[i*p.width()+h*p.dim+d]/den:0;}
            print_error("tfs_fixed_p",layer,"vs_pX_bf16_emulation_reduction_only",errors(emulated,tw.out));
        } else std::cout<<"CONTROL pX_scalar_emulation=SKIPPED_TOO_LARGE validated_in_boundary_tests=true\n";
        std::vector<double> ss,ts;
        for(int r=-warmups;r<reps;++r){Times s,t;if(r%2==0){standard_op(s);tfs_op(t);}else{tfs_op(t);standard_op(s);}
            if(r>=0){ss.push_back(s.total);ts.push_back(t.total);if(!profiling){t.neighbor_steps=sched.neighbor_steps*p.heads;t.executed_fma=t.neighbor_steps*16*q.padded_in*q.output_blocks*16;}
                print_times("standard_bf16","fixed-p",layer,r,g,p,sw,s,static_s,panel);print_times("tfs_bf16","fixed-p",layer,r,g,p,tw,t,static_s,panel);}}
        std::cout<<"FIXED_P median_standard_ms="<<pct(ss,.5)*1000<<" median_tfs_ms="<<pct(ts,.5)*1000<<" speedup="<<pct(ss,.5)/pct(ts,.5)<<" conversion_excluded=true input_preparation=shared policy=AB_BA\n";
        return 0;
    }
    if(mode!="benchmark" && mode!="profile")throw std::runtime_error("bad mode");
    if(mode=="benchmark" && profiling)throw std::runtime_error("benchmark requires speed binary without GAT_PROFILE");
    if(mode=="profile" && !profiling)throw std::runtime_error("profile requires profile binary");
    std::vector<Workspace> ws(3);for(int l=layer?layer-1:0;l<(layer?layer:3);++l){ws[l].allocate(g,model[l],path);if(policy=="z_mkl")ws[l].prepare_lr_sgemm(g,model[l]);}
    size_t workspace=0;for(const auto& w:ws)workspace+=w.bytes();std::cout<<"MODEL workspace_bytes="<<workspace<<" static_bytes="<<static_bytes<<" excludes_graph_and_master_weights=true\n";
    std::vector<float> single_input;
    if(layer){single_input=g.x;for(int l=0;l<layer-1;++l){Workspace rw;rw.allocate(g,model[l],"ref_fp32");Times t;ref_layer(g,model[l],single_input,rw,true,t);single_input=std::move(rw.out);}}
    std::cout<<"COLD startup_before_inference_ms="<<seconds(cold_start,Clock::now())*1000<<" includes_graph_load=true\n";
    std::vector<double> totals;double cold_forward=0;
    for(int r=-warmups;r<reps;++r){Times times[3];const std::vector<float>* input=layer?&single_input:&g.x;
        auto begin=Clock::now();for(int l=layer?layer-1:0;l<(layer?layer:3);++l){run_path(path,g,model[l],prep[l],sched,*input,ws[l],l<2,panel,times[l],policy);input=&ws[l].out;}double total=seconds(begin,Clock::now());
        if(r==-warmups)cold_forward=total;
        // Stop the entire model timer before scans, copies or printing.
        require_finite(*input,"benchmark output");
        if(r>=0){totals.push_back(total);for(int l=layer?layer-1:0;l<(layer?layer:3);++l){
            if(path=="tfs_bf16" && !profiling){times[l].neighbor_steps=sched.neighbor_steps*model[l].heads;times[l].executed_fma=times[l].neighbor_steps*16*prep[l].padded_in*prep[l].output_blocks*16;}
            print_times(path,mode,l+1,r,g,model[l],ws[l],times[l],static_s,panel);}
            std::cout<<"E2E path="<<path<<" rep="<<r<<" layer="<<layer<<" total_ms="<<total*1000<<" finite=1\n";}
    }
    if(!a.get("--output").empty())dump(a.get("--output"),ws[layer?layer-1:2].out);
    std::cout<<"SUMMARY path="<<path<<" median_ms="<<pct(totals,.5)*1000<<" p95_ms="<<pct(totals,.95)*1000<<" cold_forward_ms="<<cold_forward*1000<<" repeats="<<reps<<" task_accuracy=UNVERIFIED\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
