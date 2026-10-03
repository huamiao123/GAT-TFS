# yx 真实GCN数据只读清单

检查日期：2026-10-03。只读取文件目录、构建脚本、NPY头及PyTorch ZIP/pickle元数据，没有加载全部大型张量、执行pickle或修改yx。

共同根路径：`/home/huangjianqiang_group/hdacp1/data/yx/TFS/journal_rethink_20260914/`。

## Reddit

目录：`pmu_20260915/data_dgl_reddit/graphsaint_bundle/`。

- `feats.npy`：FP32，shape=(232965,602)，文件头已确认。
- `adj_full.npz`：邻接数据。
- `class_map.json`：分类标签。
- `role.json`：训练/验证/测试划分。
- `tfs_graph_csr_v1.pt`：TFS数据包，不是模型checkpoint。

同级`data_dgl_reddit/label.npy`及`node_types.npy`均为232965个int32元素，文件头已确认。构建脚本`ar_ab_20260915/build_dgl_reddit_graphsaint.py`说明特征来源为DGL Reddit 602维特征，41类标签；`fingerprint.log`记录其CSR与DGL图的对齐检查。真实特征不能当作128维随机输入直接替换，需遵守真实模型形状与节点顺序。

## ogbn-products

文件：`ar_ab_20260915/datasets_reordered/ogbn_products_natural_copy.pt`。

PyTorch ZIP的`data.pkl`元数据已确认如下键：`x`、`labels`、`rowptr`、`colidx`、`scale`、`degree`、`schedule`、`train_mask`、`valid_mask`、`test_mask`。

其中`x`为FP32，shape=(2449029,100)；`labels`为int64，shape=(2449029,)。对应脚本`ar_ab_20260915/make_reordered_products.py`说明natural_copy为canonical数据的identity permutation，保留特征、标签、归一化scale与划分。另有metis256、metis512、rcm版本及排列文件；使用它们时必须保持节点顺序一致。

## 权重与当前边界

当前搜索未确认可直接用于完整训练后GCN推理的checkpoint。发现的`w0_after_step1.npy`属于单步调试权重，不能据此宣称存在完整训练模型。数据包或排列文件的`.pt`扩展名不代表模型权重。

当前GCN-extra采用随机H/W、单位邻接sum、128→128→128；上述真实特征、标签及scale尚未接入，不能直接沿用已有速度/误差作为真实分类模型的结论。下一步应先核对checkpoint与GCN数学、输入维度、归一化、自环、bias/activation，再单独开展真实模型推理与任务精度验证。
