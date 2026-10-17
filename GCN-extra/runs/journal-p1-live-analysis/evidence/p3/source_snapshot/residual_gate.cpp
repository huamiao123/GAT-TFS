#define main original_source_main
#include "gcn_e2e_v3.cpp"
#undef main
#include "paper_methods_kernels.hpp"
#include "paper_methods_runtime.hpp"
#include <vector>
#include <sys/resource.h>
// Isolated numerical gate, not a replacement for a validated inference kernel.
static void delayed(const gcn_extra_paper::SourceGraphView& g,const uint16_t* h,const uint16_t* w,float* out,int components) {
    using namespace gcn_extra_paper;
    if(syscall(SYS_arch_prctl,0x1023,18)!=0)throw std::runtime_error("AMX permission");
    tilecfg_t cfg;setup_tilecfg(&cfg);_tile_loadconfig(&cfg);
    alignas(64) float partial[16*128],residual[16*128]={},ctile[16*128]={};
    alignas(64) uint16_t hi[16*128],lo[16*128];uint32_t bases[16],degrees[16];
    for(int i=0;i<16;i++){bases[i]=g.row[i];degrees[i]=g.row[i+1]-bases[i];}
    Profile p;
    auto project=[&](const uint16_t* input){
        for(int obp=0;obp<2;obp++) {
            _tile_loadd(TC0,ctile+obp*64,512);_tile_loadd(TC1,ctile+obp*64+16,512);
            _tile_loadd(TC2,ctile+obp*64+32,512);_tile_loadd(TC3,ctile+obp*64+48,512);
            project_panel<false>(input,w,obp,p,false);
            _tile_stored(TC0,ctile+obp*64,512);_tile_stored(TC1,ctile+obp*64+16,512);
            _tile_stored(TC2,ctile+obp*64+32,512);_tile_stored(TC3,ctile+obp*64+48,512);
        }
    };
    for(uint32_t start=0;start<degrees[0];start+=2) {
        reduce_tile<false>(g,h,bases,degrees,16,start,2,partial,hi,lo,false,p,false);
        for(int k=0;k<16*128;k++)residual[k]+=partial[k]-bf16_to_f32(hi[k]);
        project(hi);
    }
    for(int k=0;k<16*128;k++){hi[k]=f32_to_bf16(residual[k]);lo[k]=f32_to_bf16(residual[k]-bf16_to_f32(hi[k]));}
    project(hi);if(components==2)project(lo);
    std::memcpy(out,ctile,sizeof(ctile));_tile_release();
}
int main() {
    constexpr int n=16,d=128;std::vector<uint32_t> row(n+1),col;int perm[n];
    for(int i=0;i<n;i++){perm[i]=i;for(int j=0;j<4;j++)col.push_back(j);row[i+1]=col.size();}
    gcn_extra_paper::SourceGraphView g{n,col.size(),{row.data()},{col.data()}};
    alignas(64) float weight[d*d]={},hfp[n*d]={};alignas(64) uint16_t packed[d*d],h[n*d];
    // Every nonzero feature is in [-0.5,0.5]. The identity channel preserves
    // the journal document's exact cancellation counterexample on AMX.
    weight[0]=1.f;make_W_vnni(weight,packed);
    const float values[]={256.f,1.f,-256.f,-1.f/512.f};
    for(int i=0;i<4;i++)hfp[i*d]=std::ldexp(values[i],-9);
    for(int k=0;k<n*d;k++)h[k]=f32_to_bf16(hfp[k]);
    std::vector<float> reference(n*d,0),original(n*d),accurate(n*d),d1(n*d),d2(n*d),full(n*d);
    float exact=std::ldexp(0.998046875f,-9);for(int i=0;i<n;i++)reference[i*d]=exact;
    tfs_v3_fp32out(row.data(),col.data(),h,packed,original.data(),perm,n,64);
    gcn_extra_paper::Method control{"b2_accurate",2,true,false,0,true};
    gcn_extra_paper::block_kernel<false,false>(g,h,packed,accurate.data(),perm,control,nullptr);
    control.block=0;gcn_extra_paper::block_kernel<false,false>(g,h,packed,full.data(),perm,control,nullptr);
    delayed(g,h,packed,d1.data(),1);delayed(g,h,packed,d2.data(),2);
    const char* names[]={"source_tfs","b2_accurate","d1_single_delayed","d2_two_component_delayed","full_accurate"};
    const std::vector<float>* outputs[]={&original,&accurate,&d1,&d2,&full};
    int expected_status=0;
    for(int m=0;m<5;m++) {
        auto err=gcn_extra_paper::error(outputs[m]->data(),reference.data(),n*d);
        bool gate=err.finite&&err.l2<=1e-3;bool final_equal=true;
        for(int k=0;k<n*d;k++)final_equal&=f32_to_bf16((*outputs[m])[k])==f32_to_bf16(reference[k]);
        printf("RESIDUAL_GATE method=%s reference=exact_bf16_input_math output0=%.12g reference0=%.12g max_abs=%.12g relative_L2=%.12g finite=%d accurate_gate=0.001 pass=%d final_bf16_equal=%d projection_parts=%d tdpbf16ps=%d\n",names[m],(*outputs[m])[0],reference[0],err.max_abs,err.l2,err.finite,gate,final_equal,m==0?4:(m==1?4:(m==2?3:(m==3?4:2))),32*(m==0?4:(m==1?4:(m==2?3:(m==3?4:2)))));
        if((m==2&&gate)||(m!=2&&!gate)||!final_equal)expected_status=2;
    }
    printf("RESIDUAL_EXPERIMENT_COMPLETE expected_d1_rejection=1 d2_counterexample_pass_only=1 pass=%d\n",expected_status==0);
    return expected_status;
}
