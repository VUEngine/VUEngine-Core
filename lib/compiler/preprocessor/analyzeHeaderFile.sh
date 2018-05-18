#!/bin/bash

GAME_HOME=
OUTPUT_C_FILE=setupClasses.c
HEADER_FOLDERS=
DELETE_HELPER_FILES=false
WORKING_FOLDER=build/compiler/preprocessor
HELPER_FILES_PREFIX=engine

while [[ $# -gt 1 ]]
do
	key="$1"
	case $key in
		-h|-output)
		HEADER_FOLDERS="$HEADER_FOLDERS $2"
		shift # past argument
		;;
		-w|-output)
		WORKING_FOLDER="$2"
		shift # past argument
		;;
		-d|-output)
		DELETE_HELPER_FILES=true
		;;
		-p|-output)
		HELPER_FILES_PREFIX="$2"
		shift # past argument
		;;
	esac

	shift
done

HEADER_FILES=


echo Preprocessing virtual methods in:

for headerFolder in $HEADER_FOLDERS; do

	echo "	$headerFolder"

	HEADER_FILES="$HEADER_FILES "`find $headerFolder/ -name "*.h"`
done


if [ ! -d $WORKING_FOLDER ]; then
	mkdir -p $WORKING_FOLDER
fi

VIRTUAL_METHODS_FILE=$WORKING_FOLDER/$HELPER_FILES_PREFIX"VirtualMethods.txt"

#echo VIRTUAL_METHODS_FILE $VIRTUAL_METHODS_FILE

# check if necessary files already exist

if [ $DELETE_HELPER_FILES ]; then
	if [ -f $VIRTUAL_METHODS_FILE ] ; then
		rm $VIRTUAL_METHODS_FILE
	fi

	touch $VIRTUAL_METHODS_FILE
else
	if [ ! -f $VIRTUAL_METHODS_FILE ] ; then
		touch $VIRTUAL_METHODS_FILE
	fi
fi

# if the header files list was populated, generate the helper files
#echo "$HEADER_FILES $HEADER_FILES"
if [ -n "$HEADER_FILES" ]; then

	for headerFile in $HEADER_FILES; do

		className=`grep "__CLASS(" $headerFile`
		className=`echo $className | sed 's/__CLASS(//' | sed 's/);//'`

		if ! [[ "$className" =~ "define" ]]; then

			if [ -n "$className" ]; then

				virtualMethods=`grep "VIRTUAL_DEC" $headerFile | grep -v "#define" | cut -d, -f3 | cut -d\) -f1 | sed '/^\s*$/d'`
				overrodeMethods=`grep "__VIRTUAL_SET" $headerFile | grep -v "#define" | cut -d, -f3  | cut -d\) -f1 | sed '/^\s*$/d'`

				for method in $virtualMethods
				do
					if [ ! -z "$method" ]
					then
						methodCall="$className""_""$method"
						hasMethod=`grep -e "|$methodCall(" $VIRTUAL_METHODS_FILE`

						if [ -z "$hasMethod" ];
						then
							echo -n "$separator\<$methodCall[ 	]*(.*" >> $VIRTUAL_METHODS_FILE
							separator="\|"
						fi
					fi
				done

				for method in $overrodeMethods
				do
					if [ ! -z "$method" ]
					then
						methodCall="$className""_""$method"
						hasMethod=`grep -e "|$methodCall(" $VIRTUAL_METHODS_FILE`

						if [ -z "$hasMethod" ];
						then
							echo -n "$separator\<$methodCall[ 	]*(.*" >> $VIRTUAL_METHODS_FILE
							separator="\|"
						fi
					fi
				done
			fi
		fi
	done
fi
