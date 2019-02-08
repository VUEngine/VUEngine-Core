#!/bin/bash
#

INPUT_FILE=
OUTPUT_FILE=
WORKING_FOLDER=build/compiler/preprocessor
PRINT_DEBUG_OUTPUT=
CLASSES_HIERARCHY_FILE=$WORKING_FOLDER/classes/hierarchies/classesHierarchy.txt
HEADERS_FOLDER=
LIBRARY_NAME=
LIBRARIES=
LIBRARIES_PAHT=

while [ $# -gt 1 ]
do
	key="$1"
	case $key in
		-i)
		INPUT_FILE="$2"
		shift # past argument
		;;
		-o)
		OUTPUT_FILE="$2"
		shift # past argument
		;;
		-w)
		WORKING_FOLDER="$2"
		shift # past argument
		;;
		-d)
		PRINT_DEBUG_OUTPUT="true"
		;;
		-c)
		CLASSES_HIERARCHY_FILE="$2"
		shift # past argument
		;;
		-h)
		HEADERS_FOLDER="$2"
		shift # past argument
		;;
		-n)
		LIBRARY_NAME="$2"
		shift # past argument
		;;
		-lp)
		LIBRARIES_PATH="$2"
		shift # past argument
		;;
		-l)
		;;
		*)
		LIBRARIES="$LIBRARIES $1"
		;;
	esac

	shift
done

if [ -z "$INPUT_FILE" ] || [ ! -f "$INPUT_FILE" ];
then
	echo "Input file not found: $INPUT_FILE"
	exit 0
fi

if [ "$INPUT_FILE" = "$OUTPUT_FILE" ];
then
	echo Files are the same
	echo "$INPUT_FILE" 
	echo "$OUTPUT_FILE"
	exit 0
fi

classDeclaration=`grep -n -e "^[ 	]*[A-z0-9]*[ 	]*class[ 	]\+[A-Z][A-z0-9]*[ 	]*:[ 	]*[A-Z][A-z0-9]*" $INPUT_FILE`
line=`cut -d: -f1 <<< "$classDeclaration"`
cleanClassDeclaration=`cut -d: -f2,3 <<< "$classDeclaration"`
classModifiers=`sed -e 's#^\(.*\)class .*#\1#' <<< "$cleanClassDeclaration"`
className=`sed -e 's#^.*class \([A-z][A-z0-9]*\)[ 	]*\:.*#\1#' <<< "$cleanClassDeclaration"`
baseClassName=`cut -d: -f2 <<< "$cleanClassDeclaration" | sed -e 's/[^[:alnum:]_-]//g'`

if [ -z "$className" ];
then
	cp -f $INPUT_FILE $OUTPUT_FILE
#	echo No class in $INPUT_FILE
	exit 0
fi

# Build headers search path
searchPaths="$HEADERS_FOLDER/source"
for library in $LIBRARIES;
do
	searchPaths=$searchPaths" $LIBRARIES_PATH/$library/source"
done

mustBeReprocessed=false
# Call upwards
if [ ! -z "${className##Object}" ];
then
	baseClassFile=`find $HEADERS_FOLDER/source -not -path "*/working/*" -name "$baseClassName.h"`
	processedBaseClassFile=`sed -e 's#.*'"$LIBRARY_NAME"'/\(.*\)#'"$WORKING_FOLDER"'/sources/'"$LIBRARY_NAME"'/\1#g' <<< "$baseClassFile"`

	# Call upwards if base class belongs to library
	if [ -f "$baseClassFile" ];
	then
#		echo Call upwards to "$baseClassName" from $className 
#		echo baseClassFile $baseClassFile 
#		echo processedBaseClassFile $processedBaseClassFile
		bash $VBDE/libs/vuengine/core/lib/compiler/preprocessor/processHeaderFile.sh -i $baseClassFile -o $processedBaseClassFile -w $WORKING_FOLDER -c $CLASSES_HIERARCHY_FILE -n $LIBRARY_NAME -h $HEADERS_FOLDER -lp $LIBRARIES_PATH -l $LIBRARIES
	fi

	processedBaseClassFile=`find $WORKING_FOLDER/sources -name "$baseClassName.h"`

#	echo processedBaseClassFile $processedBaseClassFile
	if [ -f "$processedBaseClassFile" ] && [ "$processedBaseClassFile" -nt "$OUTPUT_FILE" ];
	then
		mustBeReprocessed=true
	fi
fi

if [ ! "$mustBeReprocessed" = true ] &&[ -f "$OUTPUT_FILE" ] && [ "$OUTPUT_FILE" -nt "$INPUT_FILE" ];
then
#	ls -l $OUTPUT_FILE
#	ls -l $INPUT_FILE
#	echo Up to date "$className"
	exit 0
fi

# The continue
echo "Preprocessing class: $className"

# replace any previous entry
if [ -f $CLASSES_HIERARCHY_FILE ];
then
	sed -e "s#^$className:.*##g" $CLASSES_HIERARCHY_FILE > $CLASSES_HIERARCHY_FILE.tmp
	mv $CLASSES_HIERARCHY_FILE.tmp $CLASSES_HIERARCHY_FILE
else
	touch $CLASSES_HIERARCHY_FILE
fi


#echo "classDeclaration: $classDeclaration"
#echo "cleanClassDeclaration: $cleanClassDeclaration"
#echo "line: $line"
#echo "Modifiers: $classModifiers"

# Compute the class' whole hierarchy
baseClassesNamesHelper=$baseClassName":"

if [ ! -z "$baseClassName" ];
then
	baseClassesNames=$baseClassName
	baseBaseClassName=$baseClassName

	CLASSES_HIERARCHY=`cat $WORKING_FOLDER/classes/hierarchies/*.txt`

	while : ; do

		baseBaseClassName=`grep -e "^$baseBaseClassName:.*" <<< "$CLASSES_HIERARCHY" | cut -d ":" -f 2`
		baseClassesNames="$baseBaseClassName $baseClassesNames"
		baseClassesNamesHelper=$baseClassesNamesHelper$baseBaseClassName":"

		if [ -z "${baseBaseClassName##Object}" ];
		then
			break
		fi
	done
fi

# save new hierarchy
echo "$className:$baseClassesNamesHelper:$classModifiers" >> $CLASSES_HIERARCHY_FILE

# Must prevent Object class trying to actually inherit from itself
if [ -z "${className##Object}" ];
then
	baseClassName=
fi

#echo className $className
#echo baseClassesNames $baseClassesNames

end=`tail -n +$line $INPUT_FILE | grep -m 1 -n "}" | cut -d: -f1`
end=$((line + end))
line=$((line + 1))

classDeclarationBlock=`cat $INPUT_FILE | sed ''"$line"','"$end"'!d' | grep -v -e '^[ 	]*[\*//]\+.*' | sed -e 's#[{}]#\'$'\n#' | tr -d "\r\n"  | sed -e 's/;/;\'$'\n/g'`
#echo "$classDeclarationBlock"

# Get class' methods
methods=`grep -v -e '^[ 	\*A-z0-9]\+[ 	]*([ 	]*\*' <<< "$classDeclarationBlock" | grep -e '(.*)[ 	=0]*;[ 	]*'`
attributes=`grep -v -e '^[ 	\*A-z0-9]\+[ 	]*([ 	]*[^\*]' <<< "$classDeclarationBlock" | grep -e ';' | sed -e 's#&\\\##' | tr -d "\r\n"`

#echo "$classDeclarationBlock"
#echo
#echo "methods
#$methods"
#echo
#echo "attributes
#$attributes"

isSingletonClass=false
isAbstractClass=false
isStaticClass=false
isFinalClass=false


virtualMethodDeclarations="#define "$className"_METHODS(ClassName)"
virtualMethodOverrides="#define "$className"_SET_VTABLE(ClassName)"

if [ ! -z "$baseClassName" ];
then
	virtualMethodDeclarations=$virtualMethodDeclarations" "$baseClassName"_METHODS(ClassName) "
	virtualMethodOverrides=$virtualMethodOverrides" "$baseClassName"_SET_VTABLE(ClassName) "
fi

# Process each method to generate the final header
methodDeclarations=

# Create method dictionaries
CLASS_OWNED_METHODS_DICTIONARY=$WORKING_FOLDER/classes/dictionaries/$className"MethodsOwned.txt"
rm -f $CLASS_OWNED_METHODS_DICTIONARY
touch $CLASS_OWNED_METHODS_DICTIONARY

CLASS_INHERITED_METHODS_DICTIONARY=$WORKING_FOLDER/classes/dictionaries/$className"MethodsInherited.txt"
rm -f $CLASS_INHERITED_METHODS_DICTIONARY
touch $CLASS_INHERITED_METHODS_DICTIONARY

CLASS_VIRTUAL_METHODS_DICTIONARY=$WORKING_FOLDER/classes/dictionaries/$className"MethodsVirtual.txt"
rm -f $CLASS_VIRTUAL_METHODS_DICTIONARY
touch $CLASS_VIRTUAL_METHODS_DICTIONARY

CLASS_DEPENDENCIES_FILE=$WORKING_FOLDER/classes/dependencies/$LIBRARY_NAME/$className".d"
#echo "$OUTPUT_FILE:" | sed -e 's@'"$WORKING_FOLDER"'/@@g' > $CLASS_DEPENDENCIES_FILE

if [ ! -z "$baseClassesNames" ];
then
	echo "$OUTPUT_FILE: \\" > $CLASS_DEPENDENCIES_FILE
fi

# Get base classes' methods
for ancestorClassName in $baseClassesNames;
do
	ancestorInheritedMethodsDictionary=$WORKING_FOLDER/classes/dictionaries/$ancestorClassName"MethodsInherited.txt"
	ancestorVirtualMethodsDictionary=$WORKING_FOLDER/classes/dictionaries/$ancestorClassName"MethodsVirtual.txt"
	cat $ancestorInheritedMethodsDictionary | sed -e 's/^\([A-Z][A-z]*\)_\(.*\)/'"$className"'_\2 \1_\2/g' >> $CLASS_OWNED_METHODS_DICTIONARY
	cat $ancestorInheritedMethodsDictionary >> $CLASS_INHERITED_METHODS_DICTIONARY
	cat $ancestorVirtualMethodsDictionary | sed -e 's/'"$ancestorClassName"'/'"$className"'/g' >> $CLASS_VIRTUAL_METHODS_DICTIONARY
	headerFile=`find $searchPaths -name "$ancestorClassName.h"`

	if [ -f "$headerFile" ];
	then
		echo " $headerFile \\" >> $CLASS_DEPENDENCIES_FILE
	fi
done

echo >> $CLASS_DEPENDENCIES_FILE
rm -f $CLASS_DEPENDENCIES_FILE-e

isFirstMethod=
firstMethodLine=-1

methodDeclarations=`sed -e 's#^[ 	][ 	]*\(virtual\)[ 	][ 	]*\(.*\)#\2<\1>#;s#^[ 	][ 	]*\(override\)[ 	][ 	]*\(.*$\)#\2<\1>#;s#^[ 	][ 	]*\(static\)[ 	][ 	]*\(.*$\)#\2<\1>#' <<< "$methods"`

virtualMethodDeclarations=$virtualMethodDeclarations" "`grep -e "<virtual>" <<< "$methodDeclarations" | sed -e 's/\(^.*\)[ 	][ 	]*\([a-z][A-z0-9]*\)(\([^;]*;\)<virtual>.*/ __VIRTUAL_DEC(ClassName,\1,\2,\3/g' | sed -e 's/,[ 	]*)[ 	]*;/);/g' | tr -d "\r\n"`
virtualMethodOverrides=$virtualMethodOverrides" "`grep -e "<override>\|<virtual>" <<< "$methodDeclarations" | grep -v -e ")[ 	]*=[ 	]*0[ 	]*;" | sed -e 's/^.*[ 	][ 	]*\([a-z][A-z0-9]*\)(.*/ __VIRTUAL_SET(ClassName,'"$className"',\1);/g' | tr -d "\r\n"`
virtualMethodNames=`grep -e "<virtual>" <<< "$methodDeclarations" | sed -e 's/^.*[ 	][ 	]*\([a-z][A-z0-9]*\)(.*$/\1/g' | sed -e 's/,[ 	]*)[ 	]*;/);/g'`

methodCalls=`grep -v -e "<static>\|<virtual>\|<override>" <<< "$methodDeclarations" | sed -e 's/^.*[ 	][ 	]*\([a-z][A-z0-9]*\)(.*$/'"$className"'_\1/g'`

# Clean up method declarations
virtualMethodDeclarations=`sed -e 's/)[ 	]*=[ 	]*0[ 	]*;/);/g' -e 's/,[ 	]*)[ 	]*;/);/g' <<< "$virtualMethodDeclarations"`
methodDeclarations=`sed -e 's/)[ 	]*=[ 	]*0[ 	]*;/);/g' <<< "$methodDeclarations"`
methodDeclarations=`sed -e 's/\(^.*[ 	][ 	]*\)\([a-z][A-z0-9]*\)(\(.*\)/\1'"$className"'_\2(void* _this,\3/g' <<< "$methodDeclarations"`
methodDeclarations=`sed -e 's/\(^.*\)void\* _this,\(.*\)<static>/\1\2/g' -e 's#<virtual>#	#;s#<override>#	#;s#<static>#	#' -e 's/,[ 	]*)[ 	]*;/);/g' <<< "$methodDeclarations"`

if [ ! -z "$virtualMethodNames" ];
then
	sed -e 's/\(^.*\)/'"$className"'_\1 __VIRTUAL_CALL('"$className"',\1,/g' <<< "$virtualMethodNames" >> $CLASS_VIRTUAL_METHODS_DICTIONARY
fi

if [ ! -z "$methodCalls" ];
then
	echo "$methodCalls" >> $CLASS_INHERITED_METHODS_DICTIONARY
fi

# Remove duplicates
awk '!x[$0]++' $CLASS_OWNED_METHODS_DICTIONARY > $CLASS_OWNED_METHODS_DICTIONARY.tmp
mv $CLASS_OWNED_METHODS_DICTIONARY.tmp $CLASS_OWNED_METHODS_DICTIONARY

grep -v -e "_constructor\|_destructor\|_new" $CLASS_OWNED_METHODS_DICTIONARY > $CLASS_OWNED_METHODS_DICTIONARY.tmp
mv $CLASS_OWNED_METHODS_DICTIONARY.tmp $CLASS_OWNED_METHODS_DICTIONARY

awk '!x[$0]++' $CLASS_VIRTUAL_METHODS_DICTIONARY > $CLASS_VIRTUAL_METHODS_DICTIONARY.tmp
mv $CLASS_VIRTUAL_METHODS_DICTIONARY.tmp $CLASS_VIRTUAL_METHODS_DICTIONARY

awk '!x[$0]++' $CLASS_INHERITED_METHODS_DICTIONARY > $CLASS_INHERITED_METHODS_DICTIONARY.tmp
mv $CLASS_INHERITED_METHODS_DICTIONARY.tmp $CLASS_INHERITED_METHODS_DICTIONARY


#echo methodDeclarations
#echo "$methodDeclarations"

while IFS= read -r classModifier;
do
    if [ -z "$classModifier" ];
    then
        continue;
    fi

	if [ -z "${classModifier##*abstract *}" ];
	then

		isAbstractClass=true
	fi

	if [ -z "${classModifier##*final *}" ];
	then

		isFinalClass=true;
	fi

	if [ -z "${classModifier##*singleton *}" ];
	then

		isSingletonClass=true
	fi

	if [ -z "${classModifier##*static *}" ];
	then

		isStaticClass=true
	fi

done <<< "$classModifiers"

# Add destructor declaration
if [ ! "$isStaticClass" = true ];
then
	methodDeclarations=$methodDeclarations"
	void "$className"_destructor(void* _this);"
fi

# Add allocator if it is not abstract nor a singleton class
if [ ! "$isAbstractClass" = true ] && [ ! "$isSingletonClass" = true ] && [ ! "$isStaticClass" = true ] ;
then

	constructor=`grep -m 1 -e "void[ 	]\+"$className"_constructor[ 	]*(.*);" <<< "$methodDeclarations"`
#	echo constructor $constructor
#	echo "Adding allocator based on constructor $constructor"

	if [ -z "$constructor" ];
	then
		echo "Error: no constructor defined for $className : $baseClassName in $methodDeclarations"
		exit 0
	else
#		echo "Added allocator"
#		echo constructor $constructor
#		parameters=`cut -d\( -f2 <<< $constructor | cut -d\) -f1 | cut -d, -f2- | sed -e 's#^.*this##'`
		parameters=`cut -d\( -f2 <<< $constructor | cut -d\) -f1 | cut -d, -f2- | sed -e 's#^.*this##'`
#		echo parameters $parameters
		allocator="	"$className" "$className"_new("$parameters");"
		methodDeclarations=$methodDeclarations"
$allocator"

	fi
fi

TEMPORAL_FILE=$WORKING_FOLDER/temporal.txt
touch $TEMPORAL_FILE

#echo "" > $TEMPORAL_FILE
if [ ! "$isStaticClass" = true ];
then
	echo "$virtualMethodDeclarations" >> $TEMPORAL_FILE
#	echo "" >> $TEMPORAL_FILE
	echo "$virtualMethodOverrides" >> $TEMPORAL_FILE
#	echo "" >> $TEMPORAL_FILE

	if [ ! "$isFinalClass" = true ];
	then
		if [ ! -z "$baseClassName" ];
		then
			attributes="#define "$className"_ATTRIBUTES "$baseClassName"_ATTRIBUTES $attributes"

			virtualMethodDeclarations=$virtualMethodDeclarations" "$baseClassName"_METHODS(ClassName) "
			virtualMethodOverrides=$virtualMethodOverrides" "$baseClassName"_SET_VTABLE(ClassName) "
		else
			attributes="#define "$className"_ATTRIBUTES $attributes"
		fi

		echo "$attributes" >> $TEMPORAL_FILE
	fi
fi

#echo "" >> $TEMPORAL_FILE
if [ ! "$isStaticClass" = true ];
then
	echo "__CLASS($className);" >> $TEMPORAL_FILE
fi

#echo "" >> $TEMPORAL_FILE

# Adjust the output in order to make the methods to appear in the same lines as in the original header file
if [ ! -z "$firstMethodLine" ];
then
	if [ "$firstMethodLine" -gt 0 ];
	then
		methodsTargetLine=`wc -l < $TEMPORAL_FILE`
		methodsTargetLine=$((methodsTargetLine + line))

		if [ "$firstMethodLine" -gt "$methodsTargetLine" ];
		then
			paddingLines=$((firstMethodLine - methodsTargetLine))
			while [ "$paddingLines" -gt "0" ];
			do
				echo >> $TEMPORAL_FILE
				paddingLines=$((paddingLines - 1))
			done
		fi
	fi
fi

sed -e 's#static[ 	]\+##g' <<< "$methodDeclarations" >> $TEMPORAL_FILE
#echo >> $TEMPORAL_FILE

prelude=$((line - 2))
totalLines=`wc -l < $INPUT_FILE`
remaining=$((totalLines - end + 1))

head -${prelude} $INPUT_FILE > $OUTPUT_FILE
cat $TEMPORAL_FILE >> $OUTPUT_FILE
tail -${remaining} $INPUT_FILE >> $OUTPUT_FILE

# Clean up
sed -i -e 's#^[ 	]*class[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*;#__FORWARD_CLASS(\1);#' $OUTPUT_FILE
sed -i -e 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g' $OUTPUT_FILE
sed -i -e 's/static[ 	]inline[ 	]/inline /g' $OUTPUT_FILE
sed -i -e 's/inline[ 	]static[ 	]/inline /g' $OUTPUT_FILE
sed -i -e '/^[[:space:]]*$/d' $CLASSES_HIERARCHY_FILE

rm -f $TEMPORAL_FILE
rm -f $OUTPUT_FILE"-e"

