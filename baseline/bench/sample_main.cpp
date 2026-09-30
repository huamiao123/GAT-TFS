#include "tfs_sampled.hpp"
#include <iomanip>
#include <iostream>
using namespace gat;
int main(int argc,char** argv){try{
    if(argc!=5)throw std::runtime_error("usage: sample_tfs GRAPH THREADS WARMUPS REPEATS");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]);
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_dynamic(0);mkl_set_num_threads(threads);
    #pragma omp parallel
    {volatile int id=omp_get_thread_num();(void)id;}
    std::vector<double> floors;for(int i=0;i<10001;++i){auto a=Clock::now();auto b=Clock::now();floors.push_back(seconds(a,b)*1e9);}std::sort(floors.begin(),floors.end());
    std::cout<<"CLOCK_FLOOR median_ns="<<floors[floors.size()/2]<<" calibration=empty_Clock_now_pair approximation_only=true\n";
    Graph g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto sched=degree_schedule(g);
    std::vector<Prepared> prepared;std::vector<Workspace> base(3),sample(3);
    for(int l=0;l<3;++l){prepared.push_back(prepare(model[l]));base[l].allocate(g,model[l],"tfs_bf16");sample[l].allocate(g,model[l],"tfs_bf16");}
    std::vector<float> oracle;
    for(int r=-warm;r<reps;++r){std::vector<int> paths={-1,0,128,512};if(r>=0&&r%2)std::reverse(paths.begin(),paths.end());
        for(int period:paths){auto& ws=period<0?base:sample;const std::vector<float>* input=&g.x;Times ts[3];SampleStats ss[3];auto begin=Clock::now();
            for(int l=0;l<3;++l){const auto& p=model[l];auto& w=ws[l];
                if(period<0)tfs_layer(g,p,prepared[l],sched,*input,w,l<2,64,ts[l],"bf16");
                else {auto lb=Clock::now(),a=lb;convert(input->data(),w.xbf.data(),input->size());ts[l].convert=seconds(a,Clock::now());
                    a=Clock::now();lr_reordered(g,p,prepared[l],*input,w,"bf16");ts[l].lr=seconds(a,Clock::now());
                    a=Clock::now();max_prescan(g,p,w);ts[l].max_prescan=seconds(a,Clock::now());
                    a=Clock::now();sampled_aggregate(g,p,prepared[l],sched,w,64,uint64_t(period),uint64_t(r+warm+1)*131+17,ss[l]);ts[l].aggregate=seconds(a,Clock::now());
                    finish(g,p,w,l<2,ts[l]);ts[l].total=seconds(lb,Clock::now());}
                input=&w.out;
            }
            double e2e=seconds(begin,Clock::now());require_finite(*input,"sample output");
            if(period<0&&oracle.empty())oracle=*input;
            if(oracle.size()!=input->size()||std::memcmp(oracle.data(),input->data(),oracle.size()*4))throw std::runtime_error("sampled output not bit identical to original B1");
            if(r>=0)for(int l=0;l<3;++l){auto s=ss[l];std::cout<<std::setprecision(10)<<"{\"period\":"<<period<<",\"rep\":"<<r<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<e2e*1000<<",\"layer_ms\":"<<ts[l].total*1000<<",\"kernel_ms\":"<<ts[l].aggregate*1000
                <<",\"schedule_sample_worker_ms\":"<<s.schedule*1000<<",\"score_sample_worker_ms\":"<<s.score*1000<<",\"staging_sample_worker_ms\":"<<s.stage*1000<<",\"matrix_sample_worker_ms\":"<<s.matrix*1000<<",\"output_sample_worker_ms\":"<<s.output*1000
                <<",\"sampled_tiles\":"<<s.tiles<<",\"sampled_steps\":"<<s.steps<<",\"sampled_active_edges\":"<<s.active_edges<<",\"bit_identical\":true}\n";}
        }
    }
    std::cout<<"SAMPLE_COMPLETE bit_identical_to_original_B1=true task_precision=NOT_ACCEPTED\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
