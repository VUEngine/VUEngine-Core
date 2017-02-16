#!/bin/bash

VUENGINE_HOME=$1
GAME_HOME=$2
OUTPUT_C_FILE=$3

VUENGINE_HEADER_FILES=`find $VUENGINE_HOME/source/ -name "*.h"`
GAME_HEADER_FILES=`find $GAME_HOME/source/  -name "*.h"`
SAVED_HEADERS_FILE=$GAME_HOME/lib/compiler/headerFiles.txt
TEMPORAL_HEADERS_FILE=temporalHeaderFiles.txt

CLASSES_FILE="classFile.txt"

echo $VUENGINE_HEADER_FILES > $TEMPORAL_HEADERS_FILE
echo $GAME_HEADER_FILES >> $TEMPORAL_HEADERS_FILE

HEADER_FILES=

# check for header files additions or deletions
if [ ! -f $SAVED_HEADERS_FILE ] || [ ! -f $OUTPUT_C_FILE ] ; then
    echo $VUENGINE_HEADER_FILES > $SAVED_HEADERS_FILE
    echo $GAME_HEADER_FILES >> $SAVED_HEADERS_FILE
	HEADER_FILES=`cat $SAVED_HEADERS_FILE`
else
	SAVED_HEADER_FILES=`cat $SAVED_HEADERS_FILE`
	TEMPORAL_HEADER_FILES=`cat $TEMPORAL_HEADERS_FILE`

	if ! [ "$SAVED_HEADER_FILES" == "$TEMPORAL_HEADER_FILES" ]; then
		echo "Files differ"
		HEADER_FILES=$TEMPORAL_HEADERS_FILE
		`cat $TEMPORAL_HEADERS_FILE > $SAVED_HEADERS_FILE`
		HEADER_FILES=`cat $SAVED_HEADERS_FILE`
    fi
fi

# if the header files list was populated, generate the setupClass.c files
if [ -n "$HEADER_FILES" ]; then

	rm -f $CLASSES_FILE
	rm -f $OUTPUT_C_FILE
	echo " " > $OUTPUT_C_FILE

	echo "// Do not modify this file, it is auto-generated" > $OUTPUT_C_FILE

	for header in $HEADER_FILES; do
		className=`grep "__CLASS(" $header`
		className=`echo $className | sed 's/__CLASS(//' | sed 's/);//'`
		if ! [[ "$className" =~ "define" ]]; then
			if [ -n "$className" ]; then
				echo $className >> $CLASSES_FILE
			fi
		fi
	done

	CLASSES_NAMES=`cat $CLASSES_FILE`

	echo " " >> $OUTPUT_C_FILE
	echo "// includes" >> $OUTPUT_C_FILE

	# Create the include directives
	for className in $CLASSES_NAMES; do
		echo "#include <"$className".h>" >> $OUTPUT_C_FILE
	done

	echo " " >> $OUTPUT_C_FILE
	echo "// setup function" >> $OUTPUT_C_FILE

	#create the function
	echo "void setupClasses(void)" >> $OUTPUT_C_FILE
	echo "{" >> $OUTPUT_C_FILE

	# Create the calls directives
	for className in $CLASSES_NAMES; do
		echo "	"$className"_setVTable();" >> $OUTPUT_C_FILE
	done
	echo "}" >> $OUTPUT_C_FILE
fi

rm -f $CLASSES_FILE
rm -f $TEMPORAL_HEADERS_FILE
