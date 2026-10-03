// Generated from unchanged Original TFS; only C memset removed.
static void tfs_v3_nozero(
    const uint32_t *indptr,
    const uint32_t *indices,
    const uint16_t *Hb,
    const uint16_t *Wv,
    float          *C,
    const int      *perm,
    int N, int R)
{
    // Redundant full-C memset removed; every row/panel is stored below.

    #pragma omp parallel
    {
        if (syscall(SYS_arch_prctl, 0x1023, 18) != 0) {
            perror("arch_prctl"); exit(1);
        }
        tilecfg_t cfg;
        setup_tilecfg(&cfg);
        _tile_loadconfig(&cfg);

        uint16_t Hbuf[TR * K_IN]  __attribute__((aligned(64)));
        float    Ctmp[TR * 16]    __attribute__((aligned(64)));
        uint32_t base_local[TR];
        uint32_t deg_local[TR];
        int      orig_row[TR];

        #pragma omp for schedule(dynamic, 1) nowait
        for (int rg = 0; rg < N; rg += R) {
            int rg_end = (rg + R < N) ? (rg + R) : N;

            for (int i = rg; i < rg_end; i += TR) {
                int batch = ((i + TR) <= rg_end) ? TR : (rg_end - i);

                int max_deg = 0;
                for (int n = 0; n < batch; n++) {
                    int row = perm[i + n];
                    orig_row[n]   = row;
                    base_local[n] = indptr[row];
                    deg_local[n]  = indptr[row + 1] - indptr[row];
                    if ((int)deg_local[n] > max_deg)
                        max_deg = (int)deg_local[n];
                }

                for (int obp = 0; obp < NP; obp++) {
                    _tile_zero(TC0);
                    _tile_zero(TC1);
                    _tile_zero(TC2);
                    _tile_zero(TC3);

                    memset(Hbuf, 0, TR * K_IN * sizeof(uint16_t));
                    int active_from = 0;

                    for (int s = 0; s < max_deg; s++) {

                        /* Prefetch next step's H data into L1 */
                        if (s + 1 < max_deg) {
                            for (int n = active_from; n < batch; n++) {
                                if ((uint32_t)(s + 1) < deg_local[n]) {
                                    uint32_t j_next = indices[base_local[n] + s + 1];
                                    const char *addr =
                                        (const char*)&Hb[(size_t)j_next * K_IN];
                                    _mm_prefetch(addr,       _MM_HINT_T0);
                                    _mm_prefetch(addr + 64,  _MM_HINT_T0);
                                    _mm_prefetch(addr + 128, _MM_HINT_T0);
                                    _mm_prefetch(addr + 192, _MM_HINT_T0);
                                }
                            }
                        }

                        /* Zero rows that just became inactive */
                        while (active_from < batch &&
                               (uint32_t)s >= deg_local[active_from]) {
                            memset(&Hbuf[active_from * K_IN], 0,
                                   K_IN * sizeof(uint16_t));
                            active_from++;
                        }

                        /* Early termination */
                        if (active_from >= batch) break;

                        /* Gather active rows only */
                        for (int n = active_from; n < batch; n++) {
                            uint32_t j = indices[base_local[n] + s];
                            memcpy(&Hbuf[n * K_IN],
                                   &Hb[(size_t)j * K_IN],
                                   K_IN * sizeof(uint16_t));
                        }

                        /* KB-inner loop */
                        for (int kb = 0; kb < KB; kb++) {
                            _tile_loadd(TA,
                                        (const uint8_t*)Hbuf + kb * 64,
                                        K_IN * 2);

                            int ob0 = obp * 4;
                            const uint16_t *Wb = Wv;

                            #define WV_OFF(kb_, ob_) \
                                (((kb_) * NB + (ob_)) * 16 * 32)

                            _tile_loadd(TB0, &Wb[WV_OFF(kb, ob0 + 0)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb, ob0 + 1)], 64);
                            _tile_dpbf16ps(TC0, TA, TB0);
                            _tile_dpbf16ps(TC1, TA, TB1);

                            _tile_loadd(TB0, &Wb[WV_OFF(kb, ob0 + 2)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb, ob0 + 3)], 64);
                            _tile_dpbf16ps(TC2, TA, TB0);
                            _tile_dpbf16ps(TC3, TA, TB1);

                            #undef WV_OFF
                        }
                    }

                    /* Store C tiles to ORIGINAL row positions */
                    int col = obp * 64;

                    _tile_stored(TC0, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 0],
                               &Ctmp[n * 16], 64);

                    _tile_stored(TC1, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 16],
                               &Ctmp[n * 16], 64);

                    _tile_stored(TC2, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 32],
                               &Ctmp[n * 16], 64);

                    _tile_stored(TC3, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 48],
                               &Ctmp[n * 16], 64);
                }
            }
        }
        _tile_release();
    }
}

