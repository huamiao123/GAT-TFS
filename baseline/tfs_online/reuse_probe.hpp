#pragma once
#include "gat.hpp"
#include <iostream>
#include <map>
#include <unordered_map>

namespace gat {
// Diagnostic only, outside all forward timers. Systematic samples include the
// final high-degree window; these are not unbiased whole-graph estimates.
inline void probe_graph_reuse(const Graph& g,const Schedule& schedule,int block=32) {
    constexpr uint64_t period=64;
    for(uint64_t rows:{16,64,256,1024}) {
        uint64_t windows=0,edges=0,unique=0,maximum_sources=0;
        for(uint64_t first=0;first<g.n;first+=rows) {
            uint64_t ordinal=first/rows,end=std::min(g.n,first+rows);
            if(ordinal%period && end!=g.n)continue;
            std::vector<uint32_t> sources;
            for(uint64_t at=first;at<end;++at) {
                uint32_t node=schedule.perm[at];
                sources.insert(sources.end(),g.col.begin()+g.row[node],g.col.begin()+g.row[node+1]);
            }
            edges+=sources.size();std::sort(sources.begin(),sources.end());
            uint64_t count=std::unique(sources.begin(),sources.end())-sources.begin();
            unique+=count;maximum_sources=std::max(maximum_sources,count);++windows;
        }
        std::cout<<"REUSE_WINDOW rows="<<rows<<" sample_period="<<period
                 <<" includes_high_degree_tail=true population_estimate=false windows="<<windows
                 <<" sampled_edges="<<edges<<" summed_unique_sources="<<unique
                 <<" edge_per_unique_source="<<(unique?double(edges)/unique:0)
                 <<" max_unique_sources="<<maximum_sources
                 <<" L2_all_head_FP32_projected_cache_max_bytes="<<maximum_sources*256*4<<'\n';
    }
    uint64_t tiles=0,blocks=0,edges=0,unique=0,dense_slots=0;
    uint64_t shared_edges=0,large_shared_group_edges=0,duplicate_source_row=0;
    for(uint64_t first=0;first<g.n;first+=16) {
        uint64_t end=std::min(g.n,first+16),ordinal=first/16;
        if(ordinal%period && end!=g.n)continue;
        uint64_t degree=0;
        for(uint64_t at=first;at<end;++at){auto row=schedule.perm[at];degree=std::max(degree,g.row[row+1]-g.row[row]);}
        ++tiles;
        for(uint64_t begin=0;begin<degree;begin+=uint64_t(block)) {
            std::unordered_map<uint32_t,uint16_t> supports;
            uint64_t block_edges=0;
            for(uint64_t at=first;at<end;++at) {
                uint32_t row=schedule.perm[at];uint16_t bit=uint16_t(1u<<(at-first));
                uint64_t stop=std::min(g.row[row+1]-g.row[row],begin+uint64_t(block));
                for(uint64_t e=begin;e<stop;++e) {
                    auto& mask=supports[g.col[g.row[row]+e]];
                    if(mask&bit)++duplicate_source_row;
                    mask|=bit;++block_edges;
                }
            }
            std::map<uint16_t,uint64_t> groups;
            for(const auto& item:supports)++groups[item.second];
            for(const auto& item:groups) {
                auto support=uint64_t(__builtin_popcount(unsigned(item.first)));
                if(support>1)shared_edges+=support*item.second;
                if(support>=3 && item.second>=8)large_shared_group_edges+=support*item.second;
            }
            edges+=block_edges;unique+=supports.size();
            dense_slots+=16*((supports.size()+31)/32*32);++blocks;
        }
    }
    std::cout<<"REUSE_BLOCK block="<<block<<" rows=16 sample_period="<<period
             <<" includes_high_degree_tail=true population_estimate=false sampled_tiles="<<tiles
             <<" sampled_blocks="<<blocks<<" sampled_edges="<<edges
             <<" summed_unique_sources="<<unique<<" edge_per_unique_source="<<(unique?double(edges)/unique:0)
             <<" AMX_union_dense_slots="<<dense_slots
             <<" AMX_union_padding_per_edge="<<(edges?double(dense_slots)/edges:0)
             <<" multi_destination_supported_edges="<<shared_edges
             <<" group_support_ge3_size_ge8_edges="<<large_shared_group_edges
             <<" duplicate_source_row_occurrences="<<duplicate_source_row
             <<" masks_preserve_membership_not_parallel_multiplicity=true\n";
}
}
