# 受控复测的实现解释与证据边界

本轮针对 GCN-extra 推理计算流，使用与论文表格对应的 `gcn_e2e_v3.cpp`。此前不符合本轮控制变量的实验已按用户要求删除；仅保留本轮受控结果。主结果见 `PAPER_METHODS_RESULTS_20261004.md`；连续测量的执行顺序对照见 `PAPER_CACHE_CONTROL_RESULTS_20261004.md`。

## 实际保留了什么

DegreeSort、16 个 destination 组成 tile、64 行动态 OpenMP 调度块、相同 VNNI-packed W、AMX BF16 矩阵乘、原节点顺序输出都保留。每个线程仅持有当前 tile 的 partial 和 output，未生成全图 `AH[N×128]`。

原 TFS 每个邻居 step 投影一次。新方法先在 FP32 中归约 B 个邻居，立即把当前 partial 交给 AMX，累加到局部输出。FULL 是对当前 destination tile 的全部邻居归约后投影；不会先完成全图 SpMM。

FAST 将 partial 截断为一份 BF16。ACCURATE 将 partial 分解为 BF16 hi 和 BF16 residual lo，做两次 AMX 投影。两者均使用原源码的截断转换，未替换成另一种舍入方式。名称 ACCURATE 表示归约表示误差更低，不表示已经验证分类准确率。

## 执行工作模型：以源码为准

固定 D=F=128，KB=4，NP=2。记每个 DegreeSort 后的 16-row tile 的最大度为 m。

| 路径 | 当前 tile 的 TDPBF16PS 指令次数 | 邻居访问组织 |
|---|---:|---|
| 原 TFS v3 | `32m` | 先一个 64-feature 输出 panel，再遍历另一个 panel |
| B FAST | `32 ceil(m/B)` | 每个 partial 被两个输出 panel 消费 |
| B ACCURATE | `64 ceil(m/B)` | 两个 panel 分别消费 hi 和 lo |
| FULL FAST | 非空 tile 为 32 | 当前 tile 完整归约后投影 |
| FULL ACCURATE | 非空 tile 为 64 | 当前 tile 完整归约后投影 hi/lo |

这些是源码推导的指令计数，包含失活行和尾行的 tile padding，未冒充 PMU 实测。不能直接以 `E/B` 替代 tile-level 实际执行次数。

原 TFS 的两个输出 panel 各自遍历邻居。新方法的 partial 跨输出 panel 复用，因此与原 TFS 的收益包含投影次数减少和邻居读取组织改变，不能全部归因于 FLOP 减少。不同 B 的新路径使用相同组织。

新路径还付出局部输出 staging：每个 partial 写回 `ctile[16×128]`，下一 partial 投影前重新加载它。这是线程局部存储，不是完整全图 AH，但存在 tile store/load 开销。B 小时不能忽略这部分成本。原 TFS 在一个输出 panel 的整个邻居循环中保留 TMM accumulator。

## 独立分阶段计时

`runs/paper-method-reconciled-20261004/stages.csv` 保存每个方法三次分阶段测量：Layer1、ReLU、中间 BF16 conversion、Layer2、总计。它与主性能测量分开，不能用主性能的某个时间作分阶段的分母。

下表来自分阶段运行，单位 ms，均为该列三次测量的中位数。各列独立取中位数，因此四段之和可能不等于 total 中位数。

| 图 | 方法 | Layer1 | ReLU | Conversion | Layer2 | Total |
|---|---|---:|---:|---:|---:|---:|
| Products | 原 TFS | 163.89 | 4.37 | 4.98 | 160.53 | 339.34 |
| Products | B64 FAST | 142.47 | 4.17 | 4.68 | 139.46 | 291.81 |
| Products | FULL FAST | 140.65 | 4.16 | 4.95 | 137.88 | 287.42 |
| Reddit | 原 TFS | 149.94 | 0.46 | 1.64 | 150.13 | 310.65 |
| Reddit | B64 FAST | 62.56 | 0.45 | 1.66 | 63.08 | 126.63 |
| Reddit | FULL FAST | 67.90 | 0.44 | 1.67 | 65.11 | 135.52 |
| Mycielskian19 | 原 TFS | 1108.47 | 0.72 | 0.83 | 1115.76 | 2205.91 |
| Mycielskian19 | B64 FAST | 559.30 | 0.71 | 0.86 | 530.20 | 1096.50 |
| Mycielskian19 | FULL FAST | 1201.81 | 0.72 | 0.84 | 1225.44 | 2428.81 |
| RoadNet-CA | 原 TFS | 14.32 | 5.88 | 4.04 | 11.53 | 35.98 |
| RoadNet-CA | B64 FAST | 16.37 | 5.63 | 4.11 | 13.30 | 38.93 |

## 采样细分的解释限制

`sampled_profiles.csv` 分别记录调度、partial 清零、CSR/prefetch、feature gather/decode、FP32 归约、partial BF16 转换、tile load、tile compute、tile store、output scatter 和 AMX setup。BF16 final output 的转换包含在 output scatter 中。

这些是有计时插桩的采样线程时间。多线程求和不等于 wall time；频繁的时钟读取本身也会干扰短段，故不把采样比例当作正式无插桩运行的精确占比。

已确认的观察：增大 B 后，AMX 投影减少，但读取、解码和 FP32 归约仍随真实邻居数增长；Mycielskian19 的 FULL 两层都显著慢于 B64。代码中 FULL 连续完成一个 destination 的长邻居序列，B64 则在 16 行之间按 block 交替；其局部性差异是值得检查的原因，尚未用 cache miss/带宽 counter 建立因果结论。

目前没有在本轮获取 PMU 指标，不声称已经证明 compute-bound 到 memory-bound 的交叉点。不能仅凭平均度选择 FULL，也不能把逐图事后最快 B 当作已实现的自适应方法。

五次主测量仍有波动：例如 Wiki-Talk 的 FULL FAST 最大值与最小值之差约为中位数的 11%。`docs/figures/paper_methods_20261004/repeat_variability.csv` 保留每组的 min/P25/median/P75/max、样本标准差和完整重复值。它是描述统计，不是显著性检验；接近 1.00× 的差异不称为稳定加速。

## 数值边界

主运行 1020 项全输出检查全部通过，门限在运行前固定。FULL FAST 的两层 FP32-final 对原 TFS 的最大 relative L2 为约 0.00605；FULL ACCURATE 约 0.0000575。实际 BF16-final 的误差另行检查，不用 FP32-final 结果替代。

数据仍是原源码定义的随机 H/W、单位 CSR 邻接、128→128→128 的两层计算流。已验证数值与耗时；真实训练 checkpoint 的分类精度、归一化 GCN 完整任务、其他 feature shape 都不在本轮结论内。
