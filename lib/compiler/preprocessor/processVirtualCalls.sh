#!/bin/bash

INPUT_FILE=$1
OUTPUT_FILE=$2
cat $INPUT_FILE > $OUTPUT_FILE

if [[ ${INPUT_FILE} != *"source/"* ]];then

	exit 0
fi

GAME_HOME=$3

WORKING_FOLDER=$GAME_HOME/lib/compiler/preprocessor
virtualMethods=`cat $WORKING_FOLDER/virtualMethods.txt`
fileClass=`grep "__CLASS_DEFINITION(" $OUTPUT_FILE | cut -d, -f1 | cut -d\( -f2 `
fileBaseClass=`grep "__CLASS_DEFINITION(" $OUTPUT_FILE | cut -d, -f2  | cut -d\) -f1 `
TEMPORAL_METHOD_LIST=$WORKING_FOLDER/processedMethods.txt

echo "" > $TEMPORAL_METHOD_LIST

# replace base method calls
sed -i -e "s#Base_\([A-z][A-z0-0]\+\)(#__CALL_BASE_METHOD($fileBaseClass, \1, #g" $OUTPUT_FILE

for method in $virtualMethods
do
	if [ ! -z "$method" ];
	then
		#echo "Checking $method"
		methodPartialCall="_$method"
		grep -o -e "[A-z]\+[a-zA-z0-9]*$methodPartialCall(.*)" $OUTPUT_FILE  | while IFS= read -r methodCall;
		do
			methodCall=`echo $methodCall | sed -e "s#$method(.*[A-Za-z0-9] \+[A-Za-z0-9].*)#VUEngine_DEC_MARK#g"`

			if [[ ${methodCall} != *"VUEngine_DEC_MARK"* ]];then

				pureMethodCall=`echo $methodCall | sed -e "s#(.*##g"`

				methodAlreadyProcessed=`grep $pureMethodCall $TEMPORAL_METHOD_LIST`

				if [ ! -z "$methodAlreadyProcessed" ];
				then
					#echo "Already processed $pureMethodCall"
					continue
				fi

				echo $pureMethodCall >> $TEMPORAL_METHOD_LIST

				isVirtual=`grep $pureMethodCall $WORKING_FOLDER/virtualMethodCalls.txt`
				if [ ! -z "$isVirtual" ];
				then
					class=`echo $pureMethodCall | cut -d_ -f1 `
					#echo $method is going to be virtualized

					# flag declarations so they don't get replaced
					sed -i -e "s#\(\<$pureMethodCall\>(.*[A-z] [A-z].*\)#VUEngine_DEC_MARK\1#g" $OUTPUT_FILE

					# replace virtual method calls
					sed -i -e "s#\<$pureMethodCall\>(#__VIRTUAL_CALL($class, $method, #g" $OUTPUT_FILE

					# remove declaration
					sed -i -e "s#VUEngine_DEC_MARK##g" $OUTPUT_FILE
				fi
            fi

		done;
	fi
done

cat $OUTPUT_FILE > $OUTPUT_FILE.cc
