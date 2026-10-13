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
    re