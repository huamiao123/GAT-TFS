# Crosslayer / partial bundle / ordered source 深审（2026-10-18）

审阅对象：父 agent 下载的 GitHub 9369b9e 快照，根目录 `C:/Users/花喵/tmp/tfs18`。本报告全部相对路径从该目录起算。只读原始源码、协议、日志和 CSV，未运行 benchmark，未改动任何冻结源码。另运行只读 Python 重算：`recompute_crosslayer_review.py`；完整数值保存 `audit_crosslayer_calculations.json`。

以下简称 C=`results/crosslayer-fusion-20261008`，P=`results/partial-bundle-20261013`，O=`results/ordered-source-plan-20261014`。数字是已存日志的重算，不能当作本次重跑结果。

## 1. 应立即修正的结论

### 1.1 “自适应在 Amazon/Reddit 无回归”没有被独立测量

最新冻结源码 `C/runs/recompute-window-adaptive-build-20261008-234221/source_snapshot/recompute_window_adaptive.cpp:206` 是 baseline lambda：第一层输出全图 FP32 H1，再独立 ReLU、转换成 BF16 H1b，最后第二层。

同文件 :208 的 fallback 明确只有 `bt=run_base(); memcpy(cand,base,...); ct=bt;`。因此两个相同时间是赋值结果，最终 BF16 exact 是复制后的必然结果，不是两次独立 forward 验证，也不是 selector 零开销的实验。排序在 :189、rho 分析在 :196，均在主计时前。fallback 还固定用了 min_source 顺序，不能推成相对于默认 DegreeSort 的无回归。

此错误已有发布 review 承认；本次直接核对源码和日志确认。合理表述是“分支选择了既有 materialized 算法”，不能写“实测自动回退不损失性能”。

### 1.2 老的跨层融合收益没有对照已经存在的 PACK_EPI 强基线

跨层 baseline 上述 :206 保存 FP32 H1并做两次独立全图操作。候选在 `recompute_window_kernel.cpp:103` 开始直接 ReLU+截断写 BF16 源缓存。于是对照同时改变了跨层调度、H1 表示、epilogue 和缓存分配。

但更早的 `C/runs/state-reuse-20261007-05/source_snapshot/state_kernels.hpp:240` 已实现保留原语义的融合 epilogue；:120 的 DIRECTPACK 已减少末端 partial 暂存。其协议明确要求把 PACK_EPI 当后续强基线。

所以即使 RoadNet 跨层比旧 FULL 快，也不能全部归于跨层融合。必须补充“相同顺序+相同 epilogue 的 materialized PACK_EPI”控制。尤其 RoadNet 正是 epilogue 的显著受益图，早期独享记录 PACK_EPI/P0 提升约 1.65×，所以这不是细枝末节。

### 1.3 统计口径存在两种不一致

最新 adaptive :214 对六样本排序后取 `[3]`，这是上中位数。RoadNet 标准中位数为 **57.65247345 / 50.395488739 = 1.144000681×**，不是 1.165×；candidate CV=16.20%，baseline CV=5.73%，仍是 shared exploratory 信号。

更早的 interleave 冻结源码 `C/runs/recompute-window-interleave-build-20261008-230314/source_snapshot/recompute_window_interleave.cpp:223` 在 PAR_RESULT 取 `[3]`，而 :227 的 PAR_GATE speedup 取 `[1]/[1]`（第二小值比）。因此旧会话约1.34×/1.31×来自不同分位，并非标准中位数。逐样本复算：

| run 末尾 | 图/顺序/batch | baseline/candidate 通常中位数 ms | 比值 |
|---|---|---:|---:|
| 230519 | RoadNet identity /16 | 70.426941/54.899454 | 1.282835 |
| 230519 | RoadNet min_source /16 | 78.112483/58.786392 | 1.328751 |
| 231443 | RoadNet identity /64 | 56.795001/40.473461 | 1.403265 |
| 231443 | RoadNet min_source /64 | 59.285522/43.132544 | 1.374496 |
| 230957 | Amazon degree /16 | 59.893608/340.351462 | 0.175976 |
| 230957 | Amazon identity /16 | 53.027987/269.381881 | 0.196851 |
| 230957 | Amazon min_source /16 | 51.587939/301.580429 | 0.171059 |
| adaptive 234255 | RoadNet min_source /64 | 57.652473/50.395489 | 1.144001 |

都是各自 run 内比较，不跨节点混分母。正信号没有因修正消失，但强度、稳定性和归因都不能夸大。

### 1.4 hot_source 负对照实现存在初始化缺陷

实际构建源码 `C/runs/source-partition-weighted-build-20261008-232709/source_snapshot/source_partition_hot.cpp:32` 将 `hot_src` 初始化为 0；:104 只给 min/max 赋首邻居，未给 hot_src；:105 只有某邻居的行度严格大于源0的行度才替换。

例如源0行度100，当前行邻居为行度10/20的源3/4，得到 hot_src=0，而0根本不是当前行邻居。故它不保证实现“最热邻居锚点”。同时它按源行自身 degree 取热点，这代表生成成本，而不是消费复用次数，两者也不等价。

未拿到图数据，无法本地统计这项缺陷实际影响多少行；不能宣布所有结果数值都错。但旧“热点源聚类失败，因此这条策略排除”的结论应降为“这份有缺陷/特定定义的启发式没有改善”，不能据此排除正确热点策略。

### 1.5 源缓存不是小片上缓冲，也未证明总内存/DRAM减少

adaptive :31 为每线程分配 N 字节 mark，:45 按整个线程分区 unique 源数分配 SourceCache。`recompute_window_kernel.cpp:32` 令 data=capacity×128 BF16、slot_of=N个int32，另有 ids/free_slots。整个 W0 分区期间从不 erase。这个路径跨批复用的是大规模软件缓存，不是容量受限的 L1/L2 tile ring。

RoadNet 最新 source_rows=2,043,040，逻辑缓存 payload=523,018,240 B；全图 BF16 H1=1,971,281×256=504,647,936 B。仅 mark+slot_of 就约315.4 MB（32×N×5），还没加源ID数组、unique容量和分配开销。旧 FP32+BF16 H1有约1.514 GB，因此候选可能比旧表示少一些，但这不是相对于 PACK_EPI/BF16-only 表示的省内存证明。

benchmark main :193 始终保留 baseline H1/H1b，候选运行时它们也没释放。没有独立 RSS峰值或真实DRAM traffic对照，不能把“没有单一全局 H1 矩阵”写成“省了 H1 内存和DRAM访问”。`peak_cache`还仅取各线程最大值，不是全线程总峰。

## 2. frontier→window 结构模型与真实 kernel 并未闭环

frontier 本身研究的是固定串行 producer 顺序、BF16源状态256 B、FP32 partial512 B之间的状态保留；`C/experiments/frontier_structure_20261008/PROTOCOL.md:11` 起清楚声明无重算、无复杂spill等限制。capacity-two matching 是该限制下下界，不是一般算法下界，更不是可执行全局调度。

Reddit证书最大有效下界56,528,640 B，相对简单 row-complete 的 postdrain 峰59,162,368 B，仅留约4.45%空间。Myciel下界不紧。参见 `C/runs/frontier-certificate-20261008-02/compute-10899028/reddit/verified.json` 和 myciel对应文件。证书有效的研究意义是排除这一固定顺序模型下的“大幅状态压缩”，不是证明跨层融合整体不可做。

window scanner另有更严重的定义/实现问题：

- `C/runs/recompute-window-20261008-04/source_snapshot/window_scan.cpp:17` 定义 consumer rank 为该 destination 最大邻居ID/16+1；:30 producer rank 为 sourceID/16+1。这是 node-ID生产、row-complete消费的结构代理。
- 同一行 :17 的 `degree[j]++` 统计源的列度/消费者数，而 :42 将它用作 H1_j 重算的第一层边数。一般有向CSR应是 `row[j+1]-row[j]`；只有对称结构或恰好行列度相等时数值才一致。当前原始图是否每个都满足，不能不核验就假设。
- :46 将 `live`（行数）与 `ceil(events/16)`（AMX组数）相加，输出 `peak_rows`。缺少明确定义的瞬时执行模型，单位不一致，不是合法缓存行峰值证明。可能希望计一块group scratch，但不能每rank把组数当源行数。
- 后续 serial kernel `recompute_window_kernel.cpp:274` 使用 DegreeSort destination 顺序，:130 的 next_use 是 destination tile rank，生产也按需生成。两者不是同一个 W 的执行语义。

因此旧 RoadNet W1024 结构预测额外173,153源行不能直接对照kernel；真实kernel日志为额外1,687,277（还是按source_rows-N计算，而非减实际used sources）。这种差距不能简单说“模型准确但缓存慢”，首先要统一模型。

真实 serial sweep Job10900360九个正W全部 BF16 exact，但全部比同run materialized慢；W0有正信号。注意 `recompute_window_kernel.cpp:153` 的 next-use构造和缓存分配发生于 :164 计时前，正W容量固定20,000行（:300）。这批数据应表述为prepared串行内核，不是含计划构造的端到端部署速度。

## 3. 跨层可保留的真实信号

`C/runs/recompute-window-parallel-exclusive-10901902/program.log:4` 等保存实际独享结果：

- RoadNet DegreeSort：39.532/70.284=0.56246×；identity：31.248/30.419=1.02725×；min_source：33.013/29.920=1.10338×。
- 同次 Amazon min_source：10.690/23.940=0.44653×，其余更差。更早共享Amazon正结果被独享反证，不能再保留为支持证据。
- RoadNet source行生成由DegreeSort 3,627,178降到min_source 2,043,040；改变顺序会显著改变跨线程重复生成量，这是成立的机制证据。
- 跨destination tile打包把min_source source_projection_groups由184,732降到batch16的131,318、batch64的128,585。逻辑工作减少真实存在。它是第一层待生成行的AMX填充改善，不是邻居聚合 B 参数，也没有证明AVX/AMX并行重叠。
- `C/runs/recompute-window-batch-20261008-222837/program.log` 的独享36条结果全部exact，外层FAILED是预期计数错；每个case三次独立调用有明显baseline漂移，不能挑最大ratio。修正记录保留是正确做法。

这支持“跨线程源重复工作/源AMX填充是跨层融合的重要代价”，尚不支持“已得到通用自适应融合加速器”。阈值1.25没有留出运行验证，rho也未完整覆盖projection、尾部、缓存footprint及线程最大负载。

## 4. 10月7–8日全部19个实验目录清点

这不是19项互不重复的创新。多项是同一假设的控制、失败重试和更强证据。

| 目录（C/experiments） | 真正问题与当前证据 |
|---|---|
| grouping_balance_20261007 | qsource/qpage/qsamplepage/qguard，在q64桶内改变destination顺序；固定工作与精度。qguard 17图交错GM1.03108×，连续1.02985×，qsource约1.0015×。是局部性保护启发式，不是硬件cost model。 |
| order_balance_20261007 | 每层A=先聚合、Pall/Pused=先投影后聚合；精度匹配容差，保留全局FP32 Q，包含32与128两种形状。128等宽六图shared固定Pall_Pall相对A_A仅0.6230×。不能用降维32的收益替代128基线。 |
| state_reuse_20261007 | NOZERO/DIRECTPACK/EPILOGUE与动态PAIR拆开；PACK_EPI是真正可靠的强基线建设。17图交错PACK_EPI/P0=1.20180×，连续1.19872×；PAIR_SHARE对PACK_EPI交错0.99562×。 |
| pair_plan_20261007 | 保持TR16成员与AMX行槽，优化pair和exact window共享。17图opt_exact_s交错1.01321×，连续0.99965×；增量不稳定。 |
| pair_order_control_20261007 | synthetic-only，同一plan内存/同一执行对象，仅换完整pair访问顺序；排查2.3–2.5×顺序效应，不是原图通用收益。 |
| order_share_factorial_20261007 | 将顺序与exact sharing因子分开，三个固定seed；诊断4图四队列/协议GM0.99817–1.00325×，扩展门槛失败。 |
| access_prefetch_20261008 | PF开关/提前距离的访问时间控制；已读协议和最终审计事件，546 PMU regions等门槛通过，固定17图扩展失败。此项未在本分报告逐个反汇编深审，应由主审补充。 |
| event_trace_20261008 | 2-bit动态head消费事件提前编码，same-event replay对比动态；trace_s_rep四种队列/协议诊断4 GM0.97079–0.98517×，明显未胜PACK_EPI。 |
| frontier_structure_20261008 | 结构状态模型+matching证书；非性能，固定顺序/表示下的有限结论。 |
| recompute_audit_20261008 | 再生成源行的逻辑work扫描，前四版本失败/缺记录、v5通过；无运行加速。 |
| recompute_window_20261008 | W窗口结构扫描；定义与内核不同且degree/单位问题见前文。 |
| recompute_window_kernel_20261008 | 单线程合法计算原型与九W sweep；正W全部慢，W0正信号，finalBF16 exact。 |
| recompute_window_parallel_20261008 | 静态destination分区+每线程源缓存；RoadNet排序依赖，Amazon/Reddit负边界。 |
| recompute_window_batch_20261008 | 将多个destination tile的缺失源统一凑AMX16行；真实减少投影组数，独享原始事件计数需修正。 |
| recompute_window_interleave_20261008 | 交替测量控制实验顺序；不是NUMA interleave。统计分位bug已重算；日志曾误标exclusive，须用修正/Slurm。 |
| recompute_window_adaptive_20261008 | rho<=1.25分支原型；只有RoadNet实际运行fused，fallback非独立计时。 |
| source_partition_audit_20261008 | 等destination分区的unique source duplication；结构筛查，分区按行与kernel按tile存在边界差异。 |
| source_partition_weighted_20261008 | 用源row-degree加权复制量；heldout/RoadUSA只有结构记录，无heldout推理速度。 |
| source_partition_hot_20261008 | hot_src默认0缺陷；不能据此排除正确热点聚类，且“生成成本”不等同“消费热度”。 |

对早期家族，本次逐个读取协议、最终handoff事件、原始计时CSV，复算所列对照；抽查关键实现，而非声称读完每个重复快照/每条ASM。具体关键源码：grouping `grouping_schedule.hpp:164` 是LRU估计筛查；state `state_kernels.hpp:120/240` 是direct pack/epi；pair `pair_plan.hpp:251/275` 通过完整序列memcmp决定exact标志；trace `trace_reducer.hpp:42/56` 依mask执行仍逐边add。这些路径没有把共同邻居合并成一次公共和，也没有真实流水并发设计。

1.20180×的P0分母定义已定点核对：`C/runs/state-reuse-20261007-05/source_snapshot/state_runtime.hpp:12/14` 的 p0_degree 为kind2、scope0(FULL)、schedule0(DegreeSort)、accurate=false(FAST)，:37注入window64(S64)。PACK_EPI :16保持FULL/FAST/DegreeSort，:28同样S64，新增NOZERO+DIRECTPACK+第一层EPI；:41–44给出完整两层路径。它是相对S64/MFULL/FAST解耦P0的同精度工程增量，绝不是对原TFS新增20.18%，也不是共享decode的收益。

## 5. Partial bundle：方法真实，收益有限，归因边界清楚

`P/source_snapshot/bundle_kernels.hpp:46` 保留partial与C，每个G保存独立hi/lo；:90–117先分别归约/量化G个M-scope，再按同一panel顺次消费每个scope（含ACCURATE hi/lo）。所以保留量化边界和每个输出的投影累加顺序，主要减少C保存/重载；没有减少逐边source读取、decode或AMX TDP。

我从 `P/logs/bundle-10944719/state_time.csv` 独立复算230个方法/协议/图分组，全部与timing_summary一致。M64固定G4五图GM1.0203 FAST/1.0254 ACCURATE；M256固定G4约1.0099/1.0123。共享节点、增量小。FULL每tile仅一scope，G4无C节省却也有约1–3%时间波动，因此不能将所有正值归因C traffic。

物理解释仍需谨慎：所省C逻辑bytes只约source请求的3–6%，而C是局部状态；不能把它当同等DRAM bytes相加。G4显式hi+lo数组32KB，再加partial和C共16KB，source层面约48KB工作数组，FAST时编译器实际栈保留应看生成代码，不能只用active packed容量宣称实际stack/RSS。

这项足以作为“量化边界与C驻留边界可以分离”的正确性/机制对照，但当前没有强通用收益，不应单独包装为期刊突破。

## 6. Ordered source plan：必须正视可执行覆盖率，不是集合重叠

`O/source_snapshot/ordered_plan.hpp:36` 使用原序current-head贪心事件；:48固定eligible=matches>=8且nruns<=8；:76起独立展开核验合法序列；:129起运行时按tile/window退回ScalarReducer，:177起只在合法shared事件复用load/decode。每个真实incidence仍执行128维加法，每行浮点顺序不变。

我从 `O/raw/ordered-10955178/state_time.csv` 独立复算90组，全部与timing_summary一致。固定StaticShared五图GM：交错1.0069×、连续1.0046×。最高覆盖Myciel仍0.9980×；Products仅428/123,718,152边(0.000346%)可共享；RoadNet为0，wiki8边。因而Products约1.68%正差及RoadNet小正差不能解释为省decode取得的收益。

计划构造 Products约1.16秒、Reddit约0.883秒、Myciel约4.12秒，表中乐观摊销256/398次等只是观测小差值的除法。Myciel plan allocated约129.46MiB，不能无成本加入每次推理。

它比原event_trace新在对可压缩有用事件做选择，不是首次“静态执行计划”。关键科学信息是：固定DegreeSort相邻pair+原CSR顺序+8/8压缩限制下，可执行复用很少。不能由这项失败推论所有共享策略失败，更不能把集合交集量当实际合法共享量。

另外load/decode/add纳秒分段不能做瓶颈百分比。`O/timer_calibration_summary.json` 原计时空区间约97.465%为0，最小正差953.674ns，均值中位数25.469ns；拆段改变寄存器生存期/调度且计时量化粗。ms级完整E2E仍有效；微段相近不证明每个实际硬件成本相同。

## 7. 对下一步研究问题的判断

当前最扎实的结果是强基线工程（PACK_EPI）和一组可定位的失败边界。跨层融合尚有研究价值，但先修复量化比较/计时/模型映射，不应继续基于旧1.16×与“零回归”设计更复杂selector。

一个有区分度的研究问题应是：在保留非线性/量化边界的前提下，是否能用真正容量受限的中间表示，在源复制工作、AMX填充和可达消费顺序之间取得超过BF16-only materialized强基线的收益。这里不是机械移植GPU融合，而是明确处理“省中间量”与“重复生成”的矛盾。

另一个仍开放的问题是改变合法共享机会本身，而非在覆盖率近零的pair集合里继续换压缩格式。但需单独比较改变行分组/执行顺序后padding、负载、metadata、精度与准备成本，不能把已有约1%波动当机制收益。这只是基于当前证据的研究建议，不是完成的新方法或首创性声明。

所有上述数字限静态单位边权、seed12345随机H/W、主要128→128→128、单类AMX CPU。尚未得到真实模型任务准确率、归一化/一般边权、形状泛化与独享跨图稳定收益。NUMA是否在整个仓库其它实验违反约束由主审合并；本报告不从协议中的no_interleave字样推定全仓符合。
