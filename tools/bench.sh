#!/bin/sh

export BMB_SIZE=100
export BMB_COUNT=100000

if [ $# -gt 0 ]; then
	SUBJECTS=($@)
else
	SUBJECTS=($(find subjects/ -type f -executable))
fi

ID=$(tools/machid)

full_run()
{
	no_done=0
	cat /dev/null > $ID.d$1
	for subject in ${SUBJECTS[@]}; do
		if [ ! -f "$subject" ]; then
			echo "'$subject' doesn't exist; Did you forget to build the benchmark?"
			continue
		fi
		./"$subject" $1 >> $ID.d$1
		((++no_done))
		percent=$(echo "$no_done*100/${#SUBJECTS[@]}" | bc)
		echo "$percent%"
	done
}

echo ":: measuring latency ..."
full_run l
echo ":: measuring throughput ..."
full_run t

# FIXME Right now, this script might completely break when handling
# file paths containing spaces or other special characters.

