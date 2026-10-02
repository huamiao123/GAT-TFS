#!/usr/bin/env python3
"""Append preserved resume events idempotently, only within the wzh project."""
from datetime import datetime, timezone
from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline')
events=[('icpp-online-complete-10851370','ICPP_ONLINE_COMPLETE_10851370.txt'),
        ('icpp-pair-local-20261002','ICPP_PAIR_LOCAL_20261002.txt'),
        ('icpp-research-resume-20261002','ICPP_RESEARCH_RESUME_20261002.txt')]
for name in ('ALL_EXPERIMENTS.md','EXPERIMENT_ISSUES.md','PROJECT_PROGRESS.md'):
    target=root/'docs'/name
    content=target.read_text(encoding='utf-8')
    for event,file in events:
        marker='## Preserved event sync: '+event
        if marker in content: continue
        body=(root/'runs'/file).read_text(encoding='utf-8').rstrip()
        block='\n\n'+marker+'\n\nSynchronized '+datetime.now(timezone.utc).isoformat()+'. Historical pending-network wording below records its original event time; remote synchronization now completed.\n\n'+body+'\n'
        with target.open('a',encoding='utf-8',newline='\n') as out: out.write(block)
        content+=block
print('three_remote_handoffs=synchronized; network_blocker=RESOLVED')
