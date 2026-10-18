# 2026-10-18 C4 随机 BF16 数值压力复测（v2）

`Job10996636` 在共享 `intel` 节点完成，四图同计划 AVX 聚合比较。新的随机 BF16 输入使数值重排误差可见：Hollywood 相对 L2 `1.13e-7`、BF16 聚合差异 49 个；Mycielskian19 相对 L2 `2.11e-6`、差异 552 个，均过探针阈值。**同作业内** Myciel共同聚合约4.313s，逐源约6.330s，逐行约14.550s；Hollywood约2.194/2.330/2.862s；Amazon、RoadNet 无收益。[完整报告](../../results/consumer-mask-c4-kernel-20261018/KERNEL_PILOT_V2.md)，原始数据 `runs/consumer_mask_c4_kernel_v2_20261018-01/`。

v1、v2 CPU 分配不同，绝对时间不能跨节点对比。此处仅独立稀疏阶段，无 AMX 投影、两层输出或独享复核；Myciel计划2.355GB、构建约30.24s。下一门槛是独享单插槽32核配对复核，然后集成 canonical FULL Identity 的128→128→128 两层路径。禁用 NUMA interleave，论文性能仍 UNVERIFIED。

---

