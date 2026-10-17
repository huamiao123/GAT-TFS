# 保留 TFS 的部分邻居静态执行计划：实现与五图实验

日期：2026-10-14。静态单位边权、随机 H/W seed12345、两层 128→128→128。不是训练或真实 checkpoint 分类准确率实验。

## 研究问题与改动

上一轮 partial bundle 减少 C 暂存，但逐边 source 解码未减少。本轮检验：把已有相邻两行 current-head 匹配提前编码，能否在不改变各行 CSR 加法顺序的条件下复用部分 source 读取/解码，并避免在线匹配开销。

保留原 DegreeSort/TR16 行成员和 AMX 行槽、R64 dynamic(1)、S64、W packing、BF16 高16位截断、PACK_EPI、AMX 配置/投影和默认 NUMA。主性能固定 FULL FAST；M64/M256 和 ACCURATE 仅用于正确性检查。原 TFS/MKL 与冻结父源码不改动。

静态 inspector 复用已有 greedy current-head 合并，编码 mask1/2/3 为两字节 run。每个 S64 窗口固定选择“至少8次共享、至多8个run”；结果出来后未调阈值。不是邻居集合去重，不合并公共和，不缓存跨 forward 的特征，也不改变逐行浮点加法顺序。共享路径也可能省当前 CSR 索引读取，收益不能全部归于 BF16 decode。

| 名称 | 真实作用 |
| --- | --- |
| PACK_EPI | 规范控制；原 ScalarReducer + 已验证融合层间 epilogue |
| DynamicAll | 旧动态匹配，所有相邻窗口均执行 current-head 比较 |
| StaticIndependent | 新计划、相同选区，分别加载/解码两个 source |
| StaticShared | 新计划、相同选区，相同 source 只加载/解码一次 |
| DynamicSelected | 只在新计划选择的相同窗口做动态匹配与共享 |
| ExactOnly | 只共享完全相同正长度窗口，含不足8条的短窗口 |
| P0/Original/MKL | 既有控制及源码锚点 |

## 环境与完整性

build：`ordered-source-build-20261014-133737`；小图/结构 Job 10955177，性能 Job 10955178/qhcn017。共享 intel、单 socket32物理核、close/cores、无 interleave，`paper_eligible=false`。共享结果为探索证据。每方法1次 warmup、5次连续和10次正反顺序测量；以10次中位数为主，两种协议/raw/CV均保存。

初始 H/W、图读取、DegreeSort、计划构造在 prepared E2E 外。计划构造/审核/容量及摊销另列。PACK_EPI 路径 ReLU/截断在 Layer1 内，stage 中独立 ReLU/convert 为0；不是删除模型激活。子段分别取中位数，不能保证相加恰好等于 total 的中位数。

| 阶段 | 数值检查 | 逐位边界 | 源级工作量 | production profile一致性 |
| --- | --- | --- | --- | --- |
| 小图+结构 | 1056 | 1896 | 72 | 72 |
| 五图性能 | 160 | 140 | 60 | 60 |

上述检查全部PASS。六张小图覆盖空行、尾tile、S/M边界、无序、重复、部分公共run、交叉序列及抵消输入；保留原19值 ReLU/截断语义检查。FAST/ACCURATE、kernel_FP32/final_FP32/H1_BF16/final_BF16 四边界逐位一致；原数值阈值未放宽。完整逐边加法和 AMX 次数不变。

## 主性能：完整两层 prepared E2E，ms

ratio=PACK_EPI / StaticShared，>1 表示新路径更快。没有逐图择优或混合部署策略。

| 图 | PACK_EPI | StaticIndependent | StaticShared | DynamicSelected | ExactOnly | ratio |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 276.182 | 273.160 | 271.630 | 267.889 | 268.793 | 1.0168 |
| reddit | 120.202 | 121.561 | 117.977 | 114.494 | 115.978 | 1.0189 |
| mycielskian19 | 1087.343 | 1091.784 | 1089.537 | 1094.614 | 1083.853 | 0.9980 |
| roadNet-CA | 23.987 | 23.959 | 23.844 | 23.843 | 24.282 | 1.0060 |
| wiki-Talk | 36.235 | 36.523 | 36.418 | 36.177 | 36.425 | 0.9950 |

固定 StaticShared 五图描述性 GM (alternating)：1.0069×。
固定 StaticShared 五图描述性 GM (consecutive)：1.0046×。

| 图 | Original TFS | 源码 MKL FP32 | P0 | DynamicAll |
| --- | --- | --- | --- | --- |
| ogbn-products | 341.809 | 1356.012 | 289.295 | 274.181 |
| reddit | 307.446 | 445.827 | 120.159 | 127.250 |
| mycielskian19 | 2185.973 | 4798.844 | 1091.243 | 1093.015 |
| roadNet-CA | 35.639 | 65.133 | 39.369 | 23.840 |
| wiki-Talk | 108.454 | 90.799 | 46.058 | 37.307 |

共有1个方法/协议CV>5%；见 timing_anomalies.csv。不能把共享节点小差异自动视为稳定论文加速。

## 覆盖率、准备成本与容量

| 图 | 所有匹配/E | 选中匹配/E | 新增部分匹配/E | plan MiB | allocated MiB | 构造 ms | 审核 ms | 乐观摊销次数 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 0.018301% | 0.000346% | 0.000008% | 2.340 | 2.342 | 1162.488 | 0.263 | 256 |
| reddit | 1.672323% | 0.205849% | 0.103563% | 0.690 | 0.751 | 882.512 | 0.671 | 398 |
| mycielskian19 | 9.425782% | 1.296366% | 0.641223% | 66.181 | 129.460 | 4122.957 | 44.556 | NO_POSITIVE_MEDIAN_GAIN |
| roadNet-CA | 1.501803% | 0.000000% | 0.000000% | 2.468 | 2.880 | 63.449 | 1.053 | 453 |
| wiki-Talk | 0.429063% | 0.000159% | 0.000000% | 2.801 | 3.283 | 32.666 | 1.017 | NO_POSITIVE_MEDIAN_GAIN |

“所有匹配”仍仅是既有 DegreeSort 相邻配对、窗口和 greedy head 顺序约束下的匹配，并非图的复用上界。partial列不含完全相同窗口。摊销只用观测中位数差估算，未考虑噪声、不含双方共同的 DegreeSort，不能承诺真实 break-even。先在一核结构作业核对覆盖率，再运行同阈值32核五图；两次结构计数一致。

## 精细计时与工作量

| 图 | 方法 | Layer1 | ReLU | 独立Convert | Layer2 | Total |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | pack_epi_degree | 135.621 | 0.000 | 0.000 | 136.594 | 271.699 |
| ogbn-products | event_independent | 135.932 | 0.000 | 0.000 | 138.272 | 274.205 |
| ogbn-products | event_share | 136.461 | 0.000 | 0.000 | 136.412 | 273.283 |
| ogbn-products | dynamic_selected_share | 136.085 | 0.000 | 0.000 | 136.619 | 274.092 |
| reddit | pack_epi_degree | 59.923 | 0.000 | 0.000 | 59.160 | 118.488 |
| reddit | event_independent | 60.476 | 0.000 | 0.000 | 57.350 | 121.237 |
| reddit | event_share | 58.888 | 0.000 | 0.000 | 60.867 | 117.489 |
| reddit | dynamic_selected_share | 60.086 | 0.000 | 0.000 | 59.212 | 119.247 |
| mycielskian19 | pack_epi_degree | 544.614 | 0.000 | 0.000 | 521.012 | 1077.120 |
| mycielskian19 | event_independent | 547.025 | 0.000 | 0.000 | 559.739 | 1100.005 |
| mycielskian19 | event_share | 536.256 | 0.000 | 0.000 | 543.402 | 1086.224 |
| mycielskian19 | dynamic_selected_share | 557.165 | 0.000 | 0.000 | 562.979 | 1111.019 |
| roadNet-CA | pack_epi_degree | 11.810 | 0.000 | 0.000 | 12.166 | 23.963 |
| roadNet-CA | event_independent | 11.467 | 0.000 | 0.000 | 12.379 | 23.841 |
| roadNet-CA | event_share | 11.445 | 0.000 | 0.000 | 12.301 | 23.746 |
| roadNet-CA | dynamic_selected_share | 11.461 | 0.000 | 0.000 | 12.223 | 23.765 |
| wiki-Talk | pack_epi_degree | 19.487 | 0.000 | 0.000 | 17.680 | 36.957 |
| wiki-Talk | event_independent | 17.502 | 0.000 | 0.000 | 17.523 | 35.119 |
| wiki-Talk | event_share | 18.644 | 0.000 | 0.000 | 17.648 | 36.189 |
| wiki-Talk | dynamic_selected_share | 18.347 | 0.000 | 0.000 | 17.510 | 35.620 |

12个外层phase、24个细分字段与每线程active/finish/edges/tiles均保留。注意父快照中source load/decode只在外层 gather_decode 合测，24字段中的独立 load/decode 为未测占位零值；add已经另测。不能把这两个零值说成零成本。partial state、packing、AMX A/B/TDP/C、output、permission/config/release的已有独立计时保留。另建只用于诊断的快照补全load/decode拆分，附加结果单列，不替换本主性能快照。

细分结果来自 sampled/thread-local diagnostic instrumentation，仅标明采样范围中的线程时间，不是 wall 分解，不能加成 E2E 百分比，采样计时开销也可能改变流水。每个production profile与无插桩输出逐位一致。详见 raw/event_profile.csv、event_detail.csv、event_thread.csv；thread_imbalance.csv提供运行时间不均衡指标。

| 图 | 每层真实边 | 静态共享省decode行 | 省decode比例 | 独立16BF16 ops | 共享16BF16 ops | AMX TDP |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 123718152 | 428 | 0.000346% | 989745216 | 989741792 | 4801248 |
| reddit | 114615892 | 235936 | 0.205849% | 916927136 | 915039648 | 465952 |
| mycielskian19 | 903194710 | 11708708 | 1.296366% | 7225557680 | 7131888016 | 786432 |
| roadNet-CA | 5533214 | 0 | 0.000000% | 44265712 | 44265712 | 3930464 |
| wiki-Talk | 5021410 | 8 | 0.000159% | 40171280 | 40171216 | 295264 |

这些是源级向量操作与逻辑读取请求，不是精确 retired decode 指令或实际 DRAM bytes。编译器可自然优化，未引入 volatile/barrier 强迫独立控制。每个真实 incidence 的128维加法仍执行；共享只省 source load/decode/部分当前CSR读取。prefetch请求、partial状态与packing、AMX工作量保持相同。

## PMU

成功有效采集 60/60 次。只纳入32线程完整覆盖、无新增/失踪/零running线程、成功缩放的记录。cycles/instructions/cache_misses/cache_references为线程组总量；EXE.AMX_BUSY为原helper在SPR模型识别后采集的busy cycles，不是TDP数量或wall百分比。

| 图 | 方法 | instructions | cycles | IPC | generic cache misses | AMX busy |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | pack_epi_degree | 10771186421 | 25344425001 | 0.4250 | 561943121 | 154209440 |
| ogbn-products | event_independent | 10803046986 | 25112546501 | 0.4302 | 560251993 | 154227176 |
| ogbn-products | event_share | 10940665330 | 25286610454 | 0.4327 | 559481477 | 154218256 |
| ogbn-products | dynamic_selected_share | 11346039354 | 25697534564 | 0.4415 | 559013900 | 154222120 |
| reddit | pack_epi_degree | 9816003643 | 11236318193 | 0.8736 | 653572599 | 14922772 |
| reddit | event_independent | 9992057898 | 11006341210 | 0.9078 | 653189459 | 14929160 |
| reddit | event_share | 9800036164 | 11145163335 | 0.8793 | 653123848 | 14928980 |
| reddit | dynamic_selected_share | 10026328569 | 11196885824 | 0.8955 | 653166498 | 14927532 |
| mycielskian19 | pack_epi_degree | 80529825828 | 104014547743 | 0.7742 | 4044545413 | 25203816 |
| mycielskian19 | event_independent | 79075957847 | 103220484173 | 0.7661 | 4043838613 | 25249620 |
| mycielskian19 | event_share | 80194780537 | 104636350863 | 0.7664 | 4045653869 | 25252532 |
| mycielskian19 | dynamic_selected_share | 78703541090 | 101177681568 | 0.7779 | 4045349695 | 25319864 |
| roadNet-CA | pack_epi_degree | 993568120 | 1967321766 | 0.5050 | 14659633 | 125991264 |
| roadNet-CA | event_independent | 1074347603 | 1954404584 | 0.5497 | 14552561 | 125989356 |
| roadNet-CA | event_share | 1073784052 | 1951506444 | 0.5502 | 14438162 | 125990492 |
| roadNet-CA | dynamic_selected_share | 1075026175 | 1949434128 | 0.5515 | 14425592 | 125982280 |
| wiki-Talk | pack_epi_degree | 2674939880 | 3392641957 | 0.7885 | 28163009 | 9459876 |
| wiki-Talk | event_independent | 2739416424 | 3422567367 | 0.8004 | 25806450 | 9459276 |
| wiki-Talk | event_share | 2874150981 | 3549638656 | 0.8097 | 26257874 | 9459868 |
| wiki-Talk | dynamic_selected_share | 2825407173 | 3511388297 | 0.8046 | 25987895 | 9459364 |

没有独立 L1/L2/LLC 分级、DRAM带宽或TLB归因证据，不能把缺失计数填成已证实的memory bound。PMU和sampled profile为另外独立诊断调用，不混入主表。

## 可证明与不能证明的结论

可证明：部分公共邻居的静态执行计划可以保持TFS和逐行浮点加法顺序，并达到四边界逐位一致；新增部分共享不改变projection数量及量化边界。真实图中固定DegreeSort相邻配对、原CSR顺序与压缩元数据约束显著限制其覆盖率，不能把集合相似度等同于可执行复用。

需按表逐图解释：StaticShared vs StaticIndependent检验相同计划的共享收益；StaticShared vs DynamicSelected检验相同选区的静态匹配收益；StaticShared vs ExactOnly观察新增部分窗口价值；vs PACK_EPI才是新增整体收益。RoadNet和wiki基本无新增覆盖，是元数据/分支控制；Products覆盖接近零，任何明显加速不能仅归因于省decode。Myciel拥有最高新增覆盖，但仍很小，FULL退化是否改变必须按实际总时间判断。

不能证明：这是强通用优化、是已满足期刊创新的独立方法、classification accuracy改善、所有匹配可独立省DRAM读取、PMU cache_misses等于带宽或FULL退化由decode单独主导。共享节点单轮结果不能替代独占重复实验。

后续设计应先量化“合法且可压缩的复用覆盖率”，再决定是否增加通用配对/重排开销。若需改变DegreeSort配对或CSR序列，必须作为另一实验同时控制padding、逐行加法顺序、预处理成本、矩阵投影不均衡及NUMA，不能把这些变化藏在本轮共享解码实现里。本轮不据五图结果回调8/8阈值，也不构造逐图最优oracle。

## 文件与复现

`source_snapshot/`：实际编译快照；`raw/ordered-*`：所有原始输出/命令/哈希/gates；`audit.json`：完整环境、manifest、counts；`timing_summary.csv`、`paired_comparisons.csv`、`coverage_and_setup.csv`、`stage_summary.csv`、`work_comparison.csv`、`pmu_summary.csv`、`correctness_summary.csv`、`thread_imbalance.csv`。原 frozen snapshot 未覆盖。

## 补充：独立诊断快照的 load/decode/add 拆分

主性能快照不变。新增诊断 build `ordered-source-detail-build-20261014-135515`；小图Job10960717、五图32核Job10960725/qhcn017。全部数值、四边界、工作量及插桩输出检查PASS；逻辑工作量与主快照逐字段一致。未从这份build采集或混入任何STATE_TIME主性能样本。继承STATE_CONFIG的warmup/repeat标签在诊断模式不执行，以实际0条STATE_TIME为准。

原helper的load/decode合测、add另测；独立load/decode字段原为未测零。补充仅修改sampled的P=true分支，先加载8个256-bit BF16向量，再做扩展/位移decode，随后原FP32 add；非sampled及production分支仍用原表达式。独立计时改变向量生存期、寄存器压力、流水及timer开销；值不能直接解释production decode成本、不能求和冒充E2E wall百分比。

下表为Layer2，单位 sampled thread ms；全部Layer1/2、6方法、24细分段及12外层段保存在 separate_detail_evidence/raw 和 separated_detail_summary.csv。

| 图 | 方法 | BF16 load | BF16 decode | FP32 add | 采样source事件 |
|---|---|---:|---:|---:|---:|
| ogbn-products | pack_epi_degree | 15.0201 | 13.3746 | 13.6006 | 523976 |
| ogbn-products | event_independent | 14.8959 | 13.3419 | 13.5317 | 523976 |
| ogbn-products | event_share | 15.1112 | 13.2360 | 13.4027 | 523976 |
| ogbn-products | dynamic_selected_share | 15.0099 | 13.4082 | 13.7107 | 523976 |
| reddit | pack_epi_degree | 14.0371 | 13.3429 | 13.6755 | 506889 |
| reddit | event_independent | 13.9430 | 13.4668 | 13.6628 | 506889 |
| reddit | event_share | 14.0467 | 13.3207 | 13.5999 | 505886 |
| reddit | dynamic_selected_share | 14.1251 | 13.3793 | 13.5293 | 505886 |
| mycielskian19 | pack_epi_degree | 162.7409 | 157.2015 | 158.6246 | 6091897 |
| mycielskian19 | event_independent | 166.2996 | 168.3609 | 170.6026 | 6091897 |
| mycielskian19 | event_share | 157.2466 | 159.2951 | 161.3772 | 5776914 |
| mycielskian19 | dynamic_selected_share | 158.9940 | 160.4581 | 160.9085 | 5776914 |
| roadNet-CA | pack_epi_degree | 1.3344 | 0.6196 | 0.5870 | 21676 |
| roadNet-CA | event_independent | 1.3566 | 0.6311 | 0.6213 | 21676 |
| roadNet-CA | event_share | 1.3602 | 0.6275 | 0.6275 | 21676 |
| roadNet-CA | dynamic_selected_share | 1.3309 | 0.6194 | 0.5770 | 21676 |
| wiki-Talk | pack_epi_degree | 2.9747 | 2.7509 | 2.7640 | 110027 |
| wiki-Talk | event_independent | 3.0153 | 2.7530 | 2.8696 | 110027 |
| wiki-Talk | event_share | 2.8706 | 2.8462 | 2.8646 | 110027 |
| wiki-Talk | dynamic_selected_share | 3.0172 | 2.7907 | 2.8248 | 110027 |

本轮细分提供定位线索，尚不能证明load/decode哪一项是production主瓶颈。最明确的结构证据仍是：固定原DegreeSort配对和CSR顺序后，可压缩部分共享覆盖率低；新静态计划五图prepared GM仅约1.0069×，最高覆盖Myciel没有改善。不要扩展17图后以逐图最好阈值拼成论文加速。

## 补充：空区间计时校准

同节点qhcn017、单socket32物理核 Job10960745，复制原tick/detail_tock的omp_get_wtime表达式进行空区间校准，1批warmup+10批×32线程×20000区间，320条正式统计。源代码、flags、环境及全部原始结果保存于 timer_calibration_evidence。

空区间均值的中位数 **25.469 ns**；min/max 24.641/26.631 ns；区间中位数 0.000 ns；零值比例中位数 **97.465%**；最小观测正差 953.674 ns；返回double的ULP中位数 238.419 ns。没有负区间。

这意味着单次微段往往小于返回时间戳的可表示间距，聚合均值同时包含计时开销和量化误差。补充的load/decode/add数据仅作诊断线索，不能据此宣称decode占production某个百分比；也不能机械减去一个空区间均值。细分计时还会改变寄存器生存期及指令调度。ms级E2E中位数仍来自原主快照，没有修正或替换。后续若要定量区分这些纳秒微段，应测足够长的成组区域，并校准更细粒度的时间基准/硬件事件；这属于测量设计，不是当前已实现的加速方法。
