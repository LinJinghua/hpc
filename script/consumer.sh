#! /bin/bash
# Tested using bash version 4.1.5

set -xem

function wait_job() {
    FAIL=0
    for job in $(jobs -p); do
        wait $job || let "FAIL+=1"
    done
    [ "$FAIL" == "0" ] || echo "FAIL: ($FAIL)"
}

function term_job() {
    echo "Capture SIGTERM"
    # for job in $(jobs -p); do
        # kill -15 $job
    # done
}

trap term_job SIGINT
trap term_job SIGTERM

SCHE=$(cat ./sehe.host)
run_dir=/dev/shm
host_name=$(hostname)
log_file=consumer-${host_name}.out

# source /WORK/app/osenv/ln1/set3.sh
tar -xzf software.tar.gz -C ${run_dir}
# cp -rpf ./software/ ${run_dir}

process_num=$(grep -c ^processor /proc/cpuinfo)
for i in $(seq 1 ${process_num}); do
    ii=$(printf "%02d" ${i})
    out=${run_dir}/consumer-${host_name}.${ii}.out
    ${run_dir}/software/consumer ${SCHE}:8080 zinc_datazinc_ligand_1w_sort &> ${out} &
done

wait_job

# find ${run_dir} -maxdepth 1 -name 'consumer*.out' -exec mv {} ./ \;
for (( i=1; i<=${process_num}; ++i )); do
    ii=$(printf "%02d" ${i})
    out=${run_dir}/consumer-${host_name}.${ii}.out
    echo ${out} >> ${log_file}
    cat ${out} >> ${log_file}
    rm -f ${out}&
done
mv ${log_file} ./
rm -rf ${run_dir}/software/
