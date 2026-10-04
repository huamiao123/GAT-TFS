#!/usr/bin/env python3
"""Standalone figures for the complete controlled comparison, no selected losses."""
import csv,json,pathlib
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
root=pathlib.Path(__file__).resolve().parents[1]
run=root/'runs/paper-method-reconciled-20261004'
rows=list(csv.DictReader((run/'all_summary.csv').open()))
graphs=sorted(list(csv.DictReader((run/'per_graph_e2e.csv').open())),key=lambda x:float(x['avg_degree']),reverse=True)
assert len(graphs)==17
methods=[f'b{b}_{p}' for b in ['2','4','8','16','32','64','full'] for p in ['fast','accurate']]
lookup={(x['graph'],x['method']):float(x['speedup_median_vs_tfs']) for x in rows if x['kind']=='e2e'}
values=np.array([[lookup[g['graph'],m] for g in graphs] for m in methods])
out=root/'docs/figures/paper_methods_20261004';out.mkdir(parents=True,exist_ok=True)
plt.rcParams.update({'font.size':9,'pdf.fonttype':42})
fig,ax=plt.subplots(figsize=(15,8))
limit=max(1.0,float(np.max(np.abs(np.log2(values)))))
im=ax.imshow(np.log2(values),cmap='RdBu_r',vmin=-limit,vmax=limit,aspect='auto')
ax.set_xticks(range(17),[x['graph'] for x in graphs],rotation=55,ha='right')
ax.set_yticks(range(14),[m.replace('bfull','FULL').replace('_',' ') for m in methods])
for i in range(14):
 for j in range(17):ax.text(j,i,f'{values[i,j]:.2f}',ha='center',va='center',fontsize=7,color='white' if abs(np.log2(values[i,j]))>limit*.6 else 'black')
ax.set_title('Interleaved median two-layer speedup vs original paper TFS v3\nAll 17 eligible graphs; same-process controls; BF16 final output')
ax.set_xlabel('Graphs ordered by average degree (highest to lowest)')
c=fig.colorbar(im,ax=ax,pad=.02);c.set_label('log2(speedup): positive = faster, negative = slower')
fig.tight_layout();fig.savefig(out/'all_block_sizes.png',dpi=180);fig.savefig(out/'all_block_sizes.pdf');plt.close(fig)
fig,ax=plt.subplots(figsize=(10,6))
for method,marker,color,label in [('bfull_fast','o','#3268aa','FULL FAST'),('bfull_accurate','s','#db8635','FULL ACCURATE'),('b64_fast','^','#438550','B64 FAST')]:
 ax.scatter([float(g['avg_degree']) for g in graphs],[lookup[g['graph'],method] for g in graphs],label=label,marker=marker,color=color,s=42,alpha=.8)
for g in graphs:
 if g['graph'] in ['wiki-Talk','reddit','email-Enron','indochina-2004','com-Youtube','ogbn-products','mycielskian19','roadNet-CA']:
  right=g['graph']=='mycielskian19'
  ax.annotate(g['graph'],(float(g['avg_degree']),lookup[g['graph'],'bfull_fast']),xytext=(-5 if right else 4,6),ha='right' if right else 'left',textcoords='offset points',fontsize=7)
ax.axhline(1,color='gray',lw=1,ls='--');ax.set_xscale('log');ax.set_xlabel('Average degree');ax.set_ylabel('Median E2E speedup vs original paper TFS')
ax.set_title('Interleaved execution: degree-dependent crossover, all 17 real graphs');ax.legend();ax.grid(alpha=.18)
fig.tight_layout();fig.savefig(out/'degree_crossover.png',dpi=180);fig.savefig(out/'degree_crossover.pdf');plt.close(fig)
print('PAPER_METHOD_FIGURES',out)
cache=root/'runs/paper-cache-reconciled-20261004'
if cache.exists():
 comparison=list(csv.DictReader((cache/'protocol_comparison.csv').open()))
 fig,axes=plt.subplots(1,2,figsize=(13,5),sharey=True)
 y=np.arange(len(comparison));labels=[x['method'].replace('bfull','FULL').replace('_',' ') for x in comparison]
 for ax,base,title in zip(axes,['tfs','mkl'],['Original paper TFS v3','Unchanged source MKL FP32']):
  for offset,key,color,label in [(-.18,'interleaved','#999999','Interleaved'),(.18,'consecutive','#3268aa','Consecutive after warmup')]:
   v=[float(x[f'{key}_gmean_vs_{base}']) for x in comparison]
   ax.barh(y+offset,v,height=.34,color=color,label=label)
   for k,value in enumerate(v):ax.text(value+.02,k+offset,f'{value:.3f}',va='center',fontsize=8)
  ax.set_yticks(y,labels);ax.axvline(1,color='black',ls='--',lw=1)
  ax.set_xlim(0,max(float(x[f'{p}_gmean_vs_{base}']) for x in comparison for p in ['interleaved','consecutive'])*1.16)
  ax.set_xlabel('All-17 geometric mean speedup');ax.set_title(title);ax.grid(axis='x',alpha=.15)
 axes[0].invert_yaxis();handles,legend_labels=axes[0].get_legend_handles_labels();fig.legend(handles,legend_labels,loc='lower center',ncol=2,frameon=False,fontsize=9)
 fig.suptitle('Execution-order control: fixed methods, all eligible real graphs\nEach speedup uses its own same-process baseline; no losing graph removed',fontsize=11)
 fig.tight_layout(rect=[0,.08,1,1]);fig.savefig(out/'execution_order_control.png',dpi=180);fig.savefig(out/'execution_order_control.pdf');plt.close(fig)
