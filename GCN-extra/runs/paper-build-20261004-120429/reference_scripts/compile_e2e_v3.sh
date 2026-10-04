#!/bin/bash
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3 2>/dev/null || \
module load intel/intel-oneapi-mkl/2023.2.0/intel2021.10.0-mijbebf 2>/dev/null
cd ~/data/SpMM_project/code
echo "Compiling gcn_e2e_v3 ..."
icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 \
     -qopenmp -qmkl=parallel -o gcn_e2e_v3 gcn_e2e_v3.cpp
if [ $? -eq 0 ]; then echo "OK"; ls -lh gcn_e2e_v3; else echo "FAIL"; exit 1; fi
