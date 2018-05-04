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
	esac

	shift
done

HEADER_FILES=


echo Preprocessing virtual methods in:

for headerFolder in $HEADER_FOLDERS; do

	echo "	"$headerFolder | sed 's/.//2' | sed 's/./\:\//3' | sed 's/./\u&/2'

	HEADER_FILES="$HEADER_FILES "`find $headerFolder/ -name "*.h"`
done

WORKING_FOLDER=$GAME_HOME/lib/compiler/preprocessor

VIRTUAL_METHODS_FILE=$WORKING_FOLDER/virtualMethods.txt
VIRTUAL_CALLS_FILE=$WORKING_FOLDER/virtualMethodCalls.txt

# check if necessary files already exist
if [ -f $VIRTUAL_METHODS_FILE ] ; then
	rm $VIRTUAL_METHODS_FILE
fi

if [ -f $VIRTUAL_CALLS_FILE ] ; then
	rm $VIRTUAL_CALLS_FILE
fi

touch $VIRTUAL_METHODS_FILE
touch $VIRTUAL_CALLS_FILE

# if the header files list was populated, generate the helper files
if [ -n "$HEADER_FILES" ]; then

	for headerFile in $HEADER_FILES; do

		className=`grep "__CLASS(" $headerFile`
		className=`echo $className | sed 's/__CLASS(//' | sed 's/);//'`

		if ! [[ "$className" =~ "define" ]]; then

			if [ -n "$className" ]; then

				virtualMethods=`grep "VIRTUAL_DEC" $headerFile | grep -v "#define" | cut -d, -f3 | cut -d\) -f1 | sed '/^\s*$/d'`
				overrodeMethods=`grep "__VIRTUAL_SET" $headerFile | grep -v "#define" | cut -d, -f3  | cut -d\) -f1 | sed '/^\s*$/d'`

				for method in $virtualMethods$overrodeMethods
				do
					if [ ! -z "$method" ]
					then
						methodCall="$className""_""$method"

						hasMethod=`grep -sw $method $VIRTUAL_METHODS_FILE`
						if [ -z "$hasMethod" ];
						then
							echo "$method" >> $VIRTUAL_METHODS_FILE
						fi

						echo "$methodCall" >> $VIRTUAL_CALLS_FILE
					fi
				done
			fi
		fi
	done
fi
