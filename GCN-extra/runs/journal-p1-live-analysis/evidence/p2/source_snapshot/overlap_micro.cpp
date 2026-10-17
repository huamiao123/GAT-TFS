#define main original_source_main
#include "gcn_e2e_v3.cpp"
#undef main
#include "paper_methods_kernels.hpp"
#include "micro_pmu.hpp"
#include <vector>
#include <random>
#include <sys/resource.h>
enum Phase { CSR_PREFETCH_P, LOAD_DECODE_P, ADD_P, PARTIAL_STORE_P, PACK_P,
    AMX_A_P, AMX_B_P, AMX_TDP_P, AMX_ZERO_P, AMX_STORE_P, PHASES_P };
const char* names_p[]={"csr_prefetch","source_load_decode","fp32_add","partial_store","partial_pack",
    "amx_a_load","amx_b_load","amx_tdp","amx_zero","amx_c_store"};
struct ProfileP { double sec[PHASES_P]={};uint64_t count[PHASES_P]={}; };
template<bool P> double begin_p(bool sample){if constexpr(P)if(sample)return omp_get_wtime();return 0;}
template<bool P> void end_p(ProfileP& p,Phase phase,double t,bool sample){if constexpr(P)if(sample){p.sec[phase]+=omp_get_wtime()-t;p.count[phase]++;}}
template<bool P> void reduce_rows(const std::vector<uint32_t>& col,const uint16_t* h,int next_tile,int degree,int row_begin,int row_end,float* partial,ProfileP& p,bool sample) {
    for(int row=row_begin;row<row_end;row++) {
        __m512 acc[8];for(auto& v:acc)v=_mm512_setzero_ps();size_t base=(size_t(next_tile)*16+row)*degree;
        for(int k=0;k<degree;k++) {
            double t=begin_p<P>(sample);uint32_t j=col[base+k];
            if(k+1<degree){const char* next=reinterpret_cast<const char*>(h+size_t(col[base+k+1])*128);for(int o=0;o<256;o+=64)_mm_prefetch(next+o,_MM_HINT_T0);}
            end_p<P>(p,CSR_PREFETCH_P,t,sample);t=begin_p<P>(sample);__m512 val[8];const uint16_t* src=h+size_t(j)*128;
            for(int f=0;f<8;f++)val[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(src+f*16))),16));
            end_p<P>(p,LOAD_DECODE_P,t,sample);t=begin_p<P>(sample);
            for(int f=0;f<8;f++)acc[f]=_mm512_add_ps(acc[f],val[f]);end_p<P>(p,ADD_P,t,sample);
        }
        double t=begin_p<P>(sample);for(int f=0;f<8;f++)_mm512_store_ps(partial+row*128+f*16,acc[f]);end_p<P>(p,PARTIAL_STORE_P,t,sample);
    }
}
template<bool P> void pack_p(const float* partial,uint16_t* hi,ProfileP& p,bool sample) {
    double t=begin_p<P>(sample);
    for(int v=0;v<16*128;v+=16){auto bits=_mm512_castps_si512(_mm512_load_ps(partial+v));_mm256_store_si256((__m256i*)(hi+v),_mm512_cvtepi32_epi16(_mm512_srli_epi32(bits,16)));}
    end_p<P>(p,PACK_P,t,sample);
}
template<bool P,bool I> ProfileP run_p(const std::vector<uint32_t>& col,const uint16_t* h,const uint16_t* w,float* out,int tiles,int degree) {
    if(syscall(SYS_arch_prctl,0x1023,18)!=0)throw std::runtime_error("AMX permission");tilecfg_t cfg;setup_tilecfg(&cfg);_tile_loadconfig(&cfg);
    ProfileP p;alignas(64) float partial[2][16*128];alignas(64) uint16_t hi[2][16*128];
    reduce_rows<P>(col,h,0,degree,0,16,partial[0],p,true);pack_p<P>(partial[0],hi[0],p,true);
    for(int tile=0;tile<tiles;tile++) {
        bool sample=P&&(tile%64==0);int cur=tile%2,next=1-cur;bool has_next=tile+1<tiles;
        for(int obp=0;obp<2;obp++) {
            double t=begin_p<P>(sample);_tile_zero(TC0);_tile_zero(TC1);_tile_zero(TC2);_tile_zero(TC3);end_p<P>(p,AMX_ZERO_P,t,sample);
            for(int kb=0;kb<4;kb++) {
                t=begin_p<P>(sample);_tile_loadd(TA,(const uint8_t*)hi[cur]+kb*64,256);end_p<P>(p,AMX_A_P,t,sample);int ob0=obp*4;
                t=begin_p<P>(sample);_tile_loadd(TB0,w+((kb*8+ob0)*16*32),64);_tile_loadd(TB1,w+((kb*8+ob0+1)*16*32),64);end_p<P>(p,AMX_B_P,t,sample);
                t=begin_p<P>(sample);_tile_dpbf16ps(TC0,TA,TB0);_tile_dpbf16ps(TC1,TA,TB1);end_p<P>(p,AMX_TDP_P,t,sample);
                t=begin_p<P>(sample);_tile_loadd(TB0,w+((kb*8+ob0+2)*16*32),64);_tile_loadd(TB1,w+((kb*8+ob0+3)*16*32),64);end_p<P>(p,AMX_B_P,t,sample);
                t=begin_p<P>(sample);_tile_dpbf16ps(TC2,TA,TB0);_tile_dpbf16ps(TC3,TA,TB1);end_p<P>(p,AMX_TDP_P,t,sample);
                if constexpr(I)if(has_next){int begin=(obp*4+kb)*2;reduce_rows<P>(col,h,tile+1,degree,begin,begin+2,partial[next],p,sample);}
            }
            float* c=out+size_t(tile)*16*128+obp*64;t=begin_p<P>(sample);
            _tile_stored(TC0,c,512);_tile_stored(TC1,c+16,512);_tile_stored(TC2,c+32,512);_tile_stored(TC3,c+48,512);end_p<P>(p,AMX_STORE_P,t,sample);
        }
        if(has_next){if constexpr(!I)reduce_rows<P>(col,h,tile+1,degree,0,16,partial[next],p,sample);pack_p<P>(partial[next],hi[next],p,sample);}
    }
    _tile_release();return p;
}
int main() {
    constexpr int tiles=1024,nout=tiles*16*128;float* weight=(float*)aligned_alloc(64,128*128*4);uint16_t* w=(uint16_t*)aligned_alloc(64,128*128*2);
    srand(12345);for(int k=0;k<128*128;k++)weight[k]=(float(rand())/RAND_MAX-.5f)/128;make_W_vnni(weight,w);
    float* out=(float*)aligned_alloc(64,size_t(nout)*4);std::vector<float> expected(nout);
    for(auto spec:{std::pair<int,int>{1024,1},{1024,8},{1024,64},{524288,64},{524288,128}}) {
        int pool=spec.first,degree=spec.second;std::string name=(pool==1024?"hot":"random")+std::to_string(pool)+"_d"+std::to_string(degree);
        uint16_t* h=(uint16_t*)aligned_alloc(64,size_t(pool)*128*2);srand(12345);
        for(size_t k=0;k<size_t(pool)*128;k++)h[k]=f32_to_bf16(float(rand())/RAND_MAX-.5f);
        std::vector<uint32_t> col(size_t(tiles)*16*degree);std::mt19937 rng(20261005);for(auto& j:col)j=rng()%pool;
        printf("OVERLAP_CONFIG case=%s tiles=%d degree=%d source_pool=%d source_bytes=%llu source_visits=%llu feature_adds=%llu partial_pack_vectors=%llu tdpbf16ps=%d output_store_bytes=%llu threads=1 NUMA=default buffer_count=2\n",name.c_str(),tiles,degree,pool,(unsigned long long)(uint64_t(pool)*256),(unsigned long long)col.size(),(unsigned long long)(col.size()*128),(unsigned long long)(uint64_t(tiles)*128),tiles*32,(unsigned long long)(uint64_t(nout)*4));
        run_p<false,false>(col,h,w,out,tiles,degree);std::memcpy(expected.data(),out,size_t(nout)*4);
        run_p<false,true>(col,h,w,out,tiles,degree);bool bits=std::memcmp(out,expected.data(),size_t(nout)*4)==0;bool finite=true;for(float v:expected)finite&=std::isfinite(v);
        printf("OVERLAP_GATE case=%s bitwise=%d finite=%d pass=%d\n",name.c_str(),bits,finite,bits&&finite);if(!bits||!finite)return 2;
        auto run=[&](int method){if(method)run_p<false,true>(col,h,w,out,tiles,degree);else run_p<false,false>(col,h,w,out,tiles,degree);};
        run(0);run(1);
        for(int rep=0;rep<7;rep++)for(int pos=0;pos<2;pos++){int method=rep%2?1-pos:pos;double t=omp_get_wtime();run(method);printf("OVERLAP_TIME case=%s method=%s repeat=%d order=%d ms=%.12g\n",name.c_str(),method?"interleaved":"serial",rep,pos,(omp_get_wtime()-t)*1000);fflush(stdout);}
        for(int method=0;method<2;method++) {
            auto p=method?run_p<true,true>(col,h,w,out,tiles,degree):run_p<true,false>(col,h,w,out,tiles,degree);
            bits=std::memcmp(out,expected.data(),size_t(nout)*4)==0;printf("OVERLAP_PROFILE_GATE case=%s method=%s bitwise=%d pass=%d\n",name.c_str(),method?"interleaved":"serial",bits,bits);if(!bits)return 2;
            for(int ph=0;ph<PHASES_P;ph++)printf("OVERLAP_DETAIL case=%s method=%s phase=%s sampled_thread_ms=%.12g clocked_sections=%llu additive_wall=0\n",name.c_str(),method?"interleaved":"serial",names_p[ph],p.sec[ph]*1000,(unsigned long long)p.count[ph]);
            for(int rep=0;rep<3;rep++){run(method);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(method);double wall=(omp_get_wtime()-t)*1000;auto result=region.stop();gcn_extra_projection_pmu::print(result,name,method?"interleaved":"serial",rep);printf("OVERLAP_PMU_WALL case=%s method=%s repeat=%d ms=%.12g\n",name.c_str(),method?"interleaved":"serial",rep,wall);}
        }
        free(h);
    }
    free(weight);free(w);free(out);printf("OVERLAP_COMPLETE cases=5 output_gates=5 profile_gates=10 pass=1\n");return 0;
}
