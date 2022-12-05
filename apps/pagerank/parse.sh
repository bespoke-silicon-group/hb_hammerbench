#!/bin/bash

for n in wiki-Vote offshore ljournal-2008 hollywood-2009 road_usa roadNet-CA road_central;
do
    echo $n
    find direction_pull__fn_pagerank_pull_u8__graph_"$n"__dist_* -name manycore_stats.log |  xargs grep ^kernel > /tmp/$n.csv
    sed -i 's/^direction_//' /tmp/$n.csv
    sed -i 's/__[a-z\-]*_/,/g' /tmp/$n.csv
    sed -i 's/\/.*:/,/g' /tmp/$n.csv
    cat /tmp/$n.csv | tr -s ' ' ',' > /tmp/$n.csv.tmp
    mv /tmp/$n.csv.tmp /tmp/$n.csv
    echo "direction,function,graph,distribution,pod,npods,Tag ID,Aggr Instructions,Aggr I$ Misses,Aggr Stall Cycles,Aggr Bubble Cycles, Aggr Total Cycles,Abs Total Cycles,IPC,FLOPS/Cycle,% of Kernel Cycles" > $n.csv
    cat /tmp/$n.csv >> $n.csv
    head $n.csv
    python3 parse.py $n.csv
done

