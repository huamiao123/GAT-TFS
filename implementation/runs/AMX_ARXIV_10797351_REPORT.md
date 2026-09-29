# ogbn-arxiv 三层 GAT / TFS / AMX 初步测量

日期：2026-09-29。有效作业：Slurm `10797351`，`intel` 共享节点 `qhcn006`，1 节点、16 CPU、OMP/MKL 各 16 线程，运行 48 秒。图为 169343 节点、2484941 条 CSR 边（含工程既定 self-loop 处理）；模型为 `D_in→8×32→8×32→1×40`。邻居块 32、row panel 64。作业脚本、源代码、二进制、数据和操作规则的 SHA-256 见 `amx-arxiv-10797351/artifact.sha256` 与 `preflight.sha256`。

计时边界为完整三层前向，不含图读取、调度预处理、随机模型初始化和正确性比较。每条路径单独启动进程，rep0 预热，rep1 和 rep2 为测量；下表取两个测量值的中点。正确性检查使用独立的 profile/verify 运行。共享节点和仅两次测量意味着这些数字是初步结果，不作为正式性能结论。

| 路径 | rep1 / s | rep2 / s | 中点 / s | 相对 transform-first 加速比 |
| --- | ---: | ---: | ---: | ---: |
| 匹配调度的 FP32 transform-first | 0.635683 | 0.657641 | 0.646662 | 1.000× |
| FP32 TFS-Online fused | 1.057586 | 1.136206 | 1.096896 | 0.590× |
| BF16 高低位补偿 AMX TFS-Online | 1.528660 | 1.500780 | 1.514720 | 0.427× |

AMX 相对 transform-first **慢 2.34 倍**，相对 FP32 TFS fused **慢 1.38 倍**。FP32 TFS fused 相对 transform-first 慢 1.70 倍。这里的 `0.427×` 是基线时间除以 AMX 时间，不能称为“加速了 0.427 倍”。

## 精度与瓶颈

FP32 六路径检查返回 `CORRECTNESS PASS`。AMX 与同输入的 FP32 reference 对比，三层输出最大绝对误差 `2.4586916e-6`、平均绝对误差 `1.94128404e-7`、相对 L2 `1.30553722e-6`，当前 FP32 容差通过；第二层相对 L2 `1.07754355e-6`。这验证的是固定随机参数的前向数值，不是已训练 GAT 的任务指标。

第二层 `D=256, K=8, d_h=32`：TFS 聚合的 sparse feature elements 为 `5,089,159,168`，transform-first 为 `636,144,896`，恰好 **8×**。AMX profile 的第二层 wall time 为 `0.621435 s`，其中 worker 累计 packing `3.258313 s`、tile compute `1.120430 s`、gather `1.444625 s`，packing 明显重于 tile 计算。worker 累计时间跨 16 线程，不能直接加到 wall time，也不是硬件 DRAM 流量。该层 `197586` 个邻居块，`48653` 次逻辑 rescale；当前实现将 rescale 融在块合并中，独立 rescale 时间尚未测得。当前不能把性能损失全部归因于 rescale；动态 gather/packing、BF16 双份表示及四次 tile dot 都值得进一步拆解。

## 作业记录

- `10797349`：启动即失败，Slurm 把相对脚本路径放到 `/var/spool/slurmd`，脚本错误地把那里识别为工程根目录；没有执行模型。将脚本默认根路径固定到 `wzh/GAT` 后提交 `10797351`。
- `10797351`：Slurm `COMPLETED 0:0`，原始日志在 `amx-arxiv-10797351/`。`fp32_correctness.log` 与 `amx_correctness.log` 是数值和分段诊断证据；`*_rep*.log` 是速度证据。

当前问题已有明确答案：这一版融合 AMX 数据流**没有抵消** multi-head 的稀疏工作量膨胀，也没有跑赢 FP32 TFS fused。正式结论还需独占节点、更多重复和固定亲和性复测。
