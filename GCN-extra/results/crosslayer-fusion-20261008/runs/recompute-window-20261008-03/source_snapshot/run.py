import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
study,run=map(Path,sys.argv[1:3]); src=study/'source_snapshot'; build=run/'build'; result=run/'result'
def sha(p):
 h=hashlib.sha256()
 with Path(p).open('rb') as f:
  for b in iter(lambda:f.read(8*1024*1024),b''): h.update(b)
 return h.hexdigest()
def save(p,o):
 with Path(p).open('x') as f: json.dump(o,f,indent=2); f.write('\n')
def call(label,argv,cwd=None):
 save(run/(label+'.command.json'),[str(x) for x in argv])
 with (run/(label+'.stdout.log')).open('wb') as o,(run/(label+'.stderr.log')).open('wb') as e:
  ret=subprocess.run([str(x) for x in argv],cwd=cwd,stdout=o,stderr=e)
 save(run/(label+'.exit.json'),{'exit_code':ret.returncode}); assert ret.returncode==0,(label,ret.returncode)
def main():
 m=json.loads((study/'snapshot_manifest.json').read_text())
 for n,d in m['source_sha256'].items(): assert sha(src/n)==d,n
 for p,d in m['authority_sha256'].items(): assert sha(p)==d,p
 for n,d in m['authority_spec_sha256'].items(): assert sha(ROOT.parent/'TFS-Train/docs/spec'/n)==d,n
 assert sha(ROOT/'IMPLEMENTATION_CONTRACT.md')==m['contract_sha256']
 build.mkdir(); result.mkdir(); binary=build/'window_scan'
 call('compile',['icpx','-O3','-std=c++17',src/'window_scan.cpp','-o',binary],cwd=build)
 save(run/'binary_identity.json',{'path':str(binary),'sha256':sha(binary)})
 prior=json.loads((src/'frontier_prior_inputs.json').read_text()); outs=[]
 for g in sorted(prior['graphs'],key=lambda x:x['E']):
  out=result/(g['graph']+'.json'); call('scan_'+g['graph'],[binary,out,g['path'],g['graph']],cwd=build)
  d=json.loads(out.read_text()); assert d['status']=='PASS'; outs.append(d); print('DONE',g['graph'],flush=True)
 save(result/'audit.json',{'schema':'recompute-window-audit-v1','status':'PASS','results':outs,'performance_claim':False,'no_numa_interleave':True,'limitations':['logical proxies only','no partial/ready state','no DRAM/cache/synchronization','no legal full-kernel schedule']})
if __name__=='__main__': main()
