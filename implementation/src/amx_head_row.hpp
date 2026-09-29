#pragma once

// One legal head-as-row AMX-BF16 block: P[heads,32] * X[32,16].
// The caller owns score generation, online block merging, and later W projection.
// The function returns unnormalised FP32 numerators plus denominators formed from
// the BF16 weights actually submitted to AMX. This is a real AMX microkernel,
// not a complete three-layer backend.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>
#include <immintrin.h>
#if defined(__linux__)
#include <asm/prctl.h>
#include <cpuid.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace gat_amx {
using MicroClock=std::chrono::steady_clock;
inline MicroClock::time_point micro_now() {
#ifdef GAT_NO_FINE_TIMING
    return MicroClock::time_point{};
#else
    return MicroClock::now();
#endif
}
inline double micro_seconds(MicroClock::time_point a,MicroClock::time_point b) {
    return std::chrono::duration<double>(b-a).count();
}

inline bool available_and_permitted() {
#if defined(__linux__) && defined(__AMX_BF16__) && defined(__AMX_TILE__)
    unsigned int eax=0,ebx=0,ecx=0,edx=0;
    if(!__get_cpuid_count(7,0,&eax,&ebx,&ecx,&edx) ||
       !(edx&(1u<<24)) || !(edx&(1u<<22))) return false;
    return syscall(SYS_arch_prctl,ARCH_REQ_XCOMP_PERM,18)==0;
#else
    return false;
#endif
}

struct alignas(64) TileConfig {
    uint8_t palette=1,start_row=0,reserved[14]{};
    uint16_t colsb[16]{};
    uint8_t rows[16]{};
};
static_assert(sizeof(TileConfig)==64,"AMX tile config must occupy 64 bytes");

inline uint16_t bf16_rne_flush(float x) {
    if(!std::isfinite(x)) throw std::invalid_argument("nonfinite BF16 operand");
    uint32_t bits;
    std::memcpy(&bits,&x,sizeof(bits));
    bits+=0x7fffu+((bits>>16)&1u);
    uint16_t result=uint16_t(bits>>16);
    if((result&0x7f80u)==0) result&=0x8000u; // AMX BF16 subnormal input contract
    if((result&0x7f80u)==0x7f80u) throw std::overflow_error("BF16 operand overflow");
    return result;
}
inline float bf16_to_float(uint16_t x) {
    uint32_t bits=uint32_t(x)<<16;
    float value;
    std::memcpy(&value,&bits,sizeof(value));
    return value;
}

struct BlockResult {
    std::vector<float> numerator; // [heads,D], still unnormalised
    std::vector<float> denominator; // same quantized weights as AMX left operand
    std::vector<float> reference; // per-head block maximum
    uint64_t pack_bytes=0,tile_load_bytes=0,tile_store_bytes=0;
    double setup_s=0,score_max_s=0,weight_exp_bf16_s=0;
    double feature_zero_s=0,feature_bf16_pack_s=0,output_extract_s=0,tile_release_s=0;
    double packing_s=0,tile_config_s=0,tile_load_s=0,tile_compute_s=0,tile_store_s=0;
};

#if defined(__AMX_BF16__) && defined(__AMX_TILE__)
inline BlockResult head_as_row_block(const float* scores,const float* values,
                                     int heads,int count,int D,int value_stride,
                                     bool compensated=false) {
    if(!scores || !values || heads<1 || heads>8 || count<1 || count>32 ||
       D<1 || value_stride<D)
        throw std::invalid_argument("invalid AMX block shape");
    auto setup_tick=micro_now();
    BlockResult result;
    result.numerator.assign(size_t(heads)*D,0.0f);
    result.denominator.assign(heads,0.0f);
    result.reference.assign(heads,-std::numeric_limits<float>::infinity());
    result.setup_s=micro_seconds(setup_tick,micro_now());
    auto tick=micro_now();
    alignas(64) uint16_t P[8][32]{};
    alignas(64) uint16_t Plo[8][32]{};
    for(int h=0;h<heads;h++) {
        float m=-std::numeric_limits<float>::infinity();
        for(int k=0;k<count;k++) {
            float s=scores[size_t(h)*count+k];
            if(!std::isfinite(s)) throw std::invalid_argument("nonfinite score");
            m=std::max(m,s);
        }
        result.reference[h]=m;
    }
    result.score_max_s+=micro_seconds(tick,micro_now());
    tick=micro_now();
    for(int h=0;h<heads;h++) {
        float m=result.reference[h];
        for(int k=0;k<count;k++) {
            float p=std::exp(scores[size_t(h)*count+k]-m);
            P[h][k]=bf16_rne_flush(p);
            if(compensated) Plo[h][k]=bf16_rne_flush(p-bf16_to_float(P[h][k]));
            result.denominator[h]+=bf16_to_float(P[h][k])+bf16_to_float(Plo[h][k]);
        }
        if(!(result.denominator[h]>0)) throw std::overflow_error("zero BF16 denominator");
    }
    result.weight_exp_bf16_s+=micro_seconds(tick,micro_now());
    result.packing_s+=result.score_max_s+result.weight_exp_bf16_s;
    TileConfig cfg;
    cfg.rows[0]=uint8_t(heads);cfg.colsb[0]=64; // 16 FP32 output columns
    cfg.rows[1]=uint8_t(heads);cfg.colsb[1]=64; // 32 BF16 reduction elements
    cfg.rows[2]=16;cfg.colsb[2]=64;             // 16 VNNI-packed BF16 pairs
    if(compensated) {
        cfg.rows[3]=uint8_t(heads);cfg.colsb[3]=64; // P low part
        cfg.rows[4]=16;cfg.colsb[4]=64;             // X low part
    }
    tick=micro_now();
    _tile_loadconfig(&cfg);
    result.tile_config_s+=micro_seconds(tick,micro_now());
    tick=micro_now();
    _tile_loadd(1,P,64);
    if(compensated) _tile_loadd(3,Plo,64);
    result.tile_load_s+=micro_seconds(tick,micro_now());
    result.pack_bytes+=uint64_t(heads)*32*2*(compensated?2:1);
    result.tile_load_bytes+=uint64_t(heads)*64*(compensated?2:1);
    alignas(64) uint16_t Xpack[16][32];
    alignas(64) uint16_t Xlo[16][32];
    alignas(64) float C[8][16];
    for(int f0=0;f0<D;f0+=16) {
        tick=micro_now();
        std::memset(Xpack,0,sizeof(Xpack));
        if(compensated) std::memset(Xlo,0,sizeof(Xlo));
        result.feature_zero_s+=micro_seconds(tick,micro_now());
        tick=micro_now();
        for(int pair=0;pair<16;pair++) for(int f=0;f<16 && f0+f<D;f++) {
            int k=pair*2;
            if(k<count) {
                float x=values[size_t(k)*value_stride+f0+f];
                Xpack[pair][2*f]=bf16_rne_flush(x);
                if(compensated) Xlo[pair][2*f]=bf16_rne_flush(x-bf16_to_float(Xpack[pair][2*f]));
            }
            if(k+1<count) {
                float x=values[size_t(k+1)*value_stride+f0+f];
                Xpack[pair][2*f+1]=bf16_rne_flush(x);
                if(compensated) Xlo[pair][2*f+1]=bf16_rne_flush(x-bf16_to_float(Xpack[pair][2*f+1]));
            }
        }
        result.feature_bf16_pack_s+=micro_seconds(tick,micro_now());
        tick=micro_now();
        _tile_zero(0);
        _tile_loadd(2,Xpack,64);
        if(compensated) _tile_loadd(4,Xlo,64);
        result.tile_load_s+=micro_seconds(tick,micro_now());
        tick=micro_now();
        _tile_dpbf16ps(0,1,2);
        if(compensated) {
            _tile_dpbf16ps(0,1,4);
            _tile_dpbf16ps(0,3,2);
            _tile_dpbf16ps(0,3,4);
        }
        result.tile_compute_s+=micro_seconds(tick,micro_now());
        tick=micro_now();
        _tile_stored(0,C,64);
        result.tile_store_s+=micro_seconds(tick,micro_now());
        tick=micro_now();
        for(int h=0;h<heads;h++) for(int f=0;f<16 && f0+f<D;f++)
            result.numerator[size_t(h)*D+f0+f]=C[h][f];
        result.output_extract_s+=micro_seconds(tick,micro_now());
        result.pack_bytes+=sizeof(Xpack)*(compensated?2:1);
        result.tile_load_bytes+=sizeof(Xpack)*(compensated?2:1);
        result.tile_store_bytes+=uint64_t(heads)*64;
    }
    result.packing_s+=result.feature_zero_s+result.feature_bf16_pack_s;
    tick=micro_now();
    _tile_release();
    result.tile_release_s+=micro_seconds(tick,micro_now());
    return result;
}
#else
inline BlockResult head_as_row_block(const float*,const float*,int,int,int,int,bool=false) {
    throw std::runtime_error("compile with -mamx-tile -mamx-bf16");
}
#endif

} // namespace gat_amx
