"""Derive an explicitly separate PH layout diagnostic from preserved original."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = root / 'tfs_online' / 'head_ph_probe.cpp'
target = root / 'tfs_online' / 'head_ph_compact_probe.cpp'
if target.exists():
    raise SystemExit('candidate already exists; refusing to overwrite')
text = source.read_text(encoding='utf-8')
text = text.replace('"head_ph_probe.hpp"', '"head_ph_compact_probe.hpp"')
text = text.replace('namespace gat::icpp_block', 'namespace gat::icpp_ph_compact')
text = text.replace('configured_rows=16', 'configured_rows=8')
text = text.replace('void probe_head_PH(', 'void probe_head_PH_compact(')
text = text.replace('Config(){for(int t:{0,4,5,6}){cols[t]=64;rows[t]=16;}}',
                    'Config(){for(int t:{0,4,6}){cols[t]=64;rows[t]=8;}cols[5]=64;rows[5]=16;}')
begin = text.index('void gather_pack_H(')
end = text.index('\nvoid avx_PH(', begin)
text = text[:begin] + '''void gather_pack_H(const gat::Workspace& base,Scratch& w) {
    // Native BF16 pair interleave, directly from the two source rows.
    // No float conversion and no intermediate row-major H copy.
    for(int kp=0;kp<B/2;++kp) {
        const BF16* a=base.xbf.data()+size_t(w.source[2*kp])*D;
        const BF16* b=base.xbf.data()+size_t(w.source[2*kp+1])*D;
        for(int ob=0;ob<D/16;++ob) {
            __m256i raw_a=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a+ob*16));
            __m256i raw_b=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b+ob*16));
            __m512i lo=_mm512_cvtepu16_epi32(raw_a);
            __m512i hi=_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw_b),16);
            _mm512_store_si512(w.hpacked+size_t(ob)*512+kp*32,_mm512_or_si512(lo,hi));
        }
    }
}
''' + text[end:]
text = text.replace('"AMX_PHI_BF16_PH"', '"AMX_PHI_BF16_PH_COMPACT8"')
text = text.replace('"AMX_PHI_PLO_BF16_PH"', '"AMX_PHI_PLO_BF16_PH_COMPACT8"')
text = text.replace('<<" H_local_gather_write_bytes="<<(mode?rawH_bytes:0)',
                    '<<" H_local_gather_write_bytes="<<0')
text = text.replace('<<" allocation=outside_timers AMX_permission_config_release=in_full_microtimer"',
                    '<<" allocation=outside_timers AMX_permission_config_release=in_full_microtimer"\n'
                    '      <<" H_packing=direct_vector_BF16_pair_interleave intermediate_Hrow_copy=0"')
target.write_text(text, encoding='utf-8', newline='\n')
print(target)
