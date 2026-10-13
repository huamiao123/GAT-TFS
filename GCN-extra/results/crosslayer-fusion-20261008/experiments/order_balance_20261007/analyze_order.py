#!/usr/bin/env python3
"""Read-only order-balance evidence analysis; all writes go to a NEW --out directory.

Usage: python3 analyze_order.py RUN_ROOT [RUN_ROOT ...] --out NEW_DIRECTORY
Optional: --cohort-map JSON maps Slurm job IDs (or log-directory names) to cohort
strings, e.g. {"10870001":"shared_primary","10870002":"shared_retest"}.
The script never executes a benchmark, repairs evidence, or updates snapshots.
"""
import argparse
import collections
import csv
import hashlib
import json
import math
import pathlib
import re
import statistics
import sys

WORKSPACE = pathlib.Path("/home/huangjianqiang_group/hdacp1/data/wzh")
SHAPES = ("128_32_32", "128_128_128")
FLOWS = ("A", "Pall", "Pused")
METHODS = tuple(a + "_" + b + "_" + s for a in FLOWS for b in FLOWS
                for s in ("degree", "qsource"))
BASE = "A_A_degree"
ANCHOR = "shape_mkl_fp32_original_style"
BOUNDARIES = ("kernel_FP32", "e2e_FP32_final", "e2e_BF16_final")
METRICS = (
    "edges", "hot", "cold", "cold_tiles", "cache_tiles", "parts", "tdp",
    "sparse_scalar_adds", "sparse_vector_adds", "sparse_feature_request_bytes",
    "bf16_decode_vectors", "demand_map_lookups", "prefetch_map_lookups",
    "window_state_load_bytes", "window_state_store_bytes",
    "cache_H_request_bytes", "cache_Q_write_bytes", "cache_storage_bytes")
STAGES = ("cache1_ms", "reduce_project1_ms", "relu_ms", "interlayer_ms",
          "cache2_ms", "reduce_project2_ms", "total_ms")
STRATUM = ("cohort", "data_class", "build_fingerprint", "shape", "kind")
CORE_FILES = ("selective_shape.cpp", "selective_kernels.hpp", "order_grouping.hpp",
              "gcn_e2e_v3.cpp", "paper_methods_kernels.hpp",
              "paper_methods_runtime.hpp", "projection_window_kernels.hpp",
              "projection_window_pmu.hpp", "PROTOCOL.md")
INPUT_FILES = {}
ISSUES = []


def digest(p):
    h = hashlib.sha256()
    with p.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def track(p):
    p = pathlib.Path(p)
    if p.exists() and p.is_file():
        INPUT_FILES[str(p.resolve())] = digest(p)


def read_json(p, default=None):
    if not p.exists():
        return {} if default is None else default
    track(p)
    return json.loads(p.read_text(encoding="utf-8"))


def read_csv(p):
    if not p.exists():
        return []
    track(p)
    with p.open(newline="", encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def issue(level, code, **data):
    ISSUES.append(dict(level=level, code=code, **data))


def gm(values):
    vals = list(values)
    if not vals:
        return ""
    if any(not math.isfinite(x) or x <= 0 for x in vals):
        raise ValueError("geometric mean requires finite positive values")
    return math.exp(statistics.mean(math.log(x) for x in vals))


def ratio(a, b):
    return a / b if b else ""


def metadata_key(r):
    return tuple(r[x] for x in STRATUM)


def numeric(v):
    return float(v) if v not in (None, "") else None


def write_csv(out, name, rows, fields=()):
    columns = list(fields)
    for row in rows:
        for key in row:
            if key not in columns:
                columns.append(key)
    if not columns:
        columns = ["no_records"]
    with (out / name).open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=columns)
        w.writeheader()
        w.writerows(rows)


def collect_packets(roots, cohort_map):
    packets = []
    seen = set()
    for root in roots:
        manifest = read_json(root / "manifest.json")
        hashes = []
        for name in CORE_FILES:
            p = root / "source_snapshot" / name
            if p.is_file():
                track(p)
                hashes.append((name, INPUT_FILES[str(p.resolve())]))
        fingerprint = hashlib.sha256(json.dumps(hashes).encode()).hexdigest()
        if len(hashes) != len(CORE_FILES):
            issue("warning", "incomplete_core_source_inventory", run=str(root),
                  present=len(hashes), expected=len(CORE_FILES))
        logs = root / "logs"
        if not logs.is_dir():
            issue("warning", "no_logs_directory", run=str(root))
            continue
        # Read per-graph folders only. Job-level aggregate CSVs would duplicate rows.
        folders = {p.parent for p in logs.glob("*/*/status.json")}
        folders.update(p.parent for p in logs.glob("*/*/order_time.csv"))
        folders.update(p.parent for p in logs.glob("*/*/order_accept.csv"))
        for folder in sorted(folders):
            canonical = str(folder.resolve())
            if canonical in seen:
                continue
            seen.add(canonical)
            jobdir = folder.parent
            env = read_json(jobdir / "environment.json")
            event = read_json(jobdir / "event.json")
            completion = read_json(jobdir / "completion.json", event.get("completion", {}))
            graph_list = read_json(jobdir / "graph_list.json", [])
            node_path = jobdir / "node.txt"
            track(node_path)
            node = node_path.read_text().strip() if node_path.exists() else ""
            status = read_json(folder / "status.json")
            jid = str(event.get("job") or env.get("SLURM_JOB_ID") or
                      (re.search(r"(\d+)$", jobdir.name).group(1)
                       if re.search(r"(\d+)$", jobdir.name) else jobdir.name))
            partition = str(event.get("partition") or
                            env.get("SLURM_JOB_PARTITION") or "unknown")
            allocation = "exclusive" if partition == "intel_expr" or str(event.get("exclusive", "")).lower() in ("1", "true") else (
                "shared" if partition == "intel" else "unknown")
            mode = str(event.get("mode") or completion.get("mode") or "unknown")
            inferred_paths = " ".join(str(x.get("path", "")) for x in graph_list)
            data_class = "smoke" if mode == "smoke" or "/fixtures/" in inferred_paths else (
                "synthetic" if mode == "synthetic" or "/synthetic/" in inferred_paths else "real")
            cohort = cohort_map.get(jid, cohort_map.get(jobdir.name,
                       event.get("cohort", "")))
            if isinstance(cohort, dict):
                cohort = cohort.get("cohort", "")
            tables = {}
            for name in ("time", "check", "equiv", "accept", "work", "stage",
                         "profile_gate", "complete", "config", "group", "plan",
                         "group_setup", "plan_setup"):
                tables[name] = read_csv(folder / ("order_" + name + ".csv"))
            graph = str(status.get("graph") or folder.name)
            # A completed graph can be analyzed while its job is processing another graph.
            # Missing final event is explicitly labelled IN_PROGRESS, never final PASS.
            job_ok = not event or (event.get("status") == "PASS" and int(event.get("exit_status", 1)) == 0)
            graph_ok = status.get("status") == "PASS"
            explicit = bool(cohort)
            packets.append(dict(root=str(root), folder=str(folder), jobdir=str(jobdir),
                job=jid, node=str(event.get("node") or node), partition=partition,
                allocation=allocation, data_class=data_class, mode=mode,
                cohort=str(cohort), explicit_cohort=explicit, graph=graph,
                input_sha256=str(status.get("actual_input_sha256", "")),
                build_fingerprint=fingerprint, tables=tables,
                job_ok=job_ok, graph_ok=graph_ok, graph_status=status.get("status", "MISSING"),
                job_status=event.get("status", "IN_PROGRESS"),
                manifest_study=manifest.get("study", "")))
    # Earliest OBSERVED run is primary, including failed attempts. Do not promote
    # a successful rerun into primary and thereby hide earlier failures.
    order = lambda p: (int(p["job"]) if p["job"].isdigit() else 10**30, p["jobdir"])
    grouped = collections.defaultdict(list)
    for p in packets:
        grouped[(p["allocation"], p["data_class"], p["build_fingerprint"],
                 p["graph"])].append(p)
    for group in grouped.values():
        group.sort(key=order)
        for i, p in enumerate(group):
            if not p["explicit_cohort"]:
                p["cohort"] = p["allocation"] + ("_primary" if i == 0 else "_retest")
    return packets


def analyze(packets):
    per_graph, audits, rejected, anchors, pairs = [], [], [], [], []
    work_rows, stage_rows, numerical_rows = [], [], []
    medians = {}
    samples = {}
    contexts = {}
    for packet in packets:
        tables = packet["tables"]
        context = {k: packet[k] for k in (
            "root", "folder", "job", "node", "partition", "cohort", "data_class",
            "graph", "input_sha256", "build_fingerprint")}
        context["job_status"] = packet["job_status"]
        context["graph_status"] = packet["graph_status"]
        if packet["manifest_study"] not in ("", "order"):
            issue("error", "not_order_experiment", **context)
            continue
        for shape in SHAPES:
            shape_context = dict(context, shape=shape)
            cfg = [r for r in tables["config"] if r.get("shape") == shape]
            complete = [r for r in tables["complete"] if r.get("shape") == shape]
            checks = [r for r in tables["check"] if r.get("shape") == shape]
            equiv = [r for r in tables["equiv"] if r.get("shape") == shape]
            accepts = [r for r in tables["accept"] if r.get("shape") == shape]
            # Empty failed folders still leave an audit trail.
            if not any((cfg, complete, checks, equiv, accepts)):
                audits.append(dict(shape_context, method="", valid=0,
                                   reason="no_shape_evidence"))
                continue
            config_ok = len(cfg) == 1 and cfg[0].get("aggregate") == "FULL_ACCURATE"
            config_ok = config_ok and cfg[0].get("final") == "BF16"
            config_ok = config_ok and cfg[0].get("projected_cache") == "FP32"
            config_ok = config_ok and cfg[0].get("numa") == "default_no_interleave"
            completed = len(complete) == 1 and complete[0].get("execution_pass") == "1"
            gr = tables["group"]
            grouping_ok = len(gr) == 2 and {r.get("grouping") for r in gr} == {"degree", "qsource"}
            grouping_ok = grouping_ok and all(r.get("pass") == "1" and r.get("same_per_slot_q64") == "1" for r in gr)
            grouping_ok = grouping_ok and all(len({r.get(k) for r in gr}) == 1 for k in ("rows", "full_tiles", "visit_rounds", "row_windows"))
            accept_map = collections.defaultdict(list)
            for r in accepts:
                accept_map[r.get("method", "")].append(r)
            time_groups = collections.defaultdict(list)
            for r in tables["time"]:
                if r.get("shape") == shape:
                    time_groups[(r.get("method", ""), r.get("kind", ""))].append(r)
            expected_set = set(METHODS)
            unexpected = {m for m, _ in time_groups} - expected_set - {ANCHOR}
            if unexpected:
                issue("error", "unexpected_timed_methods", methods=sorted(unexpected),
                      **shape_context)
            for m in METHODS:
                reasons = []
                if not packet["job_ok"]: reasons.append("job_not_completed_successfully")
                if not packet["graph_ok"]: reasons.append("graph_not_completed_successfully")
                if not config_ok: reasons.append("config_missing_or_precision_mismatch")
                if not completed: reasons.append("shape_not_complete")
                if not grouping_ok: reasons.append("fixed_q64_grouping_budget_failed_or_missing")
                if len(accept_map[m]) != 1:
                    reasons.append("accept_record_missing_or_duplicate")
                    accepted = False
                else:
                    accepted = accept_map[m][0].get("accepted") == "1"
                    if not accepted:
                        reasons.append("candidate_rejected")
                        rejected.append(dict(shape_context, method=m,
                                             accepted=0, timing_records=sum(
                                                len(time_groups[(m, k)]) for k in ("consecutive", "paired"))))
                cr = [r for r in checks if r.get("method") == m]
                good_checks = len(cr) == 3 and {r.get("boundary") for r in cr} == set(BOUNDARIES)
                for r in cr:
                    threshold = .01 if r.get("boundary") == "e2e_BF16_final" else .001
                    reference_ok = r.get("reference") == "matched_quantized_MKL_math"
                    gate_ok = abs(float(r.get("gate", "nan")) - threshold) < 1e-12
                    measured_ok = (r.get("finite") == "1" and
                        math.isfinite(float(r.get("relative_L2", "nan"))) and
                        math.isfinite(float(r.get("normalized_max", "nan"))) and
                        float(r.get("relative_L2", "nan")) < threshold and
                        float(r.get("normalized_max", "nan")) < threshold)
                    record_ok = reference_ok and gate_ok and measured_ok and r.get("pass") == "1"
                    good_checks = good_checks and record_ok
                    numerical_rows.append(dict(shape_context, **{
                        "method": m, "boundary": r.get("boundary", ""),
                        "reference": r.get("reference", ""), "gate": r.get("gate", ""),
                        "relative_L2": r.get("relative_L2", ""),
                        "normalized_max": r.get("normalized_max", ""),
                        "max_abs": r.get("max_abs", ""), "reported_pass": r.get("pass", ""),
                        "reference_gate_measurement_valid": int(record_ok)}))
                if not good_checks: reasons.append("numerical_checks_failed_or_missing")
                er = [r for r in equiv if r.get("method") == m and
                      r.get("reference") == "degree_same_dataflow"]
                if m.endswith("_qsource"):
                    equiv_ok = len(er) == 3 and {r.get("boundary") for r in er} == set(BOUNDARIES)
                    equiv_ok = equiv_ok and all(r.get("pass") == "1" and r.get("bitwise") == "1" for r in er)
                    if not equiv_ok: reasons.append("grouping_bitwise_gate_failed_or_missing")
                if m == BASE and shape == "128_128_128":
                    frozen = [r for r in equiv if r.get("method") == m and
                              r.get("reference") == "frozen_P0_FULL_ACCURATE"]
                    if len(frozen) != 1 or frozen[0].get("pass") != "1" or frozen[0].get("bitwise") != "1":
                        reasons.append("frozen_P0_gate_failed_or_missing")
                wr = [r for r in tables["work"] if r.get("shape") == shape and r.get("method") == m]
                pg = [r for r in tables["profile_gate"] if r.get("shape") == shape and r.get("method") == m]
                if accepted:
                    if len(wr) != 2 or {r.get("layer") for r in wr} != {"1", "2"} or not all(r.get("pass") == "1" and r.get("tdp") == r.get("expected_tdp") for r in wr):
                        reasons.append("work_gate_failed_or_missing")
                    if len(pg) != 2 or {r.get("layer") for r in pg} != {"1", "2"} or not all(r.get("pass") == "1" and r.get("bitwise") == "1" for r in pg):
                        reasons.append("profile_gate_failed_or_missing")
                if not accepted and any(time_groups[(m, k)] for k in ("consecutive", "paired")):
                    issue("error", "rejected_candidate_was_timed", method=m, **shape_context)
                for kind, count in (("consecutive", 5), ("paired", 10)):
                    kr = list(reasons)
                    rr = time_groups[(m, kind)]
                    vals = []
                    repeat_map = {}
                    try:
                        vals = [float(r["ms"]) for r in rr]
                        repeat_map = {int(r["repeat"]): float(r["ms"]) for r in rr}
                        timing_ok = (len(rr) == count and set(repeat_map) == set(range(count))
                                     and all(math.isfinite(v) and v > 0 for v in vals))
                        timing_ok = timing_ok and all(r.get("boundary") == "prepared_forward_Q_rebuilt" and r.get("anchor_precision_mismatch") == "0" for r in rr)
                        if kind == "paired":
                            timing_ok = timing_ok and all(int(r.get("pair", -1)) == int(r["repeat"]) // 2 for r in rr)
                    except (KeyError, ValueError):
                        timing_ok = False
                    if accepted and not timing_ok: kr.append("timing_incomplete_or_invalid")
                    valid = accepted and not kr and timing_ok
                    row = dict(shape_context, kind=kind, method=m,
                        order=m.rsplit("_", 1)[0], grouping=m.rsplit("_", 1)[1],
                        accepted=int(accepted), valid=int(valid), reason=";".join(kr),
                        samples=len(rr), median_ms="", min_ms="", max_ms="", cv="",
                        speedup_vs_A_A_degree="", speedup_vs_same_order_degree="",
                        paired_gm_vs_A_A_degree="", paired_gm_vs_same_order_degree="",
                        best_fixed_posthoc_candidate="", best_fixed_common_graphs="",
                        speedup_vs_best_fixed_posthoc="", fixed_reference_is_posthoc_oracle=1)
                    audits.append(dict(shape_context, kind=kind, method=m,
                                       valid=int(valid), reason=";".join(kr), samples=len(rr)))
                    if valid:
                        row.update(median_ms=statistics.median(vals), min_ms=min(vals),
                                   max_ms=max(vals), cv=statistics.pstdev(vals)/statistics.mean(vals))
                        key = (packet["folder"], shape, kind, m)
                        medians[key] = row["median_ms"]
                        samples[key] = repeat_map
                        contexts[key] = row
                    per_graph.append(row)
                # Keep raw counters/stages even if gates reject or timing incomplete.
                for r in wr:
                    work_rows.append(dict(shape_context, method=m, layer=r.get("layer", ""),
                        reported_work_pass=r.get("pass", ""), candidate_accepted=int(accepted),
                        **{x: r.get(x, "") for x in METRICS},
                        D=r.get("D", ""), F=r.get("F", ""), dataflow=r.get("dataflow", ""),
                        tdp_hi=(int(r["tdp"])//2 if r.get("dataflow")=="A" else int(r["tdp"])),
                        tdp_lo=(int(r["tdp"])//2 if r.get("dataflow")=="A" else 0),
                        two_shared_Q_buffers_allocated_bytes=2*int(cfg[0]["N"])*int(shape.split("_")[1])*4 if cfg else "",
                        expected_tdp=r.get("expected_tdp", "")))
                sr = [r for r in tables["stage"] if r.get("shape") == shape and r.get("method") == m]
                if sr:
                    stage_rows.append(dict(shape_context, method=m, samples=len(sr),
                        candidate_accepted=int(accepted), independent_from_main=1,
                        **{x: statistics.median(float(r[x]) for r in sr) for x in STAGES}))
            # Unmatched FP32 MKL is never a main speedup denominator or fixed winner.
            for kind, count in (("consecutive", 5), ("paired", 10)):
                rr = time_groups[(ANCHOR, kind)]
                vals = [float(r["ms"]) for r in rr if r.get("ms")]
                good = len(rr) == count and {r.get("repeat") for r in rr} == {str(i) for i in range(count)}
                good = good and all(r.get("anchor_precision_mismatch") == "1" and r.get("boundary") == "prepared_forward_Q_rebuilt" for r in rr)
                good = good and all(math.isfinite(v) and v > 0 for v in vals)
                anchors.append(dict(shape_context, kind=kind, method=ANCHOR,
                    samples=len(rr), median_ms=statistics.median(vals) if vals and good else "",
                    valid_timing=int(good and packet["job_ok"] and packet["graph_ok"]),
                    precision="FP32_master_H_W_FP32_interlayer", matched_precision=0,
                    comparable_fusion_speedup_denominator=0))
    # A graph name resolving to different inputs cannot be double-weighted or pooled.
    input_sets = collections.defaultdict(set)
    for row in per_graph:
        if row["input_sha256"]:
            input_sets[(metadata_key(row), row["graph"])].add(row["input_sha256"])
    conflicts = {k for k, v in input_sets.items() if len(v) > 1}
    for key in conflicts:
        issue("error", "graph_input_hash_conflict", stratum=key[0], graph=key[1],
              hashes=sorted(input_sets[key]))
    for audit in audits:
        if audit.get("kind") and (metadata_key(audit), audit["graph"]) in conflicts:
            audit["valid"] = 0
            audit["reason"] += ";graph_input_hash_conflict"
    for row in per_graph:
        key = (row["folder"], row["shape"], row["kind"], row["method"])
        if (metadata_key(row), row["graph"]) in conflicts:
            row["valid"] = 0
            row["reason"] += ";graph_input_hash_conflict"
            medians.pop(key, None);samples.pop(key, None)
        if not row["valid"]:
            continue
        for ref, field, pfield in (
            (BASE, "speedup_vs_A_A_degree", "paired_gm_vs_A_A_degree"),
            (row["order"] + "_degree", "speedup_vs_same_order_degree",
             "paired_gm_vs_same_order_degree")):
            rk = (row["folder"], row["shape"], row["kind"], ref)
            if rk not in medians:
                continue
            row[field] = medians[rk] / row["median_ms"]
            if row["kind"] == "paired":
                pr = []
                for rep, ms in sorted(samples[key].items()):
                    v = samples[rk][rep] / ms;pr.append(v)
                    pairs.append(dict(**{x: row[x] for x in shape_context_keys()},
                        kind=row["kind"], method=row["method"], reference=ref,
                        repeat=rep, pair=rep//2, candidate_ms=ms,
                        reference_ms=samples[rk][rep], speedup=v))
                row[pfield] = gm(pr)
    return per_graph, audits, rejected, anchors, pairs, work_rows, stage_rows, numerical_rows


def shape_context_keys():
    return ("root", "folder", "job", "node", "partition", "cohort", "data_class",
            "graph", "input_sha256", "build_fingerprint", "shape")


def summarize(per_graph):
    # Collapse repeated jobs to one equally weighted graph observation per stratum.
    graph_method = collections.defaultdict(list)
    stratum_graphs = collections.defaultdict(set)
    for r in per_graph:
        st = metadata_key(r)
        stratum_graphs[st].add(r["graph"])
        if r["valid"] and r["speedup_vs_A_A_degree"] != "":
            graph_method[(st, r["graph"], r["method"])].append(r)
    graph_summary = []
    graph_ratios = {}
    for (st, graph, method), rr in sorted(graph_method.items()):
        v = gm(r["speedup_vs_A_A_degree"] for r in rr)
        graph_ratios[(st, graph, method)] = v
        graph_summary.append(dict(zip(STRATUM, st), graph=graph, method=method,
            job_observations=len(rr), job_ids=";".join(sorted({r["job"] for r in rr})),
            equal_graph_weight=1, gm_job_speedup_vs_A_A_degree=v,
            median_job_median_ms=statistics.median(r["median_ms"] for r in rr)))
    fixed, selections = [], {}
    for st, all_graphs in sorted(stratum_graphs.items()):
        common = {g for g in all_graphs if all((st, g, m) in graph_ratios for m in METHODS)}
        scores = {}
        for method in METHODS:
            available = sorted(g for g in all_graphs if (st, g, method) in graph_ratios)
            vals = [graph_ratios[(st, g, method)] for g in available]
            common_vals = [graph_ratios[(st, g, method)] for g in sorted(common)]
            score = gm(common_vals)
            if common_vals: scores[method] = score
            fixed.append(dict(zip(STRATUM, st), method=method,
                graph_count=len(available), observed_graph_count=len(all_graphs),
                graph_names=";".join(available), complete_coverage=int(set(available)==all_graphs),
                gm_vs_A_A_degree=gm(vals), wins_vs_A_A_degree=sum(v>1 for v in vals),
                common_graph_count=len(common), common_graph_names=";".join(sorted(common)),
                common_graph_gm_vs_A_A_degree=score, posthoc_selected=0))
        if scores:
            winner = min(scores, key=lambda m: (-scores[m], m))
            selections[st] = (winner, len(common))
    local_lookup = {(r["folder"], r["shape"], r["kind"], r["method"]): r
                    for r in per_graph if r["valid"]}
    for r in per_graph:
        st = metadata_key(r)
        if st in selections:
            winner, n = selections[st]
            r["best_fixed_posthoc_candidate"] = winner
            r["best_fixed_common_graphs"] = n
            ref = local_lookup.get((r["folder"], r["shape"], r["kind"], winner))
            if r["valid"] and ref:
                r["speedup_vs_best_fixed_posthoc"] = ref["median_ms"]/r["median_ms"]
    for row in fixed:
        selected = selections.get(tuple(row[x] for x in STRATUM))
        row["posthoc_selected"] = int(bool(selected and selected[0] == row["method"]))
    # Interaction >1: qsource helps this order more than it helps AA.
    interactions = []
    for r in per_graph:
        if not r["valid"] or r["grouping"] != "qsource":
            continue
        def get(m):
            return local_lookup.get((r["folder"], r["shape"], r["kind"], m))
        od, aa, aq = get(r["order"]+"_degree"), get(BASE), get("A_A_qsource")
        if not all((od, aa, aq)):
            continue
        grouping_gain = od["median_ms"]/r["median_ms"]
        aa_gain = aa["median_ms"]/aq["median_ms"]
        order_gain = aa["median_ms"]/od["median_ms"]
        interactions.append(dict(**{x:r[x] for x in shape_context_keys()},kind=r["kind"],
            order=r["order"], grouping_gain=grouping_gain, AA_grouping_gain=aa_gain,
            interaction_ratio=grouping_gain/aa_gain, order_effect_degree=order_gain,
            combined_speedup_vs_A_A_degree=aa["median_ms"]/r["median_ms"],
            factorization_error=abs(order_gain*grouping_gain-aa["median_ms"]/r["median_ms"])))
    collapsed = collections.defaultdict(list)
    for r in interactions:
        collapsed[(metadata_key(r),r["graph"],r["order"])].append(r)
    per_order = collections.defaultdict(list)
    for (st,graph,order),rr in collapsed.items():
        per_order[(st,order)].append({
            x:gm(r[x] for r in rr) for x in ("grouping_gain","AA_grouping_gain",
                "interaction_ratio","order_effect_degree","combined_speedup_vs_A_A_degree")})
    interaction_summary = [
        dict(zip(STRATUM,st),order=order,graph_count=len(rr),
             **{x:gm(r[x] for r in rr) for x in rr[0]})
        for (st,order),rr in sorted(per_order.items())]
    return graph_summary, fixed, interactions, interaction_summary


def compare_work(work_rows):
    lookup = {(r["folder"],r["shape"],r["method"],r["layer"]):r for r in work_rows}
    out, invariants = [], []
    for r in work_rows:
        row = dict(r)
        order, grouping = r["method"].rsplit("_",1)
        for ref,label in ((BASE,"AA_degree"),(order+"_degree","same_order_degree")):
            base = lookup.get((r["folder"],r["shape"],ref,r["layer"]))
            if not base:
                continue
            for metric in METRICS + ("tdp_hi", "tdp_lo", "two_shared_Q_buffers_allocated_bytes"):
                if r.get(metric,"")=="" or base.get(metric,"")=="":
                    continue
                a,b = int(r[metric]),int(base[metric])
                row[metric+"_delta_vs_"+label] = a-b
                row[metric+"_ratio_vs_"+label] = ratio(a,b)
        out.append(row)
        if grouping=="qsource":
            base=lookup.get((r["folder"],r["shape"],order+"_degree",r["layer"]))
            missing=[];changed=[]
            if base:
                for metric in METRICS:
                    if r.get(metric,"")=="" or base.get(metric,"")=="":
                        missing.append(metric)
                    elif int(r[metric])!=int(base[metric]):
                        changed.append(metric)
            else:
                missing=["degree_work_record"]
            invariants.append(dict(**{x:r[x] for x in shape_context_keys()},
                method=r["method"],layer=r["layer"],pass_same_effective_work=int(not changed and not missing),
                changed_metrics=";".join(changed),missing_metrics=";".join(missing)))
    return out,invariants


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("runs",nargs="+",type=pathlib.Path)
    parser.add_argument("--out",required=True,type=pathlib.Path)
    parser.add_argument("--cohort-map",type=pathlib.Path)
    parser.add_argument("--cohort",action="append",default=[],metavar="JOB=COHORT",
                        help="explicit cohort override; repeatable, useful before the final job event exists")
    args=parser.parse_args()
    roots=[p.resolve() for p in args.runs]
    if any(not p.is_dir() for p in roots):
        parser.error("all run roots must be existing directories")
    workspace=WORKSPACE.resolve();out=args.out.resolve()
    if not out.is_relative_to(workspace):
        parser.error("--out must be inside the authorized wzh workspace")
    if out.exists():
        parser.error("--out must not exist; frozen/previous analyses are never overwritten")
    mapping=read_json(args.cohort_map) if args.cohort_map else {}
    for item in args.cohort:
        if "=" not in item:
            parser.error("--cohort requires JOB=COHORT")
        job, cohort = item.split("=", 1)
        if not job or not cohort:
            parser.error("--cohort requires nonempty JOB and COHORT")
        mapping[job] = cohort
    packets=collect_packets(roots,mapping)
    if not packets:
        parser.error("no per-graph order evidence found at logs/*/<graph>/")
    per_graph,audits,rejected,anchors,pairs,work,stages,numerical=analyze(packets)
    graph_summary,fixed,interaction,interaction_summary=summarize(per_graph)
    work_compare,work_invariants=compare_work(work)
    out.mkdir(parents=True,exist_ok=False)
    datasets={
        "per_graph.csv":per_graph, "fixed_method_gm.csv":fixed,
        "graph_equal_weight_summary.csv":graph_summary,
        "grouping_order_interaction.csv":interaction,
        "grouping_order_interaction_gm.csv":interaction_summary,
        "paired_samples.csv":pairs, "actual_work_comparison.csv":work_compare,
        "group_work_invariants.csv":work_invariants, "independent_stage_summary.csv":stages,
        "numerical_checks.csv":numerical, "candidate_audit.csv":audits,
        "rejected_candidates.csv":rejected, "fp32_mkl_anchor.csv":anchors}
    for name,rows in datasets.items():
        write_csv(out,name,rows)
    summary=dict(input_runs=[str(p) for p in roots],graph_job_packets=len(packets),
        valid_candidate_protocol_records=sum(r["valid"] for r in per_graph),
        rejected_candidate_records=len(rejected),
        invalid_candidate_protocol_records=sum(not r["valid"] for r in per_graph),
        strata=[dict(zip(STRATUM,st)) for st in sorted({metadata_key(r) for r in per_graph})],
        issues=ISSUES,
        interpretation=[
            "Q reconstruction is inside every prepared forward and every projected layer.",
            "FAST and ACCURATE experiments must never be used as interchangeable denominators.",
            "All main ratios match graph/job/shape/timing-kind; paired ratios also match repeat.",
            "Geometric means use each graph once; repeated jobs collapse within graph first.",
            "Shared, exclusive, primary, and retest cohorts remain separated.",
            "Build fingerprints and smoke/synthetic/real inputs remain separated.",
            "A complete PASS graph in a running job is included with job_status=IN_PROGRESS; final job status remains pending.",
            "Best fixed candidates are selected posthoc on the common valid graph set; not a deployed selector.",
            "FP32 master-input MKL is a precision-mismatched anchor, never a main fixed winner.",
            "Rejected or incomplete candidates do not enter successful performance summaries.",
            "Stage medians and sampled thread times are not additive primary E2E components.",
            "Logical bytes are not DRAM traffic; software-counted TDP instructions are not AMX wall utilization.",
            "tdp_hi/tdp_lo split follows the measured branch: FULL ACCURATE uses two equal passes, Pall/Pused one hi pass.",
            "cache_storage_bytes is active per-layer Q; two_shared_Q_buffers_allocated_bytes is the shared maximum-capacity allocation.",
            "The objective is latency under fixed error gates, not balanced AVX/AMX utilization."],
        input_hashes=INPUT_FILES,analysis_source_sha256=digest(pathlib.Path(__file__)))
    (out/"analysis.json").write_text(json.dumps(summary,indent=2,ensure_ascii=False,allow_nan=False)+"\n",encoding="utf-8")
    report=["ORDER BALANCE ANALYSIS","",*summary["interpretation"],"",
        "Records: valid=%d rejected=%d invalid=%d" % (
            summary["valid_candidate_protocol_records"],len(rejected),
            summary["invalid_candidate_protocol_records"]),
        "","Fixed configurations (per-graph equal weight; >1 is faster than A_A_degree):"]
    for row in fixed:
        report.append("%s | %s | %s | %s | %s | n=%d | GM=%s | common_n=%d | posthoc_selected=%d" % (
            row["cohort"],row["data_class"],row["shape"],row["kind"],row["method"],
            row["graph_count"],row["gm_vs_A_A_degree"],row["common_graph_count"],
            row["posthoc_selected"]))
    report.extend(["","Audits and issues:"]+[json.dumps(x,ensure_ascii=False) for x in ISSUES])
    (out/"REPORT.txt").write_text("\n".join(report)+"\n",encoding="utf-8")
    print(json.dumps(dict(out=str(out),valid=summary["valid_candidate_protocol_records"],
        rejected=len(rejected),invalid=summary["invalid_candidate_protocol_records"],
        issues=len(ISSUES)),ensure_ascii=False))
    for line in summary["interpretation"][:9]:
        print(line)
    return 2 if any(x["level"]=="error" for x in ISSUES) else 0


if __name__=="__main__":
    sys.exit(main())
