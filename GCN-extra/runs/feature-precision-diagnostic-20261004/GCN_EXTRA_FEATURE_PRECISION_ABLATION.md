# GCN_EXTRA Feature Precision × Fusion Granularity Ablation


**BF16/FP32 时间比 > 1 表示 FP32 更快；< 1 表示 BF16 更快。** 本报告仅使用此 STUDY 内同图、同进程、同节点、同协议的数据。


## 来源、边界和执行状态


- 冻结源码：`8aeef1613fdeaeb929379874c4152eccde620f74`；锚点核验：`True`。

- Study：`feature-precision-ablation-20261004-214015`；完成真实图记录 5 组；失败/未完成 0 组；smoke 3 组。

- 当前工作树：`bdcf9b0c75ea657be451abcc782b46bd2fabc4ba`。编译器、flags、dirty 状态、源码/二进制哈希均保留于 [manifest.json](../manifest.json)。

- 随机种子 `12345`；模型计算形状 `[128, 128, 128]`；随机 H/W 推理计算流，未接入训练模型分类。

- 编译参数：`-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel`。

- 本轮由用户授权共享 `intel` 节点，32 物理核、单 socket；默认 NUMA、无显式 interleave、无新 MKL override。旧独享结果不作分母。实际分配见每 RUN 的 affinity.json/CPU/NUMA 证据。

- Prepared E2E 排除初始 H 准备，包含本路径的层间表示转换；native FP32 保留 H1 FP32。单层、matched-input 和 preparation-inclusive 是不同边界。

- 四条预取始终为 offsets 0/64/128/192：两种输入都请求前 256B；BF16 覆盖整行，FP32 仅覆盖半行。没有暗中添加八条预取。

- native H0 FP32 的 serial first-touch 与 H0 BF16 的 parallel conversion、H1 的 scatter/conversion 可能形成不同页面分布。默认 NUMA 不证明实际页面放置相同；numa_maps 与 buffer 地址保留。

- 同图重跑作为独立 run 保留，不跨节点合并，不选择最快 run。数值失败不会被静默删除；原始负结果与 gate failures 见 raw CSV、record_sets.csv 和每图日志。


## 表 1：完整两层 prepared E2E


单位 ms，主值为 10 次的中位数。比值为两个中位数相除；paired ratio/IQR 另见 paired_comparisons.csv。

| 图 | RUN | N | E | B64 BF16 | B64 FP32 | B64 BF/FP | FULL BF16 | FULL FP32 | FULL BF/FP | 原 TFS | 源码 MKL |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | 393,215 | 903,194,710 | 1124.760 | 2062.020 | 0.545 | 2468.570 | 3722.925 | 0.663 | 2215.486 | 4799.347 |
| ogbn-products | diagnostic-10867492 | 2,449,029 | 123,718,152 | 286.844 | 453.236 | 0.633 | 285.502 | 440.729 | 0.648 | 339.964 | 1350.686 |
| reddit | diagnostic-10867492 | 232,965 | 114,615,892 | 123.210 | 398.026 | 0.310 | 135.841 | 463.460 | 0.293 | 307.728 | 440.791 |
| roadNet-CA | diagnostic-10867492 | 1,971,281 | 5,533,214 | 39.955 | 37.695 | 1.060 | 40.160 | 37.879 | 1.060 | 37.375 | 71.260 |
| wiki-Talk | diagnostic-10867492 | 2,394,385 | 5,021,410 | 49.272 | 52.211 | 0.944 | 47.128 | 49.255 | 0.957 | 108.271 | 95.681 |



### 单层与 matched-input / preparation-inclusive 补充


matched_kernel 的 FP32 值由 BF16 无损展开，准备过程不计入 kernel；这是表示与加载/解码控制，仍需核对两份 buffer 的页分布。

| 图 | RUN | B | 边界 | BF16 ms | FP32 ms | BF/FP | paired ratio IQR | 稳定方向 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | B64 | kernel | 551.798 | 1019.672 | 0.541 | [0.529, 0.553] | True |
| mycielskian19 | diagnostic-10867492 | B64 | matched_kernel | 567.044 | 1019.525 | 0.556 | [0.545, 0.563] | True |
| mycielskian19 | diagnostic-10867492 | B64 | preparation_inclusive | 1114.767 | 2032.152 | 0.549 | [0.541, 0.559] | False |
| mycielskian19 | diagnostic-10867492 | FULL | kernel | 1238.754 | 1890.766 | 0.655 | [0.644, 0.667] | True |
| mycielskian19 | diagnostic-10867492 | FULL | matched_kernel | 1241.729 | 1813.790 | 0.685 | [0.679, 0.686] | True |
| mycielskian19 | diagnostic-10867492 | FULL | preparation_inclusive | 2474.726 | 3732.643 | 0.663 | [0.663, 0.676] | False |
| ogbn-products | diagnostic-10867492 | B64 | kernel | 141.304 | 252.104 | 0.561 | [0.560, 0.605] | True |
| ogbn-products | diagnostic-10867492 | B64 | matched_kernel | 144.597 | 244.982 | 0.590 | [0.587, 0.599] | True |
| ogbn-products | diagnostic-10867492 | B64 | preparation_inclusive | 312.878 | 465.255 | 0.672 | [0.668, 0.680] | False |
| ogbn-products | diagnostic-10867492 | FULL | kernel | 139.288 | 243.701 | 0.572 | [0.570, 0.612] | True |
| ogbn-products | diagnostic-10867492 | FULL | matched_kernel | 140.410 | 240.606 | 0.584 | [0.579, 0.585] | True |
| ogbn-products | diagnostic-10867492 | FULL | preparation_inclusive | 313.125 | 461.497 | 0.678 | [0.673, 0.679] | False |
| reddit | diagnostic-10867492 | B64 | kernel | 60.567 | 193.731 | 0.313 | [0.306, 0.324] | True |
| reddit | diagnostic-10867492 | B64 | matched_kernel | 60.995 | 189.867 | 0.321 | [0.313, 0.335] | True |
| reddit | diagnostic-10867492 | B64 | preparation_inclusive | 131.015 | 402.499 | 0.326 | [0.323, 0.327] | False |
| reddit | diagnostic-10867492 | FULL | kernel | 67.784 | 223.439 | 0.303 | [0.296, 0.306] | True |
| reddit | diagnostic-10867492 | FULL | matched_kernel | 67.501 | 221.804 | 0.304 | [0.301, 0.316] | True |
| reddit | diagnostic-10867492 | FULL | preparation_inclusive | 138.502 | 470.731 | 0.294 | [0.293, 0.297] | False |
| roadNet-CA | diagnostic-10867492 | B64 | kernel | 16.202 | 17.107 | 0.947 | [0.942, 0.951] | True |
| roadNet-CA | diagnostic-10867492 | B64 | matched_kernel | 16.451 | 17.234 | 0.955 | [0.951, 0.960] | True |
| roadNet-CA | diagnostic-10867492 | B64 | preparation_inclusive | 55.093 | 48.951 | 1.125 | [1.121, 1.134] | False |
| roadNet-CA | diagnostic-10867492 | FULL | kernel | 16.333 | 17.114 | 0.954 | [0.953, 0.956] | False |
| roadNet-CA | diagnostic-10867492 | FULL | matched_kernel | 16.451 | 17.246 | 0.954 | [0.947, 0.957] | True |
| roadNet-CA | diagnostic-10867492 | FULL | preparation_inclusive | 55.670 | 48.730 | 1.142 | [1.119, 1.147] | False |
| wiki-Talk | diagnostic-10867492 | B64 | kernel | 20.370 | 25.655 | 0.794 | [0.778, 0.850] | True |
| wiki-Talk | diagnostic-10867492 | B64 | matched_kernel | 21.224 | 25.351 | 0.837 | [0.813, 0.855] | True |
| wiki-Talk | diagnostic-10867492 | B64 | preparation_inclusive | 67.686 | 63.816 | 1.061 | [1.061, 1.078] | False |
| wiki-Talk | diagnostic-10867492 | FULL | kernel | 19.453 | 24.547 | 0.792 | [0.799, 0.819] | False |
| wiki-Talk | diagnostic-10867492 | FULL | matched_kernel | 20.138 | 23.750 | 0.848 | [0.807, 0.911] | True |
| wiki-Talk | diagnostic-10867492 | FULL | preparation_inclusive | 65.534 | 62.019 | 1.057 | [1.050, 1.076] | False |



## 表 2：未插桩阶段计时


阶段来自独立 5 次 stage 测量，各列为中位数；conversion% 为每次 conversion/total 的中位数，阶段中位数之和不必恰好等于 total 中位数。

| 图 | RUN | 方法 | L1 ms | ReLU ms | 层间 conversion ms | L2 ms | total ms | conversion % |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | 563.137 | 0.721 | 0.881 | 567.516 | 1131.998 | 0.082 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | 1024.822 | 0.705 | 0.000 | 1002.359 | 2032.369 | 0.000 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | 1257.518 | 0.698 | 0.880 | 1208.870 | 2468.566 | 0.036 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | 1841.254 | 0.699 | 0.000 | 1785.154 | 3652.705 | 0.000 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | 144.107 | 4.294 | 4.945 | 142.289 | 296.482 | 1.654 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | 253.222 | 4.301 | 0.000 | 201.322 | 454.840 | 0.000 |
| ogbn-products | diagnostic-10867492 | full_bf16 | 141.141 | 4.294 | 4.931 | 138.694 | 290.716 | 1.694 |
| ogbn-products | diagnostic-10867492 | full_fp32 | 247.626 | 4.302 | 0.000 | 196.498 | 449.281 | 0.000 |
| reddit | diagnostic-10867492 | b64_bf16 | 63.276 | 0.458 | 1.513 | 61.896 | 127.256 | 1.192 |
| reddit | diagnostic-10867492 | b64_fp32 | 194.418 | 0.463 | 0.000 | 204.365 | 397.899 | 0.000 |
| reddit | diagnostic-10867492 | full_bf16 | 68.277 | 0.453 | 1.542 | 66.304 | 137.599 | 1.146 |
| reddit | diagnostic-10867492 | full_fp32 | 231.119 | 0.447 | 0.000 | 237.843 | 469.408 | 0.000 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | 16.491 | 6.073 | 4.046 | 13.370 | 39.844 | 10.216 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | 17.573 | 6.020 | 0.000 | 14.809 | 38.504 | 0.000 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | 16.473 | 5.778 | 4.064 | 13.234 | 39.821 | 10.240 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | 17.402 | 6.089 | 0.000 | 14.482 | 37.756 | 0.000 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | 21.145 | 3.516 | 4.629 | 20.160 | 49.762 | 9.555 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | 25.782 | 3.593 | 0.000 | 22.755 | 52.273 | 0.000 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | 20.610 | 3.502 | 4.633 | 18.240 | 47.132 | 10.083 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | 25.318 | 3.655 | 0.000 | 22.078 | 49.638 | 0.000 |



## 表 3：实际代码的逻辑请求与运算计数（每层）


load 指令模型为每边 8 条：BF16 为 256-bit load，FP32 为 512-bit load；BF16 另有 8 次 zero-extend 和 8 次 shift，FP32 没有这些 decode。不是 retired 指令或实测 DRAM 字节；编译器可能使用内存操作数融合。C spill 包括最后必要 store。完整所有计数列见 logical_operations.csv。

| 图 | RUN | 方法 | 层 | H 请求 B | load vectors | zext vectors | shift vectors | FP32 add vectors | 投影 scopes | TDP | C store B | C reload B |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | 1 | 231,217,845,760 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 897,060 | 28,705,920 | 7,348,715,520 | 7,147,388,928 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | 2 | 231,217,845,760 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 897,060 | 28,705,920 | 7,348,715,520 | 7,147,388,928 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | 1 | 462,435,691,520 | 7,225,557,680 | 0 | 0 | 7,225,557,680 | 897,060 | 28,705,920 | 7,348,715,520 | 7,147,388,928 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | 2 | 462,435,691,520 | 7,225,557,680 | 0 | 0 | 7,225,557,680 | 897,060 | 28,705,920 | 7,348,715,520 | 7,147,388,928 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | 1 | 231,217,845,760 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 24,576 | 786,432 | 201,326,592 | 0 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | 2 | 231,217,845,760 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 7,225,557,680 | 24,576 | 786,432 | 201,326,592 | 0 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | 1 | 462,435,691,520 | 7,225,557,680 | 0 | 0 | 7,225,557,680 | 24,576 | 786,432 | 201,326,592 | 0 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | 2 | 462,435,691,520 | 7,225,557,680 | 0 | 0 | 7,225,557,680 | 24,576 | 786,432 | 201,326,592 | 0 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | 1 | 31,671,846,912 | 989,745,216 | 989,745,216 | 989,745,216 | 989,745,216 | 218,018 | 6,976,576 | 1,786,003,456 | 556,883,968 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | 2 | 31,671,846,912 | 989,745,216 | 989,745,216 | 989,745,216 | 989,745,216 | 218,018 | 6,976,576 | 1,786,003,456 | 556,883,968 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | 1 | 63,343,693,824 | 989,745,216 | 0 | 0 | 989,745,216 | 218,018 | 6,976,576 | 1,786,003,456 | 556,883,968 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | 2 | 63,343,693,824 | 989,745,216 | 0 | 0 | 989,745,216 | 218,018 | 6,976,576 | 1,786,003,456 | 556,883,968 |
| ogbn-products | diagnostic-10867492 | full_bf16 | 1 | 31,671,846,912 | 989,745,216 | 989,745,216 | 989,745,216 | 989,745,216 | 150,039 | 4,801,248 | 1,229,119,488 | 0 |
| ogbn-products | diagnostic-10867492 | full_bf16 | 2 | 31,671,846,912 | 989,745,216 | 989,745,216 | 989,745,216 | 989,745,216 | 150,039 | 4,801,248 | 1,229,119,488 | 0 |
| ogbn-products | diagnostic-10867492 | full_fp32 | 1 | 63,343,693,824 | 989,745,216 | 0 | 0 | 989,745,216 | 150,039 | 4,801,248 | 1,229,119,488 | 0 |
| ogbn-products | diagnostic-10867492 | full_fp32 | 2 | 63,343,693,824 | 989,745,216 | 0 | 0 | 989,745,216 | 150,039 | 4,801,248 | 1,229,119,488 | 0 |
| reddit | diagnostic-10867492 | b64_bf16 | 1 | 29,341,668,352 | 916,927,136 | 916,927,136 | 916,927,136 | 916,927,136 | 119,990 | 3,839,680 | 982,958,080 | 863,674,368 |
| reddit | diagnostic-10867492 | b64_bf16 | 2 | 29,341,668,352 | 916,927,136 | 916,927,136 | 916,927,136 | 916,927,136 | 119,990 | 3,839,680 | 982,958,080 | 863,674,368 |
| reddit | diagnostic-10867492 | b64_fp32 | 1 | 58,683,336,704 | 916,927,136 | 0 | 0 | 916,927,136 | 119,990 | 3,839,680 | 982,958,080 | 863,674,368 |
| reddit | diagnostic-10867492 | b64_fp32 | 2 | 58,683,336,704 | 916,927,136 | 0 | 0 | 916,927,136 | 119,990 | 3,839,680 | 982,958,080 | 863,674,368 |
| reddit | diagnostic-10867492 | full_bf16 | 1 | 29,341,668,352 | 916,927,136 | 916,927,136 | 916,927,136 | 916,927,136 | 14,561 | 465,952 | 119,283,712 | 0 |
| reddit | diagnostic-10867492 | full_bf16 | 2 | 29,341,668,352 | 916,927,136 | 916,927,136 | 916,927,136 | 916,927,136 | 14,561 | 465,952 | 119,283,712 | 0 |
| reddit | diagnostic-10867492 | full_fp32 | 1 | 58,683,336,704 | 916,927,136 | 0 | 0 | 916,927,136 | 14,561 | 465,952 | 119,283,712 | 0 |
| reddit | diagnostic-10867492 | full_fp32 | 2 | 58,683,336,704 | 916,927,136 | 0 | 0 | 916,927,136 | 14,561 | 465,952 | 119,283,712 | 0 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | 1 | 1,416,502,784 | 44,265,712 | 44,265,712 | 44,265,712 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | 2 | 1,416,502,784 | 44,265,712 | 44,265,712 | 44,265,712 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | 1 | 2,833,005,568 | 44,265,712 | 0 | 0 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | 2 | 2,833,005,568 | 44,265,712 | 0 | 0 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | 1 | 1,416,502,784 | 44,265,712 | 44,265,712 | 44,265,712 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | 2 | 1,416,502,784 | 44,265,712 | 44,265,712 | 44,265,712 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | 1 | 2,833,005,568 | 44,265,712 | 0 | 0 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | 2 | 2,833,005,568 | 44,265,712 | 0 | 0 | 44,265,712 | 122,827 | 3,930,464 | 1,006,198,784 | 0 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | 1 | 1,285,480,960 | 40,171,280 | 40,171,280 | 40,171,280 | 40,171,280 | 15,131 | 484,192 | 123,953,152 | 48,365,568 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | 2 | 1,285,480,960 | 40,171,280 | 40,171,280 | 40,171,280 | 40,171,280 | 15,131 | 484,192 | 123,953,152 | 48,365,568 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | 1 | 2,570,961,920 | 40,171,280 | 0 | 0 | 40,171,280 | 15,131 | 484,192 | 123,953,152 | 48,365,568 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | 2 | 2,570,961,920 | 40,171,280 | 0 | 0 | 40,171,280 | 15,131 | 484,192 | 123,953,152 | 48,365,568 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | 1 | 1,285,480,960 | 40,171,280 | 40,171,280 | 40,171,280 | 40,171,280 | 9,227 | 295,264 | 75,587,584 | 0 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | 2 | 1,285,480,960 | 40,171,280 | 40,171,280 | 40,171,280 | 40,171,280 | 9,227 | 295,264 | 75,587,584 | 0 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | 1 | 2,570,961,920 | 40,171,280 | 0 | 0 | 40,171,280 | 9,227 | 295,264 | 75,587,584 | 0 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | 2 | 2,570,961,920 | 40,171,280 | 0 | 0 | 40,171,280 | 9,227 | 295,264 | 75,587,584 | 0 |



## 表 4：独立 PMU 诊断


只对 available 且 coverage_complete 的区域报告 raw；scaled 还要求 scaled_complete。逐 TID 先缩放再求和。N/A 表示不可用/不完整，不能由逻辑字节补造。cycles 是线程总 CPU cycles，AMX_BUSY 是 speculative arithmetic-busy cycles；均不是 wall cycles、TDP 次数或墙钟阶段占比。generic caches 不等于所有层 cache misses，load-stall 事件不是完整 TopDown，共享 uncore 带宽未归因。

| 图 | RUN | 方法 | 事件组 | raw可用/总 | scaled可用/总 | scaled中位数 | 最大启停跨度/区域 % | 错误 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | basic | 3/3 | 3/3 | amx_busy=923,926,656; cache_misses=4,002,632,626; cache_references=5,483,477,497; cycles=98,833,629,576; instructions=88,732,251,062 | 0.096 | none |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | cache | 3/3 | 3/3 | cycles=98,261,145,009; instructions=87,530,703,916; l1_miss=749,524,637; l2_miss=241,188,742; l3_miss=181,344,686 | 0.094 | none |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | tlb | 3/3 | 3/3 | cycles=99,510,253,671; instructions=89,249,921,251; stlb_hit=25,902,689; walk_active=39,688,433; walk_completed=99,451 | 0.087 | none |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | stall | 3/3 | 3/3 | bound_on_loads=51,646,953,706; cycles=98,288,307,250; instructions=87,583,567,334; stalls_l2_miss=21,086,214,547; stalls_l3_miss=1,899,248,113 | 0.089 | none |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | basic | 3/3 | 3/3 | amx_busy=927,689,244; cache_misses=6,225,921,167; cache_references=9,547,581,546; cycles=188,143,582,767; instructions=66,858,721,153 | 0.047 | none |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | cache | 3/3 | 3/3 | cycles=182,726,332,736; instructions=60,045,557,577; l1_miss=7,136,168,118; l2_miss=1,789,400,521; l3_miss=1,386,661,467 | 0.048 | none |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | tlb | 3/3 | 3/3 | cycles=188,103,398,168; instructions=66,509,681,997; stlb_hit=39,476,190; walk_active=75,217,039; walk_completed=125,190 | 0.049 | none |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | stall | 3/3 | 3/3 | bound_on_loads=140,927,789,218; cycles=188,655,721,470; instructions=66,294,271,300; stalls_l2_miss=134,841,007,526; stalls_l3_miss=40,753,553,615 | 0.048 | none |
| mycielskian19 | diagnostic-10867492 | full_bf16 | basic | 3/3 | 3/3 | amx_busy=25,311,848; cache_misses=9,459,567,196; cache_references=12,433,933,463; cycles=243,843,538,530; instructions=90,275,811,017 | 0.039 | none |
| mycielskian19 | diagnostic-10867492 | full_bf16 | cache | 3/3 | 3/3 | cycles=240,175,029,115; instructions=86,579,900,156; l1_miss=802,945,475; l2_miss=566,718,022; l3_miss=428,455,604 | 0.039 | none |
| mycielskian19 | diagnostic-10867492 | full_bf16 | tlb | 3/3 | 3/3 | cycles=241,070,450,348; instructions=86,986,663,258; stlb_hit=29,147,509; walk_active=69,233,299; walk_completed=104,559 | 0.039 | none |
| mycielskian19 | diagnostic-10867492 | full_bf16 | stall | 3/3 | 3/3 | bound_on_loads=186,442,605,672; cycles=239,461,647,335; instructions=85,506,940,578; stalls_l2_miss=97,105,166,488; stalls_l3_miss=4,970,211,070 | 0.040 | none |
| mycielskian19 | diagnostic-10867492 | full_fp32 | basic | 3/3 | 3/3 | amx_busy=25,514,812; cache_misses=12,312,192,951; cache_references=18,554,081,288; cycles=366,603,445,396; instructions=77,495,551,138 | 0.026 | none |
| mycielskian19 | diagnostic-10867492 | full_fp32 | cache | 3/3 | 3/3 | cycles=361,521,387,010; instructions=71,030,008,125; l1_miss=7,905,426,437; l2_miss=3,546,999,781; l3_miss=2,739,802,332 | 0.027 | none |
| mycielskian19 | diagnostic-10867492 | full_fp32 | tlb | 3/3 | 3/3 | cycles=357,121,315,961; instructions=64,241,153,262; stlb_hit=62,390,084; walk_active=92,712,346; walk_completed=126,808 | 0.028 | none |
| mycielskian19 | diagnostic-10867492 | full_fp32 | stall | 3/3 | 3/3 | bound_on_loads=305,878,972,003; cycles=366,244,548,652; instructions=75,594,828,301; stalls_l2_miss=303,321,726,870; stalls_l3_miss=87,448,467,182 | 0.026 | none |
| ogbn-products | diagnostic-10867492 | b64_bf16 | basic | 3/3 | 3/3 | amx_busy=224,150,480; cache_misses=573,419,585; cache_references=1,214,715,738; cycles=27,349,790,108; instructions=12,483,812,751 | 0.378 | none |
| ogbn-products | diagnostic-10867492 | b64_bf16 | cache | 3/3 | 3/3 | cycles=27,085,444,302; instructions=12,203,986,114; l1_miss=177,278,996; l2_miss=90,122,092; l3_miss=69,131,073 | 0.338 | none |
| ogbn-products | diagnostic-10867492 | b64_bf16 | tlb | 3/3 | 3/3 | cycles=27,344,801,859; instructions=12,456,675,747; stlb_hit=661,675,434; walk_active=58,300,430; walk_completed=375,195 | 0.332 | none |
| ogbn-products | diagnostic-10867492 | b64_bf16 | stall | 3/3 | 3/3 | bound_on_loads=18,106,891,353; cycles=27,458,977,860; instructions=12,486,739,063; stalls_l2_miss=14,134,634,274; stalls_l3_miss=11,545,334,743 | 0.334 | none |
| ogbn-products | diagnostic-10867492 | b64_fp32 | basic | 3/3 | 3/3 | amx_busy=224,275,964; cache_misses=1,047,045,726; cache_references=2,574,203,432; cycles=40,983,258,995; instructions=8,295,939,999 | 0.253 | none |
| ogbn-products | diagnostic-10867492 | b64_fp32 | cache | 3/3 | 3/3 | cycles=40,888,925,952; instructions=8,009,238,666; l1_miss=779,218,583; l2_miss=238,133,992; l3_miss=189,017,971 | 0.250 | none |
| ogbn-products | diagnostic-10867492 | b64_fp32 | tlb | 3/3 | 3/3 | cycles=41,033,808,949; instructions=8,057,123,308; stlb_hit=697,236,088; walk_active=635,917,897; walk_completed=1,904,178 | 0.242 | none |
| ogbn-products | diagnostic-10867492 | b64_fp32 | stall | 3/3 | 3/3 | bound_on_loads=33,956,104,220; cycles=41,713,649,132; instructions=8,916,864,750; stalls_l2_miss=33,266,077,050; stalls_l3_miss=29,060,434,488 | 0.243 | none |
| ogbn-products | diagnostic-10867492 | full_bf16 | basic | 3/3 | 3/3 | amx_busy=154,258,663; cache_misses=571,939,510; cache_references=1,211,088,560; cycles=27,300,617,669; instructions=12,317,002,508 | 0.336 | none |
| ogbn-products | diagnostic-10867492 | full_bf16 | cache | 3/3 | 3/3 | cycles=26,865,728,324; instructions=11,942,987,464; l1_miss=158,332,911; l2_miss=88,731,817; l3_miss=68,098,789 | 0.337 | none |
| ogbn-products | diagnostic-10867492 | full_bf16 | tlb | 3/3 | 3/3 | cycles=26,911,079,456; instructions=12,054,680,808; stlb_hit=706,995,292; walk_active=61,321,730; walk_completed=400,899 | 0.333 | none |
| ogbn-products | diagnostic-10867492 | full_bf16 | stall | 3/3 | 3/3 | bound_on_loads=18,215,061,325; cycles=27,060,017,926; instructions=12,114,208,721; stalls_l2_miss=14,441,434,736; stalls_l3_miss=11,797,622,244 | 0.340 | none |
| ogbn-products | diagnostic-10867492 | full_fp32 | basic | 3/3 | 3/3 | amx_busy=154,000,748; cache_misses=1,047,554,683; cache_references=2,566,742,856; cycles=40,807,832,686; instructions=8,457,908,761 | 0.248 | none |
| ogbn-products | diagnostic-10867492 | full_fp32 | cache | 3/3 | 3/3 | cycles=40,538,288,115; instructions=8,018,140,955; l1_miss=755,195,009; l2_miss=235,070,170; l3_miss=186,647,233 | 0.247 | none |
| ogbn-products | diagnostic-10867492 | full_fp32 | tlb | 3/3 | 3/3 | cycles=40,602,993,882; instructions=7,986,639,826; stlb_hit=715,397,089; walk_active=600,588,723; walk_completed=1,832,839 | 0.248 | none |
| ogbn-products | diagnostic-10867492 | full_fp32 | stall | 3/3 | 3/3 | bound_on_loads=33,936,477,813; cycles=40,693,037,255; instructions=7,895,746,755; stalls_l2_miss=33,360,921,186; stalls_l3_miss=29,126,695,772 | 0.244 | none |
| reddit | diagnostic-10867492 | b64_bf16 | basic | 3/3 | 3/3 | amx_busy=122,973,872; cache_misses=651,373,151; cache_references=884,484,307; cycles=11,735,044,657; instructions=10,858,092,399 | 0.864 | none |
| reddit | diagnostic-10867492 | b64_bf16 | cache | 3/3 | 3/3 | cycles=11,869,116,778; instructions=10,901,087,736; l1_miss=194,707,240; l2_miss=124,393,285; l3_miss=95,490,816 | 0.752 | none |
| reddit | diagnostic-10867492 | b64_bf16 | tlb | 3/3 | 3/3 | cycles=11,731,422,733; instructions=10,979,850,593; stlb_hit=33,843,774; walk_active=4,086,699; walk_completed=22,469 | 0.746 | none |
| reddit | diagnostic-10867492 | b64_bf16 | stall | 3/3 | 3/3 | bound_on_loads=5,093,490,029; cycles=11,937,668,014; instructions=10,917,742,323; stalls_l2_miss=2,903,423,567; stalls_l3_miss=422,079,870 | 0.733 | none |
| reddit | diagnostic-10867492 | b64_fp32 | basic | 3/3 | 3/3 | amx_busy=124,554,640; cache_misses=1,531,009,466; cache_references=2,263,715,492; cycles=37,213,415,801; instructions=7,991,796,553 | 0.234 | none |
| reddit | diagnostic-10867492 | b64_fp32 | cache | 3/3 | 3/3 | cycles=37,148,998,857; instructions=7,789,951,342; l1_miss=1,105,493,022; l2_miss=511,896,546; l3_miss=391,878,071 | 0.230 | none |
| reddit | diagnostic-10867492 | b64_fp32 | tlb | 3/3 | 3/3 | cycles=37,083,035,145; instructions=7,871,348,435; stlb_hit=45,257,093; walk_active=51,697,806; walk_completed=73,711 | 0.226 | none |
| reddit | diagnostic-10867492 | b64_fp32 | stall | 3/3 | 3/3 | bound_on_loads=31,107,314,946; cycles=36,892,521,042; instructions=7,583,020,885; stalls_l2_miss=30,602,773,601; stalls_l3_miss=8,508,599,864 | 0.237 | none |
| reddit | diagnostic-10867492 | full_bf16 | basic | 3/3 | 3/3 | amx_busy=14,915,600; cache_misses=713,000,904; cache_references=967,421,621; cycles=12,959,588,664; instructions=11,092,329,419 | 0.687 | none |
| reddit | diagnostic-10867492 | full_bf16 | cache | 3/3 | 3/3 | cycles=12,864,660,077; instructions=11,184,102,653; l1_miss=161,402,476; l2_miss=126,596,265; l3_miss=97,533,219 | 0.696 | none |
| reddit | diagnostic-10867492 | full_bf16 | tlb | 3/3 | 3/3 | cycles=12,728,053,684; instructions=10,925,925,302; stlb_hit=38,186,726; walk_active=3,862,295; walk_completed=22,706 | 0.698 | none |
| reddit | diagnostic-10867492 | full_bf16 | stall | 3/3 | 3/3 | bound_on_loads=6,137,275,435; cycles=13,125,024,991; instructions=11,196,961,129; stalls_l2_miss=3,907,134,316; stalls_l3_miss=1,082,498,120 | 0.697 | none |
| reddit | diagnostic-10867492 | full_fp32 | basic | 3/3 | 3/3 | amx_busy=15,036,408; cache_misses=1,786,329,996; cache_references=2,591,224,750; cycles=45,476,410,559; instructions=8,344,786,157 | 0.194 | none |
| reddit | diagnostic-10867492 | full_fp32 | cache | 3/3 | 3/3 | cycles=45,552,422,897; instructions=8,346,658,627; l1_miss=1,099,488,741; l2_miss=588,781,936; l3_miss=449,986,253 | 0.190 | none |
| reddit | diagnostic-10867492 | full_fp32 | tlb | 3/3 | 3/3 | cycles=45,377,833,940; instructions=8,508,836,954; stlb_hit=94,786,423; walk_active=61,733,705; walk_completed=88,808 | 0.191 | none |
| reddit | diagnostic-10867492 | full_fp32 | stall | 3/3 | 3/3 | bound_on_loads=39,071,145,273; cycles=45,357,730,193; instructions=8,185,887,379; stalls_l2_miss=38,734,200,583; stalls_l3_miss=9,329,957,811 | 0.194 | none |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | basic | 3/3 | 3/3 | amx_busy=125,941,792; cache_misses=24,896,472; cache_references=97,884,995; cycles=3,485,817,992; instructions=1,575,630,676 | 2.700 | none |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | cache | 3/3 | 3/3 | cycles=3,479,406,487; instructions=1,569,568,061; l1_miss=26,405,752; l2_miss=2,124,483; l3_miss=1,782,365 | 2.606 | none |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | tlb | 3/3 | 3/3 | cycles=3,488,131,068; instructions=1,594,724,276; stlb_hit=3,761,656; walk_active=2,331,655; walk_completed=20,681 | 2.585 | none |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | stall | 3/3 | 3/3 | bound_on_loads=1,818,444,786; cycles=3,484,028,630; instructions=1,569,523,342; stalls_l2_miss=1,336,116,410; stalls_l3_miss=1,130,017,511 | 2.557 | none |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | basic | 3/3 | 3/3 | amx_busy=126,073,922; cache_misses=36,310,641; cache_references=124,188,228; cycles=3,114,430,709; instructions=1,205,873,732 | 2.780 | none |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | cache | 3/3 | 3/3 | cycles=3,239,011,351; instructions=1,203,871,518; l1_miss=47,939,387; l2_miss=3,334,159; l3_miss=2,730,822 | 2.734 | none |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | tlb | 3/3 | 3/3 | cycles=3,233,168,965; instructions=1,201,900,768; stlb_hit=4,382,161; walk_active=5,201,441; walk_completed=33,255 | 2.743 | none |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | stall | 3/3 | 3/3 | bound_on_loads=1,754,550,192; cycles=3,204,005,234; instructions=1,199,794,476; stalls_l2_miss=1,389,355,497; stalls_l3_miss=1,233,338,100 | 2.738 | none |
| roadNet-CA | diagnostic-10867492 | full_bf16 | basic | 3/3 | 3/3 | amx_busy=125,939,504; cache_misses=24,898,423; cache_references=97,882,537; cycles=3,466,400,607; instructions=1,569,472,475 | 2.563 | none |
| roadNet-CA | diagnostic-10867492 | full_bf16 | cache | 3/3 | 3/3 | cycles=3,469,217,567; instructions=1,579,466,640; l1_miss=26,391,854; l2_miss=2,119,727; l3_miss=1,778,045 | 2.558 | none |
| roadNet-CA | diagnostic-10867492 | full_bf16 | tlb | 3/3 | 3/3 | cycles=3,531,760,687; instructions=1,620,310,934; stlb_hit=3,771,983; walk_active=2,405,968; walk_completed=21,445 | 2.568 | none |
| roadNet-CA | diagnostic-10867492 | full_bf16 | stall | 3/3 | 3/3 | bound_on_loads=1,839,246,757; cycles=3,513,722,448; instructions=1,575,559,081; stalls_l2_miss=1,354,177,421; stalls_l3_miss=1,134,292,422 | 2.543 | none |
| roadNet-CA | diagnostic-10867492 | full_fp32 | basic | 3/3 | 3/3 | amx_busy=126,083,321; cache_misses=36,238,848; cache_references=124,095,922; cycles=3,146,117,446; instructions=1,200,003,094 | 2.753 | none |
| roadNet-CA | diagnostic-10867492 | full_fp32 | cache | 3/3 | 3/3 | cycles=3,232,997,728; instructions=1,219,323,887; l1_miss=47,894,157; l2_miss=3,352,535; l3_miss=2,740,181 | 2.766 | none |
| roadNet-CA | diagnostic-10867492 | full_fp32 | tlb | 3/3 | 3/3 | cycles=3,200,369,304; instructions=1,195,699,032; stlb_hit=4,375,209; walk_active=5,091,130; walk_completed=33,447 | 2.759 | none |
| roadNet-CA | diagnostic-10867492 | full_fp32 | stall | 3/3 | 3/3 | bound_on_loads=1,759,137,372; cycles=3,222,668,142; instructions=1,205,813,762; stalls_l2_miss=1,390,628,240; stalls_l3_miss=1,232,699,519 | 2.769 | none |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | basic | 3/3 | 3/3 | amx_busy=15,522,560; cache_misses=39,449,202; cache_references=109,943,513; cycles=4,828,851,896; instructions=3,495,304,914 | 2.004 | none |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | cache | 3/3 | 3/3 | cycles=4,622,960,287; instructions=3,320,464,805; l1_miss=13,345,857; l2_miss=2,794,187; l3_miss=2,315,746 | 1.861 | none |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | tlb | 3/3 | 3/3 | cycles=4,625,455,769; instructions=3,277,055,895; stlb_hit=11,472,909; walk_active=5,186,521; walk_completed=68,406 | 1.846 | none |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | stall | 3/3 | 3/3 | bound_on_loads=1,320,963,452; cycles=4,689,657,001; instructions=3,384,649,972; stalls_l2_miss=967,861,452; stalls_l3_miss=889,412,459 | 1.836 | none |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | basic | 3/3 | 3/3 | amx_busy=15,534,740; cache_misses=59,174,654; cache_references=140,473,217; cycles=4,922,185,958; instructions=3,416,222,062 | 1.722 | none |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | cache | 3/3 | 3/3 | cycles=4,962,422,803; instructions=3,432,330,190; l1_miss=35,545,886; l2_miss=9,118,733; l3_miss=7,342,232 | 1.762 | none |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | tlb | 3/3 | 3/3 | cycles=5,001,294,847; instructions=3,476,461,141; stlb_hit=10,614,766; walk_active=16,621,859; walk_completed=108,611 | 1.727 | none |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | stall | 3/3 | 3/3 | bound_on_loads=1,437,005,796; cycles=4,938,398,135; instructions=3,411,415,572; stalls_l2_miss=1,236,393,082; stalls_l3_miss=1,096,700,358 | 1.724 | none |
| wiki-Talk | diagnostic-10867492 | full_bf16 | basic | 3/3 | 3/3 | amx_busy=9,455,828; cache_misses=40,467,995; cache_references=110,599,017; cycles=4,730,748,188; instructions=3,432,137,396 | 1.958 | none |
| wiki-Talk | diagnostic-10867492 | full_bf16 | cache | 3/3 | 3/3 | cycles=4,572,748,227; instructions=3,271,878,702; l1_miss=11,826,631; l2_miss=2,796,376; l3_miss=2,289,845 | 1.927 | none |
| wiki-Talk | diagnostic-10867492 | full_bf16 | tlb | 3/3 | 3/3 | cycles=4,676,191,303; instructions=3,380,095,131; stlb_hit=11,063,604; walk_active=5,140,872; walk_completed=65,408 | 1.928 | none |
| wiki-Talk | diagnostic-10867492 | full_bf16 | stall | 3/3 | 3/3 | bound_on_loads=1,336,867,456; cycles=4,585,178,934; instructions=3,285,933,398; stalls_l2_miss=983,818,894; stalls_l3_miss=909,043,683 | 1.869 | none |
| wiki-Talk | diagnostic-10867492 | full_fp32 | basic | 3/3 | 3/3 | amx_busy=9,465,484; cache_misses=60,131,040; cache_references=140,968,564; cycles=4,725,430,195; instructions=3,165,476,780 | 1.900 | none |
| wiki-Talk | diagnostic-10867492 | full_fp32 | cache | 3/3 | 3/3 | cycles=4,971,404,734; instructions=3,432,655,171; l1_miss=33,669,422; l2_miss=9,555,765; l3_miss=7,714,803 | 1.717 | none |
| wiki-Talk | diagnostic-10867492 | full_fp32 | tlb | 3/3 | 3/3 | cycles=4,828,469,111; instructions=3,299,762,560; stlb_hit=9,975,620; walk_active=16,654,489; walk_completed=104,707 | 1.786 | none |
| wiki-Talk | diagnostic-10867492 | full_fp32 | stall | 3/3 | 3/3 | bound_on_loads=1,457,933,861; cycles=4,698,087,777; instructions=3,172,199,025; stalls_l2_miss=1,281,144,742; stalls_l3_miss=1,126,005,212 | 1.867 | none |



## 表 5：正确性、边界和 checksum


原 BF16 controls 的既有 gates、各路径自己的数学 oracle、无损展开 EQ、诊断与未插桩 bitwise gate 分别报告。native BF16/FP32 差异只作描述，不意味着分类精度。所有字段原值及失败项见 raw_check/eq/diff/profile_gate.csv。

| 图 | RUN | 方法 | 类型 | 边界 | 参考 | gate类型 | gate | max_abs | relative L2 | finite | bitwise | pass | actual checksum | reference checksum |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.398437500 | 0.000003193 | 1 | 0 | 1 | 5aa5120862a4cee5 | 6878de5759112bcf |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 53.531250000 | 0.003072248 | 1 | 0 | 1 | 5aa5120862a4cee5 | 73446d066907ab53 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 6176.000000000 | 0.000002906 | 1 | 0 | 1 | a2d8e767f7abc22f | c4f0644a21ee03b0 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 5920.000000000 | 0.000031162 | 1 | 0 | 1 | a2d8e767f7abc22f | 6b0c106a9a579d57 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 1630688.000000000 | 0.005629241 | 1 | 0 | 1 | a2d8e767f7abc22f | f60be9dda10837ec |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 524288.000000000 | 0.000248576 | 1 | 0 | 1 | 41c007c3f7bc1847 | 104277bc5c484c12 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 2097152.000000000 | 0.006005708 | 1 | 0 | 1 | 41c007c3f7bc1847 | 17605bb0cd96b8fb |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 6012768.000000000 | 0.016537247 | 1 | 0 | 1 | 41c007c3f7bc1847 | 22af6d3bb5ba7ed7 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.344726562 | 0.000003165 | 1 | 0 | 1 | 6743b2bb3a3286a5 | 165ac2abbfcfb73c |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 8320.000000000 | 0.000002925 | 1 | 0 | 1 | 09dfaf893b65f40b | fcd6400448884f51 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 8320.000000000 | 0.000012770 | 1 | 0 | 1 | 09dfaf893b65f40b | f645908e533a46c3 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 524288.000000000 | 0.000144053 | 1 | 0 | 1 | c1ffabac8c31789d | bab8dbb4f8a69797 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.003906250 | 0.000000073 | 1 | 0 | 1 | c071d7f4d237c5c4 | 6d2bb44e168ba686 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 72.324218750 | 0.003259883 | 1 | 0 | 1 | c071d7f4d237c5c4 | 73446d066907ab53 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 32.000000000 | 0.000000082 | 1 | 0 | 1 | 8e526e0bb7764e71 | 3057c0c0da4c14d7 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 32768.000000000 | 0.000024924 | 1 | 0 | 1 | 8e526e0bb7764e71 | bcc8427592a52f1d |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 1773728.000000000 | 0.005889720 | 1 | 0 | 1 | 8e526e0bb7764e71 | f60be9dda10837ec |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 262144.000000000 | 0.000054569 | 1 | 0 | 1 | 0fdcc5e0796d6009 | 01b05a27b61962c4 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 2097152.000000000 | 0.006295386 | 1 | 0 | 1 | 0fdcc5e0796d6009 | 17605bb0cd96b8fb |
| mycielskian19 | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 6012768.000000000 | 0.016627137 | 1 | 0 | 1 | 0fdcc5e0796d6009 | 22af6d3bb5ba7ed7 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.003906250 | 0.000000073 | 1 | 0 | 1 | 337f68d07fb1e90d | 7adc7118f00de6b9 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 32.000000000 | 0.000000082 | 1 | 0 | 1 | 3e82283786a888a6 | d282a72a26212c9d |
| mycielskian19 | diagnostic-10867492 | full_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 32385.000000000 | 0.000012248 | 1 | 0 | 1 | 3e82283786a888a6 | d0631254a5c5d7c4 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 524288.000000000 | 0.000035524 | 1 | 0 | 1 | c72e1d737bd3dddf | 44cfa1a07d0418fc |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | EQ | matched_layer1_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 5aa5120862a4cee5 | 5aa5120862a4cee5 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | a2d8e767f7abc22f | a2d8e767f7abc22f |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_BF16 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 41c007c3f7bc1847 | 41c007c3f7bc1847 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | EQ | matched_layer1_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | c071d7f4d237c5c4 | c071d7f4d237c5c4 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 8e526e0bb7764e71 | 8e526e0bb7764e71 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_BF16 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 0fdcc5e0796d6009 | 0fdcc5e0796d6009 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 23.293945312 | 0.002722196 | 1 | 0 | 1 | 6743b2bb3a3286a5 | 5aa5120862a4cee5 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 1229472.000000000 | 0.003577372 | 1 | 0 | 1 | 09dfaf893b65f40b | a2d8e767f7abc22f |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 2097152.000000000 | 0.004455088 | 1 | 0 | 1 | c1ffabac8c31789d | 41c007c3f7bc1847 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 45.575439453 | 0.002942807 | 1 | 0 | 1 | 337f68d07fb1e90d | c071d7f4d237c5c4 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 1461392.000000000 | 0.004469347 | 1 | 0 | 1 | 3e82283786a888a6 | 8e526e0bb7764e71 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 2097152.000000000 | 0.005099197 | 1 | 0 | 1 | c72e1d737bd3dddf | 0fdcc5e0796d6009 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 5aa5120862a4cee5 | 5aa5120862a4cee5 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 41c007c3f7bc1847 | 41c007c3f7bc1847 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 6743b2bb3a3286a5 | 6743b2bb3a3286a5 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | c1ffabac8c31789d | c1ffabac8c31789d |
| mycielskian19 | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | c071d7f4d237c5c4 | c071d7f4d237c5c4 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 0fdcc5e0796d6009 | 0fdcc5e0796d6009 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 337f68d07fb1e90d | 337f68d07fb1e90d |
| mycielskian19 | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | c72e1d737bd3dddf | c72e1d737bd3dddf |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.011230469 | 0.000000216 | 1 | 0 | 1 | dce19cadcce983eb | 8b7c9321d4b52d4e |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 7.428222656 | 0.003288266 | 1 | 0 | 1 | dce19cadcce983eb | 281c679d172acfb2 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 31.000000000 | 0.000000616 | 1 | 0 | 1 | 1ead2d6e633fb12c | 910f9c5149aa5473 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 63.994140625 | 0.000021609 | 1 | 0 | 1 | 1ead2d6e633fb12c | 3369818c75c67c36 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 35613.000000000 | 0.005714741 | 1 | 0 | 1 | 1ead2d6e633fb12c | 1ab0b3a71a5fedcc |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 4096.000000000 | 0.000094179 | 1 | 0 | 1 | 9313093e457ffbd9 | 08baf30d6997285b |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 32768.000000000 | 0.006096755 | 1 | 0 | 1 | 9313093e457ffbd9 | d3eb831a71fa4eba |
| ogbn-products | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 120561.500000000 | 0.018274687 | 1 | 0 | 1 | 9313093e457ffbd9 | 8428024dfd3c62ba |
| ogbn-products | diagnostic-10867492 | b64_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.005737305 | 0.000000217 | 1 | 0 | 1 | b9971d8604fbb15d | fa61901a21b1cd34 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 29.500000000 | 0.000000622 | 1 | 0 | 1 | 511e059c3e99c530 | a688202e5241fbe3 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 126.500000000 | 0.000014408 | 1 | 0 | 1 | 511e059c3e99c530 | 891d74a381d6aa65 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 4096.000000000 | 0.000072317 | 1 | 0 | 1 | d7aee77b237901d3 | 132accf30778b341 |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000244141 | 0.000000077 | 1 | 0 | 1 | 22d40bb0be038794 | 8e13b7fc871c5087 |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 6.762207031 | 0.003294578 | 1 | 0 | 1 | 22d40bb0be038794 | 281c679d172acfb2 |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.500000000 | 0.000000056 | 1 | 0 | 1 | af15c2141ce07b7f | 2ba3f3a3c611c196 |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 128.003906250 | 0.000015185 | 1 | 0 | 1 | af15c2141ce07b7f | d24198119fbd34b4 |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 35290.500000000 | 0.005883072 | 1 | 0 | 1 | af15c2141ce07b7f | 1ab0b3a71a5fedcc |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 2048.000000000 | 0.000043399 | 1 | 0 | 1 | ffa66c1420fc5eeb | 16193e66f5a5d77b |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 32768.000000000 | 0.006284023 | 1 | 0 | 1 | ffa66c1420fc5eeb | d3eb831a71fa4eba |
| ogbn-products | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 120561.500000000 | 0.018335771 | 1 | 0 | 1 | ffa66c1420fc5eeb | 8428024dfd3c62ba |
| ogbn-products | diagnostic-10867492 | full_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000366211 | 0.000000078 | 1 | 0 | 1 | b1a432c1c2bab744 | 7dbb64da2e33ba34 |
| ogbn-products | diagnostic-10867492 | full_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.500000000 | 0.000000056 | 1 | 0 | 1 | d9284c06b7577599 | d20fe90fbf32d551 |
| ogbn-products | diagnostic-10867492 | full_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 126.500000000 | 0.000011447 | 1 | 0 | 1 | d9284c06b7577599 | 402293a107c535e3 |
| ogbn-products | diagnostic-10867492 | full_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 1024.000000000 | 0.000031690 | 1 | 0 | 1 | 593f18f504f2897b | d1a06219b4da4b51 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | EQ | matched_layer1_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | dce19cadcce983eb | dce19cadcce983eb |
| ogbn-products | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1ead2d6e633fb12c | 1ead2d6e633fb12c |
| ogbn-products | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_BF16 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 9313093e457ffbd9 | 9313093e457ffbd9 |
| ogbn-products | diagnostic-10867492 | full_fp32 | EQ | matched_layer1_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 22d40bb0be038794 | 22d40bb0be038794 |
| ogbn-products | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | af15c2141ce07b7f | af15c2141ce07b7f |
| ogbn-products | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_BF16 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | ffa66c1420fc5eeb | ffa66c1420fc5eeb |
| ogbn-products | diagnostic-10867492 | b64_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 5.504104614 | 0.003823702 | 1 | 0 | 1 | b9971d8604fbb15d | dce19cadcce983eb |
| ogbn-products | diagnostic-10867492 | b64_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 32432.000000000 | 0.005247394 | 1 | 0 | 1 | 511e059c3e99c530 | 1ead2d6e633fb12c |
| ogbn-products | diagnostic-10867492 | b64_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 32768.000000000 | 0.005722286 | 1 | 0 | 1 | d7aee77b237901d3 | 9313093e457ffbd9 |
| ogbn-products | diagnostic-10867492 | full_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 6.350585938 | 0.003830468 | 1 | 0 | 1 | b1a432c1c2bab744 | 22d40bb0be038794 |
| ogbn-products | diagnostic-10867492 | full_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 40164.000000000 | 0.005554764 | 1 | 0 | 1 | d9284c06b7577599 | af15c2141ce07b7f |
| ogbn-products | diagnostic-10867492 | full_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 32768.000000000 | 0.006025332 | 1 | 0 | 1 | 593f18f504f2897b | ffa66c1420fc5eeb |
| ogbn-products | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | dce19cadcce983eb | dce19cadcce983eb |
| ogbn-products | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 9313093e457ffbd9 | 9313093e457ffbd9 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | b9971d8604fbb15d | b9971d8604fbb15d |
| ogbn-products | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | d7aee77b237901d3 | d7aee77b237901d3 |
| ogbn-products | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 22d40bb0be038794 | 22d40bb0be038794 |
| ogbn-products | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | ffa66c1420fc5eeb | ffa66c1420fc5eeb |
| ogbn-products | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | b1a432c1c2bab744 | b1a432c1c2bab744 |
| ogbn-products | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 593f18f504f2897b | 593f18f504f2897b |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.012695312 | 0.000000668 | 1 | 0 | 1 | 09647ce158037e02 | abfd534d15a9adcd |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 8.901123047 | 0.003248006 | 1 | 0 | 1 | 09647ce158037e02 | 2df40fdb3b101f00 |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 202.000000000 | 0.000001265 | 1 | 0 | 1 | 18a55e3d416269c4 | d7b6693d7ac71e9d |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 229.375000000 | 0.000009063 | 1 | 0 | 1 | 18a55e3d416269c4 | bdcadf72ca6d7507 |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 184064.000000000 | 0.005605371 | 1 | 0 | 1 | 18a55e3d416269c4 | 39e0d278a6da77da |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 65536.000000000 | 0.000135195 | 1 | 0 | 1 | 3bc695c8144fdbd0 | e8ac8c9007ce6ba8 |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 262144.000000000 | 0.005980695 | 1 | 0 | 1 | 3bc695c8144fdbd0 | a07a3580ddbd51b3 |
| reddit | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 586268.000000000 | 0.017462511 | 1 | 0 | 1 | 3bc695c8144fdbd0 | 124ffecf035c0b2d |
| reddit | diagnostic-10867492 | b64_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.010742188 | 0.000000664 | 1 | 0 | 1 | b458f81b7c47e977 | 8880c3ce776b5d29 |
| reddit | diagnostic-10867492 | b64_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 184.000000000 | 0.000001259 | 1 | 0 | 1 | eef147f20e50dde9 | 6140f986a7d4d677 |
| reddit | diagnostic-10867492 | b64_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 256.000000000 | 0.000005699 | 1 | 0 | 1 | eef147f20e50dde9 | 541297023671c410 |
| reddit | diagnostic-10867492 | b64_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 65536.000000000 | 0.000105201 | 1 | 0 | 1 | 51cdf6890fc86011 | 93d02611b1b2792d |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000610352 | 0.000000078 | 1 | 0 | 1 | ac61641086dcb43e | f738a201abeb032c |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 8.810424805 | 0.003286143 | 1 | 0 | 1 | ac61641086dcb43e | 2df40fdb3b101f00 |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 4.000000000 | 0.000000065 | 1 | 0 | 1 | 5e359171a420b7b3 | 14a765a98d55d74d |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 2048.125000000 | 0.000013163 | 1 | 0 | 1 | 5e359171a420b7b3 | 3fa95fccf4a3b3ba |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 197034.000000000 | 0.005855896 | 1 | 0 | 1 | 5e359171a420b7b3 | 39e0d278a6da77da |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 32768.000000000 | 0.000039123 | 1 | 0 | 1 | 3ffad8586dcaa128 | cfb271806478ea06 |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 262144.000000000 | 0.006257883 | 1 | 0 | 1 | 3ffad8586dcaa128 | a07a3580ddbd51b3 |
| reddit | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 595806.000000000 | 0.017551764 | 1 | 0 | 1 | 3ffad8586dcaa128 | 124ffecf035c0b2d |
| reddit | diagnostic-10867492 | full_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000488281 | 0.000000078 | 1 | 0 | 1 | 52f2ce233869ad02 | a3e07ec4c7a3d018 |
| reddit | diagnostic-10867492 | full_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 5.000000000 | 0.000000065 | 1 | 0 | 1 | eeca1796bc89c0c9 | b7b4e9633126ccdf |
| reddit | diagnostic-10867492 | full_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 2047.875000000 | 0.000009308 | 1 | 0 | 1 | eeca1796bc89c0c9 | b45b8626cd2557a1 |
| reddit | diagnostic-10867492 | full_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 32768.000000000 | 0.000031992 | 1 | 0 | 1 | d8acbadf0ed56407 | 3c980d393c197323 |
| reddit | diagnostic-10867492 | b64_fp32 | EQ | matched_layer1_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 09647ce158037e02 | 09647ce158037e02 |
| reddit | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 18a55e3d416269c4 | 18a55e3d416269c4 |
| reddit | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_BF16 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 3bc695c8144fdbd0 | 3bc695c8144fdbd0 |
| reddit | diagnostic-10867492 | full_fp32 | EQ | matched_layer1_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | ac61641086dcb43e | ac61641086dcb43e |
| reddit | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 5e359171a420b7b3 | 5e359171a420b7b3 |
| reddit | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_BF16 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 3ffad8586dcaa128 | 3ffad8586dcaa128 |
| reddit | diagnostic-10867492 | b64_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 7.580200195 | 0.003603248 | 1 | 0 | 1 | b458f81b7c47e977 | 09647ce158037e02 |
| reddit | diagnostic-10867492 | b64_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 144706.000000000 | 0.004463031 | 1 | 0 | 1 | eef147f20e50dde9 | 18a55e3d416269c4 |
| reddit | diagnostic-10867492 | b64_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 131072.000000000 | 0.005089073 | 1 | 0 | 1 | 51cdf6890fc86011 | 3bc695c8144fdbd0 |
| reddit | diagnostic-10867492 | full_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 7.748962402 | 0.003635337 | 1 | 0 | 1 | 52f2ce233869ad02 | ac61641086dcb43e |
| reddit | diagnostic-10867492 | full_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 181924.000000000 | 0.005064571 | 1 | 0 | 1 | eeca1796bc89c0c9 | 5e359171a420b7b3 |
| reddit | diagnostic-10867492 | full_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 262144.000000000 | 0.005610682 | 1 | 0 | 1 | d8acbadf0ed56407 | 3ffad8586dcaa128 |
| reddit | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 09647ce158037e02 | 09647ce158037e02 |
| reddit | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 3bc695c8144fdbd0 | 3bc695c8144fdbd0 |
| reddit | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | b458f81b7c47e977 | b458f81b7c47e977 |
| reddit | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 51cdf6890fc86011 | 51cdf6890fc86011 |
| reddit | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | ac61641086dcb43e | ac61641086dcb43e |
| reddit | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 3ffad8586dcaa128 | 3ffad8586dcaa128 |
| reddit | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 52f2ce233869ad02 | 52f2ce233869ad02 |
| reddit | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | d8acbadf0ed56407 | d8acbadf0ed56407 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000011444 | 0.000000064 | 1 | 0 | 1 | 1fa5d37365db6e99 | ed45dcfd48913d19 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 0.146816254 | 0.002760771 | 1 | 0 | 1 | 1fa5d37365db6e99 | 479b476ac70bf536 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000183105 | 0.000000070 | 1 | 0 | 1 | 179600f238caeca0 | 2f5ed290a33b751d |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.250030518 | 0.000014046 | 1 | 0 | 1 | 179600f238caeca0 | 688ad10ea0379b68 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 4.624084473 | 0.005080335 | 1 | 0 | 1 | 179600f238caeca0 | 39e5f9a521fedb94 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 2.000000000 | 0.000037559 | 1 | 0 | 1 | 1ed96900c55712af | 198316fca4e83434 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 8.000000000 | 0.005617078 | 1 | 0 | 1 | 1ed96900c55712af | 7329b36c933e999c |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 16.631530762 | 0.017508345 | 1 | 0 | 1 | 1ed96900c55712af | 942b3831dfe5770f |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000011444 | 0.000000078 | 1 | 0 | 1 | 80ff88ced8cacdc4 | 917bb1e733dd012a |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000183105 | 0.000000070 | 1 | 0 | 1 | 342048974ed89b37 | 5b54abb8bca2512f |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.250015259 | 0.000015047 | 1 | 0 | 1 | 342048974ed89b37 | 3af2dd93398233b6 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 2.000000000 | 0.000039880 | 1 | 0 | 1 | 7a1550a97f571521 | eab8985aab231ed7 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000011444 | 0.000000064 | 1 | 0 | 1 | 1fa5d37365db6e99 | ed45dcfd48913d19 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 0.146816254 | 0.002760771 | 1 | 0 | 1 | 1fa5d37365db6e99 | 479b476ac70bf536 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000183105 | 0.000000070 | 1 | 0 | 1 | 179600f238caeca0 | 2f5ed290a33b751d |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.250030518 | 0.000014046 | 1 | 0 | 1 | 179600f238caeca0 | 688ad10ea0379b68 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 4.624084473 | 0.005080335 | 1 | 0 | 1 | 179600f238caeca0 | 39e5f9a521fedb94 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 2.000000000 | 0.000037559 | 1 | 0 | 1 | 1ed96900c55712af | 198316fca4e83434 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 8.000000000 | 0.005617078 | 1 | 0 | 1 | 1ed96900c55712af | 7329b36c933e999c |
| roadNet-CA | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 16.631530762 | 0.017508345 | 1 | 0 | 1 | 1ed96900c55712af | 942b3831dfe5770f |
| roadNet-CA | diagnostic-10867492 | full_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000011444 | 0.000000078 | 1 | 0 | 1 | 80ff88ced8cacdc4 | 917bb1e733dd012a |
| roadNet-CA | diagnostic-10867492 | full_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000183105 | 0.000000070 | 1 | 0 | 1 | 342048974ed89b37 | 5b54abb8bca2512f |
| roadNet-CA | diagnostic-10867492 | full_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.250015259 | 0.000015047 | 1 | 0 | 1 | 342048974ed89b37 | 3af2dd93398233b6 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 2.000000000 | 0.000039880 | 1 | 0 | 1 | 7a1550a97f571521 | eab8985aab231ed7 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | EQ | matched_layer1_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1fa5d37365db6e99 | 1fa5d37365db6e99 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 179600f238caeca0 | 179600f238caeca0 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_BF16 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1ed96900c55712af | 1ed96900c55712af |
| roadNet-CA | diagnostic-10867492 | full_fp32 | EQ | matched_layer1_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1fa5d37365db6e99 | 1fa5d37365db6e99 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 179600f238caeca0 | 179600f238caeca0 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_BF16 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1ed96900c55712af | 1ed96900c55712af |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 0.172460556 | 0.003384607 | 1 | 0 | 1 | 80ff88ced8cacdc4 | 1fa5d37365db6e99 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 5.033935547 | 0.005232375 | 1 | 0 | 1 | 342048974ed89b37 | 179600f238caeca0 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 8.000000000 | 0.005758446 | 1 | 0 | 1 | 7a1550a97f571521 | 1ed96900c55712af |
| roadNet-CA | diagnostic-10867492 | full_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 0.172460556 | 0.003384607 | 1 | 0 | 1 | 80ff88ced8cacdc4 | 1fa5d37365db6e99 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 5.033935547 | 0.005232375 | 1 | 0 | 1 | 342048974ed89b37 | 179600f238caeca0 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 8.000000000 | 0.005758446 | 1 | 0 | 1 | 7a1550a97f571521 | 1ed96900c55712af |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1fa5d37365db6e99 | 1fa5d37365db6e99 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1ed96900c55712af | 1ed96900c55712af |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 80ff88ced8cacdc4 | 80ff88ced8cacdc4 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 7a1550a97f571521 | 7a1550a97f571521 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1fa5d37365db6e99 | 1fa5d37365db6e99 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 1ed96900c55712af | 1ed96900c55712af |
| roadNet-CA | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 80ff88ced8cacdc4 | 80ff88ced8cacdc4 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 7a1550a97f571521 | 7a1550a97f571521 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.077636719 | 0.000002525 | 1 | 0 | 1 | ee8935583731482c | 72ecfe2c6c631846 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 27.347656250 | 0.003161305 | 1 | 0 | 1 | ee8935583731482c | f73e17ee9f20e606 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 15.250000000 | 0.000001545 | 1 | 0 | 1 | 5b0eb778f0a94efb | 3acf4b235a958d34 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 62.750000000 | 0.000039654 | 1 | 0 | 1 | 5b0eb778f0a94efb | 64bd3fea98b315ba |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 19292.750000000 | 0.005669620 | 1 | 0 | 1 | 5b0eb778f0a94efb | 2e3aeda6d82df31b |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 4096.000000000 | 0.000176456 | 1 | 0 | 1 | 9a134a6583948ccc | d338984b20b202a6 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 16384.000000000 | 0.006039043 | 1 | 0 | 1 | 9a134a6583948ccc | 2858e1ebf4591e45 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 57626.500000000 | 0.017506075 | 1 | 0 | 1 | 9a134a6583948ccc | 812b882cdb2534f2 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.137695312 | 0.000002723 | 1 | 0 | 1 | 96886ec9e7e7c7d8 | c75ec79b1b59c588 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 12.375000000 | 0.000001498 | 1 | 0 | 1 | d1a25297d5bf47aa | 798fca52103d280b |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 66.562500000 | 0.000018754 | 1 | 0 | 1 | d1a25297d5bf47aa | c274c66e83501406 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 4096.000000000 | 0.000131397 | 1 | 0 | 1 | f284bee79f47856b | b3c0f759d6a49137 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000976562 | 0.000000074 | 1 | 0 | 1 | 3a8eb3f1458bc9d7 | 8a8874f617548202 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | kernel_FP32 | original_TFS | relative_L2_and_normalized_max | 0.010000 | 37.036132812 | 0.003276747 | 1 | 0 | 1 | 3a8eb3f1458bc9d7 | f73e17ee9f20e606 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.375000000 | 0.000000053 | 1 | 0 | 1 | 2a9dc8247e03a685 | 4739a1bff0db3b6f |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 32.000000000 | 0.000003369 | 1 | 0 | 1 | 2a9dc8247e03a685 | efcf7a7da49c2b6d |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | e2e_FP32_final | original_TFS | relative_L2_and_normalized_max | 0.010000 | 18391.500000000 | 0.005843446 | 1 | 0 | 1 | 2a9dc8247e03a685 | 2e3aeda6d82df31b |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 512.000000000 | 0.000011495 | 1 | 0 | 1 | 997ab7be2063754f | f2d1da8dbdfa95c1 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | original_TFS | relative_L2_and_normalized_max | 0.020000 | 16384.000000000 | 0.006246486 | 1 | 0 | 1 | 997ab7be2063754f | 2858e1ebf4591e45 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | CHECK | e2e_BF16_final | source_MKL_FP32 | relative_L2_and_normalized_max | 0.030000 | 57626.500000000 | 0.017563192 | 1 | 0 | 1 | 997ab7be2063754f | 812b882cdb2534f2 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | CHECK | layer1_FP32 | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.000976562 | 0.000000073 | 1 | 0 | 1 | d93381363f7a6f63 | df4df14bedec8266 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | CHECK | layer2_FP32_actual_input | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 0.250000000 | 0.000000053 | 1 | 0 | 1 | ccc2283090ed75eb | 6b33539ad4e8ef80 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | CHECK | e2e_FP32_final | own_math_oracle | relative_L2_and_normalized_max | 0.010000 | 63.250000000 | 0.000009503 | 1 | 0 | 1 | ccc2283090ed75eb | 3bad5baeab07334d |
| wiki-Talk | diagnostic-10867492 | full_fp32 | CHECK | e2e_BF16_final | own_math_oracle | relative_L2_and_normalized_max | 0.020000 | 1024.000000000 | 0.000027396 | 1 | 0 | 1 | e7aca89817f6bd00 | cda00f629784e20f |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | EQ | matched_layer1_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | ee8935583731482c | ee8935583731482c |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_FP32 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 5b0eb778f0a94efb | 5b0eb778f0a94efb |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | EQ | matched_layer2_BF16 | b64_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 9a134a6583948ccc | 9a134a6583948ccc |
| wiki-Talk | diagnostic-10867492 | full_fp32 | EQ | matched_layer1_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 3a8eb3f1458bc9d7 | 3a8eb3f1458bc9d7 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_FP32 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 2a9dc8247e03a685 | 2a9dc8247e03a685 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | EQ | matched_layer2_BF16 | full_bf16 | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 997ab7be2063754f | 997ab7be2063754f |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 11.991210938 | 0.003180369 | 1 | 0 | 1 | 96886ec9e7e7c7d8 | ee8935583731482c |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 15776.250000000 | 0.004590589 | 1 | 0 | 1 | d1a25297d5bf47aa | 5b0eb778f0a94efb |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 16384.000000000 | 0.005171344 | 1 | 0 | 1 | f284bee79f47856b | 9a134a6583948ccc |
| wiki-Talk | diagnostic-10867492 | full_fp32 | DIFF | layer1_FP32 | BF16_same_B_native | report_only | 0.000000 | 21.235351562 | 0.003351110 | 1 | 0 | 1 | d93381363f7a6f63 | 3a8eb3f1458bc9d7 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | DIFF | e2e_FP32_final | BF16_same_B_native | report_only | 0.000000 | 16092.000000000 | 0.005032143 | 1 | 0 | 1 | ccc2283090ed75eb | 2a9dc8247e03a685 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | DIFF | e2e_BF16_final | BF16_same_B_native | report_only | 0.000000 | 16384.000000000 | 0.005572600 | 1 | 0 | 1 | e7aca89817f6bd00 | 997ab7be2063754f |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | ee8935583731482c | ee8935583731482c |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 9a134a6583948ccc | 9a134a6583948ccc |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 96886ec9e7e7c7d8 | 96886ec9e7e7c7d8 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | f284bee79f47856b | f284bee79f47856b |
| wiki-Talk | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 3a8eb3f1458bc9d7 | 3a8eb3f1458bc9d7 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | 997ab7be2063754f | 997ab7be2063754f |
| wiki-Talk | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer1_FP32 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | d93381363f7a6f63 | d93381363f7a6f63 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | PROFILE_GATE | layer2_BF16 | uninstrumented_same_method | bitwise | 0.000000 | 0.000000000 | 0.000000000 | 1 | 1 | 1 | e7aca89817f6bd00 | e7aca89817f6bd00 |



## Q1–Q7：由本轮数据回答


### Q1：全局 FP32→BF16 层间转换是否为大图主瓶颈？


已测 BF16 方法的 conversion 中位数范围 0.880–4.945 ms，占独立 stage total 的 0.036–10.240%。各图分别见表 2；这衡量全局一次转换，不能代表每边 decode。

按 E≥10M 明确界定的大图子集，该占比范围 0.036–1.694%。是否为主要瓶颈应依据这个占比与 L1/L2 时间，不能从历史 1–5 ms 猜测。


### Q2：hot loop 每邻居 BF16 decode 的诊断成本？


| 图 | RUN | 方法 | 层 | 采样 thread ms | clocked sections |
| --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | 1 | 160.282612 | 6091897 |
| mycielskian19 | diagnostic-10867492 | b64_bf16 | 2 | 157.464743 | 6091897 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | 1 | 0.000000 | 0 |
| mycielskian19 | diagnostic-10867492 | b64_fp32 | 2 | 0.000000 | 0 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | 1 | 156.488419 | 6091897 |
| mycielskian19 | diagnostic-10867492 | full_bf16 | 2 | 165.152311 | 6091897 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | 1 | 0.000000 | 0 |
| mycielskian19 | diagnostic-10867492 | full_fp32 | 2 | 0.000000 | 0 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | 1 | 13.570070 | 523976 |
| ogbn-products | diagnostic-10867492 | b64_bf16 | 2 | 13.477325 | 523976 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | 1 | 0.000000 | 0 |
| ogbn-products | diagnostic-10867492 | b64_fp32 | 2 | 0.000000 | 0 |
| ogbn-products | diagnostic-10867492 | full_bf16 | 1 | 13.372898 | 523976 |
| ogbn-products | diagnostic-10867492 | full_bf16 | 2 | 13.592243 | 523976 |
| ogbn-products | diagnostic-10867492 | full_fp32 | 1 | 0.000000 | 0 |
| ogbn-products | diagnostic-10867492 | full_fp32 | 2 | 0.000000 | 0 |
| reddit | diagnostic-10867492 | b64_bf16 | 1 | 13.817310 | 506889 |
| reddit | diagnostic-10867492 | b64_bf16 | 2 | 13.519049 | 506889 |
| reddit | diagnostic-10867492 | b64_fp32 | 1 | 0.000000 | 0 |
| reddit | diagnostic-10867492 | b64_fp32 | 2 | 0.000000 | 0 |
| reddit | diagnostic-10867492 | full_bf16 | 1 | 13.623238 | 506889 |
| reddit | diagnostic-10867492 | full_bf16 | 2 | 13.406277 | 506889 |
| reddit | diagnostic-10867492 | full_fp32 | 1 | 0.000000 | 0 |
| reddit | diagnostic-10867492 | full_fp32 | 2 | 0.000000 | 0 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | 1 | 0.653982 | 21676 |
| roadNet-CA | diagnostic-10867492 | b64_bf16 | 2 | 0.686884 | 21676 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | 1 | 0.000000 | 0 |
| roadNet-CA | diagnostic-10867492 | b64_fp32 | 2 | 0.000000 | 0 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | 1 | 0.620604 | 21676 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | 2 | 0.683069 | 21676 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | 1 | 0.000000 | 0 |
| roadNet-CA | diagnostic-10867492 | full_fp32 | 2 | 0.000000 | 0 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | 1 | 3.049374 | 110027 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | 2 | 2.951860 | 110027 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | 1 | 0.000000 | 0 |
| wiki-Talk | diagnostic-10867492 | b64_fp32 | 2 | 0.000000 | 0 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | 1 | 2.817392 | 110027 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | 2 | 2.892017 | 110027 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | 1 | 0.000000 | 0 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | 2 | 0.000000 | 0 |


这些值来自独立插桩采样：加载/解码分组、timer 开销和寄存器 spill 会影响结果。它们不是 E2E 占比，不能将其直接减去 E2E 预测去 decode 的收益。原始细分与线程尾部见 sampled_profiles.csv / thread_profiles.csv。


### Q3：native FP32 去掉 decode 是否节省计算？


代码模型确实移除每边 8 个 BF16 zero-extend 和 8 个 shift，同时保留 8 次 FP32 vector add；FP32 仍执行部分和截断、AMX、C staging。matched_kernel 观察相同数值的表示控制（表 1 补充），PMU 指令/cycles 用独立 native E2E 诊断（表 4）。后者还包含 native 数值和层间表示差异，不能独自证明 decode 的因果成本。


### Q4：两倍 feature 请求字节是否抵消节省？


- `e2e`：10 个图×B 对照中，BF16/FP32 ratio>1 的 2 个，<1 的 8 个；满足声明的稳定方向判据 10 个。

- `kernel`：10 个图×B 对照中，BF16/FP32 ratio>1 的 0 个，<1 的 10 个；满足声明的稳定方向判据 8 个。

- `matched_kernel`：10 个图×B 对照中，BF16/FP32 ratio>1 的 0 个，<1 的 10 个；满足声明的稳定方向判据 10 个。

H 请求模型由 256E B 变为 512E B。结果只说明这两条完整受控路径的净效应；未测过程专属 DRAM 带宽，不将净变化全部归因于内存流量或 decode。


### Q5：B64 与 FULL 的答案是否不同？


| 图 | RUN | B64 paired IQR方向 | B64稳定 | FULL paired IQR方向 | FULL稳定 | ratio交互 FULL/B64 |
| --- | --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | bf16_faster | True | bf16_faster | True | 1.216 |
| ogbn-products | diagnostic-10867492 | bf16_faster | True | bf16_faster | True | 1.024 |
| reddit | diagnostic-10867492 | bf16_faster | True | bf16_faster | True | 0.947 |
| roadNet-CA | diagnostic-10867492 | fp32_faster | True | fp32_faster | True | 1.000 |
| wiki-Talk | diagnostic-10867492 | bf16_faster | True | bf16_faster | True | 1.014 |


两者共享相同原归约循环但投影次数、partial 转换与 C spill/reload 不同，具体计数见表 3。方向变化本身不等于硬件机制已被证明。


### Q6：是否存在稳定 precision×B crossover？


本轮发现 0 个满足描述性稳定判据的 crossover：无。

判据预先写入脚本：每个 B 有完整 10 对，paired ratio 的 P25–P75 完全在 1 同一侧，forward/reverse 中位数方向一致，两个方法各自 CV≤10%；B64/FULL 的稳定方向相反才记 crossover。IQR 判据不是统计显著性检验或硬件因果证明，完整结果不因 CV 警告而删除。


### Q7：Mycielskian 的 FULL 异常是否被 FP32 明显改变？


| 图 | RUN | BF16 FULL/B64 | FP32 FULL/B64 | precision×B ratio | 稳定 crossover |
| --- | --- | --- | --- | --- | --- |
| mycielskian19 | diagnostic-10867492 | 2.195 | 1.805 | 1.216 | False |


若两种表示的 FULL/B64 都明显>1，去 decode 未消除此退化，不支持 decode 单独主导的解释；但仍不能完全排除 decode 与访存/调度的交互。若明显改变，需结合 matched-input、线程尾部、页面分布、PMU 及启停偏差复核。


## 波动、顺序与失败证据


统计定义：P25/P75 采用排序后线性插值；CV=population std/mean。CV>10% 或 forward/reverse 方法中位数跨度>10% 标为诊断警告，保留所有原值。共享节点波动可影响小幅胜负。

| 图 | RUN | 方法 | 边界 | CV % | forward ms | reverse ms | order max/min |
| --- | --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | diagnostic-10867492 | b64_fp32 | kernel | 5.378 | 228.489 | 252.197 | 1.104 |
| roadNet-CA | diagnostic-10867492 | full_bf16 | kernel | 35.874 | 16.322 | 16.347 | 1.002 |
| wiki-Talk | diagnostic-10867492 | b64_bf16 | preparation_inclusive | 14.433 | 68.164 | 66.636 | 1.023 |
| wiki-Talk | diagnostic-10867492 | full_bf16 | kernel | 17.819 | 20.214 | 19.111 | 1.058 |
| wiki-Talk | diagnostic-10867492 | full_fp32 | preparation_inclusive | 10.719 | 61.384 | 70.533 | 1.149 |


| 图 | RUN | 状态 | gate failures | 证据 |
| --- | --- | --- | --- | --- |




## 可导出数据和图


主表与统计：timing_summary.csv、paired_comparisons.csv、stage_medians.csv、logical_operations.csv、pmu_summary.csv、correctness.csv、factor_interactions.csv；所有重复及失败保留在 raw_*.csv。

独立图未生成：disabled。
