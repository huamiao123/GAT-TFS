#include "joint_full.hpp"
#include <iostream>
#include <iomanip>
#include <array>
using namespace gat;

static Error diff(const std::vector<float>& a,const std::vector<float>& b){
    if(a.size()!=b.size()||a.empty())throw std::runtime_error("compare shape");
    double mx=0,ab=0,sq=0,norm=0;int bad=0;
    #pragma omp parallel for reduction(max:mx) reduction(+:ab,sq,norm) reduction(|:bad)
    for(size_t i=0;i<a.size();++i){bad|=!std::isfinite(a[i])||!std::isfinite(b[i]);double d=double(b[i])-a[i];mx=std::max(mx,std::abs(d));ab+=std::abs(d);sq+=d*d;norm+=double(a[i])*a[i];}
    Error e;e.finite=!bad;e.max_abs=mx;e.mean_abs=ab/a.size();e.relative_l2=norm?std::sqrt(sq/norm):(sq?INFINITY:0);if(bad)e.max_abs=e.mean_abs=e.relative_l2=INFINITY;return e;
}
static void report(const std::string& path,int layer,const Error& e,const std::string& ref){
    std::cout<<std::setprecision(12)<<"CHECK path="<<path<<" layer="<<layer<<" reference="<<ref<<" max_abs_error="<<e.max_abs<<" mean_abs_error="<<e.mean_abs<<" relative_L2_error="<<e.relative_l2<<" finite="<<e.finite<<" master_gate=UNVERIFIED\n";
    if(!e.finite)throw std::runtime_error("nonfinite output");
}
static uint64_t hash(const std::vector<float>& x){uint64_t out=0;
    #pragma omp parallel for reduction(^:out)
    for(size_t i=0;i<x.size();++i){uint32_t bits;std::memcpy(&bits,x.data()+i,4);uint64_t v=uint64_t(bits)^((uint64_t(i)+1)*0x9e3779b97f4a7c15ULL);v=(v^(v>>30))*0xbf58476d1ce4e5b9ULL;out^=v^(v>>27);}return out;
}
static Graph graph(int D){Graph g;g.n=35;g.din=D;g.classes=47;g.row={0};const int degrees[]={0,1,2,15,16,17,31,32,33,63,64,65,129};
    for(uint64_t i=0;i<g.n;++i){for(int k=0;k<degrees[i%13];++k)g.col.push_back((i*3+k*7)%g.n);g.row.push_back(g.col.size());}g.e=g.col.size();g.x.resize(g.n*D);for(size_t i=0;i<g.x.size();++i)g.x[i]=std::sin(float(i)*.17f)*.5f;return g;
}
static void smoke(){uint64_t tested=0,bits=0;double mx=0;
    for(int D:{17,33,128,256})for(int K:{1,2,3,4,5,7,8})for(int dim:{7,32,47}){
        auto g=graph(D);auto p=random_model(D,dim,11)[0];p.heads=K;p.dim=dim;p.w.resize(D*K*dim);p.al.resize(K*dim);p.ar.resize(K*dim);
        for(size_t i=0;i<p.w.size();++i)p.w[i]=std::sin(float(i)*.39f)*.07f;
        for(size_t i=0;i<p.al.size();++i){p.al[i]=std::cos(float(i)*.43f)*.2f;p.ar[i]=std::sin(float(i)*.27f)*.2f;}
        auto q=local::prepare_local(p);auto sched=degree_schedule(g);joint::WorkspaceJoint v1,v2;v1.allocate(g,p,true);v2.allocate(g,p,true);
        for(int block:{16,32,64})for(int group:{1,2,4,8}){joint::Timing a,b;joint::layer(g,p,q,sched,g.x,v1,true,group,block,64,a,0,true);joint_full::layer(g,p,q,sched,g.x,v2,true,group,block,64,b,0,true);
            auto e=diff(v1.out,v2.out);++tested;bits+=std::memcmp(v1.out.data(),v2.out.data(),v1.out.size()*4)==0;mx=std::max(mx,e.max_abs);
            if(!e.finite||e.max_abs>1e-6||e.relative_l2>1e-6)throw std::runtime_error("full-group smoke numeric gate");
            if(v1.blocks!=v2.blocks||v1.updates!=v2.updates||v1.rescales!=v2.rescales)throw std::runtime_error("full-group smoke counters");
        }
    }
    std::cout<<"JOINT_FULL_SMOKE_PASS checks="<<tested<<" bit_equal="<<bits<<" max_abs="<<mx<<" heads=1,2,3,4,5,7,8 groups=1,2,4,8 blocks=16,32,64\n";
}
struct Name{std::string path;int group;bool full;};
int main(int argc,char** argv){try{
    if(argc==2&&std::string(argv[1])=="--smoke"){omp_set_dynamic(0);omp_set_num_threads(2);mkl_set_num_threads(2);mkl_set_dynamic(0);smoke();return 0;}
    if(argc!=5)throw std::runtime_error("usage: joint_compare GRAPH THREADS WARMUPS REPEATS | --smoke");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]);if(threads<1||warm<0||reps<1)throw std::runtime_error("bad config");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_num_threads(threads);mkl_set_dynamic(0);
    #pragma omp parallel
    {volatile int id=omp_get_thread_num();(void)id;}
    auto g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto sched=degree_schedule(g);
    std::vector<local::PreparedLocal> prep;std::vector<Workspace> base(3);std::vector<joint::WorkspaceJoint> v1(3),v2(3);std::vector<std::vector<float>> ref(3);
    for(int l=0;l<3;++l){prep.push_back(local::prepare_local(model[l]));base[l].allocate(g,model[l],"standard_bf16");v1[l].allocate(g,model[l],true);v2[l].allocate(g,model[l],true);}
    std::cout<<std::setprecision(12)<<"CONFIG N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes<<" threads="<<threads<<" warmups="<<warm<<" repeats="<<reps<<" seed=11 untrained=true block=32 panel=64 tile_rows=16 degree_sort_ms="<<sched.degree_sort_s*1000<<" full_group_guards=compile_time_removed master_reference_gate=UNVERIFIED\n";
    const std::vector<float>* x=&g.x;for(int l=0;l<3;++l){Times t;standard_layer(g,model[l],prep[l].base,*x,base[l],l<2,t,true);ref[l]=base[l].out;x=&base[l].out;
        std::cout<<"STATIC layer="<<l+1<<" prepare_ms="<<prep[l].prepare_s*1000<<" joint_U_scratch_bytes_per_worker="<<16ull*model[l].heads*prep[l].base.padded_in*4<<"\n";}
    for(int l=0;l<3;++l){const auto& input=l?ref[l-1]:g.x;joint::Timing t;joint::layer(g,model[l],prep[l],sched,input,v1[l],l<2,8,32,64,t);
        for(int group:{1,2,4,8}){joint::Timing u;joint_full::layer(g,model[l],prep[l],sched,input,v2[l],l<2,group,32,64,u);auto e=diff(v1[l].out,v2[l].out);report("full_fixed_input_g"+std::to_string(group),l+1,e,"joint_v1_g8_same_input");
            std::cout<<"MATCHED_IDENTITY layer="<<l+1<<" group="<<group<<" bit_equal="<<(std::memcmp(v1[l].out.data(),v2[l].out.data(),v1[l].out.size()*4)==0)<<"\n";
            if(e.max_abs>.003||e.relative_l2>1e-5)throw std::runtime_error("full-group real fixed input gate");}}
    const std::vector<Name> names={{"B0_FP32",0,false},{"B0_BF16",0,false},{"joint_v1_g2",2,false},{"joint_v1_g8",8,false},{"joint_full_g2",2,true},{"joint_full_g4",4,true},{"joint_full_g8",8,true}};
    for(int rep=-warm-1;rep<reps;++rep){auto order=names;if(rep>=0&&rep%2)std::reverse(order.begin(),order.end());for(const auto& name:order){x=&g.x;Times bt[3];joint::Timing jt[3];auto begin=Clock::now();
        for(int l=0;l<3;++l){if(!name.group){standard_layer(g,model[l],prep[l].base,*x,base[l],l<2,bt[l],name.path=="B0_FP32");x=&base[l].out;}
            else{auto& w=name.full?v2[l]:v1[l];if(name.full)joint_full::layer(g,model[l],prep[l],sched,*x,w,l<2,name.group,32,64,jt[l]);else joint::layer(g,model[l],prep[l],sched,*x,w,l<2,name.group,32,64,jt[l]);x=&w.out;}}
        auto total=seconds(begin,Clock::now());if(rep==-warm-1)for(int l=0;l<3;++l)report(name.path,l+1,diff(ref[l],!name.group?base[l].out:name.full?v2[l].out:v1[l].out),"B0_FP32_full_model");
        if(rep>=0){require_finite(*x,name.path);for(int l=0;l<3;++l)std::cout<<std::setprecision(12)<<"{\"path\":\""<<name.path<<"\",\"rep\":"<<rep<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<total*1000<<",\"layer_ms\":"<<(name.group?jt[l].total:bt[l].total)*1000<<",\"lr_ms\":"<<(name.group?jt[l].lr:bt[l].lr)*1000<<",\"kernel_ms\":"<<(name.group?jt[l].kernel:bt[l].aggregate)*1000<<",\"projection_ms\":"<<bt[l].projection*1000<<",\"conversion_ms\":"<<bt[l].convert*1000<<",\"max_prescan_ms\":"<<bt[l].max_prescan*1000<<",\"normalization_ms\":"<<bt[l].normalize*1000<<",\"activation_ms\":"<<(name.group?jt[l].activation:bt[l].activation)*1000<<"}\n";}}
    }
    for(const auto& name:names)if(name.group){uint64_t fingerprints[3];x=&g.x;for(int l=0;l<3;++l){joint::Timing t;auto& w=name.full?v2[l]:v1[l];if(name.full)joint_full::layer(g,model[l],prep[l],sched,*x,w,l<2,name.group,32,64,t);else joint::layer(g,model[l],prep[l],sched,*x,w,l<2,name.group,32,64,t);x=&w.out;fingerprints[l]=hash(*x);}
        x=&g.x;for(int l=0;l<3;++l){joint::Timing t;auto& w=name.full?v2[l]:v1[l];if(name.full)joint_full::layer(g,model[l],prep[l],sched,*x,w,l<2,name.group,32,64,t,128,true);else joint::layer(g,model[l],prep[l],sched,*x,w,l<2,name.group,32,64,t,128,true);x=&w.out;if(fingerprints[l]!=hash(*x))throw std::runtime_error("full compare profile hash");const auto& s=t.sample;
            std::cout<<std::setprecision(12)<<"PROFILE path="<<name.path<<" layer="<<l+1<<" layer_wall_ms="<<t.total*1000<<" kernel_wall_ms="<<t.kernel*1000<<" sampled_tiles="<<s.sampled_tiles<<" init_worker_ms="<<s.init*1000<<" score_exp_den_worker_ms="<<s.score*1000<<" weighted_spmm_rescale_worker_ms="<<s.spmm_rescale*1000<<" UW_worker_ms="<<s.gemm*1000<<" normalization_scatter_worker_ms="<<s.output*1000<<" sampled_source_vector_loads="<<s.sampled_source_vector_loads<<" sampled_fma_vectors="<<s.sampled_fma_vectors<<"\n";
            if(name.full&&name.group==8)joint::print_stats(w,model[l],l+1);}}
    std::cout<<"JOINT_COMPLETE implementation_gate=PASS master_reference_gate=UNVERIFIED task_accuracy=UNVERIFIED performance=EXPLORATORY\n";return 0;
}catch(const std::exception& e){std::cerr<<"JOINT_FAIL "<<e.what()<<"\n";return 1;}}
