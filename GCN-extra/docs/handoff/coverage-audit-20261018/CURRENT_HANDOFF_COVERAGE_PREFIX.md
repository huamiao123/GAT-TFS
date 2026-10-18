# 2026-10-18 覆盖补审与上一轮勘误

事件：`experiment-coverage-audit-20261018-history-errata`。

当前应先读 [实验覆盖补审与勘误](EXPERIMENT_COVERAGE_AUDIT_20261018.md)，再读上一轮技术总审。上一轮不能被理解为从10月4日起全部历史版本均已深审完成。现已补恢复清理前3,969文件（2,789路径不在HEAD），将36个当前组织单元、140个run容器、历史旧实验及失败/取消链分开登记。

**明确撤回上一轮附件 `review-20261018/audit_projection.md:114` 的native FP32串行首触断言。** 最终runtime已使用parallel-static精确copy，20条raw config及实际stdout一致；串行混杂的是已取消早期轮。旧SOURCE_AUDIT和报告文字滞后，不能推翻实际控制。

新增覆盖包括独立S/M的2,720条（区别于A3/P0的2,890条）、主/cache与feature原始日志、八族13版本62作业37,110主计时、清理前replay/nozero/25图/ILP64/source与paper V3基线修正。具体源码、结果、失败版本和未独立验证项见附件目录 `coverage-audit-20261018/` 的台账。原始文件、旧报告与旧事件保持不变。

本轮没有新跑benchmark、没有新性能突破声明、没有使用NUMA interleave，也没有改动128→128→128主比较口径。不要把已探索的replay、pair、fixed-address、prefetch或离线trace整体重新标成“尚未尝试”；下一候选必须指出实际新机制与强基线。

---

