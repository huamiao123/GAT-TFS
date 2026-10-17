# Ordered partial-source plan evidence

Read GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md first. Static unit-weight two-layer
128->128->128 inference, random H/W seed12345. Fixed DegreeSort adjacent pairs,
S64, >=8 matches and <=8 event runs. FULL FAST main performance; M64/M256 and
ACCURATE are correctness controls, not measured main performance methods.
Shared intel: exploratory timing, paper_eligible=false.

source_snapshot/ is the exact compiled source; raw/ contains all commands,
graphs/fixtures, checks, 675 timing samples, thread diagnostic records and PMU.
Real graph data and executables are not exported. Source/binary hashes were
checked at finalization; remote absolute paths in manifests are intentional.
local_provenance.json describes the local base commit and uncommitted changes;
the remote source tree is not a Git checkout. Do not treat a base commit as a
commit containing this experiment. All thresholds and negative outcomes retained.
