#!/bin/bash
echo "" > bla
array=(10 25 50 75 100 250 500 1000 5000)
for i in `seq 1 10`:
do
	for j in `seq 1 25`:
	do
		echo ${array[i]}
		./a.out 0.01 ${array[i]} >> bla
		sleep 1
	done
done
	

