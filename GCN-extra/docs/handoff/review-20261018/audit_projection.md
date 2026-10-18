# 9369b9e 深审：P0/P1/P2/P3/P4、feature precision、cache control

审阅日期：2026-10-18。只读审计 GitHub 快照 `C:/Users/花喵/tmp/tfs18`；没有运行 benchmark、没有修改内核。读了用户 20261018 TXT，追到 runtime、代码生成脚本、冻结源码、逐次 CSV，并抽查归档 tar 中 stdout。本文路径均相对 `GCN-extra/`。

## 主要判断

用户 TXT 对我负责范围的大多数数值与负结果归纳是准确的，独立复算没有发现需要推翻的主结果。最重要的修订是：**必须把所有“FULL”展开成完整执行定义**，不能认为名字相同就是同一内核。P1 的 FULL 已包含 S64；P3/P4 的 FULL 还包含 ACCURATE。它们都不是 10 月 15 日 DegreeSort/Identity 消融中的 rowwise FULL FAST。

另外，P0/A3 控制了源码级逻辑状态请求，而不是证明实际机器 load/store 指令、缓存流量或 DRAM 流量相同；P1 的受控图已经给出“采样 reuse/page 指标几乎相同，性能却大变”的反例。当前证据足以证明访问组织值得研究，尚不足以得出一个只靠 source reuse 的可靠模型。

## 1. 真实执行口径

| 报告名称 | 实际路径 | 源码依据 |
|---|---|---|
| 原 TFS | 每个 destination tile、每个 output panel 遍历最大邻居位置；BF16 H 进入 AMX | `original/gcn_e2e_v3.cpp:84`，`:100` |
| `bfull_fast` / rowwise FULL / P0 A0 | 每行完整 FP32 归约，逐行结束后存 partial；tile 全部结束后 BF16 hi、AMX | `src/paper_methods_kernels.hpp:46`，`:98`，`:113` |
| `s64_mfull_fast` / P0 A1 | 每行至多 64 邻居后切下一行；跨窗口读写 FP32 partial；最后投影一次 | `src/projection_window_kernels.hpp:241`，`:254`，`:355`，`:464` |
| P0 A3 rowwise | 保留 64 边窗口逻辑状态请求，但把 row 循环放在 window 外 | `experiments/journal_decoupling_a3_20261004/make_a3_kernel.py:14`，`:24` |
| P1 表中 FULL Degree / Source / Page | **S 固定 64，M=FULL，FAST**，更换有限范围 destination 排列 | `experiments/journal_grouping_p1_20261005/grouping_runtime.hpp:18`，`:66` |
| P3 FULL ACC | **S64/MFULL ACCURATE** | `experiments/journal_followup_p3_p4_20261005/residual_broad.hpp:98`，`:108` |
| P4 fused FULL | shape 模板下 **S64/MFULL ACCURATE**；等宽第一层与 frozen P0 逐位比对 | `experiments/journal_followup_p3_p4_20261005/selective_shape.cpp:49`，`:58`；`selective_kernels.hpp:106` |

因此 P1 的 Myciel 1102.35→904.91 ms，是在 S64/MFULL 已有收益之上做分组；不是 rowwise FULL 的 2437.96→904.91 ms 仅由 source/page 分组实现。P1 自身同轮主表保留了 `Coupled FULL=2437.959`，足够确认此区别。User TXT E3 应明确补上 S64，E9/E10 亦宜补上 S64/ACCURATE。

原 TFS 的 `32 Σ m_t`、FAST block 的 `32 Σ ceil(m_t/B)`、FAST FULL 的 `32 T+` 在 D=F128/TR16/KB4/NP2 下正确；这是源码 TDP 工作量，不是 PMU。原 TFS 两 output panel 还把 H gather 做两遍；新 block 的 source visits 为 E，而原模型为 2E，因此原 TFS→block 的收益并非只有 AMX 指令数变化。依据 `src/paper_methods_kernels.hpp:29` 和 `results/journal-p0-20261004/model.csv:2`。

## 2. 独立复算，而非照抄表格

从各 `time.csv` 按 graph/method/kind 取中位数，再逐图求比值和几何平均；仅使用同轮样本。

| 结论 | 复算值 | 数据 |
|---|---:|---|
| A3 轮 B256 FAST / 原 TFS | 1.319139761×，11/17 wins | `results/journal-p0-20261004/time.csv` |
| A3 轮 S64/MFULL / 原 TFS | 1.314231578×，11/17 wins | 同上 |
| A3 轮 S128/MFULL / 原 TFS | 1.318436340× | 同上 |
| S64/MFULL 对 rowwise FULL，去掉 Myciel/Reddit | 0.996603345× | 同上 |
| FAST 候选前两名差 <1% / <2% | 13/17、16/17 | 同上，候选固定为该轮 `_fast` 集合 |
| B256/FULL 投影范围差 ≤1% / ≤5% | 10/17、13/17 | `model.csv` |
| B64/B256/FULL 投影范围完全相同 | 3/17 | `model.csv` |
| P1 固定 Page vs S64/MFULL Degree | 1.012566747×，8 wins，3 图 >5% | `results/journal-p1-20261005/time.csv` |
| P1 固定 Source vs S64/MFULL Degree | 0.999484347×，4 wins，2 图 >5% | 同上 |
| P1 固定 Shuffle | 0.929103509×，0/17 wins | 同上 |
| P3 D2 / S64/MFULL ACC | 0.931715233× | `results/journal-p3-p4-20261005/p3/d2_time.csv` |
| P3 D3 / S64/MFULL ACC | 0.927018475× | 同上 |
| P4 压缩 mixed / 同 harness FULL | 1.284979150×，4/5 wins | `results/journal-p3-p4-20261005/p4/sel_time.csv` |
| P4 等宽 mixed / 同 harness FULL | 0.757946188×，1/5 win | 同上 |
| P4 压缩固定 1/4 / FULL | 0.973571715×，1/5 win | 同上 |

早一轮 `PROJECTION_WINDOW_RESULTS_20261004.md` 的 B256≈1.3263、S64/MFULL≈1.3195 来自不同 run；不能和本轮 A3 值混合，也不是 TXT 算错。

抽查 tar 内真实 stdout：

- P1 `RAW_LOGS.tar.gz!/logs/focus-10869326/reddit/methods.log:154` 的 Source 五次为 95.794916、95.376015、92.921972、99.697828、93.927145 ms，中位数 95.376015；与外层 `time.csv:147` 一致。
- P3/P4 `RAW_LOGS.tar.gz!/p3/logs/focus-10869493/reddit/residual.log:77` 的 D2 五次中位数 126.049995 ms，与 CSV 一致。
- 同 tar `p4/logs/focus-10869550/reddit/selective.log:56` 的压缩 mixed 五次中位数 53.368092 ms，与 CSV 一致。
- 独立 SHA 比较：P1 grouping runtime/schedule、P2 overlap、P3 residual、P4 selective 源码与对应 results frozen snapshot 相同。P4 superseded smoke 确实不同，不能混入主结果。
- 进一步全量对账：P1 外层2400条METHOD_TIME全部在tar真实stdout中找到完全相同字段/值（tar共2760，额外360为smoke）；P3全部200条D2_TIME、P4全部1000条SEL_TIME均逐条匹配，未匹配数均为0。P0检查1088/459/510，P1检查880/480/320/320，P3数值160、P4数值540，逐条pass均为1；D1失败是单独反例实验，不被这些统计覆盖。
- P0 README 明确原始 Slurm/stdout 与 build manifest 在服务器；GitHub 有逐次 CSV，但其证据完整性不能表述成“P0 所有原始 run 文件都已在 GitHub”。见 `results/journal-p0-20261004/README.md:18`。主审随后SSH只读读取23份methods.log（3910条含smoke/其他run），GitHub正式CSV的2890条逐字段全部匹配、0缺失；见p0_remote_raw_evidence.json和final_supplement_summary.json。

## 3. P0/A3：最有价值的机制证据，也有识别边界

Myciel A3→A1 的独立 PMU 中位数：

- instructions：78,562,713,366→81,381,803,132，增加约 3.59%。
- generic cache misses：9,431,084,663→4,040,087,066，减少约 57.16%。
- AMX busy：25,250,584→25,193,780，近乎相同。

TXT 的这个描述准确。相同 M 下输出逐位相同、投影 TDP 预算相同，可拒绝“更少 AMX 计算解释这 2.23×”的假说。**但它不能把效应唯一归因于缓存命中**，也没有分离内存级并行、编译器调度、访存/算术依赖、动态任务尾部。

A3 生成器保留源码级 partial load/store；编译器可能因 row-outer 循环而寄存器提升、消除 reload 或改变 spill。这里“相同状态请求”应始终加上逻辑/源码级限定；没有读取对应汇编前，不能升级成相同 retired load/store 或硬件流量。`projection_window_kernels.hpp:24` 自己也把 Counters 定义成 logical staging traffic，非 PMU/DRAM。

## 4. P1 不是全图最优分组，已有反例限制模型

真正的搜索空间很窄：先保留 DegreeSort，每个位置的 q64=ceil(degree/64) 不变；只在同 q64、最多4096行段中按 **前64个邻居** 的单个 source/page min-hash 排序。Page key 使用 `source_id/16`，是相对特征页代理，不是读取实际物理页。见 `grouping_schedule.hpp:12`、`:45`、`:54`、`:72`。

这轮证明“固定投影预算下，部分访问排列改变性能”，不能证明最佳 grouping 已找到；也不能从固定 Source 的负 GM 得出一切 source-aware 方法都无效。

一个用户 TXT 尚可补入的强反例：受控 `regular128_reuse1` 中 Degree→Shuffle 的 FULL 时间 **193.867→124.095 ms，约1.562×**，但 sampled source reuse p50 都为1、relative pages p50 都为1010、occupancy都为1（`group_structure.csv:70`）。这直接表明当前结构特征不足以预测执行性能；不是“提高 tile source reuse 就一定更快”的模型。

P1 prepared 不计排序/签名/分配/审计。Source Reddit 的 setup upper≈16.503ms，相对26.046ms的单次收益可以快速摊销；Products Source setup≈134.723ms、每次约4.962ms，需约28次。把所有图统一称“低预处理开销”不成立。见主报告 preprocessing 表与 `group_setup.csv`、`group_audit.csv`。

## 5. P2：其负结果只否定了一个具体交错方式

源码每个 AMX K-block 后调用 `reduce_rows(...begin,begin+2...)`，**这两行内部仍是先完整归约一行，再完整归约另一行**，并不是两行 ZMM accumulator 同时驻留。见 `experiments/avx_amx_overlap_p2_20261005/overlap_micro.cpp:16`、`:49`。

五案例 serial/interleaved 中位数比分别 1.087937、1.010928、0.831262、0.989256、0.988137。最重稀疏两个随机工作集基本持平/略慢；hot d64 明显慢。它不是完整两层或多核对照，也不是一切 AVX–AMX overlap 的不可能性证明。

TXT C1 与 P2 的机制确有区别，但 C1 也不能说旧代码没有独立流水：canonical 已有8个 acc、8段 feature load/decode 和下一邻居4条 cache-line prefetch（`paper_methods_kernels.hpp:52`、`:55`）。新点是增加多个目的行的独立 CSR/H 地址流，并避免16行反复状态写回。两/三行同时活跃给寄存器分配器更大压力，必须看真实汇编；32个ZMM容量账只是上界，不证明编译器会无 spill 实现。

## 6. P3：精度 PASS 不能转成主性能贡献

D1 的实际AMX反例 relative L2=0.001956947，严格FP32门槛失败，但最终 BF16相同。这是非常有价值的“最终输出门槛可掩盖中间错误”证据，应保留到期刊数值讨论（`results/p3-residual-gate-20261005/gate.log`；报告第1表）。

D2 不是原提案的 K+1；实现为 K 个 hi 主投影 + tile末尾残差 hi/lo 两投影，即 K+2；D3每4块修正。代码 `residual_broad.hpp:71`、`:79`、`:85`。若 U=总block、T=非空tile，则 D2=U+2T，B64ACC=2U，FULLACC=2T。因此 D2只有 U>2T 才降低相对B64ACC的投影，永远不少于FULLACC；当前RoadNet U=T时反而增加50%。TXT E9结论准确。

P3相对原TFS的严格门槛为 1e-3 relative L2 与全局 normalized max，最终BF16门槛1e-2，对源FP32 MKL门槛3e-2。它不是每元素相对误差保证，亦不是任意输入/真实模型精度验证。只有候选L1做26项profile，不能把其数值解释为L2剖析结果。见 `residual_broad.hpp:104`、`:112`、`:127`。

## 7. P4：候选失败是真实的，但尚有未隔离的执行因素

选择热点按source被消费次数，构造 compact map/Q；每个forward每层重建Q，已计入prepared。冷边 BF16 D维归约→hi/lo AMX；热边 FP32 F维Q归约；最终合并（`selective_kernels.hpp:19`、`:69`、`:101`、`:157`）。

关键代价边界比“D>F更有利”更具体：当前实现冷边2D字节、热边4F字节，所以单边请求减少条件是 **F<D/2**。压缩L1 128→32请求减半，L2 32→32请求翻倍。真实延时还含map依赖、分支、Q构建和merge，因此此式不是性能判据。

`project_all` 原source ID直接索引；`project_used` 按频率紧凑布局并查map；两者并非只差“是否算零消费source”。Myciel/Wiki等有用源数与非空destination数也不能互代。TXT应避免把used比all慢唯一归因于物化选择本身。该消融还未补齐，见 `selective_kernels.hpp:33`、`:111` 与解释文档第7节。

P4的全局/逐层对照已经相当重要。压缩mixed平均1.285×，而选择性1/4平均0.974×；这表明当前新候选没有超过简单强基线。等宽mixed平均0.758×。不过 all/used/mixed 的精度边界与FP32数学reference不同，结果是门槛接受而非所有路径逐位等价；不能将代数重排误写成浮点逐位重排。

## 8. Feature precision 与 cache-control

Feature实验两种输入都继续用BF16 W和BF16 partial AMX；“FP32路径”不等于FP32 GEMM。Native FP32还绕过层间BF16转换，改变输入量化和计算语义；只有matched-kernel控制使用BF16无损展开FP32，并做逐位检查。见 `experiments/feature_precision_8aeef16/SOURCE_AUDIT.md:21`、`:62`、`:76`。

RoadNet native prepared中FP32约快6%，但BF16单层约16.45ms、FP32约17.23ms，BF16单层仍更快；层间转换约4ms使总结果翻转。因此不能用此例说“去掉解码让RoadNet聚合更快”。大多数图BF16胜出的结论仍成立。

表示实验也没有隔离纯decode成本：FP32请求512B，BF16请求256B；四条prefetch均只覆盖前256B，对FP32只有半行覆盖；H0 native FP32串行first touch，而BF16并行转换，页分布可能不同。Default NUMA≠相同page placement。见同文件`:58`、`:72`。TXT E12整体正确，但“精度/表示对照”不能写成已经因果隔离缓存与转换成本。

Cache-control正式17图是 **独享 intel_expr**，而后续P0/A3、P1、feature、P2、P3/P4都是 **共享 intel**。Cache-control只插入每方法即时warmup+五次连续计时并提前return；冻结kernel字节不变，见 `scripts/build_paper_cache_control.sh:31`。它与先前交替轮不同时间/节点，不能把两个协议绝对时间之差全部归于cache。B64相对原TFS GM 1.309 vs 1.301只是跨协议稳健性证据。

## 9. 对用户TXT的建议修订和下一步最小证据缺口

1. E3/E9/E10 把 FULL 补成 S64/MFULL FAST 或 ACCURATE；E4则明确 rowwise FULL FAST。保持 2×2 Identity/Degree × rowwise/S64 的严格同轮对照。
2. E2“相同状态读写”加“源码逻辑请求”限定；不能表述为相同真实cache/DRAM流量。
3. C1保留为短机制验证，不称突破已成立；原代码已有8条特征累加链与prefetch。P2不构成C1重复实验，但主审进一步发现10/7 state/pair内核已有双行acc0[8]/acc1[8]驻留，因此未测范围必须缩为“无head匹配的独立行固定交错、整行驻留、不反复S64写回”的具体组合，而非笼统声称双行完全未做。
4. C2立项前必须保留P4 global/used/mixed强基线；对当前FP32 Q需明确F<D/2的字节边界，并补同源集合相同布局的对照，否则会混入map/布局因素。
5. 把P1受控reuse1反例纳入模型建设：只预测tile内复用率会遗漏大幅性能变化。
6. 继续把独享、共享、prepared、误差门槛与原始输入语义逐条保存，不能用某个 `paper_eligible` 布尔替代这些事实。例如P0 event标true，而submission实际共享intel。

总评：TXT的方向取舍相当审慎，数值基本可靠；需要加强的是跨实验命名与因果边界。现有实验没有证明新的期刊主算法，却已经有效淘汰了若干弱方案，并明确暴露“改变工作量后，原排序与访问组织不再合适”的研究问题。
