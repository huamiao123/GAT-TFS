# 10月3日至4日清理前实验历史补审

本页补充 2026-10-18 上一轮审阅遗漏的历史；不是新的性能实验。仅阅读、恢复公开历史和重算已有数据，没有运行内核，没有提交集群任务。

## 证据范围与完整性

- 固定当前分支提交：`9369b9e0491d59e328a99153b6d97314c62285da`。
- GitHub API 返回该路径的 19 条提交。10月4日 `bdcf9b0` 清理了旧实验，不能只枚举 HEAD 判断历史覆盖。
- 从清理前 `8aeef1613fdeaeb929379874c4152eccde620f74` 恢复 3,969 文件，与该提交 Git blob 校验一致：0 缺失、0 不匹配。
- 其中 2,789 个路径在 HEAD 不存在（35,129,651 bytes）。这是文件路径数量，不是实验数量；与清理事件的 2,800 文件口径不同，不能直接判定冲突。
- 证据：`coverage_github_commits.json`、`coverage_tree_precleanup.json`、`coverage_history_restore.json`、`coverage_history_inventory.json`。
- 本页路径均相对清理前快照的 `GCN-extra/`。原始结果仅支持其当时的运行协议，不能无条件并入当前默认 NUMA 的主表。
- 额外对6个历史 `build-*/source_snapshot/src/bench.cpp` 做了连续源码diff并阅读全部差异（`coverage_history_build_diff.txt`）：零nnz处理、nozero及NaN覆盖、扩展block controls、计时外释放3份参考数组、LP64边界拦截。初始replay不是由后续代码倒推出来的，它存在于最早的冻结源码和实际方法日志中。

## 按问题与版本划分的覆盖台账

| 问题/版本 | 原始执行位置 | 本轮实际审阅深度 | 结论与后续关系 |
|---|---|---|---|
| 最初 B/FULL、FAST/ACCURATE、shared/replay | `runs/formal-10861421` | `src/bench.cpp` 归约、投影、replay分支、main注册与计时循环；560条正式计时逐条对stdout；112组中位数复算 | replay 已经执行过，不能再称“从未实验的点子”；当时显式 NUMA interleave，性能不进入当前用户约束下主表 |
| 去除全局清零控制 | `runs/formal-10861923` | nozero克隆路径、NaN输出覆盖检查；440条计时、88组中位数、264条数值记录 | original_nozero 与原始输出逐位对照，分离冗余memset收益；不能把该收益归于邻居分组 |
| 扩展 B sweep | `runs/formal-10861974` | B2/4/8/16/32/64/FULL及两种精度、replay；1,680条计时、168组中位数复算 | 4个工作负载含规则q16/q256，方法轮转、1warmup+10repeat；不是17图正式论文协议 |
| 25图覆盖尝试 | `runs/formal-10862002` | 6,120条计时全部对账、612组中位数、1,836条数值记录；status/非单位值拒绝 | 主批17图成功、6非单位边权拒绝、2大图未在LP64成功；不是25图都通过 |
| Friendster ILP64补充 | `runs/formal-10862158` | 140条计时、28组中位数、84条数值记录 | 该图确实做成功过，但改成ILP64、5repeat、B≥8且内存策略不同，不能与后续忠实原源码的17图集合混写 |
| Road-USA ILP64补充 | `runs/formal-10862162` | 360条计时、36组中位数、108条数值记录 | 同样是历史补充；后续源协议主动排除，并非项目从未尝试过 |
| 初始正确性/尾部/全空图 | `runs/smoke-10861412/10861414/10861420/10861922/10861973/10862001/10862091` | 七版共994条已有计时全部对stdout；smoke仅验证程序；逐个stderr及correctness检查 | `10861414/tail-n17-q0` 有 `FATAL MKL create`；后续显式处理零nnz。失败run不能因已有部分PASS记录改写为成功 |
| 对齐源代码协议 | `runs/source-formal-10864754` 与 `10864792` | generator插入/去除可恢复原源码；allocator、初始化顺序、线程/MKL/default NUMA、min5口径；2,890计时全部匹配stdout并复算578组min/median；765条数值gate | 该阶段对齐的是旧 `gcn_e2e_bench.cpp`，后来发现它不是论文Table2对应版本。因此早期0.891×等不能解释论文TFS能力 |
| RGG边界修复、live launcher失效及隔离验证 | `docs/SOURCE_RGG_BOUNDARY_20261004.event.json`、`SOURCE_LAUNCHER_ISSUE_20261004.event.json`、`SOURCE_LAUNCHER_VALIDATION_20261004.event.json`；`source-smoke-10864752/10864830` | 读事件与修复代码；两次smoke各380计时/102数值gate；正式main的exit2和逐图完成分开 | N*128==2^31 最后索引仍可用，>=改>后补跑RGG；运行中修改shell导致收尾读取错位，后续launcher快照隔离；保留失败job事实 |
| 论文V3基线真正复现 | `runs/paper-formal-10864848`、`docs/PAPER_REPRODUCTION_RESULTS_20261004.md`、`scripts/paper_reproduction_suite.py` | 340条原始repeats逐一与e2e/tfs_kernel/mkl_kernel stdout对账；68组min/median复算；17完整BF16输出gate核对 | 旧bench→`gcn_e2e_v3.cpp`不只是名字变化。17图min5 E2E GM1.90616，median5 GM1.88592；与早期0.891结论方向不同。源码BEST=min，不能标成median |
| 真实特征和权重资产盘点 | `docs/YX_REAL_GCN_ASSETS_20261003.md`、`runs/paper-assets-20261004-120700` | 读资产说明、元数据约束及paper复现中的对应说明；未运行pickle或训练 | Reddit602维、Products100维真实特征已有记录；调试单步W不是完整checkpoint；这不是已完成真实模型准确率实验 |
| 10月4日清理及后续paper-method/cache | 清理提交 `bdcf9b0`；当前保留的 `paper-method-*`、`paper-cache-*` | 对照删除路径与提交；后续正式实验另见coverage_oct04.md | 清理/归档/上传不是新的性能算法；删旧事件不等于历史没有发生 |

说明：smoke计时994是七个smoke run的已有运行记录计数（88+118+148+88+184+184+184）；不能用共享节点smoke推正式性能。

## 整体原始证据重算

`coverage_history_raw.py` 对上述旧 `formal-*` 与 `smoke-*` 共13次执行读取全部 `timings.csv`，**10,294 条 measured timing 均与 stdout 的同method/kind/repeat吻合**，6位打印精度容差1.1e-6 ms。1,672个summary中位数全部复算一致。所有已写出的correctness.csv gate为PASS；这不抹去后来发生的MKL全空图错误或先行拒绝的6个加权图。

source协议正式及smoke：从全部graph-local `time.csv`复算 min/median，随后补齐原始baseline的stdout解析。正式2,890条及两个smoke共760条全部匹配实际stdout（baseline E2E使用原日志0.01ms打印值，其余用SOURCE_TIME）。独立未修改binary的额外anchor不作为该表分母，未混进这项数量。paper V3的340条baseline计时也全部独立解析原始日志并对账。

脚本输出 `coverage_history_raw.json` 保留每run/graph/method的计数与重算值。stderr下的6项非单位边权拒绝和1项旧零nnz错误是实验记录；不是审计发现数据被篡改。

## 会改变研究判断的历史事实

1. **replay已经做过**。主程序默认将 replay_b8、replay_b32、replay_bfull 加入方法；控制运行还有replay_bfull_fast_nozero。panel外循环重复聚合换取C tile驻留。后续正式paper_methods不启用它，不等于整个项目从未测它。
2. **原始基线需要按版本定位**。早期源对齐的基准不是论文Table2真正对应版本；不能把旧统计移到v3，也不能只根据stdout里“V3”字样识别源码。
3. **精度门槛本身被修补过**。论文原计时最终写BF16，原precision段却另算FP32末层；独立validator补了实际BF16输出完整检查。maxnorm与Frobenius相对L2也不同。
4. **两张大图不是从未测试**。先有LP64失败和ILP64成功，再在忠实原实现协议中排除。方法覆盖与论文可比集合是两件事。
5. **早期性能策略已被后续协议替代**。显式NUMA interleave、强制MKL线程、初始化/统计口径变化均记录在 `SOURCE_MKL_PROTOCOL_20261004.md`；不能把这些数值当成现行默认NUMA结果。

## 本页仍不宣称完成的内容

- 没有反汇编所有历史编译产物，也未逐行人工阅读每份重复快照。
- 没有把438个服务器事件或140个runs目录当成438/140个独立科学实验。
- 没有重新运行性能实验；本页验证的是既有证据的来源、解析和结论边界。
- 历史清理前素材可恢复并已复核核心执行/结果，不代表所有19个Git提交的每个中间文件都逐字读完。被冻结的运行版本与实际结果优先于未执行的中间编辑。
