#include "gat.hpp"
namespace gat {
void projection(const Graph& g,const Param& p,const Prepared& q,const std::vector<float>& x,Workspace& w,bool bf16) {
    if(bf16)cblas_gemm_bf16bf16f32(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),p.width(),p.in,1.f,w.xbf.data(),p.in,q.w.data(),p.width(),0.f,w.z.data(),p.width());
    else cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),p.width(),p.in,1,x.data(),p.in,p.w.data(),p.width(),0,w.z.data(),p.width());
}
void lr_from_z(const Graph& g,const Param& p,Workspace& w,bool scalar) {
    #pragma omp parallel for schedule(static) if(!scalar)
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){
        const float* z=w.z.data()+i*p.width()+h*p.dim;
        const float* al=p.al.data()+h*p.dim;const float* ar=p.ar.data()+h*p.dim;
        float left=0,right=0;
        if(scalar){for(int d=0;d<p.dim;++d){left+=z[d]*al[d];right+=z[d]*ar[d];}}
        else {__m512 l=_mm512_setzero_ps(),r=l;
            for(int d=0;d<p.dim;d+=16){auto mask=tail_mask(std::min(16,p.dim-d));auto v=_mm512_maskz_loadu_ps(mask,z+d);
                l=_mm512_fmadd_ps(v,_mm512_maskz_loadu_ps(mask,al+d),l);r=_mm512_fmadd_ps(v,_mm512_maskz_loadu_ps(mask,ar+d),r);}
            left=_mm512_reduce_add_ps(l);right=_mm512_reduce_add_ps(r);}
        w.left[i*p.heads+h]=left;w.right[i*p.heads+h]=right;
    }
}
void lr_from_z_mkl(const Graph& g,const Param& p,Workspace& w) {
    // Candidate for microbenchmark only: head-specific skinny GEMM.
    if(w.lr_sgemm_tmp.size()!=g.n*2)throw std::runtime_error("L/R SGEMM workspace was not prepared");
    for(int h=0;h<p.heads;++h){
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),2,p.dim,1,w.z.data()+h*p.dim,p.width(),w.lr_sgemm_weights.data()+h*p.dim*2,2,0,w.lr_sgemm_tmp.data(),2);
        #pragma omp parallel for schedule(static)
        for(uint64_t i=0;i<g.n;++i){w.left[i*p.heads+h]=w.lr_sgemm_tmp[i*2];w.right[i*p.heads+h]=w.lr_sgemm_tmp[i*2+1];}}
}
void lr_reordered(const Graph& g,const Param& p,const Prepared& q,const std::vector<float>& x,Workspace& w,const std::string& policy) {
    const int cols=2*p.heads;
    if(policy=="bf16")cblas_gemm_bf16bf16f32(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),cols,p.in,1,w.xbf.data(),p.in,q.blr_bf16.data(),cols,0,w.lr.data(),cols);
    else if(policy=="fp32")cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),cols,p.in,1,x.data(),p.in,q.blr.data(),cols,0,w.lr.data(),cols);
    else if(policy=="avx") {
        #pragma omp parallel for schedule(static)
        for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){__m512 l=_mm512_setzero_ps(),r=l;
            const float* bl=q.blr_dot.data()+size_t(h)*p.in;const float* br=q.blr_dot.data()+size_t(p.heads+h)*p.in;
            for(int k=0;k<p.in;k+=16){auto mask=tail_mask(std::min(16,p.in-k));auto v=_mm512_maskz_loadu_ps(mask,x.data()+i*p.in+k);
                l=_mm512_fmadd_ps(v,_mm512_maskz_loadu_ps(mask,bl+k),l);r=_mm512_fmadd_ps(v,_mm512_maskz_loadu_ps(mask,br+k),r);}
            w.lr[i*cols+h]=_mm512_reduce_add_ps(l);w.lr[i*cols+p.heads+h]=_mm512_reduce_add_ps(r);}
    } else throw std::runtime_error("unknown LR policy");
    #pragma omp parallel for schedule(static)
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){w.left[i*p.heads+h]=w.lr[i*cols+h];w.right[i*p.heads+h]=w.lr[i*cols+p.heads+h];}
}
void max_prescan(const Graph& g,const Param& p,Workspace& w) {
    #pragma omp parallel for schedule(dynamic,64)
    for(uint64_t i=0;i<g.n;++i){auto mask=tail_mask(p.heads);__m512 mr=_mm512_set1_ps(-std::numeric_limits<float>::infinity());
        for(uint64_t e=g.row[i];e<g.row[i+1];++e)mr=_mm512_max_ps(mr,_mm512_maskz_loadu_ps(mask,w.right.data()+size_t(g.col[e])*p.heads));
        __m512 m=g.row[i]==g.row[i+1]?_mm512_setzero_ps():leaky(_mm512_add_ps(mr,_mm512_maskz_loadu_ps(mask,w.left.data()+i*p.heads)));
        _mm512_mask_storeu_ps(w.max.data()+i*p.heads,mask,m);
    }
}
void standard_aggregate(const Graph& g,const Param& p,Workspace& w,const float* fixed_p,const float* fixed_den,Times* times) {
    // One owner per destination. No atomics, score/alpha allocation, or statistics in hot loop.
    #pragma omp parallel
    {
    Times local;
    #pragma omp for schedule(dynamic,64)
    for(uint64_t i=0;i<g.n;++i){
        __m512 acc[8][4];for(int h=0;h<p.heads;++h)for(int b=0;b<(p.dim+15)/16;++b)acc[h][b]=_mm512_setzero_ps();
        __m512 den=_mm512_setzero_ps();auto hm=tail_mask(p.heads);
        __m512 l=_mm512_maskz_loadu_ps(hm,w.left.data()+i*p.heads),m=_mm512_maskz_loadu_ps(hm,w.max.data()+i*p.heads);
        for(uint64_t e=g.row[i];e<g.row[i+1];++e){
            auto st=tick();
            __m512 weight;
            if(fixed_p)weight=_mm512_maskz_loadu_ps(hm,fixed_p+e*p.heads);
            else {__m512 r=_mm512_maskz_loadu_ps(hm,w.right.data()+size_t(g.col[e])*p.heads);weight=_mm512_exp_ps(_mm512_sub_ps(leaky(_mm512_add_ps(l,r)),m));}
            den=_mm512_add_ps(den,weight);alignas(64) float weights[16];_mm512_store_ps(weights,weight);
            local.score_exp_worker+=seconds(st,tick());st=tick();
            const float* z=w.z.data()+size_t(g.col[e])*p.width();
            for(int h=0;h<p.heads;++h){__m512 f=_mm512_set1_ps(weights[h]);
                for(int b=0;b<(p.dim+15)/16;++b){auto dm=tail_mask(std::min(16,p.dim-b*16));acc[h][b]=_mm512_fmadd_ps(f,_mm512_maskz_loadu_ps(dm,z+h*p.dim+b*16),acc[h][b]);}}
            local.weighted_spmm_worker+=seconds(st,tick());
        }
        auto st=tick();
        _mm512_mask_storeu_ps(w.den.data()+i*p.heads,hm,fixed_den?_mm512_maskz_loadu_ps(hm,fixed_den+i*p.heads):den);
        for(int h=0;h<p.heads;++h)for(int b=0;b<(p.dim+15)/16;++b){auto dm=tail_mask(std::min(16,p.dim-b*16));_mm512_mask_storeu_ps(w.out.data()+i*p.width()+h*p.dim+b*16,dm,acc[h][b]);}
        local.output_write_worker+=seconds(st,tick());
    }
    if constexpr(profiling){if(times){
        #pragma omp critical(gat_standard_profile)
        {times->score_exp_worker+=local.score_exp_worker;times->weighted_spmm_worker+=local.weighted_spmm_worker;times->output_write_worker+=local.output_write_worker;}
    }}
    }
}
void finish(const Graph& g,const Param& p,Workspace& w,bool hidden,Times& t) {
    auto a=Clock::now();
    #pragma omp parallel for schedule(static)
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){float d=w.den[i*p.heads+h];__m512 scale=_mm512_set1_ps(d>0?1/d:0);
        for(int b=0;b<p.dim;b+=16){auto mask=tail_mask(std::min(16,p.dim-b));float* out=w.out.data()+i*p.width()+h*p.dim+b;
            _mm512_mask_storeu_ps(out,mask,_mm512_mul_ps(_mm512_maskz_loadu_ps(mask,out),scale));}}
    t.normalize=seconds(a,Clock::now());a=Clock::now();
    if(hidden){
        #pragma omp parallel for schedule(static)
        for(size_t i=0;i<w.out.size();i+=16){auto mask=tail_mask(int(std::min<size_t>(16,w.out.size()-i)));_mm512_mask_storeu_ps(w.out.data()+i,mask,activation(_mm512_maskz_loadu_ps(mask,w.out.data()+i)));}
    }
    t.activation=seconds(a,Clock::now());
}
void standard_layer(const Graph& g,const Param& p,const Prepared& q,const std::vector<float>& input,Workspace& w,bool hidden,Times& t,bool fp32,const std::string& lr_policy) {
    auto begin=Clock::now(),a=begin;
    if(!fp32)convert(input.data(),w.xbf.data(),input.size());t.convert=seconds(a,Clock::now());
    a=Clock::now();projection(g,p,q,input,w,!fp32);t.projection=seconds(a,Clock::now());
    a=Clock::now();
    if(lr_policy=="z_avx")lr_from_z(g,p,w);
    else if(lr_policy=="z_mkl")lr_from_z_mkl(g,p,w);
    else lr_reordered(g,p,q,input,w,lr_policy);
    t.lr=seconds(a,Clock::now());a=Clock::now();max_prescan(g,p,w);t.max_prescan=seconds(a,Clock::now());
    a=Clock::now();standard_aggregate(g,p,w,nullptr,nullptr,&t);t.aggregate=seconds(a,Clock::now());finish(g,p,w,hidden,t);t.total=seconds(begin,Clock::now());
}
}
