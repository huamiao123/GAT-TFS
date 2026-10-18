import json, hashlib, pathlib, collections
root=pathlib.Path(r'C:\Users\花喵\tmp\tfs18')
audit=root.parent/'tfs_deep_review_20261018'
tree=json.loads((audit/'github_tree.json').read_text(encoding='utf8'))
counts=collections.Counter(); extensions=collections.Counter(); mismatches=[]; missing=[]; total=0; unique=set()
for item in tree['tree']:
    if item['type']!='blob' or not item['path'].startswith('GCN-extra/'):
        continue
    rel=item['path'][len('GCN-extra/'):]; p=root/rel
    if not p.is_file(): missing.append(rel); continue
    b=p.read_bytes(); digest=hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()
    if digest!=item['sha']: mismatches.append(rel)
    counts[rel.split('/')[0]]+=1; extensions[p.suffix]+=1; total+=len(b); unique.add(digest)
out={'commit':'9369b9e0491d59e328a99153b6d97314c62285da','checked_files':sum(counts.values()),'unique_blobs':len(unique),'bytes':total,'top_directories':dict(counts),'extensions':dict(extensions),'missing':missing,'mismatches':mismatches}
(audit/'snapshot_verification.json').write_text(json.dumps(out,ensure_ascii=False,indent=2),encoding='utf8')
print(json.dumps(out,ensure_ascii=True,indent=2))
