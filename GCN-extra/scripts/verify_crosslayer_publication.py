"""Verify file hashes and optionally all staged Git blobs using one process."""
import hashlib
import json
import pathlib
import subprocess
import sys

root=pathlib.Path(sys.argv[1]).resolve()
mode=sys.argv[2] if len(sys.argv)>2 else 'filesystem'
manifest=root/'PUBLICATION_HASHES.json'


def file_sha(p):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(1<<20),b''):h.update(b)
    return h.hexdigest()


if mode=='freeze':
    manifest.write_text(json.dumps({p.relative_to(root).as_posix():file_sha(p)
        for p in root.rglob('*') if p.is_file() and p!=manifest},indent=2)+'\n',encoding='utf-8')
    print('FROZEN_PUBLICATION_INVENTORY');sys.exit(0)

inventory=json.loads(manifest.read_text(encoding='utf-8'))
for p,digest in inventory.items():assert file_sha(root/p)==digest,p
if mode=='index':
    repo=pathlib.Path(sys.argv[3]).resolve()
    process=subprocess.Popen(['git','cat-file','--batch'],cwd=repo,stdin=subprocess.PIPE,stdout=subprocess.PIPE)
    try:
        for p,digest in inventory.items():
            relative=(root/p).relative_to(repo).as_posix()
            process.stdin.write((':'+relative+'\n').encode('utf-8'));process.stdin.flush()
            header=process.stdout.readline().decode('utf-8').strip().split()
            assert len(header)==3 and header[1]=='blob',(relative,header)
            left=int(header[2]);h=hashlib.sha256()
            while left:
                block=process.stdout.read(min(left,1<<20));assert block
                h.update(block);left-=len(block)
            assert process.stdout.read(1)==b'\n'
            assert h.hexdigest()==digest,relative
    finally:
        process.stdin.close();process.wait(timeout=30)
print('VERIFIED',len(inventory),'publication files',mode)
