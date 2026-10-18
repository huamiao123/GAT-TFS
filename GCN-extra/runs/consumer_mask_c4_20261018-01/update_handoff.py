#!/usr/bin/env python3
import pathlib
import sys

root=pathlib.Path(sys.argv[1]);prefix=pathlib.Path(sys.argv[2]).read_text(encoding='utf-8')
path=root/'docs/handoff/CURRENT_HANDOFF.md'
old=path.read_text(encoding='utf-8')
if old.startswith(prefix):
    print('CURRENT_HANDOFF_ALREADY_UPDATED')
elif '2026-10-18 C4 共同消费者 mask 结构门槛' in old:
    raise RuntimeError('unexpected existing C4 handoff section')
else:
    tmp=path.with_name(path.name+'.c4.tmp')
    tmp.write_text(prefix+old,encoding='utf-8')
    tmp.replace(path)
    print('CURRENT_HANDOFF_UPDATED')
