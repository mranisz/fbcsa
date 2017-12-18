#!/bin/bash
make
for file in english200 sources200 xml200 dna200 proteins200; do
    for ss in 3 4 5 8 12 16 32; do
        for type in 32 64; do
            sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
            taskset -c 0 ./test/extractFBCSA $type $ss $file 10000000 1
        done
    done
done
for file in english200 sources200 xml200 dna200 proteins200; do
    for seqLen in 5 10 20; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32 64; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/extractFBCSA $type $ss $file 1000000 $seqLen
            done
        done
    done
done