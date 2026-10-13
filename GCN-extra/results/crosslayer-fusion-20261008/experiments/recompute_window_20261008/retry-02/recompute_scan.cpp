#include <algorithm>
#include <cstdint>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>

using U=uint64_t;
void need(bool x,const std::string& s){if(!x)throw std::runtime_error(s);}
std::string q(const std::string& s){std::string r="\"";for(char c:s){if(c=='"'||c=='\\')r+='\\';r+=c;}return r+'"';}
struct Map{int fd=-1;size_t bytes=0;void* data=nullptr;const uint32_t* row=nullptr;const uint32_t* col=nullptr;U n=0,e=0;
 explicit Map(const std::string& path){fd=open(path.c_str(),O_RDONLY);need(fd>=0,"open");struct stat st{};need(fstat(fd,&st)==0,"stat");bytes=st.st_size;need(bytes>=36,"short");data=mmap(nullptr,bytes,PROT_READ,MAP_PRIVATE,fd,0);need(data!=MAP_FAILED,"mmap");auto*p=(const unsigned char*)data;uint32_t a,b,v;std::memcpy(&a,p,4);std::memcpy(&b,p+4,4);std::memcpy(&v,p+8,4);std::memcpy(&n,p+12,8);U nn;std::memcpy(&nn,p+20,8);std::memcpy(&e,p+28,8);need(a==0&&b==0&&v==2&&n==nn,"header");need(36+4*(n+1)+8*e==bytes,"length");need(e<=0xffffffffULL&&n<=0xffffffffULL,"u32");row=(const uint32_t*)(p+36);col=(const uint32_t*)(p+36+4*(n+1));need(row[0]==0&&row[n]==e,"rows");}
 ~Map(){if(data)munmap(data,bytes);if(fd>=0)close(fd);}
};
struct S{uint32_t producer=0,first=0,last=0,uses=0;U degree=0;};
int main(int argc,char**argv){
 try{need(argc==4,"usage scan --csr PATH --graph LABEL --out JSON");std::string csr=argv[2], graph=argv[3], outp=argv[1];
  Map m(csr);std::vector<uint32_t> rowrank(m.n);std::vector<U> degree(m.n);std::vector<uint64_t> keys;keys.reserve(m.e);
  for(U d=0;d<m.n;d++){U a=m.row[d],b=m.row[d+1];int64_t prev=-1;for(U x=a;x<b;x++){uint32_t j=m.col[x];need(j<m.n&&j>=prev,"bad col/sort");prev=j;degree[j]++;keys.push_back((uint64_t(j)<<32)|uint64_t(m.col[b-1]/16+1));}if(b>a)rowrank[d]=m.col[b-1]/16+1;}
  const U n=m.n;const U e=m.e;const uint32_t B=uint32_t((n+15)/16);
  munmap(m.data,m.bytes);close(m.fd);m.data=nullptr;m.fd=-1;
  std::sort(keys.begin(),keys.end());std::vector<S> sources; sources.reserve(n);std::vector<U> events(B+1), hist(1);
  for(size_t i=0;i<keys.size();){uint32_t j=uint32_t(keys[i]>>32),p=uint32_t(keys[i]);uint32_t first=p,last=p,uses=1;i++;while(i<keys.size()&&uint32_t(keys[i]>>32)==j){uint32_t z=uint32_t(keys[i]);if(z!=last){last=z;uses++;}i++;}S s{j/16+1,first,last,uses,degree[j]};sources.push_back(s);if(uses>hist.size()-1)hist.resize(uses+1);hist[uses]++;}
  // Rewalk sorted keys to count one regeneration event for every distinct rank after first.
  for(size_t i=0;i<keys.size();){uint32_t j=uint32_t(keys[i]>>32),first=uint32_t(keys[i]),last=first;i++;while(i<keys.size()&&uint32_t(keys[i]>>32)==j){uint32_t z=uint32_t(keys[i]);if(z!=last){last=z;events[last]++;}i++;}}
  U extraEdges=0,extraRows=0;for(const auto&s:sources){extraRows+=s.uses-1;extraEdges+=U(s.uses-1)*s.degree;}
  std::vector<int64_t> diffBase(B+2),diffCand(B+2);for(const auto&s:sources){if(s.producer<=B){diffBase[s.producer]++;if(s.last<=B)diffBase[s.last]--;diffCand[s.producer]++;if(s.first<=B)diffCand[s.first]--;}}
  U base=0,cand=0,maxBase=0,maxCand=0,maxRank=0,totalGroups=0;for(uint32_t p=1;p<=B;p++){base+=diffBase[p];cand+=diffCand[p];U groups=(events[p]+15)/16;totalGroups+=groups;U cm=cand+groups;if(base>maxBase)maxBase=base;if(cm>maxCand){maxCand=cm;maxRank=p;}}
  std::ofstream o(outp);need(bool(o),"output");o<<"{\"schema\":\"recompute-audit-cpp-v1\",\"status\":\"PASS\",\"graph\":"<<q(graph)<<",\"csr_path\":"<<q(csr)<<",\"n\":"<<n<<",\"e\":"<<e<<",\"batch_rows\":16,\"batches\":"<<B<<",\"source_rows_with_consumer\":"<<sources.size()<<",\"full_h1_bytes\":"<<256*sources.size()<<",\"extra_regeneration_rows\":"<<extraRows<<",\"extra_first_layer_edge_visits\":"<<extraEdges<<",\"extra_edge_visit_ratio\":"<<std::setprecision(12)<<double(extraEdges)/double(e)<<",\"extra_projection_rows\":"<<extraRows<<",\"extra_projection_groups_by_rank\":"<<totalGroups<<",\"baseline_peak_source_rows\":"<<maxBase<<",\"candidate_peak_source_plus_transient_rows\":"<<maxCand<<",\"candidate_peak_rank\":"<<maxRank<<",\"reuse_histogram\":{";
  bool first=true;for(size_t i=1;i<hist.size();i++)if(hist[i]){if(!first)o<<',';first=false;o<<q(std::to_string(i))<<':'<<hist[i];}o<<"},\"limitations\":[\"logical work only\",\"no DRAM/cache/synchronization\",\"optimistic one regeneration per consumer rank\",\"no legal AMX schedule proof\"]}\n";return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
}
