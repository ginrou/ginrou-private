#!/bin/sh
PROCESS=("shift-blur.out" "stereoDepthEstimation.out" "DepthFromDefocus.out" "CodedAperturePair.out")

for dir in "exp" #"expB" "expC" 
do
    echo ${dir}
    i=1
    for batch in ${dir}/*.txt
    do
	echo ${batch}
	args=()
	sed -e "s/img/${dir}/g" ${batch} > ${batch}.tmp
	mv ${batch}.tmp ${batch}
	chmod u+r ${batch}
	while read line 
	do
	    args=("${args[@]}" "$line")
	done < ${batch}
	process=${PROCESS[${args[0]}]}
	unset args[0]
	imgs_dir="${dir}/tmpImages$i"
	if [ ! -d $imgs_dir ]; then
	    mkdir $imgs_dir
	fi
	# args[5]=-1.0
	# args[6]=0.0
	# args[7]=-1.0s
	# args[8]=0.0
	echo ${args[@]} "${dir}/disparityMap$i.png"
	./$process ${args[@]} $imgs_dir "${dir}/disparityMap$i.png"
	let i=${i}+1
   done
done
echo "experiments done\n"


