# 2026-10-18 C4 独享32核稀疏阶段复核

`Job10996654` 在 `intel_expr --exclusive` 的 `qhcn818` 完成，实际32线程都在同一插槽 CPU 0–31，六种顺序交错计时且数值门槛通过。Mycielskian19 中位数：共同 mask 1.404s、同计划逐源 2.884s、独立逐行 AVX 3.977s；Hollywood 共同 mask 489.4ms，比同计划逐源快，但慢于逐行 457.5ms；Amazon/RoadNet 退化。详见[独享复核报告](../../results/consumer-mask-c4-kernel-20261018/EXCLUSIVE_SPARSE_STAGE.md)，原始数据 `runs/consumer_mask_c4_exclusive_20261018-01/`。

目前可靠结论仅是构造图上存在强稀疏阶段机制收益，尚无真实图相对逐行的独享胜例，更没有 AMX 两层融合结果；Myciel 计划约2.355GB/30.33s。下一轮先测 Indochina-2004 等真实高/中重叠图，再决定集成。仍保持128→128→128、默认NUMA、不使用interleave，论文声明 UNVERIFIED。

---

