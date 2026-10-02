# 保留 ICPP TFS 的块投影与 AMX PH：实现、实测与未解决问题

日期：2026-10-02。完整模型作业10854118，PH 布局控制10854131。两者均已完成，原始日志和源码快照保留。本轮没有提交新的 GitHub commit。

## 1. 结论

1. 保留 DegreeSort、destination TR16、OpenMP 动态 panels、真实 AMX SpMM→GeMM 消费和独立 Online 状态，可以把逐邻居投影改为逐块投影。products 第二层 AMX 调用分别减少13.80/23.55/35.60倍，但三层最佳只相对原B1快1.259倍。
2. 单 head 宽聚合仍处理 D=256：有效 sparse FMA 为258,390,124,544，仍是 transform-first sparse 部分的8倍。减少AMX投影不等于解决宽聚合。
3. 独立 heads 作为 PH 的矩阵行，可以共享 source H 并用 AMX 完成宽聚合。直接 BF16向量打包和8行配置的 PH 控制有局部收益；products 双分量相对较快AVX控制约1.237倍。尚未接入完整三层，不能称整模型加速。
4. 当前最快完整 TFS 候选仍慢于纯GAT B0 BF16，原 FP32 master绝对误差.003门仍失败。正确的内部指令实现与完整模型精度验收分开。

## 2. 原 GCN TFS 为什么不能只用“计算量放大”解释

原源码 [amx_tfs_v3.cpp](../../GAT/code/code/amx_tfs_v3.cpp:135) 的真实顺序是 TR16→output panel→neighbor→K block→AMX H_jW。有效执行 FLOP约2EDF，而两阶段GCN数学工作约2ED+2NDF。用户 products/D=F128 的例子约增加36倍；当前GAT第二层约43倍，属于同一数量级。

主要机制差异是：原kernel直接读取静态BF16 H；GAT需要动态独立head的score/exp/p、pH转换和staging。原GCN删除的AH只被后续W消费一次，GAT B0的HW则可跨边复用并降低每head聚合维数。必须同时比较AMX外的准备成本、实际矩阵指令、缓存复用和中间矩阵价值。

原 [gcn_e2e_bench.cpp](../../GAT/code/code/gcn_e2e_bench.cpp:103) 的 TFS不接收values，MKL却使用csr.values；只有确认unit-values等条件才能认为同邻接数学。原BF16转换截断，当前GAT采用RNE；原文products kernel6.67倍为32线程/128维，相应两层E2E4.09倍。以上均不能直接用作当前16核/256维/三层GAT的加速证据。

原文Eq4表达“有效算术强度”。硬件审计应分别列数学有效FLOP、实际issued AMX FLOP、padding、BF16/FP32 bytes、output-pass重复gather、转换、staging及缓存层级。逻辑字节不能当实际DRAM字节，采样worker和不能当墙钟百分比。

本报告与源码counters统一按一个scalar乘并累加计1个FMA、折算2FLOP。满形状16×32×16的TDP对应8192个scalar MAC，即16384 FLOP；BF16 pair数量不是scalar MAC数量。issued FMA按配置tile尺寸计数，不证明机器吞吐或周期与该数线性变化。

## 3. 新块投影保留什么，改变什么

源码：[icpp_block.cpp](../tfs_online/icpp_block.cpp)，独立检查：[block_checks.cpp](../tfs_online/block_checks.cpp)，完整驱动：[block_main.cpp](../tfs_online/block_main.cpp)。原 `src/` 和 `include/gat.hpp` 未修改。

每destination/head保持 m、l、V[d]。一个block产生：

\[
p_j=\exp(e_j-m'),\quad U_B=\sum_{j\in B}p_jH_j,
\quad V'=rV+U_BW,\quad O=V/l.
\]

U_B为每worker有界16×paddedD临时缓冲，block内从零FP32聚合，RNE转BF16后立即用真实AMX UW消费。V跨blocks驻留TMM，只有max更新引起旧V的spill→AVX行rescale→reload。不存在完整Z、e、alpha或U，不在末尾调用全图SGEMM。归一化仍只作用于d维输出。

保留原CSR、自环、原节点回写顺序、DegreeSort、TR16、动态panels、W VNNI packing以及独立head参数/attention/softmax。改变的是投影粒度和BF16舍入位置：旧路径每条pH量化，新路径每个U_B量化。它是原ICPP框架的块收缩扩展，并非旧逐邻居内核的逐位复现。

## 4. 运行条件和证据

- 官方arxiv：N=169343，E=2484941，Din128/C40。
- 官方products：N=2449029，E=126167053，Din100/C47；已统一自环的destination CSR。
- 模型固定Din→8×32→8×32→1×C，seed11未训练，hidden ELU，无共享heads。
- qhcn077，Xeon Max9462，shared intel，仅16物理核0–15/socket0；NUMA0/1显式interleave，OMP close/cores，MKL dynamic false。
- icpx2024.1/MKL2023.2、precise FP32 flags。block binary `890e5354f1dec9e93ef66425e7940658ef887c2967a49b020f927e71da800bf3`。
- 预分配steady-state完整三层、路径各自传递自己的输出，1warmup/3次AB-BA次序计时。静态W准备、DegreeSort、分配、比较在计时外。
- 10854118 COMPLETED0:0，10m24s，step MaxRSS59,377,892KiB（约56.6GiB），96GiB请求。
- [原始完整结果目录](../runs/icpp-block-10854118/)含rawlogs、manifest/pinning、accounting、源码快照、source/rule/data/binary hashes及TSV。均已下载本地。
- 失败10854102：小图通过，驱动在同输入/contracted-LR控制时缺失标准workspace的lr缓冲而segfault；修复驱动预分配及shape guard，旧kernel和门槛不变。失败日志保留，部分计时不进完整结果表。

以上为共享节点、未训练参数的探索性结果。

## 5. 同作业完整三层中位数

证据：[summary.tsv](../runs/icpp-block-10854118/summary.tsv)。

| 路径 | arxiv ms | products s |
|---|---:|---:|
| B0 FP32，纯GAT | 74.599 | 3.836666 |
| B0 BF16，纯GAT | 68.182 | 3.751798 |
| 原B1 TFS premax | 453.047 | 16.312194 |
| 原邻居级TFS Online B32 | 449.376 | 15.986196 |
| 双head原始H读取复用，Pair B32 | 369.766 | 10.904985 |
| Block B16 | 309.158 | 12.954649 |
| Block B32 | 305.562 | 14.129061 |
| Block B64 | 289.028 | 14.036358 |

最佳block相对原B1：arxiv1.567×、products1.259×；仍分别慢于B0 BF16约4.24倍和3.45倍。products block没有超过Pair。不能混用上一作业的13.882s或10.024s计算当前加速。

## 6. 第二层：为什么减少投影没有带来相同比例加速

证据：[block_work.tsv](../runs/icpp-block-10854118/block_work.tsv)、[online_stats.tsv](../runs/icpp-block-10854118/online_stats.tsv)、[profiles.tsv](../runs/icpp-block-10854118/profiles.tsv)。

| products L2 | B16 | B32 | B64 |
|---|---:|---:|---:|
| AMX block投影调用组 | 4,581,840 | 2,684,672 | 1,776,064 |
| 相对逐邻居TDP调用减少倍数 | 13.800 | 23.551 | 35.600 |
| 有效wide PH FMA | 258.390G | 258.390G | 258.390G |
| raw H逻辑读取 | 516.780GB | 516.780GB | 516.780GB |
| profile kernel墙钟ms | 7748.586 | 8934.278 | 9044.078 |
| PH读取/FP32聚合，采样worker ms | 799.041 | 948.341 | 998.252 |
| score生成，采样worker ms | 167.054 | 166.630 | 165.066 |
| block BF16转换，采样worker ms | 17.135 | 10.299 | 6.879 |
| AMX load+compute，采样worker ms | 27.852 | 23.077 | 11.000 |
| rescale store/vector/reload，采样worker ms | .801/2.778/1.106 | .427/1.372/.602 | .199/.560/.284 |

Speed repetitions没有热循环计时或重型counters。上表worker字段是单独profile pass、period128的采样和，不能与kernel墙钟直接加减，也不能称“PH占墙钟90%”。样本中PH显著大于AMX spans，且B64比B16的PH更贵；大block的访存、缓存和调度交互仍需硬件计数进一步定位，尚未证明单一cache-miss原因。

保存了conversion、L/R、kernel、normalization、ELU、layer及3layer E2E wall timers；kernel内另有score、max、exp/den、PH、pack、AMX、rescale三段、finalstore/scatter、scheduling/config。PH目前合并gather与FMA，AMX合并load与compute；尚未得到硬件DRAM字节或可加总的每指令墙钟分解。

## 7. DegreeSort没有自动制造高密度PH

[reuse_block.tsv](../runs/icpp-block-10854118/reuse_block.tsv)：每64个sorted TR16取一组并强制包含最高度尾组，分析完整邻居B32。此为系统样本，**不是全图无偏估计**。

arxiv/products edge-per-unique-source仅1.01858/1.00267；如果把这些TR16 destination行的source union直接做dense AMX，零填充slot/实际edge约16.111/16.151。products中support≥3行且组内≥8source的edge仅156/2,013,809样本edge。

因此当前DegreeSort下的同步neighbor blocks没有显示高跨destination overlap。不能直接假定16destination dense PH能达到AMX峰值；跨heads共享同一destination的neighbors更有结构保证。社区/索引/排序变更尚未测量。

## 8. PH微实验：打包和tile行数一起计入

原helper：[head_ph_probe.cpp](../tfs_online/head_ph_probe.cpp)。改进helper：[head_ph_compact_probe.cpp](../tfs_online/head_ph_compact_probe.cpp)，驱动：[ph_main.cpp](../tfs_online/ph_main.cpp)。

P的每一行对应一个独立head；邻居为reduction维，输入H为feature列。score、stable exp、den保持FP32不变。高分量为BF16(P)，双分量再加BF16(P-expand(P_hi))，AMX分别累加。双分量是乘法精度控制，不是第二种softmax，也不保证完整FP32精度。

改进配置A/C为8行、右操作数为16行；直接从两个source的native BF16 H加载，通过整数扩展/移位/OR写VNNI，省去Hrow memcpy和标量布局转换。AMX允许可配置行数，16是上限；这是依据[Intel官方配置说明](https://www.intel.com/content/www/us/en/developer/articles/code-sample/advanced-matrix-extensions-intrinsics-functions.html)使用的布局。

10854131，qhcn077同16核/NUMA策略，24GiB请求，COMPLETED0:0/10s。短作业sacct MaxRSS记录0，应视为未采到内存使用，而非真实零内存。binary `96cf467c6f96cf80ecbb8fd876d2a5b1b58dee01686d7d43b8037baef028d2f9`。输入在进程中共享：真实B64自身L1输出的native BF16 H和FP32 contracted L/R；不是未量化master H。

512个degree≥32的DegreeSort等rank样本、各首32neighbors。上游L1、分配、oracle/比较在microtimer外；score/exp、H读取/pack、P转换、AMX权限/config、load/compute/store/scatter均在内，含插桩和OpenMP启动。1warmup/3次交替，热样本、亚毫秒，不能外推完整邻域或模型。

证据：[PH summary](../runs/icpp-ph-10854131/head_ph_summary.tsv)、[time](../runs/icpp-ph-10854131/head_ph_time.tsv)、[error](../runs/icpp-ph-10854131/head_ph_error.tsv)。

| 512块，完整micro ms | arxiv | products |
|---|---:|---:|
| 原16行布局AVX对照 | .082640 | .105505 |
| 原16行AMX high | .099621 | .098054 |
| 原16行AMX high+low | .097386 | .097848 |
| 新8行布局AVX对照 | .092427 | .108900 |
| 新8行AMX high | .082519 | .084792 |
| 新8行AMX high+low | .080889 | .085308 |

新high+low相对自己的AVX中位数为1.143×/1.277×；对两份较快AVX控制则为**1.022×/1.237×**。两份相同逻辑AVX中位数出现约11.8%/3.2%差异，应公开该噪声，不能选更慢对照夸大速度。arxiv优势很小。两项同时改动(rows和packing)没有隔离各自因果收益。

每512块有效PH FMA33,554,432不变；8行high/high+low物理FMA33,554,432/67,108,864，原16行分别67,108,864/134,217,728。减少padding和输入准备，不是消除独立8head的有效数学工作。

12个instruction controls均通过独立quantized-operand FP64 oracle，den逐位一致。新high+low相对FP32 P/native BF16 H的FP64首块参考，归一化输出relativeL2为arxiv3.033e-7、products4.948e-7，maxabs6.808e-7/5.045e-4。误差仅针对该PH首块算子，不代表三层FP32 master或训练任务准确率通过。

## 9. 完整模型精度

原48、pair57+2、block57+2内部控制均通过，包括block的独立FP32 math、BF16指令模拟、max/den replay及四种profile/counter配置。真实图输出finite，profile fingerprint/counter identities通过。[checks.tsv](../runs/icpp-block-10854118/checks.tsv)保留所有路径和各层误差。

Block B16完整最终输出：arxiv maxabs.00414324、relL2.00351092；products maxabs6.17545、relL2.0152523。B64 products最终relL2.0150717。原abs.003门失败，未放宽。matched same-input/same contracted-LR/noELU的products L2相对FP32误差也单独记录；不是用不同attention替换后称算子等价。

参数未训练，真实checkpoint/任务accuracy未验收。速度表用于分析工程成本，不能宣布完整精度合格。

## 10. 两项瓶颈状态与下一次实现边界

| 问题 | 已实现/实测 | 当前限制 |
|---|---|---|
| 重复逐边W投影 | block U_B→真AMX UW；L2TDP少13.8–35.6倍 | blocks仍重复投影，不是每节点仅一次；收益被PH限制 |
| 原始H重复读 | 已有Pair两head共享，原算术逐位一致 | block版本尚未跨heads共享全部H |
| D/d=8有效宽聚合 | AMX head-as-row PH微控制局部有收益 | 8×数学工作仍在，未接全三层 |
| 任意跨destination共邻居复用 | 结构探针已测 | 当前样本不支持高密度假设；精确prefix/support方案仅推导 |
| FP32 master/任务精度 | 内部实现控制通过、全模型误差公开 | 原master门仍失败，task未验收 |

下一候选应将同一destination的多head PH_B合算，再逐head立即以AMX U_BW^h更新输出，保持原DegreeSort/TR16、独立Online和有界intermediate。不能让所有head共享attention，也不能通过先完整HW又丢弃来冒称aggregate-first。

硬件限制必须具体处理：PH输出按heads排，UW输入按destinations排；8heads×2输出tiles超过8个tile总数，不能同时全部驻留；完整16dest×8heads×256 FP32 block U约128KiB。若需要head分组、L1/L2 staging或输出spill/reload，必须计算其字节和时间。当前微实验没有包括后续UW与跨head状态搬运，不能用1.237×推导模型会击败B0。

保留TFS的可行研究方向已经收窄为：**块粒度减少重复投影 + 跨head共享H + AMX执行PH + 显式统计布局/状态搬运与精度**。原始36×/43×计算量只说明代价账本；是否胜出取决于完整pipeline，当前仍未解决。
