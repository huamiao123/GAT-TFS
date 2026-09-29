# GAT 实现问题记录

## 2026-09-28：AVX-512 Online 首轮数值异常

症状：初始优化构建中，AVX-512 Online 相对 reference 的误差高于阈值，running-max update 与 rescale 分布也和 FP32 不一致。证据：作业 10782447、10782456、10782466。最初 PASS 判定只覆盖 FP32，已改为同时检查 FP32 和 AVX-512。

定位：逐块对照发现 AVX score 和 exp 的表达式与 scalar 一致；加入检查后可正确输出。最终将 online_softmax_update 保留为 noinline 独立函数，避免当前编译设置下跨层内联产生异常代码行为。作业 10782477 起无逐块调试检查仍通过；10782665 的预热和三次重复也通过。尚未确定编译器级根因，不能声称已证明具体 compiler defect。今后改动 AVX 实现或编译参数后，必须重跑三层逐层及端到端数值对照。

## 2026-09-28：性能计时器开销和阶段字段

每 block 调用计时器会显著抬高 Online 层总时间，不能直接用 profile 构建与 reference 做速度结论。增加同源无细分计时 speed 构建，并用预热加 3 次重复。10782505 的 speed 构建仍有部分阶段字段使用真实时钟与禁用时钟混合，产生负值；10782601 修复。正式细分数据读取 benchmark.log，速度中位数读取 10782665 的 benchmark_speed_rep1..3.log。
