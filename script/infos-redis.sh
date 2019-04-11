#! /bin/bash
# Tested using bash version 4.1.5

set -m

SCHE=$(cat ./sehe.host)

COMMAND_OUT=infos-redis-commandstats.log
TOTAL_OUT=infos-redis-info.log

function info_redis() {
    while true
    do
        date '+date: %s %Y-%m-%d %H:%M:%S' >> ${COMMAND_OUT}
        ./redis-cli -h $SCHE info commandstats >> ${COMMAND_OUT}
        date '+date: %s %Y-%m-%d %H:%M:%S' >> ${TOTAL_OUT}
        ./redis-cli -h $SCHE info >> ${TOTAL_OUT}
        sleep 60s&
        wait
    done
}

info_redis
