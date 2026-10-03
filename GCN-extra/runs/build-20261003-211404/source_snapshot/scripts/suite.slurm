#!/usr/bin/env bash
#SBATCH --job-name=gcn-extra-25graphs
#SBATCH --partition=intel_expr
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --mem=448G
#SBATCH --hint=nomultithread
#SBATCH --distribution=block:block
#SBATCH --time=02:00:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/formal-%j.err
bash /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/scripts/job.sh formal
