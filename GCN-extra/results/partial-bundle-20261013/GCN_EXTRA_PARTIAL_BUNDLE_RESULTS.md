# 保持量化边界的 partial bundle：五图实现与实验结果

分析日期：2026-10-14。GCN-extra 静态单位边权、随机 H/W seed12345、128→128→128，两层前向；不是 checkpoint 分类实验。

## 结论

新 G=1/2/4 路径已实现、编译并完成小图与五张诊断图验证。保持 DegreeSort/TR16/R64、S64、M64/M256/FULL、FAST/ACCURATE、BF16 截断、原权重打包和 AMX 每个输出元素的投影顺序。只调整已量化 partial 的缓冲容量、输出 panel 循环和 C 暂存频率；原 TFS/MKL 与 frozen 依赖不修改。

五图增量总体较小，没有形成值得立即扩展17图的强通用收益。减少 C 暂存请求已经实现，但未减少逐边 source 读取/解码/加法或 AMX 投影。FULL 的单 partial 控制也存在时间变化，所以不能把全部差异归因于 C traffic。

## 环境与完整性

build：`partial-bundle-build-20261013-225110`。smoke Job10944718/qhcn015，一物理核，9秒；performance Job10944719/qhcn198，32物理核、单socket、close/cores、默认NUMA、无interleave，共享intel，25分22秒。两作业 COMPLETED 0:0。共享结果为探索性证据，`paper_eligible=false`。

PASS：五图440数值门槛、420逐位边界检查、200源级工作量检查、200production插桩一致性检查；另有五张小图、原19值EPI语义、抵消及bundle尾部检查。`audit.json`保存全部计数。

同精度同M的kernel FP32、最终FP32、H1 BF16、最终BF16均逐位一致。相对原TFS的FAST FP32门槛.01、ACCURATE .001，BF16分别.02/.01；相对原FP32 MKL BF16门槛.03未放宽。

## 改动与真实数据流

```text
原控制：归约一个 M partial→独立 hi/lo 截断→两个panel投影→保存C
新方法：分别归约/截断 G 个 M partial→一个panel依次消费这些partial→保存C→另一个panel
```

G不会将多个partial合并后再量化。各输出元素仍按原scope/KB/hi-lo顺序累加；生产Layer1使用已验证PACK_EPI直接写H1b。FULL每tile只有一个有效partial，G4不减少其C存取，是必要负控制。

## 主性能：完整两层，10次正反顺序中位数，ms

ratio=同M同精度PACK_EPI时间/候选时间，>1表示候选更快。未使用逐图最快G形成方法，G2/G4固定策略分别列出。完整两协议结果在paired_comparisons.csv。

### M64 FAST

| 图 | 强控制 | G1 | G2 | G4 | 控制/G2 | 控制/G4 |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 282.497 | 279.642 | 279.258 | 279.304 | 1.0116 | 1.0114 |
| reddit | 121.740 | 124.444 | 115.338 | 117.714 | 1.0555 | 1.0342 |
| mycielskian19 | 1103.324 | 1120.728 | 1111.381 | 1096.049 | 0.9928 | 1.0066 |
| roadNet-CA | 23.831 | 23.760 | 23.841 | 23.497 | 0.9996 | 1.0142 |
| wiki-Talk | 38.533 | 38.027 | 37.138 | 37.212 | 1.0376 | 1.0355 |

固定G2五图描述性GM：1.0191×。
固定G4五图描述性GM：1.0203×。

### M256 FAST

| 图 | 强控制 | G1 | G2 | G4 | 控制/G2 | 控制/G4 |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 274.775 | 274.137 | 273.558 | 274.639 | 1.0045 | 1.0005 |
| reddit | 116.810 | 117.690 | 118.506 | 116.002 | 0.9857 | 1.0070 |
| mycielskian19 | 1106.130 | 1091.672 | 1106.484 | 1094.546 | 0.9997 | 1.0106 |
| roadNet-CA | 23.439 | 23.558 | 23.626 | 23.210 | 0.9921 | 1.0098 |
| wiki-Talk | 36.853 | 36.377 | 36.216 | 36.077 | 1.0176 | 1.0215 |

固定G2五图描述性GM：0.9998×。
固定G4五图描述性GM：1.0099×。

### M64 ACCURATE

| 图 | 强控制 | G1 | G2 | G4 | 控制/G2 | 控制/G4 |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 299.194 | 299.496 | 295.626 | 294.617 | 1.0121 | 1.0155 |
| reddit | 124.217 | 123.568 | 115.620 | 116.411 | 1.0744 | 1.0671 |
| mycielskian19 | 1112.234 | 1119.784 | 1104.766 | 1092.805 | 1.0068 | 1.0178 |
| roadNet-CA | 29.967 | 29.765 | 29.820 | 29.914 | 1.0049 | 1.0017 |
| wiki-Talk | 39.833 | 39.959 | 38.884 | 38.828 | 1.0244 | 1.0259 |

固定G2五图描述性GM：1.0242×。
固定G4五图描述性GM：1.0254×。

### M256 ACCURATE

| 图 | 强控制 | G1 | G2 | G4 | 控制/G2 | 控制/G4 |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 286.787 | 287.613 | 283.832 | 285.956 | 1.0104 | 1.0029 |
| reddit | 119.361 | 119.696 | 116.428 | 114.108 | 1.0252 | 1.0460 |
| mycielskian19 | 1111.409 | 1097.703 | 1090.855 | 1101.427 | 1.0188 | 1.0091 |
| roadNet-CA | 30.076 | 29.711 | 29.845 | 29.794 | 1.0078 | 1.0095 |
| wiki-Talk | 37.345 | 36.581 | 37.066 | 37.536 | 1.0075 | 0.9949 |

固定G2五图描述性GM：1.0139×。
固定G4五图描述性GM：1.0123×。

### FULL：没有C暂存减少的控制

| 图 | FAST控制 | FAST G4 | ratio | ACCURATE控制 | ACCURATE G4 | ratio |
| --- | --- | --- | --- | --- | --- | --- |
| ogbn-products | 274.790 | 269.423 | 1.0199 | 285.494 | 282.238 | 1.0115 |
| reddit | 117.025 | 114.991 | 1.0177 | 113.717 | 114.385 | 0.9942 |
| mycielskian19 | 1103.498 | 1078.405 | 1.0233 | 1081.668 | 1093.978 | 0.9887 |
| roadNet-CA | 23.447 | 23.533 | 0.9963 | 30.412 | 30.264 | 1.0049 |
| wiki-Talk | 36.441 | 35.476 | 1.0272 | 36.529 | 36.553 | 0.9993 |

FULL的变化不能由减少C store/reload解释。代码生成、控制流程、地址/缓存状态、共享节点噪声等尚未被逐项隔离；不提前指定唯一原因。

## 原TFS/MKL锚点与累积收益

| 图 | 源TFS ms | 源MKL FP32 ms | PACK_EPI FULL FAST ms |
| --- | --- | --- | --- |
| ogbn-products | 337.793 | 1353.806 | 274.790 |
| reddit | 301.960 | 441.642 | 117.025 |
| mycielskian19 | 2189.355 | 4877.611 | 1103.498 |
| roadNet-CA | 35.944 | 66.274 | 23.447 |
| wiki-Talk | 108.433 | 91.233 | 36.441 |

源TFS/MKL保持原实现；它们精度不同。已有邻居归约/EPI收益不得归给本轮bundle。增量归因只能使用上面的同M同精度控制。

## 工作量与逻辑访存

以下单层请求，不是实测DRAM。每层source=256E bytes，decode=8E组，FP32 scalar add=128E；TDP保持32Q/64Q。C原请求=8192(2Q-1)/tile，新请求=8192(2ceil(Q/G)-1)。

| 图/M | source GB | C控制 GB | C G4 GB | C减少 | 所省C/source |
| --- | --- | --- | --- | --- | --- |
| ogbn-products/m64 | 31.672 | 2.343 | 1.311 | 44.03% | 3.26% |
| reddit/m64 | 29.342 | 1.847 | 0.481 | 73.94% | 4.65% |
| mycielskian19/m64 | 231.218 | 14.496 | 3.642 | 74.88% | 4.69% |
| roadNet-CA/m64 | 1.417 | 1.006 | 1.006 | 0.00% | 0.00% |
| wiki-Talk/m64 | 1.285 | 0.172 | 0.097 | 43.78% | 5.87% |

即使C相对自身大幅下降，对source请求的节省也有限，且C主要是局部状态。不能将两种逻辑bytes按相同DRAM成本相加。bundle扩大hi/lo存活工作集，也可能抵消收益。model中的packed_bundle_capacity是活跃表示容量；代码保留hi/lo数组，不能当作实际stack/RSS测量。

## Stage与细分计时

| 图 | 方法 | Layer1 ms | ReLU/Convert | Layer2 ms | Stage Total ms |
| --- | --- | --- | --- | --- | --- |
| ogbn-products | pack_epi_m64_fast | 142.186 | fused inside Layer1 | 141.102 | 284.359 |
| ogbn-products | bundle_g2_m64_fast | 139.350 | fused inside Layer1 | 139.331 | 278.681 |
| ogbn-products | bundle_g4_m64_fast | 139.956 | fused inside Layer1 | 141.005 | 280.961 |
| reddit | pack_epi_m64_fast | 60.061 | fused inside Layer1 | 62.494 | 122.515 |
| reddit | bundle_g2_m64_fast | 60.841 | fused inside Layer1 | 59.357 | 120.811 |
| reddit | bundle_g4_m64_fast | 58.060 | fused inside Layer1 | 59.578 | 117.638 |
| mycielskian19 | pack_epi_m64_fast | 559.422 | fused inside Layer1 | 552.018 | 1120.679 |
| mycielskian19 | bundle_g2_m64_fast | 556.824 | fused inside Layer1 | 539.210 | 1090.675 |
| mycielskian19 | bundle_g4_m64_fast | 562.922 | fused inside Layer1 | 533.818 | 1094.409 |
| roadNet-CA | pack_epi_m64_fast | 11.713 | fused inside Layer1 | 11.914 | 23.641 |
| roadNet-CA | bundle_g2_m64_fast | 11.670 | fused inside Layer1 | 12.108 | 23.749 |
| roadNet-CA | bundle_g4_m64_fast | 11.508 | fused inside Layer1 | 11.956 | 23.499 |
| wiki-Talk | pack_epi_m64_fast | 18.175 | fused inside Layer1 | 19.712 | 37.544 |
| wiki-Talk | bundle_g2_m64_fast | 19.487 | fused inside Layer1 | 19.561 | 38.553 |
| wiki-Talk | bundle_g4_m64_fast | 18.110 | fused inside Layer1 | 18.387 | 36.286 |

独立3-repeat stage，不与主计时混分母；每列分别取中位数，段之和可能不同于total中位数。EPI中的ReLU/转换不是没有工作，而是被计入Layer1。

保存12粗阶段、24采样子阶段，以及每线程active/start/finish/edges/tiles。子项覆盖source load+decode、FP32 add、partial状态、hi/lo packing、AMX A/B load、TDP、C zero/load/store、EPI/store及线程setup。原始4800细分记录与6400线程记录都保留。

这些是独立插桩版本的sampled/thread-local diagnostic time；有父子重叠、时钟开销和代码调度变化，不能求和当E2E wall百分比，更不能仅由AMX compute timer宣称硬件重叠。

下面仅汇总两层的粗采样线程诊断，单位sampled thread ms；不是E2E分解。

| 图/方法 | load+decode | FP32 add | tile load | TDP interval | tile store |
| --- | --- | --- | --- | --- | --- |
| ogbn-products/pack_epi_m64_fast | 33.435 | 56.355 | 4.800 | 5.880 | 0.404 |
| ogbn-products/bundle_g4_m64_fast | 33.264 | 56.877 | 4.334 | 4.725 | 0.211 |
| reddit/pack_epi_m64_fast | 30.051 | 55.790 | 3.144 | 3.487 | 0.241 |
| reddit/bundle_g4_m64_fast | 29.989 | 56.188 | 2.716 | 2.226 | 0.063 |
| mycielskian19/pack_epi_m64_fast | 346.307 | 651.219 | 29.323 | 37.975 | 2.260 |
| mycielskian19/bundle_g4_m64_fast | 346.027 | 649.961 | 23.336 | 18.615 | 0.556 |

解码/归约粗诊断接近，tile store区间减少，与固定工作和C请求下降方向一致。TDP interval也变化，但TDP次数不变，不能解释为算术工作减少或据此算预测加速。

24个子阶段保留统一schema，但未被当前ScalarReducer单独插桩的source_bf16_load/source_bf16_decode，以及融合EPI中未单独拆分的conversion/store，不能把零事件读作零成本。load+decode目前以粗阶段共同记录；子项是否有测量必须看events。

## PMU

独立完整forward PMU：45/45条通过32线程覆盖与scaling检查，每方法3次。记录cycles/instructions/generic cache misses/references与AMX speculative busy cycles；没有L1/L2/LLC细分、DRAM BW或TLB。初末线程快照不能证明覆盖所有瞬时线程。

| 图/方法 | cycles B | instructions B | IPC | generic misses M | AMX busy M |
| --- | --- | --- | --- | --- | --- |
| ogbn-products/pack_epi_m64_fast | 25.904 | 11.132 | 0.430 | 563.402 | 224.304 |
| ogbn-products/bundle_g4_m64_fast | 25.450 | 11.115 | 0.437 | 559.816 | 224.018 |
| reddit/pack_epi_m64_fast | 11.444 | 10.423 | 0.911 | 649.543 | 122.977 |
| reddit/bundle_g4_m64_fast | 10.217 | 10.259 | 1.004 | 648.949 | 122.936 |
| mycielskian19/pack_epi_m64_fast | 98.976 | 80.869 | 0.817 | 4006.780 | 925.148 |
| mycielskian19/bundle_g4_m64_fast | 98.242 | 84.005 | 0.855 | 4026.374 | 921.466 |
| roadNet-CA/pack_epi_m64_fast | 1.965 | 1.012 | 0.515 | 14.742 | 125.971 |
| roadNet-CA/bundle_g4_m64_fast | 1.954 | 1.046 | 0.535 | 14.990 | 125.973 |
| wiki-Talk/pack_epi_m64_fast | 3.551 | 2.817 | 0.793 | 27.633 | 15.536 |
| wiki-Talk/bundle_g4_m64_fast | 3.492 | 2.787 | 0.798 | 26.822 | 15.511 |

以上为线程求和counter中位数，IPC为counter中位数之比。AMX busy是speculative arithmetic busy cycles，不是退役TDP数或AMX wall time。PMU region包含runtime；启停有时间跨度，不能直接当严格内核占比。

## 稳定性、逐图解释与下一步

两个主协议共230条方法汇总，其中4条CV>5%，原样保留在timing_anomalies.csv。不称小幅差异具有统计显著性。

- Products：固定G的改善小；FULL负控制也变化，不能证明C优化是主要因素。
- Reddit：M64下G2 FAST和G4 ACCURATE有较明显正增量，但应面对FULL同精度强控制与独立复测，不能推广为通用策略。
- Myciel：C逻辑请求明显减少，主时间仍约秒级，说明本轮没有解决逐边重负载；具体硬件原因不唯一。
- RoadNet：大多数tile仅一个scope，bundle基本没有C存取可省，是自然负对照。
- wiki-Talk：部分改善，需结合偏斜/空行与两协议波动；不能用非空destination数量替代真实source工作。

当前不将bundle作为默认取代FULL的路径，也不立即铺17图。保留独立实现，作为量化边界与C驻留分离的已验证机制对照。下一阶段更应研究逐边source load/decode复用与partial寄存器状态的交易；已有PAIR/exact-window负结果必须作为控制。此结论不是证明任何更强bundle调度都无效。

## 保存与复现

源：build/source_snapshot；精确命令与原依赖hash：build/commands.txt、source.sha256、manifest.json。raw日志、全部CSV、affinity/NUMA/environment/graph hashes：logs/bundle-10944718与10944719。分析只读原记录，analysis脚本单独冻结；没有重写raw或源baseline。

派生文件：timing_summary.csv、paired_comparisons.csv、timing_anomalies.csv、work_comparison.csv、stage_summary.csv、pmu_summary.csv、audit.json。本报告不代表GitHub已提交。
