# Legal source-aware reuse-window sweep, 2026-10-08

This follow-up keeps the isolated single-thread RoadNet-CA kernel prototype and sweeps W in `{1,4,16,64,256,512,1024,2048,4096}`. Every invocation also runs the W=0 all-retain control in the same executable. The source, binary, graph, random seed, 128 -> 128 -> 128 shape, BF16 boundaries, DegreeSort order and default NUMA policy are unchanged.

This is a work/state frontier diagnostic. It requires exact BF16 equality for every W and records baseline FULL, W=0, and the selected W timing plus actual generated source rows and cache peak. Measurements are exploratory and paper-ineligible; no per-graph oracle or single-thread timing is a final policy.
