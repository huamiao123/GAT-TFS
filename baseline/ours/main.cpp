#include "local_online.hpp"
#include <iostream>
#include <iomanip>
using namespace gat;
static Error compare_fast(const std::vector<float>& ref,const std::vector<float>& got){
    if(ref.size()!=got.size()||ref.empty())throw std::runtime_error("error size mismatch");
    double mx=0,ab=0,sq=0,norm=0;int bad=0;
    #pragma omp parallel for reduction(max:mx) reduction(+:ab,sq,norm) reduction(|:bad)
    for(size_t i=0;i<ref.size();++i){bad|=!std::isfinite(ref[i])||!std::isfinite(got[i]);double d=double(got[i])-ref[i];mx=std::max(mx,std::abs(d));ab+=std::abs(d);sq+=d*d;norm+=double(ref[i])*ref[i];}
    Error e;e.finite=!bad;e.max_abs=mx;e.mean_abs=ab/ref.size();e.relative_l2=norm>0?std::sqrt(sq/norm):(sq==0?0:INFINITY);if(bad)e.max_abs=e.mean_abs=e.relative_l2=INFINITY;return e;
}
static void report_error(const std::string& path,int l,const Error& e){
    std::cout<<std::setprecision(10)<<"CHECK path="<<path<<" layer="<<l<<" max_abs_error="<<e.max_abs<<" mean_abs_error="<<e.mean_abs<<" relative_L2_error="<<e.relative_l2<<" finite="<<e.finite<<" reference=B0_FP32 task_acceptance=UNVERIFIED\n";
    if(!e.finite)throw std::runtime_error("nonfinite output");
}
static uint64_t fingerprint(const std::vector<float>& v){uint64_t hash=0;
    #pragma omp parallel for reduction(^:hash)
    for(size_t i=0;i<v.size();++i){uint32_t bits;std::memcpy(&bits,&v[i],4);uint64_t z=uint64_t(bits)^((uint64_t(i)+1)*0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;hash^=z^(z>>27);}return hash;}
static Graph fixture(int D){Graph g;g.n=35;g.din=D;g.classes=47;g.row={0};int deg[]={0,1,2,15,16,17,31,32,33,63,64,65,129};
    for(uint64_t i=0;i<g.n;++i){for(int k=0;k<deg[i%13];++k)g.col.push_back((i*3+k*7)%g.n);g.row.push_back(g.col.size());}g.e=g.col.size();g.x.resize(g.n*D);for(size_t i=0;i<g.x.size();++i)g.x[i]=std::sin(float(i)*.17f)*.5f;return g;}
static void smoke(){
    // Validate FP32 local-U first, before enabling the approximate AMX backend.
    for(bool amx:{false,true})for(int D:{17,33,128,256})for(int K:{1,2,8})for(int dim:{7,32,40,47}){
        Graph g=fixture(D);auto p=random_model(D,dim,11)[0];p.heads=K;p.dim=dim;p.w.resize(D*K*dim);p.al.resize(K*dim);p.ar.resize(K*dim);
        for(size_t i=0;i<p.w.size();++i)p.w[i]=std::sin(float(i)*.39f)*.07f;
        for(size_t i=0;i<p.al.size();++i){p.al[i]=std::cos(float(i)*.43f)*.2f;p.ar[i]=std::sin(float(i)*.27f)*.2f;}
        auto q=local::prepare_local(p);auto sched=degree_schedule(g);Workspace ref;ref.allocate(g,p,"ref_fp32");Times rt;ref_layer(g,p,g.x,ref,false,rt);
        local::WorkspaceLocal w;w.allocate(g,p,true);
        for(int block:{16,32,64}){local::Timing t;local::layer(g,p,q,sched,g.x,w,false,amx,block,64,t,0,true);
            auto e=compare_fast(ref.out,w.out);
            // Approximate GEMM: tiny absolute errors on nearly cancelling outputs
            // do not have a meaningful relative-only gate. FP32 gate stays unchanged.
            bool relative_ok=e.relative_l2<=1e-4||(amx&&e.max_abs<=1e-5);
            if(!e.finite||e.max_abs>1e-4||!relative_ok){std::cerr<<"SMOKE_MISMATCH amx="<<amx<<" D="<<D<<" K="<<K<<" d="<<dim<<" block="<<block<<" max_abs="<<e.max_abs<<" rel="<<e.relative_l2<<"\n";throw std::runtime_error("local smoke vs R0 failed");}
            if(amx){local::WorkspaceLocal fp;fp.allocate(g,p);local::Timing ft;local::layer(g,p,q,sched,g.x,fp,false,false,block,64,ft);auto backend=compare_fast(fp.out,w.out);
                if(!backend.finite||backend.max_abs>1e-5)throw std::runtime_error("AMX vs identical FP32 local backend absolute gate failed");
                if(e.relative_l2>1e-4)std::cout<<"NEAR_ZERO_SMOKE D="<<D<<" K="<<K<<" d="<<dim<<" block="<<block<<" max_abs="<<e.max_abs<<" relative_L2="<<e.relative_l2<<" AMX_vs_local_FP32_max_abs="<<backend.max_abs<<" scope=absolute_smoke_only_not_task_acceptance\n";}
            for(int d=0;d<p.width();++d)if(w.out[d]!=0)throw std::runtime_error("empty row semantics failed");}
    }
    // Independent stable FP64 weighted-H then W oracle with adversarial fixed L/R.
    Graph g=fixture(33);Param p=random_model(33,32,11)[0];auto q=local::prepare_local(p);auto sched=degree_schedule(g);local::WorkspaceLocal w;w.allocate(g,p,true);
    for(int scenario=0;scenario<5;++scenario){
        for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){w.lr[i*2*p.heads+h]=scenario==3?10000.f:scenario==4?-10000.f:float(h)-4;
            w.lr[i*2*p.heads+p.heads+h]=scenario==0?0:scenario==1?(i==34?1000:-1000):float(i)-17;}
        std::vector<float> oracle(g.n*p.width(),0);
        for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){float max=-INFINITY;for(uint64_t e=g.row[i];e<g.row[i+1];++e)max=std::max(max,leak(w.lr[i*2*p.heads+h]+w.lr[size_t(g.col[e])*2*p.heads+p.heads+h]));
            std::vector<double> u(p.in,0);double den=0;
            for(uint64_t e=g.row[i];e<g.row[i+1];++e){float score=leak(w.lr[i*2*p.heads+h]+w.lr[size_t(g.col[e])*2*p.heads+p.heads+h]);double v=std::exp(double(score)-max);den+=v;for(int d=0;d<p.in;++d)u[d]+=v*g.x[size_t(g.col[e])*p.in+d];}
            for(int d=0;d<p.dim;++d){double val=0;for(int k=0;k<p.in;++k)val+=u[k]*p.w[size_t(k)*p.width()+h*p.dim+d];oracle[i*p.width()+h*p.dim+d]=den>0?float(val/den):0;}}
        for(bool amx:{false,true})for(int block:{16,32,64}){local::Stats st;local::aggregate(g,p,q,sched,g.x,w,amx,block,64,st,0,true);auto e=compare_fast(oracle,w.out);if(!e.finite||e.max_abs>1e-4||e.relative_l2>1e-4)throw std::runtime_error("adversarial stable FP64 oracle failed");}
    }
    std::cout<<"LOCAL_SMOKE_PASS FP32_before_AMX=true shapes=48_per_backend block_sizes=16,32,64 adversarial=equal,late_peak,mixed,positive_offset,negative_offset empty_rows=true tail_rows=true\n";
}
int main(int argc,char** argv){try{
    if(argc==2&&std::string(argv[1])=="--smoke"){omp_set_num_threads(2);mkl_set_num_threads(2);smoke();return 0;}
    if(argc!=5)throw std::runtime_error("usage: local_online GRAPH THREADS WARMUPS REPEATS | --smoke");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]);omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_num_threads(threads);mkl_set_dynamic(0);
    #pragma omp parallel
    {volatile int id=omp_get_thread_num();(void)id;}
    Graph g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto sched=degree_schedule(g);
    std::vector<local::PreparedLocal> prepared;std::vector<Workspace> b0(3),b1(3);std::vector<local::WorkspaceLocal> lf(3),la(3);std::vector<std::vector<float>> reference(3);
    for(int l=0;l<3;++l){prepared.push_back(local::prepare_local(model[l]));b0[l].allocate(g,model[l],"standard_bf16");b1[l].allocate(g,model[l],"tfs_bf16");lf[l].allocate(g,model[l]);la[l].allocate(g,model[l],true);}
    std::cout<<std::setprecision(10)<<"CONFIG N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes<<" threads="<<threads<<" block=32 panel=64 tile_rows=16 degree_sort_ms="<<sched.degree_sort_s*1000<<" seed=11 untrained=true global_U=false global_Z_local=false global_e_alpha=false AMX_products=4 precision_local=FP32_U_FP32_attention_BF16_hi_lo_UW\n";
    for(int l=0;l<3;++l)std::cout<<"STATIC layer="<<l+1<<" bLR_and_weight_pack_ms="<<prepared[l].prepare_s*1000<<" local_scratch_bytes_per_worker="<<(16*prepared[l].base.padded_in*8+16*64*4)<<"\n";
    const std::vector<float>* input=&g.x;
    for(int l=0;l<3;++l){Times t;standard_layer(g,model[l],prepared[l].base,*input,b0[l],l<2,t,true);reference[l]=b0[l].out;input=&b0[l].out;}
    std::vector<std::string> names={"B0_FP32","B0_BF16","B1_TFS","local_online_fp32","local_online_amx_hilo"};
    for(int rep=-warm-1;rep<reps;++rep){auto order=names;if(rep>=0&&rep%2)std::reverse(order.begin(),order.end());
        for(const auto& name:order){input=&g.x;Times ts[3];local::Timing lt[3];auto a=Clock::now();
            for(int l=0;l<3;++l){const auto& p=model[l];if(name=="B0_FP32"||name=="B0_BF16"){standard_layer(g,p,prepared[l].base,*input,b0[l],l<2,ts[l],name=="B0_FP32");input=&b0[l].out;}
                else if(name=="B1_TFS"){tfs_layer(g,p,prepared[l].base,sched,*input,b1[l],l<2,64,ts[l],"bf16");input=&b1[l].out;}
                else {bool amx=name=="local_online_amx_hilo";auto& ws=amx?la:lf;local::layer(g,p,prepared[l],sched,*input,ws[l],l<2,amx,32,64,lt[l]);input=&ws[l].out;}}
            double total=seconds(a,Clock::now());
            if(rep==-warm-1){for(int l=0;l<3;++l){const auto& out=name=="B1_TFS"?b1[l].out:name=="local_online_fp32"?lf[l].out:name=="local_online_amx_hilo"?la[l].out:b0[l].out;report_error(name,l+1,compare_fast(reference[l],out));}}
            if(rep>=0){require_finite(*input,name);for(int l=0;l<3;++l){bool islocal=name.find("local_online")==0;
                std::cout<<std::setprecision(10)<<"{\"path\":\""<<name<<"\",\"rep\":"<<rep<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<total*1000<<",\"layer_ms\":"<<(islocal?lt[l].total:ts[l].total)*1000
                    <<",\"lr_ms\":"<<(islocal?lt[l].lr:ts[l].lr)*1000<<",\"kernel_ms\":"<<(islocal?lt[l].kernel:ts[l].aggregate)*1000<<",\"projection_ms\":"<<ts[l].projection*1000<<",\"conversion_ms\":"<<ts[l].convert*1000
                    <<",\"max_prescan_ms\":"<<ts[l].max_prescan*1000<<",\"normalization_ms\":"<<ts[l].normalize*1000<<",\"activation_ms\":"<<(islocal?lt[l].activation:ts[l].activation)*1000<<"}\n";}}
        }
    }
    uint64_t amx_hash[3];for(int l=0;l<3;++l)amx_hash[l]=fingerprint(la[l].out);
    // Separate sampled run with counters; not used in speedup medians.
    for(bool amx:{false,true}){input=&g.x;for(int l=0;l<3;++l){local::Timing t;local::layer(g,model[l],prepared[l],sched,*input,la[l],l<2,amx,32,64,t,128,true);input=&la[l].out;
        if(!amx&&std::memcmp(lf[l].out.data(),input->data(),input->size()*4))throw std::runtime_error("sample/counter path changed FP32 output");
        if(amx&&amx_hash[l]!=fingerprint(*input))throw std::runtime_error("sample/counter AMX fingerprint mismatch");
        require_finite(*input,"sample output");
        auto s=t.sample;std::cout<<std::setprecision(10)<<"PROFILE path="<<(amx?"local_online_amx_hilo":"local_online_fp32")<<" layer="<<l+1<<" layer_wall_ms="<<t.total*1000<<" kernel_wall_ms="<<t.kernel*1000<<" sampled_tiles="<<s.sampled_tiles<<" init_worker_ms="<<s.init*1000<<" score_exp_den_worker_ms="<<s.score*1000<<" rescale_worker_ms="<<s.rescale*1000<<" weighted_spmm_worker_ms="<<s.spmm*1000<<" packing_worker_ms="<<s.packing*1000<<" gemm_load_compute_store_worker_ms="<<s.gemm*1000<<" normalization_scatter_worker_ms="<<s.output*1000<<" AMX_rescale_spill_bytes=0 AMX_rescale_reload_bytes=0 U_global_bytes=0\n";local::print_stats(la[l],model[l],l+1);}}
    std::cout<<"LOCAL_COMPLETE numerical_smoke=PASS task_accuracy=UNVERIFIED performance=EXPLORATORY\n";return 0;
}catch(const std::exception& e){std::cerr<<"LOCAL_FAIL "<<e.what()<<"\n";return 1;}}
