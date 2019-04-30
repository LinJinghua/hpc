#! /bin/bash
# Tested using bash version 4.1.5

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=2

srun bash -c "ulimit -u unlimited && ./schedule &> schedule.out& ./redis-server ./redis-conf.conf &> redis.out& wait"
