# 与原TFS源码对齐的MKL比较协议

本协议依据用户2026-10-04要求，覆盖2026-10-03合同中关于NUMA、MKL线程控制、随机数据顺序和统计方式的性能比较条款。旧代码及结果保留，不能称为原源码协议复现。GCN-extra仍是独立的推理算子实验，不改成TFS-Train训练数学。

## 已核对的差异

| 项目 | 原TFS/MKL源码及脚本 | 2026-10-03实验 |
|---|---|---|
| 显式NUMA内存策略 | 原TFS和原MKL均无 | 全进程numactl interleave，包括TFS与MKL |
| 线程声明 | OMP_NUM_THREADS=32、OMP_PROC_BIND=close、OMP_PLACES=cores | 另设MKL_NUM_THREADS、关闭MKL动态线程 |
| MKL优化提示 | mm_hint调用次数10，随后optimize | 调用次数16，随后optimize |
| FP32缓冲区 | mkl_malloc(64)，未预先清零，原代码决定首次写入 | 初始化过的aligned std::vector，多个路径复用 |
| 随机输入 | 单层H→W；两层交替W1/W2→H0，srand(12345) | H0→W1→W2，srand(12345) |
| 激活/层间转换 | 两个独立OMP loop | 融合成一个loop |
| 主统计 | 一次预热、五轮最小值 | 一次预热、十轮中位数，Friendster五轮 |
| 编译选项 | -O3 -march=sapphirerapids（源文件相应AMX选项） | 显式AVX选项、-fp-model precise，未设march |
| MKL数学 | 读取原CSR values；FP32 SpMM→SGEMM | 拒绝非单位values后用ones；FP32 SpMM→SGEMM |

原代码未调用numa_alloc、mbind、set_mempolicy，也没有numactl包装。DegreeSort是逻辑行调度，不是NUMA内存优化。原代码中的并行写入、串行初始化和操作系统默认页面分配本身会产生内存布局，应保留这些行为，不能只删除numactl后继续换分配方式。

## 新实现的权威与可核验范围

- 单层原基线：`original/mkl_baseline.cpp`，SHA256 `00d330db4b5243f4fac5c5beccc2a06bc9199afa4724ca6a8d9babfa717b2295`。
- 两层权威：`original/gcn_e2e_bench.cpp`，SHA256 `9e3600ebbed20f9566f7051487c18c951f1c1cae305a6ea308507c71e7261e58`。它输出FP32，与当前候选输出类型一致。
- `gcn_e2e_v3.cpp`的MKL也是相同FP32 SpMM→SGEMM，但它的TFS最终输出直接为BF16，不能把该版本的E2E耗时当成FP32输出版本。
- 生成的`src/source_protocol.cpp`完整保留原E2E程序，只在原计时与精度检查之后加入候选函数调用和头文件。生成器确认删除这两处插入后能逐字符恢复原源码；原MKL函数、handle创建、hint=10、optimize、输入生成、缓冲区分配、五轮计时均不改。
- 候选复用原TFS的H1/H2/H1_bf16缓冲区，保留原ReLU与转换函数，避免额外NUMA放置。CSR通过只读view传递，没有另拷贝CSR。原kernel的去清零控制只移除初始memset。
- 原MKL、原TFS和候选均无显式NUMA策略。不调用mkl_set_dynamic/mkl_set_num_threads，不指定MKL_NUM_THREADS/MKL_DYNAMIC。复现原脚本明确声明的OMP变量；历史运行是否继承了其他环境未知。
- 另编译完全未修改的原MKL、原TFS和原E2E二进制，在相同节点对Reddit/Products核对；原始stdout完整保存。
- primary报五轮最小值，secondary报五轮中位数及P25/P75。MKL_FP32与BF16_AMX精度差异沿用原论文源码协议，数值误差另列。
- 新方法数值门在自身性能测量前执行。未修改原E2E程序自身仍按原顺序先计时后精度分析，必须保留这个实际行为。
- 细分计时与采样profile单独测量，不混入主加速比。sampled_thread_ms不等于wall占比。

## 数据范围、运行约束及边界

严格LP64原协议可运行17张已确认单位边权且未超32位边界的图。六张非单位边权文件仍排除，防止原TFS忽略values而MKL使用values造成不同算子。Friendster CSR offsets和Road-USA dense范围需要ILP64；本轮严格LP64复现将它们单列，不用昨天的ILP64新MKL替代原协议结果。

source-correctness使用共享intel的小型CSR fixture。source-formal使用单intel_expr独享节点、32线程，命令直接执行二进制，不增加numactl或srun绑核包装。记录实际affinity、NUMA policy、CPU拓扑、继承环境及线程设置；硬件、分区与历史环境并非论文旧机器完整复刻。

所有远端文件写在wzh/GCN-extra，yx只读。每个build/job保存唯一目录、源码/二进制/input哈希、原始日志与CSV，随后同步三份handoff。新结果单独报告，不覆盖昨天的冻结证据。全程最多两节点，低于用户五节点上限。随机H/W的sum算子保持原源码语义，不代表训练后分类准确率。
