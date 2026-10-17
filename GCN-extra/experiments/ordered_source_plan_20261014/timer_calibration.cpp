#include <omp.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>

// Calibration only: same omp_get_wtime tick/tock expression as frozen helpers,
// with last delta saved AFTER timestamp read. This is not a kernel benchmark.
struct Profile { std::array<double,24> detail_sec{}; std::array<uint64_t,24> detail_events{}; double last=0; };
template<bool P> inline double tick(bool sample){if constexpr(P){if(sample)return omp_get_wtime();}return 0;}
template<bool P> inline void detail_tock(Profile& pr,int phase,double start,bool sampled){
    if constexpr(P)if(sampled){const double dt=omp_get_wtime()-start;pr.detail_sec[phase]+=dt;pr.detail_events[phase]++;pr.last=dt;}
}
struct Result { double mean=0,median=0,min_positive=0,p99=0,ulp=0,base=0;uint64_t zeros=0,negative=0; };
int main(){
    const int nt=omp_get_max_threads(),repeats=11,samples=20000;
    std::vector<std::array<Result,11>> rr(nt);
#pragma omp parallel
    {
        int tid=omp_get_thread_num();std::vector<double>d(samples);
        for(int rep=0;rep<repeats;rep++){
            Profile p;Result r;r.base=omp_get_wtime();r.ulp=std::nextafter(r.base,INFINITY)-r.base;
            for(int i=0;i<samples;i++){double t=tick<true>(true);detail_tock<true>(p,4,t,true);d[i]=p.last;}
            r.mean=p.detail_sec[4]/p.detail_events[4];
            for(double v:d){r.zeros+=v==0;r.negative+=v<0;if(v>0&&(r.min_positive==0||v<r.min_positive))r.min_positive=v;}
            std::sort(d.begin(),d.end());r.median=(d[samples/2-1]+d[samples/2])/2;r.p99=d[(samples*99)/100];rr[tid][rep]=r;
        }
    }
    std::puts("tid,repeat,warmup,samples,mean_empty_ns,median_empty_ns,min_positive_ns,p99_ns,zero_fraction,negative_count,timestamp_ulp_ns,timestamp_base_s");
    for(int tid=0;tid<nt;tid++)for(int rep=0;rep<repeats;rep++){
        const auto&r=rr[tid][rep];std::printf("%d,%d,%d,%d,%.12g,%.12g,%.12g,%.12g,%.12g,%llu,%.12g,%.17g\n",tid,rep,int(rep==0),samples,r.mean*1e9,r.median*1e9,r.min_positive*1e9,r.p99*1e9,double(r.zeros)/samples,(unsigned long long)r.negative,r.ulp*1e9,r.base);
    }
}
