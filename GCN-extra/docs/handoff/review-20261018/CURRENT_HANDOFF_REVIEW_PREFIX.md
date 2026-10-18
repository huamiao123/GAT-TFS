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

