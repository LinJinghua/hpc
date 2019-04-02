#! /bin/bash
# Tested using bash version 4.1.5

srun bash -c "./schedule &> schedule.out& ./redis-server ./redis-conf.conf &> redis.out& wait" &> /dev/null &
sleep 1s
SCHE=$(cat /etc/hosts | grep $(squeue --format="%N" | sed '1d') | awk '{print $1}')  && echo ${SCHE} >  ./sehe.host && cat ./sehe.host
curl --noproxy '*' ${SCHE}:8080/up?ip=${SCHE}
