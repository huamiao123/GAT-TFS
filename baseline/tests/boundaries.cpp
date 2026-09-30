#include "gat.hpp"
#include <iostream>
#include <random>
#include <functional>
using namespace gat;
static void expect(bool condition,const std::string& message){if(!condition)throw std::runtime_error(message);}
static void rejects(const std::string& name,const std::function<void()>& fn){bool failed=false;try{fn();}catch(const std::exception&){failed=true;}expect(failed,"negative test accepted: "+name);std::cout<<"NEGATIVE PASS "<<name<<"\n";}
static Param params(int D,int K,int d){Param p;p.in=D;p.heads=K;p.dim=d;p.w.resize(D*K*d);p.al.resize(K*d);p.ar.resize(K*d);
    std::mt19937 rng(D+K+d);std::uniform_real_distribution<float> rnd(-.3,.3);for(auto& x:p.w)x=rnd(rng);for(auto& x:p.al)x=rnd(rng);for(auto& x:p.ar)x=rnd(rng);return p;}
static Graph graph(int D){Graph g;g.n=35;g.din=D;g.classes=40;g.row={0};
    const int deg[]={0,1,2,15,16,17,31,32,33,63,64,65,129};
    for(uint64_t i=0;i<g.n;++i){int count=deg[i%13];for(int e=0;e<count;++e)g.col.push_back(uint32_t((i*3+e*7)%g.n));g.row.push_back(g.col.size());}
    g.e=g.col.size();g.x.resize(g.n*D);for(size_t i=0;i<g.x.size();++i)g.x[i]=std::sin(float(i)*.17f)*.5f;g.validate();return g;}
// Independent FP64 oracle, including scalar projection/attention/softmax.
static std::vector<float> oracle(const Graph& g,const Param& p){
    std::vector<double> z(g.n*p.width()),l(g.n*p.heads),r(l.size());std::vector<float> out(z.size());
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h)for(int d=0;d<p.dim;++d){double v=0;for(int k=0;k<p.in;++k)v+=double(g.x[i*p.in+k])*p.w[size_t(k)*p.width()+h*p.dim+d];z[i*p.width()+h*p.dim+d]=v;l[i*p.heads+h]+=v*p.al[h*p.dim+d];r[i*p.heads+h]+=v*p.ar[h*p.dim+d];}
    for(uint64_t i=0;i<g.n;++i)for(int h=0;h<p.heads;++h){std::vector<double> score;double m=-INFINITY,den=0;for(uint64_t e=g.row[i];e<g.row[i+1];++e){double v=l[i*p.heads+h]+r[size_t(g.col[e])*p.heads+h];v=v>=0?v:.2*v;score.push_back(v);m=std::max(m,v);}for(auto& v:score){v=std::exp(v-m);den+=v;}
        for(int d=0;d<p.dim;++d){double sum=0;for(uint64_t e=g.row[i];e<g.row[i+1];++e)sum+=score[e-g.row[i]]*z[size_t(g.col[e])*p.width()+h*p.dim+d];out[i*p.width()+h*p.dim+d]=den>0?float(sum/den):0;}}
    return out;
}
int main(int argc,char** argv){try{
    omp_set_dynamic(0);omp_set_num_threads(2);mkl_set_num_threads(2);mkl_set_dynamic(0);
    // Ties at BF16 mantissa midpoint and tails in the hardware RNE conversion.
    std::vector<float> values(37);for(size_t i=0;i<values.size();++i){uint32_t u=0x3f808000u+uint32_t(i)*0x10000;std::memcpy(&values[i],&u,4);}
    std::vector<BF16> b(values.size());convert(values.data(),b.data(),b.size());for(size_t i=0;i<b.size();++i)expect(b[i]==rne(values[i]),"RNE ties/tail mismatch");
    for(int D:{17,33,128,256})for(int K:{1,2,4,8})for(int d:{7,32,40,47}) {
        Graph g=graph(D);Param p=params(D,K,d);Prepared q=prepare(p);auto s=degree_schedule(g);
        Workspace r,b,t;r.allocate(g,p,"ref_fp32");b.allocate(g,p,"standard_bf16");t.allocate(g,p,"tfs_bf16");Times rt,bt,tt;
        ref_layer(g,p,g.x,r,false,rt);auto err=errors(oracle(g,p),r.out);expect(err.finite && err.max_abs<1e-4 && err.relative_l2<1e-4,"R0 versus FP64 oracle failed");
        standard_layer(g,p,q,g.x,b,false,bt,false);tfs_layer(g,p,q,s,g.x,t,false,64,tt,"bf16");
        expect(errors(r.out,b.out).finite && errors(r.out,t.out).finite,"BF16 nonfinite");
        for(int z=0;z<p.width();++z)expect(r.out[z]==0 && b.out[z]==0 && t.out[z]==0,"empty-row semantics failed");
        // Test exact max prescan against actual scalar scores from these L/R.
        for(uint64_t i=0;i<g.n;++i)for(int h=0;h<K;++h){float m=-INFINITY;for(uint64_t e=g.row[i];e<g.row[i+1];++e)m=std::max(m,leak(b.left[i*K+h]+b.right[size_t(g.col[e])*K+h]));if(g.row[i]==g.row[i+1])m=0;expect(m==b.max[i*K+h],"max-prescan mismatch");}
        // Same R0 p/ell in both operators and BF16 scalar-emulated edge projection.
        projection(g,p,q,g.x,b,true);standard_aggregate(g,p,b,r.p.data(),r.den.data());Times f;finish(g,p,b,false,f);
        tfs_aggregate(g,p,q,s,t,64,tt,r.p.data(),r.den.data());finish(g,p,t,false,f);
        std::vector<float> emu(t.out.size(),0);
        for(uint64_t i=0;i<g.n;++i)for(int h=0;h<K;++h){for(uint64_t e=g.row[i];e<g.row[i+1];++e)for(int k=0;k<D;++k){float v=expand(rne(r.p[e*K+h]*expand(rne(g.x[size_t(g.col[e])*D+k]))));for(int z=0;z<d;++z)emu[i*p.width()+h*d+z]+=v*expand(q.w[size_t(k)*p.width()+h*d+z]);}
            float den=r.den[i*K+h];for(int z=0;z<d;++z)emu[i*p.width()+h*d+z]=den>0?emu[i*p.width()+h*d+z]/den:0;}
        auto ce=errors(emu,t.out);expect(ce.finite && ce.max_abs<2e-4 && ce.relative_l2<2e-4,"AMX weighted edge projection versus emulator failed");
        std::cout<<"BOUNDARY PASS D="<<D<<" K="<<K<<" d="<<d<<" ref_max="<<err.max_abs<<" amx_emulator_rel="<<ce.relative_l2<<"\n";
    }
    // Score-specific adversarial max-prescan/exp tests including late peak and common offset.
    Graph g=graph(17);Param p=params(17,8,32);Workspace w;w.allocate(g,p,"standard_bf16");
    for(int scenario=0;scenario<5;++scenario){for(uint64_t i=0;i<g.n;++i)for(int h=0;h<8;++h){w.left[i*8+h]=scenario==3?10000.f:scenario==4?-10000.f:float(h)-4;
        w.right[i*8+h]=scenario==0?0:scenario==1?(i==g.n-1?1000:-1000):scenario==2?float(i)-17:std::sin(float(i+h))*8;}
        for(size_t i=0;i<w.z.size();++i)w.z[i]=float(i%9)*.1f;
        max_prescan(g,p,w);standard_aggregate(g,p,w);Times tm;finish(g,p,w,false,tm);require_finite(w.out,"adversarial scores");
        for(uint64_t i=0;i<g.n;++i)for(int h=0;h<8;++h)expect(g.row[i]==g.row[i+1]?w.den[i*8+h]==0:w.den[i*8+h]>=1,"stable denominator invalid");}
    auto nan=std::vector<float>{NAN};expect(!errors({1},nan).finite,"NaN comparison incorrectly passed");
    expect(!errors({1},{INFINITY}).finite,"Inf comparison incorrectly passed");
    rejects("nonfinite_input",[&]{auto bad=g;bad.x[0]=NAN;bad.validate();});
    rejects("nonmonotone_CSR",[&]{auto bad=g;bad.row[3]=bad.e+1;bad.validate();});
    rejects("out_of_range_source",[&]{auto bad=g;bad.col[0]=g.n;bad.validate();});
    rejects("wrong_parameter_size",[&]{auto bad=p;bad.w.pop_back();bad.validate();});
    rejects("error_size_mismatch",[&]{errors({1},{1,2});});
    rejects("allocation_overflow",[&]{checked(UINT64_MAX,2);});
    if(argc==2){Graph fixture=graph(128);save_graph(fixture,std::string(argv[1])+"/oracle.gatbin");auto m=random_model(128,40,11);save_model(m,std::string(argv[1])+"/oracle.weights");}
    std::cout<<"TESTS PASS boundary_cases=64 R0_FP64_oracle=true\n";return 0;
}catch(const std::exception& e){std::cerr<<"TESTS FAIL "<<e.what()<<"\n";return 1;}}
