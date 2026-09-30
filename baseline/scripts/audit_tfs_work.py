"""Derive executed-work estimates from saved speed logs (not a benchmark)."""
import json
import sys
from pathlib import Path
f=Path(sys.argv[1])/'model_tfs_bf16.log'
rows=[json.loads(s) for s in f.read_text().splitlines() if s.startswith('{')]
for layer in (1,2,3):
    r=next(r for r in rows if r['layer']==layer)
    n,e,d,k,hd=(r[x] for x in ('N','E','D','K','d'))
    dense=2*n*d*k*hd
    aggregation=2*e*k*hd
    edge_projection=2*e*k*d*hd
    tile=2*r['executed_fma']
    slots=r['neighbor_steps']*16
    print(json.dumps(dict(layer=layer,N=n,E=e,D=d,heads=k,head_dim=hd,
        mean_degree=e/n,standard_projection_GFLOP=dense/1e9,
        standard_aggregation_GFLOP=aggregation/1e9,
        tfs_valid_edge_projection_GFLOP=edge_projection/1e9,
        tfs_executed_AMX_GFLOP=tile/1e9,
        projection_amplification_including_padding=tile/dense,
        active_row_fraction=e*k/slots,
        tfs_BF16_feature_read_GB=2*e*k*d/1e9,
        standard_FP32_Z_read_GB=4*e*k*hd/1e9,
        byte_counts='logical loads, not measured DRAM traffic'),ensure_ascii=False))
