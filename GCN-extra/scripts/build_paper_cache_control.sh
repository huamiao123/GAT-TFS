#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
OLD="$ROOT/runs/paper-method-build-20261004-123445"
RUN="$ROOT/runs/paper-cache-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name,kind='paper_cache_control_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Interleaved method execution changes small-graph cache warmth, especially FP32 MKL; supplemental source-equivalent warmup plus five consecutive repetitions. Frozen kernels and gates unchanged.',next='Shared smoke then all-17 paired cache control')
(r/'event.json').write_text(json.dumps(e,indent=2))
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
 printf 'CACHE_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$OLD/source_snapshot/"* "$RUN/source_snapshot/"
cp "$ROOT/scripts/build_paper_cache_control.sh" "$RUN/source_snapshot/"
python3 - "$RUN" "$OLD" <<'PY'
import pathlib,sys,json,hashlib
r=pathlib.Path(sys.argv[1]);old=pathlib.Path(sys.argv[2]);p=r/'source_snapshot/paper_methods_runtime.hpp';text=p.read_text();needle='    for(bool e2e:{false,true}){';assert text.count(needle)==1
extra=r'''
    // Supplemental cache control: same source one-warmup/five-consecutive policy.
    for(size_t k=0;k<=methods.size();k++){
        run(k,true);
        for(int rep=0;rep<5;rep++){
            double t=omp_get_wtime();run(k,true);double ms=(omp_get_wtime()-t)*1000;
            printf("METHOD_TIME graph=%s method=%s kind=e2e repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,k,ms);fflush(stdout);
        }
    }
    printf("METHOD_COMPLETE graph=%s methods=%zu checks=%zu pass=1 max_rss_kib=0\n",graph,methods.size(),methods.size()*4);fflush(stdout);
    return;
'''
generated=text.replace(needle,extra+'\n'+needle);assert generated.replace(extra+'\n','')==text;p.write_text(generated)
kernel=r/'source_snapshot/paper_methods_kernels.hpp';assert kernel.read_bytes()==(old/'source_snapshot/paper_methods_kernels.hpp').read_bytes()
(r/'cache_generation.json').write_text(json.dumps(dict(frozen_kernel_sha256=hashlib.sha256(kernel.read_bytes()).hexdigest(),all_numerical_gates_unchanged=True,change='Only supplemental timing loop; one immediate warmup and five consecutive measurements per method. Existing rotating measurements preserved separately.',scope='B8/B16/B64/FULL fast and accurate, original TFS and MKL; all 17 eligible graphs'),indent=2))
PY
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
icpx "${FLAGS[@]}" "$RUN/source_snapshot/paper_methods.cpp" -o "$RUN/paper_methods" > "$RUN/build.log" 2>&1
cp "$OLD/paper_original" "$RUN/paper_original"
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/paper_methods" "$RUN/paper_original" > "$RUN/binary.sha256"
printf '%s\n' "$RUN" > "$ROOT/build/PAPER_CACHE_LATEST"
