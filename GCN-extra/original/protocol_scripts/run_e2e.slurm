#!/bin/bash
#SBATCH --job-name=gcn_e2e
#SBATCH --partition=intel
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=02:00:00
#SBATCH --output=logs/gcn_e2e_%j.log

module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3 2>/dev/null || \
module load intel/intel-oneapi-mkl/2023.2.0/intel2021.10.0-mijbebf 2>/dev/null
cd ~/data/SpMM_project/code
export OMP_NUM_THREADS=32
export OMP_PROC_BIND=close
export OMP_PLACES=cores

DDIR=~/data/SpMM_project/data

echo "=== GCN E2E Benchmark (2-layer, K=128) ==="
echo "Node: $(hostname)"
echo "Date: $(date)"
echo ""

./gcn_e2e $DDIR ALL

echo ""
echo "=== Done: $(date) ==="
