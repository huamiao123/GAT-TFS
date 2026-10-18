# 排列、H Panel Reuse 与 oneMKL 实验深审（9369b9e）

审阅范围：`full-degree-sort-ablation-20261015`、`h-panel-reuse-20261014`、`onemkl-amx-audit-20261015`、`onemkl-bf16-twostep-20261015`、`onemkl-bf16-twostep-packed-20261015`、`onemkl-bf16-interleave-20261015`。只读追溯代码、冻结快照、原始样本与汇总；未启动 benchmark，未改仓库源码。以下仓库路径均相对 `GCN-extra/`。

## 最重要的结论

1. **FULL Identity 的 17/17、GM 1.3553 可以从原始样本复算，比较本身控制得较好。** 同一 frozen FULL 内核、同一进程、同一数据，只改 destination permutation，FP32 L1 与 BF16 L2 都逐位相同。它仍是后续工作的强基线。这个结论尚为共享节点性能证据。
2. **用户 TXT 漏掉已经做完的 BF16 oneMKL 与 W prepacking 对照。** 默认 NUMA 32 核两轮中，FULL Identity 分别比 BF16 oneMKL 快 GM 2.0991、比 packed BF16 oneMKL 快 GM 2.7515，均 17/17；所以不能再说只做过 FP32 oneMKL，或者把已有 BF16 对照重新列为完全未开展。但它仍不是同内核只改物化边界的因果对照。
3. **仓库确实有用户禁止的 NUMA interleave 实验。** launcher 明确 `numactl --interleave=all`，raw policy 明确 `policy: interleave`。此组应保留为历史/不合规记录，从本项目可用性能证据中剥离；不能说整个仓库从未 interleave。
4. **H Panel Reuse 的 W16/D16 工作量记录有确定的统计 bug。** 同一个 window=16 在 Choice 列表出现两次，统计循环将窗口数累计两遍；影响 WORK 元数据，不影响实际 kernel 或计时。W32/W64 未受此 bug 影响。
5. **Identity 的 17/17 不能扩大到全部线程数。** packed 对照的默认 NUMA 4 核运行中，Identity 仅 15/17 快于 DegreeSort；Reddit、Wiki-Talk 分别慢约 6.8%、7.5%。这加强了“排序目标随执行结构/资源改变”的问题，削弱“永远取消 DegreeSort”这样的普适表述。

## A. FULL DegreeSort vs Identity：实际做了什么

实现证据：`experiments/full_degree_sort_ablation_20261015/ablation.hpp:19` 固定 `Method{"bfull_fast",0,false,false,0,true}`；21、27 行都是同一个 `block_kernel`，只换 perm。32、36 行分别 `memcmp` 第一层 FP32 和最终 BF16。61–68 行每个测量前都运行该方法一次 warmup，八轮交替 Degree/Identity 顺序。71 行之后另做 stage timing，不拿它代替主 E2E。

`generate.py:6` 起从完整原始 driver 注入 hook，原始 TFS/MKL 段保留；`ablation.hpp` 的 `ABLATION_TIME` 才是排列主比较。原始 MKL 五次连续计时先于 FULL 扫描，因此从该段导出的 MKL/FULL 比值，不是完全交错测量。

底层 `src/paper_methods_kernels.hpp:46` 的 FULL reduction 逐行按实际度累加，`92` 行仍为 `schedule(dynamic,1)`，每任务 R=64 行。`97–98` 行构造当前 tile 最大度与 step；FULL 只执行一个投影范围。`128` 行输出按 `rows[n]` scatter。因此排列同时改变 CSR 行访问次序、source-feature 地址序列、动态任务分布、输出 store 地址序列。实验能证明排列有用，不能只凭此证明是哪一个硬件因素。

核对与复算：

- 当前 kernel SHA 与冻结 source snapshot 都为 `35d291b886ee0f00e79e0e5d17c6226b2fa1ad64eb715ecbe7bcb1c3f02e7b85`。
- 从固定 17 图每张 `.out` 的 16 条 `ABLATION_TIME` 重新取两组 median，与 `summary_17.csv` 完全一致。
- 从 17 对中位数复算 Degree/Identity GM = **1.3553134043**；17/17 > 1，10/17 > 1.2。
- 六份 `results.json` 共 24 个完整图运行的 sample→median 自洽。raw stdout 另保留一个先前不完整 smoke 中已完成的小图，不能将它当全套 smoke 通过。
- default NUMA、同 socket 32 个 physical cores 在 `job.sh:17–26` 有 affinity assertion。主 17 图每图的分子分母都在同进程；没有跨节点拼接。

公平性判断：这是可信的单因素软件对照。DegreeSort setup 和 identity 初始化都排除在 prepared timer 外，且共同使用 perm 数组接口；这个选择并未给 Identity 偷加 setup 优势。不同工作量不是隐藏因素：Identity 非空 tile 数 8 图相同、9 图更多，即它并未靠少投影取胜。主要限制是共享节点、少量 noisy case（web-Google 首轮 CV 10–12%，后续重复方向一致）、缺 PMU/逐线程 attribution、只固定形状/单位边权/随机数据。

文件证据：`results/full-degree-sort-ablation-20261015/FULL_DEGREE_SORT_ABLATION_20261015.md:63`、95、121；原始数据在该目录 `full-degree-sort-ablation-20261015-113936/logs/{real-20261015-114328,validation_small-20261015-115845,validation_large-20261015-115853}/logs/*.out`。报告已有边界说明，用户 TXT 的 E4 基本准确。

## B. H Panel Reuse：正向机制成立，但 W16 work ledger 有 bug

`experiments/h_panel_reuse_20261014/h_panel_reuse.cpp:68–128`：外层窗口、再两个输出 panel；shared 路径只在 panel 0 gather；D16 在两个 panel 都 gather。窗口之间保存/恢复四个 FP32 TMM accumulator；每个 panel 内 neighbor→kb→TDP 的算术顺序保留。它保留原始 TFS 的逐邻居投影次数；不等价于 FULL/B64。

逐位正确性覆盖 L1 FP32、interlayer BF16、L2 BF16（`h_panel_reuse.cpp:197–206`）。五轮测量 forward/reverse 方法顺序（211 行），同节点另做反图顺序。原始 TFS include SHA 一致。复算 56 个真实图×方法组、280 条 timing row 的中位数，与 summary 相符：

| W64 对原 TFS | 首轮 | 反图顺序重复 |
|---|---:|---:|
| RoadNet | 1.154615 | 1.116649 |
| Reddit | 1.298627 | 1.316047 |
| Products | 1.126332 | 1.136333 |
| Myciel | 1.310314 | 1.320434 |
| 四图 GM | **1.219666** | **1.218576** |

W16 对 D16 的 matched staging 对照有价值：可以支持避免第二次 CSR/H copy/prefetch 及其缓存行为有净收益，不应缩写为“实际 DRAM 减半”。同样，W64 对原 TFS 不能乘到 FULL Identity 加速比上。

确定的统计错误：

- `h_panel_reuse.cpp:19` A3 是 window16；21 行 D16 也是 window16。
- 175 行 `for(auto &c:choices) if(c.window) windows[c.window] += ...;` 对同一个 16 累计两次。
- 179、186–187 行从被重复的 `windows_count` 算 `8*(windows_count-nonempty_batches)`，所以不只是 C count 简单翻倍，还多减/少减一个 nonempty 项。
- RoadNet raw `h-panel-reuse-10964915/results/work.csv` A3/D16 显示 windows=245654、C loads=C stores=982616；正确 windows=122827=nonempty tiles，实际这两项都应为 **0**。W32 raw 正好为122827、0、0。
- 当前实验源码与运行 source_snapshot SHA 同为 `b3acf50563943c981a79bfdb4f8d422ddf4fc35af94ca9a0a14c2dfc79dd24a6`，因此不是后期源码漂移。
- `verify_results.py:30–36` 只验证所有 TDP 相同、gather数、A0 C=0，没有独立重算每个窗口，故 `audit_results.json: PASS` 未发现该错误。

这是元数据 bug，不能据它推翻 W16/D16 的实测速度和 bitwise；也不能把整个 work ledger 当独立验证通过。修复应作为后续记录纠错，保留原始历史文件。本次按只读任务未修改。

## C. 原始 oneMKL 的 AMX 审计

`original/gcn_e2e_v3.cpp` 与 `original/mkl_baseline.cpp` 调 `mkl_sparse_s_mm` + `cblas_sgemm`，FP32 输入；不能由 MKL_VERBOSE 的 CPU capability 行推断 SGEMM 使用 AMX。

`experiments/onemkl_amx_audit_20261015/job.sh:25–32` 分别运行独立 mkl_baseline 进程和含 TFS 的 gcn_e2e_v3 进程；后者是 PMU positive control，不能归因于其中 MKL 调用。原始单事件 `.perf.csv` 显示100% counter running：

- email-Enron：mkl_baseline 两次 0；含 TFS 进程两次 45,330,202,624。
- web-Google：mkl_baseline 两次 0；含 TFS 进程两次 586,134,585,344。

这些是 AMX BF16 操作事件，不是 TDP 指令条数；结论只限此源码、版本、环境、图。原始 raw stdout 可见 oneMKL 2023.0 Update 2、SGEMM 和 `Dyn:1 NThr:32`。这项审计自洽，没有证明所有 MKL 精度/API 都不用 AMX。

## D. 默认 NUMA BF16 oneMKL / packed：用户 TXT 的实质遗漏

代码路径（packed driver 105–122 行）每层：

`BF16 H（同一截断量化） → FP32 decoded H → MKL FP32 SpMM → 全图FP32 Z → 截断BF16 Z → MKL BF16 GEMM → FP32 C`。

两层之间 ReLU→BF16 H1→重新 decode FP32；最终另转 BF16。W 在计时前用 oneMKL pack（71–78行），packed 的 GEMM 走 compute。所有方法同进程、旋转五轮（154–164行）。图值显式检查 unit weight（51行）。BF16 Z/W、FP32 accumulation 与 FULL 的名义量化边界较接近，但 MKL SpMM/GEMM reduction 次序不同，不是逐位相同算法。

复算同轮数据，数值为时间比（>1表示分子慢）：

| 对照轮次 | BF16 MKL / FULL Identity | packed BF16 MKL / FULL Identity | 原 TFS / FULL Identity | FULL Degree / Identity |
|---|---:|---:|---:|---:|
| 默认NUMA32核 qhcn071（未packed） | **2.099068**,17/17 | — | **1.700828**,17/17 | **1.355593**,17/17 |
| 默认NUMA32核 qhcn522（packed对照） | **2.712195**,17/17 | **2.751499**,17/17 | **1.625591**,17/17 | **1.302077**,17/17 |
| 默认NUMA4核 qhcn015（packed对照） | **1.807442**,17/17 | **1.872902**,17/17 | **1.467675**,17/17 | **1.230855**,15/17 |

packing/unpacked GM：32核 **1.014491**、只4/17更快；4核 **1.036217**、只1/17更快。这个接口上的静态 W prepacking 没有救回路径；不是对所有 oneMKL 版本、形状和 packing 的否定。

4核 Degree/Identity 的两个反例：Reddit **0.936**，Wiki **0.930**。应补入 TXT，避免把“FULL Identity 17图普遍强”扩写为对线程数不敏感。

证据位置：`results/onemkl-bf16-twostep-20261015/fixed17.csv`、`results/onemkl-bf16-twostep-packed-20261015/full17_32core.csv`、`full17_4core.csv`。32核packed目录有17图每方法五个原始 TIME；全部中位数复算匹配。共102个图×方法组中7组CV>5%，3组>10%，最大original TFS/as-Skitter 16.98%。FULL Identity在其中只有email-Enron CV8.43%较高；不能将共享节点2.75×视为最终正式速度。

未packed fixed17和packed4核在GitHub目录只见汇总，完整逐次日志指向服务器。本轮随后补做SSH只读核查：425+510条TIME，共187组，所有中位数与GitHub CSV差异为0；两份remote driver SHA与GitHub相同。未packed仅2/85组CV>5%，无>10%；packed4核102组全部CV<5%，最大4.49%。服务器原件未修改，完整派生记录在missing_raw_ssh_evidence.json与missing_raw_ssh_summary.json。由此4核Reddit/Wiki的Identity负例也有相对稳定原始样本支持。

AMX证据：未packed bf16_only 模式只运行BF16 oneMKL（driver相应分支），email和web-Google PMU非零；后者 raw `web-Google-bf16-mkl.perf.txt:6` 为90,088,538,112。packed路径并未单独提供对应PMU；API和冻结原型可见，不能把未packed PMU当作packed已测counter。

这些对照的边界：

- 数值check是MKL最终BF16对原TFS `relative_l2<=0.03 && normalized_max<=0.03`（packed driver133、139行）；不是MKL与FULL逐位检查。FULL只检查Degree/Identity逐位（143–147行）。同样的数值精度标签不等于相同算术次序。
- FULL输入feature存BF16，在AVX内逐次decode；MKL Sparse BLAS读取全图FP32 decoded H，因此每边feature的存储/载入格式也不同。优势可能同时来自紧凑BF16访存、局部融合、稀疏实现差异；不能把2.099×全归因于消除全图Z。
- 初始H0 decode在timer外（69行），H1 decode在timer内（114行）。这是prepared多层路径，不是包括所有输入准备的端到端时延。
- FP32 oneMKL方法在此driver最后新增BF16输出转换（100–101行），它与原始纯FP32 two-layer计时不是逐行完全相同边界，避免跨表直接拼。
- `job.sh:10` unset MKL_DYNAMIC，而非强制FALSE；原审计的MKL_VERBOSE显示Dyn:1。这是库默认策略，可作实际库对照，但未证明每个BF16调用都持续使用与TFS相同数量的活跃线程。
- packed32核 raw `numa_policy.txt` 是default，但CPU集合是0..33含空洞，并非FULL排序专项的32..63同socket固定集合；不要跨轮除绝对ms。若要正式比较，需要独享、固定同一物理核心集合与线程策略，而不是引入interleave。

强基线仍有一项真正缺口：**同一AVX稀疏归约实现+同一截断量化+同一AMX GEMM语义，只改变tile局部消费/全图物化边界**。这能隔离fusion本身；oneMKL BF16是必要库baseline，但无法替代这个消融。

## E. interleave 是真 NUMA，不可采用

`experiments/onemkl_bf16_interleave_20261015/launch.sh:11–12`：

```text
numactl --interleave=all numactl --show ...
exec numactl --interleave=all bash ...
```

`results/onemkl-bf16-interleave-20261015/interleave_policy.txt:1–4` 明确 `policy: interleave`、`interleavemask: 0 1 2 3 4 5 6 7`。固定17作业10973480、qhcn509。不是方法交替顺序之意。它违背用户此前明确约束，需标记excluded；不可静默删除历史，不能再报告“所有实验都没有NUMA interleave”。跨节点default与interleave比值本来也不能证明interleave因果效应。

## 对 TXT 的具体修订建议

- E4保留1.3553×但补默认NUMA BF16/packed后续32核复现，并补4核15/17的边界。
- E5保留W64正结果，明确W16/D16逻辑C计数需要纠正；删除任何“完整work账本独立通过审计”的笼统断言。
- 单独加入E13：原MKL无AMX-BF16的PMU证据、BF16 MKL已使用AMX、默认NUMA packed/unpacked结果、精度/输入格式边界。
- 不要把“相同精度强SpMM+GEMM基线”统统当未实现；改为“BF16 oneMKL已实现，尚需同内核物化边界消融与独享正式复核”。
- 增加禁用记录：interleave组存在但从候选证据排除，不把它算当前方法收益。
- C1仍可优先作小验证，但FULL Identity不是唯一不变真理：32核/4核排序差异提示执行资源与排序因子要分开；最小实验至少固定Identity对比R1/2/3，并保留Degree或S64中的强者为外部上下文。

本次派生计算脚本：`C:/Users/花喵/tmp/tfs_deep_review_20261018/audit_order_compute.py`。它只读CSV/原始样本，未运行任何kernel。
