#!/usr/bin/env python3
"""Insert experimental hooks after the untouched original comparisons."""
import hashlib,json,pathlib,sys
root=pathlib.Path(sys.argv[2])
out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
experiment=out
original=(root/'original/gcn_e2e_v3.cpp').read_bytes();src=original.decode()
for name in ['paper_methods_kernels.hpp','paper_methods_runtime.hpp','projection_window_kernels.hpp','projection_window_pmu.hpp']:
 (out/name).write_bytes((root/'src'/name).read_bytes())
for name in ['residual_broad.hpp']:
 assert (experiment/name).is_file(),name
marker='static void benchmark(const char *dir, const char *name) {'
hook='        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);'
include='#include "residual_broad.hpp"\n\n'
call='''        residual_broad::run(csr,perm,H0_f32,W1,W2,Wv1,Wv2,H0_bf16,
            H1_f32,H1_bf16,H2_bf16,H2_f32_for_cmp,A_mkl,descr,Z,H1_mkl,H2_mkl,name);

'''
assert src.count(marker)==src.count(hook)==1
generated=src.replace(marker,include+marker).replace(hook,call+hook)
inserts=[]
def insert_before(anchor, text):
 global generated
 assert generated.count(anchor)==1,anchor
 generated=generated.replace(anchor,text+anchor);inserts.append(text)
def checkpoint(phase):
 return '    printf("METHOD_SETUP graph=%s phase='+phase+' ms=%.12g\\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();\n'
insert_before('    csr_t csr=load_csrbin(path);','    double projection_setup_tick=omp_get_wtime();\n')
insert_before('    int N=csr.N; uint32_t nnz=csr.nnz;',checkpoint('csr_file_load'))
insert_before('    srand(12345);','    projection_setup_tick=omp_get_wtime();\n')
insert_before('    for(int i=0;i<K_DIM*K_DIM;i++){W1[i]',checkpoint('tensor_allocation'))
insert_before('    for(size_t i=0;i<(size_t)N*K_DIM;i++) H0_f32[i]',checkpoint('weight_generation'))
insert_before('    int *perm=make_degree_perm(csr.indptr,N);',checkpoint('feature_generation'))
insert_before('    uint16_t *Wv1=',checkpoint('degree_sort'))
insert_before('    make_W_vnni(W1,Wv1);',checkpoint('packed_weight_allocation'))
insert_before('    /* Pre-convert H0 (outside timing) */',checkpoint('weight_vnni_packing'))
insert_before('    convert_f32_to_bf16(H0_f32,H0_bf16,(size_t)N*K_DIM);',checkpoint('input_bf16_allocation'))
insert_before('    /* Buffers */',checkpoint('input_bf16_conversion'))
insert_before('    printf("=== TFS V3 Hybrid Pipeline ===\\n");',checkpoint('tfs_output_allocation'))
insert_before('    MKL_INT *rs=', '    projection_setup_tick=omp_get_wtime();\n')
insert_before('    for(int i=0;i<N;i++){rs[i]',checkpoint('mkl_index_allocation'))
insert_before('    sparse_matrix_t A_mkl;',checkpoint('mkl_index_conversion'))
insert_before('    if(st!=SPARSE_STATUS_SUCCESS){',checkpoint('mkl_csr_create'))
insert_before('    mkl_sparse_set_mm_hint(A_mkl,', '    projection_setup_tick=omp_get_wtime();\n')
insert_before('    {\n        float *Z=',checkpoint('mkl_hint_and_optimize'))
insert_before('        /* Warmup */\n        mkl_spmm_gemm(A_mkl,', '        printf("METHOD_SETUP graph=%s phase=mkl_output_allocation ms=%.12g\\n",name,(omp_get_wtime()-projection_setup_tick)*1000);\n')
recovered=generated.replace(include,'').replace(call,'')
for addition in inserts:recovered=recovered.replace(addition,'')
assert recovered.encode()==original
(out/'projection_methods.cpp').write_bytes(generated.encode())
manifest=dict(original_source_sha256=hashlib.sha256(original).hexdigest(),paper_source_recoverable_byte_for_byte=True,
 frozen_candidate_sha256=hashlib.sha256((root/'src/paper_methods_kernels.hpp').read_bytes()).hexdigest(),
 independent_variables=['residual correction period/components'],controls=['original source unchanged','original MKL unchanged','Original DegreeSort/TR16/R64 unchanged','BF16 high-word truncation unchanged','default NUMA unchanged'],
 extra_gate='Frozen accurate gates; reject candidate before timing',
 setup_timer_insertions=len(inserts),original_hot_loops_unchanged=True)
(out/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest))
