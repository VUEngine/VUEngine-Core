#!/bin/bash

OUTPUT_C_FILE=setupClasses.c
HEADER_FOLDERS=
WORKING_FOLDER=build/compiler/preprocessor

while [[ $# -gt 1 ]]
do
	key="$1"
	case $key in
		-h|-output)
		HEADER_FOLDERS="$HEADER_FOLDERS $2"
		shift # past argument
		;;
		-o|-output)
		OUTPUT_C_FILE="$2"
		shift # past argument
		;;
		-w|-output)
		WORKING_FOLDER="$2"
		shift # past argument
		;;
		*)
		HEADER_FOLDERS="$HEADER_FOLDERS $1"
		;;
	esac

	shift
done

HEADER_FILES=
SAVED_HEADERS_FILE=$WORKING_FOLDER/headerFilesFor-$OUTPUT_C_FILE.txt
TEMPORAL_HEADERS_FILE=temporalHeaderFiles.txt
SETUP_FUNCTION=`echo $OUTPUT_C_FILE | sed -e "s/.c//g"`
OUTPUT_C_FILE="$WORKING_FOLDER/$OUTPUT_C_FILE"
#echo WORKING_FOLDER $WORKING_FOLDER
#echo OUTPUT_C_FILE $OUTPUT_C_FILE

echo Preprocessing classes in:

for headerFolder in $HEADER_FOLDERS; do

	echo "	$headerFolder"
	HEADER_FILES="$HEADER_FILES "`find $headerFolder/source/ -name "*.h"`
done

if [ ! -d $WORKING_FOLDER ]; then
	mkdir -p $WORKING_FOLDER
fi

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
	echo $OUTPUT_C_FILE
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
	echo "void $SETUP_FUNCTION(void)" >> $OUTPUT_C_FILE
	echo "{" >> $OUTPUT_C_FILE

	# Create the calls directives
	for className in $CLASSES_NAMES; do
		echo "	"$className"_setVTable();" >> $OUTPUT_C_FILE
	done
	echo "}" >> $OUTPUT_C_FILE

	FINAL_SETUP_CLASSES_FILE=$WORKING_FOLDER/setupClasses.c

	if [ -f $FINAL_SETUP_CLASSES_FILE ]; then
		rm $FINAL_SETUP_CLASSES_FILE
	fi

	# Setup calls in final file
	SETUP_CLASSES_FILES=`find $WORKING_FOLDER/ -name "*SetupClasses.c"`

	echo "// setup function" > $FINAL_SETUP_CLASSES_FILE

	#create the function
	echo "void setupClasses(void)" >> $FINAL_SETUP_CLASSES_FILE
	echo "{" >> $FINAL_SETUP_CLASSES_FILE

	# Create the calls directives
	for setupClassFile in $SETUP_CLASSES_FILES
	do
		setupFunction=`grep "SetupClasses" $setupClassFile | sed -e "s/.*void \+\(.*SetupClasses\)(.*/\1/g"`
		echo setupFunction $setupFunction

		# add function setup call
		echo "	"$setupFunction"();" >> $FINAL_SETUP_CLASSES_FILE

		#add forward declaration
		echo "void $setupFunction(void);" | cat - $FINAL_SETUP_CLASSES_FILE > $WORKING_FOLDER/temp.txt && mv $WORKING_FOLDER/temp.txt $FINAL_SETUP_CLASSES_FILE
	done

	echo "}" >> $FINAL_SETUP_CLASSES_FILE
fi

rm -f $CLASSES_FILE
rm -f $TEMPORAL_HEADERS_FILE
