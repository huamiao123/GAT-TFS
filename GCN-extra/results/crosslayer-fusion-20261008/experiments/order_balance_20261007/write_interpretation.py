"""Write the explanation from audited measurements, with fixed comparisons."""
import csv
import json
import pathlib
import sys

root = pathlib.Path(sys.argv[1])
doc = pathlib.Path(sys.argv[2])


def rows(part, name):
    with (root/part/name).open(newline='', encoding='utf-8') as f:
        return list(csv.DictReader(f))


def table(headers, data):
    return '\n'.join(['| '+' | '.join(headers)+' |', '| '+' | '.join(['---']*len(headers))+' |']+
                     ['| '+' | '.join(map(str,r))+' |' for r in data])


graphs = ['ogbn-products','reddit','mycielskian19','roadNet-CA','wiki-Talk']
t3 = {(r['graph'],r['method']):float(r['median_ms']) for r in rows('p3','timing_summary.csv')}
t4 = {(r['graph'],r['shape'],r['method']):float(r['median_ms']) for r in rows('p4','timing_summary.csv')}
audit = json.loads((root/'ANALYSIS.json').read_text())
text = ['''# 综合研究方案续验：P3 残差与 P4 选择性物化

## 本轮回答及研究取舍

本轮完成两项实现与真实硬件验证：P3-D2 最终双分量残差修正、D3 每四块修正；P4 热点源
选择性投影与冷源局部融合。先 smoke，再受控图，再 Products、Reddit、Mycielskian19、
RoadNet-CA、wiki-Talk 五张诊断图。原 paper-compatible TFS/MKL 和旧 frozen snapshot 未改。

**D2/D3 在本轮数值门槛内通过，但五图均未胜过解耦 FULL ACCURATE。选择性物化在压缩层
有条件收益，然而没有超过所有普通投影优先／逐层混合强对照。** 当前证据不足以将二者
作为期刊主要改进，也不支持“选择性物化在所有形状下无效”的普遍判断。

源码：[journal_followup_p3_p4_20261005](../experiments/journal_followup_p3_p4_20261005/)。
全部结果：[JOURNAL_P3_P4_RESULTS_20261005.md](../results/journal-p3-p4-20261005/JOURNAL_P3_P4_RESULTS_20261005.md)。
原文：[综合研究方案](TFS_JOURNAL_STUDY_INPUT_20261004.txt)。
父提交 `af943926c7d4f8e68bcb95838924e21b14dd2e86`。

## 1. 环境、固定变量与计时边界

- P3 frozen build：`runs/residual-broad-build-20261005-121707`；smoke 作业10869475，真实图10869493/qhcn274。
- P4 aligned frozen build：`runs/selective-shape-build-20261005-123418`；smoke10869541，受控图10869544，真实图10869550/qhcn177。
- 所有作业 COMPLETED/0:0。主实验32个物理核、单 socket，共享 SPR、close/cores、默认NUMA，无interleave；smoke四核。本轮没有同时占用多个节点。
- 原 Intel oneAPI 2024.1 flags：`-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel`。
- TR16、R64、S64、原 DegreeSort、动态调度、BF16位截断与W packing、ReLU、原层间BF16边界及最终BF16输出。图不加自环、不重新排列行内邻居，按文件中的单位边权执行。
- H/W 使用原源码离散 `rand()%200` 分布及seed12345；等宽128形状保留原交错W1/W2随机数序列。不是训练好模型，不报告分类准确率。
- 各图的方法在同进程、同节点内比较。warmup后五次正反交替 prepared forward，另三次stage、三次独立PMU。prepared区间不含初始H/W准备、CSR载入与调度预处理，含层间转换及P4每层Q重建。
- 共享节点和五次样本只能支持当前机制判断；没有删异常样本，没有以事后最佳参数构造已部署策略。

P4 为独立 **shape/dataflow harness**，测128→32→32和128→128→128；没有将它的绝对时间
替换到原论文复现表。不能用不同节点、buffer footprint、harness的P3/P4时间互作分母。
等宽P4的原始第一层FP32输出逐位匹配 frozen P0 FULL；数值一致仍不等于跨harness的执行
环境和辅助阶段时间相同。原风格shape MKL使用FP32 master H/W、FP32层间值，与AMX路径
的BF16边界不同；保留为单独anchor，不将其速度比解释为纯融合收益。

## 2. 正确性与证据核验

P3：160条数值检查，40个方法/图接受记录，16个插桩逐位检查；P4：540条数值检查，
180个接受记录，50个退化/原P0逐位检查，360个双层插桩逐位检查，360条实际工作量检查。
全部通过、零拒绝。计数包含aligned smoke和受控图。P3-D1在上一轮抵消反例已被拒绝，
这个结论没有被D2通过随机输入覆盖。

P3相对原TFS的FP32 relative_L2及normalized_max门槛为1e-3；BF16-final相对原TFS为1e-2，
相对源FP32 MKL为3e-2。P4数学reference使用解码后的BF16 H/W，并保留同样的BF16层间
边界；FP32门槛1e-3，BF16-final门槛1e-2。门槛是本实验的输出范数检验，不是逐元素
相对误差、任意输入误差证明或分类精度保证。
''']
checks = []
for p in ['p3','p4']:
    for r in audit[p]['check_boundary_maxima']:
        checks.append([p,r['boundary'],r['reference'],f"{r['max_rel_L2']:.6g}",f"{r['max_normalized_max']:.6g}"])
text.append(table(['实验','检查边界','参考','最大relative L2','最大normalized max'],checks))
text += ['''
所有 source.sha256 与原文件核验、实际edge/hot/cold/TDP计数与拓扑计划交叉核验、
Slurm状态/exit status/结果行数核对完成。660个PMU region成功，线程初末快照与scaled记录
完整；不保证捕获瞬时线程。先前P4首次smoke10869522使用小连续随机分布、八路径，144条
检查通过；在跑真实图前已换成aligned新build。旧smoke作为superseded证据独立保存，
不进入本轮主表和上述计数。分析脚本首次因P3工作量表没有pass列中止，修复schema后全量
重跑；这不是kernel失败，也没有删除或重跑性能样本。

## 3. P3：双分量与周期残差修正

### 数据流和实际新增工作

每个B64 partial先FP32归约，高分量BF16立即投影；将 `partial-decode(hi)` 累加到每线程
8KiB FP32残差。D2在tile末尾做残差hi/lo两份投影，D3每四个B64块及最后尾块做同样修正。
保留local partial、AMX、输出tile staging，没有完整AH全局物化。

令U=Σtile块数、T=非空tile数：B64 ACCURATE为2U份，D2为U+2T份，FULL ACCURATE为2T份。
所以D2只有U>2T时才比B64少投影，从来不比FULL少。D3增加修正频率及状态操作；实际
corrections/parts/TDP都保存，没有用理论有效行数代替补齐16行后的执行量。

### 五张图的完整两层prepared E2E

单位ms；速度比=FULL/D2，大于1表示D2更快。
''']
text.append(table(['图','原TFS','B64 ACC','FULL ACC','D2','D3 period4','FULL/D2'],[
    [g]+[f'{t3[g,m]:.3f}' for m in ['paper_tfs','b64_accurate','s64_mfull_accurate','d2_b64_all','d3_b64_period4']]+[f"{t3[g,'s64_mfull_accurate']/t3[g,'d2_b64_all']:.4f}"] for g in graphs]))
work = {r['graph']:r for r in rows('p3','projection_comparison.csv') if r['method']=='d2_b64_all'}
text.append('\n'+table(['图','U','T','B64份数','D2份数','FULL份数'],[
    [g,work[g]['blocks'],work[g]['nonempty_tiles'],work[g]['B64_accurate_parts'],work[g]['parts'],work[g]['FULL_accurate_parts']] for g in graphs]))
text += ['''
Myciel的D2相对B64约1.039×，但相对FULL为0.9965×，0.35%的差异不能支持稳定胜负。
五图固定D2相对FULL几何平均0.9317×；D3为0.9270×，两者都没有胜出图。
Products的U/T约1.45，D2比B64反而增加18.8%投影；RoadNet约1，D2增加50%。
RoadNet的独立stage中，FULL Layer1/Layer2为17.35/14.41ms，D2为20.87/17.69ms，
ReLU与转换接近，退化发生在kernel。Myciel D2的AMX_BUSY计数大幅低于B64，E2E只改善
约4%，说明减少矩阵工作不能直接等同于总时间收益；残差状态和既有稀疏归约没有消失。

26个细分阶段覆盖source load/decode、归约、hi转换、residual subtract/load/add/store、
修正hi/lo、Cload/store、AMX A/Bload、compute、输出等。**P3细分profile目前只覆盖Layer1**，
两层均有独立stage时间；不能声称第二层也做了26项拆分。RoadNet D2 residual load/add/store
分别采样到1.697/1.768/1.737 thread-ms，这些含插桩且非wall分解，不据此计算E2E占比。

判断：保留P3作为数值/成本边界证据；目前停止将D2/D3包装成性能主改进。若未来有明确的
有限投影范围需求，再研究误差约束和状态生命周期；不能仅为保留残差模块而弱化FULL对照。

## 4. P4：热点源投影与冷源TFS融合

### 两条真实数据流

热点源S：BF16 H_S×BF16 W→AMX→FP32 Q_S；每个forward、每层重新构造Q。
热边从Q读取FP32 F维并归约。冷边从BF16 H读取D维、decode、FP32归约，tile末尾hi/lo
转换后AMX投影，最后热/冷FP32结果合并并scatter。冷分支保留DegreeSort/TR16/R64/S64
及局部SpMM→GeMM消费，没有完整全局AH。

选源依据CSR中source被消费的次数，固定选N/64、N/16、N/4，零频源不选。显式none/all
对照、全节点project_all、仅实际被用源project_used、预先定义的project_used-L1/FULL-L2
对照都保留。未按逐图结果调比例。all/hot全覆盖情况下会退化成普通投影优先，不能把这个
强对照称为新的TFS贡献。逐层混合只在第二层保留FULL，不是两层都使用选择性TFS。

热点Q为FP32：冷边请求2D bytes，热边4F bytes。128→32时热边从256B变为128B；
32→32时从64B变为128B；128→128时从256B变为512B。两层的trade-off不同。
只有整个16行tile没有冷边才跳过冷投影，covered rows不能直接换成节省的AMX tile数。

### 真实图，两层128→32→32

单位ms；各固定比例全部列出，project_all和逐层混合是强对照。
''']
methods = ['fused_full_accurate','top_1_64','top_1_16','top_1_4','project_all','project_used_L1_full_L2']
for shape, title in [('128_32_32',''),('128_128_128','### 真实图，两层128→128→128')]:
    if title:text.append('\n'+title+'\n')
    text.append(table(['图','FULL','1/64','1/16','1/4','全局project_all','project_used-L1/FULL-L2'],[
        [g]+[f'{t4[g,shape,m]:.3f}' for m in methods] for g in graphs]))
text += ['''
压缩配置中，1/64比例0/5胜出、1/16和1/4各1/5；固定比例几何平均依次0.8809、0.8889、
0.9736×。Myciel 1/4比FULL快1.3485×，却不及普通逐层混合1.9474×。Products/Reddit的
选择性比例都较慢，而project_all分别1.4531/1.4592×。不能把这些普通全局策略收益归给
选择性物化。等宽配置中，RoadNet 1/16只有1.45%小收益，其余四图所有部分比例都慢。

相对新harness内FULL，压缩mixed强对照五图几何平均1.2850×，equal mixed为0.7579×。
这只能说明按层shape考虑计算顺序有必要，还不是新的自适应方法、论文复现速度比或新颖性证明。

### 两层stage拆分：为什么第一层收益会被第二层抵消

以下是另三次stage测量的各子阶段中位数，不能和主E2E五样本混用。单位ms。
Bypass阶段实测的约1微秒时钟开销写为bypass；各子项中位数不必相加等于总中位数。
''']
stage = {(r['graph'],r['shape'],r['method']):r for r in rows('p4','stage_summary.csv')}
stage_methods=['fused_full_accurate','top_1_4','project_all','project_used_L1_full_L2']
stage_table=[]
for g in graphs:
    for m in stage_methods:
        r=stage[g,'128_32_32',m]
        a=[g,m]
        for field in ['cache1_ms','reduce_project1_ms','relu_ms','interlayer_ms','cache2_ms','reduce_project2_ms','total_ms']:
            bypass=field=='cache1_ms' and r['cache1_bypass']=='1' or field=='cache2_ms' and r['cache2_bypass']=='1'
            a.append('bypass' if bypass else f'{float(r[field]):.3f}')
        stage_table.append(a)
text.append(table(['图','方法','Q1重建','归约/投影L1','ReLU','层间转换','Q2重建','归约/投影L2','stage total'],stage_table))
text += ['''
Products 1/4第一层归约/投影146.94→126.39ms，但第二层41.47→71.82ms；Myciel第一层
544.96→322.19ms，第二层111.87→167.96ms。与L1缓存短F、L2读取更宽FP32 Q的工作量
方向一致，但branch/routing、缓存布局、merge也同时存在，不能把差值全归因于带宽。

### 逻辑请求与实际投影量

下表固定1/4，不按图挑最好。覆盖比例为热边/E；projection ratio为实际TDP数量/FULL。
稀疏请求比统计cold H和hot Q逻辑请求，不包括cache构造、CSR、状态与输出，也不是DRAM流量。
''']
work4={(r['graph'],r['shape'],r['method'],r['layer']):r for r in rows('p4','work_plan_summary.csv')}
data=[]
for g in graphs:
    for li in ['1','2']:
        r=work4[g,'128_32_32','top_1_4',li];b=work4[g,'128_32_32','fused_full_accurate',li]
        data.append([g,li,f"{int(r['hot'])/int(r['edges']):.3f}",r['selected_sources'],r['covered_nonempty_rows'],r['cold_tiles'],r['cache_tiles'],
                     f"{float(r['logical_sparse_bytes_over_FULL']):.3f}",f"{int(r['tdp'])/int(b['tdp']):.3f}"])
text.append(table(['图','层','热边覆盖','源s','全覆盖行k','冷tile','源cache tile','稀疏逻辑bytes/FULL','TDP/FULL'],data))
text += ['''
Myciel 1/4热边覆盖约72.5%，L1逻辑请求为FULL的0.638，L2却为1.725。其实际TDP仅减少
约6.3%；冷tile19951加新增cache tile6144，不能将82112个全覆盖行都视为省掉的投影。
Products全覆盖行160621，但冷tile仍为150039，与FULL相同：tile内其他行有冷边，新source
projection是额外工作。这说明source频率或行级k本身不足以决定硬件收益。

### 预处理成本

计数、rank排序、map构造及拓扑审计在prepared之外另计。candidate setup_cost_estimate
把共同count/rank整个pass计入每个候选，含该计划审计；不是直接独立候选计时，也不是严格
硬件上界。静态图可摊销拓扑选择，Q则每层每次forward重建，不能跨输入复用。
''']
data=[]
for g in graphs:
    r=work4[g,'128_32_32','top_1_4','1']
    full=t4[g,'128_32_32','fused_full_accurate'];candidate=t4[g,'128_32_32','top_1_4'];cost=float(r['setup_cost_estimate_ms'])
    data.append([g,f'{cost:.3f}',f'{candidate:.3f}',f'{cost+candidate:.3f}',f'{full:.3f}', '无性能收益' if candidate>=full else '约'+str(__import__('math').ceil(cost/(full-candidate)))])
text.append(table(['图','1/4 setup估算ms','prepared ms','一次合计估算ms','FULL prepared ms','估算摊销forward数'],data))
text += ['''
Myciel的约540ms setup超过单次171ms收益，至少约四个forward才可能摊销相对FULL；这个
比较还未计FULL本来拥有的图/调度准备。global或used投影优先已有相关工作，本轮不主张首次。

## 5. 受控source-sharing图与噪声

两个有向双向二部关系算子均N65536、E4194304、每行degree64、无自环，source编号固定
shuffle。hot每侧只用512个source，spread每侧用32768个。两者degree和E相同，source
共享改变；它们不是训练模型，也不承诺对称归一化GCN。
''']
data=[]
for g in ['bipartite_hot','bipartite_spread']:
    for shape in ['128_32_32','128_128_128']:
        data.append([g,shape]+[f'{t4[g,shape,m]:.3f}' for m in ['fused_full_accurate','top_1_64','project_all','project_used','project_used_L1_full_L2']])
text.append(table(['图','shape','FULL','1/64热点','project_all','project_used','mixed'],data))
text += ['''
hot的1/64选择1024源、覆盖所有65536目标行，cold tile为0；spread同样选择1024源却仅
覆盖1/64边、没有全覆盖目标行。压缩hot可获益，压缩spread部分缓存较慢，说明相同平均
degree下结果也会改变。等宽hot即便跳过所有冷投影仍较慢，不能只用k>s的计算量判据预测时间。

短受控图有显著共享节点/顺序噪声：hot压缩FULL CV25.6%、mixed CV29.9%，spread压缩
FULL CV12.9%、mixed CV11.2%，完整min/max保留。受控图的大小收益不能称稳定规律；
不删除outlier，也未为了得到更好比值重跑挑样。本轮真实图主表所有方法CV≤5%。

## 6. Profiling、PMU与能支持的机制解释

P4两层各24个sampled阶段：CSR/index/membership、prefetch、BF16 H load/decode、cold
FP32 reduce、FP32 Q load/hot reduce、state load/store、partial hi/lo、AMX A/B load、compute、
C store、cache gather/store、merge、scatter、AMX setup/release。thread_imbalance_summary
保留每阶段/层的活跃时间和处理量。插桩会制造停顿及长尾，不能将它当生产kernel关键路径。

以下PMU为单独三次完整forward的中位数，cycles/instructions/cache misses/AMX_BUSY均为
线程累加；IPC是每个region指令/周期比值的中位数。计数与主时间不是同一批样本。
''']
pmu={(r['graph'],r['method']):r for r in rows('p4','pmu_summary.csv') if r['mode']=='focus'}
data=[]
for g in ['ogbn-products','mycielskian19']:
    for m in stage_methods:
        r=pmu[g,'shape128_32_'+m]
        data.append([g,m,f"{float(r['cycles_scaled'])/1e9:.3f}",f"{float(r['instructions_scaled'])/1e9:.3f}",
                     f"{float(r['median_per_region_IPC']):.3f}",f"{float(r['cache_misses_scaled'])/1e6:.1f}",f"{float(r['amx_busy_scaled'])/1e6:.2f}"])
text.append(table(['图','方法','cycles G','instructions G','IPC','generic misses M','AMX busy cycles M'],data))
text += ['''
Products 1/4相对FULL的instructions和generic misses均增加；Myciel 1/4 instructions增加
约35%，generic misses减少约17%，E2E仍改善。不能只用指令数、miss数或TDP数单独预测
速度。mixed减少宽第一层归约且保留窄第二层FULL，与其更低逻辑请求及计数方向一致；
这提供辅助证据，不构成排除所有其他因素的因果实验。

没有采集DRAM带宽、L1/L2/LLC分级miss、TLB、reference cycles、stall/backend分类，不能
写成memory-bound已证明。AMX_BUSY是speculative arithmetic busy cycles，不是退役TDP
数量，也不是AMX的wall时间。所有细分sampled/thread时间不能相加冒充E2E百分比。

## 7. 未隔离的因素和逐图结论

- Products：部分热点的第二层变慢，冷tile没有减少。全局投影对照在压缩配置更好；等宽FULL更好。
- Reddit：所有部分比例均慢；压缩project_all/mixed更好；等宽热点读FP32 Q显著退化。
- Myciel：压缩1/4有收益，但mixed更强；等宽部分比例都慢。投影数减少远小于边覆盖率。
- RoadNet：压缩FULL最好，等宽1/16只有小收益；稀疏热环收益有限而setup仍较大。
- wiki-Talk：只有约6.16%的destination非空，却约98.95%的source被用；used cache很大。
  不能按非空destination数量替代source投影成本。压缩mixed只约3%收益，需更稳定复测。

当前频率排序同时决定选源集合和compact Q布局；project_used需map依赖读取，而project_all
用原source ID直接索引。尚未做“同源集合、ID顺序Q布局”的消融，不能把used较慢唯一归为
选择性物化本身。routing/branch、mixed partial状态、merge、输出访存、first-touch/runtime
和共享资源也未逐项隔离。原生ReLU/转换在P4独立harness的绝对时间与P3不同，不能复用
P3对应阶段作为P4分母或把跨harness差异归给AMX kernel。这些边界不影响本表同进程
方法比较，但限制跨实验外推及单一瓶颈归因。

## 8. 当前方案验证到哪里，下一步做什么

综合文档P0已有全图集及机制消融，feature precision也已有17图2×2；P1已有17图与受控
结构验证；P2只做了同工作交错micro，当前实现没有重稀疏负载收益；P3-D1被拒，D2/D3
本轮广泛经验通过但无FULL性能点；P4本轮两shape/五真实图及受控关系算子通过，选择性
split未战胜所有强对照。整份方案的主候选已获得首轮硬件证据，**不是期刊级完整验证**。

当前更有证据的主线仍是P0访问/投影边界解耦和P1投影预算下分组。后续优先为P1建立
固定、可预测的适用条件，在保留图上验证选择与setup成本；再做同源集合Q布局消融，判断
P4的局部计划是否存在全局/逐层计划覆盖不了的区域。D2/D3与当前交错版不继续铺全图性能。
一般边权、更多形状、真实推理checkpoint、正式独享复测、相关工作排重、误差约束模型及
固定策略的留出验证仍未完成。不得把常规全局重排、缓存或双缓冲单独作为期刊创新。

## 9. 复核入口

结果目录 `GCN-extra/results/journal-p3-p4-20261005/`：

- `p3/p4/source_snapshot/`、`manifest.json`、`source.sha256`、`build/`：实际编译源码与命令，不含benchmark executable；哈希清单中的Python运行缓存也保留。
- `*_time.csv`、`timing_summary.csv`、`timing_anomalies.csv`：全部原始主样本和min/max/CV。
- `*_stage.csv`、`stage_summary.csv`：独立stage原样本与中位数。
- `*_check.csv`、`*_accept.csv`、`*_profile_gate.csv`、P4 `sel_equiv.csv`：全部检查边界及退化一致性。
- `*_detail.csv`、`sel_thread.csv`、`thread_imbalance_summary.csv`：子阶段和线程诊断。
- `method_pmu.csv`、`method_pmu_thread.csv`、`pmu_summary.csv`：独立完整region PMU及每TID。
- `p3/projection_comparison.csv`、`p4/work_plan_summary.csv`：实际工作量、源集合、覆盖、bytes、cache与TDP。
- `RAW_LOGS.tar.gz`、`slurm_accounting.txt`、`summary.json`：逐图stdout、hash、affinity、Slurm与完成状态。
- `artifact_hashes.json`：reconcile原始inventory；`FINAL_ARTIFACT_HASHES.json`：发布整理后inventory。
- `superseded_smoke/`：旧smoke及freeze，排除主表；`ANALYSIS.json`：完整核验统计与边界最大误差。

运行 `analyze_followup.py <结果目录>` 可重新核验并生成派生表；`write_interpretation.py
<结果目录> <本文路径>` 重建说明。两者不修改kernel、计时样本或frozen源码。
''']
doc.write_text('\n\n'.join(text).rstrip()+'\n',encoding='utf-8')
print('WROTE',doc)
