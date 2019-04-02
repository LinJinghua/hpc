#! /bin/bash
# Tested using bash version 4.1.5

#SBATCH --nodes=100
#SBATCH --ntasks-per-node=1

SCHE=$(cat ./sehe.host)
run_dir=/dev/shm
out=${run_dir}/consumer-'`hostname`'.\$\{i\}.out
consumer_str="${run_dir}/software/consumer ${SCHE}:8080 zinc_datazinc_ligand_1w_sort &> ${out}"
pre_str="cp -rp ./software/ ${run_dir}"
loop_str='; for i in $(seq 1 $(grep -c ^processor /proc/cpuinfo)); do '
log_str="find ${run_dir} -maxdepth 1 -name 'consumer*.out' -exec mv {} ./ \;"
end_str="& done; wait; rm -rf /dev/shm/software/; "${log_str}
command_str=${pre_str}${loop_str}${consumer_str}${end_str}
srun bash -c "${command_str}"
