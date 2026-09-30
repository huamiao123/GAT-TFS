#include "gat.hpp"
#include <iomanip>
#include <iostream>
#include <random>
namespace gat {
size_t checked(uint64_t a,uint64_t b) {
    if(b && a>SIZE_MAX/b) throw std::runtime_error("size overflow");
    return size_t(a*b);
}
void require_finite(const std::vector<float>& a,const std::string& name) {
    for(size_t i=0;i<a.size();++i) if(!std::isfinite(a[i]))
        throw std::runtime_error("nonfinite "+name+" index="+std::to_string(i));
}
template<class T> void read(std::ifstream& f,T* data,size_t n) {
    if(n>size_t(std::numeric_limits<std::streamsize>::max())/sizeof(T)) throw std::runtime_error("section too large");
    f.read(reinterpret_cast<char*>(data),n*sizeof(T));
    if(!f) throw std::runtime_error("truncated file");
}
template<class T> void write(std::ofstream& f,const T* data,size_t n) {
    f.write(reinterpret_cast<const char*>(data),n*sizeof(T));
    if(!f) throw std::runtime_error("write failed");
}
void Graph::validate() const {
    if(!n || n>INT32_MAX || din<=0 || classes<=0 || row.size()!=n+1 || col.size()!=e || x.size()!=checked(n,din))
        throw std::runtime_error("invalid graph sizes");
    if(row.front()!=0 || row.back()!=e) throw std::runtime_error("bad CSR bounds");
    for(size_t i=0;i<n;++i) if(row[i]>row[i+1]) throw std::runtime_error("nonmonotone CSR");
    for(auto j:col) if(j>=n) throw std::runtime_error("source index out of bounds");
    require_finite(x,"graph input");
}
Graph load_graph(const std::string& path) {
    std::ifstream f(path,std::ios::binary); if(!f) throw std::runtime_error("cannot open graph");
    char magic[8];read(f,magic,8); if(std::memcmp(magic,"GATBIN1",7)) throw std::runtime_error("bad graph magic");
    Graph g;uint32_t d,c;read(f,&g.n,1);read(f,&g.e,1);read(f,&d,1);read(f,&c,1);
    if(!g.n || g.n>INT32_MAX || d>INT32_MAX || c>INT32_MAX || !d || !c) throw std::runtime_error("bad graph dimensions");
    g.din=d;g.classes=c;g.row.resize(g.n+1);g.col.resize(checked(g.e,1));g.x.resize(checked(g.n,d));
    read(f,g.row.data(),g.row.size());read(f,g.col.data(),g.col.size());read(f,g.x.data(),g.x.size());
    g.validate(); return g;
}
void save_graph(const Graph& g,const std::string& path) {
    g.validate();std::ofstream f(path,std::ios::binary);const char magic[8]="GATBIN1";
    uint32_t d=g.din,c=g.classes;write(f,magic,8);write(f,&g.n,1);write(f,&g.e,1);write(f,&d,1);write(f,&c,1);
    write(f,g.row.data(),g.row.size());write(f,g.col.data(),g.col.size());write(f,g.x.data(),g.x.size());
}
void Param::validate() const {
    if(in<=0 || heads<=0 || heads>8 || dim<=0 || dim>64 || w.size()!=checked(in,width()) || al.size()!=size_t(width()) || ar.size()!=size_t(width()))
        throw std::runtime_error("invalid parameter shape (heads <=8, head_dim <=64)");
    require_finite(w,"W");require_finite(al,"aL");require_finite(ar,"aR");
}
Model random_model(int din,int classes,int seed) {
    Model m;
    for(int layer=0;layer<3;++layer) {
        Param p;p.in=layer?256:din;p.heads=layer<2?8:1;p.dim=layer<2?32:classes;
        p.w.resize(checked(p.in,p.width()));p.al.resize(p.width());p.ar.resize(p.width());
        std::mt19937 rng(seed+11*layer);std::normal_distribution<float> norm;
        for(auto& x:p.w)x=norm(rng)/std::sqrt(float(p.in));
        for(auto& x:p.al)x=norm(rng)/std::sqrt(float(p.dim));
        for(auto& x:p.ar)x=norm(rng)/std::sqrt(float(p.dim));
        p.validate();m.push_back(std::move(p));
    }
    return m;
}
void save_model(const Model& m,const std::string& path) {
    std::ofstream f(path,std::ios::binary);const char magic[8]="GATWGT1";write(f,magic,8);
    uint32_t n=m.size();write(f,&n,1);
    for(const auto& p:m) {p.validate();uint32_t shape[3]={uint32_t(p.in),uint32_t(p.heads),uint32_t(p.dim)};
        write(f,shape,3);write(f,p.w.data(),p.w.size());write(f,p.al.data(),p.al.size());write(f,p.ar.data(),p.ar.size());}
}
Model load_model(const std::string& path,int din,int classes) {
    std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("cannot open weights");
    char magic[8];read(f,magic,8);if(std::memcmp(magic,"GATWGT1",7))throw std::runtime_error("bad weights magic");
    uint32_t count;read(f,&count,1);if(count!=3)throw std::runtime_error("model must have 3 layers");
    Model m;
    for(int l=0;l<3;++l) {uint32_t s[3];read(f,s,3);
        if(s[0]!=uint32_t(l?256:din) || s[1]!=uint32_t(l<2?8:1) || s[2]!=uint32_t(l<2?32:classes))throw std::runtime_error("checkpoint model shape mismatch");
        Param p;p.in=s[0];p.heads=s[1];p.dim=s[2];p.w.resize(checked(p.in,p.width()));p.al.resize(p.width());p.ar.resize(p.width());
        read(f,p.w.data(),p.w.size());read(f,p.al.data(),p.al.size());read(f,p.ar.data(),p.ar.size());p.validate();m.push_back(std::move(p));}
    return m;
}
BF16 rne(float x) {uint32_t u;std::memcpy(&u,&x,4);
    if((u&0x7f800000u)==0x7f800000u && (u&0x7fffffu))return BF16((u>>16)|0x40u);
    u+=0x7fffu+((u>>16)&1u);return BF16(u>>16);
}
float expand(BF16 b) {uint32_t u=uint32_t(b)<<16;float x;std::memcpy(&x,&u,4);return x;}
void convert(const float* a,BF16* b,size_t n) {
    #pragma omp parallel for schedule(static)
    for(size_t base=0;base<n;base+=16) {
        int len=int(std::min<size_t>(16,n-base));auto mask=tail_mask(len);
        __m256bh v=_mm512_cvtneps_pbh(_mm512_maskz_loadu_ps(mask,a+base));
        if(len==16)_mm256_storeu_si256(reinterpret_cast<__m256i*>(b+base),(__m256i)v);
        else {alignas(32) BF16 tmp[16];_mm256_store_si256(reinterpret_cast<__m256i*>(tmp),(__m256i)v);std::memcpy(b+base,tmp,len*2);}
    }
}
std::vector<float> quantized(const std::vector<float>& a) {
    std::vector<float> q(a.size());for(size_t i=0;i<a.size();++i)q[i]=expand(rne(a[i]));return q;
}
Prepared prepare(const Param& p,bool attention,bool tfs_pack,bool bf16_weights) {
    p.validate();auto start=Clock::now();Prepared q;q.padded_in=(p.in+31)/32*32;q.output_blocks=(p.dim+15)/16;
    if(bf16_weights || tfs_pack){q.w.resize(p.w.size());convert(p.w.data(),q.w.data(),q.w.size());}
    if(attention){
    q.blr.resize(size_t(p.in)*2*p.heads);
    for(int k=0;k<p.in;++k)for(int h=0;h<p.heads;++h) {
        float l=0,r=0;for(int d=0;d<p.dim;++d){float w=p.w[size_t(k)*p.width()+h*p.dim+d];l+=w*p.al[h*p.dim+d];r+=w*p.ar[h*p.dim+d];}
        q.blr[size_t(k)*2*p.heads+h]=l;q.blr[size_t(k)*2*p.heads+p.heads+h]=r;
    }
    q.blr_bf16.resize(q.blr.size());convert(q.blr.data(),q.blr_bf16.data(),q.blr.size());
    q.blr_dot.resize(q.blr.size());for(int c=0;c<2*p.heads;++c)for(int k=0;k<p.in;++k)q.blr_dot[size_t(c)*p.in+k]=q.blr[size_t(k)*2*p.heads+c];
    require_finite(q.blr,"static attention contraction");
    }
    if(tfs_pack){
    q.packed.assign(size_t(p.heads)*(q.padded_in/32)*q.output_blocks*512,0);
    for(int h=0;h<p.heads;++h)for(int kb=0;kb<q.padded_in/32;++kb)for(int ob=0;ob<q.output_blocks;++ob)
        for(int kp=0;kp<16;++kp)for(int n=0;n<16;++n)for(int pair=0;pair<2;++pair) {
            int k=kb*32+kp*2+pair,d=ob*16+n;
            if(k<p.in && d<p.dim)q.packed[(((size_t(h)*(q.padded_in/32)+kb)*q.output_blocks+ob)*512)+kp*32+n*2+pair]=q.w[size_t(k)*p.width()+h*p.dim+d];
        }
    }
    q.weight_prepare_s=seconds(start,Clock::now());return q;
}
Schedule degree_schedule(const Graph& g) {
    Schedule q;auto start=Clock::now();q.perm.resize(g.n);std::iota(q.perm.begin(),q.perm.end(),0);
    std::sort(q.perm.begin(),q.perm.end(),[&](uint32_t a,uint32_t b){auto da=g.row[a+1]-g.row[a],db=g.row[b+1]-g.row[b];return da!=db?da<db:a<b;});
    for(size_t base=0;base<g.n;base+=16){uint32_t row=q.perm[std::min<uint64_t>(g.n,base+16)-1];q.neighbor_steps+=g.row[row+1]-g.row[row];}
    q.degree_sort_s=seconds(start,Clock::now());return q;
}
void Workspace::allocate(const Graph& g,const Param& p,const std::string& path) {
    size_t nw=checked(g.n,p.width()),nk=checked(g.n,p.heads);
    out.resize(nw);left.resize(nk);right.resize(nk);max.resize(nk);den.resize(nk);
    if(path!="ref_fp32" && path!="standard_fp32")xbf.resize(checked(g.n,p.in));
    if(path!="tfs_bf16")z.resize(nw);
    if(path=="tfs_bf16" || path=="matched_attention")lr.resize(checked(g.n,2*p.heads));
    if(path=="tfs_bf16") {thread_hbuf.resize(size_t(omp_get_max_threads())*16*((p.in+31)/32*32));thread_cbuf.resize(size_t(omp_get_max_threads())*16*64);}
    if(path=="ref_fp32"){score.resize(checked(g.e,p.heads));alpha.resize(score.size());this->p.resize(score.size());}
}
void Workspace::prepare_lr_sgemm(const Graph& g,const Param& p) {
    lr_sgemm_tmp.resize(g.n*2);lr_sgemm_weights.resize(size_t(p.width())*2);
    for(int h=0;h<p.heads;++h)for(int d=0;d<p.dim;++d){lr_sgemm_weights[(h*p.dim+d)*2]=p.al[h*p.dim+d];lr_sgemm_weights[(h*p.dim+d)*2+1]=p.ar[h*p.dim+d];}
}
size_t Workspace::bytes() const {return (z.size()+left.size()+right.size()+max.size()+den.size()+out.size()+lr.size()+score.size()+p.size()+alpha.size()+thread_cbuf.size()+lr_sgemm_weights.size()+lr_sgemm_tmp.size())*4+(xbf.size()+thread_hbuf.size())*2;}
Trace capture(const Workspace& w) {return {w.z,w.left,w.right,w.max,w.den,w.score,w.p,w.alpha};}
Error errors(const std::vector<float>& ref,const std::vector<float>& got) {
    if(ref.empty() || ref.size()!=got.size())throw std::runtime_error("comparison size mismatch");
    Error e;std::vector<double> abs;abs.reserve(ref.size());double sq=0,den=0;
    for(size_t i=0;i<ref.size();++i){if(!std::isfinite(ref[i]) || !std::isfinite(got[i])){e.finite=false;continue;}
        double d=double(got[i])-ref[i],a=std::abs(d);e.max_abs=std::max(e.max_abs,a);e.mean_abs+=a;sq+=d*d;den+=double(ref[i])*ref[i];abs.push_back(a);}
    if(!e.finite){e.max_abs=e.mean_abs=e.relative_l2=e.rmse=e.p50=e.p90=e.p99=std::numeric_limits<double>::infinity();return e;}
    e.mean_abs/=ref.size();e.rmse=std::sqrt(sq/ref.size());e.relative_l2=den>0?std::sqrt(sq/den):(sq==0?0:std::numeric_limits<double>::infinity());
    auto pct=[&](double q){size_t k=std::min(abs.size()-1,size_t(std::ceil(q*abs.size())-1));std::nth_element(abs.begin(),abs.begin()+k,abs.end());return abs[k];};
    e.p50=pct(.5);e.p90=pct(.9);e.p99=pct(.99);return e;
}
void print_error(const std::string& path,int layer,const std::string& stage,const Error& e) {
    std::cout<<std::setprecision(10)<<"ERROR path="<<path<<" layer="<<layer<<" stage="<<stage<<" finite="<<e.finite
      <<" max_abs="<<e.max_abs<<" mean_abs="<<e.mean_abs<<" relative_l2="<<e.relative_l2<<" rmse="<<e.rmse<<" p50="<<e.p50<<" p90="<<e.p90<<" p99="<<e.p99<<"\n";
}
void print_times(const std::string& path,const std::string& mode,int layer,int rep,const Graph& g,const Param& p,const Workspace& w,const Times& t,double stat,int panel) {
    std::cout<<std::setprecision(10)<<"{\"path\":\""<<path<<"\",\"mode\":\""<<mode<<"\",\"layer\":"<<layer<<",\"rep\":"<<rep
      <<",\"N\":"<<g.n<<",\"E\":"<<g.e<<",\"D\":"<<p.in<<",\"K\":"<<p.heads<<",\"d\":"<<p.dim<<",\"threads\":"<<omp_get_max_threads()
      <<",\"profile\":"<<(profiling?"true":"false")<<",\"precision\":\""<<(path=="ref_fp32"||path=="standard_fp32"?"FP32":"BF16_RNE_FP32_state")<<"\""
      <<",\"projection_ms\":"<<t.projection*1000<<",\"lr_ms\":"<<t.lr*1000<<",\"max_prescan_ms\":"<<t.max_prescan*1000
      <<",\"score_exp_aggregate_ms\":"<<t.aggregate*1000<<",\"normalize_ms\":"<<t.normalize*1000<<",\"activation_ms\":"<<t.activation*1000
      <<",\"convert_ms\":"<<t.convert*1000<<",\"total_ms\":"<<t.total*1000<<",\"workspace_bytes\":"<<w.bytes()<<",\"static_preprocess_ms\":"<<stat*1000
      <<",\"panel_R\":"<<panel<<",\"tile_rows\":16,\"score_exp_worker_ms\":"<<t.score_exp_worker*1000
      <<",\"gather_weight_convert_worker_ms\":"<<t.gather_weight_convert_worker*1000<<",\"tile_load_worker_ms\":"<<t.tile_load_worker*1000
      <<",\"gather_load_worker_ms\":"<<t.gather_load_worker*1000<<",\"p_times_x_convert_worker_ms\":"<<t.p_times_x_convert_worker*1000
      <<",\"weighted_spmm_worker_ms\":"<<t.weighted_spmm_worker*1000
      <<",\"amx_compute_worker_ms\":"<<t.amx_compute_worker*1000<<",\"tile_store_worker_ms\":"<<t.tile_store_worker*1000
      <<",\"tile_config_worker_ms\":"<<t.tile_config_worker*1000<<",\"scheduling_worker_ms\":"<<t.scheduling_worker*1000
      <<",\"output_write_worker_ms\":"<<t.output_write_worker*1000<<",\"neighbor_steps\":"<<t.neighbor_steps<<",\"executed_fma\":"<<t.executed_fma
      <<",\"padding_executed_flops\":"<<(t.executed_fma?2*(t.executed_fma-g.e*uint64_t(p.heads)*p.in*p.dim):0)<<"}\n";
}
void save_fixture(const Graph& g,const Param& p,const Fixture& q,const std::string& path) {
    if(q.input.size()!=checked(g.n,p.in)||q.p.size()!=checked(g.e,p.heads)||q.den.size()!=checked(g.n,p.heads))throw std::runtime_error("bad fixture sizes");
    std::ofstream f(path,std::ios::binary);const char magic[8]="GATFIX1";write(f,magic,8);
    uint64_t sizes[5]={g.n,g.e,uint64_t(p.in),uint64_t(p.heads),uint64_t(p.dim)};write(f,sizes,5);
    // Store CSR to reject an equal-shape but different graph.
    write(f,g.row.data(),g.row.size());write(f,g.col.data(),g.col.size());
    write(f,p.w.data(),p.w.size());write(f,p.al.data(),p.al.size());write(f,p.ar.data(),p.ar.size());
    write(f,q.input.data(),q.input.size());write(f,q.p.data(),q.p.size());write(f,q.den.data(),q.den.size());
}
Fixture load_fixture(const Graph& g,const Param& p,const std::string& path) {
    std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("cannot open fixture");char magic[8];read(f,magic,8);
    if(std::memcmp(magic,"GATFIX1",7))throw std::runtime_error("bad fixture magic");
    uint64_t s[5];read(f,s,5);if(s[0]!=g.n||s[1]!=g.e||s[2]!=uint64_t(p.in)||s[3]!=uint64_t(p.heads)||s[4]!=uint64_t(p.dim))throw std::runtime_error("fixture shape mismatch");
    std::vector<uint64_t> row(g.row.size());std::vector<uint32_t> col(g.col.size());read(f,row.data(),row.size());read(f,col.data(),col.size());
    if(row!=g.row || col!=g.col)throw std::runtime_error("fixture CSR mismatch");
    for(const auto* master:{&p.w,&p.al,&p.ar}){std::vector<float> v(master->size());read(f,v.data(),v.size());if(v!=*master)throw std::runtime_error("fixture checkpoint mismatch");}
    Fixture q;q.input.resize(checked(g.n,p.in));q.p.resize(checked(g.e,p.heads));q.den.resize(checked(g.n,p.heads));
    read(f,q.input.data(),q.input.size());read(f,q.p.data(),q.p.size());read(f,q.den.data(),q.den.size());
    require_finite(q.input,"fixture input");require_finite(q.p,"fixture p");require_finite(q.den,"fixture denominator");
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){float sum=0;
        for(uint64_t e=g.row[i];e<g.row[i+1];++e){float x=q.p[e*p.heads+h];if(x<0 || x>1)throw std::runtime_error("fixture p outside [0,1]");sum+=x;}
        float den=q.den[i*p.heads+h];if(std::abs(sum-den)>1e-5f*std::max(1.f,sum) || (g.row[i]!=g.row[i+1] && den<=0))throw std::runtime_error("fixture denominator mismatch");}
    return q;
}
}
