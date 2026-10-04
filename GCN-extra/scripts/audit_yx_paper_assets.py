#!/usr/bin/env python3
"""Read-only inventory: NPY headers and PT pickle metadata, never unpickle."""
import ast,datetime,json,os,pathlib,pickletools,struct,zipfile
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
yx=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/yx/TFS')
run=root/'runs'/('paper-assets-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S'));run.mkdir()
inventory=[]
for base,dirs,files in os.walk(str(yx)):
 for name in files:
  p=pathlib.Path(base)/name
  if p.suffix.lower() in {'.pt','.pth','.npy','.npz','.ckpt'}:
   inventory.append(dict(path=str(p),bytes=p.stat().st_size,suffix=p.suffix))
details=[]
for rec in inventory:
 p=pathlib.Path(rec['path']);name=p.name.lower()
 if name=='feats.npy' or name=='w0_after_step1.npy':
  with p.open('rb') as f:
   magic=f.read(6);ver=f.read(2);assert magic==b'\x93NUMPY'
   n=struct.unpack('<H' if ver[0]==1 else '<I',f.read(2 if ver[0]==1 else 4))[0]
   d=ast.literal_eval(f.read(n).decode('latin1').strip())
  details.append(dict(rec,header=d,role='true input features' if name=='feats.npy' else 'single-step debugging weight; incomplete trained model'))
 elif name=='ogbn_products_natural_copy.pt':
  with zipfile.ZipFile(str(p)) as z:
   info=next(x for x in z.infolist() if x.filename.endswith('/data.pkl'))
   meta=z.read(info)
   strings=[arg for op,arg,pos in pickletools.genops(meta) if op.name in {'BINUNICODE','SHORT_BINUNICODE','UNICODE'}]
   details.append(dict(rec,pickle_member=info.filename,pickle_unicode_metadata=strings,role='graph/features/labels package; not established model checkpoint',executed_pickle=False))
checkpoint_candidates=[x for x in inventory if x['suffix'].lower() in {'.pth','.ckpt'} or any(k in pathlib.Path(x['path']).name.lower() for k in ['checkpoint','weight','model'])]
result=dict(readonly_root=str(yx),inventory=inventory,selected_headers=details,checkpoint_candidates=checkpoint_candidates,scope='All tensor archives/NPY/NPZ/CKPT under yx/TFS; no complete trained checkpoint identified by name. This is not proof that no checkpoint exists anywhere under yx.',paper_original_input='Both original benchmarks generate random H/W with srand(12345), D=F=128; they do not load these real feature/weight files.')
(run/'inventory.json').write_text(json.dumps(result,indent=2)+'\n')
event=dict(id=run.name,kind='paper_real_asset_audit',status='READ_ONLY_AUDIT_COMPLETE',evidence=str(run),paper_eligible=False,issues='Available real features have 602/100 input dimensions; single-step W snapshots are not complete model checkpoints. Paper performance reproduction must first use source-defined random 128-dimensional input.',next='Reproduce original paper source, keep checkpoint inference as a separate protocol')
(run/'event.json').write_text(json.dumps(event,indent=2))
print(json.dumps(dict(run=str(run),inventory_count=len(inventory),headers=details,checkpoint_candidates=checkpoint_candidates),indent=2))
