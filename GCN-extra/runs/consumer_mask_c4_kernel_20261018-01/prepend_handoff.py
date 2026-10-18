#!/usr/bin/env python3
import pathlib
import sys
root=pathlib.Path(sys.argv[1]);prefix=pathlib.Path(sys.argv[2]).read_text(encoding='utf-8')
path=root/'docs/handoff/CURRENT_HANDOFF.md';old=path.read_text(encoding='utf-8')
marker=prefix.splitlines()[0]
if old.startswith(prefix):print('CURRENT_HANDOFF_ALREADY_UPDATED')
elif marker in old:raise RuntimeError('marker exists but prefix differs')
else:
    tmp=path.with_name(path.name+'.c4kernel.tmp');tmp.write_text(prefix+old,encoding='utf-8');tmp.replace(path)
    print('CURRENT_HANDOFF_UPDATED')
