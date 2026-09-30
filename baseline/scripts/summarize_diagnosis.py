"""Read diagnostic logs and NUMA mappings; no measurement code."""
import json, re, statistics, sys
from pathlib import Path
root = Path(sys.argv[1])
keys = ('convert_ms','projection_ms','lr_ms','max_prescan_ms','score_exp_aggregate_ms','normalize_ms','activation_ms','total_ms')
for f in sorted(root.glob('*_standard_*.log')):
    lines=f.read_text().splitlines()
    rows=[json.loads(s) for s in lines if s.startswith('{')]
    print(f.stem, next((s for s in lines if s.startswith('SUMMARY')), ''))
    for layer in (1,2,3):
        r=[r for r in rows if r['layer']==layer]
        if r: print(' L',layer, {k:round(statistics.median(x[k] for x in r),3) for k in keys})
    nm=f.with_name(f.stem+'_after_allocate.numa_maps')
    if nm.exists():
        maps=sorted((int(s.split()[0],16),s) for s in nm.read_text().splitlines())
        for s in lines:
            if not s.startswith('BUFFER'): continue
            for name,addr in re.findall(r'(z|out)=(0x[0-9a-f]+)',s):
                address=int(addr,16)
                candidates=[r for r in maps if r[0]<=address]
                if candidates: print(' ',s.split()[1],name,'mapping',candidates[-1][1])
