# 2026-10-18 C4 真实图独享复核与强基线勘误

`Job10996686` 在 `qhcn818` 独享32核完成。Indochina-2004 的同计划共同 mask 聚合中位数494.53ms，逐源789.74ms，机制收益1.597×；as-Skitter为1.092×，LiveJournal1、Reddit近于无益。四图数值探针通过。见[真实图报告](../../results/consumer-mask-c4-kernel-20261018/REALGRAPH_SPARSE_STAGE.md)，原始证据 `runs/consumer_mask_c4_realgraphs_20261018-01/`。

**重要勘误：**独立 rowwise 参考逐边读写输出；canonical FULL 每行以8个 AVX-512 寄存器累计后一次写出。此前所有“胜独立逐行”的倍数不能当作胜 FULL 的证据；同计划 fanout→shared 对照仍有效。下一步必须在同一独享作业加入寄存器驻留 rowwise 强对照，再决定是否集成 AMX 两层。当前未有论文级突破；仍保持128→128→128和默认NUMA，禁用interleave。

---

