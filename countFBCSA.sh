#!/bin/bash
make
for file in english200 sources200 xml200; do
    for m in 8 16 32 64; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32 64 32-hyb 64-hyb 32-lut2 64-lut2; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss $file 500000 $m
            done
        done
    done
done
for file in dna200; do
    for m in 12 16 32 64; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32 64 32-hyb 64-hyb 32-lut2 64-lut2; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss $file 500000 $m
            done
        done
    done
done
for file in proteins200; do
    for m in 5 16 32 64; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32 64 32-hyb 64-hyb 32-lut2 64-lut2; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss $file 500000 $m
            done
        done
    done
done
for file in english200 sources200 xml200; do
    for m in 8 16 32 64; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32-hash 64-hash 32-hash-dense 64-hash-dense; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss 8 0.9 $file 500000 $m
            done
        done
    done
done
for file in dna200; do
    for m in 12 16 32 64; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32-hash 64-hash 32-hash-dense 64-hash-dense; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss 12 0.9 $file 500000 $m
            done
        done
    done
done
for file in proteins200; do
    for m in 5 16 32 64; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32-hash 64-hash 32-hash-dense 64-hash-dense; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss 5 0.9 $file 500000 $m
            done
        done
    done
done
for file in english50 sources50 xml50 dna50 proteins50; do
    for m in 16; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32 32-lut2; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss $file 500000 $m
            done
        done
    done
done
for file in english50 sources50 xml50; do
    for m in 16; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32-hash 32-hash-dense; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss 8 0.9 $file 500000 $m
            done
        done
    done
done
for file in dna50; do
    for m in 16; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32-hash 32-hash-dense; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss 12 0.9 $file 500000 $m
            done
        done
    done
done
for file in proteins50; do
    for m in 16; do
        for ss in 3 4 5 8 12 16 32; do
            for type in 32-hash 32-hash-dense; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFBCSA $type $ss 5 0.9 $file 500000 $m
            done
        done
    done
done