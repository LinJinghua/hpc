#! /bin/bash
# Tested using bash version 4.1.5
SCHE=$(cat ~/jobs/sehe.host)
date > date-start.txt
for i in $(seq "1" "$1"); 
do
    seq=$(printf "%05d" ${i})
    out=consumer-${seq}.out
    err=consumer-${seq}.err
    # echo ${seq}
    srun -n 1 ~/code/hpc/build/consumer ${SCHE}:8080 zinc_datazinc_ligand_1w_sort > "${out}" 2> "${err}" &
done


# cd ~/jobs
# srun nohup ~/bin/bin/schedule > schedule.out &
# SCHE=`squeue --format="%N" | sed '1d'` && echo ${SCHE} >  ~/jobs/sehe.host && cat ~/jobs/sehe.host
# srun bash -c "curl --noproxy '*' ${SCHE}:8080/up 2> /dev/null && ./redis-server ./redis-conf.conf > redis.out" &
# # ~/code/hpc/build/producer `curl --noproxy '*' ${SCHE}:8080/get` 6379 "mongodb://12.11.70.12:10101"
# ./run.sh 20


# SCHE=`cat ~/jobs/sehe.host`
# curl --noproxy '*' ${SCHE}:8080/get
# srun ~/code/hpc/build/producer `curl --noproxy '*' ${SCHE}:8080/get` 6379 "mongodb://12.11.70.12:10101"
# redis-cli -h `curl --noproxy '*' ${SCHE}:8080/get`
# scancel `squeue --format='%A' | sed '1d'`
# cat consumer-000{01..20}.err | grep molecule | wc -l
