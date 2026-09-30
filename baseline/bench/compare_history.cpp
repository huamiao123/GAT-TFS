// Controlled bridge: same real graph, bit-identical parameters, affinity, NUMA,
// and allocation-inclusive three-layer timing for historical and current paths.
#include "gat.hpp"
#define GAT_TFS_EMBEDDED
#include "gat_tfs_online.cpp"
#include <iomanip>
#include <iostream>
int main(int argc,char** argv){try{
    if(argc!=5)throw std::runtime_error("usage: compare_history GRAPH THREADS WARMUPS REPEATS");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]);
    if(threads<1||warm<0||reps<1)throw std::runtime_error("bad benchmark controls");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_dynamic(0);mkl_set_num_threads(threads);
    #pragma omp parallel
    {volatile int tid=omp_get_thread_num();(void)tid;}
    ::Graph g=::load_graph(argv[1]);auto old=model(g);
    auto current=gat::random_model(g.din,g.classes,11);
    for(int l=0;l<3;++l)for(auto pair:{std::make_pair(&old[l].w,&current[l].w),std::make_pair(&old[l].al,&current[l].al),std::make_pair(&old[l].ar,&current[l].ar)})
        if(pair.first->size()!=pair.second->size()||std::memcmp(pair.first->data(),pair.second->data(),pair.first->size()*4))throw std::runtime_error("parameters are not bit identical");
    gat::Graph ng;ng.n=g.n;ng.e=g.e;ng.din=g.din;ng.classes=g.classes;ng.row=g.row;ng.col=g.col;ng.x=g.x;
    double sort_s=0;auto perm=degree_perm(g,sort_s);auto schedule=gat::degree_schedule(ng);
    if(perm!=schedule.perm)throw std::runtime_error("degree permutations differ");
    std::cout<<"BRIDGE graph="<<argv[1]<<" N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes
       <<" parameters=BIT_IDENTICAL seed=11/22/33 degree_perm=IDENTICAL threads="<<threads
       <<" boundary=three_layer_forward_including_workspace_and_weight_preparation excludes_graph_model_init_sort_checks_and_prints=true\n";
    std::vector<float> oracle;
    const std::vector<std::string> paths={"old_reference","old_panel_transform_first","old_tfs_local_u","new_standard_fp32","new_standard_bf16","new_tfs_edge_projection"};
    for(int round=-warm;round<reps;++round){
        auto order=paths;if(round>=0&&round%2)std::reverse(order.begin(),order.end());
        for(const auto& path:order){std::vector<float> h=g.x;double layer_ms[3]={};
            auto begin=gat::Clock::now();
            for(int l=0;l<3;++l){auto start=gat::Clock::now();
                if(path=="old_reference")h=::reference(g,h,old[l],l<2).out;
                else if(path=="old_panel_transform_first")h=panel_transform_first(g,h,old[l],32,l<2,perm,64).out;
                else if(path=="old_tfs_local_u")h=tfs_online_fused(g,h,old[l],32,l<2,perm,64).out;
                else {bool tfs=path=="new_tfs_edge_projection",fp32=path=="new_standard_fp32";
                    auto p=gat::prepare(current[l],tfs,tfs,!fp32);gat::Workspace w;
                    w.allocate(ng,current[l],tfs?"tfs_bf16":fp32?"standard_fp32":"standard_bf16");gat::Times t;
                    if(tfs)gat::tfs_layer(ng,current[l],p,schedule,h,w,l<2,64,t,"bf16");
                    else gat::standard_layer(ng,current[l],p,h,w,l<2,t,fp32);
                    h=std::move(w.out);
                }
                layer_ms[l]=gat::seconds(start,gat::Clock::now())*1000;
            }
            double total=gat::seconds(begin,gat::Clock::now())*1000;
            gat::require_finite(h,path);
            if(path=="old_reference"&&oracle.empty())oracle=h;
            if(!oracle.empty()&&round==-warm)gat::print_error(path,3,"bridge_same_parameters",gat::errors(oracle,h));
            if(round>=0)std::cout<<std::setprecision(10)<<"{\"path\":\""<<path<<"\",\"rep\":"<<round<<",\"total_ms\":"<<total
                <<",\"layer1_ms\":"<<layer_ms[0]<<",\"layer2_ms\":"<<layer_ms[1]<<",\"layer3_ms\":"<<layer_ms[2]<<",\"finite\":true}\n";
        }
    }
    std::cout<<"BRIDGE_COMPLETE task_accuracy=UNCONFIGURED random_parameters=true\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
