#!/bin/bash
#SBATCH --job-name=e2e_v3
#SBATCH --partition=intel
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=12:00:00
#SBATCH --output=logs/gcn_e2e_v3_all_%j.log

module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3 2>/dev/null || \
module load intel/intel-oneapi-mkl/2023.2.0/intel2021.10.0-mijbebf 2>/dev/null
cd ~/data/SpMM_project/code
export OMP_NUM_THREADS=32
export OMP_PROC_BIND=close
export OMP_PLACES=cores

echo "=== GCN E2E v3 — Hybrid Pipeline, ALL 25 ==="
echo "Node: $(hostname)"
echo "Date: $(date)"
echo ""

DDIR=~/data/SpMM_project/data
MATRICES="amazon0601 as-Skitter cage15 cit-Patents com-Friendster com-LiveJournal com-Youtube email-Enron FullChip hollywood-2009 indochina-2004 kron_g500-logn21 mycielskian19 ogbn-products rajat31 reddit rgg_n_2_24_s0 roadNet-CA road_usa scircuit soc-LiveJournal1 soc-Pokec sx-stackoverflow web-Google wiki-Talk"

for mat in $MATRICES; do
    echo "############################################"
    echo "# $mat"
    echo "############################################"
    echo ""
    timeout 1200 ./gcn_e2e_v3 $DDIR $mat
    RET=$?
    if [ $RET -eq 124 ]; then echo "  TIMEOUT"; elif [ $RET -ne 0 ]; then echo "  ERROR ($RET)"; fi
    echo ""
done

echo "=== Done: $(date) ==="
