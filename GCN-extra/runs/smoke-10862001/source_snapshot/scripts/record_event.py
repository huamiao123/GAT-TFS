#!/usr/bin/env python3
"""Append immutable events and synchronize all three inference handoffs."""
import datetime, json, pathlib, sys
root = pathlib.Path(__file__).resolve().parents[1]
docs = root / 'docs' / 'handoff'
docs.mkdir(parents=True, exist_ok=True)
events = docs / 'events.jsonl'
names = ['ALL_EXPERIMENTS.md', 'EXPERIMENT_ISSUES.md', 'PROJECT_PROGRESS.md']
if sys.argv[1] == 'check':
    if events.exists() and events.stat().st_size:
        event = json.loads(events.read_text(encoding='utf-8').splitlines()[-1])
        for name in names:
            assert (docs / name).exists() and event['id'] in (docs / name).read_text(encoding='utf-8'), (name,event['id'])
    print('HANDOFF_SYNCHRONIZED')
    sys.exit(0)
event = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding='utf-8'))
event.setdefault('date', datetime.datetime.now(datetime.timezone.utc).isoformat())
for name in names:
    p = docs / name
    if not p.exists(): p.write_text('# '+name[:-3]+' — GCN-extra inference\n\nClaims remain UNVERIFIED until qualified evidence is available.\n',encoding='utf-8')
    with p.open('a',encoding='utf-8') as f:
        f.write('\n## '+event['id']+'\n\n')
        f.write('```json\n'+json.dumps(event,ensure_ascii=False,indent=2)+'\n```\n')
with events.open('a',encoding='utf-8') as f: f.write(json.dumps(event,ensure_ascii=False)+'\n')
print('RECORDED', event['id'])
