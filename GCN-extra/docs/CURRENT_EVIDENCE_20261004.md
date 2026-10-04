# Current evidence and authorized cleanup

Retained: original TFS, exact original source MKL, modified TFS; 17-graph main
controlled sweep plus consecutive-measurement control, including all slow cases.
Main raw data: ../runs/paper-method-reconciled-20261004/.
Supplement raw data: ../runs/paper-cache-reconciled-20261004/.
Sources and reproducible active scripts are linked in ../README.md.

Removed: superseded experiment runs, reports, root source variants, their
launch/build scripts, all superseded archives and historical result entries
in the active handoffs. Graph data and toolchains were not touched. Unrelated
GAT work was not touched. The Git experiment branch will record ordinary
deletions; previous commits remain available, without force push/history rewrite.

Before/after hashes protect every retained current run file and all three
original source files. Active generated .cpp and kernel bytes match the
actually compiled main snapshot. Graph inventory now contains independent
full input hashes; no active launcher depends on a deleted old run directory.
No performance measurement was repeated or changed during cleanup.

Cleanup inventory and verification: ../runs/cleanup-current-20261004/.
Clean archive manifest there supersedes the old current-run packaging manifest,
which is retained only as immutable provenance of the earlier packaging step.
Frozen snapshots retain actual generation inputs/launchers even when they are
obsolete today; they are not alternative active implementations.
