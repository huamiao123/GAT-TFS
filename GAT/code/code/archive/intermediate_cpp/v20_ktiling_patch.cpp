// ============================================================
// V20 K-Tiling Patch
// 基于 V17c，只修改 AMX kernel 部分
// 核心改动: K_tile=32 (4 passes × 32 cols) 替代 K_tile=64 (2 passes × 64 cols)
// ============================================================
//
// 使用方法:
//   1. cp amx_spmm_v17c_bf16fb.cpp amx_spmm_v20_ktiling.cpp
//   2. 在 v20 中做以下替换（搜索标记 → 替换为对应代码）
//   3. 编译: icpx -O3 -g -mamx-tile -mamx-bf16 -mavx512f -mavx512bf16 -mavx512bw -fopenmp -qmkl=sequential -o bin/amx_v20 amx_spmm_v20_ktiling.cpp
//
// ============================================================

// ============================================================
// 改动 1: 新增 gather2 函数（替代 gather4）
// 位置: 在 gather4 函数之后新增
// ============================================================
// gather2: 每 B 行只读 32 BF16 (64 bytes)，拆成 2 个 VNNI-packed buffer
// 对比 gather4: 读 64 BF16 (128 bytes) 拆成 4 个 buffer
// 
// 参数说明:
//   B: BF16 格式的稠密矩阵，行主序，每行 K 个 BF16 元素
//   br: 当前 tile 的 32 个 B 行索引（16 对 even/odd）
//       br[p*2] = even row index, br[p*2+1] = odd row index
//       -1 表示该位置无有效行（padding zero）
//   kb: K 维度的起始偏移（0, 32, 64, 96）
//   b0, b1: 输出 buffer，每个 1024 bytes (16 VNNI pairs × 64 bytes)
//       b0 覆盖 KC chunk 0 (列 kb+0  到 kb+15)
//       b1 覆盖 KC chunk 1 (列 kb+16 到 kb+31)
//
// VNNI 格式说明:
//   AMX TDPBF16PS 要求 B tile 中每个 32-bit word = [odd_bf16 << 16 | even_bf16]
//   即相邻两行的同一列的 BF16 值打包在一个 uint32 中
//   _mm512_cvtepu16_epi32: 将 16 个 uint16 零扩展为 16 个 uint32
//   _mm512_slli_epi32(x, 16): 左移 16 位，将 odd 行放到高 16 位
//   _mm512_or_si512: 合并 even(低16位) 和 odd(高16位)

static inline void gather2(const uint16_t*B,const int*br,int kb,
                           uint8_t*b0,uint8_t*b1){
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        uint32_t*p0=(uint32_t*)(b0+p*64);
        uint32_t*p1=(uint32_t*)(b1+p*64);
        if(re>=0&&ro>=0){
            // 两行都有效：读 32 BF16/行 = 64 bytes/行
            const uint16_t*be=&B[(int64_t)re*K+kb];
            const uint16_t*bo=&B[(int64_t)ro*K+kb];
            // _mm512_loadu_si512: 加载 512 bits = 64 bytes = 32 个 BF16
            __m512i f0=_mm512_loadu_si512(be);   // even row: 32 BF16
            __m512i g0=_mm512_loadu_si512(bo);   // odd row: 32 BF16
            __m512i ie,io;
            // 低 256 bits = 前 16 个 BF16 → KC chunk 0 (b0)
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(f0));
            io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(g0));
            _mm512_store_si512((__m512i*)p0,
                _mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            // 高 256 bits = 后 16 个 BF16 → KC chunk 1 (b1)
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f0,1));
            io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(g0,1));
            _mm512_store_si512((__m512i*)p1,
                _mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
        }else if(re>=0){
            // 只有 even 行，odd 行填零
            const uint16_t*be=&B[(int64_t)re*K+kb];
            __m512i f0=_mm512_loadu_si512(be);
            _mm512_store_si512((__m512i*)p0,
                _mm512_cvtepu16_epi32(_mm512_castsi512_si256(f0)));
            _mm512_store_si512((__m512i*)p1,
                _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(f0,1)));
        }else{
            // 两行都无效，全部填零
            __m512i z=_mm512_setzero_si512();
            _mm512_store_si512((__m512i*)p0,z);
            _mm512_store_si512((__m512i*)p1,z);
        }
    }
}

// ============================================================
// 改动 2: 新增 pf_tile2 函数（替代 pf_tile）
// 位置: 在 pf_tile 函数之后新增
// ============================================================
// pf_tile2: 每 B 行只预取 1 个 cache line (64 bytes = 32 BF16)
// 对比 pf_tile: 预取 2 个 cache line (128 bytes = 64 BF16)
//
// _mm_prefetch 说明:
//   将指定地址的 cache line (64 bytes) 提前加载到 cache
//   _MM_HINT_T1: 加载到 L2 cache（不进 L1，避免污染 L1）
//   这样当 gather2 真正读取时，数据已在 L2，延迟从 HBM ~150cy 降到 L2 ~12cy

static inline void pf_tile2(const uint16_t*B,const int*br,int kb){
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        // 32 BF16 = 64 bytes = 1 个 cache line，只需 1 次 prefetch
        if(re>=0)_mm_prefetch((const char*)&B[(int64_t)re*K+kb],_MM_HINT_T1);
        if(ro>=0)_mm_prefetch((const char*)&B[(int64_t)ro*K+kb],_MM_HINT_T1);
    }
}

// ============================================================
// 改动 3: 替换 amx_kern 函数
// 位置: 替换整个 amx_kern 函数（约第 146-175 行）
// ============================================================
// 核心改动:
//   - 2 passes × 64 cols → 4 passes × 32 cols
//   - gather4 → gather2
//   - 4 TDPBF16PS/tile → 2 TDPBF16PS/tile
//   - 4 C 累加器 (tiles 0,3,4,5) → 2 C 累加器 (tiles 0,3)
//   - C 写回: 64 列/pass → 32 列/pass

void amx_kern(const uint16_t*B,float*C,
              const std::vector<Panel>&panels,const uint16_t*A,int tr){
    int np=(int)panels.size();
    if(np==0)return;
    #pragma omp parallel
    {
        // 请求 AMX 权限
        // arch_prctl(ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA)
        // 每个线程都需要独立请求
        syscall(SYS_arch_prctl,0x1023,18);

        // Tile 配置
        // tile 0: C 累加器 0, tr 行 × KC×4 bytes (16 FP32)
        // tile 1: A tile, tr 行 × 64 bytes (32 BF16)
        // tile 2: B tile, 16 行 × KC×4 bytes (VNNI packed)
        // tile 3: C 累加器 1, tr 行 × KC×4 bytes (16 FP32)
        // tiles 4-7: 未使用（可用于未来的 double buffering）
        tile_config_t cfg;
        memset(&cfg,0,64);
        cfg.palette_id=1;
        cfg.rows[0]=tr; cfg.colsb[0]=KC*4;   // C0: tr × 16 FP32
        cfg.rows[1]=tr; cfg.colsb[1]=64;      // A:  tr × 32 BF16
        cfg.rows[2]=16; cfg.colsb[2]=KC*4;    // B:  16 VNNI pairs × 16 FP32
        cfg.rows[3]=tr; cfg.colsb[3]=KC*4;    // C1: tr × 16 FP32
        // tiles 4-7 不配置（rows=0 则禁用）
        _tile_loadconfig(&cfg);

        // 每线程的 gather buffer 和 C 缓冲区
        // alignas(64): 对齐到 64 字节边界（cache line 对齐，AMX 要求）
        alignas(64)uint8_t b0[1024],b1[1024];  // 只需 2 个 buffer（原来 4 个）
        alignas(64)float c0[16][KC],c1[16][KC]; // 只需 2 个 C 缓冲（原来 4 个）

        // Panel 并行：dynamic 调度让线程自动负载均衡
        // dynamic,8: 每次取 8 个 panel 执行，减少调度开销
        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){
            const Panel&P=panels[pi];
            const uint16_t*pA=A+P.A_offset;

            // ★ K-tiling: 4 passes × 32 列（原来 2 passes × 64 列）
            for(int pass=0;pass<4;pass++){
                int kb=pass*32;  // ★ 步长 32（原来 64）

                // 清零 C 累加器（只需 2 个）
                _tile_zero(0);
                _tile_zero(3);

                // 预取前 PF_DIST 个 tile 的 B 数据到 L2
                for(int pf=0;pf<PF_DIST&&pf<P.ntiles;pf++)
                    pf_tile2(B,&P.b_rows[pf*32],kb);  // ★ pf_tile2

                // 遍历当前 panel 的所有 tile
                for(int t=0;t<P.ntiles;t++){
                    // 预取未来 tile 的 B 数据
                    if(t+PF_DIST<P.ntiles)
                        pf_tile2(B,&P.b_rows[(t+PF_DIST)*32],kb);  // ★ pf_tile2

                    // ★ gather2: 只读 32 BF16/行（原来 gather4 读 64）
                    gather2(B,&P.b_rows[t*32],kb,b0,b1);

                    // 加载 A tile（同一个 panel 的 A tile 每个 pass 都要重新加载）
                    // 这是 K-tiling 的代价，但 A tile 已在 L1/L2 cache 中
                    _tile_loadd(1,pA+(size_t)t*tr*TILE_C,64);

                    // ★ 2 次 TDPBF16PS（原来 4 次）
                    // C[tr×16] += A[tr×32] × B[32×16]
                    _tile_loadd(2,b0,64);
                    _tile_dpbf16ps(0,1,2);  // C0 += A × B_chunk0
                    _tile_loadd(2,b1,64);
                    _tile_dpbf16ps(3,1,2);  // C1 += A × B_chunk1
                }

                // 从 tile 寄存器存到内存缓冲
                _tile_stored(0,c0,KC*4);
                _tile_stored(3,c1,KC*4);

                // 写回 C 矩阵（32 列/pass）
                for(int i=0;i<P.nrows;i++){
                    float*d=C+(int64_t)P.rows[i]*K+kb;
                    // ★ 只写 2 × KC = 32 列（原来 4 × KC = 64 列）
                    for(int k=0;k<KC;k++)d[k]=c0[i][k];
                    for(int k=0;k<KC;k++)d[k+KC]=c1[i][k];
                }
            }
        }
        _tile_release();
    }
}
