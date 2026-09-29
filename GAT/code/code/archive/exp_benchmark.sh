#!/bin/bash
# Full benchmark: V17c + MKL, 7 matrices x K=32,64,128

DATA=~/data/SpMM_project/data
RESULTS=~/data/SpMM_project/results/benchmark
mkdir -p $RESULTS

# Detect binaries
SPMM=""
MKL=""
for p in ~/data/SpMM_project/code/amx_spmm \
         ~/data/SpMM_project/code/spmm_amx \
         ~/data/SpMM_project/code/v17c_spmm \
         ~/data/SpMM_project/code/JitSpMM/build/spmm; do
    if [ -x "$p" ]; then
        echo "Found SpMM binary: $p"
        SPMM="$p"
        break
    fi
done

for p in ~/data/SpMM_project/scripts/mkl_spmm_perf \
         ~/data/SpMM_project/code/mkl_spmm_perf \
         ~/data/SpMM_project/code/mkl_bench; do
    if [ -x "$p" ]; then
        echo "Found MKL binary: $p"
        MKL="$p"
        break
    fi
done

# List all executables to help locate
echo ""
echo "=== All executables in code/ ==="
find ~/data/SpMM_project/code/ -maxdepth 3 -type f -executable 2>/dev/null | head -30
echo ""
echo "=== All executables in scripts/ ==="
find ~/data/SpMM_project/scripts/ -maxdepth 2 -type f -executable 2>/dev/null | head -20

MATRICES="hollywood-2009 web-Google amazon0601 soc-Pokec kron_g500-logn21 rajat31 wiki-Talk"
KS="32 64 128"

export OMP_NUM_THREADS=64
export OMP_PROC_BIND=true
export OMP_WAIT_POLICY=ACTIVE

echo ""
echo "=========================================="
echo "  Benchmark Start: $(date)"
echo "  OMP_NUM_THREADS=$OMP_NUM_THREADS"
echo "=========================================="

for mat in $MATRICES; do
    csrbin="$DATA/$mat/$mat.csrbin"
    mtx="$DATA/$mat/$mat.mtx"
    if [ ! -f "$csrbin" ] && [ ! -f "$mtx" ]; then
        echo "[$mat] SKIP (no data file)"
        continue
    fi
    
    echo ""
    echo "######################################################################"
    echo "# $mat"
    echo "######################################################################"
    
    for k in $KS; do
        echo "  --- K=$k ---"
        
        # V17c / custom SpMM
        if [ -n "$SPMM" ]; then
            echo "  [V17c] Running..."
            # Try common argument patterns
            if [ -f "$csrbin" ]; then
                $SPMM "$csrbin" $k 2>&1 | tail -5
            elif [ -f "$mtx" ]; then
                $SPMM "$mtx" $k 2>&1 | tail -5
            fi
        else
            echo "  [V17c] Binary not found, skipping"
        fi
        
        # MKL
        if [ -n "$MKL" ]; then
            echo "  [MKL] Running..."
            if [ -f "$csrbin" ]; then
                $MKL "$csrbin" $k 2>&1 | tail -5
            elif [ -f "$mtx" ]; then
                $MKL "$mtx" $k 2>&1 | tail -5
            fi
        else
            echo "  [MKL] Binary not found, skipping"
        fi
        echo ""
    done
done

echo "=========================================="
echo "  Benchmark Done: $(date)"
echo "=========================================="
