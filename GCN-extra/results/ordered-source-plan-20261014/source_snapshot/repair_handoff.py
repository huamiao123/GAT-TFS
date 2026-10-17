#!/usr/bin/env python3
import pathlib,json,hashlib
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
p=root/'docs/handoff/CURRENT_HANDOFF.md'
text=p.read_text(encoding='utf-8-sig')
anchor='# GCN-extra 当前交接摘要'
assert anchor in text
old=text[text.index(anchor):]
header='''# 最新实验补充（2026-10-14）

partial bundle 已完成：experiments/partial_bundle_20261013/；冻结构建 runs/partial-bundle-build-20261013-225110/；结果 results/partial-bundle-20261013/GCN_EXTRA_PARTIAL_BUNDLE_RESULTS.md。

保留 DegreeSort/TR16/R64、S64、M64/M256/FULL、原量化边界和 PACK_EPI，仅缓存已量化 partial 减少 C 存取。五小图与五诊断图全部门槛通过，作业10944718/10944719已结束。共享32核单socket、默认NUMA，固定G4 M64 FAST五图GM1.0203、ACCURATE1.0254；FULL单partial控制也变化，不能把全部改善归给C存取。paper_eligible=false。

当前继续 ordered_source_plan_20261014：固定 DegreeSort tile、逐行 CSR 加法和 AMX 投影，用静态短事件计划复用部分邻居的加载/解码；先正确性与结构覆盖，再性能。

当前用户节点上限为5，本轮最多两个单节点作业；下方历史10节点表述不适用于当前任务。

## 以下保留历史摘要

'''
p.write_text(header+old,encoding='utf-8')
assert '最新实验补充' in p.read_text(encoding='utf-8')
e={'id':'partial-bundle-handoff-encoding-repair-20261014','kind':'documentation_encoding_repair','status':'PASS','evidence':str(p),'paper_eligible':False,'issues':'Previous PowerShell stdin replaced new Chinese header with question marks. Restored only that header from UTF-8 uploaded script; historical body and benchmark evidence unchanged. Future Unicode writes use uploaded UTF-8 files.','sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'next':'Ordered source plan implementation and small gates'}
q=root/'experiments/ordered_source_plan_20261014/encoding_repair_event.json';q.write_text(json.dumps(e,indent=2)+'\n')
