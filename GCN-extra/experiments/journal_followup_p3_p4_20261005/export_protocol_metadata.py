"""Export run controls missing from the compact raw-log archive."""
import json
import pathlib
import shutil
import sys

out=pathlib.Path(sys.argv[1])
summary=json.loads((out/'summary.json').read_text())
for study in ['p3','p4']:
    for record in summary[study]['runs']:
        source=pathlib.Path(record['path'])
        target=out/'protocol_metadata'/study/source.name
        target.mkdir(parents=True,exist_ok=False)
        for name in ['graph_list.json','suite_status.json','affinity.json','cpu.txt',
                     'node.txt','numa_policy.txt','preflight.sha256','binary.sha256',
                     'compiler.txt','exit_status.txt','event.json','completion.json',
                     'contract.read.txt']:
            path=source/name
            if path.is_file():
                shutil.copyfile(path,target/name)
print('EXPORTED run metadata')
