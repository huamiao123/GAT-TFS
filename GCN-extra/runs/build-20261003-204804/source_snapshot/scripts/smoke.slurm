#!/usr/bin/env bash
#SBATCH --job-name=gcn-extra-smoke
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=16G
#SBATCH --hint=nomultithread
#SBATCH --distribution=block:block
#SBATCH --time=00:10:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/smoke-%j.err
bash /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/scripts/job.sh smoke
