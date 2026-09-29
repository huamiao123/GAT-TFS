#include <immintrin.h>
#include <cmath>
#include <cstdio>
int main(){
 alignas(64) float x[16],v[16],y[16];
 for(int i=0;i<16;i++) x[i]=(i-8)*0.25f;
 __m512 sum=_mm512_load_ps(x);
 __m512 zero=_mm512_setzero_ps();
 __m512 slope=_mm512_set1_ps(.2f);
 __m512 evec=_mm512_mask_mul_ps(sum,_mm512_cmp_ps_mask(sum,zero,_CMP_LT_OQ),sum,slope);
 _mm512_store_ps(v,evec);
 __m512 ex=_mm512_exp_ps(_mm512_sub_ps(evec,_mm512_set1_ps(.5f)));
 _mm512_store_ps(y,ex);
 for(int i=0;i<16;i++) printf("%d x=%g leak=%g scalar=%g exp=%g scalar_exp=%g\n",i,x[i],v[i],x[i]<0?.2f*x[i]:x[i],y[i],std::exp(v[i]-.5f));
}

