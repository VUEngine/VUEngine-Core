#!/bin/bash

INPUT_FILE=
OUTPUT_FILE=
WORKING_FOLDER=build/compiler/preprocessor
HELPER_FILES_PREFIXES=
PRINT_DEBUG_OUTPUT=
CLASSES_HIERARCHY_FILE=$WORKING_FOLDER/classesHierarchy.txt

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
		-c|-output)
		CLASSES_HIERARCHY_FILE="$2"
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

className=`grep -m 1 -e '^.*::[ 	]*constructor[ 	]*(' $OUTPUT_FILE | sed -e 's#^.*[ 	]\+\([A-Z][A-z0-9]*\)::.*#\1#'`

firstMethodDeclarationLine=`grep -m1 -n -e "^[ 	]*[^\*//]*[ 	]\+[A-Z][A-z0-9]*[ 	]*::[ 	]*[a-z][A-z0-9]*[ 	]*(.*)" $OUTPUT_FILE | cut -d ":" -f1`

sed -i -e 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g' $OUTPUT_FILE

if [ -z "$className" ];then
	exit 0
fi

echo Class: $className

baseClassName=`grep -m1 -e "^$className:" $CLASSES_HIERARCHY_FILE | cut -d ":" -f2`
if [ -z "$baseClassName" ];then
	exit 0
fi

classModifiers=`grep -m1 -e "^$className:" $CLASSES_HIERARCHY_FILE | cut -d ":" -f3`

classDefinition="__CLASS_DEFINITION("$className", "$baseClassName");"
# Add allocator if it is not abstract nor a singleton class
if [[ ! $classModifiers = *"singleton "* ]] && [[ ! $classModifiers = *"abstract "* ]] ;
then
#	echo "Adding allocator"
	constructor=`grep -m 1 -e $className"_constructor[ 	]*(.*)" $OUTPUT_FILE`
	constructorParameters=`sed -e 's#^.*(\(.*\))[ 	]*$#\1#' <<< "$constructor"`
#	echo "constructorParameters $constructorParameters"
	allocatorParameters=`cut -d "," -f2- <<< "$constructorParameters,"`
#	echo "allocatorParameters $allocatorParameters"
	allocatorArguments=`sed -e 's#[ 	*]\+\([A-z0-9]\+[ 	]*,\)#<\1>\n#g' <<< "$allocatorParameters" | sed -e 's#.*<\(.*\)>.*#\1#g' | tr -d '\n' | sed -e 's#\(.*\),#\1#'`
	allocatorParameters=`sed -e 's#\(.*\),#\1#' <<< "$allocatorParameters"`

	if [ -z "$allocatorParameters" ];then
		classDefinition=$classDefinition"\n__CLASS_NEW_DEFINITION($className)"
		classDefinition=$classDefinition"\n__CLASS_NEW_END($className);"
	else
		classDefinition=$classDefinition"\n__CLASS_NEW_DEFINITION($className, $allocatorParameters)"
		classDefinition=$classDefinition"\n__CLASS_NEW_END($className, $allocatorArguments);"
	fi
else
	if [[ $classModifiers = *"singleton "* ]] ;
	then
		classDefinition=$classDefinition"\n void "$className"_constructor("$className" this);"

		hasCustomSingletonDefinition=`grep -e '#define[ 	]\+.*SINGLETON.*(' $OUTPUT_FILE`

		if [ -z "$hasCustomSingletonDefinition" ];
		then
			if [[ $classModifiers = *"dynamic_singleton "* ]] ;
			then
				classDefinition=$classDefinition"\n __SINGLETON_DYNAMIC($className);"
			else
				classDefinition=$classDefinition"\n __SINGLETON($className);"
			fi
		fi
	fi
fi

echo "Base: $baseClassName"

if [ ! -z "$firstMethodDeclarationLine" ];then
	firstMethodDeclarationLine=$((firstMethodDeclarationLine - 1))
	sed -i -e "${firstMethodDeclarationLine}s#.*#&\n$classDefinition#" $OUTPUT_FILE
fi

sed -i -e 's#[ 	]*friend[ 	]\+class[ 	]\+\([A-z0-9]\+\)#__CLASS_FRIEND_DEFINITION(\1)#' $OUTPUT_FILE

# replace base method calls
sed -i -e "s#Base_constructor(\(.*\)#__CONSTRUCT_BASE($baseClassName,\1#g" -e 's#,[ 	]*);#);#' $OUTPUT_FILE
sed -i -e "s#Base_destructor()#__DESTROY_BASE#g" $OUTPUT_FILE

# replace base method calls
sed -i -e "s#Base_\([A-z][A-z0-0]\+\)(#__CALL_BASE_METHOD($baseClassName,\1, #g" $OUTPUT_FILE



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
	TEMPORAL_METHOD_LIST=$WORKING_FOLDER/processedMethods.txt


	if [ -f $TEMPORAL_METHOD_LIST ] ; then
		rm $TEMPORAL_METHOD_LIST
	fi

	touch $TEMPORAL_METHOD_LIST

	#echo "Processing source $INPUT_FILE"
	virtualMethodsInFile=`grep -o -n -e "$virtualMethods" $OUTPUT_FILE`

	if [ -z "$virtualMethodsInFile" ] ; then
		continue;
	fi

	while : ; do

		while IFS= read -r methodCall;
		do
			if [ -z "$methodCall" ];
			then
				continue;
			fi

#			echo "Checking $methodCall"
			pureMethodCall=`echo $methodCall | cut -d: -f2 | cut -d\( -f1`
			#echo pureMethodCall $pureMethodCall

			line=`cut -d: -f1 <<< "$methodCall"`
			class=`cut -d_ -f1 <<< "$pureMethodCall"`
			method=`cut -d_ -f2 <<< "$pureMethodCall"`
#			echo "$pureMethodCall is going to be virtualized into $class and $method at $line"
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