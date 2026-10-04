#!/bin/bash
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
cd ~/data/SpMM_project/code
echo "Compiling amx_tfs_v3.cpp ..."
icpx -O3 -march=sapphirerapids \
     -mamx-bf16 -mamx-tile -mavx512bf16 \
     -qopenmp \
     -o amx_tfs_v3 amx_tfs_v3.cpp
if [ $? -eq 0 ]; then
    echo "OK: amx_tfs_v3 compiled"
    ls -lh amx_tfs_v3
else
    echo "FAIL: compilation error"
    exit 1
fi
