# Feature-input precision ablation, 2026-10-04

This directory is a copy of the reconciled result tables from the immutable
server study `GCN-extra/runs/feature-precision-ablation-20261004-214015`.
The diagnostic run was Slurm job `10867492` on `qhcn064`; the 12-graph
extension was job `10868182` on `qhcn052`. Each graph's BF16/FP32 comparisons
use measurements from one job, node, process, graph, and measurement order.
The 17 graph records are never pooled to produce absolute times across nodes.

The frozen control source is commit `8aeef1613fdeaeb929379874c4152eccde620f74`.
The separately saved report explains the first-touch correction and the
numerical, timing, profiling, and PMU boundaries. The original raw Slurm logs,
source snapshot, binary hashes, CPU affinity, and NUMA evidence remain in the
server study directory.

`feature_precision_paired_e2e.pdf` and `.png` plot the paired E2E ratios
from `paired_comparisons.csv`; the plotting script is
`GCN-extra/experiments/feature_precision_8aeef16/plot_feature_precision.py`.
The dotted line at one marks equal time. Points to the right favor BF16.
