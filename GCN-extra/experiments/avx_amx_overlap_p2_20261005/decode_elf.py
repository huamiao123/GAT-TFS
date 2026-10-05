#!/usr/bin/env python3
"""Decode actual ELF function bytes with iced-x86, including AMX instructions."""
import pathlib,re,json,hashlib,sys,struct,importlib.metadata
binary,old_disasm,out=map(pathlib.Path,sys.argv[1:4])
if len(sys.argv)>4:sys.path.insert(0,sys.argv[4])
from iced_x86 import Decoder,Formatter,FormatterSyntax,Code
data=binary.read_bytes();assert data[:6]==bytes([127,69,76,70,2,1]),'need little-endian ELF64'
phoff,shoff=struct.unpack_from('<QQ',data,32);phentsize,phnum,shentsize,shnum=struct.unpack_from('<HHHH',data,54)
segments=[struct.unpack_from('<IIQQQQQQ',data,phoff+i*phentsize) for i in range(phnum)]
sections=[struct.unpack_from('<IIQQQQIIQQ',data,shoff+i*shentsize) for i in range(shnum)]
symbols=[]
for sh in sections:
    if sh[1]!=2:continue
    strings=sections[sh[6]];strdata=data[strings[4]:strings[4]+strings[5]]
    for offset in range(sh[4],sh[4]+sh[5],sh[9]):
        name,info,other,index,value,size=struct.unpack_from('<IBBHQQ',data,offset)
        if info&15==2 and size:
            end=strdata.find(bytes([0]),name);symbols.append(dict(name=strdata[name:end].decode(),address=value,size=size))
old=old_disasm.read_text().splitlines();fmt=Formatter(FormatterSyntax.INTEL);audit={}
for label,arg in [('serial','false'),('interleaved','true')]:
    header=next(s for s in old if re.match(r'^[0-9a-f]+ <ProfileP run_p<false, '+arg+r'>',s));address=int(header.split()[0],16)
    sym=next(s for s in symbols if s['address']==address);seg=next(s for s in segments if s[0]==1 and s[3]<=address< s[3]+s[5]);offset=seg[2]+address-seg[3]
    code=data[offset:offset+sym['size']];lines=[header];positions={k:[] for k in ['tdpbf16ps','vpmovzxwd','vaddps','tileloadd','tilestored']};invalid=0
    for instr in Decoder(64,code,ip=address):
        invalid+=instr.code==Code.INVALID;formatted=fmt.format(instr);lines.append(f"{instr.ip:016x}: {code[instr.ip-address:instr.ip-address+instr.len].hex():32s} {formatted}")
        for k in positions:
            if formatted.split()[0].lower()==k:positions[k].append(len(lines)-1)
    assert invalid==0,'invalid decoded function bytes';assert positions['tdpbf16ps'],'decoder must recognize AMX'
    text='\n'.join(lines)+'\n';(out/(label+'_asm.txt')).write_text(text)
    audit[label]=dict(symbol=sym,line_count=len(lines),instruction_positions=positions,sha256=hashlib.sha256(text.encode()).hexdigest(),counts_are_static_not_executed=True,invalid_instructions=invalid)
audit.update(binary_sha256=hashlib.sha256(data).hexdigest(),decoder='iced-x86 '+importlib.metadata.version('iced-x86'),decoder_source='https://pypi.org/project/iced-x86/',old_binutils='2.30 cannot decode AMX; old text used only for demangled symbol/address mapping',scope='Instruction ordering audit; does not establish effective hardware overlap as unique timing cause.')
(out/'assembly_audit.json').write_text(json.dumps(audit,indent=2)+'\n');print(json.dumps({k:{'line_count':v['line_count'],'static_tdp':len(v['instruction_positions']['tdpbf16ps']),'first_tdp':v['instruction_positions']['tdpbf16ps'][:5]} for k,v in audit.items() if isinstance(v,dict)}))
