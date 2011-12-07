#!/bin/sh

#gt="exp/disparityMap.png"
for dir in "expB" "expC" "expD"
do
    echo $dir
    i=1
    gt="$dir/disparityMap.png"
    for dispMap in ${dir}/disparityMap*.png
    do
	./errorCount.out $gt $dispMap "$dir/error$i.png" 200 150 350 380
	let i=$i+1
	echo "\n"
    done
    echo "\n\n\n"

done
echo "analyze done\n"
