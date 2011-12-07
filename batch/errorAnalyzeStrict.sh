#!/bin/sh

#gt="exp/disparityMap.png"
for dir in "expB" #"expB" "expC"
do
    echo $dir
    i=1
    gt="$dir/disparityMap.png"
    for dispMap in ${dir}/disparityMap*.png
    do
	./errorCountStrict.out $gt $dispMap "$dir/error$i.png" 100 150 420 400
	let i=$i+1
    done
    echo "\n\n\n"

done
echo "analyze done\n"
