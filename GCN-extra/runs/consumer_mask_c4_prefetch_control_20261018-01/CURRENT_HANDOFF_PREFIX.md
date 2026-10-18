# 2026-10-18 C4 有/无预取保守逐行对照完成

`Job10996728` 在 `qhcn818` 独享32核完成，四方法八轮轮换，数值、输入哈希、CPU布局及汇编门槛通过。以每图寄存器逐行有/无预取中较快者作为**仅供评估的保守对照**，真实 Indochina-2004 的共同 mask 聚合中位数257.413ms，对照413.826ms，约1.608×；构造 Mycielskian19 约3.402×。Hollywood近于持平，as-Skitter、Reddit、RoadNet退化。同计划逐源→共同 mask 在 Indochina 约2.399×。详见[完整报告](../../results/consumer-mask-c4-kernel-20261018/PREFETCH_CONTROL_RESULTS.md)，原始数据 `runs/consumer_mask_c4_prefetch_control_20261018-01/`。

**边界：**此前 Reddit 对带预取逐行的收益被无预取强对照推翻；每图挑更快逐行仅是诊断 oracle，不能声称部署策略。不同独享作业的绝对时延波动显著，不能跨作业算比。当前仅稀疏聚合阶段；下一实验需把分组机制接到 canonical FULL Identity 的 AVX→BF16→AMX 两层路径，正面对照并计入计划构建/复用，当前不得称论文级突破。保持128→128→128、默认NUMA，禁止interleave。

---

