"""Independent scalar invariants and frozen schedule identities for real outputs."""
import json
from pathlib import Path
import sys

data=json.loads(Path(sys.argv[1]).read_text())
prior=json.loads(Path(sys.argv[2]).read_text())
name=sys.argv[3]
g=next(x for x in prior['graphs'] if x['graph']==name)
assert data['schema_version']=='frontier-structure-v1'
assert data['analysis_pass'] is True and data['graph']==name
assert (data['n'],data['e'])==(g['N'],g['E'])
assert data['file_bytes']==36+4*(g['N']+1)+8*g['E']
assert data['batch_rows']==16 and data['rank_base']==1
assert (data['source_bytes'],data['partial_bytes'],data['ready_bytes'])==(256,512,256)
assert data['full_hidden_reference_bytes']==256*g['N']
assert data['empty_fastpath'] is True
model=data['model']
for field in ('read_values','intradrain_peak_modeled','hardware_cache_or_dram_claim',
              'metadata_bytes_in_state','amx_scratch_in_state','empty_rows_enter_ready',
              'no_consumer_sources_retained'):
    assert model[field] is False,field
assert model['strict_csr_occurrence_order'] is True
assert model['single_producer_batch'] is True and model['ready_arbitrary_rows'] is True
assert model['partial_segment_spill_assumed'] is True
n,e=data['n'],data['e']
nonempty=n-data['rows_skipped_zero']
used=n-data['no_consumer_sources']
assert 0<=nonempty<=n and 0<=used<=n
assert data['unsorted_rows']==0, 'Frozen prior diagnosed all four CSR files sorted'
assert 0<=data['adjacent_duplicate_occurrences']<=e-nonempty
orders={o['name']:o for o in data['orders']}
assert len(data['orders'])==3 and set(orders)=={'nodeid','degree','qguard'}
for order in ('degree','qguard'):
    assert type(orders[order]['permutation_fnv64']) is int
    assert orders[order]['permutation_fnv64']==int(g[order+'_fnv64']),('Frozen permutation mismatch',name,order)
    work=orders[order]['first_layer_work_proxy']
    old=g[order+'_audit']
    assert work['sum_tile_max_degree_x16']==int(old['padded_steps'])
    assert work['sum_tile_s64_windows']==int(old['windows'])
    assert work['nonempty_producer_tiles']==int(old['full_projections'])
    assert work['actual_kernel_work_claim'] is False
h=1469598103934665603
for j in range(n):
    h=((h^j)*1099511628211)&((1<<64)-1)
assert orders['nodeid']['permutation_fnv64']==h
assert orders['nodeid']['pure_forced']['peak_source_rows']==0
for order,o in orders.items():
    assert o['producer_batches']==(n+15)//16
    f=o['pure_forced']
    assert f['peak_bytes']==256*f['peak_source_rows']
    assert f['byte_batch_area']==256*f['source_row_batch_area']
    assert 0<=f['peak_source_rows']<=used
    methods={m['name']:m for m in o['methods']}
    assert len(o['methods'])==5 and set(methods)=={'checkpoint_1','checkpoint_4','checkpoint_16','checkpoint_ALL','row_complete'}
    assert f['eager_segments']==methods['checkpoint_1']['segments']
    assert f['source_row_batch_area']==methods['checkpoint_1']['source_row_batch_area']
    assert methods['checkpoint_1']['segments']>=methods['checkpoint_4']['segments']>=methods['checkpoint_16']['segments']>=nonempty
    for key,m in methods.items():
        assert m['logical_edges_consumed']==e
        assert m['nonempty_rows_completed']==nonempty
        assert nonempty<=m['segments']<=e
        assert m['resumes']==m['segments']-nonempty
        assert m['partial_stores']==m['partial_loads']==m['resumes']
        assert m['logical_partial_store_bytes']==m['logical_partial_load_bytes']==512*m['resumes']
        assert m['logical_source_operand_bytes']==256*e
        assert m['full_output_groups']==nonempty//16 and m['final_tail_rows']==nonempty%16
        assert m['final_tail_groups']==int(nonempty%16!=0)
        assert m['projection_accounted_rows']==16*((nonempty+15)//16)
        assert m['peak_all_boundaries_bytes']==max(m['peak_before_batch_bytes'],m['peak_after_produce_bytes'],m['peak_after_drain_bytes'])
        assert 0<=m['peak_boundary_source_rows']<=used
        assert 0<=m['peak_boundary_partial_rows']<=nonempty
        assert 0<=m['peak_boundary_ready_rows']<=15
        assert m['postdrain_byte_batch_area']==256*m['source_row_batch_area']+512*m['partial_row_batch_area']+256*m['ready_row_batch_area']
        assert m['post_final_flush_state']==dict(source_rows=0,partial_rows=0,ready_rows=0,bytes=0)
        if key in ('checkpoint_ALL','row_complete'):
            assert m['segments']==nonempty and m['resumes']==0
            assert m['partial_row_batch_area']==m['peak_boundary_partial_rows']==0
    assert methods['checkpoint_ALL']['peak_after_produce_bytes']==used*256
    assert methods['row_complete']['peak_boundary_source_rows']<=methods['checkpoint_ALL']['peak_boundary_source_rows']
print(json.dumps(dict(status='PASS',graph=name,orders=3,methods_per_order=5,
                     frozen_degree_qguard_permutations_equal=True,precision='structure-only'),sort_keys=True))
