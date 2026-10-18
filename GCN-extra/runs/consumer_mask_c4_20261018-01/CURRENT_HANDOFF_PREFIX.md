# 2026-10-18 C4 共同消费者 mask 结构门槛

已完成 `Job10996266`：共享 `intel` 节点 `qhcn252`、1 CPU、17图、Identity/DegreeSort 共34条记录，Slurm `COMPLETED 0:0`；小图独立 oracle、17份输入 SHA256、边覆盖和历史顺序哈希均通过。详见[完整报告](../../results/consumer-mask-c4-20261018/STRUCTURAL_GATE.md)，原始数据在 `runs/consumer_mask_c4_20261018-01/`。

关键结果是**结构可行性而非加速**：相同消费者 mask 下的理想 AVX 加法节省，Identity Mycielskian19 67.11%、Hollywood 43.57%、Indochina 77.15%，但 Products 0.08%、RoadNet 0.79%。DegreeSort 在 Reddit 提高潜在节省，在多个高重叠图降低它。尚未实现分组聚合内核，也未验证 FP32/BF16 误差、计划物化成本和端到端时间；不能据此称期刊突破。原 FULL 的 AMX tile 数不会自动减少。

下一门槛：固定同一分组计划，比较逐源 fanout 与 mask 共聚合，计入 plan build/bytes；随后与 FULL Identity 做同精度、同 epilogue 的两层对照。主形状仍 128→128→128；禁止 NUMA interleave。`scripts/record_event.py check` 应保持三账同步。

---

