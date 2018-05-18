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

if [[ ${INPUT_FILE} = *"assets/"* ]];then
	exit 0
fi


className=`grep -m 1 -e '^.*::[ 	]*constructor[ 	]*(' $OUTPUT_FILE | sed -e 's#^.*[ 	]\+\([A-Z][A-z0-9]*\)::.*#\1#'`
isStatic=false

if [ -z "$className" ];then
# Maybe it is a static class
	isStatic=false

	grep -o -m 1 -e '^.*[ 	]\+[A-Z][A-z0-9]*[ 	]*::[ 	]*[a-z][A-z0-9]*[ 	]*(' $OUTPUT_FILE
	className=`grep -o -m 1 -e '^.*[ 	]\+[A-Z][A-z0-9]*[ 	]*::[ 	]*[a-z][A-z0-9]*[ 	]*(' $OUTPUT_FILE | sed -e 's/^.*[ 	]\+\([A-Z][A-z0-9]*\)[ 	]*::.*/\1/'`
fi
# INJECTION OF ClassName _this into method declarations

mark="@N@"
# Mark block starters
sed -i -e 's/{/{<START_BLOCK>/g' $OUTPUT_FILE

# Inline multine declaratarions
sed -i -e 's/,[ 	]*$/,<·>/g' $OUTPUT_FILE
awk '{if ($0 ~ "<·>") printf "%s ", $0; else print;}' $OUTPUT_FILE > $OUTPUT_FILE.tmp &&   mv $OUTPUT_FILE.tmp $OUTPUT_FILE

#sed -i -e '/,<·>[ 	]*$/N' -e 's/\n//g' $OUTPUT_FILE
#sed -i -e '/,<·>[ 	]*$/N' -e 's/\n//g' $OUTPUT_FILE
#sed -i -e '/,<·>[ 	]*$/N' -e 's/\n//g' $OUTPUT_FILE
#sed -i -e '/,<·>[ 	]*$/N' -e 's/\n//g' $OUTPUT_FILE

# Identify static declarations
sed -i -e 's/.*static.*/&<%>/g' $OUTPUT_FILE
echo >> $OUTPUT_FILE

# Find method declarations
sed -e 's/.*/'"$mark"'&/g' $OUTPUT_FILE | tr -d '\n' | sed -e 's/'"$mark"'\([ 	]*[A-z0-9_ 	]*[A-z0-9_\*]\+[ 	]\+'"$className"'[ 	]*::[ 	]*[a-z][A-z0-9]*[ 	]*([^@]*)[ 	<%>]*'"$mark"'[ 	]*{[ 	]*<START_BLOCK>[^@]*\)'"$mark"'/'"$mark"'<DECLARATION>\1<%DECLARATION>/g'  > $OUTPUT_FILE

# Add static qualifier to static methods block start
sed  -i -e 's/<%>@N@{/@N@<%>{/g' $OUTPUT_FILE

# Put back line breaks
sed  -i -e 's/'"$mark"'/\n/g'  $OUTPUT_FILE
#exit 0

# Inject this pointer
sed -i -e 's/<%>{<START_BLOCK>/{\n/g' -e 's/<START_BLOCK>\(.*\)<%DECLARATION>/'"$className"' this '"__attribute__ ((unused))"' = __SAFE_CAST('"$className"' , _this);\1\n/g' $OUTPUT_FILE

# Inject _this parameter
sed -i -e '/<DECLARATION>[A-z0-9 	]*static[ 	]\+/!s/\(<DECLARATION>[ 	]*.*'"$className"'[ 	]*::[ 	]*[a-z][A-z0-9]*[ 	]*\)(/\1(void* _this '"__attribute__ ((unused))"',/g' $OUTPUT_FILE
# Clean methods with no parameters declarations
sed -i -e 's/,[ 	]*)/)/g' $OUTPUT_FILE

# Replace :: by _
sed -i -e 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g' $OUTPUT_FILE

firstMethodDeclarationLine=`grep -m1 -n -e "^<DECLARATION>" $OUTPUT_FILE | cut -d ":" -f1`
prototypes=`grep -e '^<DECLARATION>.*)' $OUTPUT_FILE | sed -e 's#)$#);#' | tr -d '\n'`
#prototypes=`grep -e '^<DECLARATION>.*)' $OUTPUT_FILE | sed -e 's#)$#);#'`

#echo "prototypes $prototypes"
#echo "firstMethodDeclarationLine $firstMethodDeclarationLine"

if [ -z "$className" ];then
	sed -i -e 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g' $OUTPUT_FILE
	sed -i -e 's/<%>//g' $OUTPUT_FILE
	sed -i -e 's/<[%]*DECLARATION>[ 	]*static[ 	]\+/ /g' $OUTPUT_FILE
	sed -i -e 's/<[%]*DECLARATION>//g' $OUTPUT_FILE
	sed -i -e 's/<START_BLOCK>//g' $OUTPUT_FILE
	sed -i -e 's/,<·>/,\n/g' $OUTPUT_FILE
	exit 0
fi

baseClassName=`grep -m1 -e "^$className:" $CLASSES_HIERARCHY_FILE | cut -d ":" -f2`
if [ -z "$baseClassName" ];then
	exit 0
fi

if [[ ${INPUT_FILE} != *"source/"* ]];then
	exit 0
fi

if [ ! -d $WORKING_FOLDER ]; then
	mkdir -p $WORKING_FOLDER
fi

# Move declaration mark to the end in preparation for virtual method call substitutions
sed -i -e 's/<DECLARATION>.*/&<DECLARATION>/g' $OUTPUT_FILE

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
#			echo pureMethodCall $pureMethodCall

			line=`cut -d: -f1 <<< "$methodCall"`
			class=`cut -d_ -f1 <<< "$pureMethodCall"`
			method=`cut -d_ -f2 <<< "$pureMethodCall"`
#			echo "$pureMethodCall is going to be virtualized into $class and $method at $line"
			anyMethodVirtualized=true

			# replace virtual method calls
			sed -i -e "${line}s#\([^A-z]*\)$pureMethodCall(#\1__VIRTUAL_CALL($class, $method, #g" $OUTPUT_FILE

		done <<< "$(grep -o -n -e "$virtualMethods" $OUTPUT_FILE | grep -v -e '<DECLARATION>')";

		pendingSubstitutions=`grep -o -n -e "$virtualMethods" $OUTPUT_FILE | grep -v -e '<DECLARATION>'`

		if [ -z "$pendingSubstitutions" ];
		then
			break;
		fi

		#echo "Pending substitutions: \necho $pendingSubstitutions"
	done
done

# clean up
sed -i -e 's/<%>//g' $OUTPUT_FILE
sed -i -e 's/<[%]*DECLARATION>[ 	]*static[ 	]\+/ /g' $OUTPUT_FILE
sed -i -e 's/<[%]*DECLARATION>//g' $OUTPUT_FILE
sed -i -e 's/<START_BLOCK>//g' $OUTPUT_FILE

classModifiers=`grep -m1 -e "^$className:" $CLASSES_HIERARCHY_FILE | cut -d ":" -f3`

if [[ ! $classModifiers = *"static "* ]] ;
then
	classDefinition="__CLASS_DEFINITION($className, $baseClassName);$prototypes"

	# Add allocator if it is not abstract nor a singleton class
	if [[ ! $classModifiers = *"singleton "* ]] && [[ ! $classModifiers = *"static "* ]] && [[ ! $classModifiers = *"abstract "* ]] ;
	then
	#	echo "Adding allocator"
		constructor=`grep -m 1 -e $className"_constructor[ 	]*(.*)" $OUTPUT_FILE`
		constructorParameters=`sed -e 's#^.*(\(.*\))[ 	{]*$#\1#' <<< "$constructor"`
		#echo "constructorParameters $constructorParameters"
		allocatorParameters=`cut -d "," -f2- <<< "$constructorParameters,"`
		#echo "allocatorParameters $allocatorParameters"
		allocatorArguments=`sed -e 's#[ 	*]\+\([A-z0-9]\+[ 	]*,\)#<\1>\n#g' <<< "$allocatorParameters" | sed -e 's#.*<\(.*\)>.*#\1#g' | tr -d '\n' | sed -e 's#\(.*\),#\1#'`
		allocatorParameters=`sed -e 's#\(.*\),#\1#' <<< "$allocatorParameters"`

		if [ -z "$allocatorParameters" ];then
			classDefinition=$classDefinition"__CLASS_NEW_DEFINITION($className)"
			classDefinition=$classDefinition"__CLASS_NEW_END($className);"
		else
			classDefinition=$classDefinition"__CLASS_NEW_DEFINITION($className, $allocatorParameters)"
			classDefinition=$classDefinition"__CLASS_NEW_END($className, $allocatorArguments);"
		fi
	else
		if [[ $classModifiers = *"singleton "* ]] ;
		then
			customSingletonDefinition=`grep -o -e '#define[ 	]\+.*SINGLETON.*(' $OUTPUT_FILE`

			if [ -z "$customSingletonDefinition" ];
			then
				if [[ $classModifiers = *"dynamic_singleton "* ]] ;
				then
					classDefinition=$classDefinition"__SINGLETON_DYNAMIC($className);"
				else
					classDefinition=$classDefinition"__SINGLETON($className);"
				fi
			else
				customSingletonDefinition=`sed -e 's@^.*[ \t]\+\(.*SINGLETON.*\)(@\1@' <<< $customSingletonDefinition`
				classDefinition=$classDefinition"$customSingletonDefinition($className);"
			fi

			sed -i -e "s/Base_destructor();/_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED; Base_destructor();/" $OUTPUT_FILE

		fi
	fi
else
	classDefinition="$prototypes"
fi

echo "$classModifiers $className inherits from $baseClassName"


if [ ! -z "$firstMethodDeclarationLine" ];then

	firstMethodDeclarationLine=$((firstMethodDeclarationLine - 1))
	orig=$'\n'; replace=$'\\\n'
	sed -i -e "${firstMethodDeclarationLine}s@.*@&\n${classDefinition//$orig/$replace}@" $OUTPUT_FILE
fi

sed -i -e 's#[ 	]*friend[ 	]\+class[ 	]\+\([A-z0-9]\+\)#__CLASS_FRIEND_DEFINITION(\1)#' $OUTPUT_FILE

# replace base method calls
sed -i -e "s#Base_constructor(\(.*\)#__CONSTRUCT_BASE($baseClassName,\1#g" -e 's#,[ 	]*);#);#' $OUTPUT_FILE
sed -i -e "s#Base_destructor()#__DESTROY_BASE#g" $OUTPUT_FILE

# replace base method calls
sed -i -e "s#Base_\([A-z][A-z0-0]\+\)(#__CALL_BASE_METHOD($baseClassName,\1, #g" $OUTPUT_FILE

# clean up
sed -i -e 's/<%>//g' $OUTPUT_FILE
sed -i -e 's/<[%]*DECLARATION>[ 	]*static[ 	]\+/ /g' $OUTPUT_FILE
sed -i -e 's/<[%]*DECLARATION>//g' $OUTPUT_FILE
sed -i -e 's/<START_BLOCK>//g' $OUTPUT_FILE
sed -i -e 's/,<·>/,\n/g' $OUTPUT_FILE

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


