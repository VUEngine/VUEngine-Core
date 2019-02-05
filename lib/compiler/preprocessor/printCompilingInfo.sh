#!/bin/bash
#
INPUT_FILE=$1

if [ -z "${INPUT_FILE##*assets/*}" ];
then
	echo $INPUT_FILE | sed -e 's#^.*assets/\(.*$\)#Compiling asset: \1#g'
	exit 0
fi

className=`grep -m 1 -e '__CLASS_DEFINITION([A-z][A-z0-9]*],' $INPUT_FILE | sed -e 's#__CLASS_DEFINITION(\([A-Z][A-z0-9]*\),.*#\1#'`

if [ -z "$className" ];
then
	# Maybe it is a static class
	className=`grep -o -m 1 -e '^.*[ 	][ 	]*[A-Z][A-z0-9]*[ 	]*_[ 	]*[a-z][A-z0-9]*[ 	]*(' $INPUT_FILE | sed -e 's/^.*[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*_.*/\1/'`
fi

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