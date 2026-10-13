// Structure-only, single-producer-batch abstract execution. No feature values read.
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <omp.h>

namespace gcn_extra_paper {
struct SourceGraphView { int n=0; uint64_t e=0; const uint32_t *row=nullptr,*col=nullptr; };
}
#include "grouping_schedule.hpp" // Frozen helper: deliberately unmodified.

namespace frontier_structure {
using U=uint64_t;
using I=int64_t;
constexpr U SOURCE_BYTES=256,PARTIAL_BYTES=512,READY_BYTES=256;
constexpr uint32_t BATCH=16;
void require(bool b,const std::string& msg) { if(!b)throw std::runtime_error(msg); }
std::string quote(const std::string& s) {
    std::string r="\"";
    for(unsigned char c:s) {
        if(c=='"'||c=='\\'){r+='\\';r+=char(c);}
        else if(c=='\n')r+="\\n";
        else if(c=='\r')r+="\\r";
        else if(c=='\t')r+="\\t";
        else if(c<32){const char* h="0123456789abcdef";r+="\\u00";r+=h[c>>4];r+=h[c&15];}
        else r+=char(c);
    }
    return r+'"';
}
template<class T> void array_json(std::ostream& o,const std::vector<T>& a) {
    o<<'[';for(size_t i=0;i<a.size();i++){if(i)o<<',';o<<+a[i];}o<<']';
}
struct Mapping {
    int fd=-1; size_t bytes=0; void* data=MAP_FAILED;
    gcn_extra_paper::SourceGraphView g;
    U empty_rows=0,no_consumer_sources=0,unsorted_rows=0,adjacent_duplicate_occurrences=0;
    std::vector<unsigned char> used;
    explicit Mapping(const std::string& path) {
        require(sizeof(size_t)==8,"64-bit process required");
        uint32_t endian=1;require(*reinterpret_cast<unsigned char*>(&endian)==1,"little-endian host required");
        fd=open(path.c_str(),O_RDONLY);require(fd>=0,"open CSR failed");
        struct stat st{};require(fstat(fd,&st)==0&&st.st_size>=36,"invalid CSR size");
        bytes=size_t(st.st_size);
        data=mmap(nullptr,bytes,PROT_READ,MAP_PRIVATE,fd,0);require(data!=MAP_FAILED,"mmap failed");
        const auto* p=static_cast<const unsigned char*>(data);
        uint32_t fmt[3];U nr,nc,e;
        std::memcpy(fmt,p,12);std::memcpy(&nr,p+12,8);std::memcpy(&nc,p+20,8);std::memcpy(&e,p+28,8);
        require(fmt[0]==0&&fmt[1]==0&&fmt[2]==2,"unsupported CSR header (expected 0,0,2)");
        require(nr==nc&&nr<=U(std::numeric_limits<int>::max()),"CSR must be square and n <= INT_MAX");
        require(e<=std::numeric_limits<uint32_t>::max(),"uint32 CSR row offsets require E <= UINT32_MAX");
        const U expected=36+4*(nr+1)+8*e;
        require(expected==bytes,"CSR file length mismatch");
        g={int(nr),e,reinterpret_cast<const uint32_t*>(p+36),reinterpret_cast<const uint32_t*>(p+36+4*(nr+1))};
        require(g.row[0]==0&&g.row[nr]==e,"CSR endpoint mismatch");
        used.assign(nr,0);
        for(U d=0;d<nr;d++) {
            require(g.row[d]<=g.row[d+1]&&g.row[d+1]<=e,"CSR row offset out of range/nonmonotone");
            empty_rows+=g.row[d]==g.row[d+1];bool unsorted=false;
            for(U x=g.row[d];x<g.row[d+1];x++) {
                uint32_t j=g.col[x];require(j<nr,"CSR column out of range");used[j]=1;
                if(x>g.row[d]){unsorted|=j<g.col[x-1];adjacent_duplicate_occurrences+=j==g.col[x-1];}
            }
            unsorted_rows+=unsorted;
        }
        no_consumer_sources=std::count(used.begin(),used.end(),0);
    }
    ~Mapping(){if(data!=MAP_FAILED)munmap(data,bytes);if(fd>=0)close(fd);}
    Mapping(const Mapping&)=delete;Mapping& operator=(const Mapping&)=delete;
};
struct State {
    U source_rows=0,partial_rows=0,ready_rows=0;
    U bytes()const{return SOURCE_BYTES*source_rows+PARTIAL_BYTES*partial_rows+READY_BYTES*ready_rows;}
};
void state_json(std::ostream& o,const State& s) {
    o<<"{\"source_rows\":"<<s.source_rows<<",\"partial_rows\":"<<s.partial_rows
     <<",\"ready_rows\":"<<s.ready_rows<<",\"bytes\":"<<s.bytes()<<'}';
}
struct BatchRecord {
    uint32_t p=0;
    State before,produced,drained;
    U edges=0,segments=0,resumes=0,completed=0,groups=0;
};
struct Policy {
    std::string name;uint32_t stride=0;bool row_complete=false,emit=false;
    std::vector<uint32_t> lastuse,first,last,gcount;
    std::vector<U> starts,completions,segments,edges,release;
    std::vector<BatchRecord> records;
    U total_segments=0,total_resumes=0,total_edges=0,nonempty=0;
    U peak_source_rows=0,peak_partial_rows=0,peak_ready_rows=0;
    U peak_before=0,peak_after_produce=0,peak_after_drain=0;
    uint32_t peak_before_batch=0,peak_after_produce_batch=0,peak_after_drain_batch=0;
    U source_row_batch_area=0,partial_row_batch_area=0,ready_row_batch_area=0;
    U full_groups=0,final_tail=0;
    Policy(std::string name_,uint32_t stride_,bool whole_row,U n,uint32_t b,bool em):
      name(std::move(name_)),stride(stride_),row_complete(whole_row),emit(em),lastuse(n,0),
      starts(b+1,0),completions(b+1,0),segments(b+1,0),edges(b+1,0),release(b+1,0) {
        if(emit){first.assign(n,0);last.assign(n,0);gcount.assign(n,0);}
    }
    uint32_t checkpoint(uint32_t r,uint32_t b)const {
        if(!r)return 0;if(!stride)return b;
        return uint32_t(std::min<U>(b,((U(r)+stride-1)/stride)*stride));
    }
    void row_finish(U d,uint32_t f,uint32_t l,uint32_t count) {
        if(!count)return;
        starts[f]++;completions[l]++;nonempty++;total_segments+=count;
        if(emit){first[d]=f;last[d]=l;gcount[d]=count;}
    }
    void finish(const std::vector<uint32_t>& rank,const std::vector<U>& produced_used,uint32_t b) {
        for(U j=0;j<lastuse.size();j++)if(lastuse[j]){
            require(rank[j]<=lastuse[j]&&lastuse[j]<=b,"invalid source lifetime");
            release[lastuse[j]]++;source_row_batch_area+=lastuse[j]-rank[j];
        }
        records.reserve(b);
        State cur;U cum_edges=0,cum_segments=0,cum_resumes=0,cum_complete=0,cum_groups=0;
        for(uint32_t p=1;p<=b;p++) {
            BatchRecord rec;rec.p=p;rec.before=cur;rec.produced=cur;
            rec.produced.source_rows+=produced_used[p];
            require(release[p]<=rec.produced.source_rows,"source release underflow");
            cur.source_rows=rec.produced.source_rows-release[p];
            require(completions[p]<=cur.partial_rows+starts[p],"partial completion underflow");
            cur.partial_rows=cur.partial_rows+starts[p]-completions[p];
            const U ready=cur.ready_rows+completions[p];
            cum_groups+=ready/BATCH;cur.ready_rows=ready%BATCH;rec.drained=cur;
            require(segments[p]>=starts[p]&&segments[p]>=completions[p],"segment accounting mismatch");
            cum_edges+=edges[p];cum_segments+=segments[p];cum_resumes+=segments[p]-starts[p];
            cum_complete+=completions[p];
            rec.edges=cum_edges;rec.segments=cum_segments;rec.resumes=cum_resumes;rec.completed=cum_complete;rec.groups=cum_groups;
            records.push_back(rec);
            if(rec.before.bytes()>peak_before){peak_before=rec.before.bytes();peak_before_batch=p;}
            if(rec.produced.bytes()>peak_after_produce){peak_after_produce=rec.produced.bytes();peak_after_produce_batch=p;}
            if(cur.bytes()>peak_after_drain){peak_after_drain=cur.bytes();peak_after_drain_batch=p;}
            peak_source_rows=std::max(peak_source_rows,rec.produced.source_rows);
            peak_partial_rows=std::max(peak_partial_rows,std::max(rec.before.partial_rows,cur.partial_rows));
            peak_ready_rows=std::max(peak_ready_rows,std::max(rec.before.ready_rows,cur.ready_rows));
            partial_row_batch_area+=cur.partial_rows;
            // Final tail flush is at the same rank B, so contributes no following interval.
            if(p<b)ready_row_batch_area+=cur.ready_rows;
        }
        require(cur.source_rows==0&&cur.partial_rows==0,"final unfinished source/partial state");
        require(cum_segments==total_segments&&cum_complete==nonempty,"final row/segment count mismatch");
        total_resumes=cum_resumes;total_edges=cum_edges;full_groups=cum_groups;final_tail=cur.ready_rows;
    }
};
struct Result {
    std::string order;U fnv=1469598103934665603ULL;
    uint32_t batches=0;
    std::vector<int> permutation;
    std::vector<uint32_t> rank,forced_end,rowmax;
    std::vector<U> forced_curve;
    U forced_peak_rows=0,forced_area=0,eager_segments=0;
    uint32_t forced_peak_batch=0;
    U sum_tile_max_degree=0,sum_tile_max_degree_x16=0,sum_tile_s64_windows=0;
    U sum_tile_degree_spread=0,max_tile_degree=0,nonempty_producer_tiles=0;
    std::vector<Policy> policies;
};
Result analyze(const Mapping& map,const std::string& name,const std::vector<int>& perm,bool emit) {
    const auto& g=map.g;const U n=U(g.n);Result z;z.order=name;z.batches=uint32_t((n+BATCH-1)/BATCH);
    z.permutation=perm;z.rank.resize(n);z.forced_end.resize(n);z.rowmax.resize(n);
    std::vector<unsigned char> seen(n,0);std::vector<U> produced_used(z.batches+1,0);
    for(U i=0;i<n;i++) {
        int j=perm[i];require(j>=0&&U(j)<n&&!seen[j],"invalid order permutation");seen[j]=1;
        z.fnv=(z.fnv^U(j))*1099511628211ULL;
        z.rank[j]=uint32_t(i/BATCH+1);z.forced_end[j]=z.rank[j];
        if(map.used[j])produced_used[z.rank[j]]++;
    }
    for(U i=0;i<n;i+=BATCH) {
        U hi=0,lo=std::numeric_limits<U>::max();
        for(U k=i;k<std::min<U>(i+BATCH,n);k++) {
            U degree=U(g.row[perm[k]+1])-g.row[perm[k]];hi=std::max(hi,degree);lo=std::min(lo,degree);
        }
        z.sum_tile_max_degree+=hi;z.sum_tile_max_degree_x16+=hi*BATCH;z.sum_tile_s64_windows+=(hi+63)/64;
        z.sum_tile_degree_spread+=hi-lo;z.max_tile_degree=std::max(z.max_tile_degree,hi);z.nonempty_producer_tiles+=hi!=0;
    }
    z.policies.reserve(5);
    z.policies.emplace_back("checkpoint_1",1,false,n,z.batches,emit);
    z.policies.emplace_back("checkpoint_4",4,false,n,z.batches,emit);
    z.policies.emplace_back("checkpoint_16",16,false,n,z.batches,emit);
    z.policies.emplace_back("checkpoint_ALL",0,false,n,z.batches,emit);
    z.policies.emplace_back("row_complete",1,true,n,z.batches,emit);
    // Each occurrence is visited once here. Four fixed policies are a constant factor.
    for(U d=0;d<n;d++) {
        uint32_t prefix=0;std::array<uint32_t,4> prev{},first{},counts{};
        for(U x=g.row[d];x<g.row[d+1];x++) {
            const uint32_t j=g.col[x];prefix=std::max(prefix,z.rank[j]);
            z.forced_end[j]=std::max(z.forced_end[j],prefix);
            for(size_t k=0;k<4;k++) {
                auto& p=z.policies[k];const uint32_t c=p.checkpoint(prefix,z.batches);
                p.edges[c]++;p.lastuse[j]=std::max(p.lastuse[j],c);
                if(c!=prev[k]) {p.segments[c]++;counts[k]++;if(!prev[k])first[k]=c;prev[k]=c;}
            }
        }
        z.rowmax[d]=prefix;
        for(size_t k=0;k<4;k++)z.policies[k].row_finish(d,first[k],prev[k],counts[k]);
    }
    // Source-only: one complete ordered row at its final producer's arrival.
    auto& p=z.policies[4];
    for(U d=0;d<n;d++) {
        const uint32_t c=z.rowmax[d];if(!c)continue;
        p.row_finish(d,c,c,1);p.segments[c]++;p.edges[c]+=U(g.row[d+1])-g.row[d];
        for(U x=g.row[d];x<g.row[d+1];x++)p.lastuse[g.col[x]]=std::max(p.lastuse[g.col[x]],c);
    }
    std::vector<I> delta(z.batches+2,0);
    for(U j=0;j<n;j++) {
        require(z.forced_end[j]>=z.rank[j],"forced interval inverted");
        delta[z.rank[j]]++;delta[z.forced_end[j]]--;
        z.forced_area+=z.forced_end[j]-z.rank[j];
    }
    I cur=0;z.forced_curve.resize(z.batches+1,0);
    for(uint32_t b=1;b<=z.batches;b++) {cur+=delta[b];require(cur>=0,"forced curve underflow");z.forced_curve[b]=U(cur);if(U(cur)>z.forced_peak_rows){z.forced_peak_rows=U(cur);z.forced_peak_batch=b;}}
    require(cur==0,"forced curve final state nonzero");
    for(auto& pol:z.policies){pol.finish(z.rank,produced_used,z.batches);require(pol.total_edges==g.e,"logical edge count != E");}
    z.eager_segments=z.policies[0].total_segments;
    require(z.policies[0].lastuse.size()==z.forced_end.size(),"forced/eager size mismatch");
    for(U j=0;j<n;j++)if(map.used[j])require(z.policies[0].lastuse[j]==z.forced_end[j],"forced/eager release mismatch");
    if(name=="nodeid"&&map.unsorted_rows==0)require(z.forced_peak_rows==0,"sorted CSR nodeid must have zero cross-batch holes");
    return z;
}
void batch_json(std::ostream& o,const BatchRecord& r) {
    o<<"{\"p\":"<<r.p<<",\"before_batch\":";state_json(o,r.before);
    o<<",\"after_produce\":";state_json(o,r.produced);o<<",\"after_drain\":";state_json(o,r.drained);
    o<<",\"cumulative_edges_consumed\":"<<r.edges<<",\"cumulative_segments\":"<<r.segments
     <<",\"cumulative_resumes\":"<<r.resumes<<",\"cumulative_completed_rows\":"<<r.completed
     <<",\"full_output_groups\":"<<r.groups<<",\"ready_tail_rows\":"<<r.drained.ready_rows<<'}';
}
void result_json(std::ostream& o,const Result& z,bool emit,std::ostream* csv) {
    o<<"{\"name\":"<<quote(z.order)<<",\"permutation_fnv64\":"<<z.fnv<<",\"producer_batches\":"<<z.batches
     <<",\"pure_forced\":{\"peak_source_rows\":"<<z.forced_peak_rows<<",\"peak_bytes\":"<<SOURCE_BYTES*z.forced_peak_rows
     <<",\"source_row_batch_area\":"<<z.forced_area<<",\"byte_batch_area\":"<<SOURCE_BYTES*z.forced_area
     <<",\"eager_segments\":"<<z.eager_segments<<",\"peak_batch\":"<<z.forced_peak_batch<<'}'
     <<",\"first_layer_work_proxy\":{\"sum_tile_max_degree\":"<<z.sum_tile_max_degree
     <<",\"sum_tile_max_degree_x16\":"<<z.sum_tile_max_degree_x16<<",\"sum_tile_s64_windows\":"<<z.sum_tile_s64_windows
     <<",\"sum_tile_degree_spread\":"<<z.sum_tile_degree_spread<<",\"max_tile_degree\":"<<z.max_tile_degree
     <<",\"nonempty_producer_tiles\":"<<z.nonempty_producer_tiles
     <<",\"actual_kernel_work_claim\":false}";
    if(emit) {
        o<<",\"permutation\":";array_json(o,z.permutation);o<<",\"rank\":";array_json(o,z.rank);
        o<<",\"forced_release_rank\":";array_json(o,z.forced_end);o<<",\"row_completion_rank\":";array_json(o,z.rowmax);
        o<<",\"forced_postdrain_source_rows\":";array_json(o,z.forced_curve);
    }
    o<<",\"methods\":[";bool comma=false;
    for(const auto& p:z.policies) {
        if(comma)o<<',';comma=true;
        o<<"{\"name\":"<<quote(p.name)<<",\"checkpoint_batches\":"<<p.stride<<",\"row_complete\":"<<(p.row_complete?"true":"false")
         <<",\"logical_edges_consumed\":"<<p.total_edges<<",\"segments\":"<<p.total_segments<<",\"resumes\":"<<p.total_resumes
         <<",\"nonempty_rows_completed\":"<<p.nonempty<<",\"partial_stores\":"<<p.total_resumes<<",\"partial_loads\":"<<p.total_resumes
         <<",\"logical_partial_store_bytes\":"<<PARTIAL_BYTES*p.total_resumes<<",\"logical_partial_load_bytes\":"<<PARTIAL_BYTES*p.total_resumes
         <<",\"logical_source_operand_bytes\":"<<SOURCE_BYTES*p.total_edges
         <<",\"full_output_groups\":"<<p.full_groups<<",\"final_tail_rows\":"<<p.final_tail
         <<",\"final_tail_groups\":"<<(p.final_tail?1:0)<<",\"projection_accounted_rows\":"<<BATCH*(p.full_groups+(p.final_tail?1:0))
         <<",\"peak_boundary_source_rows\":"<<p.peak_source_rows<<",\"peak_boundary_partial_rows\":"<<p.peak_partial_rows
         <<",\"peak_boundary_ready_rows\":"<<p.peak_ready_rows<<",\"peak_before_batch_bytes\":"<<p.peak_before
         <<",\"peak_after_produce_bytes\":"<<p.peak_after_produce<<",\"peak_after_drain_bytes\":"<<p.peak_after_drain
         <<",\"peak_before_batch_rank\":"<<p.peak_before_batch<<",\"peak_after_produce_rank\":"<<p.peak_after_produce_batch
         <<",\"peak_after_drain_rank\":"<<p.peak_after_drain_batch
         <<",\"peak_all_boundaries_bytes\":"<<std::max({p.peak_before,p.peak_after_produce,p.peak_after_drain})
         <<",\"source_row_batch_area\":"<<p.source_row_batch_area<<",\"partial_row_batch_area\":"<<p.partial_row_batch_area
         <<",\"ready_row_batch_area\":"<<p.ready_row_batch_area
         <<",\"postdrain_byte_batch_area\":"<<SOURCE_BYTES*p.source_row_batch_area+PARTIAL_BYTES*p.partial_row_batch_area+READY_BYTES*p.ready_row_batch_area
         <<",\"post_final_flush_state\":{\"source_rows\":0,\"partial_rows\":0,\"ready_rows\":0,\"bytes\":0}";
        if(emit) {
            o<<",\"source_last_use_rank\":";array_json(o,p.lastuse);o<<",\"first_consume_rank\":";array_json(o,p.first);
            o<<",\"last_consume_rank\":";array_json(o,p.last);o<<",\"segments_per_row\":";array_json(o,p.gcount);
            o<<",\"batches\":[";for(size_t i=0;i<p.records.size();i++){if(i)o<<',';batch_json(o,p.records[i]);}o<<']';
        }
        o<<'}';
        if(csv)for(const auto& r:p.records) {
            const State* states[]={&r.before,&r.produced,&r.drained};const char* names[]={"before_batch","after_produce","after_drain"};
            for(int i=0;i<3;i++)*csv<<z.order<<','<<p.name<<','<<r.p<<','<<names[i]<<','<<states[i]->source_rows<<','<<states[i]->partial_rows<<','<<states[i]->ready_rows<<','<<states[i]->bytes()<<'\n';
        }
    }
    o<<"]}";
}
std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> r;size_t a=0;
    while(a<=s.size()){size_t b=s.find(',',a);if(b==std::string::npos)b=s.size();r.push_back(s.substr(a,b-a));a=b+1;}return r;
}
int run(int argc,char** argv) {
    std::string csr,out,graph="graph",orders="all",batchout;bool emit=false;
    for(int i=1;i<argc;i++) {
        const std::string a=argv[i];
        if(a=="--emit-plan"){emit=true;continue;}
        if(a=="--help"){std::cout<<"frontier_structure --csr PATH --out JSON [--graph LABEL] [--orders all|nodeid,degree,qguard] [--checkpoints 1,4,16,whole] [--batch-out CSV] [--emit-plan]\n";return 0;}
        require(i+1<argc,"missing option value");const std::string v=argv[++i];
        if(a=="--csr")csr=v;else if(a=="--out")out=v;else if(a=="--graph")graph=v;
        else if(a=="--orders")orders=v;else if(a=="--batch-out")batchout=v;
        else if(a=="--checkpoints")require(v=="1,4,16,whole"||v=="1,4,16,ALL","fixed checkpoint set is 1,4,16,whole");
        else throw std::runtime_error("unknown option: "+a);
    }
    require(!csr.empty()&&!out.empty(),"--csr and --out are required");
    require(csr!=out&&csr!=batchout&&out!=batchout,"input/output paths must differ");
    {std::ifstream exists(out);require(!exists.good(),"--out already exists (refusing overwrite)");}
    if(!batchout.empty()){std::ifstream exists(batchout);require(!exists.good(),"--batch-out already exists");}
    auto names=orders=="all"?std::vector<std::string>{"nodeid","degree","qguard"}:split(orders);
    std::vector<std::string> checked;
    for(const auto& x:names){require(x=="nodeid"||x=="degree"||x=="qguard","unsupported order");require(std::find(checked.begin(),checked.end(),x)==checked.end(),"duplicate order");checked.push_back(x);}
    const auto begin=std::chrono::steady_clock::now();Mapping map(csr);
    require(!emit||map.g.n<=4096,"--emit-plan limited to n <= 4096");
    std::vector<int> nodeid(map.g.n),degree;std::iota(nodeid.begin(),nodeid.end(),0);
    std::vector<gcn_extra_grouping::Schedule> prepared;
    const bool need_degree=std::find(names.begin(),names.end(),"degree")!=names.end()||std::find(names.begin(),names.end(),"qguard")!=names.end();
    if(need_degree) {
        degree=nodeid;
        // Must match frozen native comparator: intentionally NO tie-break.
        std::sort(degree.begin(),degree.end(),[&](int a,int b){return map.g.row[a+1]-map.g.row[a]<map.g.row[b+1]-map.g.row[b];});
        if(!degree.empty())prepared=gcn_extra_grouping::prepare(map.g,degree.data(),graph.c_str());
    }
    std::ofstream output(out,std::ios::out|std::ios::binary);require(bool(output),"cannot create output JSON");
    std::ofstream csv;
    if(!batchout.empty()){csv.open(batchout);require(bool(csv),"cannot create batch CSV");csv<<"order,method,p,phase,source_rows,partial_rows,ready_rows,bytes\n";}
    output<<"{\"schema_version\":\"frontier-structure-v1\",\"graph\":"<<quote(graph)<<",\"csr_path\":"<<quote(csr)
      <<",\"n\":"<<map.g.n<<",\"e\":"<<map.g.e<<",\"file_bytes\":"<<map.bytes<<",\"batch_rows\":16,\"rank_base\":1"
      <<",\"rows_skipped_zero\":"<<map.empty_rows<<",\"no_consumer_sources\":"<<map.no_consumer_sources
      <<",\"unsorted_rows\":"<<map.unsorted_rows<<",\"adjacent_duplicate_occurrences\":"<<map.adjacent_duplicate_occurrences
      <<",\"empty_fastpath\":true,\"source_bytes\":256,\"partial_bytes\":512,\"ready_bytes\":256"
      <<",\"full_hidden_reference_bytes\":"<<SOURCE_BYTES*U(map.g.n)
      <<",\"model\":{\"read_values\":false,\"strict_csr_occurrence_order\":true,\"single_producer_batch\":true,\"ready_arbitrary_rows\":true,\"empty_rows_enter_ready\":false,\"no_consumer_sources_retained\":false,\"intradrain_peak_modeled\":false,\"hardware_cache_or_dram_claim\":false,\"partial_segment_spill_assumed\":true,\"metadata_bytes_in_state\":false,\"amx_scratch_in_state\":false}"
      <<",\"orders\":[";
    for(size_t i=0;i<names.size();i++) {
        const auto& name=names[i];const auto& perm=name=="nodeid"?nodeid:(name=="degree"?degree:(prepared.empty()?nodeid:prepared[4].perm));
        auto z=analyze(map,name,perm,emit);if(i)output<<',';result_json(output,z,emit,csv.is_open()?&csv:nullptr);
        std::cout<<"FRONTIER_ORDER graph="<<graph<<" order="<<name<<" fnv64="<<z.fnv<<" pass=1 forced_peak_rows="<<z.forced_peak_rows<<" eager_segments="<<z.eager_segments<<'\n';
    }
    double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
    output<<"],\"analysis_wall_ms\":"<<std::setprecision(12)<<ms<<",\"analysis_pass\":true}\n";
    output.close();require(bool(output),"failed writing output JSON");
    if(csv.is_open()){csv.close();require(bool(csv),"failed writing batch CSV");}
    std::cout<<"FRONTIER_ANALYSIS graph="<<graph<<" pass=1 output="<<out<<" values_read=0\n";
    return 0;
}
}
int main(int argc,char** argv) {
    try{return frontier_structure::run(argc,argv);}
    catch(const std::exception& e){std::cerr<<"FRONTIER_ERROR "<<e.what()<<'\n';return 2;}
}
