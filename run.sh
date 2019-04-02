#! /bin/bash
# Tested using bash version 4.1.5
SCHE=$(cat sehe.host)
date > date-start.txt
run_dir=/dev/shm
for i in $(seq "1" "$1");
do
    seq=$(printf "%05d" ${i})
    # echo ${seq}
    out=${run_dir}/consumer-${seq}.\$\{i\}.out
    # err=consumer-${seq}.\$\{i\}.err
    consumer_str="${run_dir}/software/consumer ${SCHE}:8080 zinc_datazinc_ligand_1w_sort &> ${out}"
    pre_str="cp -rp ./software/ ${run_dir}"
    loop_str='; for i in $(seq 1 $(grep -c ^processor /proc/cpuinfo)); do '
    log_str="find ${run_dir} -name 'consumer-*.out' -exec mv {} ./ \;"
    end_str="& done; wait; rm -rf /dev/shm/software/; "${log_str}
    command_str=${pre_str}${loop_str}${consumer_str}${end_str}
    srun -n 1 bash -c "${command_str}" &
done


# cd ~/jobs
# srun nohup ~/bin/bin/schedule > schedule.out &
# SCHE=`squeue --format="%N" | sed '1d'` && echo ${SCHE} >  ~/jobs/sehe.host && cat ~/jobs/sehe.host
# srun bash -c "curl --noproxy '*' ${SCHE}:8080/up 2> /dev/null && ./redis-server ./redis-conf.conf > redis.out" &
# # ~/code/hpc/build/pmongo `curl --noproxy '*' ${SCHE}:8080/get` 6379 "mongodb://12.11.70.12:10101"
# ./run.sh 20

# srun bash -c "~/bin/bin/schedule &> schedule.out& ./redis-server ./redis-conf.conf &> redis.out& wait" &> /dev/null &
# SCHE=$(cat /etc/hosts | grep $(squeue --format="%N" | sed '1d') | awk '{print $1}')  && echo ${SCHE} >  ~/jobs/sehe.host && cat ~/jobs/sehe.host
# curl --noproxy '*' ${SCHE}:8080/up?ip=${SCHE}
# ./run.sh 20

# SCHE=`cat ~/jobs/sehe.host`
# curl --noproxy '*' ${SCHE}:8080/get
# srun ~/code/hpc/build/pmongo `curl --noproxy '*' ${SCHE}:8080/get` 6379 "mongodb://12.11.70.12:10101"
# redis-cli -h `curl --noproxy '*' ${SCHE}:8080/get`
# scancel `squeue --format='%A' | sed '1d'`
# cat consumer-000{01..20}.err | grep molecule | wc -l
