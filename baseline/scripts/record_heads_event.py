#!/usr/bin/env python3
"""Append one preserved heads event to all local handoff documents."""
import sys
from datetime import datetime,timezone
from pathlib import Path
root=Path(__file__).resolve().parents[1]
event,bodyfile=sys.argv[1:3]
body=(root/'runs'/bodyfile).read_text(encoding='utf-8').rstrip()
marker=f'## Local reconciliation: {event}'
for name in ('ALL_EXPERIMENTS.md','EXPERIMENT_ISSUES.md','PROJECT_PROGRESS.md'):
    path=root/'docs'/name
    if marker in path.read_text(encoding='utf-8'):continue
    with path.open('a',encoding='utf-8',newline='\n') as f:
        f.write(f'\n\n{marker}\n\nReconciled {datetime.now(timezone.utc).isoformat()}; event time in retained evidence.\n\n{body}\n')
print('local_three_handoffs=reconciled event='+event)
