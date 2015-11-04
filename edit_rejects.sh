#!/bin/bash

rejects=$(find . -name "*.rej")

for reject in $rejects; do
	echo -e "* Found reject $reject"
	# extract file name
	#filename=$(echo "$reject" | cut -d'.' -f1-3)
	filename=${reject%.*}
	echo -e "* Opening $filename to fix rejects"
	geany $filename $reject
done
