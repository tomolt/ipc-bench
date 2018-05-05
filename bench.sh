#!/usr/bin/bash

METHODS=$@

if [ $# -eq 0 ]; then
	METHODS=(*_lat *_thr)
fi

ID=$(./machid)

LAT=""
cat /dev/null > $ID.l
for i in "${!METHODS[@]}"; do
	method="${METHODS[$i]}"
	if [[ "$method" != *_lat ]]; then
		continue
	fi
	if [ ! -f "$method" ]; then
		echo "'$method' doesn't exist; Did you forget to build the benchmark?"
		continue
	fi
	./"$method" 100 10000 >> $ID.l
	percent=$(echo "scale=2; ($i+1)/${#METHODS[@]}*100" | bc)
	echo "$percent%"
done

if false; then
THR=THR_$ID.dat
cp /dev/null $THR
./pipe_thr 100 10000 >> $THR
./unix_thr 100 10000 >> $THR
./tcp_thr 100 10000 >> $THR
fi

