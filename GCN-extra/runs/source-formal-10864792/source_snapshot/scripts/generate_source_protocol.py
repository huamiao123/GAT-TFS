#!/usr/bin/env python3
"""Insert candidate measurements into the byte-preserved original E2E harness."""
import hashlib,json,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
original=root/'original/gcn_e2e_bench.cpp'
raw=original.read_bytes()
assert hashlib.sha256(raw).hexdigest()=='9e3600ebbed20f9566f7051487c18c951f1c1cae305a6ea308507c71e7261e58'
src=raw.decode('utf-8')
legacy=(root/'src/bench.cpp').read_text(encoding='utf-8')
assert hashlib.sha256((root/'src/bench.cpp').read_bytes()).hexdigest()=='b33c962bac335ff3059e03f8cc735e97932ec08921c322f76148490eb0a9c0c6'

def function(text,name):
    start=text.index('static void '+name+'(')
    brace=text.index('{',start);depth=0
    for end in range(brace,len(text)):
        if text[end]=='{':depth+=1
        elif text[end]=='}':
            depth-=1
            if depth==0:return text[start:end+1]
    raise AssertionError(name)

nozero=function(src,'tfs_v3_kernel')
zero='    memset(C, 0, (size_t)N * K_DIM * sizeof(float));'
assert nozero.count(zero)==1
nozero=nozero.replace('tfs_v3_kernel(','tfs_v3_kernel_nozero(',1).replace(zero,'    // Only the redundant output memset is omitted.')
common=legacy[legacy.index('constexpr int DIM=128;'):legacy.index('struct Graph {')]
kernels=legacy[legacy.index('template<bool P> inline void project_panel'):legacy.index('// Instrumented copy of the original hot loop.')]
assert kernels.count('const Graph&')==2
kernels=kernels.replace('const Graph&','const SourceGraphView&')
view='''struct SourceArrayView {
    const uint32_t* p;
    uint32_t operator[](size_t i) const { return p[i]; }
};
struct SourceGraphView { int n; uint64_t e; SourceArrayView row,col; };
'''
generated='''// Generated from the frozen original and previously validated candidate.
// No CSR copy or new memory-placement policy is introduced.
#include <array>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
namespace gcn_extra_source {
'''+common+view+nozero+'\n'+kernels+'}\n'
with (root/'src/source_protocol_kernels.hpp').open('w',encoding='utf-8',newline='\n') as stream:
    stream.write(generated)
include_marker='static void benchmark(const char *dir, const char *name) {'
assert src.count(include_marker)==1
src=src.replace(include_marker,'#include "source_protocol_runtime.hpp"\n\n'+include_marker)
hook='        mkl_free(Z_mkl); mkl_free(H1_mkl); mkl_free(H2_mkl);'
assert src.count(hook)==1
call='''        gcn_extra_source::run_source_candidates(csr, perm, H0, W1, W2,
            Wv1, Wv2, H0_bf16, H1_tfs, H2_tfs, H1_tfs_bf16,
            A_mkl, descr, Z_mkl, H1_mkl, H2_mkl, name);

'''
src=src.replace(hook,call+hook)
output=root/'src/source_protocol.cpp'
with output.open('w',encoding='utf-8',newline='\n') as stream:
    stream.write(src)
recovered=src.replace('#include "source_protocol_runtime.hpp"\n\n','').replace(call,'')
assert recovered==raw.decode('utf-8'), 'Original benchmark must recover exactly'
manifest={'original_e2e_sha256':hashlib.sha256(raw).hexdigest(),
          'legacy_candidate_sha256':hashlib.sha256((root/'src/bench.cpp').read_bytes()).hexdigest(),
          'original_baseline_recoverable':True,'unchanged_mkl_helper':function(src,'mkl_spmm_gemm')==function(raw.decode(),'mkl_spmm_gemm'),
          'transformations':['insert header before benchmark','insert candidate call after original timings and precision check','Graph view substitutes owning vectors in candidate kernels','original_nozero removes only initial output memset']}
(root/'docs/SOURCE_PROTOCOL_GENERATION_20261004.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
print(json.dumps(manifest))
