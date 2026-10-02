#!/usr/bin/env python3
"""Append an auditable event to all three project handoffs. Never rewrite runs."""
import argparse
from datetime import datetime, timezone
from pathlib import Path

root = Path('/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline')
parser = argparse.ArgumentParser()
parser.add_argument('event')
parser.add_argument('body_file')
args = parser.parse_args()
body_path = Path(args.body_file).resolve()
if root.resolve() not in body_path.parents:
    raise SystemExit('event text must remain under the project')
body = body_path.read_text(encoding='utf-8')
stamp = datetime.now(timezone.utc).isoformat()
for name in ('ALL_EXPERIMENTS.md', 'EXPERIMENT_ISSUES.md', 'PROJECT_PROGRESS.md'):
    target = root / 'docs' / name
    if not target.is_file():
        raise SystemExit(f'missing existing handoff: {target}')
    with target.open('a', encoding='utf-8', newline='\n') as out:
        out.write(f'\n\n## {stamp} {args.event}\n\n{body.rstrip()}\n')
print(f'recorded={args.event} all_three_handoffs=true')
