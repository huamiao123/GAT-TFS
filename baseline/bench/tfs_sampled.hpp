#pragma once
#include "gat.hpp"
namespace gat {
struct SampleStats {
    double schedule=0,score=0,stage=0,matrix=0,output=0;
    uint64_t tiles=0,steps=0,active_edges=0;
};
void sampled_aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,Workspace&,int,uint64_t,uint64_t,SampleStats&);
}
