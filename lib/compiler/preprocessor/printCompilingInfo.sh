#!/bin/bash
#
INPUT_FILE=$1

message="Compiling file:  $INPUT_FILE..."

if [ -z "${INPUT_FILE##*assets/*}" ];
then
	message=`sed -e 's#^.*assets/\(.*$\)#Compiling asset: \1...#g' <<< $INPUT_FILE`
fi

className=`grep -m 1 -e '^.*::[ 	]*constructor[ 	]*(' $INPUT_FILE | sed -e 's#^.*[ 	][ 	]*\([A-Z][A-z0-9]*\)::.*#\1#'`

if [ -z "$className" ];
then
	className=`grep -o -m 1 -e '^.*[ 	][ 	]*[A-Z][A-z0-9\*()]*[ 	][ 	]*[A-Z][A-z0-9]*[ 	]*::[ 	]*[a-z][A-z0-9]*[ 	]*(' $INPUT_FILE | sed -e 's/^.*[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*::.*/\1/'`
fi

if [ -z "$className" ];
then
	if [ -z "${INPUT_FILE##*source*}" ];
	then
		message=`sed -e 's#^.*source[s]*/\(.*$\)#Compiling file:  \1...#g' <<< $INPUT_FILE`
	else
		if [ -z "${INPUT_FILE##*object*}" ];
		then
			message=`sed -e 's#^.*object[s]*/\(.*$\)#Compiling file:  \1...#g' <<< $INPUT_FILE`
		fi
	fi
else
	message="Compiling class: $className..."
fi

echo -n "$message"