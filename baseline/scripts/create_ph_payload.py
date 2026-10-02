from pathlib import Path
root=Path(__file__).resolve().parents[1]
target=root/'scripts'/'icpp_ph_payload.sh'
if target.exists():raise SystemExit('refusing overwrite')
text=(root/'scripts'/'icpp_block_payload.sh').read_text(encoding='utf-8')
text=text.replace('ICPP_BLOCK_NUMA_APPLIED','ICPP_PH_NUMA_APPLIED').replace('icpp_block_payload.sh','icpp_ph_payload.sh')
start=text.index('    echo \'purpose=')
end=text.index('    numactl --show',start)
text=text[:start]+'''    echo 'purpose=head_as_row_PH_layout_control_micro_only'
    echo 'variants=original16_scalar_pack,compact8_direct_vector_BF16_pack'
    echo 'arithmetic=independent8heads;FP32_p/den;BF16_high_or_highlow_PH'
    echo 'upstream=B64_own_L1;micro_neighbors=first32;maxsamples=512;warmup=1;reps=3'
    echo 'timing=score_exp_Hgather_Ppack_AMXconfig_loadcompute_store_scatter_included'
    echo 'performance=sampled_hot_instrumented_micro;fullmodel_speedup=NOT_MEASURED'
    echo 'acceptance=independent_quantized_FP64_instruction_gate;master_gate=NOT_EVALUATED'
'''+text[end:]
start=text.index('echo "START smoke')
text=text[:start]+'''for graph in arxiv products; do
    echo "START $graph $(date -Is)"
    stdbuf -oL "$ROOT/build/icpp_ph" "$ROOT/../data/$graph.gatbin" > "$RUN/$graph.log" 2>&1
    echo "DONE $graph $(date -Is)"
done
echo ICPP_PH_CLUSTER_COMPLETE
'''
target.write_text(text,encoding='utf-8',newline='\n')
