#!/bin/bash

rm /tmp/barnes_hut.csv

for n in 8x_8y 7x_7y 6x_6y 5x_5y 4x_4y 3x_3y 2x_2y;
do
#    forces_bh__0x_0y_id__8192_nbodies__8x_8y_pods
    find forces_bh__*_nbodies__$n\_pods.deploy -name manycore_stats.log |  xargs grep ^kernel >> /tmp/barnes_hut.csv
    # Parse x/y pod id and arch size
    sed -i 's/\([0-9]\)x_\([0-9]\)y/\1,\2/g' /tmp/barnes_hut.csv
    sed -i 's/_bh__/,/g' /tmp/barnes_hut.csv
    sed -i 's/_pods\.deploy.*:/,/g' /tmp/barnes_hut.csv
    sed -i 's/_[a-z\-]*__/,/g' /tmp/barnes_hut.csv
    cat /tmp/barnes_hut.csv | tr -s ' ' ',' > /tmp/barnes_hut.csv.tmp
    mv /tmp/barnes_hut.csv.tmp /tmp/barnes_hut.csv
done
echo "function,podx,pody,nbodies,dimx,dimy,Tag ID,Aggr Instructions,Aggr I$ Misses,Aggr Stall Cycles,Aggr Bubble Cycles, Aggr Total Cycles,Abs Total Cycles,IPC,FLOPS/Cycle,% of Kernel Cycles" > barnes_hut.csv
mv /tmp/barnes_hut.csv >> barnes_hut.csv
python3 parse.py barnes_hut.csv


