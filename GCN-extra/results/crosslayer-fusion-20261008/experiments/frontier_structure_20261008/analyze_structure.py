import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys

root=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
study=root/'runs/frontier-structure-20261008-01'
run=study/'compute-10898893'
out=root/'results/frontier_structure_20261008/analysis-01'
assert not out.exists()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(p,v):p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
completion=json.loads((run/'completion.json').read_text())
assert completion['status']=='PASS'
accounting=subprocess.check_output(['sacct','-j','10898893','-n','-P','--format=JobIDRaw,State,ExitCode,Elapsed,NodeList,AllocCPUS,ReqMem'],text=True)
parent=next(line.split('|') for line in accounting.splitlines() if line.startswith('10898893|'))
assert parent[1:3]==['COMPLETED','0:0'] and parent[4]==completion['node']
manifest=json.loads((study/'snapshot_manifest.json').read_text())
for name,digest in manifest['source_sha256'].items():assert sha(study/'source_snapshot'/name)==digest,name
oracle=json.loads((run/'oracle/status.json').read_text())
assert oracle==completion['oracle_status'] and oracle['status']=='PASS'
assert oracle['fixture_count']==81 and oracle['malformed_count']==6
assert oracle['script_sha256']==manifest['source_sha256']['audit_frontier.py']
assert {s['graph'] for s in completion['graphs']}=={'mycielskian19','reddit','roadNet-CA','amazon0601'}
out.mkdir()
(out/'slurm_accounting.txt').write_text(accounting)
rows=[];graphs=[]
for name in ['mycielskian19','reddit','roadNet-CA','amazon0601']:
    state=next(s for s in completion['graphs'] if s['graph']==name)
    p=run/name/'structure.json';assert sha(p)==state['structure_sha256']
    subprocess.run(['python3',str(study/'source_snapshot/validate_real.py'),str(p),str(study/'source_snapshot/frontier_prior_inputs.json'),name],check=True)
    d=json.loads(p.read_text());ref=d['full_hidden_reference_bytes'];orders={x['name']:x for x in d['orders']}
    reference_work=orders['degree']['first_layer_work_proxy']['sum_tile_s64_windows']
    comparison=dict(graph=name,n=d['n'],e=d['e'],full_h1_bytes=ref,source=str(p),source_sha256=sha(p),orders={})
    for order,o in orders.items():
        methods={m['name']:m for m in o['methods']}
        late=methods['row_complete']
        dominance=[]
        for m in o['methods']:
            if m['name']=='row_complete':continue
            keys=['peak_all_boundaries_bytes','resumes','postdrain_byte_batch_area']
            no_worse=all(late[k]<=m[k] for k in keys)
            strict=any(late[k]<m[k] for k in keys)
            dominance.append(dict(other=m['name'],no_worse_in_three_metrics=no_worse,strictly_dominates=no_worse and strict))
        comparison['orders'][order]=dict(forced_peak_bytes=o['pure_forced']['peak_bytes'],
            forced_peak_fraction=o['pure_forced']['peak_bytes']/ref,
            row_complete_peak_bytes=late['peak_all_boundaries_bytes'],row_complete_fraction=late['peak_all_boundaries_bytes']/ref,
            row_complete_dominance=dominance,
            s64_window_proxy_ratio_vs_degree=o['first_layer_work_proxy']['sum_tile_s64_windows']/reference_work)
        for m in o['methods']:
            rows.append(dict(graph=name,order=order,policy=m['name'],full_h1_bytes=ref,
                forced_peak_bytes=o['pure_forced']['peak_bytes'],
                peak_boundary_bytes=m['peak_all_boundaries_bytes'],peak_fraction_of_full_h1=m['peak_all_boundaries_bytes']/ref,
                peak_after_drain_bytes=m['peak_after_drain_bytes'],peak_after_drain_rank=m['peak_after_drain_rank'],
                resumes=m['resumes'],segments=m['segments'],logical_partial_save_reload_bytes=1024*m['resumes'],
                postdrain_byte_batch_area=m['postdrain_byte_batch_area'],
                mean_postdrain_fraction=m['postdrain_byte_batch_area']/max(1,o['producer_batches'])/ref,
                s64_window_proxy=o['first_layer_work_proxy']['sum_tile_s64_windows'],
                full_output_groups=m['full_output_groups'],final_tail_rows=m['final_tail_rows']))
    graphs.append(comparison)
assert len(rows)==60
with (out/'plan_summary.csv').open('w',newline='') as f:
    writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
save(out/'comparison.json',graphs)
audit=dict(status='PASS',job='10898893',node=completion['node'],slurm_state=parent[1],exit_code=parent[2],
           oracle_fixtures=81,expected_malformed_rejects=6,graphs=4,plans=60,
           manifest_sha256=sha(study/'snapshot_manifest.json'),completion_sha256=sha(run/'completion.json'),
           rows_complete_policy_is_baseline=True,scope='STRUCTURE_ONLY',paper_performance_eligible=False,
           desktop_write=False)
save(out/'audit.json',audit)
record=dict(id='frontier-structure-analysis-20261008-01',status='PASS',evidence=str(out),audit=audit,
            findings=graphs,paper_eligible=False,
            issues='Negative opportunity evidence: frequent partial consumption can increase both boundary state and logical resume payload. Byte-batch area is not elapsed time, ready count is abstract, and source E*256 is not measured traffic.',
            decision='Do not develop complex cross-layer runtime from these counts alone. Compare simple row-complete policy and seek valid mixed-state lower-bound certificates before further implementation.',
            next='Evaluate source/partial capacity-two matching certificates at fixed production frontiers. No claim about arbitrary reorderings or a performance speedup.')
save(out/'event.json',record)
subprocess.run(['python3',str(root/'scripts/record_event.py'),str(out/'event.json')],check=True)
subprocess.run(['python3',str(root/'scripts/record_event.py'),'check'],check=True)
print(json.dumps(audit,ensure_ascii=False))
