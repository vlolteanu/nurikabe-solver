#!/bin/bash

mkdir -p result

for i in $(ls *.txt)
do
	echo $i
	$(which time) ./nurikabe $i > result/$i.res 2> result/$i.err
done
