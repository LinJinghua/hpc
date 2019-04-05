#! /bin/bash
# Tested using bash version 4.1.5
# sbatch --nodes=100 --ntasks-per-node=1 --cpus-per-task=24 batch.sh

#SBATCH --nodes=100
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=24

srun bash -c "$(cat ./consumer.sh)"
