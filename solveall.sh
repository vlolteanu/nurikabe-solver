#!/bin/bash

for i in $(ls *.txt)
do
	echo $i
	$(which time) ./nurikabe $i &> i.solved
done
