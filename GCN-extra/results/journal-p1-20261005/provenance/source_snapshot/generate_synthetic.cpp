#include <algorithm>
#include <numeric>
#include <vector>
#include <random>
#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include <filesystem>
int main(int argc,char** argv) {
    if(argc!=2)return 1;const uint64_t n=524288,d=128,e=n*d;
    std::vector<uint32_t> ip(n+1),src(n),col(e);std::vector<int> perm(n);
    for(uint64_t i=0;i<=n;i++)ip[i]=i*d;
    std::iota(perm.begin(),perm.end(),0);std::iota(src.begin(),src.end(),0);
    std::sort(perm.begin(),perm.end(),[&](int a,int b){return (ip[a+1]-ip[a])<(ip[b+1]-ip[b]);});
    uint64_t hash=1469598103934665603ULL;for(int row:perm)hash=(hash^uint64_t(row))*1099511628211ULL;
    std::mt19937 gen(20261005);std::shuffle(src.begin(),src.end(),gen);
    for(uint64_t reuse:{1,4,16}) {
        std::vector<uint32_t> counts(n,0);
        for(uint64_t pos=0;pos<n;pos++) {
            uint64_t tile=pos/16,lane=pos%16,group=lane/reuse;
            uint64_t base=(tile*(16/reuse)*d+group*d)%n;
            for(uint64_t k=0;k<d;k++){uint32_t j=src[(base+k)%n];col[uint64_t(perm[pos])*d+k]=j;counts[j]++;}
        }
        for(uint32_t c:counts)if(c!=d)throw std::runtime_error("source marginal frequency changed");
        std::string name="regular128_reuse"+std::to_string(reuse);auto folder=std::filesystem::path(argv[1])/name;std::filesystem::create_directories(folder);
        FILE* f=fopen((folder/(name+".csrbin")).c_str(),"wb");if(!f)throw std::runtime_error("open CSR failed");
        uint32_t types[]={0,0,2};uint64_t dims[]={n,n,e};
        fwrite(types,4,3,f);fwrite(dims,8,3,f);fwrite(ip.data(),4,n+1,f);fwrite(col.data(),4,e,f);
        std::vector<float> values(1<<20,1.f);for(uint64_t offset=0;offset<e;offset+=values.size())fwrite(values.data(),4,std::min<uint64_t>(values.size(),e-offset),f);
        if(fclose(f))throw std::runtime_error("close CSR failed");
        printf("SYNTHETIC graph=%s N=%llu E=%llu degree=%llu global_source_indegree=%llu native_degree_sort_tile_reuse=%llu source_seed=20261005 native_perm_fnv64=%llu\n",name.c_str(),(unsigned long long)n,(unsigned long long)e,(unsigned long long)d,(unsigned long long)d,(unsigned long long)reuse,(unsigned long long)hash);
    }
}
