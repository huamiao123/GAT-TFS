// 最小化复现：16行，每行1个邻居，验证TFS计算是否正确
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <initializer_list>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>

struct TileCfg {
    uint8_t  palette_id, start_row, reserved[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
} __attribute__((aligned(64)));

static inline __bf16 f2bf16(float f){
    uint32_t u; memcpy(&u,&f,4);
    uint16_t h=(uint16_t)(u>>16);
    __bf16 r; memcpy(&r,&h,2); return r;
}

int main(){
    syscall(SYS_arch_prctl,0x1023,18);

    // ── 构造测试：16行，每行邻居就是自己，H=identity，W=identity
    // 期望：C[i][i] = 1.0，其余 = 0.0
    const int N=16, K=32;  // 最小尺寸：1个K_block
    alignas(64) __bf16 H[N*K], W[K*N];
    memset(H,0,sizeof(H)); memset(W,0,sizeof(W));
    for(int i=0;i<N;i++){
        H[i*K+i]=f2bf16(1.f);   // H[i][i]=1，其余0
        W[i*N+i]=f2bf16(1.f);   // W[i][i]=1，其余0（K×N identity）
    }
    // 不对，K=32, N=16，W是32×16
    // 重新：H[N=16][K=32], W[K=32][N=16]
    // H[i][0]=1 for all i，W[0][j]=1 for all j
    // 期望：C[i][j] = H[i]·W[:,j] = H[i][0]*W[0][j] = 1*1 = 1 for all i,j
    memset(H,0,sizeof(H)); memset(W,0,sizeof(W));
    for(int i=0;i<N;i++) H[i*K+0]=f2bf16(1.f);   // H[i][0]=1
    for(int j=0;j<N;j++) W[0*N+j]=f2bf16(1.f);   // W[0][j]=1

    // VNNI打包W：[1个kb][1个ob][16 kpair][32 BF16]
    alignas(64) __bf16 Wv[16*32]={};
    for(int kp=0;kp<16;kp++){
        int k0=kp*2, k1=k0+1;
        for(int n=0;n<16;n++){
            Wv[kp*32+n*2+0] = W[k0*N+n];
            Wv[kp*32+n*2+1] = W[k1*N+n];
        }
    }

    // Tile配置
    TileCfg cfg={};
    cfg.palette_id=1;
    for(int t:{0,1,2,3,4}){cfg.rows[t]=16;cfg.colsb[t]=64;}
    _tile_loadconfig(&cfg);

    // A_buf：16行每行32个BF16 = H[j=0][0:32]（邻居都是节点0）
    alignas(64) __bf16 Abuf[16*32]={};
    for(int r=0;r<16;r++){
        // 每行的邻居是节点 r（自环）
        __m512i v=_mm512_loadu_si512((const __m512i*)(H+r*K));
        _mm512_store_si512((__m512i*)(Abuf+r*32),v);
    }

    // TFS计算
    alignas(64) float C[N*N]={};
    _tile_zero(0); _tile_zero(1);
    _tile_loadd(3, Wv,    64);  // B0: ob=0
    _tile_loadd(4, Wv+16*32/2, 64); // B1: 不存在，用同一个
    // 只测ob=0（N=16输出列）
    _tile_loadd(3, Wv, 64);
    _tile_loadd(2, Abuf, 64);
    _tile_dpbf16ps(0, 2, 3);  // C0 += A × B0

    // store C
    alignas(64) float Cout[16*16]={};
    _tile_stored(0, Cout, 64); // stride=64bytes=16 FP32
    _tile_release();

    // 验证
    printf("C[0][0..3] = %.3f %.3f %.3f %.3f  期望: 1 0 0 0\n",
           Cout[0],Cout[1],Cout[2],Cout[3]);
    printf("C[1][0..3] = %.3f %.3f %.3f %.3f  期望: 0 1 0 0\n",
           Cout[16],Cout[17],Cout[18],Cout[19]);

    // 参考答案（手算）
    float ref[16*16]={};
    for(int i=0;i<16;i++)
        for(int k=0;k<32;k++)
            for(int n=0;n<16;n++){
                float hi,wk;
                uint16_t h2; memcpy(&h2,&H[i*K+k],2);
                uint32_t hu=(uint32_t)h2<<16; memcpy(&hi,&hu,4);
                uint16_t w2; memcpy(&w2,&W[k*N+n],2);
                uint32_t wu=(uint32_t)w2<<16; memcpy(&wk,&wu,4);
                ref[i*16+n]+=hi*wk;
            }
    printf("Ref[0][0..3] = %.3f %.3f %.3f %.3f\n",ref[0],ref[1],ref[2],ref[3]);
    printf("Ref[1][0..3] = %.3f %.3f %.3f %.3f\n",ref[16],ref[17],ref[18],ref[19]);

    float maxerr=0;
    for(int i=0;i<256;i++){float d=Cout[i]-ref[i];if(d<0)d=-d;if(d>maxerr)maxerr=d;}
    printf("最大绝对误差: %.6f  %s\n", maxerr, maxerr<0.01?"✅ PASS":"❌ FAIL");
    return 0;
}
