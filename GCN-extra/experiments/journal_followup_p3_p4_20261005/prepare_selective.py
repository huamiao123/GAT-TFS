import pathlib
p=pathlib.Path(__file__).resolve().parent
s=(p/'build_residual.sh').read_text()
s=s.replace('residual-broad','selective-shape').replace('residual_broad','selective_shape')
s=s.replace('RESIDUAL_BROAD_LATEST','SELECTIVE_SHAPE_LATEST').replace('generate_residual_driver.py','selective_shape.cpp')
s=s.replace('residual_methods','selective_shape')
s=s.replace('residual_suite.py','selective_suite.py').replace('projection_methods.cpp','selective_shape.cpp')
s=s.replace('build_residual.sh','build_selective.sh').replace('job.sh','selective_job.sh').replace('submit.sh','selective_submit.sh')
start=s.index('cp "$EXP/selective_shape.hpp"');end=s.index('FLAGS=(')
s=s[:start]+'''cp "$EXP/selective_kernels.hpp" "$EXP/selective_shape.cpp" "$EXP/selective_suite.py" "$EXP/build_selective.sh" "$EXP/selective_job.sh" "$EXP/selective_submit.sh" "$EXP/PROTOCOL.md" "$RUN/source_snapshot/"
cp "$ROOT/experiments/journal_grouping_p1_20261005/evidence_utils.py" "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/src/paper_methods_runtime.hpp" "$ROOT/src/projection_window_kernels.hpp" "$ROOT/src/projection_window_pmu.hpp" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
'''+s[end:]
s=s.replace("Frozen ACCURATE controls unchanged; isolated D2/D3 code","P4 isolated ACCURATE shape controls; source TFS/MKL unchanged").replace("Inspect accepted candidates against decoupled FULL ACCURATE","Compare selective materialization to FULL and all-used-source projection")
(p/'build_selective.sh').write_bytes(s.replace('\r\n','\n').encode())
s=(p/'job.sh').read_text().replace('residual_broad','selective_shape').replace('residual_methods','selective_shape').replace('residual_suite.py','selective_suite.py').replace('Inspect accepted candidates against decoupled FULL ACCURATE','Check conditional benefits against project_all and project_used, including cache rebuild')
(p/'selective_job.sh').write_bytes(s.replace('\r\n','\n').encode())
s=(p/'submit.sh').read_text().replace('residual_broad','selective_shape').replace('residual-','selective-').replace('source_snapshot/job.sh','source_snapshot/selective_job.sh')
(p/'selective_submit.sh').write_bytes(s.replace('\r\n','\n').encode())
