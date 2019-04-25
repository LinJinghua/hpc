#! /bin/bash
# Tested using bash version 4.1.5

set -xm

out_dir=sd

function run_obabel() {
    for j in `cat $1`; do
        /dev/shm/software/obabel -imol "../data/$j" -osd "$out_dir/$j.sd";
    done
}

mkdir -p $out_dir
for i in `ls task/`; do
   run_obabel "task/$i" &> out-$i.out&
done
