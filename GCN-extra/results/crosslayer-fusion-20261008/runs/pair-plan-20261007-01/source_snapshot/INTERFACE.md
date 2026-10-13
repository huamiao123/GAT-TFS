# Pair-plan interface, frozen before implementation
Namespace gcn_extra_pair_plan. Include state_kernels.hpp/pair_reducer.hpp.
struct TilePlan { uint8_t order[16]; uint32_t windows; uint64_t mask_offset; };
struct Plan { std::string name; std::vector<TilePlan> tiles; std::vector<uint8_t> exact_masks; ...stats allowed... };
Each order is a bijection on [0,batch), paired (order[0],order[1]), etc. Remaining order bytes=255. Original perm/AMX row slots unchanged.
Tile number = i/16. Exact mask byte for global S64 window ws/64, bit p for p-th pair, is set ONLY after equal positive lengths and memcmp of the actual CSR sequences. Empty pairs do not set flags. Valid across M64/M256/FULL.
prepare_plan(g,perm,const std::string& name,int kind,const char* graph) -> Plan. kind0 adjacent,1 sampled-greedy,2 seeded-random. Outputs PAIRPLAN_SETUP and PAIRPLAN_STRUCTURE outside timing. Immutable Plan after build; no global mutable context.
Kernel: template<bool P,bool B16,int MODE> void kernel(g,h,w,out,perm,const gcn_extra_state::Method&,const Plan&,Profile*); MODE0 table scalar;1 dynamic interleave;2 dynamic share;3 exact-window interleave else scalar;4 exact-window share else scalar.
Profile derives gcn_extra_state::Profile; optional extra counters allowed. Same source_visits=E,adds=128E,projection counts,prefetch requests; decode E minus dynamic hits(MODE2) or exact shared edges(MODE4). Exact MODE4 may remove current-index reads for the second row; report logical CSR requests honestly.
No source graph edits, CSR row reorder, precision change, AMX row remapping, NUMA or prefetch changes. Ordinary intrinsics; natural compiler CSE allowed. In exact I read distinct row CSR and decode separately; in exact S decode first row once and add to both. Never weaken control using volatile asm.
Builder owns pair_plan.hpp; kernel owns planned_reducer.hpp/planned_kernel.hpp; runtime owns plan_runtime.hpp/plan_suite.py. Root owns protocol/generator/build/submission/analysis.
