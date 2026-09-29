#pragma once

// Software state protocol for one destination row and one independent head.
// This is a correctness model for deferred feature-block alignment, not an AMX kernel.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace gat_state {

struct Counters {
    uint64_t neighbor_blocks=0;
    uint64_t reference_updates=0;
    uint64_t physical_feature_rescales=0;
    uint64_t rescaled_feature_elements=0;
};

struct FeatureBlock {
    size_t begin=0, end=0;
    uint64_t consumed_edges=0;
    uint64_t reference_version=0;
    double stored_reference=0;
    std::vector<double> numerator;
};

class RowHeadState {
public:
    RowHeadState(size_t features,size_t feature_block,double window=0)
        : features_(features),window_(window) {
        if (!features || !feature_block || !std::isfinite(window) || window<0)
            throw std::invalid_argument("invalid feature block or window");
        for(size_t b=0;b<features;b+=feature_block) {
            size_t e=std::min(features,b+feature_block);
            FeatureBlock x;
            x.begin=b;x.end=e;x.numerator.resize(e-b,0);
            blocks_.push_back(std::move(x));
        }
    }

    // Values are count x features, row major. Each valid edge enters exactly once.
    // A multi-head caller creates one RowHeadState per head; graph indices may be shared.
    void consume(const float* scores,const float* values,size_t count,size_t value_stride) {
        if (!count) return;
        if (!scores || !values || value_stride<features_ ||
            count>std::numeric_limits<uint64_t>::max()-consumed_edges_)
            throw std::invalid_argument("invalid neighbor block");
        double maximum=-std::numeric_limits<double>::infinity();
        for(size_t k=0;k<count;k++) {
            if (!std::isfinite(scores[k])) throw std::invalid_argument("nonfinite score");
            maximum=std::max(maximum,double(scores[k]));
            for(size_t f=0;f<features_;f++)
                if(!std::isfinite(values[k*value_stride+f]))
                    throw std::invalid_argument("nonfinite value");
        }
        if (!consumed_edges_) {
            reference_=maximum;
            version_=1;
        } else if (maximum-reference_>window_) {
            const double factor=std::exp(reference_-maximum);
            denominator_*=factor;
            reference_=maximum;
            ++version_;
            ++counters_.reference_updates;
        }
        // P remains immutable until every feature block has consumed this block.
        std::vector<double> weights(count);
        for(size_t k=0;k<count;k++) {
            weights[k]=std::exp(double(scores[k])-reference_);
            if(!std::isfinite(weights[k])) throw std::overflow_error("weight overflow");
            denominator_+=weights[k]; // denominator is updated once per neighbor, not per feature block
        }
        for(auto& b:blocks_) {
            if(b.consumed_edges!=consumed_edges_)
                throw std::logic_error("feature block edge prefix mismatch");
            if(b.consumed_edges && b.reference_version!=version_) {
                const double factor=std::exp(b.stored_reference-reference_);
                for(double& u:b.numerator) u*=factor;
                ++counters_.physical_feature_rescales;
                counters_.rescaled_feature_elements+=b.numerator.size();
            }
            for(size_t k=0;k<count;k++)
                for(size_t f=b.begin;f<b.end;f++)
                    b.numerator[f-b.begin]+=weights[k]*double(values[k*value_stride+f]);
            b.stored_reference=reference_;
            b.reference_version=version_;
            b.consumed_edges+=count;
        }
        consumed_edges_+=count;
        ++counters_.neighbor_blocks;
        if (!std::isfinite(denominator_) || denominator_<=0)
            throw std::overflow_error("invalid denominator; replay row with safer path");
    }

    std::vector<float> finish() const {
        std::vector<float> output(features_,0);
        if(!consumed_edges_) return output; // explicit empty-row policy
        for(const auto& b:blocks_) {
            if(b.consumed_edges!=consumed_edges_ || b.reference_version!=version_)
                throw std::logic_error("incomplete feature block");
            for(size_t f=b.begin;f<b.end;f++) {
                double value=b.numerator[f-b.begin]/denominator_;
                if(!std::isfinite(value) || std::abs(value)>std::numeric_limits<float>::max())
                    throw std::overflow_error("output range exceeded");
                output[f]=float(value);
            }
        }
        return output;
    }

    const Counters& counters() const {return counters_;}
    uint64_t consumed_edges() const {return consumed_edges_;}
    uint64_t reference_version() const {return version_;}
    double reference() const {return reference_;}
    double denominator() const {return denominator_;}

private:
    size_t features_;
    double window_;
    double reference_=0,denominator_=0;
    uint64_t consumed_edges_=0,version_=0;
    Counters counters_;
    std::vector<FeatureBlock> blocks_;
};

// Exact final maximum for additive Vanilla GAT, with monotone LeakyReLU.
// Empty rows are represented by -infinity and must use the caller's empty-row policy.
inline float exact_max_prescan(float left,const float* right,
                               const uint32_t* neighbors,size_t count,float slope=0.2f) {
    if(!std::isfinite(left) || !std::isfinite(slope) || slope<=0 ||
       (count && (!right || !neighbors)))
        throw std::invalid_argument("invalid prescan input");
    if(!count) return -std::numeric_limits<float>::infinity();
    float max_right=-std::numeric_limits<float>::infinity();
    for(size_t k=0;k<count;k++) {
        float r=right[neighbors[k]];
        if(!std::isfinite(r)) throw std::invalid_argument("nonfinite right score");
        max_right=std::max(max_right,r);
    }
    float score=left+max_right;
    if(!std::isfinite(score)) throw std::overflow_error("prescan score overflow");
    return score>=0?score:slope*score;
}

} // namespace gat_state
