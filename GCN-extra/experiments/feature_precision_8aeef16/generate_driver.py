#!/usr/bin/env python3
"""Make an isolated driver; recover the original source exactly."""
import hashlib,json,pathlib,sys
study=pathlib.Path(sys.argv[1]);source=study/'source_snapshot'
original=(source/'frozen_source/gcn_e2e_v3.cpp').read_bytes();text=original.decode()
marker='static void benchmark(const char *dir, const char *name) {'
hook='        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);'
include='#include "feature_precision_runtime.hpp"\n\n'
call='''        gcn_extra_feature_precision::run_methods(csr,perm,H0_f32,W1,W2,Wv1,Wv2,H0_bf16,
            H1_f32,H1_bf16,H2_bf16,H2_f32_for_cmp,A_mkl,descr,Z,H1_mkl,H2_mkl,name);

'''
assert text.count(marker)==text.count(hook)==1
generated=text.replace(marker,include+marker).replace(hook,call+hook)
assert generated.replace(include,'').replace(call,'').encode()==original
(source/'feature_precision_driver.cpp').write_bytes(generated.encode())
manifest={'original_source_sha256':hashlib.sha256(original).hexdigest(),'original_recoverable_byte_for_byte':True,
 'frozen_kernel_sha256':hashlib.sha256((source/'frozen_source/paper_methods_kernels.hpp').read_bytes()).hexdigest(),
 'original_TFS_MKL_functions_and_timing_unchanged':True,'hook_after_original_timings':True,
 'fp32_changes':['H source pointer type','source load from float without BF16 expand/shift','native layer2 reads FP32 activation; bypass global conversion'],
 'fp32_input_preparation':'Exact parallel-static copy from source H0 outside prepared timings; avoids source serial first-touch versus BF16 parallel first-touch confound',
 'prefetch_control':'unchanged four instructions at offsets 0,64,128,192; no extra prefetch lines for FP32',
 'fixed_commit':'8aeef1613fdeaeb929379874c4152eccde620f74'}
(study/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest))
