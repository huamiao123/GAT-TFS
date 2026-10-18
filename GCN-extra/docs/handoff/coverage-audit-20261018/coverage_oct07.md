# 10 月 7–8 日八个实验族的补充深审与覆盖账本

审阅快照：GitHub `9369b9e0491d59e328a99153b6d97314c62285da`。本文件中的 `C/` 指 `GCN-extra/results/crosslayer-fusion-20261008/`。只读源码、保存的原始日志和 CSV；没有重新执行内核或提交集群实验。

这不是“所有文件都逐字看完”的声明。此次补审覆盖八族的**实际执行路径、最终变体、失败版本、全部已保存主计时及主要控制链**；每份庞大反汇编未逐指令重新审计，所有大型真实 CSR 的结构统计也未在本机重放。前轮只用“跨层族”概括这批工作，会遗漏它们实际是层内归约/访存控制的独立研究链。

## 覆盖深度与数量

八族共 **13 个 run 版本、62 个有日志的作业目录、37,110 条主计时**。142 份 graph-local `*_time.csv` 与相应 `methods.log` 的 graph/method/kind/shape/repeat/ms 全量逐字段对账：**0 不匹配、0 原始计时遗漏**。每个方法的中位数由原始日志重新计算，未直接照搬报告汇总。早期 grouping/order 的 smoke 也打印计时，统计数量包含它们；它们不进入真实图性能 GM。

| 实验族 | run 版本 | 有日志 job | 主计时条数 | 本次 review_depth | 仍未重新核验的内容 |
|---|---:|---:|---:|---|---|
| grouping_balance | 1 | 8 | 8,100 | 五种排列生成、保护规则、M64/256/FULL、全 raw、资源声明 | 17 图全 CSR 的 minhash/LRU 机会统计独立重放；真实缓存机制 |
| order_balance | 2 | 5 | 7,410 | 18 种两层顺序、两种形状、precision reference、Q 重建、全 raw | 全 PMU 线程计数重算；实际应用精度 |
| state_reuse | 5 | 13 | 5,160 | 编译失败、三轮语义失败、最终 nozero/directpack/epi/pair、全 raw | 全部热循环反汇编逐指令重做；完整输入精度证明 |
| pair_plan | 1 | 12 | 8,850 | partner/窗口精确匹配/动态匹配、17 图与三次合成图 cohort、全 raw | 17 图全部配对元数据本地重建；完整汇编因果审查 |
| pair_order_control | 1 | 3 | 450 | 同地址 scratch、pair membership 与访问次序分离、全 raw | 性能硬件成因、真实图泛化（该轮仅 synthetic） |
| order_share_factorial | 1 | 7 | 2,730 | 三个预定 seed、13 方法、固定执行对象、扩展门槛、全 raw | 全 seed 的统计显著性；更多真实图（门槛未过） |
| access_prefetch | 1 | 7 | 2,730 | PF0/1/8 目标多重集、scope 前奏、fixed slot、全 raw | 缓存层级/MLP 因果；最优预取距离（本轮未搜索） |
| event_trace | 1 | 7 | 1,680 | builder/独立 validator/三种 replay、malformed gates、机会诊断、全 raw | 本机全真实 CSR 独立 replay；完整 codegen 比较 |

本轮阅读的最终实验源码与每个 performance run 的 source_snapshot 比较一致；例外是后补分析脚本版本，例如 state run05 的 `analyze_state.py` 与实验目录最终脚本不同。因此性能数字从 frozen `methods.log` 重算，不能仅凭最终分析脚本推断当时执行。旧失败 run 的源码不同是预期版本演进，已保留。

资源分类以 **launcher command.json 是否带 `--exclusive`** 为准，不能只看 `SLURM_JOB_PARTITION`：grouping 10887942、order 10887943 是 `intel --exclusive`，属于普通分区整节点独享；其他多轮使用 `intel_expr --exclusive`。原 grouping 专用分区请求 10887790 在 PENDING 时取消，没有计算样本。

## 1. grouping_balance：不是只换了一个分组名

源码 `experiments/grouping_balance_20261007/grouping_schedule.hpp` 生成 degree/qsource/qpage/qsamplepage/qguard 五种排列。每个位置固定 `ceil(degree/64)`，并限制在同 q64 的至多 4096 行段内；这保持 M64、M256、FULL 的投影组数和 S64 状态预算，但不保证 `maxdegree×16` padding 步数相同。

qsource 用前 64 邻居的 source minhash；qpage 用 `source_id/16` 的 minhash；qsamplepage 用全行最多 64 个均匀位置。qpage 的“page”是 128 维 BF16 行按相对 source ID 分块的代理，不是从实际虚拟地址读取的物理页/DMA/NUMA归属。

qguard 不是 per-tile 最优选择器。它只在高 q64 tile 上抽至多 128 个 tile、各 first/middle/last 窗口，使用容量 256/1024/4096 feature-row 的 LRU；要求三个容量都不退化、至少两个改善 5%，再选择一个候选用于高 q 行，低 q 行保持 degree。它比较的是**取样串联轨迹**，不是线程并发真实缓存模型。

独立 raw 重算：MFULL qguard 对同轮 degree，shared17 两种计时 GM **1.029854 / 1.031078**，胜 **12/17、10/17**；`intel --exclusive` 诊断四图为 **1.117632 / 1.132474**，均 3/4 胜。不能把诊断四图 GM 当全17。

一个细节：`structure()` 的 `{0,nw/2,nw-1}` 没有像 guard_probe 一样去重。nw=1 时同窗计三次，nw=2 时第二窗计两次。因此 `GROUP_STRUCTURE` 的采样分位数包含重复窗口加权；这不改变运行内核，也不改变 qguard 的实际选择，但不能称“均匀不重复窗口样本”。

## 2. order_balance：实际已经做过两层 3×3 顺序组合

run01 在测量前被 run02 的诊断计数修正取代；run01 没有 jobs。run02 最终共有每形状 18 方法：每层独立 A/Pall/Pused，再交叉 degree/qsource。实际 coverage 是 **6 个 shared 真实图 + 4 个整节点独享复核图**，不是17图。

`selective_shape.cpp` 和 `selective_kernels.hpp` 核实：A 使用 BF16 H→FP32 归约→hi/lo 两次 AMX；Pall 使用全源一次 BF16 AMX投影→FP32 Q→AVX归约；Pused 只投影被用到的源，但带 compact consumer-rank 布局和每边 map lookup。因此 Pused/Pall 差异不是单纯少算几个源。每次 timed forward 都重建 Q，未把输入相关 Q 偷放到 setup 外；source rank/map、分组和分配在计时外。

128→128→128 的 Pall_A 对 A_A，shared6 两协议 GM **0.756173 / 0.761994**，均仅1/6胜；128→32→32 则 **1.439348 / 1.450804**、5/6胜。独享四图同样呈现等宽退化、窄输出获益。这比“先聚合或先投影尚未比较”更明确：**已经比较过完整的逐层顺序空间，只是当前不是受容量限制的在线生产—消费方案**。

FP32 Q 每边为4F字节、BF16 H每边为2D字节，所以 F<D 不足以保证更少读流量，需要 F<D/2；此外还有Q生产、映射和状态流量。这个结论可直接由代码得到，不依赖PMU归因。

## 3. state_reuse：失败版本解释了为什么不能只看最终表

版本链：

- run01：编译失败，旧源码的 TR 宏被 namespace qualification 展开为 `::16`；无运行样本。
- run02/job10888776：10个fixture均在额外ReLU语义检查失败；0计时。
- run03/job10888795：noinline测试仍失败，确认 `0x00010000/0x80010000` subnormal 被 compiler MAXPS/DAZ 路径变成零，而原 compare+masked-store保留原bits；0计时。
- run04/job10888814：整数mask修复subnormal后，vector NaN仍被优化为零；0计时。
- run05：局部 `#pragma float_control(precise,on,push)`，ordered compare+整数bit清零+原高16位截断，所有10 fixture通过，才进入真实性能。

这不是误差门限放宽。最终生产ReLU语义还保留负零、NaN payload和DAZ相关位型；19值语义fixture与有限数值benchmark严格区分。原始失败日志全保留。

NOZERO、DIRECTPACK、EPILOGUE 被分别消融；`state_identity` 是无算法改变wrapper以暴露codegen/dispatch差异。最终PACK_EPI对P0的shared17 GM **1.198716 / 1.201796**，PAIR_I对PACK_EPI **0.996580 / 0.995617**，PAIR_S为 **0.994806 / 0.995615**。所以共享加速不能借用epilogue的20%收益。

双行驻留事实边界：`pair_reducer.hpp:213`附近确有 `__m512 acc0[8],acc1[8]`，two-row greedy head merge，能够在一个S64窗口里保留两行各8个累加器；每行自身CSR加法顺序不变，unsorted/duplicate也合法。但 `finish_row` 在非terminal窗口仍写回partial，下一窗口再load。因此此前“C1双行驻留全部没做过”不成立；**全行独立轮转、无head比较、避免每S64写回**仍不是当前实现。源码数组声明也不是无spill的证据，最终还需看相应unprofiled汇编。

## 4. pair_plan：分清“动态发现共享”和“静态精确窗口”

计划始终不改变TR16成员/AMX行槽，只改变tile内pair。opt在每行first/middle/last窗口取4个minhash，候选至多3个伙伴加原adj，有限真greedy score后配对；小于原adj采样分数则退回。这个回退仅保证**采样score**不低，不保证全行复用、实际缓存或时间不退化。

MODE0 scalar；MODE1/2 dynamic independent/shared；MODE3/4仅对长度相等且CSR序列memcmp完全相同的窗口做lockstep independent/shared，其他窗口scalar。ExactS不仅少decode，还少第二行current CSR读取；每行next-index和原prefetch保留，所以不能把两者差值全部命名“BF16 decode成本”。

Myciel实际结构：adj dynamic共享85,133,168边、exact仅5,953,578边；opt dynamic共享164,570,245边、exact15,678,698边，总E903,194,710。**提升动态可共享率也未自动转化为性能**：optDynS对PACK_EPI shared17 GM **0.977725 / 0.993672**。optExactS是 **0.999650 / 1.013214**；协议差别、setup成本和共享/独享复核均不支持把1.3%宣传成稳定突破。

合成 regular128_reuse1 无任何共享，但randScalar显著快于adjScalar，促成下一轮地址控制。此时random改变pair membership和访问顺序，不能直接归因为共享或只归因为顺序。

## 5. pair_order_control：已排除一部分地址与pair混杂

kernel源码与前轮byte-identical。使用一个固定容量/地址的scratch Plan，通过std::copy替换内容；feature/weight/output地址也固定。四种计划分别adj、opt、rand、pairs-shuffle；pairs-shuffle仅重排完整pair，pair成员/方向不变、odd tail保持最后、exact mask同步重映射。

全部slot路径跑同一个MODE0内核，故该轮不做共享；copy和warmup在计时外且单独报告，不是可直接部署的算法。三张合成图、shared/exclusive各一轮，没有真实图。

adjScalar/pairsScalar 的synthetic3 GM：独享 **1.578401 / 1.568053**，shared **1.598371 / 1.605752**，均2/3胜。这个控制说明既有特定合成图收益无需减少算术/共享即可发生；但没有证据把它定性为哪级cache冲突，也没有真实图广泛加速结论。

## 6. order_share_factorial：不是随便挑一个随机seed

四种order=adj+3个预定seed，三种mode=scalar/exactI/exactS，外加PACK基线，共13方法。fixed scratch之外还用固定地址、固定短名的 `slot_exec` 对象，检查实际timed object与数值gate对象的四边界一致，避免method字符串/对象布局混杂。

扩展17图门槛预定为s1ExactS对PACK，两个资源类×两个计时协议全部GM≥1.03且至少3/4胜。实算为：exclusive **1.003250 / 1.000839**，shared **0.998172 / 1.000142**，所以没有扩展17图。这轮明确把“合成图顺序效应”与“真实图可用候选”分开；不能只取某seed最佳数值。

## 7. access_prefetch：已经做过严格的发出时机控制

adj/s1×Scalar/ExactS×OFF/PF1/PF8，共12加PACK。PF8不是单纯 `k+8`：在每行每projection scope首次访问发出1..min(7,L-1)前奏，之后发k+8。源码中 `first = ws == scope_start`，前奏不会每个S64窗口重复。于是PF1和PF8目标多重集都是1..L-1、每目标四条T0 hint；ExactS仍给两行分别prefetch。

因此该轮真正测试的是**相同预取目标/次数下的发出时机**，而不是通过减少prefetch数量偷换工作。与此同时前奏突发、指令调度等本来就是treatment，不能视作纯延迟单变量。PF0仅关软件hint，没有修改硬件prefetcher。

预定推广候选s1ExactSP8对PACK，exclusive GM **1.007716 / 1.008566**、shared **1.013238 / 1.015343**，各2/4胜，门槛未过。adjScalar PF8/PF1的诊断收益约1.1–2.4%，不能扩大为普遍新方法。PF1与旧planned路径的汇编identity报告存在；本次读源码并核对应snapshot，但未重写该规范化反汇编算法。

## 8. event_trace：在线匹配已经被移出过热循环

trace builder把合法greedy消费事件编码为2bit take1/2/3；每pair/window有1byte count，即使已经exhausted也可能有count0 header；每record独立向上取整payload字节，不能全图只做一次ceil。Trace不保存source ID副本、不重排行内加法。

三种executor：TraceI保留两个head和独立decode；TraceSCached保留head请求但共享decode；TraceSRep以每event代表索引进一步减少current CSR读。语义、固定地址、完整trace validator、malformed fixture都先于timing；preparation与trace常驻容量单独记录。全部variant包括PACK都带同一份trace常驻，所以RSS不能用来推单独部署PACK的额外内存。

缓存TraceS相对DynS诊断四图GM **1.014–1.026**，说明移除在线发现有一小部分收益；但TraceSRep对PACKdegree为 **0.971–0.985**，对PACKqguard为 **0.868–0.882**，全8个推广门槛均不通过。不能只报告“比动态共享快”而忽略更简单PACK基线。

这一轮另有 **S64 vs full-row greedy的结构机会扫描**：Myciel共享85,133,168→99,729,155，额外仅占E约1.616%；Reddit1,916,748→2,879,498，额外占E约0.840%。这是固定pair的机会，不是full-row实现性能。其unsorted反例 A=[0]×65、B=[0]×63+[1,0] 中S64共享64而full greedy共享63，已保留；因此“放宽窗口一定改善共享”仅对相应sorted事件模型成立。

## 这批证据对后续路线的约束

不能再把普通两行驻留、pair interleave、静态配对、同地址顺序、预取距离、离线消费轨迹等整体列为从未探索。现有链条已分别控制过精度、地址、pair membership、CSR次序、prefetch次数、dispatch对象和强epilogue基线。

但也不能据此宣布AVX-AMX合作或共享归约全部无价值：这些路径仍主要减少**load/decode**，每条边的128次加法仍保留；PairShare甚至是同一源值加进两个独立acc，未提取公共子和。下一候选若改变算术量（公共子归约）、全行状态生命周期或容量受限投影复用，必须明确与这些已失败机制的差别。

所有这八族的核心新performance对照仍基于DegreeSort或qguard、S64/MFULL/PACK_EPI；没有后来FULL Identity×PACK_EPI×行访问组织的完整交叉控制。因而不能用旧共享失败直接推断新baseline下同样失败，也不能沿用旧较弱baseline包装新收益。

## 可复查产物

- `coverage_oct07_scan.py`：从所有graph-local methods.log读取原始行、gate失败、结构记录和中位数。
- `coverage_oct07_raw.json`：13 run/62 job逐packet证据清单、所有37,110计时行与中位数。
- `coverage_oct07_analyze.py`：142份CSV全量对账、source_snapshot一致性检查、按实际exclusive声明分组计算对照。
- `coverage_oct07_stats.json`：各cohort所有图内比值、source匹配与对账明细。
- `coverage_oct07.json`：简洁run/job lineage及review_depth/未核验项，方便主审总覆盖表合并。

准确的完成表述应是：“已补审这八族的独立实现链和全部保存主计时，失败版本与控制实验已入账；剩余汇编/真实CSR重放明确列出。”不应把目录覆盖或计时解析等同于每项实验已完全证明。
