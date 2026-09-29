# GAT / TFS / AMX 实现状态（2026-09-29）

依据 `docs/GAT_TFS_AMX_问题审计与方案设计全记录.md` 实施（服务器副本位于 `wzh/GAT/docs/`）。本文件只记录当前实际代码与已运行的轻量验证；原始 2026-09-28 探索性结果不覆盖。

## 已落地

- `src/gat_online.cpp`：输出比较集中到一个显式有限值、尺寸和零参考保护的门禁；旧 `>` 判定漏过 NaN 的问题已消除。图读取补充 `N×D` 溢出和有限输入检查。在线路径复用每层 workspace，避免逐 row/head 分配 `OnlineState::u`。
- `src/gat_tfs_online.cpp`：三层 E2E 在最后一层结束立即停止计时，再比较和打印；attention 等价性增加逐边 score 检查。`benchmark` 模式每进程只运行一条路径，避免六条路径同时持有结果。`GAT_NO_FINE_TIMING` 构建删除行级统计维护和 OpenMP 统计合并；profile 输出声明 softmax 包含 rescale，未单独测的 transition 显示 `not_measured`。
- `src/online_block_state.hpp`：独立的软件分块状态参考，分别记录特征块的尺度版本和边前缀；分母每个邻居块仅更新一次。附带 Vanilla GAT 的精确最大值标量预扫描函数。它是状态协议验证工具，尚未接入高性能路径。
- `src/amx_head_row.hpp`：真正执行 `TDPBF16PS` 的单邻居块微核，head 作为行，共享同一节点邻居的原始特征；支持 1–8 heads、至多 32 条边、任意正 D 的特征尾部。每个 head 独立的块最大值与分母。BF16 权重和特征有单份及高低两部分模式，累加器为 FP32。
- `src/gat_tfs_amx.cpp`：实验性三层 aggregate-first AMX 前向。动态 gather/packing、各 head 标准分块 online 合并、局部 panel U、FP32 MKL GeMM、输出归一化、ELU 与原节点顺序写回。没有完整全局 U。当前默认使用 BF16 高低补偿，避免小图上单 BF16 精度失败。
- `scripts/build_tfs.sh`：根路径可从脚本位置解析，模块可用环境变量覆盖；单独构建 AMX profile/speed 与探针。
- `scripts/run_amx_arxiv.slurm`：为大图正确性及同线程数、同数据的匹配路径速度对照准备的作业脚本。**尚未提交作业。**

## 已运行的轻量验证

| 证据目录 | 结果 | 边界 |
| --- | --- | --- |
| `runs/implementation-smoke-JnRW5cWG/` | 状态探针先失败 | 原探针缺 `-fp-model precise`；保留失败日志，不作为GAT结果 |
| `runs/implementation-smoke-F83Zo7Xy/` | 状态协议 PASS、AMX单块 PASS、1024节点FP32三层 GAT PASS | 单线程登录节点轻量正确性 |
| `runs/amx-smoke-1PeSoU4T/` | 单BF16三层最大绝对误差 `0.0041688`，相对L2 `0.0026946`，未过原FP32门槛 | 负结果；诊断时间含输出打印 |
| `runs/amx-smoke-LPdex3Ln/` | BF16高低补偿三层最大绝对误差 `7.8380e-6`，相对L2 `4.1065e-6`，过当前FP32门槛 | 仅1024节点、随机权重；不代表真实checkpoint或全数据集 |
| `runs/amx-smoke-H0MRsG2C/` | 加入分段计时后复核相同误差，仍通过 | Layer 2 packing worker 累计约0.048 s、AMX tile compute约0.010 s；仅用于定位小图瓶颈，非正式速度结论 |
| `runs/amx-smoke-vbVtGW75/` | 增加逐层同输入门槛后，三层分别 PASS；三层 E2E 仍 PASS | Layer 1 首次 MKL 初始化使诊断时间显著变动；不用于速度结论 |

AMX 计时字段区分 packing、tile config/load/compute/store、在线块合并、局部 GeMM、归一化和层 wall time。worker 时间是线程累计值，不能与 wall time 相加。`base_accumulator_logical_bytes` 和 tile 字节数是软件逻辑口径，不是 DRAM 硬件流量。额外 rescale 与常规块合并共用同一遍 U，尚无独立的 rescale 时间测量。

## 尚未完成的验收

1. ogbn-arxiv 上的新 AMX 精度与匹配速度实验，尤其 Layer 2 的 8 倍稀疏工作量。`run_amx_arxiv.slurm` 需要按服务器 `AGENTS.md` 获得高负载作业授权后才能提交。
2. 真实 Vanilla GAT checkpoint、标签/划分和任务指标；仓库目前只有固定随机参数的前向实验。
3. 状态模拟器中的延迟消费、乱序特征块及回退协议向 AMX 路径移植；精确最大值预扫描、窗口参考和分段低维合并尚未成为生产候选。
4. 对实际 AMX 路径补充输入范围与溢出回退、硬件流量、NUMA/亲和性、峰值内存、短行专用路径及更强的 transform-first AMX/AVX 控制。
5. AMX 路径仍每块加载 tile 配置、分配 `BlockResult` 并写出块级结果；这是正确性优先的原型，尚未完成文档提出的 tile 生命周期与双缓冲优化。

当前阶段可以说“真实 AMX head-as-row 数据流已在小图跑通并通过当前FP32输出门槛”，不能说“大图或正式推理已经加速”。
