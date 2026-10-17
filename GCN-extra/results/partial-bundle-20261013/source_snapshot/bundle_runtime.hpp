#pragma once
#include "paper_methods_runtime.hpp"
#include "projection_window_kernels.hpp"
#include "projection_window_pmu.hpp"
#include "grouping_schedule.hpp"
#include "state_kernels.hpp"
#include "bundle_kernels.hpp"
#include "pair_reducer.hpp"
#include <cstdlib>
namespace gcn_extra_bundle_runtime {
using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
struct Method { std::string name; int kind=2,scope=0,schedule=0; bool nozero=false,direct=false,epi=false; int pair=0; bool accurate=false; int bundle=0; };
inline std::vector<Method> candidates(bool) {
    std::vector<Method> a{{"paper_tfs",0,1},{"p0_degree",2,0,0}};
    for(int scope : {64,256,0}) for(bool accurate : {false,true}) {
        const std::string tag = (scope ? "m" + std::to_string(scope) : "full") +
                                std::string(accurate ? "_accurate" : "_fast");
        a.push_back({"pack_epi_"+tag,3,scope,0,true,true,true,0,accurate,0});
        if(scope) for(int g : {1,2,4})
            a.push_back({"bundle_g"+std::to_string(g)+"_"+tag,3,scope,0,true,true,true,0,accurate,g});
        else a.push_back({"bundle_g4_"+tag,3,scope,0,true,true,true,0,accurate,4});
    }
    return a;
}
template<bool P,bool B16> void state_layer(const SourceGraphView& g,const Method& m,const uint16_t* h,const uint16_t* w,
    std::conditional_t<B16,uint16_t,float>* out,const int* perm,bool relu,gcn_extra_state::Profile* profile) {
    gcn_extra_state::Method c{m.name,64,m.scope,m.accurate,m.nozero,m.direct,relu};
    if(m.bundle) {
        if(m.bundle==1)gcn_extra_bundle::kernel<1,P,B16>(g,h,w,out,perm,c,profile);
        else if(m.bundle==2)gcn_extra_bundle::kernel<2,P,B16>(g,h,w,out,perm,c,profile);
        else if(m.bundle==4)gcn_extra_bundle::kernel<4,P,B16>(g,h,w,out,perm,c,profile);
        else throw std::runtime_error("invalid bundle");
        return;
    }
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
__attribute__((noinline)) inline void semantic_runtime_scatter(uint16_t* got,const float* input,int count){
    gcn_extra_state::Method m{"semantic",64,0,false,false,false,true};gcn_extra_state::Profile pr;
    gcn_extra_state::scatter_state<false,true>(got,input,count,m,pr,false);
}
inline void semantic_gate(const char* graph){
    const uint32_t bits[]={0u,0x80000000u,0x3f800000u,0xbf800000u,0x7fc12345u,0xffc12345u,0x7f800000u,0xff800000u,1u,0x80000001u,0x3f807fffu,0x3f808000u,0xbf808000u,0x00010000u,0x80010000u,0x7fa12345u,0xffa12345u,0x3eabcdefu,0xbeabcdefu};
    alignas(64) float input[19],ref[19];alignas(64) uint16_t got[19],expected[19];
    memcpy(input,bits,sizeof(bits));memcpy(ref,bits,sizeof(bits));::relu_f32(ref,19);::convert_f32_to_bf16(ref,expected,19);
    gcn_extra_state::Method m{"semantic",64,0,false,false,false,true};gcn_extra_state::Profile pr;
    semantic_runtime_scatter(got,input,19);
    bool pass=memcmp(got,expected,sizeof(got))==0; if(!pass)for(int i=0;i<19;i++)printf("STATE_SEMANTIC_DIFF graph=%s index=%d input=%08x got=%04x expected=%04x mxcsr=%08x\n",graph,i,bits[i],unsigned(got[i]),unsigned(expected[i]),_mm_getcsr());printf("STATE_SEMANTIC graph=%s values=19 bitwise=%d pass=%d kind=relu_truncation_signedzero_nan_vector_tail\n",graph,pass,pass);fflush(stdout);if(!pass)throw std::runtime_error("ReLU conversion semantics changed");
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
    for(auto& m:methods)printf("STATE_PLAN graph=%s method=%s kind=%d scope=%d schedule=%s partial_nozero=%d direct_pack=%d epilogue=%d pair=%d accurate=%d bundle=%d\n",graph,m.name.c_str(),m.kind,m.scope,schedules[m.schedule].name.c_str(),m.nozero,m.direct,m.epi,m.pair,m.accurate,m.bundle);
    {
        std::vector<float> fref(h2f,h2f+n);std::vector<uint16_t>bref(h2b,h2b+n);layer<false>(g,methods[0],h0b,wv1,h1,perm);std::vector<float>kref(h1,h1+n);
        for(auto& m:methods){const int* p=schedules[m.schedule].perm.data();bool exact=m.kind==0;
            std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,m,h0b,wv1,h1,p);gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kref.data(),n,m.accurate?.001:.01,exact);
            forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,p);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,fref.data(),n,m.accurate?.001:.01,exact);
            forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,p);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,bref.data(),n,m.accurate?.01:.02,exact);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);
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
        for(bool ac:{false,true})for(int sc:{64,256,0})for(int bg:{0,1,2,4}){Method m{"stress_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_g"+std::to_string(bg),3,sc,0,true,true,true,0,ac,bg};equivalence(g,m,sh.data(),wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph);}
    }
    printf("STATE_GATE_COMPLETE graph=%s pass=1\n",graph);fflush(stdout);
    auto run=[&](size_t k){if(k==methods.size()){::mkl_spmm_gemm(a,descr,h0,w1,z,mh1,g.n);::relu_f32(mh1,n);::mkl_spmm_gemm(a,descr,mh1,w2,z,mh2,g.n);}else{auto&m=methods[k];forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,schedules[m.schedule].perm.data());}};
    if(!smoke){
        for(size_t k=0;k<=methods.size();k++){run(k);for(int rep=0;rep<5;rep++){double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("STATE_TIME graph=%s method=%s kind=consecutive repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,k,ms);fflush(stdout);}}
        for(size_t k=0;k<=methods.size();k++)run(k);
        for(int rep=0;rep<10;rep++)for(size_t pos=0;pos<=methods.size();pos++){size_t k=rep%2?methods.size()-pos:pos;double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("STATE_TIME graph=%s method=%s kind=alternating repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,pos,ms);fflush(stdout);}
    }
    // Stage wall timing is separate from uninstrumented primary timing.
    if(!smoke)for(auto& m:methods)for(int rep=0;rep<3;rep++) {
        double t0=omp_get_wtime(),t1,t2,t3;
        if(m.epi){layer<true>(g,m,h0b,wv1,h1b,perm,true);t1=t2=t3=omp_get_wtime();}
        else {layer<false>(g,m,h0b,wv1,h1,perm);t1=omp_get_wtime();
              ::relu_f32(h1,n);t2=omp_get_wtime();::convert_f32_to_bf16(h1,h1b,n);t3=omp_get_wtime();}
        layer<true>(g,m,h1b,wv2,h2b,perm,false);double t4=omp_get_wtime();
        printf("BUNDLE_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g convert_ms=%.12g layer2_ms=%.12g total_ms=%.12g epilogue=%s\n",
           graph,m.name.c_str(),rep,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000,m.epi?"inside_layer1":"separate");
    }
    for(auto& m:methods){if(m.kind!=3)continue;
        uint64_t qs=0,bs=0,tiles=0;
        const uint64_t bg=std::max(1,m.bundle);
        for(int i=0;i<g.n;i+=TR){uint32_t md=0;
            for(int r=0;r<std::min(TR,g.n-i);r++)md=std::max(md,g.row[perm[i+r]+1]-g.row[perm[i+r]]);
            if(md){uint64_t q=m.scope?(uint64_t(md)+m.scope-1)/m.scope:1;
                   qs+=q;bs+=(q+bg-1)/bg;tiles++;}}
        printf("BUNDLE_MODEL graph=%s method=%s M=%d G=%d accurate=%d N=%d E=%llu nonempty_tiles=%llu scopes=%llu bundles=%llu source_bytes=%llu decode_groups=%llu scalar_adds=%llu tdp=%llu C_store_bytes=%llu C_reload_bytes=%llu packed_bundle_capacity_bytes=%llu unit=source_level_not_DRAM\n",
            graph,m.name.c_str(),m.scope,m.bundle,m.accurate,g.n,(unsigned long long)g.e,
            (unsigned long long)tiles,(unsigned long long)qs,(unsigned long long)bs,
            (unsigned long long)(g.e*256),(unsigned long long)(g.e*8),(unsigned long long)(g.e*128),
            (unsigned long long)(qs*32*(m.accurate?2:1)),(unsigned long long)(bs*8192),
            (unsigned long long)((bs-tiles)*8192),(unsigned long long)(bg*4096*(m.accurate?2:1)));
        for(int li=0;li<2;li++){
            forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm);
            std::vector<uint16_t> ref(li==0?h1b:h2b,(li==0?h1b:h2b)+n);
            gcn_extra_state::Profile pr;double t=omp_get_wtime();
            if(li==0)state_layer<true,true>(g,m,h0b,wv1,h1b,perm,true,&pr);
            else state_layer<true,true>(g,m,h1b,wv2,h2b,perm,false,&pr);
            const double wall=(omp_get_wtime()-t)*1000;
            const auto& c=pr.counters;const auto& q=pr.logical;
            const bool work=c.source_visits==g.e&&c.feature_adds==g.e*128&&
                c.projection_scopes==qs&&c.amx_dpbf16ps==qs*32*(m.accurate?2:1)&&
                q.decoded_source_rows==g.e&&c.amx_accumulator_stores==bs*8&&
                c.amx_accumulator_loads==(bs-tiles)*8;
            printf("STATE_WORK_GATE graph=%s method=%s layer=%d pass=%d\n",graph,m.name.c_str(),li+1,work);
            if(!work)throw std::runtime_error("Bundle work invariant failed");
            const bool bits=memcmp(li==0?h1b:h2b,ref.data(),n*2)==0;
            printf("STATE_PROFILE_GATE graph=%s method=%s layer=%d bitwise=%d pass=%d\n",graph,m.name.c_str(),li+1,bits,bits);
            if(!bits)throw std::runtime_error("Bundle profile changes output");
            printf("BUNDLE_COUNTERS graph=%s method=%s layer=%d source_visits=%llu feature_adds=%llu decode_rows=%llu scopes=%llu tdp=%llu C_store_tiles=%llu C_reload_tiles=%llu partial_load_bytes=%llu partial_store_bytes=%llu packed_bytes=%llu csr_reads=%llu prefetch_requests=%llu output_bytes=%llu\n",
               graph,m.name.c_str(),li+1,(unsigned long long)c.source_visits,(unsigned long long)c.feature_adds,
               (unsigned long long)q.decoded_source_rows,(unsigned long long)c.projection_scopes,
               (unsigned long long)c.amx_dpbf16ps,(unsigned long long)c.amx_accumulator_stores,
               (unsigned long long)c.amx_accumulator_loads,(unsigned long long)c.window_state_load_bytes,
               (unsigned long long)c.window_state_store_bytes,(unsigned long long)c.partial_packed_bytes,
               (unsigned long long)q.csr_index_reads,(unsigned long long)c.csr_prefetch_instructions,
               (unsigned long long)c.output_store_bytes);
            for(int ph=0;ph<gcn_extra_paper::PHASES;ph++)
                printf("BUNDLE_PROFILE graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g sampled_tiles=%llu diagnostic_wall_ms=%.12g\n",
                  graph,m.name.c_str(),li+1,gcn_extra_paper::phase_name[ph],pr.sec[ph]*1000,(unsigned long long)pr.tiles,wall);
            for(int ph=0;ph<gcn_extra_projection::DETAIL_PHASES;ph++)
                printf("BUNDLE_DETAIL graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g events=%llu\n",
                  graph,m.name.c_str(),li+1,gcn_extra_projection::detail_phase_name[ph],pr.detail_sec[ph]*1000,(unsigned long long)pr.detail_events[ph]);
            for(const auto& th:pr.threads)
                printf("BUNDLE_THREAD graph=%s method=%s layer=%d tid=%d active_ms=%.12g start_ms=%.12g finish_ms=%.12g tiles=%llu edges=%llu\n",
                  graph,m.name.c_str(),li+1,th.thread_id,th.kernel_active_s*1000,th.start_offset_s*1000,
                  th.finish_offset_s*1000,(unsigned long long)th.completed_tiles,(unsigned long long)th.completed_edges);
            if(pr.threads.size()!=size_t(omp_get_max_threads()))throw std::runtime_error("Unexpected participating team size");
        }
    }
    if(!smoke&&getenv("STATE_PMU")&&std::string(getenv("STATE_PMU"))=="1")for(size_t k=0;k<methods.size();k++){
        auto&m=methods[k];if(m.name!="pack_epi_m64_fast"&&m.name!="bundle_g4_m64_fast"&&m.name!="pack_epi_full_fast")continue;
        for(int rep=0;rep<3;rep++){run(k);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;auto result=region.stop();gcn_extra_projection_pmu::print(result,graph,m.name.c_str(),rep);printf("STATE_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g\n",graph,m.name.c_str(),rep,ms);}
    }
    rusage ru{};getrusage(RUSAGE_SELF,&ru);printf("STATE_COMPLETE graph=%s pass=1 methods=%zu smoke=%d max_rss_kib=%ld\n",graph,methods.size(),smoke,ru.ru_maxrss);fflush(stdout);
}
}
