# 可用数据集与编译链（2026-09-28）

所有新代码、编译输出、转换数据和实验日志均写入 /home/huangjianqiang_group/hdacp1/data/wzh/GAT。yx 目录只读。

| 数据集 | 位置 | 当前状态与用途 |
|---|---|---|
| ogbn-arxiv | wzh/TFS-Train/runs/c5_fullstep_arxiv_20260804_v3/dataset/arxiv | 官方 raw 特征、边、标签和 time split；169343 节点、128 维、40 类。已转换到 GAT/data/arxiv.gatbin，作为第一阶段真实图基线 |
| Cora / Citeseer / Pubmed | wzh/datasets/planetoid | Planetoid pickle 齐全；后续需要 loader |
| GraphSAINT Amazon / Flickr / Reddit / Yelp | wzh/datasets/graphsaint | adj_full、adj_train、feats、class_map、role 齐全；需要统一标签语义 |
| ogbn-products | wzh/datasets/ogbn-products_official/products | 官方 raw 特征、边、标签、split；大图扩展候选 |
| IGB small / medium | wzh/datasets/igb_homogeneous_small 与 wzh/datasets/igb/igb_hom_medium | processed 特征、边、标签；需核查 split |
| ogbn-papers100M | wzh/datasets/ogb/ogbn_papers100M/papers100M-bin | 官方 raw 和 time split；数据很大，当前不运行 |
| Amazon Computers | wzh/datasets/gnn_benchmark/amazon_co_buy_computer | adjacency、attributes、labels；无官方 split |

yx/TFS/data 有 email-Enron、amazon0601、Reddit、ogbn-products、Friendster 等约 25 个图、28 个 CSR binary。它们主要是图结构，不等同于可直接训练的带特征、标签、split 数据集。yx/SVSIG/datasets 的动态流当前也不作为本项目的训练数据。yx 下全部只读。

可用编译链：Intel oneAPI compilers 模块 intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp（icpx 2024.1.0）；Intel oneAPI MKL 模块 intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3（包含 mkl.h 和 SGEMM）；系统 /usr/bin/g++ 8.5.0。yx/TFS/scripts 的旧脚本仅供只读参考，其中有过时路径，不能直接运行。

目前代码使用 oneAPI icpx + MKL，AVX-512 在 Intel Xeon CPU Max 9462 节点运行。未使用 AMX。
