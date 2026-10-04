#!/usr/bin/env python3
"""Rebase validated neighbor reduction onto the exact paper hybrid harness."""
import hashlib,json,pathlib,sys
root=pathlib.Path(__file__).resolve().parents[1]
out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
original=(root/'original/gcn_e2e_v3.cpp').read_bytes();src=original.decode()
header=(root/'src/paper_methods_kernels.hpp').read_bytes()
(out/'paper_methods_kernels.hpp').write_bytes(header)
marker='static void benchmark(const char *dir, const char *name) {'
hook='        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);'
include='#include "paper_methods_runtime.hpp"\n\n'
call='''        gcn_extra_paper::run_methods(csr,perm,H0_f32,W1,W2,Wv1,Wv2,H0_bf16,
            H1_f32,H1_bf16,H2_bf16,H2_f32_for_cmp,A_mkl,descr,Z,H1_mkl,H2_mkl,name);

'''
assert src.count(marker)==src.count(hook)==1
generated=src.replace(marker,include+marker).replace(hook,call+hook)
assert generated.replace(include,'').replace(call,'').encode()==original
(out/'paper_methods.cpp').write_text(generated)
manifest=dict(original_source_sha256=hashlib.sha256(original).hexdigest(),candidate_source_sha256=hashlib.sha256((root/'src/paper_methods_kernels.hpp').read_bytes()).hexdigest(),paper_source_recoverable_byte_for_byte=True,mathematical_candidate_changed=False,changes=['copy validated frozen kernel byte for byte; obsolete generator dependency removed','source hooks after original MKL and TFS measurements; separate rotating measurements'],output='FP32 layer1, FP32 ReLU, BF16 interlayer, direct BF16 layer2 for all TFS paths',numa_policy=None)
(out/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(manifest))
