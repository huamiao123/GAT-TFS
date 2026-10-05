import pathlib
p=pathlib.Path(__file__).resolve().parent
parent=p.parents[1]
s=(parent/'experiments/journal_grouping_p1_20261005/generate_grouping_driver.py').read_text()
s=s.replace("['grouping_schedule.hpp','grouping_runtime.hpp']","['residual_broad.hpp']")
s=s.replace('#include "grouping_runtime.hpp"','#include "residual_broad.hpp"')
s=s.replace('gcn_extra_grouping_runtime::run_methods','residual_broad::run')
s=s.replace("independent_variables=['budget-preserving destination schedule']","independent_variables=['residual correction period/components']")
s=s.replace("'DegreeSort anchor, isolated q64 grouping, TR16/R64 unchanged'","'Original DegreeSort/TR16/R64 unchanged'")
s=s.replace("extra_gate='same M/precision implies bitwise identical FP32 kernel and both two-layer outputs'","extra_gate='Frozen accurate gates; reject candidate before timing'")
(p/'generate_residual_driver.py').write_bytes(s.replace('\r\n','\n').encode())
