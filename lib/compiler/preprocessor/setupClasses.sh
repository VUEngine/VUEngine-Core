#!/bin/bash
#
OUTPUT_C_FILE=setupClasses.c
HEADER_FOLDERS=
WORKING_FOLDER=build/working
CLASSES_HIERARCHY_FILE=$WORKING_FOLDER/classesHierarchy.txt
LIBRARY="LibraryName"
SETUP_FUNCTION="ClassName"

while [ $# -gt 1 ]
do
	key="$1"
	case $key in
		-n)
		LIBRARY="$2"
		SETUP_FUNCTION=`sed $'s/[^[:alnum:]\t]//g' <<< $2`
		shift # past argument
		;;
		-o)
		OUTPUT_C_FILE="$2"
		shift # past argument
		;;
		-w)
		WORKING_FOLDER="$2"
		shift # past argument
		;;
		-c)
		CLASSES_HIERARCHY_FILE="$2"
		shift # past argument
		;;
	esac

	shift
done

OUTPUT_C_FILE="$OUTPUT_C_FILE"
#echo WORKING_FOLDER $WORKING_FOLDER
#echo OUTPUT_C_FILE $OUTPUT_C_FILE

if [ ! -d $WORKING_FOLDER ]; then
	exit 0
fi

CLASSES_FILE=$WORKING_FOLDER/classesFile.txt


# if the classes hierarchy files list was populated, generate the setupClass.c files
if [ -n "$CLASSES_HIERARCHY_FILE" ]; then

#	echo -n "Generating "
#	echo $OUTPUT_C_FILE
	rm -f $CLASSES_FILE
	rm -f $OUTPUT_C_FILE
	echo " " > $OUTPUT_C_FILE
	echo " " > $CLASSES_FILE

	echo "// Do not modify this file, it is auto-generated" > $OUTPUT_C_FILE

	if [ -f "$CLASSES_HIERARCHY_FILE" ]; then
		CLASSES_NAMES=`grep -v ':.*static.*' $CLASSES_HIERARCHY_FILE | sed -e 's/:.*//g'`
	fi

#	echo "CLASSES_NAMES $CLASSES_NAMES"

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

	FINAL_SETUP_CLASSES_FILE=$WORKING_FOLDER/objects/setupClasses.c

	if [ -f $FINAL_SETUP_CLASSES_FILE ]; then
		rm $FINAL_SETUP_CLASSES_FILE
	fi

	# Setup calls in final file
	SETUP_CLASSES_FILES=`find $WORKING_FOLDER/objects/ -name "*SetupClasses.c"`

	echo "// setup function" > $FINAL_SETUP_CLASSES_FILE

	#create the function
	echo "void setupClasses(void)" >> $FINAL_SETUP_CLASSES_FILE
	echo "{" >> $FINAL_SETUP_CLASSES_FILE

	# Create the calls directives
	for setupClassFile in $SETUP_CLASSES_FILES
	do
		setupFunction=`grep "SetupClasses" $setupClassFile | sed -e "s/.*void[ 	]*\(.*SetupClasses\)(.*/\1/g"`
		#echo setupFunction $setupFunction

		# add function setup call
		echo "	"$setupFunction"();" >> $FINAL_SETUP_CLASSES_FILE

		#add forward declaration
		echo "void $setupFunction(void);" | cat - $FINAL_SETUP_CLASSES_FILE > $WORKING_FOLDER/temp.txt && mv $WORKING_FOLDER/temp.txt $FINAL_SETUP_CLASSES_FILE
	done

	echo "}" >> $FINAL_SETUP_CLASSES_FILE
fi

rm -f $CLASSES_FILE
#echo $OUTPUT_C_FILE | sed -e 's#^.*objects/\(.*$\)#Compiling source: \1#g'
echo "Setting up library: $LIBRARY"