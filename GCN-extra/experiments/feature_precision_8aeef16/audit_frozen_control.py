#!/usr/bin/env python3
"""Check the inverse edit, not a hand-written claim of source equivalence."""
import hashlib,json,pathlib,sys
p=pathlib.Path(sys.argv[1]) if len(sys.argv)>1 else pathlib.Path(__file__).parent
original=(p/'frozen_source/paper_methods_kernels.hpp').read_bytes()
clone=(p/'feature_precision_kernels.hpp').read_bytes()
body=clone[clone.index(b'#pragma once'):]
edits=[
 (b'namespace gcn_extra_feature_fp32 {',b'namespace gcn_extra_paper {'),
 (b'void reduce_tile(const SourceGraphView& g,const float* hb,',b'void reduce_tile(const SourceGraphView& g,const uint16_t* hb,'),
 (b'void block_kernel(const SourceGraphView& g,const float* hb,',b'void block_kernel(const SourceGraphView& g,const uint16_t* hb,'),
 (b'const float* src=hb+size_t(j)*DIM;',b'const uint16_t* src=hb+size_t(j)*DIM;'),
 (b'val[f]=_mm512_loadu_ps(src+f*16);',b'val[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(src+f*16))),16));'),
]
for a,b in edits:
 assert body.count(a)==1,(a,body.count(a))
 body=body.replace(a,b)
assert body==original,'Unexpected edit outside source loads/type/namespace'
expected='35d291b886ee0f00e79e0e5d17c6226b2fa1ad64eb715ecbe7bcb1c3f02e7b85'
assert hashlib.sha256(original).hexdigest()==expected,'Frozen BF16 control hash changed'
print(json.dumps(dict(frozen_BF16_sha256=expected,inverse_five_edits_recovers_frozen_bytes=True,
                      unchanged_AMX_projection_packing_schedule_scatter=True),indent=2))
