#!/usr/bin/env bash
#SBATCH --job-name=gcn_source_correct
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --mem=8G
#SBATCH --time=00:10:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-smoke-%j.err
bash /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/scripts/source_job.sh smoke
