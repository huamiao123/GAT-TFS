#!/usr/bin/env python3
import pathlib,re,json,hashlib,sys
src=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2]);lines=src.read_text().splitlines();audit={}
for label,arg in [('serial','false'),('interleaved','true')]:
    start=next(i for i,s in enumerate(lines) if re.match(r'^[0-9a-f]+ <ProfileP run_p<false, '+arg+r'>',s))
    end=next((i for i in range(start+1,len(lines)) if re.match(r'^[0-9a-f]+ <',lines[i])),len(lines))
    section=lines[start:end];text='\n'.join(section)+'\n';(out/(label+'_asm.txt')).write_text(text)
    positions={name:[i for i,s in enumerate(section) if re.search(r'\b'+name+r'\b',s)] for name in ['tdpbf16ps','vpmovzxwd','vaddps','tileloadd','tilestored']}
    audit[label]=dict(header=section[0],line_count=len(section),instruction_positions=positions,sha256=hashlib.sha256(text.encode()).hexdigest(),counts_are_static_not_executed=True)
audit['source_disassembly_sha256']=hashlib.sha256(src.read_bytes()).hexdigest();audit['scope']='Instruction ordering audit only; does not establish effective hardware overlap as unique timing cause.'
(out/'assembly_audit.json').write_text(json.dumps(audit,indent=2)+'\n');print(json.dumps({k:{'line_count':v['line_count'],'first_tdp':v['instruction_positions']['tdpbf16ps'][:3]} for k,v in audit.items() if isinstance(v,dict)}))
