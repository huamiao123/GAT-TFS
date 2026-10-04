#!/usr/bin/env python3
"""Rebase validated neighbor reduction onto the exact paper hybrid harness."""
import hashlib,json,pathlib,sys
root=pathlib.Path(__file__).resolve().parents[1]
out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
original=(root/'original/gcn_e2e_v3.cpp').read_bytes();src=original.decode()
candidate=(root/'src/source_protocol_kernels.hpp').read_text()
common=candidate[candidate.index('constexpr int DIM=128;'):candidate.index('static void tfs_v3_kernel_nozero(')]
kernels=candidate[candidate.index('template<bool P> inline void project_panel'):candidate.rindex('\n}')]
old='template<bool P> void block_kernel(const SourceGraphView& g,const uint16_t* hb,const uint16_t* w,float* c,const int* perm,'
new='template<bool P,bool B16=false> void block_kernel(const SourceGraphView& g,const uint16_t* hb,const uint16_t* w,std::conditional_t<B16,uint16_t,float>* c,const int* perm,'
assert kernels.count(old)==1;kernels=kernels.replace(old,new)
kernels=kernels.replace('size_t(g.n)*DIM*sizeof(float)','size_t(g.n)*DIM*sizeof(*c)')
kernels=kernels.replace('memcpy(c+size_t(rows[n])*DIM+obp*64+COL,tmp+n*16,64);','scatter_output<B16>(c+size_t(rows[n])*DIM+obp*64+COL,tmp+n*16,16);')
kernels=kernels.replace('memset(c+size_t(rows[n])*DIM,0,DIM*sizeof(float));','memset(c+size_t(rows[n])*DIM,0,DIM*sizeof(*c));')
oldstore='memcpy(c+size_t(rows[n])*DIM,ctile+n*DIM,DIM*sizeof(float));'
assert kernels.count(oldstore)==1
kernels=kernels.replace(oldstore,'scatter_output<B16>(c+size_t(rows[n])*DIM,ctile+n*DIM,DIM);')
scatter='''template<bool B16> inline void scatter_output(std::conditional_t<B16,uint16_t,float>* dst,const float* src,int count) {
    if constexpr(B16) { for(int k=0;k<count;k++)dst[k]=::f32_to_bf16(src[k]); }
    else memcpy(dst,src,size_t(count)*sizeof(float));
}
'''
header='''#pragma once
#include <array>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <type_traits>
namespace gcn_extra_paper {
'''+common+scatter+kernels+'\n}\n'
(out/'paper_methods_kernels.hpp').write_text(header)
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
manifest=dict(original_source_sha256=hashlib.sha256(original).hexdigest(),candidate_source_sha256=hashlib.sha256((root/'src/source_protocol_kernels.hpp').read_bytes()).hexdigest(),paper_source_recoverable_byte_for_byte=True,mathematical_candidate_changed=False,changes=['typed destination output enables direct BF16 final store','FP32 reduction/hi-lo/AMX/CSR scheduling unchanged','remove obsolete H0 reconversion outside every repeat','source hooks after original MKL and TFS measurements; separate rotating measurements'],output='FP32 layer1, FP32 ReLU, BF16 interlayer, direct BF16 layer2 for all TFS paths',numa_policy=None)
(out/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(manifest))
