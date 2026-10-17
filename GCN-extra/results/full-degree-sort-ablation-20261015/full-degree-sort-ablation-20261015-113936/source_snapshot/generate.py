#!/usr/bin/env python3
import hashlib,pathlib,sys
root=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
src=(root/'original/gcn_e2e_v3.cpp').read_text()
marker='static void benchmark(const char *dir, const char *name) {'
assert src.count(marker)==1
src=src.replace(marker,'#include "ablation.hpp"\n\n'+marker)
old='    int *perm=make_degree_perm(csr.indptr,N);'
new='    double degree_sort_begin=omp_get_wtime();\n'+old+'\n    double degree_sort_ms=(omp_get_wtime()-degree_sort_begin)*1000;'
assert src.count(old)==1
src=src.replace(old,new)
hook='        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);'
call='''        run_full_sort_ablation(csr,perm,H0_bf16,Wv1,Wv2,H1_f32,H1_bf16,H2_bf16,name,degree_sort_ms);\n'''
assert src.count(hook)==1
src=src.replace(hook,call+hook)
(out/'driver.cpp').write_text(src)
for name in ['ablation.hpp']:(out/name).write_bytes((pathlib.Path(__file__).parent/name).read_bytes())
(out/'paper_methods_kernels.hpp').write_bytes((root/'src/paper_methods_kernels.hpp').read_bytes())
print('original_sha256',hashlib.sha256((root/'original/gcn_e2e_v3.cpp').read_bytes()).hexdigest())
