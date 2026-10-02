# 原 TFS 的独立双 head 读取复用候选

日期：2026-10-02。状态：**服务器编译、57 组算子控制、2 组层控制及 arxiv/products 完整三层实验已完成，作业 10853989。实现对原 Online 的逐位控制通过；真实图 master 精度尚未验收。**

此前受限会话 SSH 返回 `Permission denied`，当时只有本地实现。随后网络权限恢复，服务器同步和验证已完成。`10851370` 的 13.882s 属于旧单 head 循环；本候选与原 Online 的公平对照使用同一次 `10853989` 作业的数据。

## 同一次作业的完整三层结果

| 路径 | arxiv，ms | products，s |
|---|---:|---:|
| B0 FP32 | 73.486 | 3.827 |
| B0 BF16 | 67.191 | 3.723 |
| 原 B1 TFS | 435.465 | 15.800 |
| 原 TFS Online B32 | 411.819 | 14.752 |
| 双 head TFS Online B32 | 328.160 | 10.024 |

双 head 相对本作业原 Online 约加速 **1.255× / 1.472×**；相对原 B1 约 **1.327× / 1.576×**。仍慢于 B0 BF16。共享节点、seed11 未训练权重、1 warmup/3 alternating reps，全部为探索性结果。原 master `.003` 门没有放宽，模型精度尚未通过。

节点 qhcn023，16 物理核 0–15、socket0、NUMA0/1 interleave。MaxRSS 49,983,688 KiB。前两次作业 `10853968/10853976` 因核分配跨 socket 在数值运行前失败并保留；指定当时空闲的共享节点后通过相同 affinity guard。原始证据见 [`runs/icpp-pair-10853989`](../runs/icpp-pair-10853989/manifest.txt)。

## 改动的边界

保留Degree Sort、destination-row CSR、TR16同步邻居步、BF16(pH)W、真实AMX和输出TMM驻留。两个head只共用源节点索引及raw BF16 H读取；各自的W、L/R、score、p、m/l、输出V和rescale完全独立。

对K8/d32，两个head分别使用TMM0/1与TMM2/3；TMM4/5为输入/权重操作数。每个head的邻居顺序、kb累加顺序、BF16 RNE保持原路径定义。某head最大值上涨时只staging该head的输出tiles。

仅even heads且d<=32使用pair内核；其它形状回退现有Online核。固定模型L1/L2使用双head读取复用，L3单head回退。没有修改模型、共享attention、缓存完整Z或完整U，也没有把AMX改为末尾SGEMM。

## 收益与局限

第二层对每条边raw H逻辑读取的head重复数由8降到4。对于products理论raw H逻辑字节由516.780GB降到258.390GB，但**不是DRAM实测结果，也不是2×E2E加速承诺**。

每个head仍须独立生成BF16(pH)，AMX投影次数不减少。此前算出的约42.887×有效算术膨胀仍存在。两个head的权重与scratch工作集增大、动态任务变重，实际收益必须测量。

## 源码与入口

- [`icpp_pair.hpp`](../tfs_online/icpp_pair.hpp)、[`icpp_pair.cpp`](../tfs_online/icpp_pair.cpp)：候选workspace、内核、fallback及计数。
- [`pair_checks.hpp`](../tfs_online/pair_checks.hpp)、[`pair_checks.cpp`](../tfs_online/pair_checks.cpp)：对原Online核的逐位/状态/计数控制。
- [`pair_main.cpp`](../tfs_online/pair_main.cpp)：同一binary的B0FP32/BF16、原B1、原Online和pair三层对照。
- [`build_icpp_pair.sh`](../scripts/build_icpp_pair.sh)、[`run_icpp_pair.slurm`](../scripts/run_icpp_pair.slurm)、[`icpp_pair_payload.sh`](../scripts/icpp_pair_payload.sh)。

旧Online build使用显式source列表，排除新增pair_main，保留旧入口独立构建。原src/include、icpp_online.cpp/main.cpp未改。

57 个算子控制及 2 个完整 layer wrapper 已通过，覆盖空行、尾部、重复边、block 跨越、相反 head 最大值更新历史、同分数和 fallback。profile/counters 四种组合的 numerator、denominator、running max、归一化输出逐位一致，逐行统计与全量计数一致。两张真实图完整三层各自输出链也通过原 Online 的逐位控制，独立 profile fingerprint 匹配。**这些通过的是实现控制，不能替代 FP32 master 或训练 checkpoint 精度门。**

最初未运行阶段的源码清单、SHA256 和冻结快照保存在 [`icpp-pair-local-20261002`](../runs/icpp-pair-local-20261002/manifest.json)，作为历史状态保留。当前已测源快照和 hashes 在 `icpp-pair-10853989`；服务器三份 handoff 已同步完成。

## 验证顺序

1. 恢复网络后先同步上次成功实验到服务器三份handoff；读取当前AGENTS/skill/contract并核对queue/partition/source。
2. 登录节点仅编译，保存唯一build目录、hashes、commands和AMX assembly。
3. 一个共享intel节点、16物理核、80GiB内存；先原48组smoke，再pair边界/独立head/empty/tail/fallback控制。
4. 小图及真实三层要求pair对原Online的out/den/max逐位一致；counter/profile开关算术不变，原master .003门不变。
5. arxiv/products，block32，同一binding、own outputs，1warmup、3交替重复；profile另做。

读次数、Tile spill/reload、rescale和AMX/FMA保留全量计数。新增coarse spans包住整个邻居步的feature staging和完整AMX kb循环，减少逐16维/逐TDP读钟的扰动。仍是采样worker时间，不当作真实wall百分比。fallback使用原细分计时，以标签区分。

**读取复用已有真实实验支持；8× D-wide 算术量和逐邻居重复投影仍未消除。下一步测试改变融合粒度的 block-local U→AMX UW；它是新量化路径，需要独立 oracle。**
