#!/bin/bash
#
GCC_OUTPUT=build/gcc.out

# Check to see if a pipe exists on stdin.
if [ -p /dev/stdin ]; then
	# If we want to read the input line by line
	while IFS= read line; do
		echo "${line}" >> $GCC_OUTPUT
	done
else
	exit 0
fi

while [ $# -gt 1 ]
do
	key="$1"
	case $key in
		-w)
		WORKING_FOLDER="$2"
		shift # past argument
		;;
		-l)
		LIBRARIES="$2"
		shift # past argument
		;;
		-lp)
		LIBRARIES_PATH="$2"
		shift # past argument
		;;
		-n)
		LIBRARY="$2"
		shift # past argument
		;;
		-np)
		LIBRARY_PATH="$2"
		shift # past argument
		;;
	esac

	shift
done

if [ ! -f "$GCC_OUTPUT" ];
then
	exit 0
fi

if [ ! -d "$WORKING_FOLDER" ];
then
	rm -f $GCC_OUTPUT
	exit 0
fi

if [ -z "$LIBRARIES" ] && [ -z "$LIBRARY" ] && [ -z "$LIBRARY_PATH" ];
then
	rm -f $GCC_OUTPUT
	exit 0
fi

for library in $LIBRARIES;
do
	pattern=$WORKING_FOLDER/objects/$library/
	replacement=$LIBRARIES_PATH/$library/
	sed -e 's@'"$pattern"'@'"$replacement"'@g' $GCC_OUTPUT > $GCC_OUTPUT.tmp
	mv $GCC_OUTPUT.tmp $GCC_OUTPUT
done

replacement=
pattern=$WORKING_FOLDER/objects/$LIBRARY

if [ ! -z "$LIBRARY" ];
then
	pattern=$WORKING_FOLDER/objects/$LIBRARY/

	if [ ! -z "$LIBRARY_PATH" ];
	then
		replacement=$LIBRARY_PATH/$LIBRARY/
	fi
fi

sed -e 's@'"$pattern"'@'"$replacement"'@g' $GCC_OUTPUT
echo 
rm -f $GCC_OUTPUT
