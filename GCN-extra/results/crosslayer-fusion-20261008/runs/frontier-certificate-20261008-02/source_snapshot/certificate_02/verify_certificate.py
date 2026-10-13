"""Read-only capacity-2 matching certificate verifier, apart from new reports.

Certified: 256*M bytes for a fixed node-ID frontier in the restricted
whole-source / independent unfinished-destination partial representation.
Not certified: maximality, native optional-source count, cache/DRAM/time,
unrestricted information lower bounds, recomputation or algebraic sharing.
"""
import argparse
import hashlib
import json
import mmap
from pathlib import Path
import shutil
import struct
import subprocess
import traceback


def need(test, message):
    if not test:
        raise AssertionError(message)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(csr, certificate_dir, structure, structure_status, identity):
    csr, certificate_dir = Path(csr), Path(certificate_dir)
    parent = json.loads(Path(structure).read_text())
    status = json.loads(Path(structure_status).read_text())
    ident = json.loads(Path(identity).read_text())
    cert = json.loads((certificate_dir / 'summary.json').read_text())
    need(status['status'] == 'PASS' and parent['analysis_pass'] is True, 'parent gate failed')
    need(digest(structure) == status['structure_sha256'], 'parent structure SHA mismatch')
    need(parent['schema_version'] == 'frontier-structure-v1', 'wrong parent schema')
    need(parent['unsorted_rows'] == 0 and parent['batch_rows'] == 16 and parent['rank_base'] == 1,
         'parent does not establish sorted node-ID model')
    need(cert['schema'] == 'frontier-certificate-v1', 'wrong certificate schema')
    need(cert['analysis_pass'] is True and cert['source_capacity'] == 1 and cert['destination_capacity'] == 2,
         'certificate analysis/capacity gate failed')
    need(cert['graph'] == parent['graph'] == status['graph'], 'graph mismatch')
    need(cert['order'] == 'nodeid' and cert['unsorted_rows'] == 0 and cert['forced_sources'] == 0,
         'wrong certificate order/model')
    need(cert['batch_rows'] == 16 and cert['rank_base'] == 1 and cert['certificate_record_bytes'] == 16,
         'wrong certificate dimensions')
    need((cert['source_bytes'],cert['partial_bytes'],cert['ready_bytes']) == (256,512,256),
         'certificate state costs differ from proof')
    need(cert['certificate_format'] == 'little-endian:uint32 source,uint32 destination,uint64 csr_offset',
         'wrong record format')
    stat = csr.stat()
    need(ident['actual_sha256'] == status['input_sha256'], 'current input SHA differs from parent')
    need(ident['size'] == stat.st_size, 'current size differs from hashed identity')
    if 'path' in ident:
        need(Path(ident['path']).resolve() == csr.resolve(), 'hashed input path mismatch')
    for key, value in [('mtime_ns', stat.st_mtime_ns), ('inode', stat.st_ino)]:
        if key in ident:
            need(ident[key] == value, 'input stat changed since full hash: '+key)
    order = next(x for x in parent['orders'] if x['name'] == 'nodeid')
    late = next(x for x in order['methods'] if x['name'] == 'row_complete')
    result = dict(schema_version='verified-frontier-certificate-v1', status='PASS', graph=cert['graph'],
        input_sha256_from_runner=ident['actual_sha256'], structure_sha256=digest(structure),
        certificate_summary_sha256=digest(certificate_dir/'summary.json'),
        no_full_csr_payload_rescan=True, frontiers=[],
        certified_scope='Single node-ID frontier; complete 256B source values or independent 512B unfinished-destination partials; no recomputation, separate per-edge contributions, cross-destination factorization, value-based compression or alternate representations.',
        nonclaims=['cache residency', 'DRAM traffic', 'runtime', 'maximum matching',
                   'native optional-source count', 'unrestricted information lower bound'])
    with csr.open('rb') as f, mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as mm:
        need(len(mm) >= 36, 'short CSR header')
        a, b, version, n, nc, e = struct.unpack_from('<IIIQQQ', mm, 0)
        need((a, b, version) == (0, 0, 2) and n == nc, 'invalid CSR header')
        need(n == cert['n'] == parent['n'] and e == cert['e'] == parent['e'], 'CSR dimensions mismatch')
        need(cert['batches'] == (n+15)//16, 'producer batch count mismatch')
        need(len(mm) == 36 + 4*(n+1) + 8*e == cert['file_bytes'] == parent['file_bytes'],
             'CSR payload size mismatch')
        rowptr = [struct.unpack_from('<I', mm, 36+4*i)[0] for i in range(n+1)]
        need(rowptr[0] == 0 and rowptr[-1] == e and all(a <= b for a, b in zip(rowptr, rowptr[1:])),
             'invalid row pointers')
        colbase = 36+4*(n+1)
        def column(offset):
            return struct.unpack_from('<I', mm, colbase+4*offset)[0]
        # One last-column read per nonempty row; relies on the SHA-bound
        # prior all-CSR sortedness gate, NOT certificate boolean assertions.
        row_completion = []
        for i in range(n):
            if rowptr[i] == rowptr[i+1]:
                row_completion.append(0)
            else:
                last = column(rowptr[i+1]-1)
                need(last < n, 'row last column out of range')
                row_completion.append(last//16+1)
        fnv = 1469598103934665603
        for j in range(n):
            fnv = ((fnv ^ j)*1099511628211) & ((1 << 64)-1)
        need(order['permutation_fnv64'] == fnv, 'parent node-ID permutation mismatch')
        frontier_seen = set()
        for entry in cert['frontiers']:
            p = entry['p']
            need(type(p) is int and 0 <= p <= (n+15)//16 and p not in frontier_seen, 'invalid/repeated frontier')
            frontier_seen.add(p)
            produced = min(16*p, n)
            need(entry['produced_limit_exclusive'] == produced, 'producer frontier mismatch')
            file = (certificate_dir / entry['certificate_file']).resolve()
            need(file.parent == certificate_dir.resolve(), 'certificate path escapes directory')
            raw = file.read_bytes()
            need(len(raw) % 16 == 0, 'truncated certificate record')
            need(len(raw) == entry['certificate_bytes'] and len(raw)//16 == entry['matching_records'],
                 'certificate record-count mismatch')
            seen_src = bytearray(n)
            dest_degree = bytearray(n)
            records = 0
            for source, dest, offset in struct.iter_unpack('<IIQ', raw):
                need(source < n and dest < n, 'record node out of range')
                need(source < produced, 'source not produced')
                need(not seen_src[source], 'source capacity exceeded')
                need(dest_degree[dest] < 2, 'destination capacity exceeded')
                need(rowptr[dest] <= offset < rowptr[dest+1], 'offset does not belong to destination')
                need(column(offset) == source, 'CSR occurrence differs from claimed source')
                need(row_completion[dest] > p, 'destination already complete')
                # This actual edge simultaneously witnesses that source has
                # at least one unfinished consumer; no native optional flag.
                seen_src[source] = 1
                dest_degree[dest] += 1
                records += 1
            active = sum(rank > p for rank in row_completion)
            completed = sum(0 < rank <= p for rank in row_completion)
            ready = completed % 16
            need(entry['active_destinations'] == active, 'active destination count mismatch')
            need(entry['completed_nonempty_rows'] == completed, 'completed destination count mismatch')
            need(entry['ready_tail_rows'] == ready, 'ready tail mismatch')
            lower = 256*records
            need(entry['lower_bound_bytes'] == lower, 'lower-bound arithmetic mismatch')
            # U and maximality are not independently certified. Check only
            # the emitter's internal arithmetic and label them separately.
            U = entry['source_only_rows']
            need(type(U) is int and records <= U <= produced, 'native source-only count inconsistent')
            need(entry['source_only_bytes'] == 256*U and entry['source_only_plus_ready_bytes'] == 256*(U+ready),
                 'native upper arithmetic mismatch')
            need(entry['unfinished_state_upper_minus_lower_bytes'] == 256*U-lower and
                 entry['source_plus_ready_upper_minus_lower_bytes'] == 256*(U+ready)-lower,
                 'native gap arithmetic mismatch')
            need(type(entry['full_optional_matching']) is bool and entry['full_optional_matching'] == (records == U),
                 'native full-matching arithmetic mismatch')
            need(lower <= late['peak_after_drain_bytes'], 'lower bound exceeds prior constructive postdrain upper bound')
            result['frontiers'].append(dict(p=p, matched_distinct_sources=records,
                matched_destinations=sum(x > 0 for x in dest_degree), active_destinations_verified=active,
                ready_tail_rows_verified=ready, certified_lower_bound_bytes=lower,
                lower_over_full_hidden=lower/(256*n) if n else 0,
                parent_rowlate_peak_after_drain_bytes=late['peak_after_drain_bytes'],
                parent_rowlate_peak_all_boundaries_bytes=late['peak_all_boundaries_bytes'],
                lower_over_parent_rowlate_postdrain_peak=lower/late['peak_after_drain_bytes'] if late['peak_after_drain_bytes'] else 0,
                native_source_only_rows_NOT_INDEPENDENTLY_CERTIFIED=U,
                native_full_optional_matching_NOT_INDEPENDENTLY_CERTIFIED=entry['full_optional_matching'],
                certificate_sha256=hashlib.sha256(raw).hexdigest(), certificate_records_verified=records))
    return result


def write_csr(path, rows):
    cols=[]; ptr=[0]
    for row in rows:
        cols.extend(row);ptr.append(len(cols))
    n=len(rows);e=len(cols)
    raw=struct.pack('<IIIQQQ',0,0,2,n,n,e)+struct.pack(f'<{n+1}I',*ptr)+struct.pack(f'<{e}I',*cols)+struct.pack(f'<{e}f',*([1.0]*e))
    path.write_bytes(raw)


def run_logged(command, directory, label):
    proc=subprocess.run(command,text=True,capture_output=True,timeout=120)
    (directory/(label+'.stdout.log')).write_text(proc.stdout,encoding='utf-8')
    (directory/(label+'.stderr.log')).write_text(proc.stderr,encoding='utf-8')
    need(proc.returncode==0, label+' binary failed: '+str(proc.returncode))


def self_test(binary, reference_binary, out):
    need(not out.exists(), 'self-test directory must be new');out.mkdir(parents=True)
    fixtures=[('capacity_two',[[0,1,2,16],[0]]+[[] for _ in range(15)]),
              ('repeated',[[0,0,1,32],[0,2,32]]+[[] for _ in range(31)]),
              ('empty',[[]]),('self17',[[i] for i in range(17)]),
              ('cycle33',[sorted([i,(i+1)%33]) for i in range(33)])]
    result=dict(schema_version='certificate-self-test-v1',status='RUNNING',valid=[],mutations=[])
    try:
        for label,rows in fixtures:
            d=out/label;d.mkdir();csr=d/'input.csrbin';write_csr(csr,rows)
            parent=d/'structure.json';parent_status=d/'parent_status.json';ident=d/'identity.json'
            run_logged([reference_binary,'--csr',str(csr),'--out',str(parent),'--graph',label,
                        '--orders','all','--checkpoints','1,4,16,whole'],d,'reference')
            parent_status.write_text(json.dumps(dict(status='PASS',graph=label,structure_sha256=digest(parent),input_sha256=digest(csr))))
            st=csr.stat();ident.write_text(json.dumps(dict(actual_sha256=digest(csr),size=st.st_size,path=str(csr.resolve()),mtime_ns=st.st_mtime_ns,inode=st.st_ino)))
            native=d/'native';frontiers=','.join(map(str,sorted({0,1,(len(rows)+15)//16})))
            run_logged([binary,'--csr',str(csr),'--out',str(native),'--frontiers',frontiers,'--graph',label],d,'certificate')
            report=verify(csr,native,parent,parent_status,ident)
            (d/'verified.json').write_text(json.dumps(report,indent=2)+'\n')
            result['valid'].append(dict(fixture=label,status='PASS',frontiers=len(report['frontiers'])))
        d=out/'capacity_two';base=d/'native';summary=json.loads((base/'summary.json').read_text())
        target=next(x for x in summary['frontiers'] if x['p']==1)
        original=(base/target['certificate_file']).read_bytes()
        need(len(original)==32,'capacity-two fixture did not yield exactly two matching edges')
        used={s for s,dest,offset in struct.iter_unpack('<IIQ',original)}
        missing=next(s for s in (0,1,2) if s not in used)
        mutations=[('duplicate_source', original[:16]+original[:16]+original[32:]),
                   ('destination_overcapacity',original+struct.pack('<IIQ',missing,0,missing)),
                   ('unproduced_source',struct.pack('<IIQ',16,0,3)+original[16:]),
                   ('completed_destination',struct.pack('<IIQ',0,1,4)+original[16:]),
                   ('wrong_offset',struct.pack('<IIQ',0,0,999)+original[16:])]
        for label,raw in mutations:
            clone=out/('mutation_'+label);shutil.copytree(base,clone)
            changed=json.loads((clone/'summary.json').read_text());entry=next(x for x in changed['frontiers'] if x['p']==1)
            (clone/entry['certificate_file']).write_bytes(raw)
            entry['certificate_bytes']=len(raw);entry['matching_records']=len(raw)//16
            # Capacity checks occur before dependent lower/upper arithmetic.
            (clone/'summary.json').write_text(json.dumps(changed))
            try:
                verify(d/'input.csrbin',clone,d/'structure.json',d/'parent_status.json',d/'identity.json')
            except AssertionError as exc:
                result['mutations'].append(dict(case=label,status='REJECTED',reason=str(exc)))
            else:
                raise AssertionError('mutation was accepted: '+label)
        result['status']='PASS'
    except Exception as exc:
        result.update(status='FAIL',error=str(exc),traceback=traceback.format_exc());raise
    finally:
        (out/'status.json').write_text(json.dumps(result,indent=2)+'\n')
    return result


def main():
    p=argparse.ArgumentParser()
    p.add_argument('--self-test',action='store_true');p.add_argument('--binary');p.add_argument('--reference-binary')
    p.add_argument('--csr');p.add_argument('--certificate-dir');p.add_argument('--structure')
    p.add_argument('--structure-status');p.add_argument('--input-identity');p.add_argument('--out',required=True,type=Path)
    a=p.parse_args()
    if a.self_test:
        need(a.binary and a.reference_binary,'self-test needs both binaries')
        result=self_test(a.binary,a.reference_binary,a.out)
    else:
        need(all([a.csr,a.certificate_dir,a.structure,a.structure_status,a.input_identity]),'missing verification input')
        need(not a.out.exists(),'output must be new')
        try:
            result=verify(a.csr,a.certificate_dir,a.structure,a.structure_status,a.input_identity)
        except Exception as exc:
            result=dict(status='FAIL',error=str(exc),traceback=traceback.format_exc())
            a.out.write_text(json.dumps(result,indent=2)+'\n');raise
        result['verifier_sha256']=digest(__file__)
        a.out.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(status=result['status'],output=str(a.out))))


if __name__=='__main__':main()
