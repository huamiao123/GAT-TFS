#!/usr/bin/env python3
"""Cheap read-only header inventory; does not scan large arrays on login node."""
import csv
import json
import pathlib
import struct
import sys

data = pathlib.Path(sys.argv[1])
target = pathlib.Path(sys.argv[2])
rows = []
for p in data.glob('*/*.csrbin'):
    if p.name != p.parent.name+'.csrbin':
        continue  # canonical source dataset, retain variants as separate future audits
    with p.open('rb') as f:
        h = f.read(36)
    a, b, c, n, nc, e = struct.unpack('<IIIQQQ', h)
    # Kernel-only references are freed before E2E allocations; live dense peak
    # is now 36*N*128 bytes. CSR + MKL copied indices + MKL unit values = 12*E.
    # CSR indices+MKL unit values, row metadata, and conservative allocator/MKL margin.
    live = 4608*n + 12*e + 32*n
    budget = 1.25*live + 4*1024**3
    rows.append({'graph':p.parent.name, 'path':str(p), 'N':n, 'E':e,
                 'avg_degree':e/n if n else 0, 'format':f'{a},{b},{c}',
                 'square':n==nc, 'size_bytes':p.stat().st_size,
                 'live_estimate_gib':live/1024**3, 'budget_gib':budget/1024**3,
                 'supported_header':a==0 and b==0 and n==nc and n<=2147483647 and e<=4294967295})
rows.sort(key=lambda x:x['avg_degree'], reverse=True)
target.parent.mkdir(parents=True,exist_ok=True)
target.write_text(json.dumps(rows,indent=2),encoding='utf-8')
with target.with_suffix('.csv').open('w',encoding='utf-8',newline='') as f:
    w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
for r in rows:
    print(f"{r['graph']:22s} N={r['N']:10,d} E={r['E']:13,d} q={r['avg_degree']:8.2f} budget={r['budget_gib']:7.1f} GiB header={r['supported_header']}")
