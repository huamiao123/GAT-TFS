#!/bin/bash
#SBATCH --job-name=all_v3
#SBATCH --partition=intel
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=12:00:00
#SBATCH --output=logs/all_v3_%j.log

module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
cd ~/data/SpMM_project/code
export OMP_NUM_THREADS=32
export OMP_PROC_BIND=close
export OMP_PLACES=cores
DDIR=~/data/SpMM_project/data

echo "=== TFS V3 ALL 25 MATRICES (K=128, R=64, 32T) ==="
echo "Node: $(hostname)"
echo "Date: $(date)"
echo ""

MATRICES="amazon0601 as-Skitter cage15 cit-Patents com-Friendster com-LiveJournal com-Youtube email-Enron FullChip hollywood-2009 indochina-2004 kron_g500-logn21 mycielskian19 ogbn-products rajat31 reddit rgg_n_2_24_s0 roadNet-CA road_usa scircuit soc-LiveJournal1 soc-Pokec sx-stackoverflow web-Google wiki-Talk"

for mat in $MATRICES; do
    echo "============================================"
    echo "Matrix: $mat"
    echo "============================================"
    timeout 1200 ./amx_tfs_v3 $DDIR $mat 64
    RET=$?
    if [ $RET -eq 124 ]; then
        echo "  TIMEOUT (1200s)"
    elif [ $RET -ne 0 ]; then
        echo "  ERROR (exit=$RET)"
    fi
    echo ""
done

echo "=== Done: $(date) ==="
