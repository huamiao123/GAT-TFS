# PROJECT_PROGRESS — GCN-extra inference

Claims remain UNVERIFIED until qualified evidence is available.

## setup-20261003-gcn-extra

```json
{
  "id": "setup-20261003-gcn-extra",
  "kind": "setup",
  "status": "PASS",
  "purpose": "User-authorized new inference-only TFS extension experiment",
  "root": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra",
  "original_source_sha256": "4e574786055816df80beb6e68181fe19a8ee02cf4be1d43717bc1e79b88cab29",
  "agents_sha256": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
  "skill_sha256": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca",
  "scope_decision": "Inference plan and GCN-extra location explicitly override TFS-Train architecture scope. Workspace and evidence workflow still apply. Handoffs belong to GCN-extra; frozen TFS-Train/GAT evidence is untouched.",
  "experiments": "No compute job at setup; plan first shared-intel smoke then one exclusive-intel_expr 32-core paired run",
  "paper_eligible": false,
  "issues": "Original kernel ignores CSR values; graph audit is mandatory. Original gather is repeated per output panel; shared/replay controls expose this difference. Original BF16 is truncation.",
  "next": "Build; check numerical correctness before timings",
  "date": "2026-10-03T11:41:28.699876+00:00"
}
```

## build-20261003-194132

```json
{
  "id": "build-20261003-194132",
  "kind": "build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/build-20261003-194132",
  "purpose": "Build unchanged Original TFS and block/local inference extension",
  "source_hashes": "source.sha256",
  "binary_hashes": "binary.sha256",
  "compiler": "compiler.txt",
  "timing_boundary": "not a benchmark",
  "paper_eligible": false,
  "issues": "See build.log; no new major issue if exit=0",
  "next": "shared intel correctness gate",
  "date": "2026-10-03T11:41:39.712809+00:00"
}
```

## submit-smoke-20261003-194254

```json
{
  "id": "submit-smoke-20261003-194254",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861412",
  "mode": "smoke",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-smoke-20261003-194254",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T11:42:55.639016+00:00"
}
```

## smoke-10861412

```json
{
  "id": "smoke-10861412",
  "kind": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10861412",
  "partition": "intel",
  "node": "qhcn006",
  "threads": "8",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-10861412",
  "timing_boundary": "kernel includes original C-zero/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 2,
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T11:43:01.248027+00:00"
}
```

## decision-20261003-numa-and-tail-gates

```json
{
  "id": "decision-20261003-numa-and-tail-gates",
  "kind": "implementation_decision",
  "status": "PASS",
  "evidence": "runs/smoke-10861412/lscpu.txt, numa_hardware.txt, smoke/correctness.csv",
  "findings": "37-node mixed-degree fixture passed. B=1 shared_fast is bitwise identical to unchanged Original TFS. Instrumented original matches unchanged original. Node qhcn006 is Xeon Max 9462, 2 sockets x 32 cores, 8 CPU NUMA domains.",
  "decision": "Use process-local interleaving across allocated socket NUMA domains for all methods. Add 137-node q130 fixture to exercise multiple B64 chunks and destination tails, plus all-empty 17-node fixture. These unresolved edge cases justify additional correctness testing before formal runs.",
  "scope": "Policy is per benchmark process; no shared configuration changes. All artifacts remain under GCN-extra.",
  "paper_eligible": false,
  "issues": "32 physical cores cannot occupy a single CPU NUMA domain on these SNC4 nodes; literal plan assumption adjusted with topology evidence.",
  "next": "Tail and empty correctness gates, then exclusive paired benchmark",
  "date": "2026-10-03T11:45:53.739974+00:00"
}
```

## submit-smoke-20261003-194557

```json
{
  "id": "submit-smoke-20261003-194557",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861414",
  "mode": "smoke",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-smoke-20261003-194557",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T11:45:59.003901+00:00"
}
```

## smoke-10861414

```json
{
  "id": "smoke-10861414",
  "kind": "smoke",
  "status": "FAILED",
  "exit_status": 2,
  "job_id": "10861414",
  "partition": "intel",
  "node": "qhcn006",
  "threads": "8",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-10861414",
  "timing_boundary": "kernel includes original C-zero/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 2,
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T11:46:00.768047+00:00"
}
```

## issue-20261003-empty-mkl-csr

```json
{
  "id": "issue-20261003-empty-mkl-csr",
  "kind": "correctness_fix",
  "status": "FIX_IMPLEMENTED_PENDING_GATE",
  "affected_job": "10861414",
  "symptom": "17-node all-empty CSR rejected by mkl_sparse_s_create_csr; job failed before this fixture's timing. Mixed-degree 37-node and regular 137-node q130 fixtures passed.",
  "evidence": "runs/smoke-10861414/tail-n17-q0/stderr.log; tail-n137-q130/info.json; smoke/info.json",
  "root_cause": "oneMKL create interface rejects zero nnz/empty indices; mathematical operator remains well defined",
  "fix": "MKL adapter bypasses sparse handle for zero edges, zeros Z explicitly and retains ordinary SGEMM. Nonempty CSR path is unchanged.",
  "prevention": "Permanent all-empty fixture before formal acceptance; numerical gate still compares full outputs and FP64 samples",
  "source": "src/bench.cpp MKLGraph constructor/run",
  "paper_eligible": false,
  "next": "Rebuild and rerun meaningful smoke fixtures, then paired real-graph run",
  "date": "2026-10-03T11:47:24.929503+00:00"
}
```

## build-20261003-194728

```json
{
  "id": "build-20261003-194728",
  "kind": "build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/build-20261003-194728",
  "purpose": "Build unchanged Original TFS and block/local inference extension",
  "source_hashes": "source.sha256",
  "binary_hashes": "binary.sha256",
  "compiler": "compiler.txt",
  "timing_boundary": "not a benchmark",
  "paper_eligible": false,
  "issues": "See build.log; no new major issue if exit=0",
  "next": "shared intel correctness gate",
  "date": "2026-10-03T11:47:35.682456+00:00"
}
```

## submit-smoke-20261003-194810

```json
{
  "id": "submit-smoke-20261003-194810",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861420",
  "mode": "smoke",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-smoke-20261003-194810",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T11:48:11.815793+00:00"
}
```

## smoke-10861420

```json
{
  "id": "smoke-10861420",
  "kind": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10861420",
  "partition": "intel",
  "node": "qhcn006",
  "threads": "8",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-10861420",
  "timing_boundary": "kernel includes original C-zero/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 2,
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T11:48:14.342993+00:00"
}
```

## submit-formal-20261003-194849

```json
{
  "id": "submit-formal-20261003-194849",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861421",
  "mode": "formal",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-formal-20261003-194849",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T11:48:50.647779+00:00"
}
```

## formal-10861421

```json
{
  "id": "formal-10861421",
  "kind": "formal",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10861421",
  "partition": "intel_expr",
  "node": "qhcn818",
  "threads": "32",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10861421",
  "timing_boundary": "kernel includes original C-zero/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 5,
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T11:50:25.488195+00:00"
}
```

## decision-20261003-original-zeroing-controls

```json
{
  "id": "decision-20261003-original-zeroing-controls",
  "kind": "performance_diagnosis",
  "status": "CONTROL_IMPLEMENTED_PENDING_GATE",
  "affected_job": "10861421",
  "result": "Four graphs passed numerical gates and 5-repeat exclusive medians. Full local improves reddit E2E 292.841 to 150.660 ms; Pokec 221.242 to 210.465 ms. Original TFS underperforms MKL on reddit/Pokec, so paper baseline trend is not reproduced yet.",
  "root_cause_evidence": "soc-Pokec/profiles.csv: original serial output_zero=65.504 ms, original uninstrumented kernel median=110.746 ms; same zero in local kernel=65.763 ms, kernel median=101.821 ms",
  "source_proof": "Original loop writes all 128 columns of every destination row, including zero-degree rows; full C clear is redundant. New shared local kernel skips all-empty tiles, so owner-thread zeroing is needed there when global clear is removed.",
  "decision": "Keep unchanged original and all first-run evidence. Add exact original function minus only global memset; add matching nozero local variants. NaN-prefill control outputs before correctness, check bitwise original equality, rerun before comparing algorithm gains.",
  "precision": "Identical source-compatible BF16 inputs/weights and quantized MKL reference; Fast extra quantization remains explicit",
  "paper_eligible": false,
  "next": "Build/nozero correctness gate; paired exclusive run including both old and zero-free paths",
  "date": "2026-10-03T11:59:17.293435+00:00"
}
```

## build-20261003-195921

```json
{
  "id": "build-20261003-195921",
  "kind": "build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/build-20261003-195921",
  "purpose": "Build unchanged Original TFS and block/local inference extension",
  "source_hashes": "source.sha256",
  "binary_hashes": "binary.sha256",
  "compiler": "compiler.txt",
  "timing_boundary": "not a benchmark",
  "paper_eligible": false,
  "issues": "See build.log; no new major issue if exit=0",
  "next": "shared intel correctness gate",
  "date": "2026-10-03T11:59:27.708706+00:00"
}
```

## submit-smoke-20261003-200016

```json
{
  "id": "submit-smoke-20261003-200016",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861922",
  "mode": "smoke",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-smoke-20261003-200016",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T12:00:18.176860+00:00"
}
```

## smoke-10861922

```json
{
  "id": "smoke-10861922",
  "kind": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10861922",
  "partition": "intel",
  "node": "qhcn016",
  "threads": "8",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-10861922",
  "timing_boundary": "kernel includes original C-zero/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 2,
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T12:00:22.409445+00:00"
}
```

## submit-formal-20261003-200150

```json
{
  "id": "submit-formal-20261003-200150",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861923",
  "mode": "formal",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-formal-20261003-200150",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T12:01:52.619415+00:00"
}
```

## formal-10861923

```json
{
  "id": "formal-10861923",
  "kind": "formal",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10861923",
  "partition": "intel_expr",
  "node": "qhcn819",
  "threads": "32",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10861923",
  "timing_boundary": "kernel includes original C-zero/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 5,
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T12:03:05.322148+00:00"
}
```

## analysis-20261003-first-round-completed

```json
{
  "id": "analysis-20261003-first-round-completed",
  "kind": "milestone_analysis",
  "status": "FIRST_ROUND_COMPLETE",
  "evidence": [
    "runs/formal-10861421",
    "runs/formal-10861923",
    "docs/FIRST_RESULTS_20261003.md"
  ],
  "numerical_records": 264,
  "failed": 0,
  "metrics": [
    {
      "graph": "reddit",
      "original_kernel_ms": 151.926994324,
      "original_nozero_kernel_ms": 146.007061005,
      "local_full_fast_kernel_ms": 65.2031898499,
      "mkl_fp32_e2e_ms": 209.553956985,
      "original_nozero_e2e_ms": 276.08704567,
      "local_full_fast_e2e_ms": 131.146907806,
      "local_full_accurate_e2e_ms": 133.47196579,
      "full_fast_speedup_corrected_original": 2.105173886969594,
      "full_fast_speedup_mkl": 1.5978566364293103
    },
    {
      "graph": "soc-Pokec",
      "original_kernel_ms": 112.365007401,
      "original_nozero_kernel_ms": 41.3780212402,
      "local_full_fast_kernel_ms": 36.6308689117,
      "mkl_fp32_e2e_ms": 130.800962448,
      "original_nozero_e2e_ms": 91.6390419006,
      "local_full_fast_e2e_ms": 80.5268287659,
      "local_full_accurate_e2e_ms": 84.4678878784,
      "full_fast_speedup_corrected_original": 1.137993924571454,
      "full_fast_speedup_mkl": 1.6243153300902016
    },
    {
      "graph": "regular-q16",
      "original_kernel_ms": 17.6939964294,
      "original_nozero_kernel_ms": 4.53615188599,
      "local_full_fast_kernel_ms": 2.89797782898,
      "mkl_fp32_e2e_ms": 20.3440189362,
      "original_nozero_e2e_ms": 12.6340389252,
      "local_full_fast_e2e_ms": 10.4701519012,
      "local_full_accurate_e2e_ms": 10.5438232422,
      "full_fast_speedup_corrected_original": 1.2066719799692682,
      "full_fast_speedup_mkl": 1.9430490720835047
    },
    {
      "graph": "regular-q256",
      "original_kernel_ms": 73.3230113983,
      "original_nozero_kernel_ms": 60.4858398438,
      "local_full_fast_kernel_ms": 36.4439487457,
      "mkl_fp32_e2e_ms": 180.510044098,
      "original_nozero_e2e_ms": 124.5470047,
      "local_full_fast_e2e_ms": 77.2590637207,
      "local_full_accurate_e2e_ms": 75.6759643555,
      "full_fast_speedup_corrected_original": 1.612069816821119,
      "full_fast_speedup_mkl": 2.33642546783331
    }
  ],
  "paper_eligible": false,
  "issues": "Redundant original serial C zeroing diagnosed and controlled; zero-nnz MKL adapter validated. Full generality, checkpoint accuracy, hardware counters and full paper baseline reproduction pending.",
  "next": "Strong source-faithful R/CLI audit followed by q/degree-distribution sensitivity; retain DegreeSort and local fusion",
  "date": "2026-10-03T12:12:52.419252+00:00"
}
```

## maintenance-20261003-utf8-handoffs

```json
{
  "id": "maintenance-20261003-utf8-handoffs",
  "kind": "evidence_portability_fix",
  "status": "PASS",
  "symptom": "Local Windows default GBK could not decode server UTF-8 handoff markdown; server validation already passed",
  "root_cause": "Implicit locale-dependent pathlib read_text/open encoding in record_event.py",
  "fix": "Use explicit UTF-8 for all handoff/event text reads and writes",
  "validation": "Local HANDOFF_SYNCHRONIZED check passed after fix; no numerical kernels/binaries/timings changed",
  "paper_eligible": false,
  "next": "First inference experiment milestone complete; next science task is source-faithful strong baseline and wider q/skew sensitivity",
  "date": "2026-10-03T12:16:14.001174+00:00"
}
```

## build-20261003-203218

```json
{
  "id": "build-20261003-203218",
  "kind": "build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/build-20261003-203218",
  "purpose": "Build unchanged Original TFS and block/local inference extension",
  "source_hashes": "source.sha256",
  "binary_hashes": "binary.sha256",
  "compiler": "compiler.txt",
  "timing_boundary": "not a benchmark",
  "paper_eligible": false,
  "issues": "See build.log; no new major issue if exit=0",
  "next": "shared intel correctness gate",
  "date": "2026-10-03T12:32:29.311241+00:00"
}
```

## submit-smoke-20261003-203238

```json
{
  "id": "submit-smoke-20261003-203238",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861973",
  "mode": "smoke",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-smoke-20261003-203238",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T12:32:40.066226+00:00"
}
```

## smoke-10861973

```json
{
  "id": "smoke-10861973",
  "kind": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10861973",
  "partition": "intel",
  "node": "qhcn009",
  "threads": "8",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-10861973",
  "timing_boundary": "kernel includes per-method C-zero policy/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 2,
  "block_sweep": "1",
  "blocks": "2,4,8,16,32,64,full",
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T12:32:43.932575+00:00"
}
```

## submit-formal-20261003-203303

```json
{
  "id": "submit-formal-20261003-203303",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10861974",
  "mode": "formal",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-formal-20261003-203303",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T12:33:04.534965+00:00"
}
```

## formal-10861974

```json
{
  "id": "formal-10861974",
  "kind": "formal",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10861974",
  "partition": "intel_expr",
  "node": "qhcn819",
  "threads": "32",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10861974",
  "timing_boundary": "kernel includes per-method C-zero policy/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 10,
  "block_sweep": "1",
  "blocks": "2,4,8,16,32,64,full",
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T12:36:19.997716+00:00"
}
```

## analysis-formal-10861974-block-sweep

```json
{
  "id": "analysis-formal-10861974-block-sweep",
  "kind": "analysis",
  "status": "PASS",
  "purpose": "Complete B2/4/8/16/32/64/Full Fast and Accurate nozero sweep including two-layer E2E",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10861974",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/BLOCK_SWEEP_RESULTS_20261003.md",
  "correctness_records": 504,
  "failed_records": 0,
  "repeats": 10,
  "warmups": 1,
  "metrics_ms": {
    "reddit": {
      "kernel": {
        "mkl_bf16_inputs": 104.680418968,
        "original": 151.6820192335,
        "original_nozero": 147.9054689405,
        "shared_b2_fast_nozero": 168.098449707,
        "shared_b2_accurate_nozero": 221.6426134105,
        "shared_b4_fast_nozero": 109.3279123305,
        "shared_b4_accurate_nozero": 135.4390382765,
        "shared_b8_fast_nozero": 82.21590518955,
        "shared_b8_accurate_nozero": 93.6665534973,
        "replay_b8_fast_nozero": 140.12408256499998,
        "shared_b16_fast_nozero": 70.77157497405,
        "shared_b16_accurate_nozero": 74.91099834445001,
        "shared_b32_fast_nozero": 62.72995471955,
        "shared_b32_accurate_nozero": 66.00093841555,
        "replay_b32_fast_nozero": 113.6730909345,
        "shared_b64_fast_nozero": 61.03706359865,
        "shared_b64_accurate_nozero": 62.69896030425,
        "shared_bfull_fast_nozero": 66.3025379181,
        "shared_bfull_accurate_nozero": 66.47956371305,
        "replay_bfull_fast_nozero": 117.539525032,
        "mkl_fp32": 104.1684150695
      },
      "e2e": {
        "mkl_bf16_inputs": 208.9014053345,
        "original": 299.94738101949997,
        "original_nozero": 286.79203987150004,
        "shared_b2_fast_nozero": 340.53897857649997,
        "shared_b2_accurate_nozero": 450.206041336,
        "shared_b4_fast_nozero": 225.17991066,
        "shared_b4_accurate_nozero": 273.481488228,
        "shared_b8_fast_nozero": 168.4379577635,
        "shared_b8_accurate_nozero": 191.62595272049998,
        "replay_b8_fast_nozero": 281.71646595050004,
        "shared_b16_fast_nozero": 143.4564590455,
        "shared_b16_accurate_nozero": 152.541875839,
        "shared_b32_fast_nozero": 130.1859617235,
        "shared_b32_accurate_nozero": 135.753393173,
        "replay_b32_fast_nozero": 231.2269210815,
        "shared_b64_fast_nozero": 124.060869217,
        "shared_b64_accurate_nozero": 125.65946579,
        "shared_bfull_fast_nozero": 133.6574554445,
        "shared_bfull_accurate_nozero": 131.3054561615,
        "replay_bfull_fast_nozero": 245.96703052499998,
        "mkl_fp32": 210.8985185625
      }
    },
    "regular-q16": {
      "kernel": {
        "mkl_bf16_inputs": 9.392976760865,
        "original": 17.81404018405,
        "original_nozero": 4.600882530210001,
        "shared_b2_fast_nozero": 6.199955940244999,
        "shared_b2_accurate_nozero": 7.800102233885,
        "shared_b4_fast_nozero": 4.2690038681,
        "shared_b4_accurate_nozero": 5.045652389524999,
        "shared_b8_fast_nozero": 3.38399410248,
        "shared_b8_accurate_nozero": 3.719449043275,
        "replay_b8_fast_nozero": 4.445552825925001,
        "shared_b16_fast_nozero": 2.90107727051,
        "shared_b16_accurate_nozero": 3.090500831605,
        "shared_b32_fast_nozero": 2.925992012025,
        "shared_b32_accurate_nozero": 3.0518770217849998,
        "replay_b32_fast_nozero": 3.9304494857799996,
        "shared_b64_fast_nozero": 2.9361248016349997,
        "shared_b64_accurate_nozero": 3.0685663223299997,
        "shared_bfull_fast_nozero": 2.89499759674,
        "shared_bfull_accurate_nozero": 3.01647186279,
        "replay_bfull_fast_nozero": 3.875494003295,
        "mkl_fp32": 10.39695739745
      },
      "e2e": {
        "mkl_bf16_inputs": 20.76709270475,
        "original": 35.3264808655,
        "original_nozero": 12.638092041,
        "shared_b2_fast_nozero": 15.76042175295,
        "shared_b2_accurate_nozero": 18.815040588400002,
        "shared_b4_fast_nozero": 11.842012405399998,
        "shared_b4_accurate_nozero": 13.34202289585,
        "shared_b8_fast_nozero": 9.749054908755,
        "shared_b8_accurate_nozero": 10.80906391145,
        "replay_b8_fast_nozero": 11.87694072725,
        "shared_b16_fast_nozero": 9.26506519318,
        "shared_b16_accurate_nozero": 10.43009757995,
        "shared_b32_fast_nozero": 9.764909744265001,
        "shared_b32_accurate_nozero": 10.668516159100001,
        "replay_b32_fast_nozero": 11.87896728515,
        "shared_b64_fast_nozero": 10.3933811188,
        "shared_b64_accurate_nozero": 10.536551475500001,
        "shared_bfull_fast_nozero": 10.040998458885,
        "shared_bfull_accurate_nozero": 10.6555223465,
        "replay_bfull_fast_nozero": 11.8930339813,
        "mkl_fp32": 20.2181339264
      }
    },
    "regular-q256": {
      "kernel": {
        "mkl_bf16_inputs": 89.08700942995,
        "original": 73.93753528595,
        "original_nozero": 60.800075531,
        "shared_b2_fast_nozero": 97.32902050015,
        "shared_b2_accurate_nozero": 120.5461025235,
        "shared_b4_fast_nozero": 62.6600980759,
        "shared_b4_accurate_nozero": 75.81448554990001,
        "shared_b8_fast_nozero": 48.328042030350005,
        "shared_b8_accurate_nozero": 55.3414821625,
        "replay_b8_fast_nozero": 68.62950325015,
        "shared_b16_fast_nozero": 42.15240478515,
        "shared_b16_accurate_nozero": 44.8755025864,
        "shared_b32_fast_nozero": 38.9815568924,
        "shared_b32_accurate_nozero": 41.364550590549996,
        "replay_b32_fast_nozero": 55.32443523405,
        "shared_b64_fast_nozero": 37.22047805785,
        "shared_b64_accurate_nozero": 38.262486457799994,
        "shared_bfull_fast_nozero": 37.6410484314,
        "shared_bfull_accurate_nozero": 37.960529327399996,
        "replay_bfull_fast_nozero": 54.0255308151,
        "mkl_fp32": 91.26055240630001
      },
      "e2e": {
        "mkl_bf16_inputs": 178.906917572,
        "original": 146.121501923,
        "original_nozero": 124.5088577275,
        "shared_b2_fast_nozero": 190.1569366455,
        "shared_b2_accurate_nozero": 240.16499519349998,
        "shared_b4_fast_nozero": 128.4046173095,
        "shared_b4_accurate_nozero": 155.0689935685,
        "shared_b8_fast_nozero": 99.1500616074,
        "shared_b8_accurate_nozero": 112.098097801,
        "replay_b8_fast_nozero": 139.10841941799998,
        "shared_b16_fast_nozero": 87.1295928955,
        "shared_b16_accurate_nozero": 90.37005901340001,
        "shared_b32_fast_nozero": 78.70841026305,
        "shared_b32_accurate_nozero": 84.3595266342,
        "replay_b32_fast_nozero": 112.7890348435,
        "shared_b64_fast_nozero": 78.48596572874999,
        "shared_b64_accurate_nozero": 78.46450805665,
        "shared_bfull_fast_nozero": 78.58943939209999,
        "shared_bfull_accurate_nozero": 78.22203636169999,
        "replay_bfull_fast_nozero": 108.5884571075,
        "mkl_fp32": 181.133031845
      }
    },
    "soc-Pokec": {
      "kernel": {
        "mkl_bf16_inputs": 65.1499032974,
        "original": 117.029428482,
        "original_nozero": 42.1150922775,
        "shared_b2_fast_nozero": 69.12052631374999,
        "shared_b2_accurate_nozero": 84.5700502396,
        "shared_b4_fast_nozero": 53.858041763299994,
        "shared_b4_accurate_nozero": 61.03885173795,
        "shared_b8_fast_nozero": 44.43943500515,
        "shared_b8_accurate_nozero": 49.691557884199995,
        "replay_b8_fast_nozero": 56.28395080565,
        "shared_b16_fast_nozero": 42.34886169435,
        "shared_b16_accurate_nozero": 43.49553585055,
        "shared_b32_fast_nozero": 37.98246383665,
        "shared_b32_accurate_nozero": 41.40949249265,
        "replay_b32_fast_nozero": 49.87847805025,
        "shared_b64_fast_nozero": 37.7680063248,
        "shared_b64_accurate_nozero": 39.04807567595,
        "shared_bfull_fast_nozero": 37.13655471805,
        "shared_bfull_accurate_nozero": 38.9629602432,
        "replay_bfull_fast_nozero": 47.12307453155,
        "mkl_fp32": 63.97151947025
      },
      "e2e": {
        "mkl_bf16_inputs": 139.78254795100003,
        "original": 232.4135303495,
        "original_nozero": 89.2368555069,
        "shared_b2_fast_nozero": 144.71101760850001,
        "shared_b2_accurate_nozero": 177.74653434750002,
        "shared_b4_fast_nozero": 113.3894920345,
        "shared_b4_accurate_nozero": 131.1864852905,
        "shared_b8_fast_nozero": 99.6254682541,
        "shared_b8_accurate_nozero": 106.9985628125,
        "replay_b8_fast_nozero": 121.017575264,
        "shared_b16_fast_nozero": 90.91544151305,
        "shared_b16_accurate_nozero": 95.6345796585,
        "shared_b32_fast_nozero": 83.66048336029999,
        "shared_b32_accurate_nozero": 90.29698371885,
        "replay_b32_fast_nozero": 107.74505138399999,
        "shared_b64_fast_nozero": 81.06291294095,
        "shared_b64_accurate_nozero": 86.64953708645001,
        "shared_bfull_fast_nozero": 81.7844867706,
        "shared_bfull_accurate_nozero": 85.4411125183,
        "replay_bfull_fast_nozero": 102.900505066,
        "mkl_fp32": 131.091475487
      }
    }
  },
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Expand graph/shape sensitivity and repeat-node evidence; retain strong Original_nozero control",
  "date": "2026-10-03T12:36:53.161990+00:00"
}
```

## build-20261003-204804

```json
{
  "id": "build-20261003-204804",
  "kind": "build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/build-20261003-204804",
  "purpose": "Build unchanged Original TFS and block/local inference extension",
  "source_hashes": "source.sha256",
  "binary_hashes": "binary.sha256",
  "compiler": "compiler.txt",
  "timing_boundary": "not a benchmark",
  "paper_eligible": false,
  "issues": "See build.log; no new major issue if exit=0",
  "next": "shared intel correctness gate",
  "date": "2026-10-03T12:48:11.833800+00:00"
}
```

## submit-smoke-20261003-204853

```json
{
  "id": "submit-smoke-20261003-204853",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10862001",
  "mode": "smoke",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-smoke-20261003-204853",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T12:48:55.772788+00:00"
}
```

## smoke-10862001

```json
{
  "id": "smoke-10862001",
  "kind": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10862001",
  "partition": "intel",
  "node": "qhcn009",
  "threads": "8",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-10862001",
  "timing_boundary": "kernel includes per-method C-zero policy/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 2,
  "block_sweep": "1",
  "blocks": "2,4,8,16,32,64,full",
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T12:48:58.357853+00:00"
}
```

## decision-20261003-full-graph-suite

```json
{
  "id": "decision-20261003-full-graph-suite",
  "kind": "decision",
  "status": "READY",
  "purpose": "User requested other graphs, prioritizing high average degree and as many available graphs as possible",
  "inventory": "docs/GRAPH_INVENTORY_20261003.json: 25 canonical yx/TFS/data CSR files, sorted by E/N",
  "scope": "Same source unweighted sum operator; D=F=128, TR=16, R=64, DegreeSort, immediate local AMX fusion; prepared two-layer inference, random H/W",
  "methods": "MKL FP32, matched BF16 MKL, unchanged original, Original_nozero; shared B2/4/8/16/32/64/Full Fast and Accurate nozero; no replay controls in this coverage sweep",
  "timing": "1 warmup, 10 measurements, rotating order, kernel and two-layer E2E, separate phase profile; each graph validates before timing",
  "memory_adjustment": "Free obsolete matched/fp32/orig full arrays after kernel checks+profile, before E2E arrays, outside measured intervals; dense live peak reduces from 6144*N to 4608*N bytes; kernels unchanged",
  "resource_plan": "one exclusive intel_expr node, 32 physical cores on one socket, 448 GiB allocation, maximum 2h; graphs run sequentially, Friendster last",
  "numa": "Graphs budget<=200 GiB use assigned socket CPU NUMA interleaving as before. Friendster budget approx409 GiB exceeds one socket memory; use online CPU NUMA domains across both sockets identically for all methods on that graph. Report this distinction",
  "failures": "Reject incompatible layouts/nonunit values/correctness failures; retain stderr, status, partial CSV; do not alter graph weights to force agreement. Continue other graphs",
  "authorization": "Current explicit user request plus prior experiments authorization; max5-node user cap, only1 node used here",
  "correctness_gate": "memory-lifetime change passed smoke-10862001, all three edge/tail/empty fixtures, unchanged Original_nozero bitwise identity",
  "issues": "no new major issue; inventory estimates now count both original colidx and MKL copied colidx plus MKL values (12 bytes per edge), not8",
  "evidence": "build-20261003-204804, smoke-10862001, docs/GRAPH_INVENTORY_20261003.json",
  "paper_eligible": false,
  "next": "Run full 25-graph suite, report failed/skipped graphs and strong-baseline speedups; do not infer general optimal B from partial results",
  "date": "2026-10-03T12:50:33.278078+00:00"
}
```

## submit-suite-20261003-205033

```json
{
  "id": "submit-suite-20261003-205033",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10862002",
  "mode": "suite",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-suite-20261003-205033",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T12:50:34.737166+00:00"
}
```

## build-20261003-211404

```json
{
  "id": "build-20261003-211404",
  "kind": "build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/build-20261003-211404",
  "purpose": "Build unchanged Original TFS and block/local inference extension",
  "source_hashes": "source.sha256",
  "binary_hashes": "binary.sha256",
  "compiler": "compiler.txt",
  "timing_boundary": "not a benchmark",
  "paper_eligible": false,
  "issues": "See build.log; no new major issue if exit=0",
  "next": "shared intel correctness gate",
  "date": "2026-10-03T13:14:11.042109+00:00"
}
```

## submit-smoke-20261003-211501

```json
{
  "id": "submit-smoke-20261003-211501",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10862091",
  "mode": "smoke",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-smoke-20261003-211501",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T13:15:03.729861+00:00"
}
```

## smoke-10862091

```json
{
  "id": "smoke-10862091",
  "kind": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10862091",
  "partition": "intel",
  "node": "qhcn006",
  "threads": "8",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-10862091",
  "timing_boundary": "kernel includes per-method C-zero policy/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 2,
  "block_sweep": "1",
  "blocks": "2,4,8,16,32,64,full",
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "See stderr/status; sampled phase timings are advisory and sum thread time, not wall",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "date": "2026-10-03T13:15:07.098052+00:00"
}
```

## decision-20261003-friendster-ilp64

```json
{
  "id": "decision-20261003-friendster-ilp64",
  "kind": "decision",
  "status": "READY",
  "purpose": "Cover Friendster without invalid signed32 CSR offsets in MKL reference",
  "evidence": "Friendster header E=3612134270 > INT32_MAX; installed mkl_types.h defines LP64 MKL_INT as int, ILP64 as MKL_INT64; ILP64 library found in current read-only module",
  "implementation": "Build with MKL_ILP64 and explicit mkl_intel_ilp64 + mkl_intel_thread + mkl_core libraries. Only MKL index width changes, float precision remains FP32 and AMX BF16. Original source and AMX kernels remain unchanged",
  "correctness": "build-20261003-211404 PASS, smoke-10862091 PASS across empty/tail/negative fixtures and Original_nozero bitwise identity",
  "memory": "MKL copied colidx is8 bytes vs4; estimated budget425.7 GiB, allocation448 GiB; CPU32 cores single socket, memory interleaved across online CPU domains of both sockets identically for all paths",
  "timing": "B2/4/8/16/32/64/Full Fast+Accurate nozero, Original and Original_nozero, FP32 and matched BF16 MKL; one warmup10 measured repetitions; full matrix+sample correctness and two-layer E2E",
  "prior_batch": "formal-10862002 retains frozen LP64 binary; its Friendster attempt may fail on reference index limits. Preserve this outcome, then use isolated ILP64 run for valid comparison",
  "authorization": "User requested as many graphs as possible. Up to2 exclusive jobs may run concurrently, below max5-node cap. Handoff appends now use advisory flock to serialize concurrent-job updates",
  "issues": "MKL LP64 signed row-offset limitation; fixed via ILP64 build, real-large-graph validation pending",
  "paper_eligible": false,
  "next": "Collect isolated Friendster result and reconcile it with25-graph coverage, keeping failed LP64 evidence",
  "date": "2026-10-03T13:17:30.769246+00:00"
}
```

## submit-suite-20261003-211730

```json
{
  "id": "submit-suite-20261003-211730",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10862093",
  "mode": "suite",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-suite-20261003-211730",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T13:17:35.338279+00:00"
}
```

## decision-20261003-friendster-budget-adjustment

```json
{
  "id": "decision-20261003-friendster-budget-adjustment",
  "kind": "decision",
  "status": "READY",
  "cancelled_job": "10862093, verified PENDING before scancel; no benchmark ran",
  "purpose": "Fit full numerical checks, kernel and two-layer timings on65.6M-node Friendster under intel_expr2h job limit",
  "new_scope": "Same ILP64 build and float precision, baseline/method kernels unchanged; B8/16/32/64/Full Fast+Accurate,1warmup5measured repeats in rotating order; every baseline also5repeats. Other24-graph attempts retain7block sizes10repeats",
  "reason": "Friendster has3.612B edges and massive dense reference arrays. Reducing small-block redundant controls and repetition count leaves budget for meaningful complete checks/E2E rather than risking a2h timeout",
  "evidence": "Full suite formal-10862002, latest build-20261003-211404, smoke-10862091; preserved queued submission10862093",
  "issues": "no new major issue; resource/timing scope adjustment must be explicit in derived report",
  "paper_eligible": false,
  "next": "Submit replacement Friendster run, retain cancellation and all comparison timing boundaries",
  "date": "2026-10-03T13:23:36.134092+00:00"
}
```

## submit-suite-20261003-212336

```json
{
  "id": "submit-suite-20261003-212336",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10862158",
  "mode": "suite",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-suite-20261003-212336",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T13:23:38.169449+00:00"
}
```

## formal-10862002

```json
{
  "id": "formal-10862002",
  "kind": "formal",
  "status": "COMPLETED_WITH_DATASET_FAILURES",
  "exit_status": 0,
  "job_id": "10862002",
  "partition": "intel_expr",
  "node": "qhcn819",
  "threads": "32",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862002",
  "timing_boundary": "kernel includes per-method C-zero policy/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 10,
  "block_sweep": "1",
  "blocks": "2,4,8,16,32,64,full",
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": {
    "kron_g500-logn21": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "cage15": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "FullChip": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "scircuit": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "sx-stackoverflow": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "rajat31": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "road_usa": "",
    "com-Friendster": ""
  },
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "datasets": {
    "mycielskian19": "PASS",
    "reddit": "PASS",
    "hollywood-2009": "PASS",
    "kron_g500-logn21": "FAILED",
    "ogbn-products": "PASS",
    "indochina-2004": "PASS",
    "cage15": "FAILED",
    "soc-Pokec": "PASS",
    "com-LiveJournal": "PASS",
    "rgg_n_2_24_s0": "PASS",
    "soc-LiveJournal1": "PASS",
    "as-Skitter": "PASS",
    "email-Enron": "PASS",
    "FullChip": "FAILED",
    "amazon0601": "PASS",
    "scircuit": "FAILED",
    "web-Google": "PASS",
    "com-Youtube": "PASS",
    "cit-Patents": "PASS",
    "sx-stackoverflow": "FAILED",
    "rajat31": "FAILED",
    "roadNet-CA": "PASS",
    "road_usa": "FAILED",
    "wiki-Talk": "PASS",
    "com-Friendster": "FAILED"
  },
  "date": "2026-10-03T13:27:42.673543+00:00"
}
```

## decision-20261003-road-ilp64-recovery

```json
{
  "id": "decision-20261003-road-ilp64-recovery",
  "kind": "decision",
  "status": "READY",
  "purpose": "Recover road_usa failure without attributing a preparation crash to an algorithm slowdown",
  "evidence": "formal-10862002/road_usa/status.json: exit_code=-11, unitvalues scan passed, stdout stops before CONFIG. N=23947347 and N*128=3065260416 exceed INT32_MAX; no numerical or timing result accepted",
  "hypothesis": "MKL LP64 internal dense-element index boundary in reference/preparation; suspected, not yet proven because no stacktrace",
  "recovery": "Use existing ILP64 build-20261003-211404 after smoke-10862091. Same original AMX and local kernels, FP32/BF16 math, inputs, B2/4/8/16/32/64/Full,1warmup10measured repeats, full checks and two-layer timing",
  "resource": "one exclusive node32physical cores,448GiB job allocation (estimated memory135GiB),2h maximum, normal socket CPU-NUMA interleave. Max3jobs including other current attempts, below5nodes",
  "issues": "road_usa LP64 preparation segmentation fault; preserve failure, validate ILP64 recovery before claiming root cause resolved",
  "paper_eligible": false,
  "next": "Collect isolated recovery and include only a validated complete run in coverage summary",
  "date": "2026-10-03T13:29:36.167006+00:00"
}
```

## submit-suite-20261003-212938

```json
{
  "id": "submit-suite-20261003-212938",
  "kind": "submission",
  "status": "SUBMITTED",
  "job_id": "10862162",
  "mode": "suite",
  "nodes": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/submit-suite-20261003-212938",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Collect job status and correctness before reporting speedup",
  "date": "2026-10-03T13:29:41.689554+00:00"
}
```

## analysis-formal-10862002-25graphs

```json
{
  "id": "analysis-formal-10862002-25graphs",
  "kind": "analysis",
  "status": "PARTIAL",
  "attempted_graphs": 25,
  "passed_graphs": 17,
  "correctness_records": 1836,
  "failed_correctness_records": 0,
  "issues": {
    "kron_g500-logn21": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "cage15": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "FullChip": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "scircuit": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "sx-stackoverflow": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "rajat31": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "road_usa": "",
    "com-Friendster": ""
  },
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862002",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/GRAPH_SUITE_BASE_RESULTS_20261003.md",
  "comparison_csv": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/GRAPH_SUITE_BASE_RESULTS_20261003.csv",
  "paper_eligible": false,
  "selection": "best B is posthoc oracle, not implemented adaptive dispatch",
  "next": "Audit high-degree results and nonunit datasets; shape/task/repeated-node evidence still pending",
  "date": "2026-10-03T13:34:15.889833+00:00"
}
```

## formal-10862162

```json
{
  "id": "formal-10862162",
  "kind": "formal",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10862162",
  "partition": "intel_expr",
  "node": "qhcn817",
  "threads": "32",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862162",
  "timing_boundary": "kernel includes per-method C-zero policy/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 10,
  "block_sweep": "1",
  "blocks": "2,4,8,16,32,64,full",
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "datasets": {
    "road_usa": "PASS"
  },
  "date": "2026-10-03T14:17:12.322084+00:00"
}
```

## formal-10862158

```json
{
  "id": "formal-10862158",
  "kind": "formal",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10862158",
  "partition": "intel_expr",
  "node": "qhcn819",
  "threads": "32",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862158",
  "timing_boundary": "kernel includes per-method C-zero policy/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion",
  "warmups": 1,
  "repeats": 5,
  "block_sweep": "1",
  "blocks": "8,16,32,64,full",
  "source_binary_hashes": "artifact.sha256 and build_source.sha256",
  "input_hashes": "dataset.sha256 plus per-graph info.json",
  "correctness": "per-graph correctness.csv; gate before timing",
  "results": "SUMMARY.md and per-graph timings/work/profiles CSV",
  "paper_eligible": false,
  "issues": "no new major issue",
  "next": "Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending",
  "datasets": {
    "com-Friendster": "PASS"
  },
  "date": "2026-10-03T14:41:41.223620+00:00"
}
```

## analysis-formal-10862002-graph_suite_results_20261003

```json
{
  "id": "analysis-formal-10862002-graph_suite_results_20261003",
  "kind": "analysis",
  "status": "PARTIAL",
  "attempted_graphs": 25,
  "passed_graphs": 19,
  "correctness_records": 2028,
  "failed_correctness_records": 0,
  "issues": {
    "kron_g500-logn21": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "cage15": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "FullChip": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "scircuit": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "sx-stackoverflow": "FATAL nonunit CSR values: reject unweighted Original TFS comparison",
    "rajat31": "FATAL nonunit CSR values: reject unweighted Original TFS comparison"
  },
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862002",
  "evidence_runs": [
    "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862002",
    "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862158",
    "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-10862162"
  ],
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/GRAPH_SUITE_RESULTS_20261003.md",
  "comparison_csv": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/GRAPH_SUITE_RESULTS_20261003.csv",
  "paper_eligible": false,
  "selection": "best B is posthoc oracle, not implemented adaptive dispatch",
  "next": "Completed all 25 canonical attempts; six nonunit-value inputs remain excluded. Investigate Full/B64 crossover and task/shape validation before paper claims.",
  "accounting": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/docs/GRAPH_SUITE_ACCOUNTING_20261003.txt",
  "figures": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/docs/figures/graph_suite",
  "performance": {
    "best_fast_posthoc_geomean": 1.35,
    "fixed_full_fast_geomean": 1.29,
    "best_accurate_posthoc_geomean": 1.297,
    "fast_faster_graphs": 17,
    "completed_graphs": 19
  },
  "resolved_issues": [
    "Road-USA and Friendster passed unchanged AMX/local kernels after MKL ILP64 recovery; historical LP64 preparation crashes remain preserved. Exact MKL fault line is unverified.",
    "Future LP64 supervisor skips E or N*128 beyond signed 32-bit limits.",
    "Future submissions explicitly set workspace cwd; child core dumps are disabled.",
    "Full does not dominate B64 on Mycielskian19; no claim of automatic B selection.",
    "Total gains include grouping, cross-output-panel gather reuse and empty-row output handling; causal shares need ablation."
  ],
  "date": "2026-10-03T14:44:28.887526+00:00"
}
```

## source-build-20261004-110400

```json
{
  "id": "source-build-20261004-110400",
  "kind": "source_protocol_build",
  "status": "FAILED",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-build-20261004-110400",
  "paper_eligible": false,
  "issues": "See build.log; source baseline and generated runner compile with original flags",
  "next": "shared source-protocol correctness gate",
  "date": "2026-10-04T03:04:02.172179+00:00"
}
```

## source-build-20261004-110419

```json
{
  "id": "source-build-20261004-110419",
  "kind": "source_protocol_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-build-20261004-110419",
  "paper_eligible": false,
  "issues": "See build.log; source baseline and generated runner compile with original flags",
  "next": "shared source-protocol correctness gate",
  "date": "2026-10-04T03:04:28.418326+00:00"
}
```

## source-submit-smoke-20261004-110448

```json
{
  "id": "source-submit-smoke-20261004-110448",
  "kind": "source_protocol_submission",
  "status": "SUBMITTED",
  "job_id": "10864752",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-submit-smoke-20261004-110448",
  "paper_eligible": false,
  "issues": "Original MKL/TFS environment, no numactl interleave for any path",
  "next": "Validate source protocol correctness before performance",
  "date": "2026-10-04T03:04:53.121925+00:00"
}
```

## source-smoke-10864752

```json
{
  "id": "source-smoke-10864752",
  "kind": "source_protocol_smoke",
  "status": "PASS",
  "exit_status": 0,
  "datasets": [
    {
      "graph": "source_tail",
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-10864752/fixtures/source_tail/source_tail.csrbin",
      "N": 37,
      "E": 538,
      "avg_degree": 14.54054054054054,
      "supported_header": true,
      "dataset_sha256": "ecb8742c5b148b14c6bad12975d8fb0da716686da6e6771264137ba475ba932d",
      "status": "PASS",
      "correctness_checks": 51,
      "elapsed_s": 0.26976919174194336
    },
    {
      "graph": "source_high_tail",
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-10864752/fixtures/source_high_tail/source_high_tail.csrbin",
      "N": 137,
      "E": 9826,
      "avg_degree": 71.72262773722628,
      "supported_header": true,
      "dataset_sha256": "b41c0169ce2328ff65db060b1782523ebefa59550de6aa6d4a394bf7a969ea61",
      "status": "PASS",
      "correctness_checks": 51,
      "elapsed_s": 0.3029911518096924
    }
  ],
  "job_id": "10864752",
  "node": "qhcn016",
  "threads": "4",
  "partition": "intel",
  "warmups": 1,
  "repeats": 5,
  "statistic": "source min-of-five, median secondary",
  "timing_boundary": "literal original prepared two-layer source MKL/TFS; separate candidate stages",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-10864752",
  "paper_eligible": false,
  "issues": "No new major issue; all methods use default NUMA policy, no explicit MKL thread controls",
  "next": "Compare original source MKL, source TFS and candidates; trained-model accuracy still unverified",
  "date": "2026-10-04T03:04:56.393445+00:00"
}
```

## source-submit-formal-20261004-110518

```json
{
  "id": "source-submit-formal-20261004-110518",
  "kind": "source_protocol_submission",
  "status": "SUBMITTED",
  "job_id": "10864754",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-submit-formal-20261004-110518",
  "paper_eligible": false,
  "issues": "Original MKL/TFS environment, no numactl interleave for any path",
  "next": "Validate source protocol correctness before performance",
  "date": "2026-10-04T03:05:19.641401+00:00"
}
```

## source-protocol-audit-20261004

```json
{
  "id": "source-protocol-audit-20261004",
  "kind": "source_and_runtime_audit",
  "status": "PASS",
  "paper_eligible": false,
  "evidence": "original/mkl_baseline.cpp, original/gcn_e2e_bench.cpp, original/gcn_e2e_v3.cpp, original/amx_tfs_v3.cpp, original/protocol_scripts, docs/SOURCE_MKL_PROTOCOL_20261004.md",
  "conclusion": "No explicit NUMA memory policy in original TFS or original MKL source and supplied scripts. DegreeSort is logical scheduling. Previous interleave applied to all methods, not just MKL.",
  "issues": [
    {
      "affected": "2026-10-03 graph suite",
      "root_cause": "Benchmark wrapper diverged from original source: explicit NUMA interleave, MKL thread controls, hint=16, initialized vectors, random draw order, fused ReLU/conversion, precision compilation flags, median-of-ten",
      "resolution": "Keep historical evidence; new generated harness preserves original source exactly except include and post-timing candidate hook; default NUMA for all paths, original buffer first writes, hint=10, five-run minimum",
      "validation": "Generation manifest confirms baseline recoverability; source build passed; smoke 10864752 passed 102 numerical checks; formal 10864754 running"
    },
    {
      "affected": "source-build-20261004-110400",
      "symptom": "TypeError: write_text() got an unexpected keyword argument 'newline'",
      "root_cause": "Server Python lacks newer pathlib.write_text newline argument",
      "resolution": "Use pathlib.Path.open with newline then write, preserving LF and compatibility",
      "validation": "source-build-20261004-110419 passed; preserve failed build directory"
    }
  ],
  "next": "Finish formal source-faithful comparison; reconcile all graphs, numerical gates, untouched binary anchors, hashes and three handoff documents",
  "date": "2026-10-04T03:09:00.774945+00:00"
}
```

## source-submit-formal-20261004-112618

```json
{
  "id": "source-submit-formal-20261004-112618",
  "kind": "source_protocol_submission",
  "status": "SUBMITTED",
  "job_id": "10864792",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-submit-formal-20261004-112618",
  "paper_eligible": false,
  "issues": "Original MKL/TFS environment, no numactl interleave for any path",
  "next": "Validate source protocol correctness before performance",
  "date": "2026-10-04T03:26:20.311561+00:00"
}
```

## source-rgg-boundary-correction-20261004

```json
{
  "id": "source-rgg-boundary-correction-20261004",
  "kind": "supervisor_range_audit",
  "status": "SUPPLEMENT_SUBMITTED",
  "paper_eligible": false,
  "affected": "source-formal-10864754/rgg_n_2_24_s0/status.json",
  "issues": "Supervisor incorrectly excluded N*128 == 2^31 using >=; maximum element index is N*128-1 and still fits INT32_MAX. Correct condition is > 2^31. No numerical kernel or compiler change.",
  "resolution": "Preserve first skip, submit supplemental source-formal-10864792 for RGG only with the same source binary and protocol. Current supervisor explicitly supports subsets and snapshots the actual launcher code separately from the build snapshot.",
  "next": "Reconcile first batch with RGG supplement without overwriting the first exclusion or mixing NUMA-interleaved historical evidence",
  "date": "2026-10-04T03:27:57.267309+00:00"
}
```

## source-formal-10864754

```json
{
  "id": "source-formal-10864754",
  "kind": "source_protocol_formal",
  "status": "FAILED",
  "exit_status": 2,
  "datasets": [
    {
      "graph": "mycielskian19",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/mycielskian19/mycielskian19.csrbin",
      "N": 393215,
      "E": 903194710,
      "avg_degree": 2296.9487684854344,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 7227130580,
      "live_estimate_gib": 11.79320190101862,
      "budget_gib": 18.741502376273274,
      "supported_header": true,
      "dataset_sha256": "e685700b64a07b5d871461ff9438126d4b4782f7bc57f25ed7045e250610cefe",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 471.24217319488525
    },
    {
      "graph": "reddit",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/reddit/reddit.csrbin",
      "N": 232965,
      "E": 114615892,
      "avg_degree": 491.9876032880476,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 917859036,
      "live_estimate_gib": 2.287652626633644,
      "budget_gib": 6.859565783292055,
      "supported_header": true,
      "dataset_sha256": "26ac2ddc04e52796ba175721fee4e376fcde46bff6ca6a8a4922cce6a296617a",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 65.66300058364868
    },
    {
      "graph": "hollywood-2009",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/hollywood-2009/hollywood-2009.csrbin",
      "N": 1139905,
      "E": 113891327,
      "avg_degree": 99.9129988902584,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 915690276,
      "live_estimate_gib": 6.198748130351305,
      "budget_gib": 11.748435162939131,
      "supported_header": true,
      "dataset_sha256": "e6fff5b1694adc03c6c26adc69f069f14af987075cc1c43f43d062e8636cd346",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 71.21698522567749
    },
    {
      "graph": "kron_g500-logn21",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/kron_g500-logn21/kron_g500-logn21.csrbin",
      "N": 2097152,
      "E": 182082942,
      "avg_degree": 86.82391262054443,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 1465052184,
      "live_estimate_gib": 11.09743545204401,
      "budget_gib": 17.871794315055013,
      "supported_header": true,
      "dataset_sha256": "71efce52f00bd37462152626bf3a8b5c780e29dc7e943603acd559c880d10151",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "com-Friendster",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-Friendster/com-Friendster.csrbin",
      "N": 65608366,
      "E": 3612134270,
      "avg_degree": 55.05600108986101,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 29159507664,
      "live_estimate_gib": 323.8845891132951,
      "budget_gib": 408.85573639161885,
      "supported_header": true,
      "dataset_sha256": "92cb33b8d41ac59be64b5770c9cd6f914c5bcb32d8a4be1085339ed9f7473315",
      "status": "SKIPPED",
      "reason": "Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol"
    },
    {
      "graph": "ogbn-products",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/ogbn-products/ogbn-products.csrbin",
      "N": 2449029,
      "E": 123718152,
      "avg_degree": 50.51722621496111,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 999541372,
      "live_estimate_gib": 11.965737104415894,
      "budget_gib": 18.957171380519867,
      "supported_header": true,
      "dataset_sha256": "b9662298403f24486a844d387ab4b6da9d6cafb3c3cf736050b4d695822ccc92",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 153.9632441997528
    },
    {
      "graph": "indochina-2004",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/indochina-2004/indochina-2004.csrbin",
      "N": 7414866,
      "E": 194109311,
      "avg_degree": 26.178397694577352,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 1582533992,
      "live_estimate_gib": 34.211473513394594,
      "budget_gib": 46.76434189174324,
      "supported_header": true,
      "dataset_sha256": "93877333cfaaa88bef37d14ed1c2926f912c95fad9094ff077cfa37293b1ac60",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 162.90507054328918
    },
    {
      "graph": "cage15",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/cage15/cage15.csrbin",
      "N": 5154859,
      "E": 99199551,
      "avg_degree": 19.24389221897243,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 814215884,
      "live_estimate_gib": 23.38452299311757,
      "budget_gib": 33.230653741396964,
      "supported_header": true,
      "dataset_sha256": "d8938ee41682cb03b9b383b219b0358f7d650bf86ad619795768a4abc35ef14a",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "soc-Pokec",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/soc-Pokec/soc-Pokec.csrbin",
      "N": 1632803,
      "E": 30622564,
      "avg_degree": 18.754598074599325,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 251511764,
      "live_estimate_gib": 7.398125424981117,
      "budget_gib": 13.247656781226397,
      "supported_header": true,
      "dataset_sha256": "246acce5100348468049a471b23e93e4a6aa0e45eef59d58de0bd67ca5e822d5",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 46.06695914268494
    },
    {
      "graph": "com-LiveJournal",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-LiveJournal/com-LiveJournal.csrbin",
      "N": 3997962,
      "E": 69362378,
      "avg_degree": 17.349434036641668,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 570890912,
      "live_estimate_gib": 18.051725082099438,
      "budget_gib": 26.564656352624297,
      "supported_header": true,
      "dataset_sha256": "6bc3fcc0678c7075e8307e43399c5989fed04b02ad0102ae985647b54b8acb8c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 118.3494861125946
    },
    {
      "graph": "rgg_n_2_24_s0",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/rgg_n_2_24_s0/rgg_n_2_24_s0.csrbin",
      "N": 16777216,
      "E": 265114400,
      "avg_degree": 15.80204963684082,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 2188024104,
      "live_estimate_gib": 75.46288430690765,
      "budget_gib": 98.32860538363457,
      "supported_header": true,
      "dataset_sha256": "46f71e04e30403826f3fddea83f1c182374f7dda8b1a632da67b91b5c4703d3c",
      "status": "SKIPPED",
      "reason": "Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol"
    },
    {
      "graph": "soc-LiveJournal1",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/soc-LiveJournal1/soc-LiveJournal1.csrbin",
      "N": 4847571,
      "E": 68993773,
      "avg_degree": 14.232648268586473,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 571340508,
      "live_estimate_gib": 21.71905219182372,
      "budget_gib": 31.14881523977965,
      "supported_header": true,
      "dataset_sha256": "b2935af5f6836753d1fa874cb8fe0c5828f776517f9c469dd01ab49acfc81886",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 143.63672471046448
    },
    {
      "graph": "as-Skitter",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/as-Skitter/as-Skitter.csrbin",
      "N": 1696415,
      "E": 22190596,
      "avg_degree": 13.080877025963575,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 184310468,
      "live_estimate_gib": 7.578779712319374,
      "budget_gib": 13.473474640399218,
      "supported_header": true,
      "dataset_sha256": "5d90b4dd437087b34b31c2f7bc6ca02607a43522d6b28f9741f1390101e13cbe",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 42.62102556228638
    },
    {
      "graph": "email-Enron",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/email-Enron/email-Enron.csrbin",
      "N": 36692,
      "E": 367662,
      "avg_degree": 10.020222391802028,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 3088104,
      "live_estimate_gib": 0.1626674309372902,
      "budget_gib": 4.203334288671613,
      "supported_header": true,
      "dataset_sha256": "6f16091c1ec66086a178258dd623e684605042f6c6212137b8b29743ab45d32c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 0.8992362022399902
    },
    {
      "graph": "FullChip",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/FullChip/FullChip.csrbin",
      "N": 2987012,
      "E": 26621990,
      "avg_degree": 8.91258220589673,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 224924008,
      "live_estimate_gib": 13.20540864020586,
      "budget_gib": 20.506760800257325,
      "supported_header": true,
      "dataset_sha256": "fbc32d0c9e7098f80762e76b5f0b6bffb18a7c114a80d43afbd5b0933d744e96",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "amazon0601",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/amazon0601/amazon0601.csrbin",
      "N": 403394,
      "E": 3387388,
      "avg_degree": 8.397219591763884,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 28712720,
      "live_estimate_gib": 1.7810583263635635,
      "budget_gib": 6.226322907954454,
      "supported_header": true,
      "dataset_sha256": "b8e8abf5adc129bfc3c65248e8acca03fa364481f37a9efc78b9a5c9bff1d1fb",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 7.39732813835144
    },
    {
      "graph": "scircuit",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/scircuit/scircuit.csrbin",
      "N": 170998,
      "E": 958936,
      "avg_degree": 5.607878454718769,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 8355520,
      "live_estimate_gib": 0.7496568858623505,
      "budget_gib": 4.937071107327938,
      "supported_header": true,
      "dataset_sha256": "ab8ccee80c59549e30f5f23282e1f8a8db551251ea2484a12e3e3570c4842a8f",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "web-Google",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/web-Google/web-Google.csrbin",
      "N": 916428,
      "E": 5105039,
      "avg_degree": 5.570583832008625,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 44506064,
      "live_estimate_gib": 4.017247248440981,
      "budget_gib": 9.021559060551226,
      "supported_header": true,
      "dataset_sha256": "db5dc3a8ea61feeb642628679c0a61ab1a301f948a68a00b29c871faf9e9c9a3",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 15.355893850326538
    },
    {
      "graph": "com-Youtube",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-Youtube/com-Youtube.csrbin",
      "N": 1134890,
      "E": 5975248,
      "avg_degree": 5.265045951590022,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 52341584,
      "live_estimate_gib": 4.971020460128784,
      "budget_gib": 10.21377557516098,
      "supported_header": true,
      "dataset_sha256": "0bc1778ef181f57cd1fa47185926abce9c05ca5f6898d67038b4286eea652015",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 26.787660837173462
    },
    {
      "graph": "cit-Patents",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/cit-Patents/cit-Patents.csrbin",
      "N": 3774768,
      "E": 16518948,
      "avg_degree": 4.376149209699775,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 147250696,
      "live_estimate_gib": 16.49665729701519,
      "budget_gib": 24.620821621268988,
      "supported_header": true,
      "dataset_sha256": "45f516b7e951c80d3c545d44ab8d6a02fe9a361007475990f263b908659721de",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 63.708494424819946
    },
    {
      "graph": "sx-stackoverflow",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/sx-stackoverflow/sx-stackoverflow.csrbin",
      "N": 2601977,
      "E": 11370342,
      "avg_degree": 4.3698856677057485,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 101370684,
      "live_estimate_gib": 11.371092297136784,
      "budget_gib": 18.21386537142098,
      "supported_header": true,
      "dataset_sha256": "c3530006006f1f1e7a956b392e7af00c69e4c17a720207c87be794f7f60f3619",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "rajat31",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/rajat31/rajat31.csrbin",
      "N": 4690002,
      "E": 20316253,
      "avg_degree": 4.331821820118627,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 181290072,
      "live_estimate_gib": 20.494129803031683,
      "budget_gib": 29.617662253789604,
      "supported_header": true,
      "dataset_sha256": "4783d72c423857fdadb072c057ad95323d9143d6373c142ff42a65b92f27c523",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "roadNet-CA",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/roadNet-CA/roadNet-CA.csrbin",
      "N": 1971281,
      "E": 5533214,
      "avg_degree": 2.806912865289119,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 52150876,
      "live_estimate_gib": 8.58040750771761,
      "budget_gib": 14.725509384647012,
      "supported_header": true,
      "dataset_sha256": "861d74683ef034b3fe719cf00770fcb2b600a862aa02ade084217d21f96da86c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 32.21322321891785
    },
    {
      "graph": "road_usa",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/road_usa/road_usa.csrbin",
      "N": 23947347,
      "E": 57708624,
      "avg_degree": 2.4098128281182882,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 557458420,
      "live_estimate_gib": 104.12949469685555,
      "budget_gib": 134.16186837106943,
      "supported_header": true,
      "dataset_sha256": "3549dd410122c681fa75b3d489554bf836f7ddab58dab282c336a1c052a0a34e",
      "status": "SKIPPED",
      "reason": "Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol"
    },
    {
      "graph": "wiki-Talk",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/wiki-Talk/wiki-Talk.csrbin",
      "N": 2394385,
      "E": 5021410,
      "avg_degree": 2.0971606487678462,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 49748860,
      "live_estimate_gib": 10.403062514960766,
      "budget_gib": 17.003828143700957,
      "supported_header": true,
      "dataset_sha256": "501f781d8950c277bd81dc436c458b40b852c0f72d1fcd186cc710e4a350f226",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 58.029242753982544
    }
  ],
  "job_id": "10864754",
  "node": "qhcn819",
  "threads": "32",
  "partition": "intel_expr",
  "warmups": 1,
  "repeats": 5,
  "statistic": "source min-of-five, median secondary",
  "timing_boundary": "literal original prepared two-layer source MKL/TFS; separate candidate stages",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-formal-10864754",
  "paper_eligible": false,
  "issues": "No new major issue; all methods use default NUMA policy, no explicit MKL thread controls",
  "next": "Compare original source MKL, source TFS and candidates; trained-model accuracy still unverified",
  "date": "2026-10-04T03:30:01.682303+00:00"
}
```

## source-launcher-tail-issue-20261004

```json
{
  "id": "source-launcher-tail-issue-20261004",
  "kind": "execution_failure_audit",
  "status": "FIX_PENDING_LIVE_JOB_COMPLETION",
  "paper_eligible": false,
  "affected": "source-formal-10864754",
  "issues": {
    "symptom": "Slurm launcher exited 2 with source_job.sh line 47 syntax error near json.dumps",
    "root_cause": "The live shell script was overwritten while its Python benchmark child was running; Bash then read the changed file tail after the child completed",
    "evidence": "runs/source-formal-10864754.out contains SOURCE_SUITE_COMPLETE 25 passed 16 before status=2; per-graph programs exit zero, output SOURCE_COMPLETE and all numeric checks pass; REPORT.md, summary.csv and all per-graph data were saved before the shell failure",
    "data_validity": "This is a launcher completion failure after all measured computation, not a numerical or timing failure. Retain original failed job status alongside individually valid graph results.",
    "resolution": "Do not edit the currently running RGG launcher. After it finishes, submission must snapshot both Slurm and shell/Python launcher files into a unique directory and execute those snapshots; validate on shared correctness fixtures."
  },
  "next": "Finish RGG, isolate future launchers, run correctness gate, reconcile failed batch completion separately from successful numerical data",
  "date": "2026-10-04T03:32:56.929408+00:00"
}
```

## source-formal-10864792

```json
{
  "id": "source-formal-10864792",
  "kind": "source_protocol_formal",
  "status": "PASS",
  "exit_status": 0,
  "datasets": [
    {
      "graph": "mycielskian19",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/mycielskian19/mycielskian19.csrbin",
      "N": 393215,
      "E": 903194710,
      "avg_degree": 2296.9487684854344,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 7227130580,
      "live_estimate_gib": 11.79320190101862,
      "budget_gib": 18.741502376273274,
      "supported_header": true,
      "dataset_sha256": "e685700b64a07b5d871461ff9438126d4b4782f7bc57f25ed7045e250610cefe",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "reddit",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/reddit/reddit.csrbin",
      "N": 232965,
      "E": 114615892,
      "avg_degree": 491.9876032880476,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 917859036,
      "live_estimate_gib": 2.287652626633644,
      "budget_gib": 6.859565783292055,
      "supported_header": true,
      "dataset_sha256": "26ac2ddc04e52796ba175721fee4e376fcde46bff6ca6a8a4922cce6a296617a",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "hollywood-2009",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/hollywood-2009/hollywood-2009.csrbin",
      "N": 1139905,
      "E": 113891327,
      "avg_degree": 99.9129988902584,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 915690276,
      "live_estimate_gib": 6.198748130351305,
      "budget_gib": 11.748435162939131,
      "supported_header": true,
      "dataset_sha256": "e6fff5b1694adc03c6c26adc69f069f14af987075cc1c43f43d062e8636cd346",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "kron_g500-logn21",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/kron_g500-logn21/kron_g500-logn21.csrbin",
      "N": 2097152,
      "E": 182082942,
      "avg_degree": 86.82391262054443,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 1465052184,
      "live_estimate_gib": 11.09743545204401,
      "budget_gib": 17.871794315055013,
      "supported_header": true,
      "dataset_sha256": "71efce52f00bd37462152626bf3a8b5c780e29dc7e943603acd559c880d10151",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "com-Friendster",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-Friendster/com-Friendster.csrbin",
      "N": 65608366,
      "E": 3612134270,
      "avg_degree": 55.05600108986101,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 29159507664,
      "live_estimate_gib": 323.8845891132951,
      "budget_gib": 408.85573639161885,
      "supported_header": true,
      "dataset_sha256": "92cb33b8d41ac59be64b5770c9cd6f914c5bcb32d8a4be1085339ed9f7473315",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "ogbn-products",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/ogbn-products/ogbn-products.csrbin",
      "N": 2449029,
      "E": 123718152,
      "avg_degree": 50.51722621496111,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 999541372,
      "live_estimate_gib": 11.965737104415894,
      "budget_gib": 18.957171380519867,
      "supported_header": true,
      "dataset_sha256": "b9662298403f24486a844d387ab4b6da9d6cafb3c3cf736050b4d695822ccc92",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "indochina-2004",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/indochina-2004/indochina-2004.csrbin",
      "N": 7414866,
      "E": 194109311,
      "avg_degree": 26.178397694577352,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 1582533992,
      "live_estimate_gib": 34.211473513394594,
      "budget_gib": 46.76434189174324,
      "supported_header": true,
      "dataset_sha256": "93877333cfaaa88bef37d14ed1c2926f912c95fad9094ff077cfa37293b1ac60",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "cage15",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/cage15/cage15.csrbin",
      "N": 5154859,
      "E": 99199551,
      "avg_degree": 19.24389221897243,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 814215884,
      "live_estimate_gib": 23.38452299311757,
      "budget_gib": 33.230653741396964,
      "supported_header": true,
      "dataset_sha256": "d8938ee41682cb03b9b383b219b0358f7d650bf86ad619795768a4abc35ef14a",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "soc-Pokec",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/soc-Pokec/soc-Pokec.csrbin",
      "N": 1632803,
      "E": 30622564,
      "avg_degree": 18.754598074599325,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 251511764,
      "live_estimate_gib": 7.398125424981117,
      "budget_gib": 13.247656781226397,
      "supported_header": true,
      "dataset_sha256": "246acce5100348468049a471b23e93e4a6aa0e45eef59d58de0bd67ca5e822d5",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "com-LiveJournal",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-LiveJournal/com-LiveJournal.csrbin",
      "N": 3997962,
      "E": 69362378,
      "avg_degree": 17.349434036641668,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 570890912,
      "live_estimate_gib": 18.051725082099438,
      "budget_gib": 26.564656352624297,
      "supported_header": true,
      "dataset_sha256": "6bc3fcc0678c7075e8307e43399c5989fed04b02ad0102ae985647b54b8acb8c",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "rgg_n_2_24_s0",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/rgg_n_2_24_s0/rgg_n_2_24_s0.csrbin",
      "N": 16777216,
      "E": 265114400,
      "avg_degree": 15.80204963684082,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 2188024104,
      "live_estimate_gib": 75.46288430690765,
      "budget_gib": 98.32860538363457,
      "supported_header": true,
      "dataset_sha256": "46f71e04e30403826f3fddea83f1c182374f7dda8b1a632da67b91b5c4703d3c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 407.4254672527313
    },
    {
      "graph": "soc-LiveJournal1",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/soc-LiveJournal1/soc-LiveJournal1.csrbin",
      "N": 4847571,
      "E": 68993773,
      "avg_degree": 14.232648268586473,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 571340508,
      "live_estimate_gib": 21.71905219182372,
      "budget_gib": 31.14881523977965,
      "supported_header": true,
      "dataset_sha256": "b2935af5f6836753d1fa874cb8fe0c5828f776517f9c469dd01ab49acfc81886",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "as-Skitter",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/as-Skitter/as-Skitter.csrbin",
      "N": 1696415,
      "E": 22190596,
      "avg_degree": 13.080877025963575,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 184310468,
      "live_estimate_gib": 7.578779712319374,
      "budget_gib": 13.473474640399218,
      "supported_header": true,
      "dataset_sha256": "5d90b4dd437087b34b31c2f7bc6ca02607a43522d6b28f9741f1390101e13cbe",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "email-Enron",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/email-Enron/email-Enron.csrbin",
      "N": 36692,
      "E": 367662,
      "avg_degree": 10.020222391802028,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 3088104,
      "live_estimate_gib": 0.1626674309372902,
      "budget_gib": 4.203334288671613,
      "supported_header": true,
      "dataset_sha256": "6f16091c1ec66086a178258dd623e684605042f6c6212137b8b29743ab45d32c",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "FullChip",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/FullChip/FullChip.csrbin",
      "N": 2987012,
      "E": 26621990,
      "avg_degree": 8.91258220589673,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 224924008,
      "live_estimate_gib": 13.20540864020586,
      "budget_gib": 20.506760800257325,
      "supported_header": true,
      "dataset_sha256": "fbc32d0c9e7098f80762e76b5f0b6bffb18a7c114a80d43afbd5b0933d744e96",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "amazon0601",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/amazon0601/amazon0601.csrbin",
      "N": 403394,
      "E": 3387388,
      "avg_degree": 8.397219591763884,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 28712720,
      "live_estimate_gib": 1.7810583263635635,
      "budget_gib": 6.226322907954454,
      "supported_header": true,
      "dataset_sha256": "b8e8abf5adc129bfc3c65248e8acca03fa364481f37a9efc78b9a5c9bff1d1fb",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "scircuit",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/scircuit/scircuit.csrbin",
      "N": 170998,
      "E": 958936,
      "avg_degree": 5.607878454718769,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 8355520,
      "live_estimate_gib": 0.7496568858623505,
      "budget_gib": 4.937071107327938,
      "supported_header": true,
      "dataset_sha256": "ab8ccee80c59549e30f5f23282e1f8a8db551251ea2484a12e3e3570c4842a8f",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "web-Google",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/web-Google/web-Google.csrbin",
      "N": 916428,
      "E": 5105039,
      "avg_degree": 5.570583832008625,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 44506064,
      "live_estimate_gib": 4.017247248440981,
      "budget_gib": 9.021559060551226,
      "supported_header": true,
      "dataset_sha256": "db5dc3a8ea61feeb642628679c0a61ab1a301f948a68a00b29c871faf9e9c9a3",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "com-Youtube",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-Youtube/com-Youtube.csrbin",
      "N": 1134890,
      "E": 5975248,
      "avg_degree": 5.265045951590022,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 52341584,
      "live_estimate_gib": 4.971020460128784,
      "budget_gib": 10.21377557516098,
      "supported_header": true,
      "dataset_sha256": "0bc1778ef181f57cd1fa47185926abce9c05ca5f6898d67038b4286eea652015",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "cit-Patents",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/cit-Patents/cit-Patents.csrbin",
      "N": 3774768,
      "E": 16518948,
      "avg_degree": 4.376149209699775,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 147250696,
      "live_estimate_gib": 16.49665729701519,
      "budget_gib": 24.620821621268988,
      "supported_header": true,
      "dataset_sha256": "45f516b7e951c80d3c545d44ab8d6a02fe9a361007475990f263b908659721de",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "sx-stackoverflow",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/sx-stackoverflow/sx-stackoverflow.csrbin",
      "N": 2601977,
      "E": 11370342,
      "avg_degree": 4.3698856677057485,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 101370684,
      "live_estimate_gib": 11.371092297136784,
      "budget_gib": 18.21386537142098,
      "supported_header": true,
      "dataset_sha256": "c3530006006f1f1e7a956b392e7af00c69e4c17a720207c87be794f7f60f3619",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "rajat31",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/rajat31/rajat31.csrbin",
      "N": 4690002,
      "E": 20316253,
      "avg_degree": 4.331821820118627,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 181290072,
      "live_estimate_gib": 20.494129803031683,
      "budget_gib": 29.617662253789604,
      "supported_header": true,
      "dataset_sha256": "4783d72c423857fdadb072c057ad95323d9143d6373c142ff42a65b92f27c523",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "roadNet-CA",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/roadNet-CA/roadNet-CA.csrbin",
      "N": 1971281,
      "E": 5533214,
      "avg_degree": 2.806912865289119,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 52150876,
      "live_estimate_gib": 8.58040750771761,
      "budget_gib": 14.725509384647012,
      "supported_header": true,
      "dataset_sha256": "861d74683ef034b3fe719cf00770fcb2b600a862aa02ade084217d21f96da86c",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "road_usa",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/road_usa/road_usa.csrbin",
      "N": 23947347,
      "E": 57708624,
      "avg_degree": 2.4098128281182882,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 557458420,
      "live_estimate_gib": 104.12949469685555,
      "budget_gib": 134.16186837106943,
      "supported_header": true,
      "dataset_sha256": "3549dd410122c681fa75b3d489554bf836f7ddab58dab282c336a1c052a0a34e",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    },
    {
      "graph": "wiki-Talk",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/wiki-Talk/wiki-Talk.csrbin",
      "N": 2394385,
      "E": 5021410,
      "avg_degree": 2.0971606487678462,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 49748860,
      "live_estimate_gib": 10.403062514960766,
      "budget_gib": 17.003828143700957,
      "supported_header": true,
      "dataset_sha256": "501f781d8950c277bd81dc436c458b40b852c0f72d1fcd186cc710e4a350f226",
      "status": "SKIPPED",
      "reason": "Outside the explicitly requested supplemental graph subset"
    }
  ],
  "job_id": "10864792",
  "node": "qhcn819",
  "threads": "32",
  "partition": "intel_expr",
  "warmups": 1,
  "repeats": 5,
  "statistic": "source min-of-five, median secondary",
  "timing_boundary": "literal original prepared two-layer source MKL/TFS; separate candidate stages",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-formal-10864792",
  "paper_eligible": false,
  "issues": "No new major issue; all methods use default NUMA policy, no explicit MKL thread controls",
  "next": "Compare original source MKL, source TFS and candidates; trained-model accuracy still unverified",
  "date": "2026-10-04T03:36:51.479917+00:00"
}
```

## source-submit-smoke-20261004-113831

```json
{
  "id": "source-submit-smoke-20261004-113831",
  "kind": "source_protocol_submission",
  "status": "SUBMITTED",
  "job_id": "10864830",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-submit-smoke-20261004-113831",
  "paper_eligible": false,
  "issues": "Original MKL/TFS environment, no numactl interleave for any path",
  "next": "Validate source protocol correctness before performance",
  "date": "2026-10-04T03:38:33.001693+00:00"
}
```

## source-smoke-10864830

```json
{
  "id": "source-smoke-10864830",
  "kind": "source_protocol_smoke",
  "status": "PASS",
  "exit_status": 0,
  "datasets": [
    {
      "graph": "source_tail",
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-10864830/fixtures/source_tail/source_tail.csrbin",
      "N": 37,
      "E": 538,
      "avg_degree": 14.54054054054054,
      "supported_header": true,
      "dataset_sha256": "ecb8742c5b148b14c6bad12975d8fb0da716686da6e6771264137ba475ba932d",
      "status": "PASS",
      "correctness_checks": 51,
      "elapsed_s": 0.5233545303344727
    },
    {
      "graph": "source_high_tail",
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-10864830/fixtures/source_high_tail/source_high_tail.csrbin",
      "N": 137,
      "E": 9826,
      "avg_degree": 71.72262773722628,
      "supported_header": true,
      "dataset_sha256": "b41c0169ce2328ff65db060b1782523ebefa59550de6aa6d4a394bf7a969ea61",
      "status": "PASS",
      "correctness_checks": 51,
      "elapsed_s": 0.2736811637878418
    }
  ],
  "job_id": "10864830",
  "node": "qhcn105",
  "threads": "4",
  "partition": "intel",
  "warmups": 1,
  "repeats": 5,
  "statistic": "source min-of-five, median secondary",
  "timing_boundary": "literal original prepared two-layer source MKL/TFS; separate candidate stages",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-10864830",
  "paper_eligible": false,
  "issues": "No new major issue; all methods use default NUMA policy, no explicit MKL thread controls",
  "next": "Compare original source MKL, source TFS and candidates; trained-model accuracy still unverified",
  "date": "2026-10-04T03:38:38.876495+00:00"
}
```

## source-comparison-final-source-formal-10864754

```json
{
  "id": "source-comparison-final-source-formal-10864754",
  "kind": "source_protocol_reconciliation",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-formal-10864754",
  "report": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/docs/SOURCE_MKL_RESULTS_20261004.md",
  "paper_eligible": false,
  "checks": 765,
  "datasets": [
    {
      "graph": "mycielskian19",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/mycielskian19/mycielskian19.csrbin",
      "N": 393215,
      "E": 903194710,
      "avg_degree": 2296.9487684854344,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 7227130580,
      "live_estimate_gib": 11.79320190101862,
      "budget_gib": 18.741502376273274,
      "supported_header": true,
      "dataset_sha256": "e685700b64a07b5d871461ff9438126d4b4782f7bc57f25ed7045e250610cefe",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 471.24217319488525
    },
    {
      "graph": "reddit",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/reddit/reddit.csrbin",
      "N": 232965,
      "E": 114615892,
      "avg_degree": 491.9876032880476,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 917859036,
      "live_estimate_gib": 2.287652626633644,
      "budget_gib": 6.859565783292055,
      "supported_header": true,
      "dataset_sha256": "26ac2ddc04e52796ba175721fee4e376fcde46bff6ca6a8a4922cce6a296617a",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 65.66300058364868
    },
    {
      "graph": "hollywood-2009",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/hollywood-2009/hollywood-2009.csrbin",
      "N": 1139905,
      "E": 113891327,
      "avg_degree": 99.9129988902584,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 915690276,
      "live_estimate_gib": 6.198748130351305,
      "budget_gib": 11.748435162939131,
      "supported_header": true,
      "dataset_sha256": "e6fff5b1694adc03c6c26adc69f069f14af987075cc1c43f43d062e8636cd346",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 71.21698522567749
    },
    {
      "graph": "kron_g500-logn21",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/kron_g500-logn21/kron_g500-logn21.csrbin",
      "N": 2097152,
      "E": 182082942,
      "avg_degree": 86.82391262054443,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 1465052184,
      "live_estimate_gib": 11.09743545204401,
      "budget_gib": 17.871794315055013,
      "supported_header": true,
      "dataset_sha256": "71efce52f00bd37462152626bf3a8b5c780e29dc7e943603acd559c880d10151",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "com-Friendster",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-Friendster/com-Friendster.csrbin",
      "N": 65608366,
      "E": 3612134270,
      "avg_degree": 55.05600108986101,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 29159507664,
      "live_estimate_gib": 323.8845891132951,
      "budget_gib": 408.85573639161885,
      "supported_header": true,
      "dataset_sha256": "92cb33b8d41ac59be64b5770c9cd6f914c5bcb32d8a4be1085339ed9f7473315",
      "status": "SKIPPED",
      "reason": "Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol"
    },
    {
      "graph": "ogbn-products",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/ogbn-products/ogbn-products.csrbin",
      "N": 2449029,
      "E": 123718152,
      "avg_degree": 50.51722621496111,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 999541372,
      "live_estimate_gib": 11.965737104415894,
      "budget_gib": 18.957171380519867,
      "supported_header": true,
      "dataset_sha256": "b9662298403f24486a844d387ab4b6da9d6cafb3c3cf736050b4d695822ccc92",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 153.9632441997528
    },
    {
      "graph": "indochina-2004",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/indochina-2004/indochina-2004.csrbin",
      "N": 7414866,
      "E": 194109311,
      "avg_degree": 26.178397694577352,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 1582533992,
      "live_estimate_gib": 34.211473513394594,
      "budget_gib": 46.76434189174324,
      "supported_header": true,
      "dataset_sha256": "93877333cfaaa88bef37d14ed1c2926f912c95fad9094ff077cfa37293b1ac60",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 162.90507054328918
    },
    {
      "graph": "cage15",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/cage15/cage15.csrbin",
      "N": 5154859,
      "E": 99199551,
      "avg_degree": 19.24389221897243,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 814215884,
      "live_estimate_gib": 23.38452299311757,
      "budget_gib": 33.230653741396964,
      "supported_header": true,
      "dataset_sha256": "d8938ee41682cb03b9b383b219b0358f7d650bf86ad619795768a4abc35ef14a",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "soc-Pokec",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/soc-Pokec/soc-Pokec.csrbin",
      "N": 1632803,
      "E": 30622564,
      "avg_degree": 18.754598074599325,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 251511764,
      "live_estimate_gib": 7.398125424981117,
      "budget_gib": 13.247656781226397,
      "supported_header": true,
      "dataset_sha256": "246acce5100348468049a471b23e93e4a6aa0e45eef59d58de0bd67ca5e822d5",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 46.06695914268494
    },
    {
      "graph": "com-LiveJournal",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-LiveJournal/com-LiveJournal.csrbin",
      "N": 3997962,
      "E": 69362378,
      "avg_degree": 17.349434036641668,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 570890912,
      "live_estimate_gib": 18.051725082099438,
      "budget_gib": 26.564656352624297,
      "supported_header": true,
      "dataset_sha256": "6bc3fcc0678c7075e8307e43399c5989fed04b02ad0102ae985647b54b8acb8c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 118.3494861125946
    },
    {
      "graph": "rgg_n_2_24_s0",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/rgg_n_2_24_s0/rgg_n_2_24_s0.csrbin",
      "N": 16777216,
      "E": 265114400,
      "avg_degree": 15.80204963684082,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 2188024104,
      "live_estimate_gib": 75.46288430690765,
      "budget_gib": 98.32860538363457,
      "supported_header": true,
      "dataset_sha256": "46f71e04e30403826f3fddea83f1c182374f7dda8b1a632da67b91b5c4703d3c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 407.4254672527313,
      "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-formal-10864792/rgg_n_2_24_s0"
    },
    {
      "graph": "soc-LiveJournal1",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/soc-LiveJournal1/soc-LiveJournal1.csrbin",
      "N": 4847571,
      "E": 68993773,
      "avg_degree": 14.232648268586473,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 571340508,
      "live_estimate_gib": 21.71905219182372,
      "budget_gib": 31.14881523977965,
      "supported_header": true,
      "dataset_sha256": "b2935af5f6836753d1fa874cb8fe0c5828f776517f9c469dd01ab49acfc81886",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 143.63672471046448
    },
    {
      "graph": "as-Skitter",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/as-Skitter/as-Skitter.csrbin",
      "N": 1696415,
      "E": 22190596,
      "avg_degree": 13.080877025963575,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 184310468,
      "live_estimate_gib": 7.578779712319374,
      "budget_gib": 13.473474640399218,
      "supported_header": true,
      "dataset_sha256": "5d90b4dd437087b34b31c2f7bc6ca02607a43522d6b28f9741f1390101e13cbe",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 42.62102556228638
    },
    {
      "graph": "email-Enron",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/email-Enron/email-Enron.csrbin",
      "N": 36692,
      "E": 367662,
      "avg_degree": 10.020222391802028,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 3088104,
      "live_estimate_gib": 0.1626674309372902,
      "budget_gib": 4.203334288671613,
      "supported_header": true,
      "dataset_sha256": "6f16091c1ec66086a178258dd623e684605042f6c6212137b8b29743ab45d32c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 0.8992362022399902
    },
    {
      "graph": "FullChip",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/FullChip/FullChip.csrbin",
      "N": 2987012,
      "E": 26621990,
      "avg_degree": 8.91258220589673,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 224924008,
      "live_estimate_gib": 13.20540864020586,
      "budget_gib": 20.506760800257325,
      "supported_header": true,
      "dataset_sha256": "fbc32d0c9e7098f80762e76b5f0b6bffb18a7c114a80d43afbd5b0933d744e96",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "amazon0601",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/amazon0601/amazon0601.csrbin",
      "N": 403394,
      "E": 3387388,
      "avg_degree": 8.397219591763884,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 28712720,
      "live_estimate_gib": 1.7810583263635635,
      "budget_gib": 6.226322907954454,
      "supported_header": true,
      "dataset_sha256": "b8e8abf5adc129bfc3c65248e8acca03fa364481f37a9efc78b9a5c9bff1d1fb",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 7.39732813835144
    },
    {
      "graph": "scircuit",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/scircuit/scircuit.csrbin",
      "N": 170998,
      "E": 958936,
      "avg_degree": 5.607878454718769,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 8355520,
      "live_estimate_gib": 0.7496568858623505,
      "budget_gib": 4.937071107327938,
      "supported_header": true,
      "dataset_sha256": "ab8ccee80c59549e30f5f23282e1f8a8db551251ea2484a12e3e3570c4842a8f",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "web-Google",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/web-Google/web-Google.csrbin",
      "N": 916428,
      "E": 5105039,
      "avg_degree": 5.570583832008625,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 44506064,
      "live_estimate_gib": 4.017247248440981,
      "budget_gib": 9.021559060551226,
      "supported_header": true,
      "dataset_sha256": "db5dc3a8ea61feeb642628679c0a61ab1a301f948a68a00b29c871faf9e9c9a3",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 15.355893850326538
    },
    {
      "graph": "com-Youtube",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/com-Youtube/com-Youtube.csrbin",
      "N": 1134890,
      "E": 5975248,
      "avg_degree": 5.265045951590022,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 52341584,
      "live_estimate_gib": 4.971020460128784,
      "budget_gib": 10.21377557516098,
      "supported_header": true,
      "dataset_sha256": "0bc1778ef181f57cd1fa47185926abce9c05ca5f6898d67038b4286eea652015",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 26.787660837173462
    },
    {
      "graph": "cit-Patents",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/cit-Patents/cit-Patents.csrbin",
      "N": 3774768,
      "E": 16518948,
      "avg_degree": 4.376149209699775,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 147250696,
      "live_estimate_gib": 16.49665729701519,
      "budget_gib": 24.620821621268988,
      "supported_header": true,
      "dataset_sha256": "45f516b7e951c80d3c545d44ab8d6a02fe9a361007475990f263b908659721de",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 63.708494424819946
    },
    {
      "graph": "sx-stackoverflow",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/sx-stackoverflow/sx-stackoverflow.csrbin",
      "N": 2601977,
      "E": 11370342,
      "avg_degree": 4.3698856677057485,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 101370684,
      "live_estimate_gib": 11.371092297136784,
      "budget_gib": 18.21386537142098,
      "supported_header": true,
      "dataset_sha256": "c3530006006f1f1e7a956b392e7af00c69e4c17a720207c87be794f7f60f3619",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "rajat31",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/rajat31/rajat31.csrbin",
      "N": 4690002,
      "E": 20316253,
      "avg_degree": 4.331821820118627,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 181290072,
      "live_estimate_gib": 20.494129803031683,
      "budget_gib": 29.617662253789604,
      "supported_header": true,
      "dataset_sha256": "4783d72c423857fdadb072c057ad95323d9143d6373c142ff42a65b92f27c523",
      "status": "SKIPPED",
      "reason": "Previously verified nonunit CSR values; original TFS ignores values while MKL uses them"
    },
    {
      "graph": "roadNet-CA",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/roadNet-CA/roadNet-CA.csrbin",
      "N": 1971281,
      "E": 5533214,
      "avg_degree": 2.806912865289119,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 52150876,
      "live_estimate_gib": 8.58040750771761,
      "budget_gib": 14.725509384647012,
      "supported_header": true,
      "dataset_sha256": "861d74683ef034b3fe719cf00770fcb2b600a862aa02ade084217d21f96da86c",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 32.21322321891785
    },
    {
      "graph": "road_usa",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/road_usa/road_usa.csrbin",
      "N": 23947347,
      "E": 57708624,
      "avg_degree": 2.4098128281182882,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 557458420,
      "live_estimate_gib": 104.12949469685555,
      "budget_gib": 134.16186837106943,
      "supported_header": true,
      "dataset_sha256": "3549dd410122c681fa75b3d489554bf836f7ddab58dab282c336a1c052a0a34e",
      "status": "SKIPPED",
      "reason": "Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol"
    },
    {
      "graph": "wiki-Talk",
      "path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/wiki-Talk/wiki-Talk.csrbin",
      "N": 2394385,
      "E": 5021410,
      "avg_degree": 2.0971606487678462,
      "format": "0,0,2",
      "square": true,
      "size_bytes": 49748860,
      "live_estimate_gib": 10.403062514960766,
      "budget_gib": 17.003828143700957,
      "supported_header": true,
      "dataset_sha256": "501f781d8950c277bd81dc436c458b40b852c0f72d1fcd186cc710e4a350f226",
      "status": "PASS",
      "correctness_checks": 45,
      "elapsed_s": 58.029242753982544
    }
  ],
  "issues": "Previous explicit NUMA interleave and other protocol changes are now isolated from source-faithful results; LP64 limits and nonunit values remain excluded",
  "next": "Use source-aligned MKL/TFS table for comparisons; independent fixed-policy validation and trained-model accuracy remain future work",
  "date": "2026-10-04T03:39:05.288031+00:00"
}
```

## source-launcher-validation-10864830

```json
{
  "id": "source-launcher-validation-10864830",
  "kind": "launcher_isolation_correctness_gate",
  "status": "PASS",
  "paper_eligible": false,
  "evidence": "runs/source-submit-smoke-20261004-113831/launcher, runs/source-smoke-10864830/launcher_snapshot, runs/source-smoke-10864830.out, runs/source-smoke-10864830/suite_status.json",
  "correctness_checks": 102,
  "issues": "First main batch failed during shell completion after measured programs succeeded. Do not rewrite that FAILED state.",
  "resolution": "Submission snapshots Slurm, shell and Python launchers into a unique directory, pins build path, and launches snapshots. Post-fix smoke 10864830 ran both empty-row/tail fixtures and finished with exit_status=0.",
  "next": "Archive new source-aligned evidence including failures and range correction; use immutable source-protocol launcher for future comparisons",
  "date": "2026-10-04T03:40:01.741796+00:00"
}
```

## paper-build-20261004-120429

```json
{
  "id": "paper-build-20261004-120429",
  "kind": "paper_source_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-build-20261004-120429",
  "paper_eligible": false,
  "next": "correctness-first paper protocol reproduction",
  "issues": "Earlier source comparison selected gcn_e2e_bench.cpp; paper Table 2 matches gcn_e2e_v3.cpp, FP32 intermediate and BF16 final output.",
  "date": "2026-10-04T04:04:36.058566+00:00"
}
```

## paper-submit-smoke-20261004-120508

```json
{
  "id": "paper-submit-smoke-20261004-120508",
  "kind": "paper_reproduction_submission",
  "status": "SUBMITTED",
  "job_id": "10864847",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-submit-smoke-20261004-120508",
  "paper_eligible": false,
  "next": "Wait for correctness smoke before formal submission",
  "date": "2026-10-04T04:05:10.052587+00:00"
}
```

## paper-smoke-10864847

```json
{
  "id": "paper-smoke-10864847",
  "kind": "paper_reproduction",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864847",
  "node": "qhcn016",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-build-20261004-120429",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-smoke-10864847",
  "completion": {
    "passed": 2,
    "excluded": 0,
    "failed": 0,
    "mode": "smoke"
  },
  "paper_eligible": false,
  "issues": "Unmodified gcn_e2e_v3 paper version; no added NUMA/MKL controls; report both source minimum and paper-described median. Separate full BF16 final-output gate precedes raw performance.",
  "date": "2026-10-04T04:05:12.515194+00:00"
}
```

## paper-submit-formal-20261004-120551

```json
{
  "id": "paper-submit-formal-20261004-120551",
  "kind": "paper_reproduction_submission",
  "status": "SUBMITTED",
  "job_id": "10864848",
  "mode": "formal",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-submit-formal-20261004-120551",
  "paper_eligible": false,
  "next": "Reconcile raw paper repetitions against historical paper logs",
  "date": "2026-10-04T04:05:53.321918+00:00"
}
```

## paper-assets-20261004-120700

```json
{
  "id": "paper-assets-20261004-120700",
  "kind": "paper_real_asset_audit",
  "status": "READ_ONLY_AUDIT_COMPLETE",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-assets-20261004-120700",
  "paper_eligible": false,
  "issues": "Available real features have 602/100 input dimensions; single-step W snapshots are not complete model checkpoints. Paper performance reproduction must first use source-defined random 128-dimensional input.",
  "next": "Reproduce original paper source, keep checkpoint inference as a separate protocol",
  "date": "2026-10-04T04:07:47.267522+00:00"
}
```

## paper-formal-10864848

```json
{
  "id": "paper-formal-10864848",
  "kind": "paper_reproduction",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864848",
  "node": "qhcn817",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-build-20261004-120429",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-formal-10864848",
  "completion": {
    "passed": 17,
    "excluded": 8,
    "failed": 0,
    "mode": "formal"
  },
  "paper_eligible": true,
  "issues": "Unmodified gcn_e2e_v3 paper version; no added NUMA/MKL controls; report both source minimum and paper-described median. Separate full BF16 final-output gate precedes raw performance.",
  "date": "2026-10-04T04:15:16.288115+00:00"
}
```

## paper-reconciliation-20261004-10864848

```json
{
  "id": "paper-reconciliation-20261004-10864848",
  "kind": "paper_reproduction_reconciliation",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-formal-10864848",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/PAPER_REPRODUCTION_RESULTS_20261004.md",
  "statistics": {
    "completion": {
      "passed": 17,
      "excluded": 8,
      "failed": 0,
      "mode": "formal"
    },
    "node": "qhcn817",
    "all17_kernel_min_gmean": 2.483165974812451,
    "all17_e2e_min_gmean": 1.9061598602772976,
    "all17_kernel_median_gmean": 2.4298650671360496,
    "all17_e2e_median_gmean": 1.885919052875715,
    "e2e_faster": 15,
    "paper_winner15_e2e_min_gmean": 2.1631839114866036,
    "max_final_bf16_relative_L2": 0.0132329377,
    "max_final_bf16_normalized_max": 0.016461868,
    "paper_timing_baseline": "unmodified gcn_e2e_v3.cpp",
    "prior_older_source": "gcn_e2e_bench.cpp; previous claim about paper original TFS slower is withdrawn",
    "paper_full25_reproduction": false,
    "reason": "17 numerically comparable graphs reproduced. Six nonunit CSR graphs have unequal source operators; two graphs exceed original LP64 safe limits."
  },
  "paper_eligible": true,
  "issues": "Correct source version reproduces major paper speedups; prior conclusion about paper original TFS slower withdrawn. Historical min/median and FP32-check/BF16-timed output discrepancies explicitly audited.",
  "next": "Any reduction extension must first rebase on this gcn_e2e_v3 baseline, with the same BF16 final output and source MKL; trained-checkpoint inference remains a separate task.",
  "date": "2026-10-04T04:16:38.044563+00:00"
}
```

## paper-package-20261004-10864848

```json
{
  "id": "paper-package-20261004-10864848",
  "kind": "paper_reproduction_archive",
  "status": "VERIFIED_RUN_READY_FOR_ARCHIVE",
  "archive": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/evidence-paper-reproduction-20261004.tar.gz",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-formal-10864848",
  "paper_eligible": true,
  "issues": "No new issue; baseline version correction and historical discrepancies retained",
  "next": "Mirror archive, verify hashes, publish on existing isolated experiment branch",
  "date": "2026-10-04T04:17:22.833859+00:00"
}
```
