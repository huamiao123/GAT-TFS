# 原 TFS 源码审计与 GAT 适配

2026-09-28 只读检查：D:\Apaper\GAT\code\code 与服务器 data/yx/TFS/code。服务器原始源码没有修改。主要参照 amx_tfs_v3.cpp、amx_tfs_v3_adaptive_R.cpp、amx_tfs_fusion.cpp、gcn_e2e_v3.cpp。服务器 SHA256 分别为 4e574786055816df80beb6e68181fe19a8ee02cf4be1d43717bc1e79b88cab29、036bea147aa1133e058fb334c14673e13c55c5a4deca67b395e67d95f5b81646、6e779950d208d41f5dfa58bd64f1e623d445f6f23d971b444e1ba1c052bee727、85c51bde92792f98223e80107e20b042f8f8d0e40068cdf6ce168143f0c7743f。

| 要素 | 源码实际实现 | GAT FP32 适配 |
|---|---|---|
| CSR | .csrbin 的 36 字节头，uint32 indptr/indices；静态邻接 | GAT 原有 .gatbin：uint64 rowptr、uint32 col，destination row；保留现有图和 self-loop |
| Degree Sort / 行调度 | amx_tfs_v3.cpp 约 101–115 行按 degree 升序生成 perm[sorted_index]=original_row；约 161 行对 R 行 panel 使用 OpenMP dynamic,1 | 复用相同逻辑排序与 panel 调度，输出仍按 original_row 写；不物理重排 H |
| 行分块 | R 默认 64；每组 TR=16 行，max degree 控制邻居 step，短行提前失活；adaptive_R 根据 max/avg degree 选 128/64/32/16 | R 可配置，TR=16，复用逻辑 row panel；GAT 各 row/head 邻居 block 保留独立 m,l,U |
| Feature / output blocking | K_IN=K_OUT=128 的源码配置；每 32 个输入特征一个 kb，每 16 个输出特征一个 ob；固定 7 个 AMX tile | GAT 支持 D=128/256、head_dim=32/C；局部 U tile 为最多 16×D，GeMM 输出最多 16×head_dim |
| BF16 packing | H 转 BF16；W 按 [KB][NB][16 k-pair][32 BF16] VNNI 排列；函数直接截取 FP32 高 16 位 | 首版保留 FP32，避免 BF16 使与原 FP32 GAT 的严格误差门限失效；没有声称已运行 AMX |
| SpMM→GeMM 融合 | 原核逐 neighbor step gather H 到 Hbuf，立即用 AMX tile_dpbf16ps 和 W 累加输出 C；没有完整 N×D 的中间 Z/U | GAT 增加动态 alpha 和 online rescale；把 16×D FP32 U 留在 tile 本地，随后立即 GeMM 并只写最终 head 输出。与原核的 panel/row/tile 数据驻留思想一致；计算指令不是原 BF16 AMX |
| tile 配置 / spill | tilecfg palette=1、7 tiles 均 16 行×64 字节；_tile_loadd/_tile_dpbf16ps/_tile_stored | 当前 FP32 版无 TMM，因此 AMX tile load/compute/store、BF16 packing 和 TMM spill/reload 均标注为未使用，不能报伪测量值 |
| OpenMP / 原始行恢复 | 每线程 AMX 配置与 Hbuf；dynamic,1 分配 row panels；perm 仅逻辑排序，结果写回 original_row | 同样以 panel 为 OpenMP 工作单位，每线程局部 U/V buffer，输出写原节点编号 |
| NUMA | 上述主核中未发现显式 numactl/mbind 或 NUMA pinning；内存由系统默认策略决定 | 不声称复用了不存在的 NUMA 策略；实验记录线程与节点 |

重要差异：原 TFS 是静态未加权邻接、固定 128×128、BF16 AMX 内积；GAT 的 A_alpha 每层每 head 动态生成，并在 running max 提高时要求按行 rescale。不能把原核直接改名后宣称适配。当前 tfs_online_fused 是**FP32、原 TFS row-panel 调度与局部中间 tile 驻留的正确性/性能适配**，不是 AMX BF16 内核。它不完整落地 N×D 的 U；tfs_online_reference 则完整落地以隔离代数与融合收益。

后续如引入 AMX，必须另外处理精度、动态 p 对 H 的加权、每行独立 rescale 和 TMM spill/reload；不得沿用 FP32 的 1e-7 误差结论。