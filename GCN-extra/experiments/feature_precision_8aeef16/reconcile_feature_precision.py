#!/usr/bin/env python3
"""Summarize one immutable feature-precision study; never pool old timings.

Usage: python reconcile_feature_precision.py STUDY [--no-plot]
Input: STUDY/logs/RUN/GRAPH/records.json (suite's tag -> list schema).
Output: STUDY/results/GCN_EXTRA_FEATURE_PRECISION_ABLATION.md and CSV evidence.
Only completed, successful real-graph records enter timing comparisons. Failed
and incomplete records, including every failed gate, remain in raw CSV files.
Distinct RUNs are independent observations and are never pooled across nodes.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import statistics
from collections import defaultdict
from pathlib import Path

METHODS = ("b64_bf16", "b64_fp32", "full_bf16", "full_fp32")
ANCHORS = ("original_TFS", "source_MKL_FP32")
FIXED_COMMIT = "8aeef1613fdeaeb929379874c4152eccde620f74"
CHECK_TAGS = ("CHECK", "EQ", "DIFF", "PROFILE_GATE")
CV_WARNING = 0.10  # Descriptive noise flag, not a numerical correctness gate.


def load_json(path: Path, default=None):
    if not path.exists():
        return default
    with path.open(encoding="utf-8-sig") as handle:
        return json.load(handle)


def digest(path: Path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def number(value):
    try:
        value = float(value)
        return value if math.isfinite(value) else None
    except (ValueError, TypeError):
        return None


def flag(value):
    return value is True or str(value).lower() in ("1", "true", "pass", "passed")


def quantile(values, q):
    values = sorted(values)
    if not values:
        return None
    p = (len(values) - 1) * q
    lo = math.floor(p)
    hi = math.ceil(p)
    return values[lo] + (values[hi] - values[lo]) * (p - lo)


def stats(values):
    values = [v for v in (number(x) for x in values) if v is not None]
    if not values:
        return dict(n=0, min_ms=None, p25_ms=None, median_ms=None, p75_ms=None,
                    max_ms=None, mean_ms=None, cv=None)
    mean = statistics.mean(values)
    return dict(n=len(values), min_ms=min(values), p25_ms=quantile(values, .25),
                median_ms=statistics.median(values), p75_ms=quantile(values, .75),
                max_ms=max(values), mean_ms=mean,
                cv=statistics.pstdev(values) / mean if mean else 0)


def divide(a, b):
    return a / b if a is not None and b is not None and b > 0 else None


def fmt(value, places=3):
    value = number(value)
    if value is None:
        return "N/A"
    if places == 0:
        return f"{value:,.0f}"
    return f"{value:.{places}f}"


def md_table(headers, rows):
    def cell(value):
        return str(value).replace("|", "\\|").replace("\n", " ")
    return ("| " + " | ".join(map(cell, headers)) + " |\n" +
            "| " + " | ".join("---" for _ in headers) + " |\n" +
            "\n".join("| " + " | ".join(map(cell, row)) + " |" for row in rows) + "\n")


def write_csv(path, rows, first=()):
    rows = list(rows)
    columns = list(first)
    for row in rows:
        for key in row:
            if key not in columns:
                columns.append(key)
    with path.open("w", newline="", encoding="utf-8-sig") as handle:
        writer = csv.DictWriter(handle, fieldnames=columns)
        writer.writeheader()
        for row in rows:
            writer.writerow({key: "N/A" if value is None else
                             json.dumps(value, ensure_ascii=False, sort_keys=True)
                             if isinstance(value, (dict, list)) else value
                             for key, value in row.items()})


def record_sets(study):
    """Prefer per-graph records and omit each RUN's aggregate duplicate."""
    for path in sorted((study / "logs").rglob("records.json")):
        if path.parent.parent == study / "logs":
            continue  # RUN/records.json is an aggregate of GRAPH/records.json.
        data = load_json(path, {})
        if not isinstance(data, dict):
            raise ValueError(f"Expected suite tag->list schema: {path}")
        records = {str(tag).removeprefix("PRECISION_"): rows
                   for tag, rows in data.items() if isinstance(rows, list)}
        parent = path.parent
        run = parent.parent
        status = load_json(parent / "status.json", {})
        completion = load_json(run / "completion.json", {})
        graph_manifest = load_json(parent / "manifest.json", {})
        graph_stats = load_json(parent / "graph_stats.json", {})
        config = records.get("CONFIG", [{}])[0] if records.get("CONFIG") else {}
        graph = str(config.get("graph", parent.name))
        complete = records.get("COMPLETE", [])
        gate_rows = [row for tag in CHECK_TAGS for row in records.get(tag, [])]
        failed_gates = [row for row in gate_rows if not flag(row.get("pass", 0))]
        declared_status = str(status.get("status", "")).upper()
        driver_exit = load_json(parent / "driver.exit.json", {})
        exit_value = next((driver_exit[key] for key in
                           ("returncode", "return_code", "exit_code", "exit_status")
                           if key in driver_exit), None)
        successful = bool(complete) and all(flag(row.get("pass", 0)) for row in complete)
        successful &= not failed_gates and declared_status not in ("FAILED", "FAIL", "ERROR")
        if exit_value is not None:
            successful &= number(exit_value) == 0
        mode = str(completion.get("mode", run.name.split("-")[0]))
        smoke = "smoke" in mode.lower() or "smoke" in run.name.lower()
        node_file = run / "node.txt"
        node = node_file.read_text().strip() if node_file.exists() else str(completion.get("node", "unknown"))
        yield dict(id=run.name + "/" + graph, graph=graph, run=run.name,
                   node=node, mode=mode, smoke=smoke, eligible=successful and not smoke,
                   successful=successful, status=status, completion=completion,
                   driver_exit=driver_exit, records=records, config=config,
                   graph_stats=graph_stats, graph_manifest=graph_manifest,
                   evidence=str(path.relative_to(study)), records_sha256=digest(path),
                   failed_gates=len(failed_gates), affinity=load_json(run / "affinity.json", {}))


def time_groups(item):
    groups = defaultdict(list)
    for row in item["records"].get("TIME", []):
        if number(row.get("ms")) is not None:
            groups[(str(row.get("method")), str(row.get("kind")))].append(row)
    return groups


def paired_comparison(groups, bf16, fp32, kind):
    b = groups.get((bf16, kind), [])
    f = groups.get((fp32, kind), [])
    bs = stats(row.get("ms") for row in b)
    fs = stats(row.get("ms") for row in f)
    bm = {int(row["repeat"]): row for row in b if "repeat" in row}
    fm = {int(row["repeat"]): row for row in f if "repeat" in row}
    paired = []
    oriented = defaultdict(list)
    for repeat in sorted(set(bm) & set(fm)):
        ratio = divide(number(bm[repeat]["ms"]), number(fm[repeat]["ms"]))
        if ratio is not None:
            paired.append(ratio)
            oriented[str(bm[repeat].get("orientation", "unknown"))].append(ratio)
    p25, p50, p75 = (quantile(paired, q) for q in (.25, .5, .75))
    direction = "fp32_faster" if p25 is not None and p25 > 1 else \
                "bf16_faster" if p75 is not None and p75 < 1 else "overlap"
    fwd = statistics.median(oriented["forward"]) if oriented["forward"] else None
    rev = statistics.median(oriented["reverse"]) if oriented["reverse"] else None
    order_consistent = (fwd is not None and rev is not None and
                        (fwd - 1) * (rev - 1) > 0 and
                        ((direction == "fp32_faster" and fwd > 1 and rev > 1) or
                         (direction == "bf16_faster" and fwd < 1 and rev < 1)))
    low_cv = all(s["cv"] is not None and s["cv"] <= CV_WARNING for s in (bs, fs))
    stable = len(paired) == 10 and direction != "overlap" and order_consistent and low_cv
    return dict(bf16_median_ms=bs["median_ms"], fp32_median_ms=fs["median_ms"],
                ratio=divide(bs["median_ms"], fs["median_ms"]), paired_repeats=len(paired),
                paired_p25=p25, paired_median=p50, paired_p75=p75,
                bf16_cv=bs["cv"], fp32_cv=fs["cv"], forward_ratio=fwd, reverse_ratio=rev,
                order_consistent=order_consistent, direction=direction, stable_iqr=stable)


def make_plot(path, comparisons):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        return dict(available=False, reason="matplotlib unavailable")
    labels = list(dict.fromkeys(row["dataset"] for row in comparisons))
    if not labels:
        return dict(available=False, reason="no complete real graphs")
    fig, ax = plt.subplots(figsize=(9.2, max(3, len(labels) * .55)))
    for block, offset, color, marker in (("b64", -.12, "#17649b", "o"),
                                         ("full", .12, "#cc6e26", "s")):
        subset = [row for row in comparisons if row["block"] == block and row["kind"] == "e2e"]
        for row in subset:
            if row["paired_median"] is None:
                continue
            center = row["paired_median"]
            ax.errorbar(center, labels.index(row["dataset"]) + offset,
                        xerr=[[center - row["paired_p25"]], [row["paired_p75"] - center]],
                        fmt=marker, color=color, capsize=3,
                        label=block.upper() if row is subset[0] else None)
    ax.axvline(1, color="#555555", linewidth=1, linestyle="--")
    ax.set_yticks(range(len(labels)), labels)
    ax.invert_yaxis()
    ax.set_xlabel("Paired BF16 / FP32 E2E time ratio (median and IQR)")
    ax.set_title("Same-run shared-node feature storage ablation")
    ax.grid(axis="x", alpha=.25)
    ax.legend(frameon=False)
    fig.tight_layout()
    for suffix in ("png", "svg", "pdf"):
        fig.savefig(path / ("feature_precision_e2e_ratio." + suffix), dpi=180)
    plt.close(fig)
    return dict(available=True, files=["feature_precision_e2e_ratio." + s for s in ("png", "svg", "pdf")])


def reconcile(study: Path, plot=True):
    study = study.resolve()
    manifest = load_json(study / "manifest.json", {})
    items = list(record_sets(study))
    source_ok = manifest.get("fixed_commit") == FIXED_COMMIT
    valid = [item for item in items if item["eligible"] and source_ok]
    out = study / "results"
    out.mkdir(parents=True, exist_ok=True)
    raw = defaultdict(list)
    index = []
    for item in items:
        index.append({key: item[key] for key in ("id", "graph", "run", "node", "mode", "smoke",
                                                "successful", "eligible", "failed_gates",
                                                "evidence", "records_sha256")})
        for tag, records in item["records"].items():
            for row in records:
                raw[tag].append(dict(dataset=item["id"], run=item["run"], node=item["node"],
                                     eligible=item["eligible"] and source_ok, **row))
    for tag, rows in raw.items():
        write_csv(out / ("raw_" + tag.lower() + ".csv"), rows)
    write_csv(out / "record_sets.csv", index)

    timing, comparison, stages, operations, correctness = [], [], [], [], []
    fine_profile, threads, pmu_summary, anomalies, interactions = [], [], [], [], []
    e2e_rows, kernel_rows, stage_rows, operation_rows = [], [], [], []
    pmu_table, correctness_rows = [], []
    for item in valid:
        graph, dataset = item["graph"], item["id"]
        groups = time_groups(item)
        base = dict(dataset=dataset, graph=graph, run=item["run"], node=item["node"])
        config, gs = item["config"], item["graph_stats"]
        n = number(config.get("N", gs.get("N", gs.get("nodes"))))
        e = number(config.get("E", gs.get("E", gs.get("edges"))))
        comparisons = {}
        for (method, kind), records in sorted(groups.items()):
            s = stats(row["ms"] for row in records)
            fwd = stats(row["ms"] for row in records if row.get("orientation") == "forward")
            rev = stats(row["ms"] for row in records if row.get("orientation") == "reverse")
            entry = dict(base, method=method, kind=kind, **s,
                         forward_median_ms=fwd["median_ms"], reverse_median_ms=rev["median_ms"])
            timing.append(entry)
            order_spread = divide(max(fwd["median_ms"], rev["median_ms"]),
                                  min(fwd["median_ms"], rev["median_ms"])) \
                           if fwd["median_ms"] is not None and rev["median_ms"] is not None else None
            if s["cv"] is not None and s["cv"] > CV_WARNING or \
               order_spread is not None and order_spread > 1.10:
                anomalies.append(dict(entry, order_max_over_min=order_spread,
                                      warning="CV>10% or orientation median spread>10%; shared-node/order confounding"))
        for block in ("b64", "full"):
            for kind in ("e2e", "kernel", "matched_kernel", "preparation_inclusive"):
                result = paired_comparison(groups, block + "_bf16", block + "_fp32", kind)
                comparisons[(block, kind)] = result
                comparison.append(dict(base, block=block, kind=kind, **result))
                if kind in ("kernel", "matched_kernel", "preparation_inclusive"):
                    kernel_rows.append([graph, item["run"], block.upper(), kind,
                                        fmt(result["bf16_median_ms"]), fmt(result["fp32_median_ms"]),
                                        fmt(result["ratio"]),
                                        f'[{fmt(result["paired_p25"])}, {fmt(result["paired_p75"])}]',
                                        result["stable_iqr"]])
        b64, full = comparisons[("b64", "e2e")], comparisons[("full", "e2e")]
        e2e_rows.append([graph, item["run"], fmt(n, 0), fmt(e, 0),
                         fmt(b64["bf16_median_ms"]), fmt(b64["fp32_median_ms"]), fmt(b64["ratio"]),
                         fmt(full["bf16_median_ms"]), fmt(full["fp32_median_ms"]), fmt(full["ratio"]),
                         fmt(stats(row["ms"] for row in groups.get((ANCHORS[0], "e2e"), []))["median_ms"]),
                         fmt(stats(row["ms"] for row in groups.get((ANCHORS[1], "e2e"), []))["median_ms"])])
        crossover = b64["stable_iqr"] and full["stable_iqr"] and b64["direction"] != full["direction"]
        full_over_b64 = {}
        for dtype in ("bf16", "fp32"):
            fm = stats(row["ms"] for row in groups.get(("full_" + dtype, "e2e"), []))["median_ms"]
            bm = stats(row["ms"] for row in groups.get(("b64_" + dtype, "e2e"), []))["median_ms"]
            full_over_b64[dtype] = divide(fm, bm)
        interactions.append(dict(base, stable_crossover=bool(crossover),
                                 b64_direction=b64["direction"], full_direction=full["direction"],
                                 b64_stable=b64["stable_iqr"], full_stable=full["stable_iqr"],
                                 interaction_ratio=divide(full["ratio"], b64["ratio"]),
                                 full_over_b64_bf16=full_over_b64["bf16"],
                                 full_over_b64_fp32=full_over_b64["fp32"]))

        for method in METHODS:
            rows = [row for row in item["records"].get("STAGE", []) if row.get("method") == method]
            medians = {field: stats(row.get(field) for row in rows)["median_ms"] for field in
                       ("layer1_ms", "relu_ms", "interlayer_conversion_ms", "layer2_ms", "total_ms")}
            fractions = [divide(number(row.get("interlayer_conversion_ms")), number(row.get("total_ms"))) for row in rows]
            fraction = stats(100 * x for x in fractions if x is not None)["median_ms"]
            stages.append(dict(base, method=method, stage_repeats=len(rows),
                               conversion_fraction_percent=fraction, **medians))
            stage_rows.append([graph, item["run"], method] + [fmt(medians[key]) for key in
                               ("layer1_ms", "relu_ms", "interlayer_conversion_ms", "layer2_ms", "total_ms")] + [fmt(fraction)])
        for row in item["records"].get("COUNTERS", []):
            dtype = "fp32" if str(row.get("method")).endswith("fp32") else "bf16"
            visits = number(row.get("source_visits"))
            decode = number(row.get("source_decode_elements"))
            adds = number(row.get("feature_adds"))
            entry = dict(base, **row, feature_dtype=dtype,
                         feature_load_vector_instructions=visits * 8 if visits is not None else None,
                         feature_load_vector_bits=512 if dtype == "fp32" else 256,
                         decode_zeroextend_vector_ops=decode / 16 if decode is not None else None,
                         decode_shift_vector_ops=decode / 16 if decode is not None else None,
                         fp32_add_vector_ops=adds / 16 if adds is not None else None,
                         expected_source_bytes=e * 128 * (4 if dtype == "fp32" else 2) if e is not None else None,
                         visits_equal_graph_edges=visits == e if visits is not None and e is not None else None,
                         prefetch_bytes_per_hint_pair_row=256,
                         prefetch_fraction_of_next_row=.5 if dtype == "fp32" else 1)
            operations.append(entry)
            operation_rows.append([graph, item["run"], row.get("method"), row.get("layer"),
                                   fmt(row.get("source_gather_requested_bytes"), 0),
                                   fmt(entry["feature_load_vector_instructions"], 0),
                                   fmt(entry["decode_zeroextend_vector_ops"], 0),
                                   fmt(entry["decode_shift_vector_ops"], 0), fmt(entry["fp32_add_vector_ops"], 0),
                                   fmt(row.get("projection_scopes"), 0), fmt(row.get("amx_dpbf16ps"), 0),
                                   fmt(row.get("amx_spill_bytes"), 0), fmt(row.get("amx_reload_bytes"), 0)])
        for tag in ("PROFILE", "DETAIL"):
            fine_profile.extend(dict(base, level=tag.lower(), **row) for row in item["records"].get(tag, []))
        threads.extend(dict(base, **row) for row in item["records"].get("THREAD", []))
        for tag in CHECK_TAGS:
            for row in item["records"].get(tag, []):
                entry = dict(base, check_kind=tag, **row)
                correctness.append(entry)
                correctness_rows.append([graph, item["run"], row.get("method"), tag, row.get("boundary"),
                                         row.get("reference"), row.get("gate_type"), fmt(row.get("gate"), 6),
                                         fmt(row.get("max_abs"), 9), fmt(row.get("relative_L2"), 9),
                                         row.get("finite"), row.get("bitwise"), row.get("pass"),
                                         row.get("checksum_actual"), row.get("checksum_reference")])

        pmu_groups = defaultdict(list)
        for row in item["records"].get("PMU", []):
            pmu_groups[(row.get("method"), row.get("setname", row.get("set", "unknown")))].append(row)
        for method in METHODS:
            for setname in ("basic", "cache", "tlb", "stall"):
                rows = pmu_groups.get((method, setname), [])
                available = [row for row in rows if flag(row.get("available")) and flag(row.get("coverage_complete"))]
                scalable = [row for row in available if flag(row.get("scaled_complete"))]
                events = sorted({key.removesuffix("_raw") for row in rows for key in row if key.endswith("_raw")})
                if not events:
                    events = ["cycles", "instructions"]
                skew = [divide((number(row.get("start_enable_span_ms")) or 0) +
                               (number(row.get("stop_disable_span_ms")) or 0), number(row.get("region_wall_ms")))
                        for row in rows]
                for event in events:
                    raw_median = stats(row.get(event + "_raw") for row in available)["median_ms"]
                    scaled_median = stats(row.get(event + "_scaled") for row in scalable)["median_ms"]
                    pmu_summary.append(dict(base, method=method, setname=setname, event=event,
                                             repeats=len(rows), available_repeats=len(available),
                                             scaled_repeats=len(scalable), raw_median=raw_median,
                                             scaled_median=scaled_median,
                                             event_type=next((row.get(event + "_type") for row in rows), None),
                                             event_config=next((row.get(event + "_config") for row in rows), None),
                                             errors=sorted({str(row.get("error", "")) for row in rows if row.get("error")}),
                                             errno=sorted({row.get("errno") for row in rows if row.get("errno")}),
                                             enable_disable_fraction_max=max((x for x in skew if x is not None), default=None)))
                compact = "; ".join(event + "=" + fmt(stats(row.get(event + "_scaled") for row in scalable)["median_ms"], 0)
                                    for event in events)
                errors = "; ".join(sorted({str(row.get("error", "")) for row in rows if row.get("error")}))
                pmu_table.append([graph, item["run"], method, setname,
                                  f"{len(available)}/{len(rows)}", f"{len(scalable)}/{len(rows)}", compact,
                                  fmt(100 * max((x for x in skew if x is not None), default=0)), errors or "none"])

    for name, rows in (("timing_summary", timing), ("paired_comparisons", comparison),
                       ("stage_medians", stages), ("logical_operations", operations),
                       ("sampled_profiles", fine_profile), ("thread_profiles", threads),
                       ("pmu_summary", pmu_summary), ("correctness", correctness),
                       ("timing_anomalies", anomalies), ("factor_interactions", interactions)):
        write_csv(out / (name + ".csv"), rows)

    plots = make_plot(out, comparison) if plot else dict(available=False, reason="disabled")
    failed = [item for item in items if not item["successful"]]
    report = ["# GCN_EXTRA Feature Precision × Fusion Granularity Ablation\n",
              "**BF16/FP32 时间比 > 1 表示 FP32 更快；< 1 表示 BF16 更快。** 本报告仅使用此 STUDY 内同图、同进程、同节点、同协议的数据。",
              "\n## 来源、边界和执行状态\n",
              f"- 冻结源码：`{manifest.get('fixed_commit', 'missing')}`；锚点核验：`{source_ok}`。",
              f"- Study：`{study.name}`；完成真实图记录 {len(valid)} 组；失败/未完成 {len(failed)} 组；smoke {sum(item['smoke'] for item in items)} 组。",
              f"- 当前工作树：`{manifest.get('source_state', {}).get('actual_worktree_head', manifest.get('source_state', {}).get('head', 'see manifest.json'))}`。编译器、flags、dirty 状态、源码/二进制哈希均保留于 [manifest.json](../manifest.json)。",
              f"- 随机种子 `{manifest.get('seed', 'missing')}`；模型计算形状 `{manifest.get('shape', 'missing')}`；随机 H/W 推理计算流，未接入训练模型分类。",
              f"- 编译参数：`{' '.join(manifest.get('flags', []))}`。",
              "- 本轮由用户授权共享 `intel` 节点，32 物理核、单 socket；默认 NUMA、无显式 interleave、无新 MKL override。旧独享结果不作分母。实际分配见每 RUN 的 affinity.json/CPU/NUMA 证据。",
              "- Prepared E2E 排除初始 H 准备，包含本路径的层间表示转换；native FP32 保留 H1 FP32。单层、matched-input 和 preparation-inclusive 是不同边界。",
              "- 四条预取始终为 offsets 0/64/128/192：两种输入都请求前 256B；BF16 覆盖整行，FP32 仅覆盖半行。没有暗中添加八条预取。",
              "- native H0 FP32 的 serial first-touch 与 H0 BF16 的 parallel conversion、H1 的 scatter/conversion 可能形成不同页面分布。默认 NUMA 不证明实际页面放置相同；numa_maps 与 buffer 地址保留。",
              "- 同图重跑作为独立 run 保留，不跨节点合并，不选择最快 run。数值失败不会被静默删除；原始负结果与 gate failures 见 raw CSV、record_sets.csv 和每图日志。",
              "\n## 表 1：完整两层 prepared E2E\n",
              "单位 ms，主值为 10 次的中位数。比值为两个中位数相除；paired ratio/IQR 另见 paired_comparisons.csv。",
              md_table(["图", "RUN", "N", "E", "B64 BF16", "B64 FP32", "B64 BF/FP", "FULL BF16", "FULL FP32", "FULL BF/FP", "原 TFS", "源码 MKL"], e2e_rows),
              "\n### 单层与 matched-input / preparation-inclusive 补充\n",
              "matched_kernel 的 FP32 值由 BF16 无损展开，准备过程不计入 kernel；这是表示与加载/解码控制，仍需核对两份 buffer 的页分布。",
              md_table(["图", "RUN", "B", "边界", "BF16 ms", "FP32 ms", "BF/FP", "paired ratio IQR", "稳定方向"], kernel_rows),
              "\n## 表 2：未插桩阶段计时\n",
              "阶段来自独立 5 次 stage 测量，各列为中位数；conversion% 为每次 conversion/total 的中位数，阶段中位数之和不必恰好等于 total 中位数。",
              md_table(["图", "RUN", "方法", "L1 ms", "ReLU ms", "层间 conversion ms", "L2 ms", "total ms", "conversion %"], stage_rows),
              "\n## 表 3：实际代码的逻辑请求与运算计数（每层）\n",
              "load 指令模型为每边 8 条：BF16 为 256-bit load，FP32 为 512-bit load；BF16 另有 8 次 zero-extend 和 8 次 shift，FP32 没有这些 decode。不是 retired 指令或实测 DRAM 字节；编译器可能使用内存操作数融合。C spill 包括最后必要 store。完整所有计数列见 logical_operations.csv。",
              md_table(["图", "RUN", "方法", "层", "H 请求 B", "load vectors", "zext vectors", "shift vectors", "FP32 add vectors", "投影 scopes", "TDP", "C store B", "C reload B"], operation_rows),
              "\n## 表 4：独立 PMU 诊断\n",
              "只对 available 且 coverage_complete 的区域报告 raw；scaled 还要求 scaled_complete。逐 TID 先缩放再求和。N/A 表示不可用/不完整，不能由逻辑字节补造。cycles 是线程总 CPU cycles，AMX_BUSY 是 speculative arithmetic-busy cycles；均不是 wall cycles、TDP 次数或墙钟阶段占比。generic caches 不等于所有层 cache misses，load-stall 事件不是完整 TopDown，共享 uncore 带宽未归因。",
              md_table(["图", "RUN", "方法", "事件组", "raw可用/总", "scaled可用/总", "scaled中位数", "最大启停跨度/区域 %", "错误"], pmu_table),
              "\n## 表 5：正确性、边界和 checksum\n",
              "原 BF16 controls 的既有 gates、各路径自己的数学 oracle、无损展开 EQ、诊断与未插桩 bitwise gate 分别报告。native BF16/FP32 差异只作描述，不意味着分类精度。所有字段原值及失败项见 raw_check/eq/diff/profile_gate.csv。",
              md_table(["图", "RUN", "方法", "类型", "边界", "参考", "gate类型", "gate", "max_abs", "relative L2", "finite", "bitwise", "pass", "actual checksum", "reference checksum"], correctness_rows),
              "\n## Q1–Q7：由本轮数据回答\n"]

    conversion = [row for row in stages if row["method"].endswith("bf16")]
    conv_ms = [row["interlayer_conversion_ms"] for row in conversion if row["interlayer_conversion_ms"] is not None]
    conv_pct = [row["conversion_fraction_percent"] for row in conversion if row["conversion_fraction_percent"] is not None]
    report.append("### Q1：全局 FP32→BF16 层间转换是否为大图主瓶颈？\n")
    if conv_ms and conv_pct:
        report.append(f"已测 BF16 方法的 conversion 中位数范围 {min(conv_ms):.3f}–{max(conv_ms):.3f} ms，占独立 stage total 的 {min(conv_pct):.3f}–{max(conv_pct):.3f}%。各图分别见表 2；这衡量全局一次转换，不能代表每边 decode。")
        big = {item["id"] for item in valid if (number(item["config"].get("E")) or 0) >= 10_000_000}
        big_pct = [row["conversion_fraction_percent"] for row in conversion if row["dataset"] in big and row["conversion_fraction_percent"] is not None]
        if big_pct:
            report.append(f"按 E≥10M 明确界定的大图子集，该占比范围 {min(big_pct):.3f}–{max(big_pct):.3f}%。是否为主要瓶颈应依据这个占比与 L1/L2 时间，不能从历史 1–5 ms 猜测。")
    else:
        report.append("尚无完成真实图 stage 数据，无法回答。")

    report.append("\n### Q2：hot loop 每邻居 BF16 decode 的诊断成本？\n")
    decode_rows = [row for row in fine_profile if row.get("level") == "detail" and row.get("phase") == "source_bf16_decode"]
    report.append(md_table(["图", "RUN", "方法", "层", "采样 thread ms", "clocked sections"],
                           [[row["graph"], row["run"], row.get("method"), row.get("layer"), fmt(row.get("sampled_thread_ms"), 6), row.get("clocked_sections")]
                            for row in decode_rows]))
    report.append("这些值来自独立插桩采样：加载/解码分组、timer 开销和寄存器 spill 会影响结果。它们不是 E2E 占比，不能将其直接减去 E2E 预测去 decode 的收益。原始细分与线程尾部见 sampled_profiles.csv / thread_profiles.csv。")

    report.append("\n### Q3：native FP32 去掉 decode 是否节省计算？\n")
    report.append("代码模型确实移除每边 8 个 BF16 zero-extend 和 8 个 shift，同时保留 8 次 FP32 vector add；FP32 仍执行部分和截断、AMX、C staging。matched_kernel 观察相同数值的表示控制（表 1 补充），PMU 指令/cycles 用独立 native E2E 诊断（表 4）。后者还包含 native 数值和层间表示差异，不能独自证明 decode 的因果成本。")

    report.append("\n### Q4：两倍 feature 请求字节是否抵消节省？\n")
    for kind in ("e2e", "kernel", "matched_kernel"):
        subset = [row for row in comparison if row["kind"] == kind and row["ratio"] is not None]
        report.append(f"- `{kind}`：{len(subset)} 个图×B 对照中，BF16/FP32 ratio>1 的 {sum(row['ratio'] > 1 for row in subset)} 个，<1 的 {sum(row['ratio'] < 1 for row in subset)} 个；满足声明的稳定方向判据 {sum(row['stable_iqr'] for row in subset)} 个。")
    report.append("H 请求模型由 256E B 变为 512E B。结果只说明这两条完整受控路径的净效应；未测过程专属 DRAM 带宽，不将净变化全部归因于内存流量或 decode。")

    report.append("\n### Q5：B64 与 FULL 的答案是否不同？\n")
    report.append(md_table(["图", "RUN", "B64 paired IQR方向", "B64稳定", "FULL paired IQR方向", "FULL稳定", "ratio交互 FULL/B64"],
                           [[row["graph"], row["run"], row["b64_direction"], row["b64_stable"], row["full_direction"], row["full_stable"], fmt(row["interaction_ratio"])] for row in interactions]))
    report.append("两者共享相同原归约循环但投影次数、partial 转换与 C spill/reload 不同，具体计数见表 3。方向变化本身不等于硬件机制已被证明。")

    report.append("\n### Q6：是否存在稳定 precision×B crossover？\n")
    cross = [row for row in interactions if row["stable_crossover"]]
    report.append(f"本轮发现 {len(cross)} 个满足描述性稳定判据的 crossover：" + (", ".join(row["dataset"] for row in cross) if cross else "无") + "。")
    report.append("判据预先写入脚本：每个 B 有完整 10 对，paired ratio 的 P25–P75 完全在 1 同一侧，forward/reverse 中位数方向一致，两个方法各自 CV≤10%；B64/FULL 的稳定方向相反才记 crossover。IQR 判据不是统计显著性检验或硬件因果证明，完整结果不因 CV 警告而删除。")

    report.append("\n### Q7：Mycielskian 的 FULL 异常是否被 FP32 明显改变？\n")
    myc = [row for row in interactions if "myciel" in row["graph"].lower()]
    report.append(md_table(["图", "RUN", "BF16 FULL/B64", "FP32 FULL/B64", "precision×B ratio", "稳定 crossover"],
                           [[row["graph"], row["run"], fmt(row["full_over_b64_bf16"]), fmt(row["full_over_b64_fp32"]), fmt(row["interaction_ratio"]), row["stable_crossover"]] for row in myc]))
    if not myc:
        report.append("本 STUDY 尚无完成 Mycielskian 结果，暂不判断。")
    report.append("若两种表示的 FULL/B64 都明显>1，去 decode 未消除此退化，不支持 decode 单独主导的解释；但仍不能完全排除 decode 与访存/调度的交互。若明显改变，需结合 matched-input、线程尾部、页面分布、PMU 及启停偏差复核。")

    report.extend(["\n## 波动、顺序与失败证据\n",
                   "统计定义：P25/P75 采用排序后线性插值；CV=population std/mean。CV>10% 或 forward/reverse 方法中位数跨度>10% 标为诊断警告，保留所有原值。共享节点波动可影响小幅胜负。",
                   md_table(["图", "RUN", "方法", "边界", "CV %", "forward ms", "reverse ms", "order max/min"],
                            [[row["graph"], row["run"], row["method"], row["kind"], fmt(100 * row["cv"]), fmt(row["forward_median_ms"]), fmt(row["reverse_median_ms"]), fmt(row["order_max_over_min"])] for row in anomalies]),
                   md_table(["图", "RUN", "状态", "gate failures", "证据"],
                            [[item["graph"], item["run"], item["status"].get("status", "incomplete"), item["failed_gates"], item["evidence"]] for item in failed]),
                   "\n## 可导出数据和图\n",
                   "主表与统计：timing_summary.csv、paired_comparisons.csv、stage_medians.csv、logical_operations.csv、pmu_summary.csv、correctness.csv、factor_interactions.csv；所有重复及失败保留在 raw_*.csv。"])
    if plots["available"]:
        report.append("图的点为 paired ratio 中位数、误差线为 IQR，与主表两个时间中位数相除的定义分开标注。\n\n![Paired ratios](feature_precision_e2e_ratio.png)\n\n导出：[SVG](feature_precision_e2e_ratio.svg)、[PDF](feature_precision_e2e_ratio.pdf)。")
    else:
        report.append("独立图未生成：" + plots["reason"] + "。")
    (out / "GCN_EXTRA_FEATURE_PRECISION_ABLATION.md").write_text("\n\n".join(report) + "\n", encoding="utf-8")

    analysis = dict(study=study.name, fixed_commit=manifest.get("fixed_commit"), source_anchor_ok=source_ok,
                    complete_real_record_sets=len(valid), failed_or_incomplete_record_sets=len(failed),
                    comparisons=comparison, interactions=interactions, anomalies=anomalies, figures=plots,
                    input_record_sets=index, source_manifest_sha256=digest(study / "manifest.json") if (study / "manifest.json").exists() else None,
                    analysis_script_sha256=digest(Path(__file__)), shared_node=True,
                    ratios="BF16_ms/FP32_ms > 1 means FP32 faster", cv_warning_threshold=CV_WARNING,
                    no_old_timings=True, no_cross_run_pooling=True)
    (out / "feature_precision_analysis.json").write_text(json.dumps(analysis, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    artifact_hashes = {str(path.relative_to(out)): digest(path) for path in sorted(out.iterdir())
                       if path.is_file() and path.name != "analysis_artifact_hashes.json"}
    (out / "analysis_artifact_hashes.json").write_text(json.dumps(artifact_hashes, indent=2) + "\n", encoding="utf-8")
    return dict(report=str(out / "GCN_EXTRA_FEATURE_PRECISION_ABLATION.md"),
                complete_real_record_sets=len(valid), failed_or_incomplete_record_sets=len(failed),
                source_anchor_ok=source_ok, figures=plots)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("study", type=Path)
    parser.add_argument("--no-plot", action="store_true")
    args = parser.parse_args()
    print(json.dumps(reconcile(args.study, not args.no_plot), indent=2))


if __name__ == "__main__":
    main()
