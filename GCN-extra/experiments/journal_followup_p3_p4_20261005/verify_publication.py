"""Check final file bytes, optionally including the staged Git blobs."""
import hashlib
import json
import pathlib
import subprocess
import sys

root=pathlib.Path(sys.argv[1]).resolve()
repo=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None
inventory=json.loads((root/'FINAL_ARTIFACT_HASHES.json').read_text())
for path,digest in inventory.items():
    actual=root/path
    assert hashlib.sha256(actual.read_bytes()).hexdigest()==digest,path
    if repo:
        git_path=actual.relative_to(repo).as_posix()
        blob=subprocess.check_output(['git','cat-file','blob',':'+git_path],cwd=repo)
        assert hashlib.sha256(blob).hexdigest()==digest,('index byte mismatch',git_path)
print('PUBLICATION_BYTES_VERIFIED',len(inventory),'files','including Git index' if repo else 'filesystem')
