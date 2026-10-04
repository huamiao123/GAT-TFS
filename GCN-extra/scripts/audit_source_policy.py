#!/usr/bin/env python3
"""Read-only source audit; store hashes, runtime settings and policy matches."""
import hashlib
import json
import pathlib
import re

root=pathlib.Path(__file__).resolve().parents[1]
authority=root.parent.parent/'yx/TFS'
sources=[root/'original'/n for n in ['amx_tfs_v3.cpp','mkl_baseline.cpp','gcn_e2e_bench.cpp','gcn_e2e_v3.cpp']]
sources+=sorted((root/'original/protocol_scripts').glob('*'))
records=[]
for path in sources:
    origin=authority/('scripts' if path.parent.name=='protocol_scripts' else 'code')/path.name
    content=path.read_bytes();text=content.decode('utf-8')
    assert origin.read_bytes()==content, 'Frozen copy differs from authority: '+str(path)
    matches=[]
    declarations=[]
    for number,line in enumerate(text.splitlines(),1):
        if re.search(r'\b(numactl|mbind|set_mempolicy|numa_alloc\w*|numa_bind|numa_run_on_node)\b',line):
            matches.append(dict(line=number,text=line))
        if re.search(r'export\s+(OMP_|MKL_|KMP_)|mkl_set_(dynamic|num_threads)|mkl_sparse_set_mm_hint',line):
            declarations.append(dict(line=number,text=line))
    records.append(dict(path=str(path),authority=str(origin),sha256=hashlib.sha256(content).hexdigest(),
                        authority_matches=True,explicit_numa_matches=matches,thread_and_hint_lines=declarations))
assert not any(r['explicit_numa_matches'] for r in records)
result=dict(scope='Selected unchanged original TFS kernels, original MKL/E2E sources and supplied run/compile scripts',
            explicit_numa_policy_found=False,files=records,
            distinction='Default page placement, parallel first writes, OS automatic NUMA balancing and internal library heuristics are not evidence of an explicit NUMA optimization in these source files.')
target=root/'docs/SOURCE_POLICY_AUDIT_20261004.json'
target.write_text(json.dumps(result,indent=2)+'\n')
print('SOURCE_POLICY_AUDIT',target,'files',len(records),'explicit_numa_matches=0')
