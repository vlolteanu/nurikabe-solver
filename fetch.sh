#!/bin/bash

puzzle=$(curl -s http://www.logicgamesonline.com/nurikabe/archive.php?pid=$1 | grep "var puzzle =" | cut -d '"' -f 2)

rm $1.txt
for i in $(seq 0 8)
do
	off=$((9 * i))
	echo ${puzzle:$off:8} >> $1.txt
done

