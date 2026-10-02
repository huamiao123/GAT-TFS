#!/usr/bin/env python3
"""Preserve derivation of the new harness; old experiment files are untouched."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
scripts=root/'scripts'
for old,new in [('build_icpp_block.sh','build_icpp_heads.sh'),('run_icpp_block.slurm','run_icpp_heads.slurm'),('icpp_block_payload.sh','icpp_heads_payload.sh')]:
    text=(scripts/old).read_text(encoding='utf-8')
    text=text.replace('icpp-block','icpp-heads').replace('icpp_block','icpp_heads').replace('ICPP_BLOCK','ICPP_HEADS')
    if new.startswith('build_'):
        begin=text.index('SOURCES=(')
        end=text.index('\n',begin)
        text=text[:begin]+'''SOURCES=("$ROOT"/src/*.cpp "$ROOT/tfs_online/icpp_online.cpp" "$ROOT/tfs_online/checks.cpp" "$ROOT/tfs_online/icpp_pair.cpp" "$ROOT/tfs_online/pair_checks.cpp" "$ROOT/tfs_online/icpp_block.cpp" "$ROOT/tfs_online/block_checks.cpp" "$ROOT/tfs_online/icpp_heads.cpp" "$ROOT/tfs_online/heads_checks.cpp" "$ROOT/tfs_online/heads_main.cpp")'''+text[end:]
        text=text.replace('icpp_heads.cpp" -o "$RUN/icpp_heads.s"','icpp_heads.cpp" -o "$RUN/icpp_heads.s"')
        text='\n'.join(line for line in text.split('\n') if 'head_ph_probe.cpp' not in line)
        text=text.replace('icpx "${FLAGS[@]}" -S "$ROOT/tfs_online/icpp_heads.cpp" -o "$RUN/icpp_heads.s" > "$RUN/assembly_build.log" 2>&1',
          'icpx "${FLAGS[@]}" -S "$ROOT/tfs_online/icpp_heads.cpp" -o "$RUN/icpp_heads.s" > "$RUN/assembly_build.log" 2>&1\ngrep -Ec "tdpbf16ps|__svml_expf16_z0|__svml_expf16" "$RUN/icpp_heads.s" > "$RUN/opcode_check.txt"')
        snapshot='mkdir -p "$RUN/source_snapshot"\ncp -r "$ROOT/tfs_online" "$ROOT/src" "$ROOT/include" "$RUN/source_snapshot/"\n'
        text=text.replace(snapshot,'')
        text=text.replace('printf \'%q \'',snapshot+'printf \'%q \'')
    elif new.startswith('run_'):
        text=text.replace('--time=00:30:00','--time=00:40:00')
    else:
        replacements={
          'purpose=block_SpMM_AMX_GeMM_fusion_retaining_DegreeSort_TR16_residentV':'purpose=cross_head_sharedH_PH_AMX_UW_fusion_retaining_DegreeSort_TR16',
          'controls=B0_FP32,B0_BF16,B1_TFS_PREMAX,ICPP_TFS_ONLINE_B32;candidate=ICPP_TFS_BLOCK_B16_B32_B64':'controls=B0_FP32,B0_BF16,B1_TFS_PREMAX,ICPP_TFS_PAIR_B32,ICPP_TFS_BLOCK_B32;candidate=HEADS_AVX_B16_B32,HEADS_AMX_HILO_B16_B32',
          'block_local_U=immediately_consumed_by_AMX;attention/W/m/l/V_independent;no_fullZ/U;new_block_RNE_interface':'block_local_U=immediately_consumed_by_AMX;attention/W/m/l/V_independent;no_fullZ/U/e/alpha;V_worker_local_count_all_head_switch_traffic;new_PH_high_low_RNE_interface',
          'blocks=16,32,64':'blocks=16,32',
          'profile=separate_coarse_staging_and_AMX_load_compute_worker_spans;not_wall_percentages':'profile=separate_source_index_score_max_exp_Hpack_Ppack_PHload_PHcompute_PHstore_UBconvert_UWload_UWcompute_Vstore_Vreload_rescale_transition;worker_sums_not_wall_percentages',
          'correctness=original_then_pair_then_block_independent_oracles_before_speed;master_abs_gate=.003_unchanged':'correctness=original_pair_block_and_independent_heads_instruction_oracles_before_speed;master_abs_gate=.003_unchanged',
          'extra_diagnostics=systematic_graph_reuse_samples_and_head_as_row_PH_microbench;not_E2E_speedup':'extra_diagnostics=same_input_contracted_attention_L2;profile_fingerprint_and_exact_work_identities;full_three_layer_own_output_E2E',
          'warmups=1;repeats=3':'check_runs=1;warmups=1;repeats=3',
        }
        for a,b in replacements.items():text=text.replace(a,b)
    (scripts/new).write_text(text,encoding='utf-8',newline='\n')
print('created=build_icpp_heads.sh,run_icpp_heads.slurm,icpp_heads_payload.sh')
