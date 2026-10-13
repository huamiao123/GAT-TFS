#pragma once
#include "paper_methods_runtime.hpp"
#include "projection_window_kernels.hpp"
#include "projection_window_pmu.hpp"
#include "grouping_schedule.hpp"
#include "state_kernels.hpp"
#include "pair_reducer.hpp"
#include <cstdlib>
namespace gcn_extra_state_runtime {
using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
struct Method { std::string name; int kind=2,scope=0,schedule=0; bool nozero=false,direct=false,epi=false; int pair=0; bool accurate=false; };
inline std::vector<Method> candidates(bool full) {
    std::vector<Method> a{{"paper_tfs",0,1},{"b256_fast",1,256},{"bfull_fast",1,0},{"p0_degree",2,0,0}};
    if(full){a.push_back({"state_identity",3,0,0});a.push_back({"nozero_degree",3,0,0,true});a.push_back({"direct_degree",3,0,0,false,true});a.push_back({"pack_degree",3,0,0,true,true});a.push_back({"epi_degree",3,0,0,false,false,true});}
    a.push_back({"pack_epi_degree",3,0,0,true,true,true});
    a.push_back({"pair_i_degree",3,0,0,true,true,true,1});
    a.push_back({"pair_s_degree",3,0,0,true,true,true,2});
    a.push_back({"p0_qguard",2,0,4});
    a.push_back({"pack_epi_qguard",3,0,4,true,true,true});
    a.push_back({"pair_i_qguard",3,0,4,true,true,true,1});
    a.push_back({"pair_s_qguard",3,0,4,true,true,true,2});
    if(full){a.push_back({"p0_m256",2,256,0});a.push_back({"pack_epi_m256",3,256,0,true,true,true});}
    return a;
}
template<bool P,bool B16> void state_layer(const SourceGraphView& g,const Method& m,const uint16_t* h,const uint16_t* w,
    std::conditional_t<B16,uint16_t,float>* out,const int* perm,bool relu,gcn_extra_state::Profile* profile) {
    gcn_extra_state::Method c{m.name,64,m.scope,m.accurate,m.nozero,m.direct,relu};
    if(m.pair==1)gcn_extra_state::kernel<P,B16,gcn_extra_state::PairReducer<false>>(g,h,w,out,perm,c,profile);
    else if(m.pair==2)gcn_extra_state::kernel<P,B16,gcn_extra_state::PairReducer<true>>(g,h,w,out,perm,c,profile);
    else gcn_extra_state::kernel<P,B16>(g,h,w,out,perm,c,profile);
}
template<bool B16> void layer(const SourceGraphView& g,const Method& m,const uint16_t* h,const uint16_t* w,
    std::conditional_t<B16,uint16_t,float>* out,const int* perm,bool relu=false) {
    if(m.kind==0){if constexpr(B16)::tfs_v3_bf16out(g.row.p,g.col.p,h,w,out,perm,g.n,64);else ::tfs_v3_fp32out(g.row.p,g.col.p,h,w,out,perm,g.n,64);}
    else if(m.kind==1){gcn_extra_paper::Method c{m.name,m.scope,m.accurate,false,0,true};gcn_extra_paper::block_kernel<false,B16>(g,h,w,out,perm,c,nullptr);}
    else if(m.kind==2){gcn_extra_projection::Method c{m.name,64,m.scope,m.accurate,true};gcn_extra_projection::kernel<false,B16>(g,h,w,out,perm,c,nullptr);}
    else state_layer<false,B16>(g,m,h,w,out,perm,relu,nullptr);
}
template<bool B16> void forward(const SourceGraphView& g,const Method& m,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,
    float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out,const int* perm) {
    if(m.epi)layer<true>(g,m,h0,w1,h1b,perm,true);
    else {layer<false>(g,m,h0,w1,h1,perm);::relu_f32(h1,size_t(g.n)*DIM);::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);}
    layer<B16>(g,m,h1b,w2,out,perm,false);
}
template<class T> void equal(const char* graph,const Method& m,const char* boundary,const T* got,const T* ref,size_t n){
    bool bits=std::memcmp(got,ref,n*sizeof(T))==0;
    printf("STATE_EQUIV graph=%s method=%s boundary=%s scope=%d accurate=%d bitwise=%d pass=%d\n",graph,m.name.c_str(),boundary,m.scope,int(m.accurate),bits,bits);fflush(stdout);
    if(!bits)throw std::runtime_error(std::string("State equality failed: ")+m.name+"/"+boundary);
}
inline void equivalence(const SourceGraphView& g,const Method& m,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,float* h1,uint16_t* h1b,float* h2f,uint16_t* h2b,const int* perm,const int* degree,const char* graph){
    size_t n=size_t(g.n)*DIM;Method ref=m;ref.kind=1;ref.epi=false;ref.schedule=0;ref.pair=0;
    {std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,m,h0,w1,h1,perm);std::vector<float> v(h1,h1+n);layer<false>(g,ref,h0,w1,h1,degree);equal(graph,m,"kernel_FP32",v.data(),h1,n);}
    {std::fill(h2f,h2f+n,std::numeric_limits<float>::quiet_NaN());forward<false>(g,m,h0,w1,w2,h1,h1b,h2f,perm);std::vector<float> v(h2f,h2f+n);forward<false>(g,ref,h0,w1,w2,h1,h1b,h2f,degree);equal(graph,m,"e2e_FP32",v.data(),h2f,n);}
    {std::fill(h1b,h1b+n,uint16_t(0x7fc0));std::fill(h2b,h2b+n,uint16_t(0x7fc0));forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm);std::vector<uint16_t> v(h2b,h2b+n),u(h1b,h1b+n);forward<true>(g,ref,h0,w1,w2,h1,h1b,h2b,degree);equal(graph,m,"H1_BF16",u.data(),h1b,n);equal(graph,m,"e2e_BF16",v.data(),h2b,n);}
}
// Input-independent graph work counts, not retired hardware events.
inline void structure(const SourceGraphView& g,const int* perm,const char* graph,const char* sched,int scope){
    uint64_t edges=0,hits=0,steps=0,scopes=0,windows=0,rowstores=0,reloads=0,terminal=0,zero_scopes=0,prefetch=0;
    for(int i=0;i<g.n;i+=16){int batch=std::min(16,g.n-i);uint32_t b[16]{},d[16]{},mx=0;
        for(int n=0;n<batch;n++){b[n]=g.row[perm[i+n]];d[n]=g.row[perm[i+n]+1]-b[n];mx=std::max(mx,d[n]);}
        if(!mx)continue;uint32_t span=scope?scope:mx;
        for(uint32_t st=0;st<mx;){uint32_t en=st+std::min(span,mx-st);scopes++;zero_scopes++;
            for(uint32_t ws=st;ws<en;){uint32_t we=ws+std::min(64u,en-ws);windows++;
                for(int n=0;n<batch;n++){uint32_t re=std::min(d[n],en),e=std::min(re,we);if(ws!=st&&ws>=e)continue;rowstores++;if(ws!=st)reloads++;if(we>=re)terminal++;
                    if(e>ws){edges+=e-ws;prefetch+=uint64_t(e-ws-(e==re?1:0))*4;}}
                for(int n=0;n<batch;n+=2){uint32_t a=ws,c=ws,ae=std::min(d[n],we),ce=n+1<batch?std::min(d[n+1],we):ws;
                    while(a<ae||c<ce){steps++;if(a>=ae)c++;else if(c>=ce)a++;else{uint32_t x=g.col[size_t(b[n])+a],y=g.col[size_t(b[n+1])+c];if(x==y){hits++;a++;c++;}else if(x<y)a++;else c++;}}
                }ws=we;
            }st=en;
        }
    }
    if(edges!=g.e||steps!=edges-hits)throw std::runtime_error("Structural work count mismatch");
    printf("STATE_STRUCTURE graph=%s schedule=%s scope=%d edges=%llu shared_heads=%llu merge_steps=%llu share_fraction=%.12g projection_scopes=%llu windows=%llu scalar_adds=%llu direct_decode_rows=%llu shared_decode_rows=%llu amx_tdp_fast=%llu baseline_partial_zero_bytes=%llu baseline_state_load_bytes=%llu baseline_state_store_bytes=%llu direct_saved_terminal_bytes=%llu prefetch_requests=%llu unit=source_level_requests_not_DRAM_or_retired\n",graph,sched,scope,(unsigned long long)edges,(unsigned long long)hits,(unsigned long long)steps,edges?double(hits)/edges:0.,(unsigned long long)scopes,(unsigned long long)windows,(unsigned long long)(edges*128),(unsigned long long)edges,(unsigned long long)(edges-hits),(unsigned long long)(scopes*32),(unsigned long long)(zero_scopes*8192),(unsigned long long)(reloads*512),(unsigned long long)(rowstores*512),(unsigned long long)(terminal*512+scopes*8192),(unsigned long long)prefetch);fflush(stdout);
}
inline void semantic_gate(const char* graph){
    const uint32_t bits[]={0u,0x80000000u,0x3f800000u,0xbf800000u,0x7fc12345u,0xffc12345u,0x7f800000u,0xff800000u,1u,0x80000001u,0x3f807fffu,0x3f808000u,0xbf808000u,0x00010000u,0x80010000u,0x7fa12345u,0xffa12345u,0x3eabcdefu,0xbeabcdefu};
    alignas(64) float input[19],ref[19];alignas(64) uint16_t got[19],expected[19];
    memcpy(input,bits,sizeof(bits));memcpy(ref,bits,sizeof(bits));::relu_f32(ref,19);::convert_f32_to_bf16(ref,expected,19);
    gcn_extra_state::Method m{"semantic",64,0,false,false,false,true};gcn_extra_state::Profile pr;
    gcn_extra_state::scatter_state<false,true>(got,input,19,m,pr,false);
    bool pass=memcmp(got,expected,sizeof(got))==0;printf("STATE_SEMANTIC graph=%s values=19 bitwise=%d pass=%d kind=relu_truncation_signedzero_nan_vector_tail\n",graph,pass,pass);fflush(stdout);if(!pass)throw std::runtime_error("ReLU conversion semantics changed");
}
inline void run_methods(const csr_t& csr,const int* perm,const float* h0,const float* w1,const float* w2,const uint16_t* wv1,const uint16_t* wv2,const uint16_t* h0b,float* h1,uint16_t* h1b,uint16_t* h2b,float* h2f,sparse_matrix_t a,matrix_descr descr,float* z,float* mh1,float* mh2,const char* graph){
    SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*DIM;semantic_gate(graph);
    for(uint64_t e=0;e<g.e;e++)if(csr.values[e]!=1.f)throw std::runtime_error("Nonunit source graph");
    bool smoke=getenv("STATE_SMOKE")&&std::string(getenv("STATE_SMOKE"))=="1";
    bool full=smoke||!getenv("STATE_ABLATIONS")||std::string(getenv("STATE_ABLATIONS"))=="1";
    double ts=omp_get_wtime();auto schedules=gcn_extra_grouping::prepare(g,perm,graph);double setup=(omp_get_wtime()-ts)*1000;
    if(schedules.size()!=5||schedules[0].name!="degree"||schedules[4].name!="qguard"||memcmp(schedules[0].perm.data(),perm,size_t(g.n)*sizeof(int)))throw std::runtime_error("Frozen schedule mismatch");
    printf("STATE_SETUP graph=%s phase=all_five_frozen_schedules ms=%.12g included_in_primary=0\n",graph,setup);
    auto methods=candidates(full);
    printf("STATE_CONFIG graph=%s N=%d E=%u D=128 H=128 F=128 S=64 R=64 TR=16 methods=%zu warmups=1 consecutive=5 alternating=10 numa=default smoke=%d ablations=%d mkl_threads=%d mkl_dynamic=%d final_output=BF16\n",graph,g.n,csr.nnz,methods.size(),smoke,full,mkl_get_max_threads(),mkl_get_dynamic());
    for(auto& m:methods)printf("STATE_PLAN graph=%s method=%s kind=%d scope=%d schedule=%s partial_nozero=%d direct_pack=%d epilogue=%d pair=%d accurate=%d\n",graph,m.name.c_str(),m.kind,m.scope,schedules[m.schedule].name.c_str(),m.nozero,m.direct,m.epi,m.pair,m.accurate);
    {
        std::vector<float> fref(h2f,h2f+n);std::vector<uint16_t>bref(h2b,h2b+n);layer<false>(g,methods[0],h0b,wv1,h1,perm);std::vector<float>kref(h1,h1+n);
        for(auto& m:methods){const int* p=schedules[m.schedule].perm.data();bool exact=m.kind==0;
            std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,m,h0b,wv1,h1,p);gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kref.data(),n,.01,exact);
            forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,p);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,fref.data(),n,.01,exact);
            forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,p);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,bref.data(),n,.02,exact);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);
            if(m.kind>=2)equivalence(g,m,h0b,wv1,wv2,h1,h1b,h2f,h2b,p,perm,graph);
        }
        if(smoke){for(bool accurate:{false,true})for(int scope:{64,256,0})for(int mode=0;mode<7;mode++){
            Method m{"smoke_m"+std::to_string(scope)+"_a"+std::to_string(accurate)+"_v"+std::to_string(mode),3,scope,0};m.accurate=accurate;m.nozero=mode!=1&&mode!=3;m.direct=mode!=0&&mode!=3;m.epi=mode>=3;m.pair=mode>=5?mode-4:0;
            equivalence(g,m,h0b,wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph);
            {layer<false>(g,m,h0b,wv1,h1,perm);gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kref.data(),n,accurate?.001:.01);forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,perm);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,fref.data(),n,accurate?.001:.01);forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,bref.data(),n,accurate?.01:.02);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);}
        }}
    }
    if(smoke){std::vector<uint16_t> sh(n);const float vals[]={1.f,-1.f,256.f,-256.f,.00390625f,-.00390625f,0.f};
        for(size_t i=0;i<n;i++)sh[i]=::f32_to_bf16(vals[(i/128+i%128)%7]);
        for(bool ac:{false,true})for(int sc:{64,256,0})for(int pair:{0,1,2}){Method m{"stress_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_p"+std::to_string(pair),3,sc,0,true,true,true,pair,ac};equivalence(g,m,sh.data(),wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph);}
    }
    printf("STATE_GATE_COMPLETE graph=%s pass=1\n",graph);fflush(stdout);
    auto run=[&](size_t k){if(k==methods.size()){::mkl_spmm_gemm(a,descr,h0,w1,z,mh1,g.n);::relu_f32(mh1,n);::mkl_spmm_gemm(a,descr,mh1,w2,z,mh2,g.n);}else{auto&m=methods[k];forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,schedules[m.schedule].perm.data());}};
    if(!smoke){
        for(size_t k=0;k<=methods.size();k++){run(k);for(int rep=0;rep<5;rep++){double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("STATE_TIME graph=%s method=%s kind=consecutive repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,k,ms);fflush(stdout);}}
        for(size_t k=0;k<=methods.size();k++)run(k);
        for(int rep=0;rep<10;rep++)for(size_t pos=0;pos<=methods.size();pos++){size_t k=rep%2?methods.size()-pos:pos;double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("STATE_TIME graph=%s method=%s kind=alternating repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,pos,ms);fflush(stdout);}
    }
    for(auto& m:methods){if(m.kind<2)continue;const int*p=schedules[m.schedule].perm.data();
        for(int li=0;li<2;li++){
            layer<false>(g,m,h0b,wv1,h1,p);std::vector<float>rf;std::vector<uint16_t>rb;
            if(li==0)rf.assign(h1,h1+n);else{::relu_f32(h1,n);::convert_f32_to_bf16(h1,h1b,n);layer<true>(g,m,h1b,wv2,h2b,p);rb.assign(h2b,h2b+n);}
            if(m.kind==3){gcn_extra_state::Profile pr;
                if(li==0)state_layer<true,false>(g,m,h0b,wv1,h1,p,false,&pr);else state_layer<true,true>(g,m,h1b,wv2,h2b,p,false,&pr);
                const auto& c=pr.counters;const auto& q=pr.logical;
                printf("STATE_COUNTERS graph=%s method=%s layer=%d unit=source_level_not_DRAM source_visits=%llu feature_adds=%llu decode_rows=%llu shared_heads=%llu projection_scopes=%llu amx_tdp=%llu partial_zero_bytes=%llu partial_load_bytes=%llu partial_store_bytes=%llu packing_load_bytes=%llu direct_rows=%llu packed_bytes=%llu prefetch_requests=%llu csr_reads=%llu comparisons=%llu\n",graph,m.name.c_str(),li+1,(unsigned long long)c.source_visits,(unsigned long long)c.feature_adds,(unsigned long long)q.decoded_source_rows,(unsigned long long)q.pair_shared_sources,(unsigned long long)c.projection_scopes,(unsigned long long)c.amx_dpbf16ps,(unsigned long long)c.partial_zero_bytes,(unsigned long long)c.window_state_load_bytes,(unsigned long long)c.window_state_store_bytes,(unsigned long long)q.packing_partial_load_bytes,(unsigned long long)q.direct_pack_rows,(unsigned long long)c.partial_packed_bytes,(unsigned long long)c.csr_prefetch_instructions,(unsigned long long)q.csr_index_reads,(unsigned long long)q.pair_comparisons);
                bool work=c.source_visits==g.e&&c.feature_adds==g.e*128&&c.amx_dpbf16ps==c.projection_scopes*32&&q.decoded_source_rows==g.e-(m.pair==2?q.pair_shared_sources:0);
                printf("STATE_WORK_GATE graph=%s method=%s layer=%d pass=%d\n",graph,m.name.c_str(),li+1,work);if(!work)throw std::runtime_error("Work invariant failed");
                bool bits=li==0?memcmp(h1,rf.data(),n*4)==0:memcmp(h2b,rb.data(),n*2)==0;
                printf("STATE_PROFILE_GATE graph=%s method=%s layer=%d bitwise=%d pass=%d\n",graph,m.name.c_str(),li+1,bits,bits);if(!bits)throw std::runtime_error("Profile changes output");
            }
        }
    }
    for(int s:{0,4})for(int scope:{64,256,0})structure(g,schedules[s].perm.data(),graph,schedules[s].name.c_str(),scope);
    if(!smoke&&getenv("STATE_PMU")&&std::string(getenv("STATE_PMU"))=="1")for(size_t k=0;k<methods.size();k++){
        auto&m=methods[k];if(m.name!="p0_degree"&&m.name!="pack_epi_degree"&&m.name!="pair_i_degree"&&m.name!="pair_s_degree")continue;
        for(int rep=0;rep<3;rep++){run(k);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;auto result=region.stop();gcn_extra_projection_pmu::print(result,graph,m.name.c_str(),rep);printf("STATE_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g\n",graph,m.name.c_str(),rep,ms);}
    }
    rusage ru{};getrusage(RUSAGE_SELF,&ru);printf("STATE_COMPLETE graph=%s pass=1 methods=%zu smoke=%d max_rss_kib=%ld\n",graph,methods.size(),smoke,ru.ru_maxrss);fflush(stdout);
}
}
