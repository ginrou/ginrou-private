#!/bin/sh
ls

for name in "A" "B" "C"
do
    echo exp$name
    for num in "1" "2" "3" "4"
    do
	./main.out exp$name/disparityMap.png exp$name/dispmap$num.png exp$name/errorMap$num.png exp$name/error$num.csv
    done
done


exit 0

