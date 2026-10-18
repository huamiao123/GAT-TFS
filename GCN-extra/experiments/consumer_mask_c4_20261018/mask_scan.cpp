#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using U = uint64_t;
static void check(bool b, const char* m) { if (!b) throw std::runtime_error(m); }
struct Graph {
    int fd=-1; void* ptr=MAP_FAILED; size_t bytes=0;
    U n=0,e=0; const uint32_t *row=nullptr,*col=nullptr;
    explicit Graph(const std::string& path) {
        fd=open(path.c_str(),O_RDONLY); check(fd>=0,"open failed");
        struct stat st{};check(fstat(fd,&st)==0 && st.st_size>=36,"invalid file size");
        bytes=size_t(st.st_size);ptr=mmap(nullptr,bytes,PROT_READ,MAP_PRIVATE,fd,0);
        check(ptr!=MAP_FAILED,"mmap failed");
        const auto* p=static_cast<const unsigned char*>(ptr);
        uint32_t fmt[3];std::memcpy(fmt,p,12);std::memcpy(&n,p+12,8);U nc;
        std::memcpy(&nc,p+20,8);std::memcpy(&e,p+28,8);
        check(fmt[0]==0 && fmt[1]==0 && fmt[2]==2,"unsupported CSR format");
        check(n==nc && n<=U(std::numeric_limits<int>::max()),"non-square/oversized graph");
        check(e<=std::numeric_limits<uint32_t>::max(),"row offset overflow");
        check(36+4*(n+1)+8*e==bytes,"CSR length mismatch");
        row=reinterpret_cast<const uint32_t*>(p+36);
        col=reinterpret_cast<const uint32_t*>(p+36+4*(n+1));
        check(row[0]==0 && row[n]==e,"CSR endpoints invalid");
        for(U i=0;i<n;i++)check(row[i]<=row[i+1] && row[i+1]<=e,"CSR row nonmonotone");
        for(U i=0;i<e;i++)check(col[i]<n,"CSR column invalid");
    }
    ~Graph(){if(ptr!=MAP_FAILED)munmap(ptr,bytes);if(fd>=0)close(fd);}
    Graph(const Graph&)=delete;Graph& operator=(const Graph&)=delete;
};
struct Stats {
    U edge_occurrences=0,unique_tile_sources=0,duplicate_occurrences=0;
    U duplicate_sources=0,eligible_source_groups=0,eligible_sources=0,eligible_edges=0;
    U saved_adds_per_feature=0,active_tiles=0,max_tile_edges=0,max_tile_sources=0;
    U group_records=0,rank_source_hist[17]{};
    double scan_ms=0,sort_ms=0;
    U perm_fnv=1469598103934665603ULL;
};
static Stats analyze(const Graph& g,const std::string& order) {
    std::vector<int> perm(g.n);std::iota(perm.begin(),perm.end(),0);
    Stats s;
    auto now=[](){return std::chrono::steady_clock::now();};
    auto ts=now();
    if(order=="degree") {
        std::sort(perm.begin(),perm.end(),[&](int a,int b){return g.row[a+1]-g.row[a]<g.row[b+1]-g.row[b];});
    } else check(order=="identity","unknown order");
    s.sort_ms=std::chrono::duration<double,std::milli>(now()-ts).count();
    for(int x:perm)s.perm_fnv=(s.perm_fnv^U(x))*1099511628211ULL;
    ts=now();
    std::unordered_map<uint32_t,uint32_t> masks;
    std::array<uint32_t,65536> mask_count{};
    std::vector<uint16_t> touched;
    for(U start=0;start<g.n;start+=16) {
        masks.clear();touched.clear();
        U tile_edges=0;
        for(U pos=start;pos<std::min(g.n,start+16);pos++) {
            int d=perm[pos];uint32_t bit=1u<<(pos-start);
            for(U x=g.row[d];x<g.row[d+1];x++) {
                auto [it,inserted]=masks.try_emplace(g.col[x],0);
                if(it->second & bit) {
                    it->second|=0x10000u;
                    s.duplicate_occurrences++;
                }
                it->second|=bit;
                tile_edges++;
            }
        }
        s.edge_occurrences+=tile_edges;s.unique_tile_sources+=masks.size();
        s.max_tile_edges=std::max(s.max_tile_edges,tile_edges);
        s.max_tile_sources=std::max<U>(s.max_tile_sources,masks.size());
        if(tile_edges)s.active_tiles++;
        for(const auto& [source,mask_with_dup]:masks) {
            if(mask_with_dup&0x10000u) {s.duplicate_sources++;continue;}
            uint16_t mask=uint16_t(mask_with_dup);
            if(mask_count[mask]++==0)touched.push_back(mask);
        }
        for(uint16_t mask:touched) {
            U n=mask_count[mask],k=__builtin_popcount((unsigned)mask);
            s.rank_source_hist[k]+=n;
            s.group_records++;
            if(n>=2 && k>=2) {
                s.eligible_source_groups++;
                s.eligible_sources+=n;s.eligible_edges+=n*k;
                s.saved_adds_per_feature+=(n-1)*(k-1);
            }
            mask_count[mask]=0;
        }
    }
    s.scan_ms=std::chrono::duration<double,std::milli>(now()-ts).count();
    check(s.edge_occurrences==g.e,"edge coverage mismatch");
    check(s.saved_adds_per_feature<=g.e,"savings exceeds edges");
    return s;
}
static void emit(std::ostream& o,const Graph& g,const std::string& path,const std::string& label,
                 const std::string& order,const Stats& s) {
    o<<"{\"schema\":\"consumer-mask-c4-v1\",\"graph\":\""<<label<<"\",\"csr\":\""<<path
     <<"\",\"order\":\""<<order<<"\",\"N\":"<<g.n<<",\"E\":"<<g.e
     <<",\"perm_fnv64\":"<<s.perm_fnv<<",\"TR\":16,\"D\":128"
     <<",\"edge_occurrences\":"<<s.edge_occurrences
     <<",\"unique_tile_sources\":"<<s.unique_tile_sources
     <<",\"duplicate_occurrences\":"<<s.duplicate_occurrences
     <<",\"duplicate_sources_excluded\":"<<s.duplicate_sources
     <<",\"eligible_groups\":"<<s.eligible_source_groups
     <<",\"eligible_sources\":"<<s.eligible_sources
     <<",\"eligible_edges\":"<<s.eligible_edges
     <<",\"saved_adds_per_feature\":"<<s.saved_adds_per_feature
     <<",\"saved_avx_float_adds_D128\":"<<s.saved_adds_per_feature*128
     <<",\"saving_fraction\":"<<(g.e?double(s.saved_adds_per_feature)/g.e:0.0)
     <<",\"plan_naive_6byte_source_plus_4byte_group_bytes\":"<<(s.unique_tile_sources*6+s.group_records*4)
     <<",\"plan_8byte_source_plus_4byte_group_bytes\":"<<(s.unique_tile_sources*8+s.group_records*4)
     <<",\"group_records\":"<<s.group_records
     <<",\"active_tiles\":"<<s.active_tiles
     <<",\"max_tile_edges\":"<<s.max_tile_edges
     <<",\"max_tile_sources\":"<<s.max_tile_sources
     <<",\"sort_ms\":"<<s.sort_ms<<",\"scan_ms\":"<<s.scan_ms
     <<",\"mask_popcount_source_hist\":[";
    for(int k=0;k<=16;k++){if(k)o<<',';o<<s.rank_source_hist[k];}
    o<<"]}"<<'\n';
}
int main(int argc,char**argv) {
    try {
        std::string path,label="graph",out,order="both";
        for(int i=1;i<argc;i++) {
            std::string k=argv[i];check(i+1<argc,"missing option value");std::string v=argv[++i];
            if(k=="--csr")path=v;else if(k=="--graph")label=v;
            else if(k=="--out")out=v;else if(k=="--order")order=v;
            else throw std::runtime_error("unknown option: "+k);
        }
        check(!path.empty()&&!out.empty(),"--csr and --out required");
        Graph g(path);
        std::ofstream f(out);check(bool(f),"output open failed");
        if(order=="both") {
            emit(f,g,path,label,"identity",analyze(g,"identity"));
            emit(f,g,path,label,"degree",analyze(g,"degree"));
        } else emit(f,g,path,label,order,analyze(g,order));
        check(bool(f),"output write failed");
        return 0;
    } catch(const std::exception& e) {std::cerr<<"ERROR: "<<e.what()<<'\n';return 2;}
}
