# Preparatory build issue

The first `icpx` compile on 2026-10-18 exited with eight type errors because `std::vector` objects were passed to helpers expecting raw pointers. It generated no runnable binary and no timing. The source was corrected to pass `.data()`; the second `icpx -O3 -std=c++17 -xCORE-AVX512 -qopenmp -Wall -Wextra` compile succeeded with empty `compile_final.err`. The successful smoke run used only the corrected binary. This note preserves the failed preparation step, which did not become a cluster run.
