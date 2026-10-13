# Same-storage order control, exploratory follow-up 2026-10-07

Motivation discovered after primary pair-plan data: regular128_reuse1 has zero sharing, identical MODE0 work and four-boundary bitwise outputs, but randScalar is about2.4x faster across3nodes/two protocols. Original controls also used different Plan storage addresses. This isolated control tests visit order with identical plan storage, and separates pair membership from pair-processing order.

Not added to primary17 GM. Same128->128->128, unit CSR, source seed, BF16 truncation, fusedEPI,32physicalcores/single socket/defaultNUMA/no interleave. Combined cap10. Kernels are byte-identical copies of pair-plan-run01. Only order-control runtime/harness/driver changes. Existing shared graph fixtures and all old runs immutable.

Prepare degree adj,opt,rand plans. Derive pairs-shuffle by permuting only full pairs of adjacent plan, preserving each pair orientation, original tile members and AMX row slots, odd final row stays last; permute exact-mask bits accordingly. Fixedseed20261007, independently verify pair sets, slot bijection and mask-bit provenance. This is a fixed diagnostic, not tuning.

Allocate one scratch Plan with fixed vector sizes/capacities. On switch, std::copy tiles and masks to that same memory without assignment/resizing/reallocation. Prove pointer invariance and feature/weight/output pointer invariance. Exclude and separately report copy/setup time; do not claim copying is a deployment strategy.

Methods: pack_epi_degree (frozen state strong baseline), slot_adj_scalar,slot_opt_scalar,slot_rand_scalar,slot_pairs_scalar. All four slot paths execute the same planned MODE0 kernel at the same scratch addresses. No sharing or AMX work reduction is introduced. All outputs require original numeric gates and same-M canonical4boundary bitwise equality; production2layer profiles/work gates, original19-value semantic fixture, inherited13smoke graph shapes; no need rerun90mode/precision matrix because all kernel sources remain identical and onlyMFULL FAST is exercised.

Timing: copy +1warmup outside clock,5consecutive samples;10forward/reverse alternating samples with copy+1warmup before EACH measured sample. Label same-slot protocol distinctly; compare only inside these jobs. PMU all5methods3reps, copy+warmup outside counted region. Retain count coverage, source/binary/input hashes, all rawtimes, static plan bytes/construction/copy costs. No hardware cause claimed merely from elapsed time; PMU generic cache events not DRAM bytes and AMX_BUSY not overlap.

Execute smoke1sharednode then same frozen synthetic3 on1exclusive and1sharednode, respecting combined active/pendingcap10. These are exploratory order controls, not new primary17 or universal graph evidence. All3 handoffs updated. Independent final report incorporates these only after complete gates and accounting. No original execution kernel changes, precision relaxation or cherry-picked seed.
