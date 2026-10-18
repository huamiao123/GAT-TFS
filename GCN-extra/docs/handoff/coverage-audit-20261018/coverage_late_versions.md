# 10 月 8 日后半段历史版本补审

审阅日期：2026-10-18。对象为 GitHub 冻结快照 `9369b9e0491d59e328a99153b6d97314c62285da` 中 `results/crosslayer-fusion-20261008/runs/` 下 frontier、recompute、source-partition 共 **52 个运行/构建容器**。本文件补充 `audit_crosslayer.md` 的最终机制分析，重点核查早期失败的真实日志、冻结源码变更和原始结果；不等同于重新运行实验，也不声称逐条审查所有汇编。前一份 `coverage_oct07.md` 保持不变。

## 审阅深度与自动对账

- 逐容器建立文件、事件、日志、冻结源码清单。检查真实错误上下文，而非只读取 event 的 PASS/FAILED。
- 对同名冻结源码做相邻版本 diff，并额外比较 certificate 的嵌套目录及 source-partition 的重命名文件。完整差异见 `coverage_late_diffs.txt`、`coverage_late_checks.json` 的 `source_diffs`。
- 重新统计 **106 组 PAR/ADAPT 结果、372 对原始 baseline/candidate 时间**，保存逐组原始样本、常规定义中位数和 upper median。这里 372 对不意味着 372 次独立候选执行；adaptive fallback 存在赋值/零时间，见下文。
- 对账事件中 **198 条 result/gate 字符串**，均能在原始日志找到。它只证明转录一致，不证明 gate 或统计定义正确。
- 核对 **153 条 PARTITION 记录**的 edge_check 与简单 unique duplication，均满足日志内算术关系；核对 **152 条 window 结构结果**的 extra_edges/E，均匹配。
- 18 组使用六次样本的结果采用 upper median（排序后下标 3），未使用常规定义的两个中间值平均；12 组为 interleaved timing，6 组为 adaptive。所有重算值在 `coverage_late_checks.json`。

## 版本谱系：失败是什么，后续改变了什么

表中 run 名字省略共同日期 `20261008`；完整 52 项及证据路径见配套 JSON。

| 实验族/版本 | 实际证据与变更 | 应怎样计入实验历史 |
|---|---|---|
| frontier-structure-01 / 10898893 | 结构分析、oracle 与预期非法输入检查完成；部分 stderr 是故意拒绝 malformed 输入 | 结构实验，不是前向推理性能 |
| frontier-certificate-01 / 10899017 → 02 / 10899028 | 01 的 `compile_certificate.stderr.log` 为 std::filesystem 未定义引用；02 编译命令加 `-lstdc++fs`；两个版本的 `frontier_certificate.cpp` 完全相同 | 链接修复，不是第二个新方法。02 验证限定表示和固定生产顺序下的 certificate，不能推广为所有融合调度下界 |
| recompute-audit-01 / 10899639 | suite.log：缺少 `source_snapshot/frontier_prior_inputs.json`；stage.py 的 NAMES 未包含它 | 打包失败，无科学负结果 |
| audit-02 / 10899667 | suite.log：hash 读取 sentinel 写成局部变量 `b`，触发 UnboundLocalError；03 改为 `b''` | 脚本错误 |
| audit-03 / 10899674 | Python 逐边扫描过慢，日志有取消/终止；没有结果 JSON，event 的 exit_code=0 不能覆盖此事实 | 未完成运行，不是候选被验证为无效 |
| audit-04 | 只有 5 个准备文件，无实际 job/result | 准备容器，不应统计为完成实验 |
| audit-05 / 10899784 | 改成 C++ 扫描、两遍/排序键组织，4 图结构结果完成 | 实际扫描器改写；仍是成本模型，不是 kernel 性能 |
| recompute-window-01 / 10899912 | **实际 PASS，4 图结果存在**，窗口 1…1024 和 0 | 不能因后续修复把它标成执行失败；但生命周期旧计数被 04 取代 |
| window-02 / 10899982 | 新增 -1 特殊模式并将 WindowMetric.window 改为 int32；循环仍用 uint32，初始化出现 narrowing 编译错误 | 新模式尚未运行 |
| window-03 / 10899992 | 循环变量改 int32，4 图 × 13 窗口完成 | 编译修复，但旧生命周期计数仍存在 |
| window-04 / 10900001 | `diff[wi][producer]++` 改为 `diff[wi][g ? starts[g] : producer]++`，再生副本从实际 regeneration start 开始存活 | **实质性模型修复**，不是更快内核；03 的内存数值不能与 04 混用 |
| kernel-build-193715 → 193923 → 194137 | 首次 `using gcn_extra_paper::TR` 与 TR 宏冲突；其次 includes 指向不存在的发布目录；第三次恢复 ../../../original 与 ../../../src 后成功 | 两次编译/路径修复，未增加新数据流 |
| kernel-10900289 | WINDOW_CONFIG、2 条 RESULT、GATE 存在，执行原型并比对输出 | 真正 kernel 试验，区别于前述结构扫描 |
| sweep-10900343 → 10900360 | 前者仅保存 W=1 一组；后者实际保存 W=1,4,16,64,256,512,1024,2048,4096 九组 | 前者 PASS 不能解读为完整九窗口通过；后者扩大测量范围 |
| parallel-build-202152 → 202420 | vector 初始化被解析成函数（vexing parse），引发 locals[tid] 错误；改 static_cast<size_t>，增加 WINDOW_KERNEL_NO_MAIN 宏保护 | 构建修复；主旨为线程局部 source cache 并行原型 |
| parallel-10900662 | 只有 degree 顺序，3 对时间、1 组结果 | 早期单顺序实测 |
| parallel-build-211935 → 212208 | 增加 identity/min_source 顺序；先因缺 std::iota 声明失败，随后加 numeric | 增加顺序机制；第二次只是头文件修复 |
| parallel-10901373 / cross-10901444 / exclusive-10901902 | 分别 3、6、6 组结果；每组 3 对时间；后两者包含跨图与独享验证 | 有效测量扩展，但当时仍固定先 baseline 后 candidate |
| batch-build-221327 | 跨多个 destination tile 收集 missing source，再按 AMX 16 行打包；source_batch_blocks 为 1/4/16/64 | **新增源生成打包机制**；这个 batch 不是邻居归约 B |
| batch-221757 / 222602 | program.log：Cannot open GCN-extra/data/roadNet-CA/roadNet-CA.csrbin；srun exit 1；无 PAR 结果 | 输入路径失败，event 的 exit_status=0 不可靠 |
| batch-222234 / 222837 | 每个容器有 108 对 PAR_RUN、36 RESULT、36 GATE；全部 exact=1，常规三样本中位数一致；外层 event 却标 FAILED | 运行和输出已完成，存在外层验收/结果计数状态不一致；不能删除这些 raw，也不能将 outer FAILED 当作数值失败。保存材料不足以在本补审断言验收代码的具体预期条数 |
| interleave-build-230314 | 改为 6 次 base-cand/cand-base 交替测量 | 测量顺序控制，不是 NUMA interleave，也不是新融合机制 |
| interleave-230519 / 230957 / 231443 / 233649 | 每个容器 18 对时间、3 结果、3 gate；不同图或 batch 设置 | 结果使用 upper median；PAR_GATE 仍取排序后下标 1，统计口径与 RESULT 不一致 |
| interleave-build-233639 | event.kind 实为 adaptive_build，FAILED/1；冻结文件只有 adaptive.cpp，无 build.log | **真实证据缺口**：不能猜测具体编译错误；同时间附近 interleave-233649 的结果不能当作此 adaptive build 成功 |
| adaptive-build-233758 → adaptive-233857 | 第一个可运行版本；无 warmup，fallback 拷贝已存在 baseline 输出并返回 0 时间 | Road 是实际 fused 计时；Amazon/Reddit 的 candidate=0 不是真正加速 |
| adaptive-build-234221 → adaptive-234255 | 增加 warmup；fallback 改为再执行 baseline，复制结果，并赋 ct=bt | 修复零时间，但 fallback 的 1.0 比值来自赋值，不是独立对照；exact 也不能证明另一候选路径 |
| source-partition-build-210228；10901309 / cross-10901392 / cross-10902064 / cross-10902071 | 初始源重叠分析器及扩图，分别 6/12/42/18 条 PARTITION | 结构证据，不是四种新内核 |
| weighted-build-232004 → weighted-232054 | 加入 source 的 CSR 行度加权；3 图 × 6 顺序 | 新加权工作模型，不能自动解释为运行时间 |
| weighted-build-232709 → weighted-232839 | 增加第 7 个 hot_source 顺序；源码 hot_src 初始为 0，仅用更高行度替换 | 这是实质候选，但初始化可能把非邻居 0 当作参照/所选热点，负结果不能当作正确 hottest-neighbor 策略已被排除 |
| weighted-heldout-234650 / weighted-roadusa-235043 | 分别 30/6 条 PARTITION，运行六顺序 weighted 版本 | 没有验证 hot_source，更没有对应前向耗时 |

## 三处需要改变结论力度的具体发现

**生命周期修复改变了模型输出，不是一个小数字修正。** RoadNet 的 W=1 峰值指标从 window-03 的 30,034 降至 window-04 的 2,539，W=-1 从 31,954 降至 2,531，W=1024 从 23,517 降至 7,516；再生工作量没有相应改变。原因是旧模型把后续再生块也从最早 producer 时刻计入存活。04 仍保留既有模型边界：列消费度不总等于再生所需的 CSR 行度；峰值字段将 source row 与 AMX group 相加，单位未统一。不能因为修复生命周期就把其余模型假设当成已校验。

**交替计时改善实验设计，但统计输出没同步修完整。** 六样本的 RESULT 取 upper median，而 PAR_GATE 还沿用三样本的下标 1。补审已对全部 18 组重算常规中位数。adaptive 的 RoadNet 在两个版本中常规中位数 speedup 分别为 1.112914499 与 1.144000681；这仍只是对应原型/配置的结果。Amazon/Reddit 不能引用第一版零时间，也不能把第二版人为相等的两列当成独立 fallback 性能实验。

**weighted duplication 的分母不是 E。** 源码分母为全局实际被消费源节点的行度和 global_weight。部分图该值小于 E；因此不能用 weighted_total/E 检验该字段。本补审仅把日志具备充分字段的 edge_check 和 simple duplication 判为可完全对账。global_weight 没写入日志，完整独立加权验证需要 CSR。这是审计边界，不是日志比值被证明错误。

## 未核查或无法恢复的边界

1. `recompute-window-interleave-build-20261008-233639` 本地快照和服务器同路径均仅 12 文件。定向服务器 find 再核仍无 build.log。保留 FAILED/1 与源码，但无法归因到某个具体编译错误。
2. 本次不运行 kernel、benchmark、完整真实 CSR replay 或逐条反汇编。结构证据、性能证据、wrapper 状态分别列出，不互相替代。
3. 窗口/分区模型中的行列度、单位和实际执行分区映射问题，沿用 `audit_crosslayer.md` 的具体分析；本补审追溯它们在哪些历史版本存在，并未宣告已修复。
4. batch 外层 FAILED 与 36 组完整 raw 的矛盾已证明；本次未定位到保存下来的外层验收脚本，不能臆造其具体阈值。
5. 52 项是这个限定前缀集合的完整清单，不表示仓库全部日期、全部实验或全部失败原因均已读完。

可复核文件：`coverage_late_raw.json`（逐容器事件/日志/源码清单），`coverage_late_diff_manifest.json` 与 `coverage_late_diffs.txt`（版本差异），`coverage_late_checks.json`（逐样本重算及对账），`coverage_late_versions.json`（52 项分类及审阅深度）。
