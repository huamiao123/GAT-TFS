#!/usr/bin/env python3
"""Standalone scientific plots from the full-coverage comparison CSV."""
import csv
import math
import pathlib
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import TwoSlopeNorm
import numpy as np

csvpath=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2]);out.mkdir(exist_ok=True,parents=True)
with csvpath.open(encoding='utf-8',newline='') as f:rows=list(csv.DictReader(f))
graphs=sorted({r['graph'] for r in rows},key=lambda g:next(float(r['avg_degree']) for r in rows if r['graph']==g),reverse=True)
blocks=['2','4','8','16','32','64','full']
lookup={(r['graph'],r['method']):r for r in rows if r['kind']=='e2e'}
matrices=[]
for mode in ['fast','accurate']:
    matrices.append(np.array([[float(lookup.get((g,f'shared_b{b}_{mode}_nozero'),{}).get('speedup_original_nozero','nan')) for b in blocks] for g in graphs]))
extent=max(1,math.ceil(max(abs(math.log2(v)) for a in matrices for v in a.flatten() if math.isfinite(v))))
fig,axes=plt.subplots(1,2,figsize=(15,max(6,0.34*len(graphs)+2)),sharey=True,layout='constrained')
norm=TwoSlopeNorm(vmin=-extent,vcenter=0,vmax=extent)
for ax,arr,title in zip(axes,matrices,['Fast','Accurate (hi + lo)']):
    im=ax.imshow(np.log2(arr),cmap='RdBu',norm=norm,aspect='auto')
    ax.set_title(title+' — prepared two-layer inference',fontsize=12)
    ax.set_xticks(range(len(blocks)),['B'+b if b!='full' else 'Full' for b in blocks]);ax.tick_params(top=True,labeltop=True,bottom=False,labelbottom=False)
    ax.set_yticks(range(len(graphs)),[g+'  (q='+format(next(float(r['avg_degree']) for r in rows if r['graph']==g),'.1f')+')' for g in graphs],fontsize=9)
    for i in range(len(graphs)):
        for j in range(len(blocks)):
            if not math.isfinite(arr[i,j]):
                ax.text(j,i,'—',ha='center',va='center',fontsize=8);continue
            color='white' if abs(math.log2(arr[i,j]))>extent*0.57 else 'black'
            ax.text(j,i,f'{arr[i,j]:.2f}',ha='center',va='center',fontsize=8,color=color)
axes[0].set_ylabel('Graphs sorted by average degree q = E/N')
ticks=list(range(-extent,extent+1));bar=fig.colorbar(im,ax=axes,shrink=0.8,ticks=ticks)
bar.ax.set_yticklabels([f'{2**t:g}x' for t in ticks]);bar.set_label('Speedup vs Original TFS without redundant output clear')
reps='Median of 10 repetitions (Friendster: 5; —: not measured)' if 'com-Friendster' in graphs else 'Median of 10 repetitions'
fig.suptitle('GCN-extra: fixed D=F=128; DegreeSort + local AMX fusion\n'+reps+'; red < 1x, blue > 1x',fontsize=13)
fig.savefig(out/'graph_suite_heatmap.png',dpi=180);fig.savefig(out/'graph_suite_heatmap.pdf');plt.close(fig)

fig,ax=plt.subplots(figsize=(9,5.8),layout='constrained')
for mode,marker in [('fast','o'),('accurate','s')]:
    for b in ['8','16','64','full']:
        selected=[lookup[(g,f'shared_b{b}_{mode}_nozero')] for g in graphs]
        ax.scatter([float(r['avg_degree']) for r in selected],[float(r['speedup_original_nozero']) for r in selected],
                   marker=marker,s=35,alpha=0.7,label=f'{mode.capitalize()} B{b if b!="full" else "Full"}')
ax.axhline(1,color='black',linestyle='--',linewidth=1)
ax.set_xscale('log');ax.set_xlabel('Average degree E/N (log scale)');ax.set_ylabel('Prepared two-layer speedup vs Original_nozero')
ax.set_title('Average degree and speedup — graph coverage, not a causal fit')
ax.grid(True,alpha=0.2);ax.legend(fontsize=8,ncol=2)
fig.savefig(out/'graph_suite_degree_speedup.png',dpi=180);fig.savefig(out/'graph_suite_degree_speedup.pdf');plt.close(fig)
print(out)
