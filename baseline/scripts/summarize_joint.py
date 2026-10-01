"""Summarize joint GAT logs without executing experiments or changing measurements.

Usage: python baseline/scripts/summarize_joint.py runs/joint-JOB
Writes summary.tsv and RESULTS.md into the run directory (or --output-dir).
Use --paths NAME,NAME,... to override the expected paths and speedup controls.
Only complete, nonnegative repetition IDs with all three layers are summarized.
E2E is counted once per path/repetition, although it occurs in three JSON rows.
"""

import argparse
import csv
import io
import json
import math
import statistics
from collections import defaultdict
from pathlib import Path


PATHS = (
    "B0_FP32", "B0_BF16", "B1_TFS", "local_online_fp32",
    "joint_g1", "joint_g2", "joint_g4", "joint_g8",
)
PHASES = (
    "layer_ms", "lr_ms", "kernel_ms", "projection_ms", "conversion_ms",
    "max_prescan_ms", "normalization_ms", "activation_ms",
)
DIAGNOSTICS = (
    "CHECK ", "MATCHED_IDENTITY ", "ORACLE_", "ATTENTION_DRIFT ",
    "PROFILE ", "JOINT_ONLINE_STATS ", "ONLINE_STATS ",
    "JOINT_COMPLETE", "JOINT_FAIL", "JOINT_SMOKE_", "JOINT_FULL_SMOKE_",
)


def number(value, context):
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError("{}: expected a number".format(context))
    value = float(value)
    if not math.isfinite(value) or value < 0:
        raise ValueError("{}: timing/count must be finite and nonnegative".format(context))
    return value


def integer(value, context):
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError("{}: expected an integer".format(context))
    return value


def distribution(values):
    return {
        "median": statistics.median(values),
        "min": min(values), "max": max(values), "count": len(values),
    }


def parse_log(path, expected_paths=PATHS):
    lines = path.read_text(encoding="utf-8", errors="strict").splitlines()
    grouped = defaultdict(dict)
    warnings = []
    skipped_warmups = 0
    for line_number, original in enumerate(lines, 1):
        line = original.lstrip("\ufeff \t")
        if not line.startswith("{"):
            continue
        context = "{}:{}".format(path.name, line_number)
        try:
            row = json.loads(line)
        except json.JSONDecodeError as error:
            raise ValueError("{}: invalid JSON: {}".format(context, error)) from error
        if not isinstance(row, dict) or not all(
                key in row for key in ("path", "rep", "layer", "e2e_ms")):
            warnings.append("{}: ignored non-timing JSON object".format(context))
            continue
        name = row["path"]
        if not isinstance(name, str) or not name:
            raise ValueError("{}: invalid path name".format(context))
        rep = integer(row["rep"], context + " rep")
        layer = integer(row["layer"], context + " layer")
        if layer not in (1, 2, 3):
            raise ValueError("{}: layer must be 1, 2, or 3".format(context))
        if rep < 0:
            skipped_warmups += 1
            continue
        for key, value in row.items():
            if key.endswith("_ms") or key == "source_logical_fp32_bytes":
                row[key] = number(value, context + " " + key)
        if "layer_ms" not in row:
            raise ValueError("{}: missing layer_ms".format(context))
        if layer in grouped[(name, rep)]:
            raise ValueError("{}: duplicate path/rep/layer {}/{}/{}; do not concatenate reruns".format(
                context, name, rep, layer))
        grouped[(name, rep)][layer] = row

    valid = defaultdict(list)
    for (name, rep), layers in sorted(grouped.items()):
        if set(layers) != {1, 2, 3}:
            warnings.append("excluded {}/rep {}: missing layers {}".format(
                name, rep, sorted({1, 2, 3} - set(layers))))
            continue
        e2e = layers[1]["e2e_ms"]
        if not all(math.isclose(e2e, layers[layer]["e2e_ms"], rel_tol=1e-10, abs_tol=1e-9)
                   for layer in (2, 3)):
            raise ValueError("{}: inconsistent E2E for {}/rep {}".format(path.name, name, rep))
        valid[name].append((rep, layers))

    paths = {}
    for name, repetitions in valid.items():
        result = {"e2e": distribution([rows[1]["e2e_ms"] for _, rows in repetitions]),
                  "rep_ids": [rep for rep, _ in repetitions], "layers": {}}
        for layer in (1, 2, 3):
            rows = [layers[layer] for _, layers in repetitions]
            fields = sorted({key for row in rows for key in row if key.endswith("_ms")}
                            - {"e2e_ms"})
            result["layers"][layer] = {}
            for field in fields:
                values = [row[field] for row in rows if field in row]
                if len(values) != len(rows):
                    warnings.append("{}/L{} {} present in {}/{} repetitions".format(
                        name, layer, field, len(values), len(rows)))
                result["layers"][layer][field] = distribution(values)
        paths[name] = result

    config = [line for line in lines if line.lstrip().startswith("CONFIG ")]
    final = [line for line in lines if line.lstrip().startswith("JOINT_COMPLETE")]
    failures = [line for line in lines if line.lstrip().startswith("JOINT_FAIL")]
    missing = [name for name in expected_paths if name not in paths]
    if missing:
        warnings.append("missing measured paths: " + ", ".join(missing))
    if not final:
        warnings.append("JOINT_COMPLETE marker not observed; report is partial")
    if failures:
        warnings.append("JOINT_FAIL marker observed; do not claim a successful completed run")
    expected_reps = None
    for line in config:
        fields = dict(item.split("=", 1) for item in line.split()[1:] if "=" in item)
        if "repeats" in fields:
            try:
                expected_reps = int(fields["repeats"])
            except ValueError:
                warnings.append("invalid repeats field in CONFIG")
    if expected_reps is not None:
        for name, result in paths.items():
            if result["rep_ids"] != list(range(expected_reps)):
                warnings.append("{} repetition IDs {} differ from CONFIG expected {}".format(
                    name, result["rep_ids"], list(range(expected_reps))))
    return {"file": path.name, "paths": paths, "warnings": warnings,
            "config": config, "static": [line for line in lines if line.lstrip().startswith("STATIC ")],
            "diagnostics": [line for line in lines if line.lstrip().startswith(DIAGNOSTICS)],
            "complete_marker": bool(final), "failed_marker": bool(failures),
            "skipped_warmup_json_rows": skipped_warmups}


def ordered_names(paths, expected_paths=PATHS):
    return [name for name in expected_paths if name in paths] + sorted(set(paths) - set(expected_paths))


def speedup(paths, control, candidate):
    if control not in paths or candidate not in paths:
        return None
    elapsed = paths[candidate]["e2e"]["median"]
    return paths[control]["e2e"]["median"] / elapsed if elapsed > 0 else None


def format_number(value, digits=6):
    return "-" if value is None else format(value, ".{}f".format(digits))


def table(headers, rows):
    def cell(value):
        return str(value).replace("|", "\\|").replace("\n", " ")
    result = ["| " + " | ".join(map(cell, headers)) + " |",
              "| " + " | ".join("---" for _ in headers) + " |"]
    result.extend("| " + " | ".join(map(cell, row)) + " |" for row in rows)
    return "\n".join(result) + "\n"


def raw_block(lines):
    # Use a fence longer than any existing backtick sequence in raw diagnostics.
    text = "\n".join(lines) if lines else "(no matching lines recorded)"
    fence = "```"
    while fence in text:
        fence += "`"
    return fence + "text\n" + text + "\n" + fence + "\n"


def make_markdown(root, datasets, expected_paths=PATHS):
    out = ["# Joint GAT experiment results\n",
           "Run directory: `{}`. All times are milliseconds.\n".format(root.name),
           "## Reading these results\n",
           "- E2E median/min/max count each path/repetition once; only repetitions with all three layers are included. Warmup rows are excluded.\n"
           "- Speedup = control E2E median / candidate E2E median. A ratio above 1 means the candidate is faster. This is a ratio of medians, not a median of paired ratios.\n"
           "- `B1_TFS` is the existing B1 TFS BF16 path. `local_online_fp32`, `joint_g*`, `joint_v1_g*`, and `joint_full_g*` are FP32 candidates; they are not improved B1 BF16. The full-group variant removes active-head guards while retaining the FP32 dataflow.\n"
           "- Each phase median is computed independently. Phase medians need not sum to the E2E median; E2E includes orchestration overhead.\n"
           "- `PROFILE` values ending in `_worker_ms` are sampled worker-time sums, not an additive wall-time decomposition. Rescale is fused into weighted SpMM; its scalar exp also belongs to the score phase. Do not add worker sums to layer wall time.\n"
           "- Zero stage fields can mean the work is inside another stage: joint/local UW and normalization are inside `kernel_ms`; a zero separate normalization field does not mean zero normalization cost.\n"
           "- Logical source bytes and source-vector counters are source-code workload estimates, not measured DRAM traffic or cache-miss counts. Sparse arithmetic amplification remains.\n"
           "- `UNVERIFIED` is preserved: an implementation comparison does not prove the original precision gate passed. CONFIG `untrained=true` means random-weight forward timing, not classification accuracy or end-to-end training performance.\n"
           "- Degree Sort and parameter preparation costs are recorded separately in CONFIG/STATIC and excluded from the reported steady-state E2E timing.\n",
           "## Dataset availability\n",
           table(["Dataset", "Log", "Complete marker", "Failure marker", "Measured paths"],
                 [[name, datasets[name]["file"], datasets[name]["complete_marker"],
                   datasets[name]["failed_marker"], len(datasets[name]["paths"])]
                  if name in datasets else [name, name + "_joint.log (not found)", "-", "-", 0]
                  for name in ("arxiv", "products")])]
    for dataset, data in datasets.items():
        paths = data["paths"]
        names = ordered_names(paths, expected_paths)
        out.append("## {}\n".format(dataset))
        if data["warnings"]:
            out.append("### Completeness notes\n")
            out.append("\n".join("- " + warning for warning in data["warnings"]) + "\n")
        out.append("### E2E\n")
        out.append(table(["Path", "Repeats", "Median ms", "Min ms", "Max ms", "Rep IDs"],
                         [[name, paths[name]["e2e"]["count"],
                           format_number(paths[name]["e2e"]["median"]),
                           format_number(paths[name]["e2e"]["min"]),
                           format_number(paths[name]["e2e"]["max"]),
                           ",".join(map(str, paths[name]["rep_ids"]))] for name in names]))
        out.append("### Speedup matrix\n")
        out.append("Rows are candidates; columns are controls. Missing controls appear as `-`.\n")
        out.append(table(["Candidate \\ Control"] + list(expected_paths),
                         [[name] + [format_number(speedup(paths, base, name), 4)
                                    for base in expected_paths] for name in names]))
        for layer in (1, 2, 3):
            out.append("### Layer {}{} phase medians\n".format(layer, " (D=256, K=8, d=32 focus)" if layer == 2 else ""))
            extra = sorted({field for name in names for field in paths[name]["layers"][layer]}
                           - set(PHASES))
            fields = list(PHASES) + extra
            out.append(table(["Path"] + fields,
                             [[name] + [format_number(paths[name]["layers"][layer].get(field, {}).get("median"))
                                        for field in fields] for name in names]))
        out.append("### Diagnostics: original log text\n")
        out.append("The following lines are reproduced without reinterpretation. Oracle selection may exclude costly high-degree rows; sampled correctness is not a full-graph accuracy guarantee.\n")
        out.append("#### Configuration and static preparation\n")
        out.append(raw_block(data["config"] + data["static"]))
        out.append("#### CHECK, matched attention, oracle, PROFILE and online statistics\n")
        out.append(raw_block(data["diagnostics"]))
    return "\n".join(out).rstrip() + "\n"


def make_tsv(datasets, expected_paths=PATHS):
    output = io.StringIO(newline="")
    writer = csv.writer(output, delimiter="\t", lineterminator="\n")
    writer.writerow(["dataset", "section", "path", "layer", "metric", "median_ms", "min_ms", "max_ms",
                     "measured_repeats"] + ["speedup_vs_" + name for name in expected_paths] + ["notes"])
    for dataset, data in datasets.items():
        paths = data["paths"]
        status = "complete_marker={};failed_marker={};precision=UNVERIFIED".format(
            data["complete_marker"], data["failed_marker"])
        for name in ordered_names(paths, expected_paths):
            result = paths[name]
            d = result["e2e"]
            writer.writerow([dataset, "e2e", name, "all", "e2e_ms", d["median"], d["min"], d["max"], d["count"]]
                            + ["" if speedup(paths, base, name) is None else speedup(paths, base, name)
                               for base in expected_paths] + [status])
            for layer in (1, 2, 3):
                for field, d in sorted(result["layers"][layer].items()):
                    writer.writerow([dataset, "phase", name, layer, field, d["median"], d["min"], d["max"], d["count"]]
                                    + [""] * len(expected_paths) + ["independent phase statistic; not additive wall decomposition"])
    return output.getvalue()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run", type=Path, help="directory containing arxiv_joint.log and/or products_joint.log")
    parser.add_argument("--output-dir", type=Path, help="output directory; defaults to the run directory")
    parser.add_argument("--paths", default=",".join(PATHS),
                        help="comma-separated expected paths, in table/control order; overrides the original default list")
    args = parser.parse_args()
    expected_paths = tuple(name.strip() for name in args.paths.split(","))
    if not all(expected_paths) or len(set(expected_paths)) != len(expected_paths):
        parser.error("--paths must contain unique, nonempty comma-separated names")
    if any(any(character.isspace() for character in name) for name in expected_paths):
        parser.error("--paths names must not contain internal whitespace")
    if not args.run.is_dir():
        parser.error("run directory does not exist: {}".format(args.run))
    datasets = {}
    for dataset in ("arxiv", "products"):
        path = args.run / (dataset + "_joint.log")
        if path.is_file():
            try:
                datasets[dataset] = parse_log(path, expected_paths)
            except (ValueError, UnicodeError) as error:
                parser.error(str(error))
    if not datasets:
        parser.error("no arxiv_joint.log or products_joint.log found")
    # Build both artifacts before writing. Importing this module never writes files.
    report = make_markdown(args.run, datasets, expected_paths)
    summary = make_tsv(datasets, expected_paths)
    output = args.output_dir if args.output_dir is not None else args.run
    output.mkdir(parents=True, exist_ok=True)
    (output / "summary.tsv").write_text(summary, encoding="utf-8")
    (output / "RESULTS.md").write_text(report, encoding="utf-8")
    print("Wrote {} and {}".format(output / "summary.tsv", output / "RESULTS.md"))


if __name__ == "__main__":
    main()
