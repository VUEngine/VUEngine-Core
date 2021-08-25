#!/bin/bash
#

while [ $# -gt 1 ]
do
	key="$1"
	case $key in
		-o)
		OUTPUT_FILE="$2"
		shift # past argument
		;;
		-w)
		WORKING_FOLDER="$2"
		shift # past argument
		;;
		-p)
		PLUGINS="$2"
		shift # past argument
		;;
		-l)
		PLUGINS_PATH="$2"
		shift # past argument
		;;
		-n)
		NAME="$2"
		shift # past argument
		;;
		-h)
		NAME_HOME="$2"
		shift # past argument
		;;
	esac

	shift
done

GCC_OUTPUT=$OUTPUT_FILE.out

# Check to see if a pipe exists on stdin.
if [ -p /dev/stdin ]; then
	# If we want to read the input line by line
	while IFS= read line; do
		echo "${line}" >> $GCC_OUTPUT
	done
else
	exit 0
fi

if [ ! -f "$GCC_OUTPUT" ];
then
	exit 0
fi

if [ ! -d "$WORKING_FOLDER" ];
then
	rm -f $GCC_OUTPUT
	exit 0
fi

if [ -z "$PLUGINS" ] && [ -z "$NAME" ] && [ -z "$NAME_HOME" ];
then
	rm -f $GCC_OUTPUT
	exit 0
fi

for plugin in $PLUGINS;
do
	pattern=^.*build/[a-z][a-z]*/objects/$objects/
	replacement=$PLUGINS_PATH/$plugin/
	sed -e 's@'"$pattern"'@'"$replacement"'@g' $GCC_OUTPUT > $GCC_OUTPUT.tmp
	mv $GCC_OUTPUT.tmp $GCC_OUTPUT
done

replacement=$NAME_HOME
pattern=^.*build/[a-z][a-z]*/objects/$NAME

sed -e 's@'"$pattern"'@'"$replacement"'@g' $GCC_OUTPUT
echo 
rm -f $GCC_OUTPUT
