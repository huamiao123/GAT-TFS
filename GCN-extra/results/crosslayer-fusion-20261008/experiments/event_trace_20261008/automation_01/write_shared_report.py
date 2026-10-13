from pathlib import Path
import csv, json, hashlib, datetime

p=Path(__file__).resolve().parent
d=json.loads((p/'shared_review_02.json').read_text(encoding='utf-8'))
assert d['partial_analysis_pass'] and not d['final_analysis_complete']
assert d['logical_expansion_decision']['already_ruled_out_by_completed_shared_cohort']
def rows(name):return list(csv.DictReader((p/name).open(encoding='utf-8',newline='')))
s=rows('shared_summary_02.csv'); st=rows('shared_structure_02.csv'); setup=rows('shared_setup_02.csv')
names=['pack_epi_degree','pack_epi_qguard','dyn_i','dyn_s','exact_s','trace_i','trace_s_cached','trace_s_rep']
graphs=['mycielskian19','reddit','roadNet-CA','amazon0601']
def ms(g,m,protocol='alternating'):
    v=[x for x in s if x['graph']==g and x['method']==m and x['protocol']==protocol]
    assert len(v)==1
    return float(v[0]['median_ms'])
def tab(table):return '\n'.join(' | '.join(map(str,r)) for r in table)
perf=tab([['图']+names]+[[g]+[f'{ms(g,m):.3f}' for m in names] for g in graphs])
gates=tab([['基线','采样','几何平均速度比','胜图数/4']]+[
    [x['baseline'],x['protocol'],f"{x['geometric_mean']:.6f}",f"{x['wins']}/4"] for x in d['shared_expansion_cohorts']])
syn=tab([['合成图','trace_i','trace_s_cached','trace_s_rep','PACK-degree','I/cached速度比','PACK/rep速度比']]+[
    [g]+[f'{ms(g,m):.3f}' for m in ['trace_i','trace_s_cached','trace_s_rep','pack_epi_degree']]+[f"{ms(g,'trace_i')/ms(g,'trace_s_cached'):.4f}",f"{ms(g,'pack_epi_degree')/ms(g,'trace_s_rep'):.4f}"]
    for g in ['regular128_reuse1','regular128_reuse4','regular128_reuse16']])
opp=[]; costs=[]
for g in graphs:
    z=next(x for x in st if x['graph']==g)
    E=int(z['source_reads']);a=int(z['shared_s64']);b=int(z['shared_full'])
    opp.append([g,E,a,b,f'{(b-a)/(E-a)*100:.4f}%',f'{int(z["plan_bytes"])/(1024**2):.3f}'])
    z=next(x for x in setup if x['graph']==g and x['phase']=='trace_plan_including_validator')
    costs.append([g,f'{float(z["build_ms"]):.3f}',f'{float(z["validate_ms"]):.3f}',f'{float(z["total_ms"]):.3f}'])
oppt=tab([['图','E','S64共享事件','full共享事件','相对当前decode再减少','当前S64 trace持久MiB']]+opp)
costt=tab([['图','trace构造ms','内置校验ms','两者合计ms']]+costs)
stamp=datetime.datetime.now().astimezone().isoformat()
text=f'''TFS 期刊扩展：消费事件计划、分组边界与下一步研究说明
日期：2026-10-08；生成时间：{stamp}
状态：共享阶段完成，独享阶段已提交、仍待终审。本文件不是完整独享结论。

一、本轮结论

把现有两行配对的共享消费顺序离线编码后，确实省掉了在线查找共享的比较，也在机器码中实现了共享事件只解码一次。但本轮固定候选 trace_s_rep 在四个诊断图上没有超过现有强 PACK 基线。按实验前固定的规则，不扩展到 17 图；这个决定不需要等待独享结果改变，因为规则要求所有分区与协议同时通过，共享部分已经失败。

这只否定当前“固定 degree 相邻配对 + 逐事件两位编码 + 当前执行器”的扩大验证价值，不能外推成所有离线计划、所有分组或 AVX–AMX 合作都无效。已有访问/投影粒度解耦与 PACK/EPI 状态管理仍是项目基础，本轮没有重新测量它们相对原 TFS 的历史 17 图收益，不能混用分母。

更有价值的新认识是：需要同时分清共享机会、执行共享的成本，以及跨访问切片保存状态的成本。当前固定配对即使取消 S64 边界，两个大图额外减少的解码也只有约 0.85% 和 1.78%；只在这个配对上继续增加复杂调度，潜在空间有限。

二、实验到底改变了什么

主配置始终为 128→128→128、32 个物理核心、单 socket 绑定、FAST BF16 路径、单位边权、原随机种子。没有 NUMA interleave，没有改变 feature first-touch，没有推送 GitHub。独享作业使用 intel_expr --exclusive，共享使用 intel；本轮峰值申请 6 个节点，包含 pending，低于用户 10 个节点上限。

访问窗口 S=64，投影归约范围 M=FULL，PF1 预取不变。八个方法是：
1. pack_epi_degree：已有 PACK/EPI 加 degree schedule。
2. pack_epi_qguard：已有 PACK/EPI 加冻结 qguard schedule；qguard 由结构探针选择，不使用当前计时作选择。
3. dyn_i：在线双头比较，每行分别解码。
4. dyn_s：相同在线事件，共享事件只解码一次。
5. exact_s：已有整窗口完全相同的快速路径。
6. trace_i：离线事件 + cached heads，仍分别解码。
7. trace_s_cached：相同离线事件 + cached heads，共享解码。
8. trace_s_rep：共享解码，改为每事件读取一个代表当前邻居索引。

后三者原样重放旧 dyn 路径的事件，不改变配对，不求新的最优序列。每 record 是一个 0..128 的事件数头部，随后每事件两位 01/10/11；每个 record 单独向字节取整，耗尽的 pair 保留零计数头，不存源节点 ID。一个非空 tile 的 trace 从一个 offset 开始顺序读取。

各方法每层均有 128E 个特征加法，相同 AMX 投影次数、相同 S64 partial 读写预算、相同量化位置和逐行加法次序。本轮测的是 AVX 端共享与控制的净成本，没有改变 AVX–AMX 的投影分工，也没有实现新的跨 tile 流水。

用户此前所说“邻居分组大，AVX 工作量大”需要区分单组与全图：单组聚合变长，但仅改变归约范围并不增加全图 128E 个加法。AMX 投影次数会减少，partial 的寿命和流量会改变。跨行共享读取/解码属于另外一个维度；即使共享解码成功，两行各自的加法仍须执行。

三、正确性与汇编证据

13 个 smoke fixture 全部通过，覆盖空行、尾部、重复和无序邻居；另有独立 Python 对真实小图 CSR 和 trace 字节逐事件重放。性能阶段的 7 个共享图全部通过原数值门槛、四个输出阶段的逐位一致、实际计时对象验证、生产/计数路径一致、独立工作公式与 immutable storage 检查。

共享性能数据共 840 个计时样本、112 个方法/协议汇总、168 个 PMU 区域、5376 个工作线程 PMU 记录。全部 PMU 区域可用，time_enabled/time_running 比值为 1；所有该阶段计时组 max/min <1.10。这里的范围描述不是置信区间，共享节点结果也不能代替独享复核。

47 项汇编审查通过。take3 共享事件中，trace_i 保留两组共 16 次向量解码与 16 次向量加法；两个共享 trace 保留 8 次解码、16 次加法。编译器没有把独立对照偷偷合并。cached 两行当前头更新加载仍在，rep 确实选择一个代表索引加载。旧 dyn_i/dyn_s 与前轮规范化机器码一致。所审生产函数没有直接基于 rsp/rbp 的向量 spill，但存在标量/控制栈访问，不能笼统写“无 spill”。

当前索引读取与 PF1 的下一索引预取请求必须分开：rep 只减少前者，后者不变。源码 decode/bytes 计数不等于退休 load 或 DRAM 流量；通用 cache 事件和 AMX busy 也不能独自证明瓶颈成因或执行重叠。

四、四个诊断图的性能

固定候选为 trace_s_rep。速度比 = 基线时间 / 候选时间，>1 才表示候选快。
{gates}

门槛在实验前固定为：两基线 × 两分区 × 两采样协议的每个 cohort 都要求 GM≥1.03 且至少胜 3/4 图，所有正确性通过。不用表现更好的另一个 trace 变体替换候选，不事后挑图。

下面均为共享节点、10 次正反交替计时的中位数，单位 ms，包含两层计算与激活/转换：
{perf}

Amazon 在连续计时对 degree 只有约 0.34% 的优势，交替协议则慢，不能称为稳定胜出。Reddit 的离线 cached 版本比 dyn_s 略快，但仍远慢于 qguard。对现有最好路径的净收益，才是扩大实验的依据。

Mycielskian19 属于构造图；这里“诊断图”沿用原 benchmark 子集，不把它们统称为真实应用图。

五、合成图告诉我们什么

下表仍为共享节点交替中位数，ms；最后两列是对应基线/候选：
{syn}

在共享机会密集的合成图上，共享解码确实能减少时间，说明实现不是完全没有作用。但 reuse4、reuse16 即使具有相同 pair 共享比例，工作集与复用结构也不同；不能单凭共享比例决定收益。reuse1 没有共享，不能把任何差异解释成解码共享。

因此，“合成数据能赚到，实际诊断图赚不到”需要通过结构覆盖和执行成本解释，不能用更多合成图替代实际图的净收益。

六、取消 S64 边界究竟还有多少空间

对相同 degree pair，另做只读 full-row greedy 诊断，不访问特征、不执行新内核，不计入性能或必要预处理。四个诊断图的 CSR 均逐行有序。
{oppt}

表中额外 decode 降幅 = (S_full−S64)/(E−S64)。Reddit 的共享事件相对增长约 50%，但总 decode 只再减少 0.8543%；Myc 的共享事件相对增长约 17%，总 decode 只再减少 1.7842%。Road/Amazon 没有新增共享。

full greedy 的匹配最优性仅限固定两行、每事件每行最多消费一条边、逐行有序这一模型。允许同源 run 在同一行连续复用已解码值时，decode 下界还能变化；改变配对或同时活跃多行也可能增加机会。无序 CSR 下 full greedy 甚至可能比 S64 共享更少，因此不能把 full−S64 一概称为“可恢复损失”。小例：A 为 65 个 0，B 为 63 个 0 接 1、0，S64 可共享 64 次，full greedy 只共享 63 次。

本诊断只能说明固定配对的机会，不是所有分组的全局上限，更不是整个内核速度上限。保存/恢复顺序、预取和缓存行为改变仍可能影响性能，需另设实验验证。

七、元数据和预处理不能忽略

{costt}

以上是实际 trace 构造与内置完整校验，单位 ms；旧 pair plan、全套五 schedule 构造以及独立运行期复核分别记录，没有将它们混成一个孤立候选成本。机会诊断是额外研究工具，不能算进必要 setup 再用来夸大摊销点。

Myc 新 trace 持久向量数据加 offset 为 213,291,475 bytes，约 203.41 MiB；另外还有 TracePlan 对象、旧 plan、图与特征内存。两位编码不等于零成本。所有方法计时期间都保留同一份 trace 和 pair plan，避免方法切换分配差异；这个暖态增量对照不代表 standalone PACK 的实际内存占用。

若候选比基线慢，则没有通过重复推理摊销变快的交点。若某图较快，理想摊销次数应以新增必要预处理时间除以每次净节省计算，并单独说明图是否静态、plan 是否跨层/多次推理复用。不能将 inspector 删除后仍用校验已通过的说法接受未经检查的部署路径。

八、为什么 K-event 轮转需要谨慎

如果改成一对行处理 K 个消费事件就切换，必须跨 slice 保存每个未完成行的 FP32[128]、各自 CSR 游标、初始化/完成状态，以及 trace 的字节/bit 位置。cached 模式还要保存当前 head；rep 可省 head 状态，但省不了游标。

为了逐位保序，应恢复同一累计值继续原行加法，不能先算每块局部和再合并。16 行 FP32 partial 为 8 KiB；按每行 8 个 zmm，16 行需 128 个寄存器，不能全部驻留 32 个 zmm。两行未完成时，一次切换存回和下次重读共 2048 bytes。

具体反例：两行 degree128、来源有序交错而不共享，S64 只跨一次边界，状态流量为 2048 bytes；K64 events 可能跨三次，流量为 6144 bytes。完全相同的两行则不会有这个三倍成本。K 个事件对应 K..2K 次行消费，不能把 K64 和 S64 当等价控制。

M=FULL 时仍需等当前 16 行归约完再做 AMX 投影；slice 轮转本身不产生 AVX–AMX 流水。固定 full 事件序列之后调整 K 只影响状态、访问组织和局部性，不再增加该序列的共享次数。

九、我建议如何继续寻找分组与创新点

优先级一：把分组目标从“共享率最高”改成“在状态容量内带来算子链净收益”。分别计算：能省多少源特征解码/索引，增加多少事件解析，访问复用距离怎样变化，partial 有多少跨 slice 保存，以及最终 AMX 消费是否受影响。度数、共享率、cache 局部性是不同指标。本轮 qguard 仍是必须超过的基线。

优先级二：在写复杂多行内核前，先估计改变配对、扩大活跃行集合的可利用空间。对有序邻居可以分析同一 tile 内的源交集/并集，但必须把固定 register 容量带进来：128 维时每行 8 个向量累加器，4 行已经占满 32 个 zmm，尚未算解码/控制临时值。若用 feature 分块降低寄存器压力，则会重复遍历 CSR/trace，且可能影响 AMX 输入准备与输出驻留。这是需要共同优化的成本，不是“组越大越好”。这些是待验证方向，本轮没有实现多行或新 feature tiling。

优先级三：先判断是否需要放宽严格数值契约，再考虑共享公共部分和。当前共享 decode 保持 128E 加法；若想实质减少聚合加法，需要更改归约结构或复用公共聚合结果，并验证真实归一化 GCN 的任务精度、带权语义和预处理摊销。保序是当前因果实验约束，不是未来论文永远不能改变的教条。

关于创新边界：离线 inspector–executor、寄存器共享、公共聚合、矩阵/向量协作已有前人。上一份桌面报告已经核对 SpRegTiling、MaSpMM、TileFusion、HAG 等近邻；本轮没有新增全文检索，不重新声称排重已完成。值得争取的是受依赖/精度约束、AVX 活跃状态与 AMX 消费共同决定的可执行分组与数据流，给出可解释、低开销且能预测失败的选择方法。不能仅因工具有来源就否定课题，也不能把事件换成两位编码包装成主要创新。

仍建议以已有工作高效的融合数据流为主线，用实际净收益决定是否纳入分组机制。本轮是缩小研究范围的负面结果，不支持把 trace 写成新的期刊加速贡献。

十、证据、待完成事项与交付

冻结构建和原始日志：
/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01
共享阶段独立分析：
/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/event_trace_20261008/analysis_inputs_01/shared_review_02.json
汇编审计：experiments/event_trace_20261008/codegen_audit_01/audit.json
运行与 K-event 成本复核：experiments/event_trace_20261008/runtime_review_01/

已完成：smoke 10896223；共享 synthetic 10896231、Myc 10896233、other 10896234，均 COMPLETED 0:0。
独享已提交：10896227、10896229、10896230；本文件生成时尚未纳入终审。
完整目标为 7 个作业、27 个图包、1680 计时、336 PMU 区域。后台交付脚本只跟踪这些既有作业；全部完成且完整审计通过，才另写桌面“完整审计”TXT。若作业、传输或审计失败，记录明确失败状态，不把本文件改成成功。

未验证：真实模型任务精度、带权/归一化实际推理、更多形状、未见图上的分组选择、真正 AVX–AMX 执行重叠。本轮保持 128 维，不用 128→32→32 替代原论文主对照。

本地取数文件 SHA-256：
'''
for name in ['shared_review_02.json','shared_summary_02.csv','shared_ratios_02.csv','shared_pmu_02.csv','shared_setup_02.csv','shared_structure_02.csv']:
    text+=name+' '+hashlib.sha256((p/name).read_bytes()).hexdigest()+'\n'
out=Path.home()/'Desktop'/'TFS_消费事件计划与分组边界_共享阶段_20261008.txt'
assert not out.exists()
out.write_text(text,encoding='utf-8')
print(json.dumps(dict(path=str(out),bytes=out.stat().st_size,sha256=hashlib.sha256(out.read_bytes()).hexdigest()),ensure_ascii=False))
