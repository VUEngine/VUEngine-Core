#!/bin/bash

INPUT_FILE=
OUTPUT_FILE=
WORKING_FOLDER=build/compiler/preprocessor
HELPER_FILES_PREFIXES=

while [[ $# -gt 1 ]]
do
	key="$1"
	case $key in
		-i|-output)
		INPUT_FILE="$2"
		shift # past argument
		;;
		-o|-output)
		OUTPUT_FILE="$2"
		shift # past argument
		;;
		-w|-output)
		WORKING_FOLDER="$2"
		shift # past argument
		;;
		-p|-output)
		HELPER_FILES_PREFIXES="$HELPER_FILES_PREFIXES $2"
		shift # past argument
		;;
	esac

	shift
done

#echo WORKING_FOLDER $WORKING_FOLDER
#echo INPUT_FILE $INPUT_FILE
#echo OUTPUT_FILE $OUTPUT_FILE
cat $INPUT_FILE > $OUTPUT_FILE

if [[ ${INPUT_FILE} != *"source/"* ]];then

	exit 0
fi

if [ ! -d $WORKING_FOLDER ]; then
	mkdir -p $WORKING_FOLDER
fi

anyMethodVirtualized=false

for prefix in $HELPER_FILES_PREFIXES
do
	#echo prefix $prefix
	VIRTUAL_METHODS_FILE=$WORKING_FOLDER/$prefix"VirtualMethods.txt"
	VIRTUAL_CALLS_FILE=$WORKING_FOLDER/$prefix"VirtualMethodCalls.txt"

	virtualMethods=`cat $VIRTUAL_METHODS_FILE`
	fileClass=`grep "__CLASS_DEFINITION(" $OUTPUT_FILE | cut -d, -f1 | cut -d\( -f2 `
	fileBaseClass=`grep "__CLASS_DEFINITION(" $OUTPUT_FILE | cut -d, -f2  | cut -d\) -f1 `
	TEMPORAL_METHOD_LIST=$WORKING_FOLDER/processedMethods.txt

	if [ -f $TEMPORAL_METHOD_LIST ] ; then
		rm $TEMPORAL_METHOD_LIST
	fi

	touch $TEMPORAL_METHOD_LIST

	# replace base method calls
	sed -i -e "s#Base_\([A-z][A-z0-0]\+\)(#__CALL_BASE_METHOD($fileBaseClass,\1, #g" $OUTPUT_FILE

	#echo "Processing source $INPUT_FILE"

	for method in $virtualMethods
	do
		if [ ! -z "$method" ];
		then
			#echo "Checking $method"
			methodPartialCall="_$method"

			while IFS= read -r methodCall;
			do

				if [ -z "$methodCall" ]; then
					break;
				fi

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

					isVirtual=`grep $pureMethodCall $VIRTUAL_CALLS_FILE`
					if [ ! -z "$isVirtual" ];
					then
						class=`echo $pureMethodCall | cut -d_ -f1 `
						#echo $method is going to be virtualized
						anyMethodVirtualized=true

						# flag declarations so they don't get replaced
						sed -i -e "s#\(\<$pureMethodCall\>(.*[A-z] [A-z].*\)#VUEngine_DEC_MARK\1#g" $OUTPUT_FILE

						# replace virtual method calls
						sed -i -e "s#\<$pureMethodCall\>(#__VIRTUAL_CALL($class, $method, #g" $OUTPUT_FILE

						# remove declaration
						sed -i -e "s#VUEngine_DEC_MARK##g" $OUTPUT_FILE
					fi
				fi
			done <<< "$(grep -o -e "[A-z]\+[a-zA-z0-9]*$methodPartialCall(.*)" $OUTPUT_FILE)";
		fi
	done
done

if [ "$anyMethodVirtualized" = true ] ; then
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "FILE: $INPUT_FILE" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	grep CALL_BASE_METHOD $OUTPUT_FILE | sed -e "s#.*\(__CALL_BASE_METHOD(.*\)#	\1#g" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	grep VIRTUAL_CALL $OUTPUT_FILE | sed -e "s#.*\(__VIRTUAL_CALL(.*\)#	\1#g" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
fi