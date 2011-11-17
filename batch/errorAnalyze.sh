#!/bin/sh


gt="disparityMap.png"
for dir in "expA" "expB" "expC"
do
    echo $dir
    i=1
    for dispMap in ${dir}/disparityMap*.png
    do
	./errorCount.out $gt $dispMap "$dir/error$i.png" 130 200 400 350
	let i=$i+1
    done
    echo "\n\n\n"

done
echo "analyze done\n"
