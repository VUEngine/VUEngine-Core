#!/bin/bash
files=`find . -name "*.wav"`
VUENGINE_HOME="/Users/jorgeche/Documents/work/workspaces/ves/vuengine/core/lib/utilities"
amplitude=$1
output=".h"

if [ -z "$1" ]; then
	amplitude=15
fi

levels=$(( amplitude / 15 ))

rm *.c
channels=""

echo $levels

for (( i = 1; i <= $levels; i++ )) do

	channels="\&PCMSoundChannel, $channels"
done

echo channels $channels

for file in $files; do

	vbwavOSX $file $amplitude

	file=`echo $file | sed -e "s@./@@g"`
	echo Processing $file
	name=`echo $file | sed -e "s@\.wav@@g"`
#	echo $name
	finalDestination=`echo $file | sed -e "s@\.wav@SoundTrack@g" | sed -e "s@sample@@g"`
#	echo $finalDestination
	mv $output $finalDestination.c
	sed -i -e 's@sample_@'$finalDestination'@g' $finalDestination.c
	sed -i -e "s@static@@g" $finalDestination.c
	length=`grep _LEN $finalDestination.c | sed -E "s@.*_LEN@@g"`
	echo $length
	rm *-e
done
