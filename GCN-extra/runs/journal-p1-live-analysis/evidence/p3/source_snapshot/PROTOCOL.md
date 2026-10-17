# P3 hardware numerical gate: delayed residual correction

Test the supplied journal plan's section 7.3 counterexample on actual AMX.
Keep the original source TFS and current ACCURATE kernel unchanged.
N=16, each destination has sources 0,1,2,3; channel0 features are
[256,1,-256,-1/512] * 2^-9 and all other channels zero. W00=1.
BF16 input and weight are exact. Reference output0=0.998046875*2^-9.
Both FP32 output and final truncated BF16 output are inspected.

Compare source TFS, B2 ACCURATE, D1 (high projections plus one truncated
summed-residual projection), D2 (high projections plus hi/lo summed-residual
projections), FULL ACCURATE. Reuse the existing BF16 truncation, reduce_tile
and AMX project_panel. D1 is expected to exceed the unchanged 1e-3 FP32
relative-L2 ACCURATE gate; experiment PASS means this expected rejection
and the remaining counterexample results are reproduced. It does not mean
D1 is accepted. D2 passing this one fixture is not general correctness.

This is a small numerical test, not a performance measurement or trained
classification test. Four physical cores on one shared SPR socket, default
NUMA, identical original compiler flags and TFS packed-W layout. All source,
binary, environment, output and handoff artifacts retained in a unique run.
No performance evaluation of D1 under the ACCURATE label after gate failure.
