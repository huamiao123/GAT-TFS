#include "joint_online.hpp"
#include "joint_checks.hpp"
#include <iostream>
#include <iomanip>
#include <array>
using namespace gat;

static Error difference(const std::vector<float>& ref,const std::vector<float>& got){
    if(ref.size()!=got.size()||ref.empty())throw std::runtime_error("compare shape mismatch");
    double mx=0,ab=0,sq=0,norm=0;int bad=0;
    #pragma omp parallel for reduction(max:mx) reduction(+:ab,sq,norm) reduction(|:bad)
    for(size_t i=0;i<ref.size();++i){bad|=!std::isfinite(ref[i])||!std::isfinite(got[i]);double d=double(got[i])-ref[i];mx=std::max(mx,std::abs(d));ab+=std::abs(d);sq+=d*d;norm+=double(ref[i])*ref[i];}
    Error e;e.finite=!bad;e.max_abs=mx;e.mean_abs=ab/ref.size();e.relative_l2=norm?std::sqrt(sq/norm):(sq?INFINITY:0);
    if(bad)e.max_abs=e.mean_abs=e.relative_l2=INFINITY;return e;
}
static void print_check(const std::string& label,int l,const Error& e,const std::string& reference){
    std::cout<<std::setprecision(12)<<"CHECK path="<<label<<" layer="<<l<<" reference="<<reference<<" max_abs_error="<<e.max_abs<<" mean_abs_error="<<e.mean_abs<<" relative_L2_error="<<e.relative_l2<<" finite="<<e.finite<<" task_acceptance=UNVERIFIED\n";
    if(!e.finite)throw std::runtime_error("nonfinite check "+label);
}
static uint64_t fingerprint(const std::vector<float>& a){uint64_t result=0;
    #pragma omp parallel for reduction(^:result)
    for(size_t i=0;i<a.size();++i){uint32_t bits;std::memcpy(&bits,a.data()+i,4);uint64_t z=uint64_t(bits)^((uint64_t(i)+1)*0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;result^=z^(z>>27);}return result;
}
static Graph fixture(int D){Graph g;g.n=35;g.din=D;g.classes=47;g.row={0};int deg[]={0,1,2,15,16,17,31,32,33,63,64,65,129};
    for(uint64_t i=0;i<g.n;++i){for(int k=0;k<deg[i%13];++k)g.col.push_back((i*3+k*7)%g.n);g.row.push_back(g.col.size());}g.e=g.col.size();g.x.resize(g.n*D);for(size_t i=0;i<g.x.size();++i)g.x[i]=std::sin(float(i)*.17f)*.5f;return g;}
static void smoke(){
    uint64_t num_checks=0,bit_equal=0;double worst_ref=0,worst_local=0;
    for(int D:{17,33,128,256})for(int K:{1,2,8})for(int dim:{7,32,40,47}){
        Graph g=fixture(D);auto p=random_model(D,dim,11)[0];p.heads=K;p.dim=dim;p.w.resize(D*K*dim);p.al.resize(K*dim);p.ar.resize(K*dim);
        for(size_t i=0;i<p.w.size();++i)p.w[i]=std::sin(float(i)*.39f)*.07f;
        for(size_t i=0;i<p.al.size();++i){p.al[i]=std::cos(float(i)*.43f)*.2f;p.ar[i]=std::sin(float(i)*.27f)*.2f;}
        auto q=local::prepare_local(p);auto sched=degree_schedule(g);Workspace ref;ref.allocate(g,p,"ref_fp32");Times rt;ref_layer(g,p,g.x,ref,false,rt);
        local::WorkspaceLocal old;old.allocate(g,p,true);joint::WorkspaceJoint now;now.allocate(g,p,true);
        for(int block:{16,32,64}){local::Timing lt;local::layer(g,p,q,sched,g.x,old,false,false,block,64,lt,0,true);
            for(int group:{1,2,4,8}){joint::Timing jt;joint::layer(g,p,q,sched,g.x,now,false,group,block,64,jt,0,true);
                auto er=difference(ref.out,now.out),el=difference(old.out,now.out);++num_checks;bit_equal+=std::memcmp(old.out.data(),now.out.data(),now.out.size()*4)==0;
                worst_ref=std::max(worst_ref,er.max_abs);worst_local=std::max(worst_local,el.max_abs);
                if(!er.finite||er.max_abs>1e-4||er.relative_l2>1e-4||!el.finite||el.max_abs>1e-6||el.relative_l2>1e-6){
                    std::cerr<<"JOINT_SMOKE_MISMATCH D="<<D<<" K="<<K<<" d="<<dim<<" block="<<block<<" group="<<group<<" R0_abs="<<er.max_abs<<" R0_rel="<<er.relative_l2<<" local_abs="<<el.max_abs<<" local_rel="<<el.relative_l2<<"\n";throw std::runtime_error("joint smoke gate failed");}
                if(now.blocks!=old.blocks||now.updates!=old.updates||now.rescales!=old.rescales)throw std::runtime_error("joint online counters changed");
                for(int d=0;d<p.width();++d)if(now.out[d]!=0)throw std::runtime_error("empty row changed");
            }
        }
    }
    Graph g=fixture(33);auto p=random_model(33,32,11)[0];auto q=local::prepare_local(p);auto sched=degree_schedule(g);joint::WorkspaceJoint w;w.allocate(g,p,true);
    checks::Limits limit;limit.max_rows=64;auto rows=checks::choose_rows(g,p,limit);
    for(int scenario=0;scenario<5;++scenario){
        for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){w.lr[i*2*p.heads+h]=scenario==3?10000.f:scenario==4?-10000.f:float(h)-4;
            w.lr[i*2*p.heads+p.heads+h]=scenario==0?0:scenario==1?(i==34?1000:-1000):float(i)-17;}
        auto oracle=checks::stable_oracle(g,p,g.x,w.lr,rows,false);
        for(int block:{16,32,64})for(int group:{1,2,4,8}){joint::Stats s;joint::aggregate(g,p,q,sched,g.x,w,group,block,64,s,0,true);
            auto e=checks::compare_sampled(oracle,g,p,w.out).error;if(!e.finite||e.max_abs>1e-4||e.relative_l2>1e-4)throw std::runtime_error("joint stable FP64 gate failed");}
    }
    std::cout<<std::setprecision(12)<<"JOINT_SMOKE_PASS checks="<<num_checks<<" bit_equal_to_local="<<bit_equal<<" max_ref_abs="<<worst_ref<<" max_local_abs="<<worst_local<<" groups=1,2,4,8 blocks=16,32,64 FP32_only=true online_counters_identical=true\n";
}
static void matched_control(const Graph& g,const Param& p,const local::PreparedLocal& q,const Schedule& sched,
                            const std::vector<float>& input,Workspace& tf,local::WorkspaceLocal& old,joint::WorkspaceJoint& now,int layer){
    Times t;standard_layer(g,p,q.base,input,tf,false,t,true);
    const auto select=checks::choose_rows(g,p);checks::print_selection(std::cout,"common_input",layer,select);
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),2*p.heads,p.in,1,input.data(),p.in,q.base.blr.data(),2*p.heads,0,now.lr.data(),2*p.heads);
    auto baseline_lr=checks::AttentionView::from_workspace(tf,g.n,p.heads),contracted_lr=checks::AttentionView::from_interleaved(now.lr,g.n,p.heads);
    checks::print_attention_drift(std::cout,"contracted_vs_B0",layer,checks::attention_drift(g,p,baseline_lr,contracted_lr,select));
    auto oracle=checks::stable_oracle(g,p,input,baseline_lr,select,false);
    checks::print_comparison(std::cout,"FP64_fixed_attention_associativity",layer,oracle.associativity);
    checks::print_comparison(std::cout,"B0_vs_fixed_attention_FP64",layer,checks::compare_sampled(oracle,g,p,tf.out,checks::OracleBranch::TransformFirst));
    // Freeze identical B0 left/right. Do not generate a second attention matrix.
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){now.lr[i*2*p.heads+h]=tf.left[i*p.heads+h];now.lr[i*2*p.heads+p.heads+h]=tf.right[i*p.heads+h];}
    std::copy(now.lr.begin(),now.lr.end(),old.lr.begin());local::Stats os;local::aggregate(g,p,q,sched,input,old,false,32,64,os);
    for(int group:{1,2,4,8}){joint::Stats s;joint::aggregate(g,p,q,sched,input,now,group,32,64,s);
        auto direct=difference(old.out,now.out);const bool bits=std::memcmp(old.out.data(),now.out.data(),now.out.size()*4)==0;
        print_check("fixed_LR_joint_g"+std::to_string(group),layer,direct,"local_same_input_same_LR");
        std::cout<<"MATCHED_IDENTITY layer="<<layer<<" group="<<group<<" bit_equal="<<bits<<"\n";
        // Declared operator gate; original task/reference absolute gate remains unchanged.
        if(!direct.finite||direct.max_abs>.003||direct.relative_l2>1e-5)throw std::runtime_error("matched joint operator gate failed");
        checks::print_comparison(std::cout,"joint_fixed_LR_g"+std::to_string(group),layer,checks::compare_sampled(oracle,g,p,now.out));
        print_check("fixed_LR_TF_vs_AF_g"+std::to_string(group),layer,difference(tf.out,now.out),"B0_same_input_same_LR");
    }
    // Isolate attention reassociation: change L/R only in the transform-first backend.
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),2*p.heads,p.in,1,input.data(),p.in,q.base.blr.data(),2*p.heads,0,now.lr.data(),2*p.heads);
    std::vector<float> tf_original=tf.out;
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){tf.left[i*p.heads+h]=now.lr[i*2*p.heads+h];tf.right[i*p.heads+h]=now.lr[i*2*p.heads+p.heads+h];}
    max_prescan(g,p,tf);standard_aggregate(g,p,tf);Times ft;finish(g,p,tf,false,ft);
    print_check("TF_attention_only_shift",layer,difference(tf_original,tf.out),"same_Z_original_B0_LR");
    joint::Stats js;joint::aggregate(g,p,q,sched,input,now,8,32,64,js);
    print_check("contracted_fixed_LR_TF_vs_AF",layer,difference(tf.out,now.out),"TF_same_contracted_LR");
}
int main(int argc,char** argv){try{
    if(argc==2&&std::string(argv[1])=="--smoke"){omp_set_dynamic(0);omp_set_num_threads(2);mkl_set_num_threads(2);mkl_set_dynamic(0);smoke();return 0;}
    if(argc!=5)throw std::runtime_error("usage: joint_online GRAPH THREADS WARMUPS REPEATS | --smoke");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]);if(threads<1||warm<0||reps<1)throw std::runtime_error("bad experiment config");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_num_threads(threads);mkl_set_dynamic(0);
    #pragma omp parallel
    {volatile int id=omp_get_thread_num();(void)id;}
    Graph g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto sched=degree_schedule(g);
    std::vector<local::PreparedLocal> prepared;std::vector<Workspace> b0(3),b1(3);std::vector<local::WorkspaceLocal> old(3);std::vector<joint::WorkspaceJoint> now(3);std::vector<std::vector<float>> reference(3);
    for(int l=0;l<3;++l){prepared.push_back(local::prepare_local(model[l]));b0[l].allocate(g,model[l],"standard_bf16");b1[l].allocate(g,model[l],"tfs_bf16");old[l].allocate(g,model[l]);now[l].allocate(g,model[l],true);}
    std::cout<<std::setprecision(12)<<"CONFIG N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes<<" threads="<<threads<<" warmups="<<warm<<" repeats="<<reps<<" block=32 panel=64 tile_rows=16 seed=11 untrained=true degree_sort_ms="<<sched.degree_sort_s*1000<<" global_U=false global_Z_joint=false global_e_alpha_joint=false precision_joint=FP32 task_acceptance=UNVERIFIED\n";
    const std::vector<float>* input=&g.x;
    for(int l=0;l<3;++l){Times t;standard_layer(g,model[l],prepared[l].base,*input,b0[l],l<2,t,true);reference[l]=b0[l].out;input=&b0[l].out;
        std::cout<<"STATIC layer="<<l+1<<" prepare_ms="<<prepared[l].prepare_s*1000<<" joint_U_scratch_bytes_per_worker="<<16ull*model[l].heads*prepared[l].base.padded_in*4<<"\n";}
    for(int l=0;l<3;++l)matched_control(g,model[l],prepared[l],sched,l?reference[l-1]:g.x,b0[l],old[l],now[l],l+1);
    const std::vector<std::string> names={"B0_FP32","B0_BF16","B1_TFS","local_online_fp32","joint_g1","joint_g2","joint_g4","joint_g8"};
    for(int rep=-warm-1;rep<reps;++rep){auto order=names;if(rep>=0&&rep%2)std::reverse(order.begin(),order.end());
        for(const auto& name:order){input=&g.x;Times ts[3];local::Timing lt[3];joint::Timing jt[3];auto begin=Clock::now();
            for(int l=0;l<3;++l){const auto& p=model[l];if(name=="B0_FP32"||name=="B0_BF16"){standard_layer(g,p,prepared[l].base,*input,b0[l],l<2,ts[l],name=="B0_FP32");input=&b0[l].out;}
                else if(name=="B1_TFS"){tfs_layer(g,p,prepared[l].base,sched,*input,b1[l],l<2,64,ts[l],"bf16");input=&b1[l].out;}
                else if(name=="local_online_fp32"){local::layer(g,p,prepared[l],sched,*input,old[l],l<2,false,32,64,lt[l]);input=&old[l].out;}
                else{int group=std::stoi(name.substr(7));joint::layer(g,p,prepared[l],sched,*input,now[l],l<2,group,32,64,jt[l]);input=&now[l].out;}}
            const double total=seconds(begin,Clock::now());
            if(rep==-warm-1)for(int l=0;l<3;++l){const auto& out=name=="B1_TFS"?b1[l].out:name=="local_online_fp32"?old[l].out:name.find("joint_")==0?now[l].out:b0[l].out;print_check(name,l+1,difference(reference[l],out),"B0_FP32_full_model");}
            if(rep>=0){require_finite(*input,name);for(int l=0;l<3;++l){const bool j=name.find("joint_")==0,lc=name=="local_online_fp32";const auto& p=model[l];const int group=j?std::min(p.heads,std::stoi(name.substr(7))):p.heads;
                std::cout<<std::setprecision(12)<<"{\"path\":\""<<name<<"\",\"rep\":"<<rep<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<total*1000<<",\"layer_ms\":"<<(j?jt[l].total:lc?lt[l].total:ts[l].total)*1000
                    <<",\"lr_ms\":"<<(j?jt[l].lr:lc?lt[l].lr:ts[l].lr)*1000<<",\"kernel_ms\":"<<(j?jt[l].kernel:lc?lt[l].kernel:ts[l].aggregate)*1000<<",\"projection_ms\":"<<ts[l].projection*1000<<",\"conversion_ms\":"<<ts[l].convert*1000
                    <<",\"max_prescan_ms\":"<<ts[l].max_prescan*1000<<",\"normalization_ms\":"<<ts[l].normalize*1000<<",\"activation_ms\":"<<(j?jt[l].activation:lc?lt[l].activation:ts[l].activation)*1000<<",\"source_logical_fp32_bytes\":"<<(j?4ull*g.e*p.in*((p.heads+group-1)/group):lc?4ull*g.e*p.in*p.heads:0)<<"}\n";}}
        }
    }
    // Independent no-clock g8 output fingerprint then profile; these runs are outside speed medians.
    uint64_t hashes[3];input=&g.x;for(int l=0;l<3;++l){joint::Timing t;joint::layer(g,model[l],prepared[l],sched,*input,now[l],l<2,8,32,64,t);input=&now[l].out;hashes[l]=fingerprint(*input);}
    input=&g.x;for(int l=0;l<3;++l){joint::Timing t;joint::layer(g,model[l],prepared[l],sched,*input,now[l],l<2,8,32,64,t,128,true);input=&now[l].out;if(hashes[l]!=fingerprint(*input))throw std::runtime_error("joint profile fingerprint differs");auto s=t.sample;
        std::cout<<std::setprecision(12)<<"PROFILE path=joint_g8 layer="<<l+1<<" layer_wall_ms="<<t.total*1000<<" kernel_wall_ms="<<t.kernel*1000<<" sampled_tiles="<<s.sampled_tiles<<" sampled_neighbor_group_blocks="<<s.sampled_neighbor_group_blocks<<" sampled_row_head_blocks="<<s.sampled_row_head_blocks<<" sampled_source_vector_loads="<<s.sampled_source_vector_loads<<" sampled_fma_vectors="<<s.sampled_fma_vectors<<" init_worker_ms="<<s.init*1000<<" score_exp_den_worker_ms="<<s.score*1000<<" weighted_spmm_rescale_worker_ms="<<s.spmm_rescale*1000<<" UW_worker_ms="<<s.gemm*1000<<" normalization_scatter_worker_ms="<<s.output*1000<<" U_global_bytes=0 rescale_timing=fused_with_weighted_spmm\n";joint::print_stats(now[l],model[l],l+1);}
    std::cout<<"JOINT_COMPLETE implementation_gate=PASS master_reference_gate=UNVERIFIED task_accuracy=UNVERIFIED performance=EXPLORATORY\n";return 0;
}catch(const std::exception& e){std::cerr<<"JOINT_FAIL "<<e.what()<<"\n";return 1;}}
