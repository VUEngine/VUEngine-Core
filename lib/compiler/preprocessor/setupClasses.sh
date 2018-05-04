#!/bin/bash

GAME_HOME=
OUTPUT_C_FILE=setupClasses.c
HEADER_FOLDERS=

while [[ $# -gt 1 ]]
do
	key="$1"
	case $key in
		-h|-output)
		HEADER_FOLDERS="$HEADER_FOLDERS $2"
		shift # past argument
		;;
		-g|-output)
		GAME_HOME="$2"
		HEADER_FOLDERS="$HEADER_FOLDERS $2"
		shift # past argument
		;;
		-o|-output)
		OUTPUT_C_FILE="$2"
		shift # past argument
		;;
		*)
		HEADER_FOLDERS="$HEADER_FOLDERS $1"
		;;
	esac

	shift
done


HEADER_FILES=

echo Preprocessing classes in:

for headerFolder in $HEADER_FOLDERS; do

	echo "	"$headerFolder | sed 's/.//2' | sed 's/./\:\//3' | sed 's/./\u&/2'
	HEADER_FILES="$HEADER_FILES "`find $headerFolder/source/ -name "*.h"`
done

WORKING_FOLDER=$GAME_HOME/lib/compiler/preprocessor

SAVED_HEADERS_FILE=$WORKING_FOLDER/headerFiles.txt
TEMPORAL_HEADERS_FILE=temporalHeaderFiles.txt

CLASSES_FILE="classFile.txt"

echo $HEADER_FILES > $TEMPORAL_HEADERS_FILE

# check if necessary files already exist
if [ ! -f $SAVED_HEADERS_FILE ] || [ ! -f $OUTPUT_C_FILE ] ; then
	# if no, create them
    echo $HEADER_FILES > $SAVED_HEADERS_FILE
	HEADER_FILES=`cat $SAVED_HEADERS_FILE`
else
	SAVED_HEADER_FILES=`cat $SAVED_HEADERS_FILE`
	TEMPORAL_HEADER_FILES=`cat $TEMPORAL_HEADERS_FILE`
	# clean to not setup the classes if not needed
	HEADER_FILES=

	# check for header files additions or deletions
	if ! [ "$SAVED_HEADER_FILES" == "$TEMPORAL_HEADER_FILES" ]; then
		echo "Files differ"
		HEADER_FILES=$TEMPORAL_HEADERS_FILE
		`cat $TEMPORAL_HEADERS_FILE > $SAVED_HEADERS_FILE`
		HEADER_FILES=`cat $SAVED_HEADERS_FILE`
    fi
fi


# if the header files list was populated, generate the setupClass.c files
if [ -n "$HEADER_FILES" ]; then

	echo -n "Generating "
	echo $OUTPUT_C_FILE | sed 's/.//1' | sed 's/./\:\//2' | sed 's/./\u&/1'
	rm -f $CLASSES_FILE
	rm -f $OUTPUT_C_FILE
	echo " " > $OUTPUT_C_FILE
	echo " " > $CLASSES_FILE

	echo "// Do not modify this file, it is auto-generated" > $OUTPUT_C_FILE

	for headerFile in $HEADER_FILES; do
		className=`grep "__CLASS(" $headerFile`
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
