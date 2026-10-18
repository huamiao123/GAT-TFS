# 当前复审结论（2026-10-18）

本节取代下方历史段落中不再成立的“当前”判断；历史保留，原始日志不改写。

- 本次完成 GitHub `9369b9e0491d59e328a99153b6d97314c62285da` 全实验族审阅与用户 20261018 TXT 的交叉核查。16,442 文件 Git blob 校验无缺失/差异。未运行 benchmark，未提交集群作业。
- 完整报告：[GITHUB_DEEP_REVIEW_20261018.md](GITHUB_DEEP_REVIEW_20261018.md)；分项深审、派生重算及完整性记录：`review-20261018/`。
- GitHub 截止该提交已经发布 10/14、10/15 的新增实验；下方“GitHub尚未发布”是历史状态，不再是最新状态。服务器源目录不是 Git checkout，不能用 live 文件替代冻结 GitHub 证据。
- 可信进展：局部归约减少重复 AMX 投影；FULL Identity 在多轮默认 NUMA 32 核 17图优于旧 DegreeSort，并优于当前 BF16 oneMKL 两阶段/packed路径。仍是后补共享节点证据，4核 Identity 对Degree仅15/17。
- oneMKL BF16+packed 基线已经做过，不能重新声称完全缺失；尚缺“同 AVX 聚合、同量化/epilogue，仅改变全图物化边界”的因果消融及正式独享复核。
- **禁止 NUMA interleave。** 已发现 `onemkl_bf16_interleave_20261015/launch.sh` 实际使用 `numactl --interleave=all`，该实验保留历史但从符合本项目约束的主证据排除。不要继续使用该 launcher。
- 撤回旧 adaptive 的“fallback无回归/零成本”表述：其 candidate 时间直接取 baseline，未独立测量。RoadNet普通中位数约1.144×，共享噪声大；另缺同顺序PACK_EPI物化强基线；SourceCache并非有界小缓冲。
- 新发现的旧记录问题：H-panel W16/D16工作量窗口重复累计（不直接影响kernel计时）；hot_source的初值0问题；window_scan列度误用为行归约工作、peak_rows混合行数/组数。详见审计，不用这些有缺陷负对照否定所有对应方向。
- P1 FULL=S64/MFULL FAST，P3/P4 FULL=S64/MFULL ACCURATE，10/15 FULL Identity=rowwise FAST。以后写结果必须展开方法定义，禁止跨run拼加速比。
- 当前没有已验证的通用 AVX–AMX 协作突破。10/7 pair内核已有双行acc0/acc1驻留，不能将C1整体说成全新；未测的区别限无匹配固定交错/整行生命周期等。C2有界物化/C4共享聚合仍是待证候选。先强基线和缺失因子，再决定是否实现，不重复扫B。
- 主对照继续128→128→128；非等宽实验另列。所有后续实验仍需AGENTS/skill/preflight、独立快照、同轮对照和三账同步。用户当前最多10节点；已有工作区更严限制存在时遵守较严限制，本次0节点。

---

以下为此前交接原文，保留用于追踪：

# 最新实验补充（2026-10-14）

partial bundle 与 ordered partial-source plan 两轮独立实验均已完成，历史正文如下。

- partial bundle：五图，Job10944718/10944719。固定G4 M64 FAST五图GM约1.0203，ACCURATE约1.0254；FULL控制也存在变化，不能全归因于C traffic。
- ordered source plan：build `ordered-source-build-20261014-133737`，小图/结构Job10955177，32核共享五图Job10955178，全部检查PASS。固定相邻DegreeSort配对、S64、>=8 matches/<=8 runs，主性能FULL FAST。六张小图+五图结构先行检查，五图675条主时间；未扩大17图或回调阈值。
- 正式报告：`results/ordered-source-plan-20261014/GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md`。源码/原始输出/环境/图哈希/计时/PMU/工作量均已导出，原TFS/MKL和父冻结源码未覆盖。共享数据不能宣称独占论文结果。
- 主性能固定StaticShared五图正反顺序GM约1.0069，连续GM约1.0046；Myciel约0.998倍。Products近零覆盖、RoadNet零覆盖，不能把小幅时间差全归于省decode。这轮不是强通用加速或已成立的期刊贡献。
- load/decode细分补充：独立诊断build/Job见 `separated_detail_audit.json`。原load/decode细分零值为未测，已在独立快照拆开并通过逐位/工作量检查；主675条时间保留原快照，不混入诊断数据。
- 空区间校准：见 `timer_calibration_summary.json`，同节点32核；纳秒级load/decode/add接近计时底噪，不能据此计算production decode占比或机械扣除空区间。计时校准和原始数据完整保留。
- 本轮证明部分共享可以保持逐行加法顺序和四边界逐位一致；真实图选区覆盖很低，必须核对同计划独立/共享、同选区动态和PACK_EPI，不能把低覆盖图的时间差全部解释为decode收益。
- 当前授权节点上限5，本轮最多2个单节点作业；服务器写入仅wzh，yx只读。下面历史“10节点”不代表当前授权。
- GitHub尚未发布本轮新实验；本地base commit和dirty状态单独记录，不能把base commit误认为已包含新增代码。

---

# GCN-extra 当前交接摘要

更新时间：2026-10-08（服务器工作区）

## 工作区与约束

- 服务器工作区：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra`
- 原始数据：`/home/huangjianqiang_group/hdacp1/data/yx/TFS/data`
- canonical 内核仍在 `src/paper_methods_kernels.hpp`；所有新实验均放在 `experiments/` 隔离目录，不能把 shadow prototype 当成 canonical 修改。
- 所有实验均保持 `128 -> 128 -> 128`、单位边权、BF16 边界和默认 NUMA 策略；明确禁止 NUMA interleave。
- 单次作业使用不超过 32 个 CPU，节点数不超过 10；当前实验均为单节点。
- 每次实验前必须执行 `scripts/preflight.sh`；每次实验后必须执行 `python3 scripts/record_event.py check`。

## 当前最重要的方法判断

研究问题已经从“遍历 B 分组参数”收缩为：

> 根据 destination 分区之间的源行重叠、源行自身的 AVX 聚合代价和 AMX tile 填充率，选择 source-aware 融合或 materialized FULL，并在融合路径中跨多个 destination tile 打包待生成源行。

当前候选机制由三部分组成：

1. `min_source` destination 顺序，降低线程分区之间的源行重复。
2. `source_batch_blocks=16/64`，跨多个 destination tile 收集缺失源行，使 H1 的 AMX 投影组尽量填满 16 行。
3. 运行前计算加权源复制率 `rho_w`；高复制率图回退 canonical materialized FULL，避免固定融合导致回归。

## 已确认的证据

- 独享节点 Job `10902135`，节点 `qhcn819`：所有 batch/order 结果 BF16 exact。RoadNet-CA 上 `min_source` 的 `source_batch_blocks=16/64` 将源投影组从 184732 降至 131318/128585；min_source 和 identity 为正，DegreeSort 为负。
- 交错计时 Job `10902281`（共享节点）：RoadNet-CA、batch16，identity 约 1.31x，min_source 约 1.34x，DegreeSort 约 0.84x。Job `10902290`（共享节点）batch64，identity 约 1.40x，min_source 约 1.36x，DegreeSort 约 0.59x。共享节点数据只能作为机制证据，不能直接作为最终论文表格。
- 加权源复制率（32 分区）：RoadNet-CA min_source `1.045`、DegreeSort `1.924`；Amazon min_source `3.672`；Reddit min_source `26.063`；Road USA min_source `1.471`。
- 自适应原型 Job `10902324`（共享节点，v2 warmup）：RoadNet-CA 选择 fused min_source+batch64，中位数约 58.7 ms -> 50.4 ms；Amazon `rho_w=3.665`、Reddit `rho_w=26.062` 自动选择 materialized fallback，输出 exact 且不引入融合回归。
- 负对照 `source-partition-hot-analysis-20261008-232839`：按“最重源节点”聚类没有改善，RoadNet-CA 加权复制率升至 1.355，说明不能把创新简化为热点源排序。

## 不能直接引用为最终结果的记录

- 所有事件中的 `paper_eligible` 当前均为 `false`。
- `recompute-window-interleave-build-20261008-233639` 是一次缺少 serial kernel 快照的失败构建；后续 `recompute-window-adaptive-build-20261008-234221` 已成功，使用后者。
- `recompute-window-adaptive-20261008-233857` 的 fallback candidate 时间为 0，是旧计时逻辑；以 `recompute-window-adaptive-20261008-234255` 的 warmup v2 为准。
- 共享节点上的 Amazon/Reddit 和交错实验存在频率、热状态和共享资源影响；必须用独享节点重复后才能进入论文表格。

## 下一步顺序

1. 在独享节点重复 adaptive v2，至少覆盖 RoadNet-CA、Amazon、Reddit，并保留 exact gate。
2. 将 `rho_w` 与 AMX group utilization、source cache footprint、边负载不均衡纳入可解释代价模型；阈值 `1.25` 目前只是保守探索值，不是训练后的论文阈值。
3. 做留一图交叉验证，不能用测试图结果反推阈值。
4. 扩展 `D/F` 形状（包括不对称和非 128 维），确认方法不是固定形状特例。
5. 最终论文只报告独享节点、交错或随机化顺序、预处理/packing 开销口径明确的结果；保留失败图和 fallback 结果。

## 复现入口

```bash
cd /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
python3 scripts/record_event.py check
cat docs/handoff/CURRENT_HANDOFF.md
cat experiments/recompute_window_adaptive_20261008/LATEST_BUILD
```

不要删除 `runs/`、`events.jsonl` 或实验快照；它们是后续审计和交接的证据链。