#!/bin/bash

mkdir -p result

for i in $(ls puzzle/*.puzzle)
do
	echo $i
	name=$(echo $i | cut -d '/' -f 2 | cut -d '.' -f 1)
	$(which time) ./nurikabe $i > result/$name.res 2> result/$name.err
done
