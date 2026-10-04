#!/bin/bash
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3 2>/dev/null || \
module load intel/intel-oneapi-mkl/2023.2.0/intel2021.10.0-mijbebf 2>/dev/null || \
echo "WARNING: Could not load MKL module, trying with -qmkl anyway"

cd ~/data/SpMM_project/code
echo "Compiling mkl_baseline.cpp ..."
icpx -O3 -march=sapphirerapids \
     -qopenmp -qmkl=parallel \
     -o mkl_baseline mkl_baseline.cpp
if [ $? -eq 0 ]; then
    echo "OK: mkl_baseline compiled"
    ls -lh mkl_baseline
else
    echo "FAIL"
    exit 1
fi
