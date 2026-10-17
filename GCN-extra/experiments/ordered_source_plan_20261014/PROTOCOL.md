# Ordered partial-source event plans, 2026-10-14

User continuation authorizes an isolated implementation and experiments.
Static unit-edge 128->128->128, source H/W seed12345, S64, FULL FAST performance.
Keep original DegreeSort TR16 members/row slots, R64 dynamic(1), per-row CSR
occurrence and addition order, BF16 truncation, PACK_EPI, AMX/W packing,
prefetch, compiler and default NUMA. No bundle, new sorting, normalization,
training or checkpoint changes. All server writes wzh; yx read-only.
At most two single-node jobs this round; our node cap remains five.

New inspector encodes the frozen current-head merge of adjacent row pairs
as mask1/mask2/mask3 runs, two bytes/run. Occurrences are not set-deduplicated.
Select windows with >=8 shared heads and <=8 runs; fixed architectural
metadata/control bound, not graph-tuned. No eligible/exact window in a tile:
retain only tile metadata and execute original ScalarReducer directly.
Exact short windows are separately marked for inherited exact-only control.
Expand every stored run against original CSR to certify order, lengths and
shared IDs. Plan is graph-static; new H activations never cached across calls.

Main methods: original TFS/MKL anchors, P0 FULL, canonical PACK_EPI,
legacy dynamic-all sharing, RLE independent/shared, dynamic sharing on the
same selected windows, exact-only sharing. Same-plan independent decoding
uses ordinary CSR loads/intrinsics; no volatile/artificial compiler barriers.
Shared execution may also remove current CSR reads; disclose that difference.
Do not call benefits over dynamic matching a win unless canonical also improves.

Before performance: tail/empty/unsorted/duplicate/crossing sequences, partial
long common runs and exact-window fixtures; four-boundary same-scope bitwise
checks; FAST/ACCURATE and M64/M256/FULL smoke; original19-valueEPI semantics.
Original numerical gates unchanged. Full production profile must preserve
output. Work gates E occurrences,128E adds,32*nonempty_tiles TDP; decoded rows
E minus the corresponding certified matched count. Packing/state/prefetch
budgets equal. Structural match counts are not physical cache or DRAM bytes.

First one-core smoke + fivegraph structure-only gate. If fixture checks pass,
inspect coverage before32-core diagnostic performance. Low/zero coverage
graphs remain negative controls. Products/Reddit/Myciel/RoadNet/wiki-Talk.
One socket32physical cores,close/cores,shared intel allowed,default NUMA.
One warmup+5 consecutive+10 forward/reverse E2E samples, all retained.
Prepared E2E excludes initial H/W/graph/sort/plan; separately record plan
construction,audit,logical+allocated bytes and amortization. Independent
3-repeat stages,12/24 sampled phases/thread records; not wall percentages.
PMU optional independent complete forward; no unsupported counters claimed.

Original frozen snapshots/controls immutable. New headers/runtime generated
from passing state/pair-plan snapshots; exact commands/source/binary/graph
hashes and all failures saved; three inference handoffs synchronized.
No graph-wise best strategy treated as deployed policy or novelty proof.
