// A structural lower-bound certificate, not a feature/performance kernel.
// The included source and grouping helper are frozen and must stay unmodified.
#define main frozen_frontier_main
#include "../frontier_structure.cpp"
#undef main
#include <cstddef>
#include <filesystem>
#include <sstream>

namespace frontier_certificate {
using frontier_structure::U;
using frontier_structure::require;
using frontier_structure::quote;
constexpr U SOURCE_BYTES=256;
constexpr uint32_t BATCH_ROWS=16;
struct Record { uint32_t source,destination; uint64_t csr_offset; };
static_assert(sizeof(Record)==16 && offsetof(Record,csr_offset)==8,"certificate layout");
struct Options { std::string csr,out,frontiers,graph; };
struct Result {
    uint32_t p=0;
    U produced=0,active=0,source_rows=0,matched=0,ready=0,completed=0,examined=0;
    std::string file;
};
Options options(int argc,char** argv) {
    Options o;
    for(int i=1;i<argc;i++) {
        std::string arg=argv[i];
        require(i+1<argc,"each option needs a value");
        std::string value=argv[++i];
        if(arg=="--csr"){require(o.csr.empty(),"duplicate --csr");o.csr=value;}
        else if(arg=="--out"){require(o.out.empty(),"duplicate --out");o.out=value;}
        else if(arg=="--frontiers"){require(o.frontiers.empty(),"duplicate --frontiers");o.frontiers=value;}
        else if(arg=="--graph"){require(o.graph.empty(),"duplicate --graph");o.graph=value;}
        else throw std::runtime_error("unknown argument: "+arg);
    }
    require(!o.csr.empty()&&!o.out.empty()&&!o.frontiers.empty()&&!o.graph.empty(),
      "usage: frontier_certificate --csr PATH --out NEWDIR --frontiers commaRanks --graph LABEL");
    return o;
}
std::vector<uint32_t> parse_frontiers(const std::string& text,uint32_t batches) {
    std::vector<uint32_t> out;
    size_t start=0;
    while(start<=text.size()) {
        size_t end=text.find(',',start);if(end==std::string::npos)end=text.size();
        require(end>start,"empty frontier rank");
        U value=0;
        for(size_t i=start;i<end;i++) {
            require(text[i]>='0'&&text[i]<='9',"frontier must be unsigned decimal");
            value=value*10+U(text[i]-'0');
            require(value<=batches,"frontier rank exceeds producer batches");
        }
        out.push_back(uint32_t(value));
        if(end==text.size())break;start=end+1;
    }
    std::sort(out.begin(),out.end());out.erase(std::unique(out.begin(),out.end()),out.end());
    require(!out.empty(),"no frontiers");return out;
}
void write_result(std::ostream& out,const Result& r) {
    const U lower=SOURCE_BYTES*r.matched,upper=SOURCE_BYTES*r.source_rows;
    const U with_ready=upper+SOURCE_BYTES*r.ready;
    out<<"{\"p\":"<<r.p<<",\"produced_limit_exclusive\":"<<r.produced
       <<",\"active_destinations\":"<<r.active<<",\"source_only_rows\":"<<r.source_rows
       <<",\"matching_records\":"<<r.matched<<",\"lower_bound_bytes\":"<<lower
       <<",\"source_only_bytes\":"<<upper<<",\"ready_tail_rows\":"<<r.ready
       <<",\"completed_nonempty_rows\":"<<r.completed
       <<",\"source_only_plus_ready_bytes\":"<<with_ready
       <<",\"unfinished_state_upper_minus_lower_bytes\":"<<(upper-lower)
       <<",\"source_plus_ready_upper_minus_lower_bytes\":"<<(with_ready-lower)
       <<",\"full_optional_matching\":"<<(r.matched==r.source_rows?"true":"false")
       <<",\"certificate_file\":"<<quote(r.file)<<",\"certificate_bytes\":"<<(16*r.matched)
       <<",\"greedy_examined_occurrences\":"<<r.examined<<'}';
}
int run(int argc,char** argv) {
    const auto o=options(argc,argv);
    const auto started=std::chrono::steady_clock::now();
    frontier_structure::Mapping map(o.csr);
    require(map.unsorted_rows==0,"certificate requires every CSR row sorted nondecreasing");
    const auto& g=map.g;const U n=U(g.n);
    const uint32_t batches=uint32_t((n+BATCH_ROWS-1)/BATCH_ROWS);
    const auto frontiers=parse_frontiers(o.frontiers,batches);
    std::vector<uint32_t> rowmax(n,0),lastuse(n,0);
    // Sortedness and every column/offset were fully checked by Mapping.
    // Node-ID producer batches imply prefix release rank == source rank: no holes.
    for(U d=0;d<n;d++)if(g.row[d]<g.row[d+1])
        rowmax[d]=g.col[U(g.row[d+1])-1]/BATCH_ROWS+1;
    // No per-edge events are materialized. One full occurrence scan builds all lifetimes.
    for(U d=0;d<n;d++)for(U x=g.row[d];x<g.row[d+1];x++) {
        const uint32_t j=g.col[x];lastuse[j]=std::max(lastuse[j],rowmax[d]);
    }
    const std::filesystem::path outdir(o.out);
    require(!std::filesystem::exists(outdir),"--out must be a fresh directory");
    require(std::filesystem::create_directory(outdir),"cannot create --out directory (parent must exist)");
    std::vector<Result> results;results.reserve(frontiers.size());
    std::vector<unsigned char> matched(n,0);
    for(uint32_t p:frontiers) {
        Result r;r.p=p;r.produced=std::min<U>(n,U(p)*BATCH_ROWS);
        for(U j=0;j<r.produced;j++)r.source_rows+=lastuse[j]>p;
        for(U d=0;d<n;d++) {
            r.active+=rowmax[d]>p;
            r.completed+=rowmax[d]>0&&rowmax[d]<=p;
        }
        r.ready=r.completed%BATCH_ROWS;
        std::fill(matched.begin(),matched.end(),0);
        r.file="cert_p"+std::to_string(p)+".bin";
        std::ofstream cert(outdir/r.file,std::ios::binary|std::ios::out|std::ios::trunc);
        require(bool(cert),"cannot create certificate");
        for(U d=0;d<n;d++)if(rowmax[d]>p) {
            unsigned capacity=2;
            for(U x=g.row[d];x<g.row[d+1]&&capacity;x++) {
                const uint32_t j=g.col[x];r.examined++;
                if(U(j)>=r.produced)break;
                if(matched[j])continue;
                require(lastuse[j]>p,"selected source has no unfinished consumer");
                matched[j]=1;capacity--;r.matched++;
                const Record rec{j,uint32_t(d),x};
                cert.write(reinterpret_cast<const char*>(&rec),sizeof(rec));
                require(bool(cert),"certificate write failed");
            }
        }
        cert.close();require(bool(cert),"certificate close failed");
        require(r.matched<=r.source_rows,"matching exceeds native source count");
        require(std::filesystem::file_size(outdir/r.file)==16*r.matched,"certificate size mismatch");
        results.push_back(r);
    }
    const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    std::ofstream out(outdir/"summary.json",std::ios::out|std::ios::trunc);
    require(bool(out),"cannot create summary");
    out<<"{\"schema\":\"frontier-certificate-v1\",\"analysis_pass\":true"
       <<",\"graph\":"<<quote(o.graph)<<",\"csr_path\":"<<quote(o.csr)
       <<",\"n\":"<<n<<",\"e\":"<<g.e<<",\"file_bytes\":"<<map.bytes
       <<",\"batch_rows\":16,\"batches\":"<<batches<<",\"rank_base\":1,\"order\":\"nodeid\""
       <<",\"unsorted_rows\":0,\"forced_sources\":0,\"rows_skipped_zero\":"<<map.empty_rows
       <<",\"certificate_record_bytes\":16"
       <<",\"certificate_format\":\"little-endian:uint32 source,uint32 destination,uint64 csr_offset\""
       <<",\"csr_offset_unit\":\"occurrence index, not byte offset\""
       <<",\"source_capacity\":1,\"destination_capacity\":2"
       <<",\"source_bytes\":256,\"partial_bytes\":512,\"ready_bytes\":256"
       <<",\"boundary\":\"post-drain of producer rank p\""
       <<",\"model\":\"fixed node-ID producer batches; whole BF16 source or one ordered FP32 partial per unfinished destination; no recomputation or alternative representation\""
       <<",\"lower_bound_scope\":\"unfinished source/partial state only; excludes ready, producer scratch, intra-drain transient, and runtime metadata\""
       <<",\"ready_policy\":\"nonempty rows complete at last producer rank; each 16 ready rows immediately consumed; final tail shown before final flush; empty rows skipped\""
       <<",\"full_matching_scope\":\"native source_only_rows count; independent record validation certifies lower_bound_bytes without trusting that count\""
       <<",\"gap_warning\":\"an incomplete greedy matching proves only the stated lower bound, not a real optimization opportunity\""
       <<",\"elapsed_seconds\":"<<std::setprecision(9)<<seconds<<",\"frontiers\":[";
    for(size_t i=0;i<results.size();i++){if(i)out<<',';write_result(out,results[i]);}
    out<<"]}\n";out.close();require(bool(out),"summary write failed");
    std::cout<<"{\"analysis_pass\":true,\"frontiers\":"<<results.size()
             <<",\"summary\":"<<quote((outdir/"summary.json").string())<<"}\n";
    return 0;
}
}
int main(int argc,char** argv) {
    try{return frontier_certificate::run(argc,argv);}
    catch(const std::exception& e){std::cerr<<"frontier_certificate: "<<e.what()<<'\n';return 2;}
}
