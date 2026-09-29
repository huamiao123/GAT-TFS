#include "../src/amx_head_row.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    if(!gat_amx::available_and_permitted()) {
        std::cout<<"AMX_BLOCK status=unavailable\n";
        return 0;
    }
    constexpr int heads=8,count=32,D=32;
    std::vector<float> score(heads*count),value(count*D);
    for(int h=0;h<heads;h++) for(int k=0;k<count;k++)
        score[h*count+k]=0.01f*float((h*7+k*3)%19)-0.1f;
    for(int k=0;k<count;k++) for(int f=0;f<D;f++)
        value[k*D+f]=0.002f*float((k*11+f*5)%23)-0.02f;
    auto block=gat_amx::head_as_row_block(score.data(),value.data(),heads,count,D,D);
    auto compensated=gat_amx::head_as_row_block(score.data(),value.data(),heads,count,D,D,true);
    double max_error=0,compensated_error=0;
    for(int h=0;h<heads;h++) for(int f=0;f<D;f++) {
        double reference=0;
        for(int k=0;k<count;k++) {
            float p=std::exp(score[h*count+k]-block.reference[h]);
            float pq=gat_amx::bf16_to_float(gat_amx::bf16_rne_flush(p));
            float vq=gat_amx::bf16_to_float(gat_amx::bf16_rne_flush(value[k*D+f]));
            reference+=double(pq)*vq;
        }
        max_error=std::max(max_error,std::abs(reference-block.numerator[size_t(h)*D+f]));
        double full=0;
        for(int k=0;k<count;k++)
            full+=double(std::exp(score[h*count+k]-compensated.reference[h]))*value[k*D+f];
        compensated_error=std::max(compensated_error,
            std::abs(full-compensated.numerator[size_t(h)*D+f]));
    }
    bool pass=max_error<2e-5 && compensated_error<2e-5;
    std::cout<<"AMX_BLOCK status="<<(pass?"PASS":"FAIL")
             <<" max_abs_error="<<max_error
             <<" compensated_fp32_max_abs_error="<<compensated_error
             <<" pack_bytes="<<block.pack_bytes
             <<" tile_load_bytes="<<block.tile_load_bytes
             <<" tile_store_bytes="<<block.tile_store_bytes
             <<" compensated_pack_bytes="<<compensated.pack_bytes<<"\n";
    return pass?0:1;
}
