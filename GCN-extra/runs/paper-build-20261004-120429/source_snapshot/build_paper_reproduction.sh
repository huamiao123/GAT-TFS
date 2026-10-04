#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
REF=/home/huangjianqiang_group/hdacp1/data/yx/TFS
RUN="$ROOT/runs/paper-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/reference_logs" "$RUN/reference_scripts"
finish() {
  status=$?
  python3 - "$RUN" "$status" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2])
e=dict(id=r.name,kind='paper_source_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,next='correctness-first paper protocol reproduction',issues='Earlier source comparison selected gcn_e2e_bench.cpp; paper Table 2 matches gcn_e2e_v3.cpp, FP32 intermediate and BF16 final output.')
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'status.txt').write_text(str(s)+'\n')
PY
  python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
  printf 'PAPER_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
for name in gcn_e2e_v3.cpp amx_tfs_v3.cpp mkl_baseline.cpp; do
  cp "$REF/code/$name" "$RUN/source_snapshot/$name"
  cmp "$REF/code/$name" "$RUN/source_snapshot/$name"
done
for name in all_v3_4624074.log all_mkl_4624075.log gcn_e2e_v3_all_4624352.log; do
  cp "$REF/logs/$name" "$RUN/reference_logs/$name"
done
for name in compile_e2e_v3.sh compile_tfs_v3.sh compile_mkl_baseline.sh run_e2e_v3_all.slurm run_all_v3.slurm run_all_mkl.slurm; do
  cp "$REF/scripts/$name" "$RUN/reference_scripts/$name"
done
cp "$ROOT/scripts/build_paper_reproduction.sh" "$ROOT/scripts/paper_reproduction_suite.py" "$RUN/source_snapshot/"
find "$RUN/source_snapshot" "$RUN/reference_logs" "$RUN/reference_scripts" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp)
printf '%q ' icpx "${FLAGS[@]}" -qmkl=parallel "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/paper_e2e_raw" > "$RUN/commands.txt"
icpx "${FLAGS[@]}" -qmkl=parallel "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/paper_e2e_raw" > "$RUN/build.log" 2>&1
printf '\nicpx -O3 -march=sapphirerapids -qopenmp -qmkl=parallel mkl_baseline.cpp\n' >> "$RUN/commands.txt"
icpx -O3 -march=sapphirerapids -qopenmp -qmkl=parallel "$RUN/source_snapshot/mkl_baseline.cpp" -o "$RUN/paper_mkl_raw" >> "$RUN/build.log" 2>&1
printf 'icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp amx_tfs_v3.cpp\n' >> "$RUN/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/amx_tfs_v3.cpp" -o "$RUN/paper_tfs_raw" >> "$RUN/build.log" 2>&1
# Additional validation executable only; raw performance sources above stay byte-identical.
python3 - "$RUN" <<'PY'
import pathlib,sys,hashlib,json
r=pathlib.Path(sys.argv[1]);src=(r/'source_snapshot/gcn_e2e_v3.cpp').read_text()
needle='        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);'
assert src.count(needle)==1
extra=r'''
        double norm2=0,diff2=0,maxabs=0,maxref=0,sumabs=0;
        for(size_t q=0;q<(size_t)N*K_DIM;q++){
            uint32_t bits=((uint32_t)H2_bf16[q])<<16;float v;memcpy(&v,&bits,4);
            double ref=H2_mkl[q],d=(double)v-ref;
            norm2+=ref*ref;diff2+=d*d;sumabs+=fabs(d);
            if(fabs(d)>maxabs)maxabs=fabs(d);if(fabs(ref)>maxref)maxref=fabs(ref);
        }
        double l2=sqrt(diff2/(norm2+1e-300)),nm=maxabs/(maxref+1e-300);
        int valid=std::isfinite(l2)&&std::isfinite(nm)&&l2<0.03&&nm<0.03;
        printf("PAPER_FINAL_CHECK max_abs=%.9g mean_abs=%.9g relative_L2=%.9g normalized_max=%.9g pass=%d\n",maxabs,sumabs/((size_t)N*K_DIM),l2,nm,valid);
        if(!valid)exit(3);
'''
# Remove measured iterations in validator. Warmups and precision computation remain.
loop='for(int run=0;run<5;run++)';assert src.count(loop)==2
check=src.replace(loop,'for(int run=0;run<0;run++)').replace(needle,extra+'\n'+needle)
assert check.replace(extra+'\n','').replace('for(int run=0;run<0;run++)',loop)==src
(r/'source_snapshot/paper_correctness_only.cpp').write_text(check)
(r/'validation_generation.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(src.encode()).hexdigest(),changes=['measured loops disabled in validator only','full final BF16 output vs MKL checked before primary raw measurements'],raw_performance_modified=False,relative_L2_gate=0.03,normalized_max_gate=0.03),indent=2))
PY
icpx "${FLAGS[@]}" -qmkl=parallel "$RUN/source_snapshot/paper_correctness_only.cpp" -o "$RUN/paper_check" >> "$RUN/build.log" 2>&1
sha256sum "$RUN/source_snapshot/paper_correctness_only.cpp" >> "$RUN/source.sha256"
sha256sum "$RUN/paper_e2e_raw" "$RUN/paper_mkl_raw" "$RUN/paper_tfs_raw" "$RUN/paper_check" > "$RUN/binary.sha256"
ldd "$RUN/paper_e2e_raw" > "$RUN/linked_libraries.txt"
printf '%s\n' "$RUN" > "$ROOT/build/PAPER_REPRODUCTION_LATEST"
