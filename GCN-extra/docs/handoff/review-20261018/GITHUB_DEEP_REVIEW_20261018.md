# GCN-extra 全实验复审与期刊方向判断（2026-10-18）

## 1. 审阅对象、方式和边界

本报告核查 GitHub 分支 `gcn-extra-experiments-20261003` 的提交 **9369b9e0491d59e328a99153b6d97314c62285da**，以及用户提供的《TFS_期刊升级_已验证实验与候选突破方向总结_20261018.txt》。不修改原始实验记录，不重跑性能实验，不提交集群作业。

下载的是 GitHub 固定提交，非服务器 live 源码替代品。GCN-extra 共 16,442 个文件、916,318,832 bytes；逐文件计算 Git blob SHA-1，与 GitHub recursive tree 比对，缺失 0、差异 0。完整性检查证明下载内容对应提交，不能证明每一项实验结论都正确。实际深审按实验族追踪：协议 → 冻结源码/driver → launcher/环境 → 原始 samples/CSV → 统计脚本 → 总结；早期重复探索采用协议、原始计时与关键路径抽查，不声称逐行人工阅读了全部 16,442 个文件。

覆盖 `results/` 的 15 个一级实验族，补充 `runs/` 中原始主实验、cache-control、projection-window，以及跨层归档内的 19 个子方向。分项证据见 [排列与库基线](review-20261018/audit_order_baselines.md)、[P0–P4与表示](review-20261018/audit_projection.md)、[跨层与共享](review-20261018/audit_crosslayer.md) 以及 `review-20261018/snapshot_verification.json`。

并非每族都有完整 raw stdout 随 GitHub 上传，因此补做服务器只读核查：P0 CSV 的2890条计时全部匹配服务器stdout；oneMKL 未packed17图425条、packed4核510条原始TIME重算的中位数全部与GitHub CSV一致，两份driver哈希也一致。P1的2400条、P3的200条、P4的1000条计时与上传tar中的stdout逐条对账；FULL排列主17图、H-panel和packed32核也追到逐次样本。原件未被这次审查改写，完整性哈希与逐条对账均不能替代实验设计本身的有效性。

用户约束仍适用：**禁止 NUMA interleave；原 128→128→128 是主对照；非等宽形状单列；不新增桌面 TXT；写入限服务器 wzh；不把共享节点诊断当独享正式结果。** 本轮使用 0 个计算节点。后续使用节点应遵守用户授权及适用工作区协议中的较严上限，不需为本次只读审阅解决历史 5/10 节点文字差异。

## 2. 结论先行

目前有真实的计算结构进展，但没有证据证明已经得到一个通用、可投稿的 AVX–AMX 协作新机制。

最可靠的两个发现是：

1. 从逐邻居投影转为局部归约后投影，确实减少了原 TFS 的重复 AMX 工作；这个收益不是幻觉。
2. 工作结构改变后，原 DegreeSort 的优势会消失甚至变成代价。FULL Identity 在多轮 32 核 17 图对照中有广泛收益，应该成为新的强基线。

访问/投影粒度解耦在少数图有很大收益；P1 排序、partial bundle、ordered sharing、残差、热点物化与跨层融合，均没有形成可替代强基线的普适突破。很多负结果是有价值的，但只能否定实际测试过的机制与实现，不能泛化为“源复用/跨层融合都不可能”。

想法 TXT 的主体数值大体能回溯到原始记录，且对失败方向的态度合理。需要补上后续 oneMKL 证据、违规 interleave 标记、几处程序/统计问题，以及更精确的计算与存储边界。

## 3. 全实验族证据地图

所有速度比均为各自同轮定义的基线时间/候选时间；不跨节点拼接毫秒数。

| 实验族 | 核查后的有效认识 | 不能扩大的结论 |
|---|---|---|
| 原始 B/FULL 主实验与 cache-control（runs） | B64 FAST 17 图 GM 相对原 TFS 约 1.301，连续计时约 1.309；11/17 胜出。原始固定形状/随机 H/W/单位边权，保留慢图 | 对 FP32 MKL 的倍数混有输入表示及精度差异；不是训练模型精度验证 |
| projection-window（runs） | 固定 M 改 S 可保留逐位输出与 AMX 次数；扩大了访存组织空间 | S 与 M、FAST/ACCURATE、rowwise FULL 不可混称一个 FULL |
| journal-p0-20261004 / A3 | Myciel rowwise FULL 约 2479.4 ms 对 S64/MFULL 1109.77 ms；A3 改变 loop order 后 cache-miss 计数大降。17 图中 13 图前两候选差 <1% | cache-misses 不是 DRAM bytes；源码逻辑 state 请求相同不等于编译后物理 load/store 相同；event 的 paper_eligible=true 不能代替共享/独享环境审查 |
| journal-p1-20261005 | S64/MFULL 上 page/source 分组对部分图有效；固定 page 17 图 GM 1.01257 | P1 的 FULL 是 S64/MFULL，不是逐行 FULL；不能把少数图大收益变成普遍排序收益 |
| p2-overlap-20261005 | 单核受控微实验低度/小工作集有小收益，高度时未稳定改善 | 未证明生产内核中的稳定 AVX–AMX 重叠，也不能否定所有指令调度 |
| journal-p3-p4-20261005 | 残差策略、选择性热点物化和简单全局/逐层顺序都有正负对照；压缩形状逐层混合 GM 1.28498，等宽约 0.75795 | 128→32→32 不是原论文主形状；P4 FULL 为 S64/MFULL ACCURATE，不能与 10/15 FAST Identity 直接比 |
| p3-residual-gate-20261005 | D2/D3 数值门槛通过；D2 五图 GM 0.93172，未胜 FULL ACCURATE | 精度可接受不等于值得采用，更不等于任务准确率不变 |
| feature-precision-20261004 | 17 图含 BF16/FP32、matched-input、prepare/page 等控制；表示与局部性影响是真实因素 | 不能只用理论 bytes 比例预测时间；不能冒称全 BF16 oneMKL SpMM |
| crosslayer-fusion-20261008 | 含 19 个方向及失败构建。RoadNet 当前 adaptive 通常中位数 1.144×，共享噪声高；其他图重复生成可能很昂贵 | fallback 时间是赋值，不是独立测量；未与同顺序 PACK_EPI 强基线充分比较；缓存不是有界小缓冲 |
| partial-bundle-20261013 | G4 M64 FAST 五图 GM 约 1.0203；Myciel 逻辑 C 请求大降，时间仅约 1.0066× | FULL 控制也变快/变慢，不能将变化全部归因 C traffic；逻辑请求非 DRAM |
| ordered-source-plan-20261014 | 严格顺序共享可保持四边界逐位一致；StaticShared 对 PACK_EPI 五图 GM 约 1.0069，覆盖非常低 | 没有高覆盖或普适收益；计时细分接近底噪，不可据此估算 production decode 占比 |
| full-degree-sort-ablation-20261015 | 原轮 32 核 17/17 Identity 胜，GM 1.3553；非空 AMX tile 数未减少。后续默认 NUMA 32 核轮次再次支持趋势 | 尚未隔离具体缓存、输出写入、线程尾部等原因；4 核不再 17/17 |
| h-panel-reuse-20261014 | 原 TFS 两 panel 重读 H 的确可改善；W64 四图正反顺序 GM 约 1.220/1.219 | 改的是原逐邻居 TFS，不等于对 FULL 增量；W16/D16 的工作量统计有重复累计 bug |
| onemkl-amx-audit-20261015 | 原 FP32 oneMKL 独立进程的 AMX-BF16 事件为0，含 TFS 正对照非零 | BLAS 名称/全进程 PMU 不等于单次算子纯耗时或指令数，不能泛化到所有 MKL API |
| onemkl-bf16-twostep-20261015 | 默认 NUMA 32 核：BF16 oneMKL/FULL Identity GM 2.0991，17/17；原 TFS/Identity GM 1.7008，17/17；BF16-only PMU 证明该 GEMM 路径实际使用 AMX | 稀疏阶段是 FP32 Sparse BLAS，含完整 Z 和量化，尚非“仅融合开关不同”的对照 |
| onemkl-bf16-twostep-packed-20261015 | 默认 NUMA 32 核：packed MKL/Identity GM 2.7515；4 核 1.8729，均 17/17。W packing 未扭转结果 | 共享节点，不跨轮比较绝对时间；4 核 Identity 相对 DegreeSort 只胜 15/17 |
| onemkl-bf16-interleave-20261015 | 归档真实保留了两方案在统一 interleave 下的比较 | **使用 numactl --interleave=all，违反用户约束；从本项目可采纳主性能证据排除，不能拿它挽救/否定任一方法** |

以上表格包括 runs 补充，所以行数大于 results 一级目录数。

跨层归档不能只读最后的 adaptive。10/7 的 state-reuse 原始计时中，PACK_EPI 相对同轮 **S64/MFULL/FAST/DegreeSort P0** 的 17 图交替测量 GM 约 **1.20180×**，而 PAIR_SHARE 相对 PACK_EPI 约 **0.99562×**。也就是说，一部分看似共享带来的收益实际上来自更好的打包/输出处理基线。旧 opt_exact_s 的 17 图交替 GM 约 1.01321，连续约 0.99965；event-trace 四图各队列/协议约 0.97079–0.98517，尚无净加速。完整 19 子方向逐项入口和覆盖程度见分项报告，不把它们都等同于跨层最终原型。

主审另补查 access-prefetch：PF8 用开头7个目标加稳态k+8，保持与PF1相同的目标多重集合；同时有两行累加器的exact-pair路径。2730条原始计时重算后，固定s1ExactSP8候选在独享连续/交替的四图GM为1.00772/1.00857，共享为1.01324/1.01534，四组均未过原定扩展门槛。说明“已有预取时序实验”也不能从历史中遗漏，不能重新做一遍再当新方向。

## 4. 必须纠正的实现和证据问题

### 4.1 interleave 不是只有命名问题

`experiments/onemkl_bf16_interleave_20261015/launch.sh` 第 11–12 行明确用 `numactl --interleave=all`，说明文档记录 Job10973480/qhcn509 和 `policy: interleave`。双方用相同策略并不能解除用户禁止此优化的约束。应保留原始记录用于审计，单独标为不符合本项目要求；后续禁止沿用该 launcher。

不要混淆：crosslayer 的 `recompute_window_interleave` 名称指交替测量顺序，其说明记录默认 NUMA；需要按 launcher/日志判断，不能仅凭 interleave 字符串批量排除。

### 4.2 跨层回退、统计及内存

adaptive fallback 只运行 baseline，再 `ct=bt` 并复制输出。因此“Amazon/Reddit 无额外开销”不成立；它只能表明选中了复用基线的分支。六样本的 `[3]` 是上中位数，普通中位数应平均 `[2]` 与 `[3]`。更早 interleave 源码的 PAR_RESULT 与 PAR_GATE 还取了不同位置。RoadNet 1.165× 应按普通中位数写 1.144×，并保留噪声。

最新跨层对比的 materialized baseline 有完整 FP32 H1、独立 ReLU、BF16 转换；候选内联这些处理。10/7 已有 PACK_EPI 强基线，因此还缺“同排序、同 fused epilogue，仅跨层物化不同”的对照。不能把全部差值归于跨层依赖调度。

SourceCache 按线程分区全部 unique source 分配并保留到分区结束。RoadNet 约 523 MB BF16 源缓存，略大于全局 BF16 H1 约 505 MB，另有每线程 mark/slot 映射。未建立单一全局 H1 数组，不等于更少内存或更少 DRAM。

早期结构分析还有两个不能忽略的错误：

- `hot_source` 从 0 开始，用 degree(0) 作阈值，可能选到不在当前邻接表中的 0；因此不能用这次负对照否定正确实现的最热邻居排序。
- `window_scan.cpp` 用列度代替某源 H1 生成所需的行度，且将活跃行数和 AMX 组数相加标为 peak_rows；有向图下工作估算错误，峰值单位不一致。这些结构数字不是已验证的缓存模型。

10/13 review 已纠正部分旧表述；本次进一步补上程序与强基线问题。若此前把该原型称为突破或无回归，应撤回这一强表述。

### 4.3 H-panel 的计数 bug 不等于计时失效

`h_panel_reuse.cpp` 为每个 Choice 累加 `windows[c.window]`；A3 与 D16 都用 window=16，使该计数翻倍，连带影响 W16/D16 的逻辑 C load/store 报告。执行 kernel 没有因此多跑一次，逐位结果与原始时间不受这一计数错误直接影响，W32/W64 也不受该重复键影响。应修正文档账本，而不是据此废弃全部实验。

### 4.4 同名 FULL 与精度门槛必须展开

最少写清：rowwise/S64、FAST/ACCURATE、DegreeSort/Identity、epilogue 是否融合、最终输出类型、计时是否含准备。多个表都叫 FULL，但并非同一个实现。随机 H/W 的数值门槛、最终 BF16 memcmp、FP32 中间误差、训练任务准确率是四类不同证据。

## 5. 重新推导计算与内存流向

### 5.1 B 不代表全图 AVX/AMX 工作量的一增一减

在现有 B/FULL 预聚合的非 replay、单位边权路径中，每个 CSR 边依然被读取和归约一次。忽略初始化、尾部及转换，AVX 加法量约为 **E×D**，并不因为 B 从 64 变 FULL 就增加。变大的只是单次归约跨度；总量相同，时间会因访问顺序/寄存器和缓存状态而变。

固定 16 行 tile、D=F=128，每投影组约 32 条 TDPBF16PS：

```
original:   32 * sum_t max_degree(t)
block B:    32 * sum_t ceil(max_degree(t) / B)
FULL:       32 * number_of_nonempty_tiles
```

ACCURATE 双分量、replay、不同形状另计。量化/partial 初始化/输出 C 暂存通常随投影组改变。不能按每行独立 ceil(degree/B) 算整 tile 投影，也不能追求 AVX/AMX 同样忙而保留多余投影。

原 TFS 两个输出 panel 还分别 gather H；新的非 replay 路径每边只 gather 一次。因此从 original 到 block 的变化还包含约 2E→E 的源码源访问次数变化，不能把全部原始加速只归于 AMX TDP 减少。

真正的选择是：在必要 AVX 工作量大体不变的条件下，如何同时减少投影、等待和状态搬运。若要进一步改变数量级，需要减少重复聚合、缩短每边处理的向量，或改变中间结果复用方式。

### 5.2 FULL 的硬件路径

```
CSR 索引 → BF16 H 的间接读取 → AVX 解码与 FP32 归约
         → tile 局部 FP32 partial → BF16 hi（必要时 lo）
         → AMX tile load + W 投影 → FP32 C 局部暂存
         → 按目的节点写回 → ReLU/层间 BF16（依版本是否融合）
```

16×128 FP32 partial 是 8 KiB，不能全部装入 32×64B 的 ZMM 寄存器。当前 rowwise 内核已经让一行的 8 个 accumulator 在寄存器中完成归约，并预取下一邻居的四条 cache line。C1 并非“首次寄存器归约”或“从一条加法链变多条”；要验证的是增加独立 CSR/H 访问流是否仍有增益。两行占 16 个 ZMM、三行占 24 个，但解码等临时值也用寄存器，必须看实际汇编。

AMX TMM 中原 TFS 常驻的是输出 C 累加器，不是先算完再留住的 AH。把“不物化 AH”表述为“AH 总在 TMM”会掩盖逐边重复投影。

### 5.3 oneMKL 胜负仍未完全隔离融合收益

当前 oneMKL BF16 两阶段实际为：BF16 H 解码成全局 FP32 H → FP32 Sparse BLAS 得 Z → Z 量化 BF16 → AMX BF16 GEMM。它是必要的库基线，且已有 AMX 使用证据，但与本方法的 BF16 源直接 AVX 聚合有不同输入流量、聚合实现和中间表示。

还需一个因果对照：**同一 AVX 聚合/同一量化边界/同一输出 epilogue**，分别直接交给 AMX，或先写完整 BF16 Z 再用相同投影内核；另以优化大 GEMM 做性能对照。这样才能区分“聚合内核更快”“表示更省”和“消除全局中间结果”的贡献。这是验证工作，不应包装成创新。

### 5.4 非等宽顺序的阈值依赖存储精度

TXT 中 F<D 倾向先投影，是算术量层面的直觉。当前 P4 冷 H 是 BF16，每边逻辑特征读取约 2D bytes；投影缓存 Q 是 FP32，每边约 4F bytes。因此仅比较这部分请求，缩短的条件是 **F<D/2**，不是 F<D。128→64 此项并不减半；128→32 才约减半。还要计投影生成、map/merge、复用、实际 cache 命中。若改 Q 为 BF16，边界和误差都会变化，需重新检验。

## 6. 对 TXT 六个候选的重新排序

### C1：保留为短期机制探针，不认定为主创新

**TXT“尚未验证双行寄存器驻留”的表述需要细化。** 10/7 `state_reuse/pair_reducer.hpp` 的 PairReducer 已有 `acc0[8], acc1[8]`，dynamic pair与exact-pair也都同时维护两行。它们采用S64窗口和head匹配/共享机制；P2的逐行准备不同，但不能只检查P2就宣布整个项目从未做双行驻留。

尚可低成本验证的具体区别是：**无邻居匹配、独立地址流的固定节奏交错、整行生命周期驻留（避免每S64轮转写回）、与FULL Identity同轮比较**。R=3也可作为容量探针。收益仍不能预设，必须与已有pair路径区分控制开销、共享覆盖和驻留跨度；否则只是重写已做过的实现。用相同逐行加法顺序和量化点控制精度，并加入 Identity×S64/MFULL 的缺失因子。若只在Myciel有收益，定位为特殊瓶颈修复；若跨图稳定，再研究为什么和适用范围。

不应为凑创新无限扩 R 或改更多启发式。真正要证明的是：相同工作量、较少 partial 往返、更多有效并发访问之间的可重复关系。

P1 已有一个应纳入机制判断的反例：受控 regular128_reuse1 上 Degree→Shuffle 约 193.867→124.095 ms（1.562×），而采样的 reuse p50、pages p50、occupancy 相同。这说明这些现有统计特征不充分；不能继续只用它们解释或预测全部收益。

### C2：研究潜力较大，但不能绕开既有 CPU 融合

“局部生成 HW、局部消费、避免全局中间量”已有很近的 CPU 工作。新的区别必须落在可执行机制：同时约束源结果生命周期、重复生产次数、BF16/FP32 转换位置和 AMX 有效行填充，并胜过 project_all / project_used / per-layer mixed。

一个可审查的问题是：**在有容量上限的局部状态内，何时保留源投影，何时重复生产，何时退回全局物化，才能避免局部融合为节省存储反而重复大量工作？** 现有 crosslayer 不满足“有容量上限”，P4 热点选择也未胜强基线，所以这是待解决问题，并非已有成功方法。

主形状继续 128→128→128；压缩/扩张用来解释顺序边界，不能替代主表。跨层保留 ReLU 等依赖屏障。

### C4：可能减少 AVX 加法，是结构筛查优先项

同一 16-bit 消费 mask 下 n 个源服务 k 个目的节点，先合并源再分发，理想加法节省约 `(n-1)(k-1)D`。这改变聚合工作量，区别于只改访问顺序，但本质与共享子聚合/HAG 接近。高 E/U 只表示源重复，不能保证相同 mask 成组，更不能保证节省后抵得过分发/元数据。

还需防止另一种误判：mask 组少不自动减少 AMX 指令。当前 FULL 已把 16 行合成一个物理投影 tile；即使只留下少数聚合组，若仍占一个 AMX tile，TDP 条数未下降。跨 tile 打包可能填满，但会扩大生命周期与映射成本。先分别核算 AVX 加法、源加载、AMX 实际 tile 组数和状态 bytes，再决定是否实现。

该候选改变 FP32 加法顺序；单位边权的实数恒等式不保证 BF16 输出逐位相同。对称归一化可分离的 α_iβ_j 可在数学上组织共享，但把 scaling 移过 BF16 量化会改变数值语义。一般边权、重复边须单独处理。

### C3/C5/C6：暂作辅助

C3 单纯 source-fanout 可能用更多 FP32 partial load/store 换少量 BF16 H decode；C5 局部 AMX SpMM 要面对 padding/packing、FP32 聚合结果到下个 BF16 GEMM 的交接；C6 联合模型需以实际新机制为基础。它们都不能靠换名或加 selector 形成新贡献。

优先级应分两轴：**最快辨别硬件余量的是 C1；更可能改变剩余计算工作的是 C2/C4。** 尚无证据宣布三者之一已突破。

## 7. 创新边界：承认继承，再明确新增问题

本次补读一手论文，而非只凭题目判重：

- [SparseLNR, ICS 2022](https://arxiv.org/html/2205.11622v1)：直接研究包含 SpMM–GeMM 的循环分配/融合，减少重复计算和中间存储。因而“把线性变换移出邻居循环”不能单独认作全新思想。
- [Tile Fusion, ICS 2025](https://hpcrl.github.io/ICS2025-webpage/program/Proceedings_ICS25/ics25-67.pdf)：构造稀疏依赖 DAG、融合 GeMM–SpMM 等算子，并按 fast-memory 容量切分。C2 不能只以 CPU、稀疏依赖、局部物化作区别。
- [Graphite, ISCA 2022](https://iacoma.cs.uiuc.edu/iacoma-papers/isca22.pdf)：CPU GNN 已研究聚合/更新融合、访存计算重叠和节点顺序。AVX/AMX 共用不是通用协作思想的首次提出。
- [HAG, KDD 2020](https://www-cs-faculty.stanford.edu/people/jure/pubs/hags-kdd20.pdf)：共享邻域的中间聚合与容量约束已存在。C4 可研究 tile 局部、低元数据、精度接口与 AMX 消费约束，但要有新算法和对照。
- [MaSpMM, TACO](https://doi.org/10.1145/3803422)：已有 cache-aware column-segment/outer-product 源复用；单纯 source-first 不是空白。

这些工作不等于不能继续。可争取的贡献是：它们未直接给出适合当前 AVX/AMX、量化接口、有限寄存器状态组合的可执行最优方案。但“没直接给出”也不自动构成贡献，需证明新机制解决了现有方法会失败的具体边界。

与其预设标题为 Work-Efficient Tile Fusion，不如暂时用更可证伪的问题组织工作：**在已消除重复投影的前提下，如何减少稀疏归约工作或交接开销，同时保留有效的矩阵批量与精度？**

## 8. 最小下一步，避免重复扫参数

本报告只提出顺序，不在本轮启动实验。

1. 固定强基线：原 TFS、rowwise FULL Identity、Identity+S64/MFULL、同精度同聚合实现的分离路径、适用形状的投影优先/逐层混合；epilogue 对齐。先补缺失因子，不使用历史比值相乘。
2. C1 小型可证伪实验：已有pair双行驻留是控制项；只验证无匹配的固定交错与整行生命周期这项差异，R=1/2/3，固定排序/逐行数值顺序/AMX组数，汇编检查spill，独享配对E2E；不靠纳秒插桩求占比。
3. C4 仅结构探针：17 图完整 CSR，Identity/Degree 分别统计 mask 频率、可省加法、plan bytes/构建时间；若工作上界很小就停止。既有 pair 低覆盖是警示，不是全 mask 的严格上界。
4. C2 先明确存储类型和生命周期预算，在 D/F 形状下算 source duplication、AMX padding、转换与物化代价；只有出现无法由简单强基线覆盖的区域，再实现新机制。

正式论文还必须加入归一化 GCN 与真实输入/模型误差验证，保留原 128 等宽对照，记录准备摊销和失败案例。每项证据用实际作业环境标注，不能只读 paper_eligible 布尔。

## 9. 固定提交证据入口

下面链接都指向本次审阅的固定提交，而不是可漂移的分支 HEAD。

- [canonical kernel](https://github.com/huamiao123/GAT-TFS/blob/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/src/paper_methods_kernels.hpp)
- [S/M kernel](https://github.com/huamiao123/GAT-TFS/blob/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/src/projection_window_kernels.hpp)
- [P0 mechanism](https://github.com/huamiao123/GAT-TFS/blob/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/docs/JOURNAL_P0_MECHANISM_20261005.md)
- [P3/P4](https://github.com/huamiao123/GAT-TFS/blob/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/docs/JOURNAL_P3_P4_INTERPRETATION_20261005.md)
- [FULL order evidence](https://github.com/huamiao123/GAT-TFS/tree/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/results/full-degree-sort-ablation-20261015)
- [default-NUMA oneMKL](https://github.com/huamiao123/GAT-TFS/blob/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/docs/ONEMKL_BF16_TWOSTEP_20261015.md)
- [excluded NUMA interleave launcher](https://github.com/huamiao123/GAT-TFS/blob/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/experiments/onemkl_bf16_interleave_20261015/launch.sh)
- [crosslayer review and raw links](https://github.com/huamiao123/GAT-TFS/blob/9369b9e0491d59e328a99153b6d97314c62285da/GCN-extra/docs/CROSSLAYER_FUSION_REVIEW_20261013.md)

本报告未修改历史结果，不将新候选称为已经实现或已经加速。服务器 handoff 的旧叙述应保留历史，但以本次复审的证据边界为当前口径。
