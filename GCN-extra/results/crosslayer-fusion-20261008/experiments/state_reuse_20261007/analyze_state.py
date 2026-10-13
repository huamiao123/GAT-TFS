#!/usr/bin/env python3
"""Audit frozen state-reuse evidence (Python 3.8 standard library only).

Usage: python3 analyze_state.py RUN_PATH NEW_OUTPUT_DIRECTORY
Reads evidence only; writes the four analysis artifacts in a NEW directory.
Run the project's analysis preflight/handoff workflow before invoking this file.
"""
import argparse
import collections
import csv
import hashlib
import json
import math
import os
import pathlib
import re
import statistics
import sys

WORKSPACE = pathlib.Path("/home/huangjianqiang_group/hdacp1/data/wzh")
ALL17 = ("ogbn-products", "reddit", "mycielskian19", "hollywood-2009",
         "indochina-2004", "soc-Pokec", "cit-Patents", "soc-LiveJournal1",
         "com-LiveJournal", "as-Skitter", "rgg_n_2_24_s0", "web-Google",
         "roadNet-CA", "wiki-Talk", "email-Enron", "amazon0601", "com-Youtube")
DIAG4 = ("mycielskian19", "reddit", "roadNet-CA", "amazon0601")
REST13 = tuple(g for g in ALL17 if g not in DIAG4)
GRAPHSETS = {"all17": ALL17, "diag4": DIAG4, "rest13": REST13}
BASE = ("paper_tfs", "b256_fast", "bfull_fast", "p0_degree",
        "pack_epi_degree", "pair_i_degree", "pair_s_degree", "p0_qguard",
        "pack_epi_qguard", "pair_i_qguard", "pair_s_qguard")
DIAG_ONLY = ("state_identity", "nozero_degree", "direct_degree", "pack_degree",
             "epi_degree", "p0_m256", "pack_epi_m256")
ANCHOR = "source_mkl_fp32"
REFERENCES = ("p0_degree", "p0_qguard", "state_identity", "pack_epi_degree",
              "pack_epi_qguard", "pair_i_degree", "pair_i_qguard",
              "b256_fast", "bfull_fast", "paper_tfs", "p0_m256")
KINDS = {"consecutive": 5, "alternating": 10}
EQ_BOUNDARIES = {"kernel_FP32", "e2e_FP32", "H1_BF16", "e2e_BF16"}
CHECK_PAIRS = {("kernel_FP32", "paper_tfs"),
               ("e2e_FP32_final", "paper_tfs"),
               ("e2e_BF16_final", "paper_tfs"),
               ("e2e_BF16_final", "source_MKL_FP32")}
INPUT_HASHES = {}


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def read_json(path, default=None):
    if not path.is_file():
        return {} if default is None else default
    raw = path.read_bytes()
    INPUT_HASHES[str(path.resolve())] = hashlib.sha256(raw).hexdigest()
    return json.loads(raw.decode("utf-8-sig"))


def read_text(path):
    if not path.is_file():
        return ""
    raw = path.read_bytes()
    INPUT_HASHES[str(path.resolve())] = hashlib.sha256(raw).hexdigest()
    return raw.decode("utf-8", errors="replace")


def as_int(value):
    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def as_float(value):
    try:
        f = float(value)
        return f if math.isfinite(f) else None
    except (TypeError, ValueError):
        return None


def truth(value):
    if value in (True, 1, "1", "true", "True"):
        return True
    if value in (False, 0, "0", "false", "False"):
        return False
    return None


def gm(values):
    values = list(values)
    if not values:
        return None
    if any(v <= 0 or not math.isfinite(v) for v in values):
        raise ValueError("GM requires finite positive values")
    return math.exp(statistics.mean(math.log(v) for v in values))


def write_csv(path, rows, initial_fields):
    fields = list(initial_fields)
    for row in rows:
        for k in row:
            if k not in fields:
                fields.append(k)
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)


def freeze_check(rows, boundaries, expected_method=None):
    return (len(rows) == len(boundaries)
            and {r.get("boundary") for r in rows} == boundaries
            and all(r.get("bitwise") == "1" and r.get("pass") == "1"
                    and (expected_method is None or r.get("method") == expected_method)
                    for r in rows))


def numerical_ok(rows, accurate, method):
    if len(rows) != 4 or {(r.get("boundary"), r.get("reference")) for r in rows} != CHECK_PAIRS:
        return False
    for r in rows:
        if r.get("reference") == "source_MKL_FP32":
            gate = .03
        elif r.get("boundary") == "e2e_BF16_final":
            gate = .01 if accurate else .02
        else:
            gate = .001 if accurate else .01
        actual_gate = as_float(r.get("gate"))
        l2, nmax = as_float(r.get("relative_L2")), as_float(r.get("normalized_max"))
        ma, mean = as_float(r.get("max_abs")), as_float(r.get("mean_abs"))
        if (r.get("pass") != "1" or actual_gate is None
                or abs(actual_gate - gate) > 1e-12
                or any(v is None or v < 0 for v in (l2, nmax, ma, mean))
                or l2 >= gate or nmax >= gate):
            return False
        if method == "paper_tfs" and r.get("reference") == "paper_tfs" and ma != 0:
            return False
    return True


def load_packets(root):
    manifest = read_json(root / "manifest.json")
    source_hashes = read_text(root / "source.sha256")
    build_fingerprint = hashlib.sha256(source_hashes.encode("utf-8")).hexdigest()
    launcher = {}
    for p in sorted((root / "launcher").glob("*/event.json")):
        event = read_json(p)
        jid = str(event.get("job", "")).split(";")[0]
        if not jid:
            continue
        cmd = read_json(p.parent / "command.json", [])
        launcher[jid] = (event, cmd, str(p))
    inventory = read_json(root.parent.parent / "docs" / "CURRENT_GRAPH_INVENTORY_20261004.json", [])
    expected_hashes = {r["graph"]: r.get("input_sha256", "") for r in inventory}
    folders = {p.parent for p in (root / "logs").glob("*/*/parsed.json")}
    folders.update(p.parent for p in (root / "logs").glob("*/*/status.json"))
    packets = []
    for folder in sorted(folders):
        jobdir = folder.parent
        event = read_json(jobdir / "event.json")
        completion = read_json(jobdir / "completion.json")
        env = read_json(jobdir / "environment.json")
        affinity = read_json(jobdir / "affinity.json")
        status = read_json(folder / "status.json")
        parsed = read_json(folder / "parsed.json")
        match = re.search(r"(\d+)$", jobdir.name)
        jid = str(event.get("job") or env.get("SLURM_JOB_ID") or
                  (match.group(1) if match else jobdir.name)).split(";")[0]
        launch, command, launch_path = launcher.get(jid, ({}, [], ""))
        partition = str(event.get("partition") or launch.get("partition") or
                        env.get("SLURM_JOB_PARTITION") or "unknown")
        exclusive = truth(launch.get("exclusive"))
        exclusive_source = "launcher_event"
        if exclusive is None and command:
            exclusive = "--exclusive" in command
            exclusive_source = "launcher_command"
        if exclusive is None:
            exclusive = truth(event.get("exclusive"))
            exclusive_source = "job_event"
        if exclusive is None and partition == "intel_expr":
            exclusive, exclusive_source = True, "dedicated_partition"
        exclusive_token = "unknown" if exclusive is None else str(exclusive).lower()
        mode = str(event.get("mode") or launch.get("mode") or completion.get("mode") or "")
        graph = str(status.get("graph") or folder.name)
        configs = parsed.get("STATE_CONFIG", [])
        smoke = mode == "smoke" or any(r.get("smoke") == "1" for r in configs)
        node = str(event.get("node") or read_text(jobdir / "node.txt").strip())
        job_status = str(event.get("status") or ("PASS" if completion and
                         completion.get("failed") == 0 else "IN_PROGRESS"))
        packets.append(dict(root=str(root), folder=str(folder), jobdir=str(jobdir),
            job=jid, node=node, partition=partition, exclusive=exclusive_token,
            exclusive_source=exclusive_source, launcher_evidence=launch_path,
            build_fingerprint=build_fingerprint, graph=graph, mode=mode,
            data_class="smoke" if smoke else ("real" if graph in ALL17 else "other"),
            graphset="diag4" if graph in DIAG4 else ("rest13" if graph in REST13 else "smoke_or_other"),
            status=status, parsed=parsed, env=env, affinity=affinity,
            completion=completion, event=event, job_status=job_status,
            expected_input_sha256=expected_hashes.get(graph, ""),
            input_sha256=str(status.get("actual_input_sha256") or status.get("input_sha256") or ""),
            manifest=manifest))
    # Earliest observed attempt, even a failed one, is primary. Reruns are not
    # promoted to primary merely because they succeed.
    groups = collections.defaultdict(list)
    for p in packets:
        groups[(p["partition"], p["exclusive"], p["data_class"], p["graph"],
                p["build_fingerprint"])].append(p)
    for group in groups.values():
        group.sort(key=lambda p: (int(p["job"]) if p["job"].isdigit() else 10**30, p["folder"]))
        for i, p in enumerate(group):
            p["attempt_role"] = "primary" if i == 0 else "retest"
    return packets


def context(p):
    return {k: p[k] for k in ("root", "folder", "job", "node", "partition",
            "exclusive", "attempt_role", "data_class", "graphset", "graph",
            "build_fingerprint", "input_sha256", "job_status")}


def audit_packet(p):
    t = p["parsed"]
    errors, warnings = [], []
    method_audits = {}
    cfgs, plans = t.get("STATE_CONFIG", []), t.get("STATE_PLAN", [])
    cfg = cfgs[0] if len(cfgs) == 1 else {}
    def require(condition, message):
        if not condition:
            errors.append(message)
    require(p["status"].get("status") == "PASS", "graph_status_not_PASS")
    require(p["job_status"] in ("PASS", "IN_PROGRESS"), "job_status_not_eligible")
    if p["job_status"] == "IN_PROGRESS":
        warnings.append("completed_graph_used_provisionally_final_job_status_pending")
    require(len(cfgs) == 1, "STATE_CONFIG_cardinality")
    require(all(cfg.get(k) == v for k, v in {
        "D": "128", "H": "128", "F": "128", "S": "64", "R": "64", "TR": "16",
        "warmups": "1", "consecutive": "5", "alternating": "10",
        "numa": "default", "final_output": "BF16"}.items()), "frozen_config_mismatch")
    smoke = p["data_class"] == "smoke"
    full = smoke or p["graph"] in DIAG4
    expected_names = set(BASE) | (set(DIAG_ONLY) if full else set())
    plan_map = {r.get("method", ""): r for r in plans}
    require(len(plans) == len(plan_map) == as_int(cfg.get("methods")), "STATE_PLAN_cardinality")
    require(set(plan_map) == expected_names, "frozen_candidate_matrix_mismatch")
    require(cfg.get("ablations") == ("1" if full else "0"), "ablation_scope_mismatch")
    require(cfg.get("smoke") == ("1" if smoke else "0"), "smoke_config_mismatch")
    for label in ("STATE_COMPLETE", "STATE_GATE_COMPLETE", "STATE_SEMANTIC"):
        rr = t.get(label, [])
        require(len(rr) == 1 and rr[0].get("pass") == "1", label + "_missing_duplicate_or_failed")
    done = t.get("STATE_COMPLETE", [])
    require(len(done) == 1 and done[0].get("methods") == cfg.get("methods") and
            done[0].get("smoke") == cfg.get("smoke"), "completion_config_mismatch")
    sem = t.get("STATE_SEMANTIC", [])
    require(len(sem) == 1 and sem[0].get("bitwise") == "1" and sem[0].get("values") == "19",
            "semantic_fixture_coverage")
    for label, rows in t.items():
        if not isinstance(rows, list):
            errors.append("table_not_list:" + label)
            continue
        require(all(r.get("graph", p["graph"]) == p["graph"] for r in rows), "cross_graph_record:" + label)
    if p["data_class"] == "real":
        require(bool(p["input_sha256"]), "missing_real_input_hash")
        if p["expected_input_sha256"]:
            require(p["input_sha256"] == p["expected_input_sha256"], "real_input_hash_mismatch")
    aff = p["affinity"]
    topo = aff.get("topology", [])
    require(aff.get("threads") == 32 and len(aff.get("cpus", [])) == 32 and
            len({r.get("socket") for r in topo}) == 1 and
            len({(r.get("socket"), r.get("core")) for r in topo}) == 32,
            "32_physical_cores_one_socket_not_verified")
    require(p["env"].get("OMP_NUM_THREADS") == "32" and
            p["env"].get("OMP_PROC_BIND") == "close" and
            p["env"].get("OMP_PLACES") == "cores", "OpenMP_binding_mismatch")
    require(all(p["env"].get(k) in (None, "") for k in
                ("MKL_NUM_THREADS", "MKL_DYNAMIC", "MKL_DOMAIN_NUM_THREADS")),
            "MKL_environment_override")
    if p["exclusive"] == "unknown":
        warnings.append("allocation_exclusivity_unknown_not_pooled_with_shared_or_exclusive")
    E = as_int(cfg.get("E"))
    require(E is not None and E >= 0, "invalid_edge_count")
    structures = t.get("STATE_STRUCTURE", [])
    smap = {(r.get("schedule"), r.get("scope")): r for r in structures}
    require(len(structures) == len(smap) == 6 and set(smap) ==
            {(s, m) for s in ("degree", "qguard") for m in ("0", "64", "256")},
            "structure_matrix_missing_or_duplicate")
    for key, r in smap.items():
        e, c, scopes = as_int(r.get("edges")), as_int(r.get("shared_heads")), as_int(r.get("projection_scopes"))
        ok = e == E and e is not None and c is not None and 0 <= c <= e // 2
        ok = ok and as_int(r.get("merge_steps")) == e-c and as_int(r.get("shared_decode_rows")) == e-c
        ok = ok and as_int(r.get("scalar_adds")) == e*128 and scopes is not None and scopes >= 0
        ok = ok and as_int(r.get("amx_tdp_fast")) == scopes*32
        require(ok, "invalid_structure:" + "/".join(key))
        st, ld, saved = [as_int(r.get(k)) for k in
                        ("baseline_state_store_bytes", "baseline_state_load_bytes", "direct_saved_terminal_bytes")]
        if None not in (st, ld, scopes, saved):
            exact_saved = st-ld + scopes*8192
            if saved != exact_saved:
                warnings.append("terminal_byte_estimate_excludes_padded_FP32_loads:" +
                                "/".join(key) + ":reported=" + str(saved) + ":exact=" + str(exact_saved))
    for scope in ("0", "64", "256"):
        a, b = smap.get(("degree", scope)), smap.get(("qguard", scope))
        if a and b:
            invariants = ("edges", "projection_scopes", "windows", "scalar_adds",
                          "amx_tdp_fast", "baseline_partial_zero_bytes",
                          "baseline_state_load_bytes", "baseline_state_store_bytes", "prefetch_requests")
            require(all(a.get(k) == b.get(k) for k in invariants), "qguard_changed_work_budget:" + scope)
    for name, plan in plan_map.items():
        reasons = []
        def method_require(ok, why):
            if not ok:
                reasons.append(why)
        kind, accurate = as_int(plan.get("kind")), plan.get("accurate") == "1"
        method_require(kind in (0, 1, 2, 3) and not accurate, "main_method_kind_or_precision_changed")
        cr = [r for r in t.get("METHOD_CHECK", []) if r.get("method") == name]
        method_require(numerical_ok(cr, accurate, name), "four_numerical_gates")
        if kind is not None and kind >= 2:
            eq = [r for r in t.get("STATE_EQUIV", []) if r.get("method") == name]
            method_require(freeze_check(eq, EQ_BOUNDARIES, name), "four_bitwise_boundaries")
            method_require(all(r.get("scope") == plan.get("scope") and
                               r.get("accurate") == plan.get("accurate") for r in eq),
                           "equivalence_scope_precision_mismatch")
        if kind == 3:
            for label in ("STATE_COUNTERS", "STATE_WORK_GATE", "STATE_PROFILE_GATE"):
                rr = [r for r in t.get(label, []) if r.get("method") == name]
                method_require(len(rr) == 2 and {r.get("layer") for r in rr} == {"1", "2"},
                               label + "_requires_exactly_two_layers")
                if label != "STATE_COUNTERS":
                    method_require(all(r.get("pass") == "1" and
                                   (label != "STATE_PROFILE_GATE" or r.get("bitwise") == "1") for r in rr),
                                   label + "_failed")
            w = smap.get((plan.get("schedule"), plan.get("scope")), {})
            for c in [r for r in t.get("STATE_COUNTERS", []) if r.get("method") == name]:
                expected_decode = w.get("shared_decode_rows") if plan.get("pair") == "2" else w.get("edges")
                expected_heads = w.get("shared_heads") if as_int(plan.get("pair")) in (1, 2) else "0"
                checks = (
                    c.get("source_visits") == w.get("edges"),
                    c.get("feature_adds") == w.get("scalar_adds"),
                    c.get("amx_tdp") == w.get("amx_tdp_fast"),
                    c.get("projection_scopes") == w.get("projection_scopes"),
                    c.get("prefetch_requests") == w.get("prefetch_requests"),
                    c.get("decode_rows") == expected_decode,
                    c.get("shared_heads") == expected_heads,
                    c.get("csr_reads") == w.get("edges"))
                method_require(all(checks), "counter_vs_structure_layer_" + str(c.get("layer")))
                # Extra state traffic must agree with the declared DIRECTPACK mode.
                if plan.get("direct_pack") == "1":
                    method_require(as_int(c.get("packing_load_bytes")) == 0, "direct_pack_retained_pack_reload")
                    oldstores = as_int(w.get("baseline_state_store_bytes"))
                    direct_rows = as_int(c.get("direct_rows"))
                    newstores = as_int(c.get("partial_store_bytes"))
                    method_require(None not in (oldstores, direct_rows, newstores) and
                                   newstores == oldstores-direct_rows*512, "direct_pack_store_delta")
                else:
                    method_require(c.get("partial_store_bytes") == w.get("baseline_state_store_bytes")
                                   and as_int(c.get("packing_load_bytes")) ==
                                   (as_int(w.get("projection_scopes")) or 0)*8192,
                                   "ordinary_pack_state_traffic")
                method_require(c.get("partial_load_bytes") == w.get("baseline_state_load_bytes"),
                               "intermediate_state_load_changed")
        method_audits[name] = dict(valid=not reasons, reasons=reasons, plan=plan,
                                  numerical_checks=cr)
        errors.extend(name + ":" + reason for reason in reasons)
    # Explicit per-name coverage, not only a total that duplicates could satisfy.
    if smoke:
        expected_extra = {"smoke_m%d_a%d_v%d" % (m, a, v)
                          for m in (64, 256, 0) for a in (0, 1) for v in range(7)}
        expected_stress = {"stress_m%d_a%d_p%d" % (m, a, v)
                           for m in (64, 256, 0) for a in (0, 1) for v in range(3)}
        for prefix, expected in (("smoke_", expected_extra), ("stress_", expected_stress)):
            rr = [r for r in t.get("STATE_EQUIV", []) if r.get("method", "").startswith(prefix)]
            require(len(rr) == len(expected)*4 and {r.get("method") for r in rr} == expected,
                    prefix + "matrix_coverage")
            for name in expected:
                sub = [r for r in rr if r.get("method") == name]
                require(freeze_check(sub, EQ_BOUNDARIES, name), "extra_bitwise:" + name)
        for name in expected_extra:
            cr = [r for r in t.get("METHOD_CHECK", []) if r.get("method") == name]
            if "_a1_" in name:
                require(numerical_ok(cr, True, name), "accurate_smoke_numerical:" + name)
            elif cr:
                require(numerical_ok(cr, False, name), "fast_smoke_numerical:" + name)
        fast_numeric = {r.get("method") for r in t.get("METHOD_CHECK", [])
                        if r.get("method") in expected_extra and "_a0_" in r.get("method", "")}
        if len(fast_numeric) != 21:
            warnings.append("extra_FAST_smoke_has_bitwise_coverage_but_not_all_individual_numerical_records")
    known_eq = {r.get("method") for r in t.get("STATE_EQUIV", [])}
    allowed_eq = {n for n, r in plan_map.items() if as_int(r.get("kind")) in (2, 3)}
    if smoke:
        allowed_eq |= expected_extra | expected_stress
    require(known_eq == allowed_eq, "unexpected_or_missing_equivalence_methods")
    for label in ("METHOD_CHECK", "STATE_EQUIV", "STATE_PROFILE_GATE", "STATE_WORK_GATE"):
        require(all(r.get("pass") == "1" for r in t.get(label, [])), "failed_record:" + label)
    return dict(**context(p), valid=not errors, errors=errors, warnings=warnings,
                methods=method_audits, structure=structures,
                counters=t.get("STATE_COUNTERS", []), work_gates=t.get("STATE_WORK_GATE", []),
                semantics=t.get("STATE_SEMANTIC", []), setup=t.get("STATE_SETUP", []),
                grouping_setup=t.get("GROUP_SETUP", []),
                raw_time_records=t.get("STATE_TIME", []),
                graph_status=p["status"], job_completion=p["completion"],
                exclusive_source=p["exclusive_source"], launcher_evidence=p["launcher_evidence"])


def summarize_packets(packets, audits):
    rows, ratios, paired = [], [], []
    lookup = {}
    packet_lookup = {p["folder"]: p for p in packets}
    for audit in audits:
        p = packet_lookup[audit["folder"]]
        plan_records = p["parsed"].get("STATE_PLAN", [])
        plans = {r.get("method"): r for r in plan_records}
        plan_positions = {r.get("method"): i for i, r in enumerate(plan_records)}
        plan_positions[ANCHOR] = len(plan_records)
        by = collections.defaultdict(list)
        for r in p["parsed"].get("STATE_TIME", []):
            by[(r.get("method", ""), r.get("kind", ""))].append(r)
        if p["data_class"] == "smoke":
            if by:
                audit["errors"].append("smoke_unexpected_STATE_TIME_records")
                audit["valid"] = False
            continue
        expected_keys = {(m, k) for m in set(plans) | {ANCHOR} for k in KINDS}
        if set(by) != expected_keys:
            audit["errors"].append("timing_matrix_missing_or_extra")
            audit["valid"] = False
        for method, kind in sorted(expected_keys | set(by)):
            rr = by.get((method, kind), [])
            vals = [as_float(r.get("ms")) for r in rr]
            repeats = [as_int(r.get("repeat")) for r in rr]
            expected_count = KINDS.get(kind, -1)
            valid_time = (len(rr) == expected_count and set(repeats) == set(range(max(0, expected_count)))
                          and all(v is not None and v > 0 for v in vals))
            if valid_time:
                position = plan_positions.get(method)
                valid_time = position is not None and all(
                    as_int(r.get("order")) == (len(plans)-position
                    if kind == "alternating" and int(r["repeat"]) % 2 else position)
                    for r in rr)
            if not valid_time:
                audit["errors"].append("timing_invalid:" + method + ":" + kind)
                audit["valid"] = False
            plan = plans.get(method, {})
            finite_values = [v for v in vals if v is not None and v > 0]
            row = dict(**context(p), method=method, timing_kind=kind,
                method_kind=plan.get("kind", "FP32_anchor"), scope=plan.get("scope", ""),
                schedule=plan.get("schedule", "none"), accurate=plan.get("accurate", ""),
                precision="FP32_master_input_interlayer" if method == ANCHOR else "BF16_input_output_FAST",
                precision_matched=int(method != ANCHOR), samples=len(rr),
                timing_valid=int(valid_time), eligible_for_performance=0,
                median_ms=statistics.median(finite_values) if finite_values else None,
                min_ms=min(finite_values) if finite_values else None,
                max_ms=max(finite_values) if finite_values else None,
                cv=statistics.pstdev(finite_values)/statistics.mean(finite_values) if finite_values else None,
                graph_audit_pass=0, reason="",
                speedup_vs_p0_degree=None, speedup_vs_same_schedule_p0=None)
            rows.append(row)
            lookup[(p["folder"], method, kind)] = (row, rr)
    # Set eligibility only after ALL candidate and timing checks for a graph.
    audmap = {a["folder"]: a for a in audits}
    for row in rows:
        audit = audmap[row["folder"]]
        row["graph_audit_pass"] = int(audit["valid"])
        row["eligible_for_performance"] = int(audit["valid"] and row["timing_valid"])
        row["reason"] = ";".join(audit["errors"])
    # Every denominator is selected from the same graph directory/job and kind.
    for row in rows:
        if row["method"] == ANCHOR:
            continue
        p = packet_lookup[row["folder"]]
        plan = next((r for r in p["parsed"].get("STATE_PLAN", []) if r.get("method") == row["method"]), {})
        for refname in REFERENCES:
            ref_pair = lookup.get((row["folder"], refname, row["timing_kind"]))
            if ref_pair is None:
                continue
            ref, ref_raw = ref_pair
            valid = bool(row["eligible_for_performance"] and ref["eligible_for_performance"])
            same_precision = row["precision"] == ref["precision"] and row["accurate"] == ref["accurate"]
            valid = valid and same_precision
            value = ref["median_ms"]/row["median_ms"] if valid else None
            rr = dict(**context(p), method=row["method"], reference=refname,
                timing_kind=row["timing_kind"], eligible=int(valid),
                precision_matched=int(same_precision), same_scope=int(row["scope"] == ref["scope"]),
                candidate_scope=row["scope"], reference_scope=ref["scope"],
                candidate_schedule=row["schedule"], reference_schedule=ref["schedule"],
                candidate_median_ms=row["median_ms"], reference_median_ms=ref["median_ms"],
                candidate_min_ms=row["min_ms"], candidate_max_ms=row["max_ms"],
                reference_min_ms=ref["min_ms"], reference_max_ms=ref["max_ms"],
                speedup_ratio_of_medians=value, alternating_repeat_gm_diagnostic=None,
                losses_are_retained=1, denominator_same_graph_job_protocol=1)
            if value is not None and refname == "p0_degree":
                row["speedup_vs_p0_degree"] = value
            same_ref = "p0_qguard" if row["schedule"] == "qguard" else (
                       "p0_m256" if row["scope"] == "256" else "p0_degree")
            if value is not None and refname == same_ref:
                row["speedup_vs_same_schedule_p0"] = value
            if valid and row["timing_kind"] == "alternating":
                cand_raw = lookup[(row["folder"], row["method"], row["timing_kind"])][1]
                ca = {int(r["repeat"]): float(r["ms"]) for r in cand_raw}
                reft = {int(r["repeat"]): float(r["ms"]) for r in ref_raw}
                rv = []
                for rep in sorted(ca):
                    v = reft[rep]/ca[rep]
                    rv.append(v)
                    paired.append(dict(**context(p), method=row["method"], reference=refname,
                        timing_kind="alternating", repeat=rep, pair=rep//2,
                        orientation="reverse" if rep%2 else "forward",
                        candidate_ms=ca[rep], reference_ms=reft[rep], speedup=v,
                        diagnostic_only=1))
                rr["alternating_repeat_gm_diagnostic"] = gm(rv)
            ratios.append(rr)
    return rows, ratios, paired


def cohort_summaries(ratios, audits):
    # Repeated jobs contribute one graph weight within a cohort/attempt role.
    # Ratios remain same-job before this aggregation.
    groups = collections.defaultdict(list)
    for r in ratios:
        if r["eligible"] and r["data_class"] == "real":
            key = (r["partition"], r["exclusive"], r["attempt_role"],
                   r["build_fingerprint"], r["timing_kind"], r["method"], r["reference"])
            groups[(key, r["graph"])].append(r)
    collapsed = {}
    for (key, graph), rr in groups.items():
        collapsed[(key, graph)] = dict(
            speedup=gm(r["speedup_ratio_of_medians"] for r in rr),
            job_ids=sorted({r["job"] for r in rr}),
            pending_job_ids=sorted({r["job"] for r in rr if r["job_status"] != "PASS"}),
            job_observations=len(rr))
    keys = sorted({k for k, _ in groups})
    result = []
    for key in keys:
        partition, exclusive, role, build, kind, method, reference = key
        for setname, target in GRAPHSETS.items():
            present = sorted(g for g in target if (key, g) in collapsed)
            missing = sorted(set(target)-set(present))
            values = [collapsed[(key, g)]["speedup"] for g in present]
            inherent_diag = method in DIAG_ONLY or reference in DIAG_ONLY
            result.append(dict(partition=partition, exclusive=exclusive, attempt_role=role,
                build_fingerprint=build, timing_kind=kind, method=method, reference=reference,
                graphset=setname, expected_graph_count=len(target), graph_count=len(present),
                graphs=present, missing_graphs=missing, complete_graphset=not missing,
                comparison_has_diagnostic_only_method=inherent_diag,
                gm_available_graphs=gm(values),
                gm_complete_graphset=gm(values) if not missing else None,
                wins=sum(v>1 for v in values), losses=sum(v<1 for v in values),
                ties=sum(v==1 for v in values),
                per_graph={g:collapsed[(key,g)] for g in present},
                estimator="GM of same-job ratios of medians; repeated jobs collapsed within graph",
                available_GM_is_partial=bool(missing)))
    coverage = []
    allocations = sorted({(a["partition"], a["exclusive"], a["attempt_role"])
                          for a in audits if a["data_class"] == "real"})
    for partition, exclusive, role in allocations:
        aa = [a for a in audits if (a["partition"],a["exclusive"],a["attempt_role"]) ==
              (partition,exclusive,role) and a["data_class"] == "real"]
        for setname,target in GRAPHSETS.items():
            eligible = sorted({a["graph"] for a in aa if a["valid"] and a["graph"] in target})
            failed = sorted({a["graph"] for a in aa if not a["valid"] and a["graph"] in target})
            coverage.append(dict(partition=partition,exclusive=exclusive,attempt_role=role,
                graphset=setname,expected_graph_count=len(target),eligible_graphs=eligible,
                failed_graphs=failed,missing_graphs=sorted(set(target)-set(eligible)),
                complete=len(eligible)==len(target)))
    return dict(coverage=coverage,comparisons=result,
                estimator="Only ratio-of-medians enters primary GM; each graph has equal weight.",
                graphsets={k:list(v) for k,v in GRAPHSETS.items()},
                no_best_candidate_selection=True)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("runPath",type=pathlib.Path)
    ap.add_argument("outputDir",type=pathlib.Path)
    args=ap.parse_args()
    root=args.runPath.resolve();out=args.outputDir.resolve();ws=WORKSPACE.resolve()
    if not root.is_dir():
        ap.error("runPath must exist")
    if os.path.commonpath([str(ws),str(out)]) != str(ws) or out == ws:
        ap.error("outputDir must be a new directory under the authorized wzh workspace")
    if out.exists():
        ap.error("outputDir already exists; previous analyses are never overwritten")
    packets=load_packets(root)
    if not packets:
        ap.error("no per-graph parsed.json/status.json evidence under logs/*/<graph>")
    audits=[]
    for p in packets:
        try:
            audits.append(audit_packet(p))
        except Exception as exc:
            audits.append(dict(**context(p),valid=False,errors=["audit_parse_exception:"+repr(exc)],
                warnings=[],methods={},raw_time_records=p["parsed"].get("STATE_TIME",[]),
                graph_status=p["status"],job_completion=p["completion"]))
    rows,ratios,paired=summarize_packets(packets,audits)
    cohorts=cohort_summaries(ratios,audits)
    # The source is recorded as an input; no mutation of run evidence is performed.
    INPUT_HASHES[str(pathlib.Path(__file__).resolve())]=digest(pathlib.Path(__file__))
    audit=dict(run=str(root),analysis_source=str(pathlib.Path(__file__).resolve()),
        analysis_pass=all(a["valid"] for a in audits),
        graph_job_packets=len(audits),passed_graph_jobs=sum(a["valid"] for a in audits),
        failed_graph_jobs=sum(not a["valid"] for a in audits),
        performance_records=sum(r["eligible_for_performance"] for r in rows),
        warnings_are_not_failures=True,graph_audits=audits,
        alternating_repeat_diagnostics=paired,input_sha256=INPUT_HASHES,
        interpretation=[
            "All primary ratios use the same graph, job/process, precision and timing protocol.",
            "consecutive and alternating timings remain separate; paired repeats are diagnostic.",
            "FP32 source MKL is a different-precision anchor, excluded from matched-speedup ratios and GM.",
            "Failed/incomplete graphs and missing counter/work gate layers are excluded, with raw records retained.",
            "Negative speedups and min/max are retained; there is no posthoc per-graph winner selection.",
            "Partition and allocation exclusivity are separate: intel --exclusive is not shared intel.",
            "Primary/retest cohorts are separate; repeated graph jobs collapse before each graph receives one weight.",
            "An all17/diag4/rest13 GM is complete only when all named graphs are present and eligible.",
            "Source-level counted loads/decodes and analytic requests are not retired instructions or measured DRAM traffic.",
            "Layer1 profile counters use the FP32 diagnostic path; they are not a complete fused-epilogue E2E byte count.",
            "Prepared forward includes the production layer/activation/interlayer/output boundary; structural setup is excluded and retained separately.",
            "Identity controls new-kernel restructuring; improvements must also be compared against frozen P0.",
            "The caller must record analysis preflight and synchronize the three handoff documents."])
    out.mkdir(parents=True,exist_ok=False)
    write_csv(out/"summary.csv",rows,["graph","job","partition","exclusive","attempt_role",
        "method","timing_kind","eligible_for_performance","median_ms","min_ms","max_ms"])
    write_csv(out/"ratios.csv",ratios,["graph","job","partition","exclusive","attempt_role",
        "method","reference","timing_kind","eligible","speedup_ratio_of_medians"])
    (out/"audit.json").write_text(json.dumps(audit,indent=2,ensure_ascii=False,allow_nan=False)+"\n",encoding="utf-8")
    (out/"cohort_summary.json").write_text(json.dumps(cohorts,indent=2,ensure_ascii=False,allow_nan=False)+"\n",encoding="utf-8")
    print(json.dumps(dict(output=str(out),analysis_pass=audit["analysis_pass"],
          passed=audit["passed_graph_jobs"],failed=audit["failed_graph_jobs"],
          performance_records=audit["performance_records"]),ensure_ascii=False))
    return 0 if audit["analysis_pass"] else 2


if __name__=="__main__":
    sys.exit(main())
