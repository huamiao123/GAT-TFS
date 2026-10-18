# 10/5、10/15 历史失败与修复补核

日期：2026-10-18。只读服务器、固定 HEAD `9369b9e` 及已有归档；没有编译、提交作业或运行内核。源代码差异、实际 stderr/stdout、目录内是否存在性能记录和 Slurm accounting 共同判定状态，不能只相信 event.json。

机器明细：`coverage_misc_versions.json`（13个版本/控制记录）；原始读取结果 `coverage_misc_remote.json`；实际数值gate统计 `coverage_misc_gate_detail.json`；直接查询的 `coverage_misc_sacct.txt`。没有改动已交付的 coverage_oct04。

## 1. Delayed residual：第一轮是启动失败，第二轮才观察到D1反例

`runs/delayed-residual-gate-20261005-112010` 构建成功，job10869361没有执行数值kernel。真实错误为：

```
job.sh: line 18: .../logs/logs/node.txt: No such file or directory
```

`preflight.sh:5` 会给全局shell变量RUN重新赋值；job把它source到 `$RUN/logs` 后，RUN已指向logs，再拼一次logs造成错误。该目录不存在 `results/gate.log`，没有性能记录。sacct为FAILED/1:0、耗时1秒。

112212版本与前者的唯一相关source diff是 `run_gate.sh` 引入 `GATE_RUN=$1`，并在preflight之后恢复 `RUN=$GATE_RUN`；数值kernel不变。job10869364实际运行并COMPLETED/0:0，其 `results/gate.log` 完整包含5条RESIDUAL_GATE和一条COMPLETE：

| 方法 | relative L2 | 严格FP32 gate | 最终BF16相同 |
|---|---:|---|---|
| source_tfs | 0 | pass | 是 |
| b2_accurate | 0 | pass | 是 |
| d1_single_delayed | 0.00195694716243 | **fail，阈值0.001** | 是 |
| d2_two_component_delayed | 0 | pass | 是 |
| full_accurate | 0 | pass | 是 |

`residual_gate.cpp` 明确把“D1失败、其他方法通过、所有最终BF16相同”作为预期成功条件。故实验COMPLETE pass1不表示D1被接受；它表示数值反例被成功验证。该程序没有计时输出，不能叫性能失败。

需要纠正 `coverage_server.md` 对112010的初步描述：首次FAILED不是D1预期反例造成，而是launch错误；D1反例来自修复后的112212。两者必须分列。

## 2. Selective shape：12:29到12:34包含实际控制增强

两个snapshot已逐行diff，差异不是仅重编译或日志修复：

- `selective_shape.cpp` 把连续小随机数生成改回原TFS的离散 `.01f*((rand()%200)-100)`；width128时W1/W2按原源码交错生成。原source依据 `original/gcn_e2e_v3.cpp:300`。
- 新增 `project_used_L1_full_L2`：第一层先投影被使用source，第二层用FULL归约后投影；`Choice.second`使两层选择可以不同。
- `selective_kernels.hpp` 增加partial state load采样、线程active记录；`selective_suite.py`把期望候选数从8调到9。
- 当前 `experiments/journal_followup_p3_p4_20261005/PROTOCOL.md` 已明确说明“首次smoke后”做上述修改，并禁止以旧smoke作主表分母。

真实日志核验：

|版本|实际执行覆盖|计时记录|数值/等价/工作量控制|
|---|---|---:|---|
|122940，smoke10869522|3小图×2形状，8候选+MKL|270|144 CHECK、15 EQUIV、96 PROFILE_GATE、96 WORK，全部pass；48个ACCEPT全部接受；6 COMPLETE|
|123418|smoke10869541三图；controlled10869544两图；focus10869550五真实图；均2形状，9候选+MKL|1000|540 CHECK、50 EQUIV、360 PROFILE_GATE、360 WORK，全部pass；180个ACCEPT全部接受；20 COMPLETE|

这条链包含一次有意义的强基线增强和输入控制修正。不能把第一版当作“同一最终实现重复测量”，不能混合270和1000样本，也不能把270条小图计时当作真实图结果。

## 3. FULL DegreeSort两次失败：第二次已有一个图的实测

目录：`runs/full-degree-sort-ablation-20261015-113936`。

- **10968050 / smoke114039**：实际stderr在affinity assertion失败，分到4个物理核但脚本硬编码要求32；sacct FAILED/1:0。没有graph stdout或kernel计时。
- **10968053 / smoke114139**：实际 `tail_37.out` 已产生 `ABLATION_CHECK l1_bitwise=1 l2_bitwise=1 pass=1`、16条ABLATION_TIME、6条STAGE、`ABLATION_COMPLETE pass=1`。然后shell执行日志提取 `rg`，节点没有rg，stderr报 `command not found`；sacct FAILED/127:0。tail137尚未执行。**不能说这次完全没有性能记录，也不能说数值kernel失败。** 这是一个完成图加后处理失败的partial run。
- **10968058 / smoke114256**：tail37与tail137各16条TIME、6条STAGE，并各自CHECK/COMPLETE通过；当前job.sh第21–22行按SLURM_CPUS_PER_TASK检查核数，第49行用grep提取。

该study没有为两次失败分别保留完整launch脚本副本；不能声称完整失败launcher源码逐字diff已获证据。但真实stderr、分配信息、graph stdout、最终修复脚本和sacct已共同封闭“失败发生在哪里”的问题。相同build下修复launcher，没有证据表明kernel算法因此变更。

## 4. Packed BF16 oneMKL：API枚举修复与图列表扩展

- **163149 build失败**：`build/compile.log`确有三处 `use of undeclared identifier 'CblasBIdentifier'`，分别为driver.cpp71/75/77行。没有生成可执行bench，没有graph数据或计时。
- **163228**：driver仅把上述三处枚举改为 `CblasBMatrix`。smoke10973261的email-Enron stdout有30条TIME；两个对TFS误差CHECK pass1，一个Identity/Degree CHECK bitwise1。不是新算法。
- **163448**：与163228的driver逐字相同；snapshot唯一代码变化在job.sh添加full17图列表分支。成功主run为10973268（32核）及10973286（4核），性能结果由先前基线审阅覆盖。
- **10973279 backfill**：本轮直接查询sacct，状态 `CANCELLED by 3538`，Start=None、Elapsed=0、NodeList=None assigned。对应目录只有取消event，没有graph stdout。它确实是等待中的重复提交被取消，未产生性能样本。取消event写exit_status1，而sacct为0:0，这再次说明单个exit字段不能替代是否启动/完成的证据。

## 5. 两次最早S/M smoke的门槛已补到真实stdout

10865904和10865927分别读取三份 `*/methods.log`，不再只信completion事件。

每轮都实际含：180 METHOD_CHECK、72 METHOD_EQUIV、84 METHOD_PROFILE_GATE，**全部pass**；3 GATE_COMPLETE和3 COMPLETE均通过；480 TIME、135 STAGE。第一轮没有METHOD_PMU记录，第二轮有81条PMU及324条PMU_THREAD。后者是诊断扩充，不是第一轮数值失败。

这关闭了coverage_oct04中“最早两轮smoke未逐条核gate”的缺口；仍然没有把每个重复profile/PMU人工解释为完整硬件因果研究。

## 本次覆盖结论

这些历史链不能统一标为“失败实验”或“重复实验”：P3首次是launch失败；D1是成功实验中的预期数值拒绝；P4引入了新强对照；FULL第二次有有效的单图raw结果但批次失败；packed首版是编译错误，backfill从未运行。现在每类都有实际代码/日志/完成标记支持。没有新增性能主张，也不把修复链包装成创新贡献。