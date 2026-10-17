#!/usr/bin/env python3
import pathlib,json,hashlib,sys
out,parent=map(pathlib.Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
deps=['gcn_e2e_v3.cpp','paper_methods_kernels.hpp','paper_methods_runtime.hpp','projection_window_kernels.hpp',
      'projection_window_pmu.hpp','grouping_schedule.hpp','state_kernels.hpp','pair_reducer.hpp','pair_plan.hpp','planned_reducer.hpp']
hashes={}
for n in deps:
 b=(parent/n).read_bytes();(out/n).write_bytes(b);hashes[n]=hashlib.sha256(b).hexdigest()
assert hashes['state_kernels.hpp']=='01a6a46bdcc92a29fb6e2a24f724971854c967c91ee31276b4ee033baa4274a4'
s=(parent/'state_kernels.hpp').read_text();marker='template<bool P, bool B16 = false, class Reducer = ScalarReducer>'
k=s[s.index(marker):s.rindex('} // namespace gcn_extra_state')]
k=k.replace(marker,'template<bool P, bool B16 = false, class Reducer = gcn_extra_ordered::Reducer<3>>')
k=k.replace('const Method& method, Profile* result)', 'const Method& method, const Plan& plan, Profile* result)')
k=k.replace('        Profile& pr = profiles[omp_get_thread_num()];','        Profile& pr = profiles[omp_get_thread_num()];\n        Reducer::bind(plan);')
k=k.replace('                if(maxdeg == 0) {','                Reducer::set_tile(i/TR);\n                if(maxdeg == 0) {')
k=k.replace('            add_logical(result->logical, pr.logical);','            add_logical(result->logical, pr.logical);\n            gcn_extra_pair_plan::add_plan_logical(result->plan_logical,pr.plan_logical);\n            add_event(result->event,pr.event);')
(out/'ordered_kernel.hpp').write_text('#pragma once\n#include "ordered_plan.hpp"\nnamespace gcn_extra_ordered {\n'+k+'\n}\n')

r=(parent/'state_runtime.hpp').read_text().replace('namespace gcn_extra_state_runtime {','namespace gcn_extra_ordered_runtime {')
r=r.replace('#include "state_kernels.hpp"','#include "state_kernels.hpp"\n#include "ordered_kernel.hpp"')
r=r.replace('bool accurate=false; };','bool accurate=false; int mode=0; const gcn_extra_ordered::Plan* plan=nullptr; };')
a=r.index('inline std::vector<Method> candidates(bool full) {');b=r.index('template<bool P,bool B16> void state_layer',a)
r=r[:a]+'''inline std::vector<Method> candidates(bool) {
    std::vector<Method> a{{"paper_tfs",0,1},{"p0_degree",2,0,0}};
    const char* names[]={"pack_epi_degree","dynamic_all_share","event_independent","event_share","dynamic_selected_share","exact_only_share"};
    for(int mode=0;mode<6;mode++) {Method m{names[mode],3,0,0,true,true,true};m.mode=mode;a.push_back(m);}
    return a;
}
'''+r[b:]
r=r.replace('gcn_extra_state::Profile* profile','gcn_extra_ordered::Profile* profile')
needle='    if(m.pair==1)gcn_extra_state::kernel'
dispatch='''    if(m.mode==1){gcn_extra_state::kernel<P,B16,gcn_extra_state::PairReducer<true>>(g,h,w,out,perm,c,profile);return;}
    if(m.mode==2){gcn_extra_ordered::kernel<P,B16,gcn_extra_ordered::Reducer<2>>(g,h,w,out,perm,c,*m.plan,profile);return;}
    if(m.mode==3){gcn_extra_ordered::kernel<P,B16,gcn_extra_ordered::Reducer<3>>(g,h,w,out,perm,c,*m.plan,profile);return;}
    if(m.mode==4){gcn_extra_ordered::kernel<P,B16,gcn_extra_ordered::Reducer<4>>(g,h,w,out,perm,c,*m.plan,profile);return;}
    if(m.mode==5){gcn_extra_ordered::kernel<P,B16,gcn_extra_ordered::Reducer<5>>(g,h,w,out,perm,c,*m.plan,profile);return;}
'''
assert r.count(needle)==1;r=r.replace(needle,dispatch+needle)
r=r.replace('gcn_extra_state::Profile pr;','gcn_extra_ordered::Profile pr;')
r=r.replace('auto schedules=gcn_extra_grouping::prepare(g,perm,graph);','std::vector<gcn_extra_grouping::Schedule> schedules(1);schedules[0].name="degree";schedules[0].perm.assign(perm,perm+g.n);')
r=r.replace('if(schedules.size()!=5||schedules[0].name!="degree"||schedules[4].name!="qguard"||memcmp', 'if(schedules.size()!=1||schedules[0].name!="degree"||memcmp')
r=r.replace('all_five_frozen_schedules','degree_schedule_copy')
r=r.replace('    auto methods=candidates(full);','    auto plan=gcn_extra_ordered::build_plan(g,perm,graph);\n    auto methods=candidates(full);for(auto& m:methods)m.plan=&plan;')
r=r.replace('pair=%d accurate=%d\\n','pair=%d accurate=%d mode=%d\\n').replace('m.pair,m.accurate);','m.pair,m.accurate,m.mode);')
r=r.replace('n,.01,exact);','n,m.accurate?.001:.01,exact);').replace('n,.02,exact);','n,m.accurate?.01:.02,exact);')
# Replace legacy variant sweeps with all new modes/scopes/representations.
start=r.index('        if(smoke){for(bool accurate:');end=r.index('    printf("STATE_GATE_COMPLETE',start)
r=r[:start]+'''        if(smoke)for(bool ac:{false,true})for(int sc:{64,256,0})for(int mode=0;mode<6;mode++) {
            Method m{"smoke_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_v"+std::to_string(mode),3,sc,0,true,true,true};
            m.mode=mode;m.accurate=ac;m.plan=&plan;
            equivalence(g,m,h0b,wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph);
            layer<false>(g,m,h0b,wv1,h1,perm);gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kref.data(),n,ac?.001:.01);
            forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,perm);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,fref.data(),n,ac?.001:.01);
            forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,bref.data(),n,ac?.01:.02);
            gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);
        }
    }
    if(smoke){std::vector<uint16_t> sh(n);const float vals[]={1.f,-1.f,256.f,-256.f,.00390625f,-.00390625f,0.f};
        for(size_t i=0;i<n;i++)sh[i]=::f32_to_bf16(vals[(i/128+i%128)%7]);
        for(bool ac:{false,true})for(int sc:{64,256,0})for(int mode=0;mode<6;mode++){
            Method m{"stress_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_v"+std::to_string(mode),3,sc,0,true,true,true};m.mode=mode;m.accurate=ac;m.plan=&plan;
            equivalence(g,m,sh.data(),wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph);
        }
    }
'''+r[end:]
start=r.index('    for(auto& m:methods){if(m.kind<2)continue;');end=r.index('    if(!smoke&&getenv("STATE_PMU")',start)
r=r[:start]+'''    if(!smoke)for(auto& m:methods)for(int rep=0;rep<3;rep++) {
        double t0=omp_get_wtime(),t1,t2,t3;
        if(m.epi){layer<true>(g,m,h0b,wv1,h1b,perm,true);t1=t2=t3=omp_get_wtime();}
        else {layer<false>(g,m,h0b,wv1,h1,perm);t1=omp_get_wtime();::relu_f32(h1,n);t2=omp_get_wtime();::convert_f32_to_bf16(h1,h1b,n);t3=omp_get_wtime();}
        layer<true>(g,m,h1b,wv2,h2b,perm,false);double t4=omp_get_wtime();
        printf("EVENT_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g convert_ms=%.12g layer2_ms=%.12g total_ms=%.12g epilogue=%s\\n",graph,m.name.c_str(),rep,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000,m.epi?"inside_layer1":"separate");
    }
    for(auto& m:methods){if(m.kind!=3)continue;
        uint64_t scopes=0;for(int i=0;i<g.n;i+=16){uint32_t md=0;for(int k=0;k<std::min(16,g.n-i);k++)md=std::max(md,g.row[perm[i+k]+1]-g.row[perm[i+k]]);scopes+=md>0;}
        for(int li=0;li<2;li++){
            forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm);
            std::vector<uint16_t> ref(li==0?h1b:h2b,(li==0?h1b:h2b)+n);
            gcn_extra_ordered::Profile pr;double t=omp_get_wtime();
            if(li==0)state_layer<true,true>(g,m,h0b,wv1,h1b,perm,true,&pr);else state_layer<true,true>(g,m,h1b,wv2,h2b,perm,false,&pr);
            double wall=(omp_get_wtime()-t)*1000;
            uint64_t saved=m.mode==1?plan.all_matches:(m.mode==3||m.mode==4?plan.eligible_matches:(m.mode==5?plan.exact_matches:0));
            bool work=pr.counters.source_visits==g.e&&pr.counters.feature_adds==g.e*128&&pr.counters.amx_dpbf16ps==scopes*32&&pr.logical.decoded_source_rows==g.e-saved;
            bool bits=memcmp(li==0?h1b:h2b,ref.data(),n*2)==0;
            printf("STATE_WORK_GATE graph=%s method=%s layer=%d pass=%d\\n",graph,m.name.c_str(),li+1,work);
            printf("STATE_PROFILE_GATE graph=%s method=%s layer=%d bitwise=%d pass=%d\\n",graph,m.name.c_str(),li+1,bits,bits);
            if(!work||!bits)throw std::runtime_error("Ordered work/profile gate failed");
            printf("EVENT_COUNTERS graph=%s method=%s layer=%d edges=%llu decoded_rows=%llu saved_decode_rows=%llu adds=%llu tdp=%llu csr_reads=%llu prefetch=%llu partial_load_bytes=%llu partial_store_bytes=%llu packed_bytes=%llu head_comparisons=%llu window_entries=%llu selected_windows=%llu run_dispatches=%llu run_bytes=%llu tile_fallback_scopes=%llu\\n",
                graph,m.name.c_str(),li+1,(unsigned long long)pr.counters.source_visits,(unsigned long long)pr.logical.decoded_source_rows,(unsigned long long)saved,(unsigned long long)pr.counters.feature_adds,(unsigned long long)pr.counters.amx_dpbf16ps,(unsigned long long)pr.logical.csr_index_reads,(unsigned long long)pr.counters.csr_prefetch_instructions,(unsigned long long)pr.counters.window_state_load_bytes,(unsigned long long)pr.counters.window_state_store_bytes,(unsigned long long)pr.counters.partial_packed_bytes,(unsigned long long)pr.logical.pair_comparisons,(unsigned long long)pr.event.window_entries,(unsigned long long)pr.event.selected_windows,(unsigned long long)pr.event.run_dispatches,(unsigned long long)pr.event.run_bytes,(unsigned long long)pr.event.tile_fallback_scopes);
            for(int ph=0;ph<gcn_extra_paper::PHASES;ph++)printf("EVENT_PROFILE graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g diagnostic_wall_ms=%.12g\\n",graph,m.name.c_str(),li+1,gcn_extra_paper::phase_name[ph],pr.sec[ph]*1000,wall);
            for(int ph=0;ph<gcn_extra_projection::DETAIL_PHASES;ph++)printf("EVENT_DETAIL graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g events=%llu\\n",graph,m.name.c_str(),li+1,gcn_extra_projection::detail_phase_name[ph],pr.detail_sec[ph]*1000,(unsigned long long)pr.detail_events[ph]);
            for(const auto& th:pr.threads)printf("EVENT_THREAD graph=%s method=%s layer=%d tid=%d active_ms=%.12g finish_ms=%.12g edges=%llu tiles=%llu\\n",graph,m.name.c_str(),li+1,th.thread_id,th.kernel_active_s*1000,th.finish_offset_s*1000,(unsigned long long)th.completed_edges,(unsigned long long)th.completed_tiles);
            if(pr.threads.size()!=size_t(omp_get_max_threads()))throw std::runtime_error("Unexpected team size");
        }
    }
'''+r[end:]
r=r.replace('m.name!="p0_degree"&&m.name!="pack_epi_degree"&&m.name!="pair_i_degree"&&m.name!="pair_s_degree"',
            'm.name!="pack_epi_degree"&&m.name!="event_independent"&&m.name!="event_share"&&m.name!="dynamic_selected_share"')
(out/'ordered_runtime.hpp').write_text(r)
orig=(out/'gcn_e2e_v3.cpp').read_bytes();text=orig.decode()
include='#include "ordered_runtime.hpp"\n\n'
call='''        gcn_extra_ordered_runtime::run_methods(csr,perm,H0_f32,W1,W2,Wv1,Wv2,H0_bf16,
            H1_f32,H1_bf16,H2_bf16,H2_f32_for_cmp,A_mkl,descr,Z,H1_mkl,H2_mkl,name);

'''
early='''    if(getenv("EVENT_STRUCTURE_ONLY")) {
        int* coverage_perm=make_degree_perm(csr.indptr,N);
        gcn_extra_paper::SourceGraphView view{csr.N,csr.nnz,{csr.indptr},{csr.indices}};
        auto coverage_plan=gcn_extra_ordered::build_plan(view,coverage_perm,name);
        free(coverage_perm);free(csr.indptr);free(csr.indices);free(csr.values);return;
    }
'''
anchors=['static void benchmark(const char *dir, const char *name) {','        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);','    srand(12345);']
for x in anchors:assert text.count(x)==1
text=text.replace(anchors[0],include+anchors[0]).replace(anchors[1],call+anchors[1]).replace(anchors[2],early+anchors[2])
assert text.replace(include,'').replace(call,'').replace(early,'').encode()==orig
(out/'ordered_methods.cpp').write_bytes(text.encode())
(out/'generation.json').write_text(json.dumps({'frozen_parent':str(parent),'dependency_sha256':hashes,'original_source_recoverable_byte_for_byte':True,'change':'partial CSR head-merge precompiled to bounded RLE runs; fixed adjacent pairs within original DegreeSort tiles','selection':'matches>=8 AND runs<=8; fixed before results','numa':'default','scope':'128 unit static inference'},indent=2)+'\n')
print('GENERATED_ORDERED',out)
