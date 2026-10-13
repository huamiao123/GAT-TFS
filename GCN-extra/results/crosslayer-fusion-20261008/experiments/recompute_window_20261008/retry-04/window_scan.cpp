#define main frozen_recompute_scan_main
#include "recompute_scan.cpp"
#undef main
#include <cstring>
#include <sstream>

struct WindowMetric { int32_t window; uint64_t extra_edges=0, extra_rows=0, projection_groups=0, peak_rows=0, peak_rank=0; };

int main(int argc,char**argv){
 try{
  need(argc==4,"usage window_scan OUT CSR GRAPH");
  const std::string outp=argv[1], csr=argv[2], graph=argv[3];
  Map m(csr); const U n=m.n,e=m.e; const uint32_t B=uint32_t((n+15)/16);
  std::vector<U> degree(n); std::vector<uint64_t> keys; keys.reserve(e);
  for(U d=0;d<n;d++){
   U a=m.row[d],b=m.row[d+1]; int64_t prev=-1;
   for(U x=a;x<b;x++){uint32_t j=m.col[x];need(j<n&&j>=prev,"bad col/sort");prev=j;degree[j]++;keys.push_back((uint64_t(j)<<32)|uint64_t(m.col[b-1]/16+1));}
  }
  munmap(m.data,m.bytes);close(m.fd);m.data=nullptr;m.fd=-1;
  std::sort(keys.begin(),keys.end());
  // -1: regenerate at every distinct consumer rank; 0: retain through all ranks.
  const std::vector<int32_t> windows={-1,1,2,4,8,16,32,64,128,256,512,1024,0};
  std::vector<WindowMetric> metrics; for(int32_t w:windows)metrics.push_back(WindowMetric{w});
  std::vector<std::vector<int64_t>> diff(windows.size(),std::vector<int64_t>(B+2));
  std::vector<std::vector<uint32_t>> events(windows.size(),std::vector<uint32_t>(B+1));
  std::vector<uint32_t> ranks;
  for(size_t i=0;i<keys.size();){
   uint32_t j=uint32_t(keys[i]>>32); ranks.clear(); uint32_t last=UINT32_MAX;
   while(i<keys.size()&&uint32_t(keys[i]>>32)==j){uint32_t z=uint32_t(keys[i]);if(z!=last){ranks.push_back(z);last=z;}i++;}
   const uint32_t producer=j/16+1;
   for(size_t wi=0;wi<windows.size();wi++){
    const int32_t w=windows[wi]; std::vector<uint32_t> starts,ends;
    uint32_t gs=ranks[0],ge=ranks[0],prev=ranks[0];
    for(size_t k=1;k<ranks.size();k++){
     uint32_t z=ranks[k];
     if((w==0)||(w>0&&z-prev<=uint32_t(w))){ge=z;prev=z;}
     else{starts.push_back(gs);ends.push_back(ge);gs=ge=prev=z;}
    }
    starts.push_back(gs);ends.push_back(ge);
    for(size_t g=0;g<starts.size();g++){diff[wi][g?starts[g]:producer]++;diff[wi][ends[g]]--;if(g)events[wi][starts[g]]++;}
    const uint32_t groups=uint32_t(starts.size());
    metrics[wi].extra_rows+=groups-1; metrics[wi].extra_edges+=U(groups-1)*degree[j];
   }
  }
  for(size_t wi=0;wi<windows.size();wi++){
   int64_t live=0; for(uint32_t p=1;p<=B;p++){live+=diff[wi][p];uint64_t groups=(events[wi][p]+15)/16;uint64_t total=uint64_t(live)+groups;metrics[wi].projection_groups+=groups;if(total>metrics[wi].peak_rows){metrics[wi].peak_rows=total;metrics[wi].peak_rank=p;}}
  }
  std::ofstream o(outp);need(bool(o),"output");o<<"{\"schema\":\"recompute-window-v1\",\"status\":\"PASS\",\"graph\":"<<q(graph)<<",\"csr_path\":"<<q(csr)<<",\"n\":"<<n<<",\"e\":"<<e<<",\"batch_rows\":16,\"batches\":"<<B<<",\"windows\":[";
  for(size_t i=0;i<metrics.size();i++){if(i)o<<',';auto&x=metrics[i];o<<"{\"window_batches\":"<<x.window<<",\"extra_first_layer_edge_visits\":"<<x.extra_edges<<",\"extra_edge_visit_ratio\":"<<std::setprecision(12)<<double(x.extra_edges)/double(e)<<",\"extra_regeneration_rows\":"<<x.extra_rows<<",\"extra_projection_groups\":"<<x.projection_groups<<",\"candidate_peak_source_plus_transient_rows\":"<<x.peak_rows<<",\"candidate_peak_rank\":"<<x.peak_rank<<"}";}o<<"]}\n";return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
}
