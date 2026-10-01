#include "gat.hpp"
namespace gat {
void ref_layer(const Graph& g,const Param& p,const std::vector<float>& input,Workspace& w,bool hidden,Times& t) {
    auto begin=Clock::now(),a=begin;
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),p.width(),p.in,1,input.data(),p.in,p.w.data(),p.width(),0,w.z.data(),p.width());
    t.projection=seconds(a,Clock::now());a=Clock::now();lr_from_z(g,p,w,true);t.lr=seconds(a,Clock::now());
    a=Clock::now();
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){
        float m=-std::numeric_limits<float>::infinity();
        for(uint64_t e=g.row[i];e<g.row[i+1];++e){float s=leak(w.left[i*p.heads+h]+w.right[size_t(g.col[e])*p.heads+h]);w.score[e*p.heads+h]=s;m=std::max(m,s);}
        // Empty rows: max=0, denominator=0, output=0 in all paths.
        w.max[i*p.heads+h]=g.row[i]==g.row[i+1]?0:m;
        float den=0;
        for(uint64_t e=g.row[i];e<g.row[i+1];++e){float v=std::exp(w.score[e*p.heads+h]-m);w.p[e*p.heads+h]=v;den+=v;}
        w.den[i*p.heads+h]=den;
        for(uint64_t e=g.row[i];e<g.row[i+1];++e)w.alpha[e*p.heads+h]=w.p[e*p.heads+h]/den;
    }
    std::fill(w.out.begin(),w.out.end(),0);
    for(uint64_t i=0;i<g.n;++i)for(uint64_t e=g.row[i];e<g.row[i+1];++e)for(int h=0;h<p.heads;++h){
        float alpha=w.alpha[e*p.heads+h];
        for(int d=0;d<p.dim;++d)w.out[i*p.width()+h*p.dim+d]+=alpha*w.z[size_t(g.col[e])*p.width()+h*p.dim+d];
    }
    t.aggregate=seconds(a,Clock::now());a=Clock::now();
    if(hidden)for(auto& v:w.out)v=elu(v);
    t.activation=seconds(a,Clock::now());t.total=seconds(begin,Clock::now());
}
}
