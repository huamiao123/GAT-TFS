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