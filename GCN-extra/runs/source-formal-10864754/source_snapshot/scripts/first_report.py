#!/usr/bin/env python3
"""Build the first-round report from raw CSV, retaining both frozen runs."""
import csv, json, pathlib, statistics, sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

project=pathlib.Path(__file__).resolve().parents[1]
first=pathlib.Path(sys.argv[1]);control=pathlib.Path(sys.argv[2]);docs=project/'docs';figs=docs/'figures';figs.mkdir(exist_ok=True)
subs=['reddit','soc-Pokec','regular-q16','regular-q256']
methods=['mkl_fp32','original','original_nozero','shared_bfull_fast_nozero','shared_bfull_accurate_nozero','shared_b32_fast_nozero','replay_bfull_fast_nozero']
labels={'mkl_fp32':'MKL FP32','original':'源码完整原版','original_nozero':'原版，仅移除清零','shared_bfull_fast_nozero':'局部 Full Fast，移除清零','shared_bfull_accurate_nozero':'局部 Full Accurate，移除清零','shared_b32_fast_nozero':'局部 B32 Fast，移除清零','replay_bfull_fast_nozero':'Full Fast 重读对照，移除清零'}
def read(path):return list(csv.DictReader(path.open()))
def medians(sub):
 groups={}
 for r in read(sub/'timings.csv'):
  if r['kind'] in ('kernel','e2e'):groups.setdefault((r['kind'],r['method']),[]).append(float(r['ms']))
 return {k:statistics.median(v) for k,v in groups.items()}
data={s:medians(control/s) for s in subs};old={s:medians(first/s) for s in subs}
lines=['# GCN-extra 首轮推理实验：局部邻居归约与原 TFS 对照','',
 '首轮实现、正确性检查、两次独占性能实验已完成。完整 25 图、K 敏感性、度分布二维实验和 checkpoint 准确率尚未完成。所有论文级泛化结论仍为 UNVERIFIED。','',
 '## 实验定义与证据','',
 f'- 第一轮 B sweep：`{first.name}`；第二轮清零控制：`{control.name}`。',
 '- 每次只使用一个独占 Xeon Max 9462 节点；实际计算为一个 socket 的 32 个物理核，内存交错于该 socket 的四个 CPU NUMA 域。节点与 CPU mask 见各 run 的 affinity.json、lscpu.txt、benchmark_numa_policy.txt。',
 '- D=F=128，TR=16，R=64；所有路径使用同一原始 CSR、DegreeSort permutation、H/W。真实图全部 CSR values 扫描为 1。已有边和 self-loop 保留，不补边。',
 '- 算子为源码实际实现的 C=(AH)W、单位边权求和；两层为该算子→ReLU→第二层算子。H/W 为固定种子随机输入。这里的两层 E2E 是 prepared sum-aggregation inference，不是已验证任务准确率的归一化 GCN。',
 '- Original、Fast、Accurate 使用相同截断 BF16 H/W；另有 FP32 MKL 和 BF16 输入/权重转回 FP32 的 MKL 参考。Fast 额外量化局部和；Accurate hi+lo 仅改善这一额外量化，不能恢复 H/W 已丢失的精度。',
 '- 一次预热、五次测量，轮换方法顺序；表内为未经分项插桩的中位数。Prepared E2E 包含两个 kernel、ReLU、层间 BF16 转换；初始 H 转换、W packing、DegreeSort、图读取分别在 preprocess.csv，未包含于此 E2E。','',
 '## 图与实际布局','',
 '| 图 | N | E | 平均度 | 最大度/平均度 | 原 TFS η | Full η |','|---|---:|---:|---:|---:|---:|---:|']
for s in subs:
 info=json.loads((control/s/'info.json').read_text());w={r['method']:r for r in read(control/s/'work.csv')};a=w['original'];b=w['shared_bfull_fast_nozero']
 lines.append(f"| {s} | {info['N']:,} | {info['E']:,} | {float(a['avg_degree']):.2f} | {float(a['degree_ratio']):.2f} | {float(a['eta']):.6f} | {float(b['eta']):.6f} |")
lines += ['','## 第二轮公平对照：kernel 与两层 E2E','',
 '所有 Original_nozero 与局部 nozero 方法都移除了冗余全输出清零。相对 Original_nozero 的加速才用于判断归约方法的额外价值。完整源码原版同时保留。','']
metrics=[]
for s in subs:
 lines += ['### '+s,'','| 方法 | kernel ms | 两层 E2E ms | E2E 相对 Original_nozero | E2E 相对 MKL FP32 |','|---|---:|---:|---:|---:|']
 d=data[s];base=d[('e2e','original_nozero')];mkl=d[('e2e','mkl_fp32')]
 for m in methods:
  k=d[('kernel',m)];e=d[('e2e',m)]
  lines.append(f'| {labels[m]} | {k:.3f} | {e:.3f} | {base/e:.3f}× | {mkl/e:.3f}× |')
 metrics.append({'graph':s,'original_kernel_ms':d[('kernel','original')],'original_nozero_kernel_ms':d[('kernel','original_nozero')],'local_full_fast_kernel_ms':d[('kernel','shared_bfull_fast_nozero')],'mkl_fp32_e2e_ms':mkl,'original_nozero_e2e_ms':base,'local_full_fast_e2e_ms':d[('e2e','shared_bfull_fast_nozero')],'local_full_accurate_e2e_ms':d[('e2e','shared_bfull_accurate_nozero')],'full_fast_speedup_corrected_original':base/d[('e2e','shared_bfull_fast_nozero')],'full_fast_speedup_mkl':mkl/d[('e2e','shared_bfull_fast_nozero')]})
 lines.append('')
lines += ['## 正确性与精度','',
 '先做全矩阵数值比较，再进行性能测量。还对小图全行、真实图按低度/高度/固定种子选取的行做 FP64 聚合与投影参考。NaN 预填充确认 nozero 内核真正写完每个输出元素。','',
 '| 图 | 原版去清零与原版 max abs | Full Fast kernel rel L2 | Full Accurate kernel rel L2 | Full Fast E2E rel L2 | Full Accurate E2E rel L2 |','|---|---:|---:|---:|---:|---:|']
failed=0;checks_total=0
for s in subs:
 checks=read(control/s/'correctness.csv');failed+=sum(r['pass']=='0' for r in checks);checks_total+=len(checks)
 def get(m,boundary,ref='mkl_bf16_inputs',key='relative_l2'):
  return float(next(r[key] for r in checks if r['method']==m and r['boundary']==boundary and r['reference']==ref))
 values=[get('original_nozero','nozero_identity','unchanged_original','max_abs'),get('shared_bfull_fast_nozero','kernel_full'),get('shared_bfull_accurate_nozero','kernel_full'),get('shared_bfull_fast_nozero','two_layer_e2e'),get('shared_bfull_accurate_nozero','two_layer_e2e')]
 lines.append('| '+s+' | '+' | '.join(f'{x:.3e}' for x in values)+' |')
lines += ['',f'第二轮正确性记录 {checks_total} 条，失败 {failed} 条。空行、全空图、目标行尾块、130 邻居跨块检查均通过。首个全空图因 oneMKL 零 nnz 接口失败的历史记录保留，适配修复见 handoff。Fast/Accurate 的误差相对于相同 BF16 输入/权重参考；相对 FP32 的完整误差另见 correctness.csv。','',
 '## 已定位的开销与研究判断','',
 '1. 第一轮发现 soc-Pokec 原始串行输出清零约 65.504 ms，而原 kernel 中位数为 110.746 ms；局部路径同样约 65.763 ms 清零。清零掩盖了 sparse 与 AMX 部分的差别。第二轮只去除该冗余操作的 Original_nozero 已通过逐元素完全一致检查。',
 '2. 原 TFS 的 DegreeSort 在 reddit 上已将 η 提高到约 0.997，但逐边投影仍存在：其物理 AMX 模型工作量约 3.768 TFLOP；局部 Full Fast 约 7.634 GFLOP，Accurate 约 15.268 GFLOP。时间不会按工作量同比下降，新的路径增加 FP32 特征归约并受 gather、输出与调度限制。',
 '3. 原 kernel 每个输出 panel 重走邻居。shared 模式只读取/归约一次；replay 对照保留两次读取和归约。二者 AMX 投影工作相同，可观察跨 panel 复用的价值；不能把 shared 的全部收益归结为投影 FLOPs 减少。',
 '4. 两张规则图固定 N=262144，q=16/256，源 ID 生成器、种子和每行邻居前缀规则相同，邻居在一行内不重复。q sweep 仍同时改变 E/数据量，只有两个点，不能据此拟合自适应 B 选择器或宣称完整因果结论。',
 '5. 当前两次实验固定 R=64。尚未复现原论文所有图的速度趋势或重新核对原论文最优 R/绑核/计时环境，完整 S0 仍需 source-faithful 独立 CLI 与 R64/128/256 对照。','',
 '## 分项计时位置与边界','',
 '每张图的 profiles.csv 包含输出清零、行调度、局部 buffer 清零、CSR/prefetch、feature gather/decode、FP32 邻居归约、BF16 转换、tile load、tile compute、tile store、输出 scatter、线程 AMX setup。MKL 另记录真实 SpMM/GeMM wall time；E2E 的 layer1/activation_conversion/layer2 在 timings.csv。',
 'AMX/归约细项采用独立的抽样插桩运行，每 256 个排序后的 destination tile 取样并包含末尾 tile；这些是所选 tile 的累计线程时间，计时器有开销，不能相加当作总 wall latency，也不能直接当作全图百分比。原版插桩输出与未改动内核逐元素一致。物理 AMX FLOPs、逻辑特征 bytes 和 spill/reload bytes 是源码模型，尚无硬件计数器证据。','',
 '## 图表','',
 '![第一轮 B sweep](figures/block_sweep.png)','',
 '![第二轮公平两层比较](figures/e2e_controls.png)','',
 '## 下一步','',
 '- 固定当前正确性与 zero-free 强基线，核对原 CLI 和 R64/128/256、DegreeSort 消融；保留 source faithful 原版作为历史锚点。',
 '- 扩展 q sweep 与固定 E 灵敏度，记录度数分位数、真实源节点复用、工作集和 NUMA；再进行 q×skew 二维实验。',
 '- 增加 reddit/mycielskian19/hollywood/products 的外部验证、K 敏感性和硬件计数器；24/25 图全量应先确认内存与 32-bit CSR 边界。',
 '- 任务准确率仅在具备匹配特征、标签、GCN checkpoint 的数据集上检查。','']
(docs/'FIRST_RESULTS_20261003.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
event={'id':'analysis-20261003-first-round-completed','kind':'milestone_analysis','status':'FIRST_ROUND_COMPLETE','evidence':['runs/'+first.name,'runs/'+control.name,'docs/FIRST_RESULTS_20261003.md'],'numerical_records':checks_total,'failed':failed,'metrics':metrics,'paper_eligible':False,'issues':'Redundant original serial C zeroing diagnosed and controlled; zero-nnz MKL adapter validated. Full generality, checkpoint accuracy, hardware counters and full paper baseline reproduction pending.','next':'Strong source-faithful R/CLI audit followed by q/degree-distribution sensitivity; retain DegreeSort and local fusion'}
(docs/'first_results_event.json').write_text(json.dumps(event,ensure_ascii=False,indent=2),encoding='utf-8')
plt.rcParams.update({'font.size':10,'axes.spines.top':False,'axes.spines.right':False})
fig,axs=plt.subplots(2,2,figsize=(11,7.5),layout='constrained')
for ax,s in zip(axs.flat,subs):
 d=old[s];bs=['2','4','8','16','32','64','full']
 for mode,color in [('fast','#1764a0'),('accurate','#c75728')]:
  ax.plot(range(len(bs)),[d[('kernel','shared_b'+b+'_'+mode)] for b in bs],marker='o',label='Shared '+mode,color=color)
 ax.axhline(d[('kernel','original')],color='#555555',linestyle='--',label='Original')
 ax.axhline(d[('kernel','mkl_fp32')],color='#458a48',linestyle=':',label='MKL FP32')
 ax.set_xticks(range(len(bs)),bs);ax.set_title(s);ax.set_xlabel('Neighbors per reduction block');ax.set_ylabel('Kernel median (ms)');ax.grid(alpha=.2);ax.legend(fontsize=8)
fig.suptitle('First run: block size sweep (all AMX paths include original serial C zeroing)');fig.savefig(figs/'block_sweep.png',dpi=200);fig.savefig(figs/'block_sweep.pdf');plt.close(fig)
fig,axs=plt.subplots(2,2,figsize=(11,7.5),layout='constrained')
bars=['mkl_fp32','original_nozero','shared_bfull_fast_nozero','shared_bfull_accurate_nozero']
for ax,s in zip(axs.flat,subs):
 vals=[data[s][('e2e',m)] for m in bars];ax.bar(range(4),vals,color=['#458a48','#777777','#1764a0','#c75728'])
 ax.set_xticks(range(4),['MKL\nFP32','Original\nno zero','Full\nFast','Full\nAccurate']);ax.set_title(s);ax.set_ylabel('Two-layer median (ms)');ax.grid(axis='y',alpha=.2)
 for i,v in enumerate(vals):ax.text(i,v,f'{v:.1f}',ha='center',va='bottom',fontsize=9)
 ax.set_ylim(0,max(vals)*1.18)
fig.suptitle('Second run: identical zero-free AMX boundaries, 32 physical cores / one socket');fig.savefig(figs/'e2e_controls.png',dpi=200);fig.savefig(figs/'e2e_controls.pdf');plt.close(fig)
print(json.dumps(metrics,indent=2));print(docs/'FIRST_RESULTS_20261003.md')
