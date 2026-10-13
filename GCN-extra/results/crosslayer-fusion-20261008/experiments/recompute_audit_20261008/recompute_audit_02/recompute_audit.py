import hashlib, json, mmap, struct, sys, time
from pathlib import Path

PRIOR=Path(sys.argv[1]); OUT=Path(sys.argv[2])

def sha(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as f:
        for b in iter(lambda:f.read(8*1024*1024),b): h.update(b)
    return h.hexdigest()
def save(path,obj):
    with Path(path).open('x',encoding='utf-8') as f: json.dump(obj,f,indent=2);f.write('\n')
def ceil16(x): return (x+15)//16

def audit(g):
    path=Path(g['path']); before=path.stat(); actual=sha(path)
    assert actual==g['input_sha256'],(g['graph'],actual,g['input_sha256'])
    with path.open('rb') as f, mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ) as mm:
        magic0,magic1,version=struct.unpack_from('<III',mm,0)
        n,nn,e=struct.unpack_from('<QQQ',mm,12)
        assert (magic0,magic1,version)==(0,0,2) and n==nn==g['N'] and e==g['E']
        assert len(mm)==36+4*(n+1)+8*e
        rowbase=36; colbase=36+4*(n+1)
        rowptr=memoryview(mm)[rowbase:colbase].cast('I')
        cols=memoryview(mm)[colbase:colbase+4*e].cast('I')
        rowrank=[0]*n; rows_with=0
        for d in range(n):
            a,b=rowptr[d],rowptr[d+1]; prev=-1
            for x in range(a,b):
                j=cols[x]; assert j<n and j>=prev; prev=j
            if b>a: rowrank[d]=cols[b-1]//16+1; rows_with+=1
        consumer_ranks=[set() for _ in range(n)]; degree=[0]*n
        for d in range(n):
            p=rowrank[d]
            for x in range(rowptr[d],rowptr[d+1]):
                j=cols[x]; degree[j]+=1; consumer_ranks[j].add(p)
        source_rows=[]; extra_edges=0; extra_rows=0; events_by_rank={}
        for j in range(n):
            ranks=sorted(consumer_ranks[j])
            if not ranks: continue
            r=len(ranks); extra=r-1
            extra_edges += extra*degree[j]; extra_rows += extra
            for p in ranks[1:]: events_by_rank[p]=events_by_rank.get(p,0)+1
            source_rows.append(dict(j=j,producer_rank=j//16+1,degree=degree[j],
                consumer_ranks=ranks,reuse_count=r,extra_regenerations=extra,
                extra_edges=extra*degree[j]))
        batches=(n+15)//16; trace=[]; max_candidate=0
        for p in range(1,batches+1):
            base=sum(1 for s in source_rows if s['producer_rank']<=p and max(s['consumer_ranks'])>p)
            candidate=sum(1 for s in source_rows if s['producer_rank']<=p and min(s['consumer_ranks'])>p)
            regenerated=events_by_rank.get(p,0); transient=ceil16(regenerated) if regenerated else 0
            max_candidate=max(max_candidate,candidate+transient)
            trace.append(dict(rank=p,baseline_live_source_rows=base,
                candidate_live_source_rows=candidate,regenerated_rows=regenerated,
                transient_amx_groups=transient,candidate_source_state_bytes=256*candidate,
                transient_regenerated_rows_bytes=256*transient,
                candidate_modeled_bytes=256*(candidate+transient)))
        peak=max(trace,key=lambda x:x['candidate_modeled_bytes']) if trace else {}
        hist={}
        for s in source_rows: hist[str(s['reuse_count'])]=hist.get(str(s['reuse_count']),0)+1
        return dict(graph=g['graph'],n=n,e=e,file_bytes=len(mm),input_sha256=actual,
            input_size=before.st_size,input_mtime_ns=before.st_mtime_ns,input_inode=before.st_ino,
            batch_rows=16,batches=batches,sorted_rows=True,
            consumer_definition='destination row completion rank max(col)//16+1',
            source_rows_with_consumer=len(source_rows),full_h1_bytes=256*len(source_rows),
            baseline_logical_h1_rows=len(source_rows),extra_regeneration_rows=extra_rows,
            extra_first_layer_edge_visits=extra_edges,extra_edge_visit_ratio=extra_edges/e if e else 0,
            total_logical_h1_projections=extra_rows,extra_projection_rows=extra_rows,
            extra_projection_groups_by_consumer_rank=sum(ceil16(x) for x in events_by_rank.values()),
            reuse_histogram=hist,events_by_rank=events_by_rank,candidate_peak=peak,
            candidate_peak_source_rows=max_candidate,candidate_peak_source_bytes=256*max_candidate,
            trace=trace)

def main():
    OUT.mkdir(parents=True,exist_ok=False)
    prior=json.loads(PRIOR.read_text()); results=[]; started=time.monotonic()
    for g in sorted(prior['graphs'],key=lambda x:x['E']):
        results.append(audit(g)); print('DONE',g['graph'],flush=True)
    save(OUT/'audit.json',dict(schema='recompute-audit-v1',status='PASS',model='128->128->128',
      no_numa_interleave=True,feature_reads=False,performance_claim=False,results=results,
      elapsed_seconds=time.monotonic()-started,
      limitations=['logical edge/projection work only','no DRAM/cache/synchronization model',
                   'optimistic one regeneration per consumer rank','does not prove legal AMX schedule',
                   'source state excludes partial/ready state']))
if __name__=='__main__': main()
