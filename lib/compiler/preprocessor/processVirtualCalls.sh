#!/bin/bash

INPUT_FILE=
OUTPUT_FILE=
WORKING_FOLDER=build/compiler/preprocessor
HELPER_FILES_PREFIXES=
PRINT_DEBUG_OUTPUT=

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
		-d|-output)
		PRINT_DEBUG_OUTPUT="true"
		;;
	esac

	shift
done

#echo WORKING_FOLDER $WORKING_FOLDER
#echo INPUT_FILE $INPUT_FILE
#echo OUTPUT_FILE $OUTPUT_FILE
cat $INPUT_FILE > $OUTPUT_FILE
sed -i -e 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)[ 	]*(#\1_\2(#g' $OUTPUT_FILE

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
	implementationDefinition=`grep -m 1 -e "^[ \t]*implements[ \t]\+" $OUTPUT_FILE | sed -e 's#implements##'`
	fileClass=`cut -d ":" -f1 <<< "$implementationDefinition" `
	fileBaseClass=`cut -d ":" -f2 <<< "$implementationDefinition" | cut -d ";" -f1`
	echo fileClass $fileClass
	echo fileBaseClass $fileBaseClass
	TEMPORAL_METHOD_LIST=$WORKING_FOLDER/processedMethods.txt

	sed -i -e 's#implements.*#__CLASS_DEFINITION('"$fileClass"', '"$fileBaseClass"');#' $OUTPUT_FILE
	sed -i -e 's#[ 	]*friend[ 	]\+class[ 	]\+\([A-z0-9]\+\)#__CLASS_FRIEND_DEFINITION(\1)#' $OUTPUT_FILE

	if [ -f $TEMPORAL_METHOD_LIST ] ; then
		rm $TEMPORAL_METHOD_LIST
	fi

	touch $TEMPORAL_METHOD_LIST

	# replace base method calls
	sed -i -e "s#Base_constructor(\(.*\)#__CONSTRUCT_BASE($fileBaseClass,\1#g" -e 's#,[ 	]*);#);#' $OUTPUT_FILE
	sed -i -e "s#Base_destructor()#__DESTROY_BASE#g" $OUTPUT_FILE

	# replace base method calls
	sed -i -e "s#Base_\([A-z][A-z0-0]\+\)(#__CALL_BASE_METHOD($fileBaseClass,\1, #g" $OUTPUT_FILE

	#echo "Processing source $INPUT_FILE"

	while : ; do

		while IFS= read -r methodCall;
		do
			if [ -z "$methodCall" ];
			then
				continue;
			fi

			#echo "Checking $methodCall"
			pureMethodCall=`echo $methodCall | cut -d: -f2 | cut -d\( -f1`
			#echo pureMethodCall $pureMethodCall

			line=`cut -d: -f1 <<< "$methodCall"`
			class=`cut -d_ -f1 <<< "$pureMethodCall"`
			method=`cut -d_ -f2 <<< "$pureMethodCall"`
			#echo "$pureMethodCall is going to be virtualized into $class and $method at $line"
			anyMethodVirtualized=true

			# replace virtual method calls
			sed -i -e "${line}s#\([^A-z]*\)$pureMethodCall(#\1__VIRTUAL_CALL($class, $method, #g" $OUTPUT_FILE

		done <<< "$(grep -o -n -e "$virtualMethods" $OUTPUT_FILE | grep -v -e '([A-Za-z0-9][A-Za-z0-9]*  *\t*[A-Za-z0-9].*')";

		pendingSubstitutions=`grep -o -n -e ".*$virtualMethods" $OUTPUT_FILE | grep -v -e '([A-Za-z0-9][A-Za-z0-9]*  *\t*[A-Za-z0-9].*'`

		if [ -z "$pendingSubstitutions" ];
		then
			break;
		fi

		#echo "Pending substitutions: \necho $pendingSubstitutions"
	done
done

if [ $PRINT_DEBUG_OUTPUT ] && [ "$anyMethodVirtualized" = true ] ; then
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "FILE: $INPUT_FILE" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	grep -n CALL_BASE_METHOD $OUTPUT_FILE | sed -e "s#\([0-9]\+:\).*\(__CALL_BASE_METHOD(.*\)#\1	\2#g" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	grep -n VIRTUAL_CALL $OUTPUT_FILE | sed -e "s#\([0-9]\+:\).*\(__VIRTUAL_CALL(.*\)#\1	\2#g" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
fi