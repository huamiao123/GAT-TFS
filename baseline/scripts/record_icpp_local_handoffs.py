#!/usr/bin/env python3
"""Reconcile local handoffs from preserved events; keep remote sync explicit."""
from datetime import datetime, timezone
from pathlib import Path

root = Path(__file__).resolve().parents[1]
events = [
    ('icpp-online-method-correction', 'ICPP_ONLINE_PREFLIGHT_20261001.txt'),
    ('icpp-online-build-20261001-215442', 'ICPP_ONLINE_BUILD_20261001.txt'),
    ('icpp-online-failed-10851291', 'ICPP_ONLINE_FAILED_10851291.txt'),
    ('icpp-online-build-20261001-220654', 'ICPP_ONLINE_REBUILD_20261001.txt'),
    ('icpp-online-failed-10851339', 'ICPP_ONLINE_FAILED_10851339.txt'),
    ('icpp-online-build-20261001-221450', 'ICPP_ONLINE_REBUILD_MATCHED_EXP_20261001.txt'),
    ('icpp-online-complete-10851370', 'ICPP_ONLINE_COMPLETE_10851370.txt'),
    ('icpp-pair-local-20261002', 'ICPP_PAIR_LOCAL_20261002.txt'),
    ('icpp-research-resume-20261002', 'ICPP_RESEARCH_RESUME_20261002.txt'),
    ('icpp-pair-build-20261002-124922', 'ICPP_PAIR_BUILD_20261002.txt'),
    ('icpp-pair-submitted-10853968', 'ICPP_PAIR_SUBMITTED_10853968.txt'),
    ('icpp-pair-failed-10853968', 'ICPP_PAIR_FAILED_10853968.txt'),
    ('icpp-pair-failed-10853976', 'ICPP_PAIR_FAILED_10853976.txt'),
    ('icpp-pair-complete-10853989', 'ICPP_PAIR_COMPLETE_10853989.txt'),
    ('icpp-block-build-20261002-132444', 'ICPP_BLOCK_BUILD_20261002.txt'),
    ('icpp-block-submitted-10854102', 'ICPP_BLOCK_SUBMITTED_10854102.txt'),
    ('icpp-block-failed-10854102', 'ICPP_BLOCK_FAILED_10854102.txt'),
    ('icpp-block-build-20261002-133145', 'ICPP_BLOCK_REBUILD_20261002.txt'),
    ('icpp-block-submitted-10854118', 'ICPP_BLOCK_SUBMITTED_10854118.txt'),
    ('icpp-ph-build-20261002-134035', 'ICPP_PH_BUILD_20261002.txt'),
    ('icpp-block-complete-10854118', 'ICPP_BLOCK_COMPLETE_10854118.txt'),
    ('icpp-ph-submitted-10854131', 'ICPP_PH_SUBMITTED_10854131.txt'),
    ('icpp-ph-complete-10854131', 'ICPP_PH_COMPLETE_10854131.txt'),
    ('icpp-research-reconciled-20261002', 'ICPP_RESEARCH_RECONCILIATION_20261002.txt'),
]
stamp = datetime.now(timezone.utc).isoformat()
for name in ('ALL_EXPERIMENTS.md', 'EXPERIMENT_ISSUES.md', 'PROJECT_PROGRESS.md'):
    target = root / 'docs' / name
    content = target.read_text(encoding='utf-8')
    for event, source in events:
        marker = f'## Local reconciliation: {event}'
        if marker in content:
            continue
        body = (root / 'runs' / source).read_text(encoding='utf-8').rstrip()
        block = f'\n\n{marker}\n\nReconciled {stamp}; event time is in preserved run evidence.\n\n{body}\n'
        with target.open('a', encoding='utf-8', newline='\n') as out:
            out.write(block)
        content += block
print('local_three_handoffs=reconciled; remote_sync_status=see_latest_event')
