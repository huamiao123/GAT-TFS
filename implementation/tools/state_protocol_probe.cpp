#include "../src/online_block_state.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

int main() {
    using gat_state::RowHeadState;
    float scores[4]={-1.0e8f,-1.0e8f,0.0f,1.0f};
    float values[4]={0,0,0,1};
    RowHeadState state(1,1,4*std::log(2.0));
    state.consume(scores,values,2,1);
    state.consume(scores+2,values+2,2,1);
    double expected=std::exp(1.0)/(1.0+std::exp(1.0));
    if(std::abs(double(state.finish()[0])-expected)>1e-6)
        throw std::runtime_error("resettable reference failed");
    if(state.consumed_edges()!=4 || state.counters().neighbor_blocks!=2)
        throw std::runtime_error("edge prefix failed");
    RowHeadState empty(33,16);
    for(float x:empty.finish()) if(x!=0) throw std::runtime_error("empty row failed");
    float right[3]={-2,3,1};uint32_t neighbors[3]={0,1,2};
    float max=gat_state::exact_max_prescan(-1,right,neighbors,3);
    if(max!=2) throw std::runtime_error("prescan failed");
    bool rejected=false;
    try {
        float nan=std::numeric_limits<float>::quiet_NaN();
        RowHeadState invalid(1,1);invalid.consume(&nan,values,1,1);
    } catch(const std::invalid_argument&) {rejected=true;}
    if(!rejected) throw std::runtime_error("NaN not rejected");
    std::cout<<"STATE_PROTOCOL status=PASS\n";
}
