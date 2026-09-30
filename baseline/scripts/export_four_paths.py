"""Export four named views of existing results; never rename raw experiment paths."""
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
run = root / 'runs' / 'local-10810186'
summary = json.loads((run / 'summary.json').read_text(encoding='utf-8'))
target = root / 'results' / 'four_paths_10810186'
target.mkdir(parents=True, exist_ok=True)
paths = {
    'B0_FP32': ('B0_FP32', 'Modified B0 FP32', 'FP32',
                ['src/baseline_standard.cpp'], 'standard_layer(..., fp32=true)'),
    'B0_BF16': ('B0_BF16', 'Modified B0 BF16', 'BF16 projection / FP32 sparse state',
                ['src/baseline_standard.cpp'], 'standard_layer(..., fp32=false)'),
    'B1_before_BF16': ('B1_TFS', 'B1 before local-U execution-order change',
                       'BF16 weighted edge projection / FP32 output',
                       ['src/baseline_tfs.cpp'], 'tfs_layer(..., lr_policy="bf16")'),
    'B1_after_FP32': ('local_online_fp32', 'B1-derived local-U improvement (FP32)',
                      'FP32 attention / online state / sparse aggregation / UW',
                      ['ours/local_online.cpp', 'ours/local_online.hpp', 'ours/main.cpp'],
                      'gat::local::layer(..., amx=false)'),
}
for filename, (raw_path, label, precision, sources, entry) in paths.items():
    result = {
        'label': label, 'original_log_path': raw_path, 'precision': precision,
        'source_files_relative_to_baseline': sources, 'entry': entry,
        'run_id': 'local-10810186', 'performance_acceptance': 'EXPLORATORY_SHARED_NODE',
        'task_accuracy_acceptance': 'UNVERIFIED',
        'timing': 'Preallocated complete three-layer forward; 1 warmup, 3 measured repetitions.',
        'note': 'B1 after is FP32, not a BF16/AMX B1 implementation.' if filename == 'B1_after_FP32'
                else 'Uses corrected explicit NUMA policy; B1 before means before local-U algorithm change.',
        'datasets': {},
    }
    for dataset, data in summary.items():
        assert data['complete']
        measurements = [json.loads(line) for line in (run / f'{dataset}_local.log').read_text().splitlines()
                        if line.startswith('{') and json.loads(line)['path'] == raw_path]
        assert len(measurements) == 9
        result['datasets'][dataset] = {
            'summary': data['paths'][raw_path], 'raw_measurements': measurements,
            'errors_vs_B0_FP32': [e for e in data['errors_vs_B0_FP32'] if e['path'] == raw_path],
            'sample_profiles': [p for p in data['sample_profiles'] if p['path'] == raw_path],
            'online_statistics': [p for p in data['online_statistics'] if p['path'] == raw_path],
            'original_log_relative_to_baseline': f'runs/local-10810186/{dataset}_local.log',
        }
    (target / f'{filename}.json').write_bytes((json.dumps(result, indent=2) + '\n').encode('utf-8'))
print(f'Exported {len(paths)} result views to {target}')
