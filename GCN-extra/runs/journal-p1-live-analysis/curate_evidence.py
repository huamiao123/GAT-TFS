import hashlib,json,pathlib,sys
repo=pathlib.Path(sys.argv[1])
out=repo/'GCN-extra/results/p2-overlap-20261005'
audit=json.loads((out/'assembly_audit.json').read_text())
event=dict(id='p2-amx-decoder-audit-20261005',kind='decoder_issue_and_resolution',status='RESOLVED',
    evidence='/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/p2-overlap-20261005-v2',
    paper_eligible=False,issues='GNU objdump 2.30 does not decode AMX; its (bad) entries are not an instruction audit. The old immutable disassembly remains preserved.',
    resolution='Decode actual ELF function bytes with iced-x86 1.21.0. ELF symbol sizes/PT_LOAD select bytes; the old demangled output supplies addresses only. Both functions have zero invalid instructions. AVX neighbor reduction appears between current-tile AMX K blocks in the interleaved function.',
    binary_sha256=audit['binary_sha256'],decoder=audit['decoder'],
    decoded_hashes={k:audit[k]['sha256'] for k in ['serial','interleaved']},
    next='Instruction ordering is validated; effective hardware overlap and full-model speedup are not thereby proved.')
(out/'decoder_issue_event.json').write_text(json.dumps(event,indent=2)+'\n',encoding='utf-8')
print(json.dumps(event,ensure_ascii=False))
