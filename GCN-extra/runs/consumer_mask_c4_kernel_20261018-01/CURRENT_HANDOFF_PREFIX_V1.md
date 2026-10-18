# 2026-10-18 C4 同计划 AVX 聚合原型（v1，数值门槛待重测）

`Job10996305` 在共享 `intel` 节点完成，16 CPU、四图、1 次预热与 3 次计时。Mycielskian19 的独立聚合阶段：共同 mask 约 1.624 s、同计划逐源分发约 3.497 s、逐行 AVX 约 4.736 s；Hollywood 只胜同计划分发，未胜逐行；Amazon/RoadNet 都退化。计划构建/内存已计录，Myciel 约 30.25 s/2.355 GB。完整报告：[KERNEL_PILOT_V1.md](../../results/consumer-mask-c4-kernel-20261018/KERNEL_PILOT_V1.md)。

**关键限制：v1 合成 BF16 值是小整数的 1/1024 倍数，四图重排输出碰巧逐位相同，不能作为充分数值验证。** 原始作业保留不改；下一步在新的源码和运行快照中使用更接近原实验随机分布的 BF16 输入，重新测误差与时间。当前只有稀疏聚合原型，没有两层 AMX 集成或论文级加速结论；禁止 NUMA interleave。

---

