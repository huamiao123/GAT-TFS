#!/usr/bin/env python3
"""Derive an isolated executor from passing state-reuse run05; freeze controls."""
import hashlib
import json
import pathlib
import sys

out = pathlib.Path(sys.argv[1])
frozen = pathlib.Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
deps = ['gcn_e2e_v3.cpp', 'paper_methods_kernels.hpp', 'paper_methods_runtime.hpp',
        'projection_window_kernels.hpp', 'projection_window_pmu.hpp',
        'grouping_schedule.hpp', 'state_kernels.hpp', 'pair_reducer.hpp']
identity = {}
for name in deps:
    blob = (frozen / name).read_bytes()
    (out / name).write_bytes(blob)
    identity[name] = hashlib.sha256(blob).hexdigest()
assert identity['state_kernels.hpp'] == '01a6a46bdcc92a29fb6e2a24f724971854c967c91ee31276b4ee033baa4274a4'
assert identity['paper_methods_kernels.hpp'] == '35d291b886ee0f00e79e0e5d17c6226b2fa1ad64eb715ecbe7bcb1c3f02e7b85'

src = (frozen / 'state_kernels.hpp').read_text()
marker = 'template<bool P, bool B16 = false, class Reducer = ScalarReducer>'
assert src.count(marker) == 1
kernel = src[src.index(marker):src.rindex('} // namespace gcn_extra_state')]
kernel = kernel.replace(marker, 'template<int G, bool P, bool B16 = false, class Reducer = ScalarReducer>')
kernel = kernel.replace('    using namespace gcn_extra_paper;\n',
                        '    static_assert(G == 1 || G == 2 || G == 4);\n    using namespace gcn_extra_paper;\n', 1)
old = 'alignas(64) uint16_t hi[TR * DIM], lo[TR * DIM];'
assert kernel.count(old) == 1
kernel = kernel.replace(old, 'alignas(64) uint16_t hi[G * TR * DIM], lo[G * TR * DIM];')
start = kernel.index('                for(uint32_t start = 0; start < maxdeg;) {')
end = kernel.index('                t = tick<P>(sample);\n                for(int n = 0; n < batch; n++)', start)
kernel = kernel[:start] + '''                for(uint32_t start = 0; start < maxdeg;) {
                    const uint32_t bundle_start = start;
                    int used = 0;
                    // Each original scope keeps its own FP32 sum and BF16 hi/lo.
                    // A bundle changes only when the output panel is saved.
                    for(; used < G && start < maxdeg; ++used) {
                        const uint32_t end = start + std::min(scope, maxdeg - start);
                        Reducer::template run<P>(g, hb, bases, degrees, batch, start, end,
                            uint32_t(method.window), partial,
                            hi + size_t(used) * TR * DIM,
                            lo + size_t(used) * TR * DIM, method, pr, sample);
                        start = end;
                    }
                    for(int obp = 0; obp < NP; obp++) {
                        t = tick<P>(sample);
                        if(bundle_start == 0) {
                            _tile_zero(TC0); _tile_zero(TC1);
                            _tile_zero(TC2); _tile_zero(TC3);
                            detail_tock<P>(pr, AMX_C_ZERO, t, sample);
                        } else {
                            _tile_loadd(TC0, ctile + obp * 64, 512);
                            _tile_loadd(TC1, ctile + obp * 64 + 16, 512);
                            _tile_loadd(TC2, ctile + obp * 64 + 32, 512);
                            _tile_loadd(TC3, ctile + obp * 64 + 48, 512);
                            detail_tock<P>(pr, AMX_C_LOAD, t, sample);
                        }
                        tock<P>(pr, bundle_start == 0 ? REDUCE_ZERO : TILE_LOAD, t, sample);
                        for(int q = 0; q < used; ++q) {
                            project_panel_detailed<P>(hi + size_t(q) * TR * DIM, w, obp, pr, sample);
                            if(method.accurate)
                                project_panel_detailed<P>(lo + size_t(q) * TR * DIM, w, obp, pr, sample);
                            if constexpr(P) {
                                const uint64_t parts = method.accurate ? 2 : 1;
                                pr.counters.projection_panels++;
                                pr.counters.amx_input_loads += parts * KB * 5;
                                pr.counters.amx_dpbf16ps += parts * KB * 4;
                                pr.counters.amx_executed_flops += parts * KB * 4 * uint64_t(2 * 16 * 16 * 32);
                            }
                        }
                        if constexpr(P) {
                            pr.counters.amx_accumulator_loads += bundle_start == 0 ? 0 : 4;
                            pr.counters.amx_accumulator_stores += 4;
                        }
                        t = tick<P>(sample);
                        _tile_stored(TC0, ctile + obp * 64, 512);
                        _tile_stored(TC1, ctile + obp * 64 + 16, 512);
                        _tile_stored(TC2, ctile + obp * 64 + 32, 512);
                        _tile_stored(TC3, ctile + obp * 64 + 48, 512);
                        detail_tock<P>(pr, AMX_C_STORE, t, sample);
                        tock<P>(pr, TILE_STORE, t, sample);
                    }
                }

''' + kernel[end:]
header = '''#pragma once
#include "state_kernels.hpp"
// Isolated derived executor. Frozen reducers, packing and EPI are unchanged.
namespace gcn_extra_bundle {
using namespace gcn_extra_state;
using gcn_extra_state::Method;
using gcn_extra_state::Profile;
''' + kernel + '\n} // namespace gcn_extra_bundle\n'
(out / 'bundle_kernels.hpp').write_text(header)

runtime = (frozen / 'state_runtime.hpp').read_text()
runtime = runtime.replace('#include "state_kernels.hpp"', '#include "state_kernels.hpp"\n#include "bundle_kernels.hpp"')
runtime = runtime.replace('namespace gcn_extra_state_runtime {', 'namespace gcn_extra_bundle_runtime {')
runtime = runtime.replace('bool accurate=false; };', 'bool accurate=false; int bundle=0; };')
cstart = runtime.index('inline std::vector<Method> candidates(bool full) {')
cend = runtime.index('template<bool P,bool B16> void state_layer', cstart)
runtime = runtime[:cstart] + '''inline std::vector<Method> candidates(bool) {
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
''' + runtime[cend:]
dispatch = '''    if(m.bundle) {
        if(m.bundle==1)gcn_extra_bundle::kernel<1,P,B16>(g,h,w,out,perm,c,profile);
        else if(m.bundle==2)gcn_extra_bundle::kernel<2,P,B16>(g,h,w,out,perm,c,profile);
        else if(m.bundle==4)gcn_extra_bundle::kernel<4,P,B16>(g,h,w,out,perm,c,profile);
        else throw std::runtime_error("invalid bundle");
        return;
    }
'''
needle = '    if(m.pair==1)gcn_extra_state::kernel'
assert runtime.count(needle) == 1
runtime = runtime.replace(needle, dispatch + needle)
runtime = runtime.replace('n,.01,exact);', 'n,m.accurate?.001:.01,exact);')
runtime = runtime.replace('n,.02,exact);', 'n,m.accurate?.01:.02,exact);')
runtime = runtime.replace('pair=%d accurate=%d\\n', 'pair=%d accurate=%d bundle=%d\\n')
runtime = runtime.replace('m.pair,m.accurate);', 'm.pair,m.accurate,m.bundle);')

# The existing cancellation test also executes every bundle, without head merging.
old = 'for(bool ac:{false,true})for(int sc:{64,256,0})for(int pair:{0,1,2}){Method m{"stress_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_p"+std::to_string(pair),3,sc,0,true,true,true,pair,ac};'
new = 'for(bool ac:{false,true})for(int sc:{64,256,0})for(int bg:{0,1,2,4}){Method m{"stress_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_g"+std::to_string(bg),3,sc,0,true,true,true,0,ac,bg};'
assert runtime.count(old) == 1
runtime = runtime.replace(old, new)

profile_start = runtime.index('    for(auto& m:methods){if(m.kind<2)continue;')
profile_end = runtime.index('    if(!smoke&&getenv("STATE_PMU")', profile_start)
runtime = runtime[:profile_start] + '''    // Stage wall timing is separate from uninstrumented primary timing.
    if(!smoke)for(auto& m:methods)for(int rep=0;rep<3;rep++) {
        double t0=omp_get_wtime(),t1,t2,t3;
        if(m.epi){layer<true>(g,m,h0b,wv1,h1b,perm,true);t1=t2=t3=omp_get_wtime();}
        else {layer<false>(g,m,h0b,wv1,h1,perm);t1=omp_get_wtime();
              ::relu_f32(h1,n);t2=omp_get_wtime();::convert_f32_to_bf16(h1,h1b,n);t3=omp_get_wtime();}
        layer<true>(g,m,h1b,wv2,h2b,perm,false);double t4=omp_get_wtime();
        printf("BUNDLE_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g convert_ms=%.12g layer2_ms=%.12g total_ms=%.12g epilogue=%s\\n",
           graph,m.name.c_str(),rep,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000,m.epi?"inside_layer1":"separate");
    }
    for(auto& m:methods){if(m.kind!=3)continue;
        uint64_t qs=0,bs=0,tiles=0;
        const uint64_t bg=std::max(1,m.bundle);
        for(int i=0;i<g.n;i+=TR){uint32_t md=0;
            for(int r=0;r<std::min(TR,g.n-i);r++)md=std::max(md,g.row[perm[i+r]+1]-g.row[perm[i+r]]);
            if(md){uint64_t q=m.scope?(uint64_t(md)+m.scope-1)/m.scope:1;
                   qs+=q;bs+=(q+bg-1)/bg;tiles++;}}
        printf("BUNDLE_MODEL graph=%s method=%s M=%d G=%d accurate=%d N=%d E=%llu nonempty_tiles=%llu scopes=%llu bundles=%llu source_bytes=%llu decode_groups=%llu scalar_adds=%llu tdp=%llu C_store_bytes=%llu C_reload_bytes=%llu packed_bundle_capacity_bytes=%llu unit=source_level_not_DRAM\\n",
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
            printf("STATE_WORK_GATE graph=%s method=%s layer=%d pass=%d\\n",graph,m.name.c_str(),li+1,work);
            if(!work)throw std::runtime_error("Bundle work invariant failed");
            const bool bits=memcmp(li==0?h1b:h2b,ref.data(),n*2)==0;
            printf("STATE_PROFILE_GATE graph=%s method=%s layer=%d bitwise=%d pass=%d\\n",graph,m.name.c_str(),li+1,bits,bits);
            if(!bits)throw std::runtime_error("Bundle profile changes output");
            printf("BUNDLE_COUNTERS graph=%s method=%s layer=%d source_visits=%llu feature_adds=%llu decode_rows=%llu scopes=%llu tdp=%llu C_store_tiles=%llu C_reload_tiles=%llu partial_load_bytes=%llu partial_store_bytes=%llu packed_bytes=%llu csr_reads=%llu prefetch_requests=%llu output_bytes=%llu\\n",
               graph,m.name.c_str(),li+1,(unsigned long long)c.source_visits,(unsigned long long)c.feature_adds,
               (unsigned long long)q.decoded_source_rows,(unsigned long long)c.projection_scopes,
               (unsigned long long)c.amx_dpbf16ps,(unsigned long long)c.amx_accumulator_stores,
               (unsigned long long)c.amx_accumulator_loads,(unsigned long long)c.window_state_load_bytes,
               (unsigned long long)c.window_state_store_bytes,(unsigned long long)c.partial_packed_bytes,
               (unsigned long long)q.csr_index_reads,(unsigned long long)c.csr_prefetch_instructions,
               (unsigned long long)c.output_store_bytes);
            for(int ph=0;ph<gcn_extra_paper::PHASES;ph++)
                printf("BUNDLE_PROFILE graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g sampled_tiles=%llu diagnostic_wall_ms=%.12g\\n",
                  graph,m.name.c_str(),li+1,gcn_extra_paper::phase_name[ph],pr.sec[ph]*1000,(unsigned long long)pr.tiles,wall);
            for(int ph=0;ph<gcn_extra_projection::DETAIL_PHASES;ph++)
                printf("BUNDLE_DETAIL graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g events=%llu\\n",
                  graph,m.name.c_str(),li+1,gcn_extra_projection::detail_phase_name[ph],pr.detail_sec[ph]*1000,(unsigned long long)pr.detail_events[ph]);
            for(const auto& th:pr.threads)
                printf("BUNDLE_THREAD graph=%s method=%s layer=%d tid=%d active_ms=%.12g start_ms=%.12g finish_ms=%.12g tiles=%llu edges=%llu\\n",
                  graph,m.name.c_str(),li+1,th.thread_id,th.kernel_active_s*1000,th.start_offset_s*1000,
                  th.finish_offset_s*1000,(unsigned long long)th.completed_tiles,(unsigned long long)th.completed_edges);
            if(pr.threads.size()!=size_t(omp_get_max_threads()))throw std::runtime_error("Unexpected participating team size");
        }
    }
''' + runtime[profile_end:]
runtime = runtime.replace('m.name!="p0_degree"&&m.name!="pack_epi_degree"&&m.name!="pair_i_degree"&&m.name!="pair_s_degree"',
                          'm.name!="pack_epi_m64_fast"&&m.name!="bundle_g4_m64_fast"&&m.name!="pack_epi_full_fast"')
(out / 'bundle_runtime.hpp').write_text(runtime)

# Preserve the original benchmark implementation and source MKL exactly.
original = (out / 'gcn_e2e_v3.cpp').read_text()
include = '#include "bundle_runtime.hpp"\n\n'
call = '''        gcn_extra_bundle_runtime::run_methods(csr,perm,H0_f32,W1,W2,Wv1,Wv2,H0_bf16,
            H1_f32,H1_bf16,H2_bf16,H2_f32_for_cmp,A_mkl,descr,Z,H1_mkl,H2_mkl,name);

'''
anchor1 = 'static void benchmark(const char *dir, const char *name) {'
anchor2 = '        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);'
assert original.count(anchor1) == original.count(anchor2) == 1
generated = original.replace(anchor1, include + anchor1).replace(anchor2, call + anchor2)
assert generated.replace(include, '').replace(call, '') == original
(out / 'bundle_methods.cpp').write_text(generated)
manifest = {'frozen_dependencies': identity, 'source_parent': str(frozen),
            'original_source_recoverable_byte_for_byte': True,
            'changes': ['bundle scratch G', 'scope/panel loop order and C staging counters'],
            'unchanged': ['TR16/R64', 'DegreeSort', 'ScalarReducer', 'S64', 'per-scope FP32 sums',
                          'hi/lo truncation', 'W VNNI packing', 'AMX configuration and per-output TDP order',
                          'EPI integer-mask semantics', 'OpenMP dynamic(1)', 'NUMA default', 'compiler flags']}
(out / 'generation.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps(manifest))
