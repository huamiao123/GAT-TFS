#!/usr/bin/env python3
"""Clone the validated S64/MFULL kernel and change only row/window loop order.

The performance path does not execute the optional partial-trace branch.
"""
from pathlib import Path
import hashlib,sys,json
base=Path(sys.argv[1])
dest=Path(sys.argv[2])
source=base.read_bytes().decode()
start=source.index('template<bool P>\ninline void reduce_scope(')
end=source.index('template<bool P, bool B16 = false>',start)
fn=source[start:end]
opening='''    for(uint32_t window_start = scope_start; window_start < scope_end;) {
        const uint32_t window_end = window_start +
            std::min(window, scope_end - window_start);
        const bool first_window = window_start == scope_start;
        if constexpr(P) pr.counters.neighbor_windows++;

        for(int n = 0; n < batch; n++) {
'''
replacement='''    if constexpr(P) pr.counters.neighbor_windows +=
        1 + (scope_end - scope_start - 1) / window;
    for(int n = 0; n < batch; n++) {
        for(uint32_t window_start = scope_start; window_start < scope_end;
            window_start += std::min(window, scope_end - window_start)) {
            const uint32_t window_end = window_start +
                std::min(window, scope_end - window_start);
            const bool first_window = window_start == scope_start;
'''
closing='''        }
        window_start = window_end;
    }

    // Identical truncation'''
replacement_close='''        }
    }

    // Identical truncation'''
assert fn.count(opening)==fn.count(closing)==1
changed=fn.replace(opening,replacement).replace(closing,replacement_close)
assert changed!=fn
out=source[:start]+changed+source[end:]
assert out.count('namespace gcn_extra_projection {')==1
out=out.replace('namespace gcn_extra_projection {','namespace gcn_extra_projection_a3 {')
out=out.replace('} // namespace gcn_extra_projection','} // namespace gcn_extra_projection_a3')
# A trace is available only in instrumented correctness calls. The compiler
# discards this branch entirely for kernel<false> primary performance calls.
old='''    bool nozero = true;      // Every destination output is explicitly written.
};'''
new='''    bool nozero = true;      // Every destination output is explicitly written.
    float* debug_partials = nullptr; // Instrumented FULL partial trace only.
};'''
assert out.count(old)==1
out=out.replace(old,new)
needle='''                    reduce_scope<P>(g, hb, bases, degrees, batch, start, end,
                                    uint32_t(method.window), partial, hi, lo,
                                    method.accurate, pr, sample);
                    for(int obp = 0; obp < NP; obp++) {'''
insert='''                    reduce_scope<P>(g, hb, bases, degrees, batch, start, end,
                                    uint32_t(method.window), partial, hi, lo,
                                    method.accurate, pr, sample);
                    if constexpr(P) {
                        if(method.debug_partials != nullptr && method.scope == 0)
                            for(int r = 0; r < batch; r++)
                                std::memcpy(method.debug_partials + size_t(rows[r]) * DIM,
                                            partial + size_t(r) * DIM,
                                            DIM * sizeof(float));
                    }
                    for(int obp = 0; obp < NP; obp++) {'''
assert out.count(needle)==1
out=out.replace(needle,insert)
dest.write_bytes(out.encode())
manifest=dict(base=str(base),output=str(dest),base_sha256=hashlib.sha256(source.encode()).hexdigest(),
              output_sha256=hashlib.sha256(out.encode()).hexdigest(),
              mechanism='row-major iteration of the same 64-neighbor windows inside each projection scope',
              unchanged=['within-row CSR order','partial initialization','per-window FP32 state load/store','per-scope BF16 truncation','AMX projection and C staging','degree sort','prefetch boundary'],
              debug_partial_trace='kernel<true> only; compiled out of timed kernel<false>')
print(json.dumps(manifest,indent=2))
