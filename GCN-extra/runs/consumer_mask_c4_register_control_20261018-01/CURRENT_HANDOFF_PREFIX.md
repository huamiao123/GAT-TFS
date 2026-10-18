# 2026-10-18 C4 寄存器驻留强对照

`Job10996710` 独享32核、六图、六顺序交错。逐行对照采用8个 ZMM 累加器与下一源行预取；汇编核查证实循环内寄存器累加。真实 Indochina-2004：共同 mask 471.19ms、逐行寄存器714.88ms、同计划逐源724.41ms；构造 Mycielskian19：1404.96/3386.28/2899.99ms。as-Skitter、RoadNet 明显退化；Reddit 的收益主要来自计划遍历，不宜归于共同求和。详见[强对照报告](../../results/consumer-mask-c4-kernel-20261018/REGISTER_CONTROL_RESULTS.md)，原始证据 `runs/consumer_mask_c4_register_control_20261018-01/`。

当前有真实图稀疏阶段的可信候选，但还需寄存器**无预取**逐行对照，不能把带预取版本当最优基线；也未完成 canonical FULL+AMX 两层实验与计划摊销。128→128→128、默认NUMA、不使用interleave，论文级结果仍 UNVERIFIED。

---

