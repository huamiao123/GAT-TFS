#!/usr/bin/env python3
"""Derive source-preserving hooks/launchers from the archived P0 infrastructure."""
import pathlib,json,subprocess,hashlib
exp=pathlib.Path(__file__).resolve().parent
root=exp.parents[1];parent=root/'experiments/journal_decoupling_a3_20261004'
s=(parent/'generate_a3_driver.py').read_text(encoding='utf-8')
s=s.replace("['projection_window_a3_kernels.hpp','projection_window_a3_runtime.hpp']","['grouping_schedule.hpp','grouping_runtime.hpp']")
s=s.replace('projection_window_a3_runtime.hpp','grouping_runtime.hpp').replace('gcn_extra_projection_runtime::run_methods','gcn_extra_grouping_runtime::run_methods')
s=s.replace("'neighbor window S','projection scope M','A3 row-major window control'","'budget-preserving destination schedule'")
s=s.replace('DegreeSort/TR16/R64 unchanged','DegreeSort anchor, isolated q64 grouping, TR16/R64 unchanged')
(exp/'generate_grouping_driver.py').write_text(s,encoding='utf-8')
(exp/'evidence_utils.py').write_bytes((parent/'projection_window_a3_suite.py').read_bytes())
job=(parent/'a3_job.sh').read_text(encoding='utf-8').replace('journal_a3_','journal_p1_').replace('A3 control partial trace only in checks','P1 same-q64 schedule control, fixed kernels').replace('projection_window_a3_suite.py','grouping_suite.py').replace('Compare A0/A1/A2/A3 then extend after successful four-graph diagnostic','Reconcile same-work grouping and preprocessing, extend after successful five-graph diagnostic')
(exp/'grouping_job.sh').write_text(job,encoding='utf-8')
submit=(parent/'submit_a3.sh').read_text(encoding='utf-8').replace('JOURNAL_A3_LATEST','JOURNAL_P1_LATEST').replace('A3_DEPENDENCY_JOB','P1_DEPENDENCY_JOB').replace('a3_job.sh','grouping_job.sh').replace('a3-$MODE','p1-$MODE').replace('journal_a3_submission','journal_p1_submission').replace('JOURNAL_A3_JOB','JOURNAL_P1_JOB').replace('Bitwise partial/output gate then focus','P1 bitwise/output/work gate then focus')
(exp/'submit_grouping.sh').write_text(submit,encoding='utf-8')
state=dict(parent_git_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root.parent,text=True).strip(),dirty_status=subprocess.check_output(['git','status','--short'],cwd=root.parent,text=True),parent_kernel_sha256=hashlib.sha256((root/'src/projection_window_kernels.hpp').read_bytes()).hexdigest(),experiment=str(exp),method='q64 constrained source/page minhash grouping, bounded 4096-row segment')
(exp/'source_state.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
