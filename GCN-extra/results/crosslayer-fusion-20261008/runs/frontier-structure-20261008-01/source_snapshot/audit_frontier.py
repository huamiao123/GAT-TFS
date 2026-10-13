"""Independent cursor simulation for frontier-structure-v1; no timing claims.

The oracle advances real per-row CSR cursors through produced source flags.
It does not use prefix maxima or interval-end formulae. Empty rows are zero
complete at rank 0 and excluded from projection-ready batches (no bias).
"""
import argparse
import hashlib
import json
import math
import pathlib
import random
import struct
import subprocess
import sys
import traceback


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def simulate(rows, order, batch_rows=16, checkpoint=1, row_complete=False):
    n = len(rows)
    require(sorted(order) == list(range(n)), "not a permutation")
    require(batch_rows > 0 and checkpoint >= 0, "invalid scheduling parameter")
    require(all(0 <= j < n for row in rows for j in row), "source out of range")
    produced = [False] * n
    held = [False] * n
    cursor = [0] * n
    remaining = [0] * n
    for row in rows:
        for j in row:
            remaining[j] += 1
    first = [0] * n
    last = [0] * n
    rank = [0] * n
    release = [0] * n
    segments = [0] * n
    completion = [0] * n
    skipped = sum(not row for row in rows)
    completed = skipped
    full_groups = ready = edges = cumulative_segments = resumes = 0
    batches = []
    total_batches = (n + batch_rows - 1) // batch_rows

    def snapshot():
        sources = sum(held)
        partials = sum(0 < cursor[i] < len(row) for i, row in enumerate(rows))
        return dict(source_rows=sources, partial_rows=partials, ready_rows=ready,
                    bytes=256 * sources + 512 * partials + 256 * ready)

    for p in range(1, total_batches + 1):
        before = snapshot()
        for j in order[(p - 1) * batch_rows:p * batch_rows]:
            require(not produced[j], "source produced twice")
            produced[j] = True
            rank[j] = p
            held[j] = remaining[j] > 0
            # Schema uses last_use=0 for a source with no consumers.
        after_produce = snapshot()
        drain = row_complete or p == total_batches or (checkpoint > 0 and p % checkpoint == 0)
        if drain:
            for i, row in enumerate(rows):
                if cursor[i] == len(row):
                    continue
                if row_complete and not all(produced[j] for j in row):
                    continue
                old = cursor[i]
                while cursor[i] < len(row) and produced[row[cursor[i]]]:
                    j = row[cursor[i]]
                    require(held[j] and remaining[j] > 0, "read source after release")
                    remaining[j] -= 1
                    if remaining[j] == 0:
                        held[j] = False
                        release[j] = p
                    cursor[i] += 1
                    edges += 1
                if cursor[i] > old:
                    segments[i] += 1
                    cumulative_segments += 1
                    if old:
                        resumes += 1
                    else:
                        first[i] = p
                    last[i] = p
                    if cursor[i] == len(row):
                        completed += 1
                        completion[i] = p
                        ready += 1
                        if ready == 16:
                            full_groups += 1
                            ready = 0
        after_drain = snapshot()
        require(0 <= ready < 16, "ready tail not drained at 16")
        require(edges + sum(remaining) == sum(map(len, rows)), "edge conservation")
        require(full_groups * 16 + ready == completed - skipped, "ready conservation")
        require(cumulative_segments - (sum(x > 0 for x in cursor)) == resumes,
                "resume/segment conservation")
        batches.append(dict(p=p, before_batch=before, after_produce=after_produce,
            after_drain=after_drain, cumulative_edges_consumed=edges,
            cumulative_segments=cumulative_segments, cumulative_resumes=resumes,
            cumulative_completed_rows=completed-skipped, full_output_groups=full_groups,
            ready_tail_rows=ready))
    require(all(produced) and all(x == 0 for x in remaining), "unfinished sources/edges")
    require(all(cursor[i] == len(row) for i, row in enumerate(rows)), "unfinished rows")
    require(not any(held), "source retained after final drain")
    require(completed == n and sum(segments) - (n - skipped) == resumes,
            "completion conservation")
    if row_complete:
        require(all(x in (0, 1) for x in segments), "row_complete resumed")
    return dict(rank=rank, source_last_use_rank=release, first_consume_rank=first,
                last_consume_rank=last, row_completion_rank=completion,
                segments_per_row=segments, batches=batches,
                final_full_output_groups=full_groups, final_ready_tail_rows=ready,
                final_tail_flush_groups=int(ready > 0), final_tail_padding_rows=(16 - ready) % 16,
                rows_skipped_zero=skipped, nonempty_rows=n-skipped,
                logical_edges_consumed=edges, total_segments=cumulative_segments,
                total_resumes=resumes)


def equal(actual, expected, path):
    require(type(actual) is type(expected), f"{path}: wrong type {type(actual).__name__}, expected {type(expected).__name__}")
    if isinstance(expected, dict):
        for key, value in expected.items():
            require(key in actual, f"{path}: missing {key}")
            equal(actual[key], value, f"{path}.{key}")
    elif isinstance(expected, list):
        require(len(actual) == len(expected), f"{path}: length {len(actual)} != {len(expected)}")
        for k, value in enumerate(expected):
            equal(actual[k], value, f"{path}[{k}]")
    else:
        require(actual == expected, f"{path}: {actual!r} != {expected!r}")


def summary_from_simulation(reference):
    records = reference["batches"]
    phases = ("before_batch", "after_produce", "after_drain")
    all_states = [rec[phase] for rec in records for phase in phases]
    tail = reference["final_ready_tail_rows"]
    groups = reference["final_full_output_groups"]
    resumes = reference["total_resumes"]
    source_area = sum(rec["after_drain"]["source_rows"] for rec in records)
    partial_area = sum(rec["after_drain"]["partial_rows"] for rec in records)
    # Areas count inter-batch intervals only. Final ready tail is flushed at B
    # and has no [B,B+1) residence, although after_drain[B] still records it.
    ready_area = sum(rec["after_drain"]["ready_rows"] for rec in records[:-1])
    result = dict(logical_edges_consumed=reference["logical_edges_consumed"],
        segments=reference["total_segments"], resumes=resumes,
        nonempty_rows_completed=reference["nonempty_rows"],
        partial_stores=resumes, partial_loads=resumes,
        logical_partial_store_bytes=512*resumes, logical_partial_load_bytes=512*resumes,
        logical_source_operand_bytes=256*reference["logical_edges_consumed"],
        full_output_groups=groups, final_tail_rows=tail, final_tail_groups=int(tail > 0),
        projection_accounted_rows=16*(groups+int(tail > 0)),
        peak_boundary_source_rows=max((x["source_rows"] for x in all_states), default=0),
        peak_boundary_partial_rows=max((x["partial_rows"] for x in all_states), default=0),
        peak_boundary_ready_rows=max((x["ready_rows"] for x in all_states), default=0),
        peak_all_boundaries_bytes=max((x["bytes"] for x in all_states), default=0),
        source_row_batch_area=source_area, partial_row_batch_area=partial_area,
        ready_row_batch_area=ready_area,
        postdrain_byte_batch_area=256*source_area+512*partial_area+256*ready_area,
        post_final_flush_state=dict(source_rows=0, partial_rows=0, ready_rows=0, bytes=0))
    for phase in phases:
        maximum = max((rec[phase]["bytes"] for rec in records), default=0)
        result[f"peak_{phase}_bytes"] = maximum
        result[f"peak_{phase}_rank"] = next((rec["p"] for rec in records
            if maximum > 0 and rec[phase]["bytes"] == maximum), 0)
    return result


def audit_native(data, rows, label):
    n, e = len(rows), sum(map(len, rows))
    used = set(j for row in rows for j in row)
    expected_top = dict(schema_version="frontier-structure-v1", graph=label,
        n=n, e=e, file_bytes=len(csr_bytes(rows)), batch_rows=16, rank_base=1,
        rows_skipped_zero=sum(not row for row in rows), no_consumer_sources=n-len(used),
        unsorted_rows=sum(any(a > b for a, b in zip(row, row[1:])) for row in rows),
        adjacent_duplicate_occurrences=sum(a == b for row in rows for a, b in zip(row, row[1:])),
        analysis_pass=True, empty_fastpath=True, source_bytes=256, partial_bytes=512,
        ready_bytes=256, full_hidden_reference_bytes=256*n,
        model=dict(read_values=False, strict_csr_occurrence_order=True,
            single_producer_batch=True, ready_arbitrary_rows=True,
            empty_rows_enter_ready=False, no_consumer_sources_retained=False,
            intradrain_peak_modeled=False, hardware_cache_or_dram_claim=False,
            partial_segment_spill_assumed=True, metadata_bytes_in_state=False,
            amx_scratch_in_state=False))
    equal(data, expected_top, label)
    require([o["name"] for o in data["orders"]] == ["nodeid", "degree", "qguard"], "wrong order matrix")
    checked_orders = []
    for entry in data["orders"]:
        name = entry["name"]
        where = f"{label}.{name}"
        order = entry["permutation"]
        require(sorted(order) == list(range(n)), f"{where}: invalid permutation")
        if name == "nodeid":
            equal(order, list(range(n)), where+".nodeid_permutation")
        elif name == "degree":
            require(all(len(rows[a]) <= len(rows[b]) for a, b in zip(order, order[1:])),
                    f"{where}: degree permutation not nondecreasing")
        fnv = 1469598103934665603
        for j in order:
            fnv = ((fnv ^ j) * 1099511628211) & ((1 << 64)-1)
        eager = simulate(rows, order, 16, 1)
        forced_curve = [0] + [r["after_drain"]["source_rows"] for r in eager["batches"]]
        expected_end = [eager["source_last_use_rank"][j] if j in used else eager["rank"][j]
                        for j in range(n)]
        tile_degrees = [[len(rows[j]) for j in order[t:t+16]] for t in range(0, n, 16)]
        tile_high = [max(tile) for tile in tile_degrees]
        work_proxy = dict(sum_tile_max_degree=sum(tile_high),
            sum_tile_max_degree_x16=16*sum(tile_high),
            sum_tile_s64_windows=sum((d+63)//64 for d in tile_high),
            sum_tile_degree_spread=sum(max(tile)-min(tile) for tile in tile_degrees),
            max_tile_degree=max(tile_high, default=0),
            nonempty_producer_tiles=sum(d > 0 for d in tile_high), actual_kernel_work_claim=False)
        equal(entry, dict(permutation_fnv64=fnv, producer_batches=(n+15)//16,
            rank=eager["rank"], forced_release_rank=expected_end,
            row_completion_rank=eager["row_completion_rank"],
            forced_postdrain_source_rows=forced_curve,
            pure_forced=dict(peak_source_rows=max(forced_curve), peak_bytes=256*max(forced_curve),
                source_row_batch_area=sum(forced_curve), byte_batch_area=256*sum(forced_curve),
                eager_segments=eager["total_segments"], peak_batch=forced_curve.index(max(forced_curve))),
            first_layer_work_proxy=work_proxy), where)
        require([m["name"] for m in entry["methods"]] ==
            ["checkpoint_1", "checkpoint_4", "checkpoint_16", "checkpoint_ALL", "row_complete"],
            f"{where}: incorrect policy matrix")
        for method in entry["methods"]:
            mname = method["name"]
            whole_row = mname == "row_complete"
            checkpoint = 1 if whole_row else {"checkpoint_1":1, "checkpoint_4":4,
                "checkpoint_16":16, "checkpoint_ALL":0}[mname]
            reference = simulate(rows, order, 16, checkpoint, whole_row)
            expected = dict(checkpoint_batches=checkpoint, row_complete=whole_row)
            for key in ("source_last_use_rank", "first_consume_rank", "last_consume_rank",
                        "segments_per_row", "batches"):
                expected[key] = reference[key]
            expected.update(summary_from_simulation(reference))
            equal(method, expected, where+"."+mname)
        checked_orders.append(dict(name=name, methods=5, batches=(n+15)//16,
                                   status="PASS", permutation_fnv64=fnv))
    return dict(status="PASS", orders=checked_orders,
                scope="All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.")


def cases():
    fixtures = [
        ("single_empty", [[]]),
        ("single_self", [[0]]),
        ("all_empty_17", [[] for _ in range(17)]),
        ("all_self_17", [[i] for i in range(17)]),
        ("all_self_31", [[i] for i in range(31)]),
        ("all_self_33", [[i] for i in range(33)]),
        ("source_repeated_after_hole", [[0, 1, 0], [], []]),
        ("unsorted_repeated", [[2, 0, 1, 0] * 33, [1, 1, 2], []]),
        ("prefix_partial_releases_source", [[0, 1], [0]]),
        ("one_source_many_consumers", [[0] for _ in range(19)]),
        ("many_sources_one_consumer", [list(range(35))] + [[] for _ in range(34)]),
        ("reverse_long_row", [list(range(34, -1, -1))] + [[] for _ in range(34)]),
        ("mixed_empty_nonempty_34", [[i] if i % 2 else [] for i in range(34)]),
        ("holes_between_batches", [[16, 0, 32, 0], [33, 1, 17, 1]] + [[] for _ in range(32)]),
        ("complete_17", [list(range(17)) for _ in range(17)]),
        ("interleaved_cliques_64", [[j for j in range(64) if j % 4 == i % 4]
                                    for i in range(64)]),
        ("checkpoint16_tail_273", [[0, 255, 272]] + [[i] for i in range(1, 273)]),
    ]
    rng = random.Random(20261008)
    sizes = [1, 2, 3, 7, 15, 16, 17, 31, 32, 33, 47, 48, 63, 64]
    for case in range(64):
        n = sizes[case % len(sizes)]
        rows = []
        for i in range(n):
            degree = rng.randrange(min(2 * n + 1, 41))
            if rng.random() < 0.15:
                degree = 0
            row = [rng.randrange(n) for _ in range(degree)]
            if case % 3 == 0:
                row.sort()
            if row and case % 4 == 0:
                row.extend([row[0], row[0]])
            rows.append(row)
        fixtures.append((f"random_{case:02d}", rows))
    return fixtures


def csr_bytes(rows):
    n = len(rows)
    ptr = [0]
    cols = []
    for row in rows:
        cols.extend(row)
        ptr.append(len(cols))
    return (struct.pack("<IIIQQQ", 0, 0, 2, n, n, len(cols)) +
            struct.pack(f"<{n+1}I", *ptr) +
            struct.pack(f"<{len(cols)}I", *cols) +
            struct.pack(f"<{len(cols)}f", *([1.0] * len(cols))))


def malformed_cases():
    good = csr_bytes([[0], [1]])
    cases = [("truncated_header", good[:20]), ("truncated_payload", good[:-1])]
    wrong_version = bytearray(good)
    struct.pack_into("<I", wrong_version, 8, 99)
    cases.append(("wrong_version", bytes(wrong_version)))
    nonsquare = bytearray(good)
    struct.pack_into("<Q", nonsquare, 20, 3)
    cases.append(("non_square", bytes(nonsquare)))
    badptr = bytearray(good)
    struct.pack_into("<3I", badptr, 36, 0, 3, 2)
    cases.append(("rowptr_out_of_bounds", bytes(badptr)))
    badcol = bytearray(good)
    struct.pack_into("<I", badcol, 36 + 3 * 4, 2)
    cases.append(("column_out_of_bounds", bytes(badcol)))
    return cases


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", required=True)
    parser.add_argument("--out", type=pathlib.Path, required=True)
    args = parser.parse_args()
    require(not args.out.exists(), "output directory must be new")
    args.out.mkdir(parents=True)
    fixture_dir = args.out / "fixtures"
    fixture_dir.mkdir()
    status = dict(schema="independent-frontier-audit-v1", status="RUNNING",
                  oracle="produced bool + per-row cursor + occurrence reference counts",
                  empty_semantics="rank0 zero-complete; no ready/projection; no bias",
                  comparisons=[], malformed=[], mathematical_structure_only=True)
    try:
        for label, rows in cases():
            csr = fixture_dir / f"{label}.csrbin"
            native = fixture_dir / f"{label}.native.json"
            csr.write_bytes(csr_bytes(rows))
            command = [args.binary, "--csr", str(csr), "--out", str(native),
                "--graph", label, "--orders", "all", "--checkpoints", "1,4,16,whole", "--emit-plan"]
            p = subprocess.run(command, text=True, capture_output=True, timeout=120)
            (fixture_dir / f"{label}.stdout.txt").write_text(p.stdout, encoding="utf-8")
            (fixture_dir / f"{label}.stderr.txt").write_text(p.stderr, encoding="utf-8")
            require(p.returncode == 0, f"{label}: native failed ({p.returncode})")
            require(native.is_file(), f"{label}: missing JSON")
            data = json.loads(native.read_text(encoding="utf-8"))
            record = audit_native(data, rows, label)
            record.update(fixture=label, nodes=len(rows), edges=sum(map(len, rows)),
                          input_sha256=hashlib.sha256(csr.read_bytes()).hexdigest(),
                          native_json=str(native.relative_to(args.out)))
            status["comparisons"].append(record)
        for label, payload in malformed_cases():
            csr = fixture_dir / f"malformed_{label}.csrbin"
            out = fixture_dir / f"malformed_{label}.native.json"
            csr.write_bytes(payload)
            p = subprocess.run([args.binary, "--csr", str(csr), "--out", str(out),
                "--graph", label, "--orders", "all", "--emit-plan"],
                text=True, capture_output=True, timeout=30)
            (fixture_dir / f"malformed_{label}.stderr.txt").write_text(p.stderr, encoding="utf-8")
            require(p.returncode > 0, f"invalid CSR {label}: not cleanly rejected (code {p.returncode})")
            status["malformed"].append(dict(case=label, status="REJECTED", returncode=p.returncode))
        status.update(status="PASS", fixture_count=len(status["comparisons"]),
                      directed_fixtures=17, random_fixtures=64,
                      malformed_count=len(status["malformed"]))
    except Exception as exc:
        status.update(status="FAIL", error=str(exc), traceback=traceback.format_exc())
        raise
    finally:
        status["script_sha256"] = hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest()
        (args.out / "status.json").write_text(json.dumps(status, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({k: v for k, v in status.items() if k not in ("comparisons", "malformed")}, indent=2))


if __name__ == "__main__":
    main()
