#!/bin/bash
#
INPUT_FILE=$1

message="Compiling file:  $INPUT_FILE..."

if [ -z "${INPUT_FILE##*assets/*}" ];
then
	message=`sed -e 's#^.*assets/\(.*$\)#Compiling asset: \1...#g' <<< $INPUT_FILE`
fi

if [ -z "${INPUT_FILE##*source*}" ];
then
	message=`sed -e 's#^.*source[s]*/\(.*$\)#Compiling file:  \1...#g' <<< $INPUT_FILE`
else
	if [ -z "${INPUT_FILE##*object*}" ];
	then
		message=`sed -e 's#^.*object[s]*/\(.*$\)#Compiling file:  \1...#g' <<< $INPUT_FILE`
	fi
fi

echo "$message"