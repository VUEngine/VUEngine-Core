#!/bin/bash
#
INPUT_FILE=$1

if [ -z "${INPUT_FILE##*assets/*}" ];
then
	echo $INPUT_FILE | sed -e 's#^.*assets/\(.*$\)#Compiling asset: \1#g'
	exit 0
fi

className=`grep -m 1 -e 'CLASS_IN_FILE([A-Z][A-z0-9]*)' $INPUT_FILE | sed -e 's#.*CLASS_IN_FILE(\([A-Z][A-z0-9]*\)).*#\1#'`

if [ -z "$className" ];
then
	if [ -z "${INPUT_FILE##*source*}" ];
	then
		echo $INPUT_FILE | sed -e 's#^.*source[s]*/\(.*$\)#Compiling file:  \1#g'
	else
		echo "Compiling file:  $INPUT_FILE"
	fi
	exit 0
fi

echo "Compiling class: $className"