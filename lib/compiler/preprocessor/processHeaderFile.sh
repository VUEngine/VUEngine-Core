#!/bin/bash
#

function waitRandom()
{
	delay=$PREPROCESSING_WAIT_FOR_LOCK_DELAY_FACTOR$(( ( ( RANDOM % 900 ) + 10 ) ))
	sleep $delay
}

function waitForLockToRelease()
{
	lockFolder=$1".lock"

	counter=0
	while [ -d $lockFolder ];
	do
		counter=$((counter + 1))

		if [ "$counter" -gt 100 ];
		then
			counter=0
			echo "Waiting on caller $CALLER for:"  >> $CLASS_LOG_FILE
			sed -e 's#.*\(/[A-z][A-z0-9]*\.*\)#		\1#g' <<< $lockFolder  >> $CLASS_LOG_FILE
		fi

		waitRandom
	done
}

function tryToLock()
{
	file=$1
	command=$2
	#echo Trying to lock $file

	if [ ! -z "$file" ];
	then
		lockFolder=$1".lock"
		lockFile=$lockFolder/stamp.txt

		waitRandom

		echo "Trying to lock on $file on caller $CALLER" >> $CLASS_LOG_FILE
		mkdir $lockFolder 2>/dev/null ||
		{
			PID=`grep Stamp $lockFile 2>/dev/null | cut -d : -f 2`
#			echo PID $PID
			if [ ! -z "$PID" ] && [ ! kill -0 $PID 2>/dev/null ];
			then
				echo "Removing stale lock of nonexistent PID ${PID} for $file" >> $CLASS_LOG_FILE
#				echo "Removing stale lock of nonexistent PID ${PID} for $file"
				rm -f $lockFile
				waitRandom
				rm -Rf $lockFolder

				tryToLock $file
				return
			else
				waitForLockToRelease $file
			fi

			if [ ! -z "$command" ] && [ -z "${command##hierarchy}" ];
			then
				echo "Waiting lock on hierarchy file with command $command on caller $CALLER" >> $CLASS_LOG_FILE
			fi

			if [ ! -z "$command" ] && [ -z "${command##exit}" ];
			then
				clean_up
				echo "Gived up with command $command on caller $CALLER" >> $CLASS_LOG_FILE
				exit 0
			fi

			tryToLock $file
			return
		}

		echo "Succeeded to lock $file on caller $CALLER" >> $CLASS_LOG_FILE
		stamp="Stamp $$ : $PPID : $UID"
		echo $stamp > $lockFile
		echo "Locked by $INPUT_FILE" >> $lockFile
		echo "Caller $CALLER" >> $lockFile

		readStamp=`grep Stamp $lockFile`

		if [ ! "$readStamp" = "$stamp" ];
		then
			echo "$className: Error on reading $file read stamp ($readStamp) doesn't match my stamp ($stamp) on caller $CALLER"
			cat $lockFile
			echo
		fi

	fi
}

function releaseLock()
{
	file=$1

	if [ ! -z "$file" ];
	then
		lockFolder=$1".lock"

		if [ -d "$lockFolder" ];
		then
			stamp="Stamp $$ : $PPID : $UID"
			lockFile=$lockFolder/stamp.txt

			readStamp=`grep Stamp $lockFile`

			if [ ! "$readStamp" = "$stamp" ];
			then
				echo "Error on unlocking $file read stamp ($readStamp) doesn't match my stamp ($stamp) on caller $CALLER" >> $CLASS_LOG_FILE
				echo "$className: Error on unlocking $file read stamp ($readStamp) doesn't match my stamp ($stamp) on caller $CALLER"
			fi

			rm -f $lockFile
			waitRandom
			rm -Rf $lockFolder
			echo "Released lock $file on caller $CALLER" >> $CLASS_LOG_FILE
		else
			echo "Cannot release lock on $file, doesn't exist the folder $lockFolder" >> $CLASS_LOG_FILE
		fi
	fi
}

function clean_up()
{
	rm -f $TEMPORAL_FILE
	rm -f $OUTPUT_FILE"-e"
	#rm -f $CLASS_LOG_FILE
}

function releaseLocks()
{
	releaseLock $CLASS_LOCK
}

INPUT_FILE=
OUTPUT_FILE=
WORKING_FOLDER=
PRINT_DEBUG_OUTPUT=
CLASSES_HIERARCHY_FILE=
HEADERS_FOLDER=
LIBRARY_NAME=
PLUGINS=
LIBRARIES_PAHT=
LIBRARIES_ARGUMENT=

while [ $# -gt 1 ]
do
	key="$1"
	case $key in
		-e)
		ENGINE_HOME="$2"
		shift # past argument
		;;
		-g)
		CALLER="$2"
		shift # past argument
		;;
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
		-p)
		LIBRARIES_PATH="$2"
		shift # past argument
		;;
		-u)
		USER_LIBRARIES_PATH="$2"
		shift # past argument
		;;
		-l)
		PLUGINS=`sed -e 's/:/ /g' <<< "$2"`
		LIBRARIES_ARGUMENT="$2"
		shift # past argument
		;;
		-t)
		PREPROCESSING_WAIT_FOR_LOCK_DELAY_FACTOR="$2"
		shift # past argument
		;;
	esac

	shift
done

if [ -z "$CALLER" ];
then
	echo "NO CALLER!!!!"
fi

if [ -z "$INPUT_FILE" ] || [ ! -f "$INPUT_FILE" ];
then
	echo "Input file not found: $INPUT_FILE"
	clean_up
	exit 1
fi

if [ "$INPUT_FILE" = "$OUTPUT_FILE" ];
then
	echo "Input and output files are the same: $INPUT_FILE"
	clean_up
#	echo "$INPUT_FILE"
#	echo "$OUTPUT_FILE"
	exit 0
fi

classDeclaration=`grep -n -e "^[ 	]*[A-z0-9]*[ 	]*class[ 	]\+[A-Z][A-z0-9]*[ 	]*:[ 	]*[A-Z][A-z0-9]*" $INPUT_FILE`
cleanClassDeclaration=`cut -d: -f2,3 <<< "$classDeclaration"`
className=`sed -e 's#^.*class \([A-z][A-z0-9]*\)[ 	]*\:.*#\1#' <<< "$cleanClassDeclaration"`
baseClassName=`cut -d: -f2 <<< "$cleanClassDeclaration" | sed -e 's/[^[:alnum:]_-]//g'`

if [ -z "$className" ];
then
	cp -f $INPUT_FILE $OUTPUT_FILE
	clean_up
#	echo No class in $INPUT_FILE
	exit 0
fi

#echo "Starting preprocessing of $className" >> $WORKING_FOLDER/traces/preprocessing.txt

if [ ! -d "$WORKING_FOLDER/classes/logs" ];
then
	mkdir -p $WORKING_FOLDER/classes/logs
fi

if [ ! -d "$WORKING_FOLDER/classes/locks" ];
then
	mkdir -p $WORKING_FOLDER/classes/locks
fi

CLASS_LOG_FILE="$WORKING_FOLDER/classes/logs/$className.log"

# This handles a race condition between a call from the makefile and from this below in this script
CLASS_LOCK="$WORKING_FOLDER/classes/locks/$className"
tryToLock $CLASS_LOCK exit
echo "Got lock on calling from $CALLER" > $CLASS_LOG_FILE
echo "INPUT_FILE $INPUT_FILE" >> $CLASS_LOG_FILE
echo "OUTPUT_FILE $OUTPUT_FILE" >> $CLASS_LOG_FILE

DEPENDENCIES_FILE=$WORKING_FOLDER/classes/dependencies/$LIBRARY_NAME/$className".d"
if [ -f "$DEPENDENCIES_FILE" ];
then
	DEPENDENCIES=`cat $DEPENDENCIES_FILE | sed -e 's/[\\:]//g' | tail -n +2 `

	mustBeReprocessed=false
	for dependency in $DEPENDENCIES;
	do
		if [ "$dependency" -nt "$OUTPUT_FILE" ];
		then
			mustBeReprocessed=true
			break;
		fi
	done

	if [ -z "${mustBeReprocessed##false}" ] && [ "$OUTPUT_FILE" -nt "$INPUT_FILE" ];
	then
		clean_up
		releaseLocks
		echo "Already processed on caller $CALLER" >> $CLASS_LOG_FILE
		exit 0
	fi
fi

echo "Will check if base class $baseClassName needs to be processed on caller $CALLER" >> $CLASS_LOG_FILE

mustBeReprocessed=false
baseClassFile=

# Call upwards
if [ ! -z "${className##Object}" ];
then
	baseClassFile=`find $HEADERS_FOLDER/source -name "$baseClassName.h" -print -quit`
	processedBaseClassFile=`sed -e 's#.*/source/#'"$WORKING_FOLDER"'/objects/'"$LIBRARY_NAME"'/source/#g' <<< "$baseClassFile"`

	# Call upwards if base class belongs to plugin
	if [ -f "$baseClassFile" ];
	then
#		echo baseClassFile $baseClassFile
#		echo processedBaseClassFile $processedBaseClassFile

		if [ -f "$processedBaseClassFile" ] && [ "$processedBaseClassFile" -nt "$OUTPUT_FILE" ];
		then
			mustBeReprocessed=true
		else
			baseClassLock=$WORKING_FOLDER/classes/locks/$baseClassName".lock"

			if [ ! -d "$baseClassLock" ];
			then
				echo "$baseClassName needs preprocessing, calling it" >> $CLASS_LOG_FILE
#				echo "$baseClassName needs preprocessing, calling it"
#				echo "$baseClassName file $baseClassFile"
#				echo "$baseClassName processedBaseClassFile $processedBaseClassFile"
				
				bash $ENGINE_HOME/lib/compiler/preprocessor/processHeaderFile.sh -e $ENGINE_HOME -i $baseClassFile -o $processedBaseClassFile -w $WORKING_FOLDER -c $CLASSES_HIERARCHY_FILE -n $LIBRARY_NAME -h $HEADERS_FOLDER -p $LIBRARIES_PATH -u $USER_LIBRARIES_PATH -g $className -t $PREPROCESSING_WAIT_FOR_LOCK_DELAY_FACTOR -l "$LIBRARIES_ARGUMENT"
			else
				mustBeReprocessed=true
			fi
		fi
	fi

	if [ ! -z "${mustBeReprocessed##true}" ];
	then
		if [ ! -f "$processedBaseClassFile" ];
		then
			processedBaseClassFile=`find $WORKING_FOLDER/objects -name "$baseClassName.h" -print -quit`
		fi

		if [ -f "$processedBaseClassFile" ] && [ "$processedBaseClassFile" -nt "$OUTPUT_FILE" ];
		then
			mustBeReprocessed=true
		fi
	fi
fi


if [ -z "${mustBeReprocessed##false}" ] && [ -f "$OUTPUT_FILE" ] && [ "$OUTPUT_FILE" -nt "$INPUT_FILE" ];
then
#	ls -l $OUTPUT_FILE
#	ls -l $INPUT_FILE
#	echo Up to date "$className"
	clean_up
	releaseLocks
	echo "Don't need processing, base class is fine, and I'm newer on caller $CALLER"  >> $CLASS_LOG_FILE
	exit 0
fi

# The continue
#echo "Preprocessing class: $className"
#echo
#echo "$className PLUGINS $PLUGINS"
#echo "$className LIBRARIES_ARGUMENT $LIBRARIES_ARGUMENT"
echo "Starting preprocessing" >> $CLASS_LOG_FILE

classModifiers=`sed -e 's#^\(.*\)class .*#\1#' <<< "$cleanClassDeclaration"`
line=`cut -d: -f1 <<< "$classDeclaration"`

#echo "classDeclaration: $classDeclaration"
#echo "cleanClassDeclaration: $cleanClassDeclaration"
#echo "line: $line"
#echo "Modifiers: $classModifiers"

# Compute the class' whole hierarchy
baseClassesNamesHelper=$baseClassName":"

if [ ! -z "${className##Object}" ];
then

	if [ ! -z "$baseClassName" ];
	then
		processedBaseClassFile=`find $WORKING_FOLDER/objects -name "$baseClassName.h" -print -quit`
		baseClassLock=$WORKING_FOLDER/classes/locks/$baseClassName".lock"
		counter=0

		while [ -z "$processedBaseClassFile" ] || [ ! -f "$processedBaseClassFile" ] || [ -d "$baseClassLock" ];
		do
			if [ "$counter" -gt 200 ];
			then
				counter=0
				echo "Waiting for $baseClassName during computation of whole hierarchy"  >> $CLASS_LOG_FILE
			fi
			waitRandom
			processedBaseClassFile=`find $WORKING_FOLDER/objects -name "$baseClassName.h" -print -quit`

			counter=$((counter + 1))

			if [ "$counter" -gt 199 ];
			then
				echo "Error processing $className while computing hierarchy on $baseClassName with file $processedBaseClassFile not found"  >> $CLASS_LOG_FILE
				echo "$className: base class $baseClassName not found"
				clean_up
				releaseLocks
				exit 1
			fi
		done
	fi

	baseClassesNames=$baseClassName
	baseBaseClassName=$baseClassName

	CLASSES_HIERARCHY_FILES=`find $WORKING_FOLDER/classes/hierarchies -type f -name "classesHierarchy.txt" -print 2> /dev/null`
	classesHierarchy=

	echo "Starting computation of whole hierarchy on caller $CALLER"  >> $CLASS_LOG_FILE

	for classesHierarchyFile in $CLASSES_HIERARCHY_FILES;
	do
		tryToLock $classesHierarchyFile
		classesHierarchy="$classesHierarchy
`cat $classesHierarchyFile`"
		releaseLock $classesHierarchyFile
	done

	while : ; do

		if [ -z "${baseBaseClassName##Object}" ];
		then
			break
		fi

		baseBaseClassNameLine=`grep -e "^$baseBaseClassName:.* | grep -v "extension <<< "$classesHierarchy"`

		if (set -f ; IFS=$'\n'; set -- x${baseBaseClassNameLine}x ; [ $# = 1 ]) ; 
		then
			baseBaseClassName=`echo $baseBaseClassNameLine | cut -d ":" -f 2`
			baseClassesNames="$baseBaseClassName $baseClassesNames"
			baseClassesNamesHelper=$baseClassesNamesHelper$baseBaseClassName":"
		else
			echo "ERROR: $className depends on $baseBaseClassName which has multiple definitions:"
			echo "$baseBaseClassNameLine"
			echo ""
			exit 1
		fi
	done
fi

echo "Hierarchy on caller $CALLER: $baseClassesNames"  >> $CLASS_LOG_FILE

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

echo "Computing attributes and methods on caller $CALLER"  >> $CLASS_LOG_FILE

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
isExtensionClass=false

if [ ! -z "$classModifiers" ] && [ -z "${classModifiers##*extension *}" ];
then
	isExtensionClass=true
fi

virtualMethodDeclarations="#define "$className"_METHODS(ClassName)"
virtualMethodOverrides="#define "$className"_SET_VTABLE(ClassName)"

if [ ! -z "$baseClassName" ];
then
	virtualMethodDeclarations=$virtualMethodDeclarations" "$baseClassName"_METHODS(ClassName) "
	virtualMethodOverrides=$virtualMethodOverrides" "$baseClassName"_SET_VTABLE(ClassName) "
fi

echo "Writing owned methods on caller $CALLER"  >> $CLASS_LOG_FILE

# Process each method to generate the final header
methodDeclarations=

CLASS_OWNED_METHODS_DICTIONARY=$WORKING_FOLDER/classes/dictionaries/$className"MethodsOwned.txt"

CLASS_INHERITED_METHODS_DICTIONARY=$WORKING_FOLDER/classes/dictionaries/$className"MethodsInherited.txt"

if [ ! "$isExtensionClass" = true ];
then
	# Create method dictionaries
	rm -f $CLASS_OWNED_METHODS_DICTIONARY
	echo "Writing inherited methods on caller $CALLER"  >> $CLASS_LOG_FILE
	rm -f $CLASS_INHERITED_METHODS_DICTIONARY
fi

touch $CLASS_OWNED_METHODS_DICTIONARY
touch $CLASS_INHERITED_METHODS_DICTIONARY

echo "Writing virtual methods on caller $CALLER"  >> $CLASS_LOG_FILE

CLASS_VIRTUAL_METHODS_DICTIONARY=$WORKING_FOLDER/classes/dictionaries/$className"MethodsVirtual.txt"
rm -f $CLASS_VIRTUAL_METHODS_DICTIONARY
touch $CLASS_VIRTUAL_METHODS_DICTIONARY

CLASS_DEPENDENCIES_FILE=$WORKING_FOLDER/classes/dependencies/$LIBRARY_NAME/$className".d"
#echo "$OUTPUT_FILE:" | sed -e 's@'"$WORKING_FOLDER"'/@@g' > $CLASS_DEPENDENCIES_FILE

# Build headers search path
searchPaths="$HEADERS_FOLDER/source $ENGINE_HOME/source "
for plugin in $PLUGINS;
do
	plugin=`echo $plugin | sed -r "s@(user//|vuengine//)@/@"`

	if [ -d "$LIBRARIES_PATH/$plugin" ]; 
	then
		searchPaths=$searchPaths" $LIBRARIES_PATH/$plugin/source"
	else
		searchPaths=$searchPaths" $USER_LIBRARIES_PATH/$plugin/source"
	fi

done

echo "Starting computation of dependcies on caller $CALLER with search path $searchPath "  >> $CLASS_LOG_FILE

if [ ! -z "$baseClassesNames" ];
then
	echo "$OUTPUT_FILE: \\" > $CLASS_DEPENDENCIES_FILE
fi

# Get base classes' methods
for ancestorClassName in $baseClassesNames;
do
	ancestorInheritedMethodsDictionary=$WORKING_FOLDER/classes/dictionaries/$ancestorClassName"MethodsInherited.txt"
	ancestorVirtualMethodsDictionary=$WORKING_FOLDER/classes/dictionaries/$ancestorClassName"MethodsVirtual.txt"

	if [ ! -z "${className##Object}" ];
	then
		ancestorLock=$WORKING_FOLDER/classes/locks/$ancestorClassName".lock"

		counter=0

		while [ -d "$ancestorLock" ] || [ ! -f "$ancestorVirtualMethodsDictionary" ];
		do
			counter=$((counter + 1))
			if [ "$counter" -gt 100 ];
			then
				counter=0
				echo "$className waiting (2) for $baseClassName"  >> $CLASS_LOG_FILE
			fi
			waitRandom
		done
	fi

	cat $ancestorInheritedMethodsDictionary | sed -e 's/^\([A-Z][A-z]*\)_\(.*\)/'"$className"'_\2 \1_\2/g' >> $CLASS_OWNED_METHODS_DICTIONARY
	cat $ancestorInheritedMethodsDictionary >> $CLASS_INHERITED_METHODS_DICTIONARY
	cat $ancestorVirtualMethodsDictionary | sed -e 's/'"$ancestorClassName"'/'"$className"'/g' >> $CLASS_VIRTUAL_METHODS_DICTIONARY
	headerFile=`find $searchPaths -name "$ancestorClassName.h" -print -quit`

	if [ -f "$headerFile" ];
	then
		##echo "."
		echo " $headerFile \\" >> $CLASS_DEPENDENCIES_FILE
	else

		echo "$className: header file not found for $ancestorClassName in $searchPaths with $PLUGINS "
		rm -f $CLASS_DEPENDENCIES_FILE
		rm -f $OUTPUT_FILE
		clean_up
		releaseLocks
		exit 1
	fi
done


if [ ! -z "$baseClassesNames" ];
then
	echo " $INPUT_FILE " >> $CLASS_DEPENDENCIES_FILE
fi

echo "Computation of dependcies done on caller $CALLER"  >> $CLASS_LOG_FILE

rm -f $CLASS_DEPENDENCIES_FILE-e

isFirstMethod=
firstMethodLine=-1

echo "Computing final header text on caller $CALLER"  >> $CLASS_LOG_FILE

#echo "."
methodDeclarations=`sed -e 's#^[ 	][ 	]*\(virtual\)[ 	][ 	]*\(.*\)#\2<\1>#;s#^[ 	][ 	]*\(override\)[ 	][ 	]*\(.*$\)#\2<\1>#;s#^[ 	][ 	]*\(static\)[ 	][ 	]*\(.*$\)#\2<\1>#' <<< "$methods"`

#echo "."
virtualMethodDeclarations=$virtualMethodDeclarations" "`grep -e "<virtual>" <<< "$methodDeclarations" | sed -e 's/\(^.*\)[ 	][ 	]*\([a-z][A-z0-9]*\)(\([^;]*;\)<virtual>.*/ __VIRTUAL_DEC(ClassName,\1,\2,\3/g' | sed -e 's/,[ 	]*)[ 	]*;/);/g' | tr -d "\r\n"`
#echo "."
virtualMethodOverrides=$virtualMethodOverrides" "`grep -e "<override>\|<virtual>" <<< "$methodDeclarations" | grep -v -e ")[ 	]*=[ 	]*0[ 	]*;" | sed -e 's/^.*[ 	][ 	]*\([a-z][A-z0-9]*\)(.*/ __VIRTUAL_SET(ClassName,'"$className"',\1);/g' | tr -d "\r\n"`
#echo "."
virtualMethodNames=`grep -e "<virtual>" <<< "$methodDeclarations" | sed -e 's/^.*[ 	][ 	]*\([a-z][A-z0-9]*\)(.*$/\1/g' | sed -e 's/,[ 	]*)[ 	]*;/);/g'`

#echo "."
methodCalls=`grep -v -e "<static>\|<virtual>\|<override>" <<< "$methodDeclarations" | sed -e 's/^.*[ 	][ 	]*\([a-z][A-z0-9]*\)(.*$/'"$className"'_\1/g'`

# Clean up method declarations
#echo "."
virtualMethodDeclarations=`sed -e 's/)[ 	]*=[ 	]*0[ 	]*;/);/g' -e 's/,[ 	]*)[ 	]*;/);/g' <<< "$virtualMethodDeclarations"`
#echo "."
methodDeclarations=`sed -e 's/)[ 	]*=[ 	]*0[ 	]*;/);/g' <<< "$methodDeclarations"`
#echo "."
methodDeclarations=`sed -e 's/\(^.*[ 	][ 	]*\)\([a-z][A-z0-9]*\)(\(.*\)/\1'"$className"'_\2(void* _this,\3/g' <<< "$methodDeclarations"`
#echo "."
methodDeclarations=`sed -e 's/\(^.*\)void\* _this,\(.*\)<static>/\1\2/g' -e 's#<virtual>#	#;s#<override>#	#;s#<static>#	#' -e 's/,[ 	]*)[ 	]*;/);/g' <<< "$methodDeclarations"`
#echo "."

if [ ! -z "$virtualMethodNames" ];
then
	sed -e 's/\(^.*\)/'"$className"'_\1 __VIRTUAL_CALL('"$className"',\1,/g' <<< "$virtualMethodNames" >> $CLASS_VIRTUAL_METHODS_DICTIONARY
fi

if [ ! -z "$methodCalls" ];
then
	echo "$methodCalls" >> $CLASS_INHERITED_METHODS_DICTIONARY
fi

echo "Cleaning owned methods dictionary on caller $CALLER"  >> $CLASS_LOG_FILE

# Remove duplicates
awk '!x[$0]++' $CLASS_OWNED_METHODS_DICTIONARY > $CLASS_OWNED_METHODS_DICTIONARY.tmp
mv $CLASS_OWNED_METHODS_DICTIONARY.tmp $CLASS_OWNED_METHODS_DICTIONARY

echo "Writing owned methods dictionary on caller $CALLER"  >> $CLASS_LOG_FILE

grep -v -e "_constructor\|_destructor\|_new" $CLASS_OWNED_METHODS_DICTIONARY > $CLASS_OWNED_METHODS_DICTIONARY.tmp
mv $CLASS_OWNED_METHODS_DICTIONARY.tmp $CLASS_OWNED_METHODS_DICTIONARY

echo "Writing virtual methods dictionary on caller $CALLER"  >> $CLASS_LOG_FILE

awk '!x[$0]++' $CLASS_VIRTUAL_METHODS_DICTIONARY > $CLASS_VIRTUAL_METHODS_DICTIONARY.tmp
mv $CLASS_VIRTUAL_METHODS_DICTIONARY.tmp $CLASS_VIRTUAL_METHODS_DICTIONARY

echo "Writing inherited methods dictionary on caller $CALLER"  >> $CLASS_LOG_FILE

awk '!x[$0]++' $CLASS_INHERITED_METHODS_DICTIONARY > $CLASS_INHERITED_METHODS_DICTIONARY.tmp
mv $CLASS_INHERITED_METHODS_DICTIONARY.tmp $CLASS_INHERITED_METHODS_DICTIONARY

#echo methodDeclarations
#echo "$methodDeclarations"
echo "Computing class modifiers on caller $CALLER"  >> $CLASS_LOG_FILE

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
if [ ! "$isStaticClass" = true ] && [ ! "$isExtensionClass" = true ];
then
	methodDeclarations=$methodDeclarations"
	void "$className"_destructor(void* _this);"
fi

echo "Computing constructor/destructor/allocators on caller $CALLER"  >> $CLASS_LOG_FILE

# Add allocator if it is not abstract nor a singleton class
if [ ! "$isAbstractClass" = true ] && [ ! "$isSingletonClass" = true ] && [ ! "$isStaticClass" = true ] && [ ! "$isExtensionClass" = true ] ;
then

	constructor=`grep -m 1 -e "void[ 	]\+"$className"_constructor[ 	]*(.*);" <<< "$methodDeclarations"`
#	echo constructor $constructor
#	echo "Adding allocator based on constructor $constructor"

	if [ -z "$constructor" ];
	then
		echo "$className: no constructor defined for $className : $baseClassName in $methodDeclarations"
		clean_up
		releaseLocks
		exit 1
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

echo "Writing temporal file on caller $CALLER"  >> $CLASS_LOG_FILE

TEMPORAL_FILE=$WORKING_FOLDER/$className"Temporal.txt"
#echo created $TEMPORAL_FILE
touch $TEMPORAL_FILE

#echo "" > $TEMPORAL_FILE
if [ ! "$isStaticClass" = true ] && [ ! "$isExtensionClass" = true ];
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
if [ "$isExtensionClass" = true ];
then
	echo "__FORWARD_CLASS($className);" >> $TEMPORAL_FILE
else
	if [ ! "$isStaticClass" = true ]
	then
		echo "__CLASS($className);" >> $TEMPORAL_FILE
	fi
fi

#echo "" >> $TEMPORAL_FILE
echo "Padding temporal file on caller $CALLER"  >> $CLASS_LOG_FILE

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

echo "Writing $OUTPUT_FILE file on caller $CALLER"  >> $CLASS_LOG_FILE

prelude=$((line - 2))
totalLines=`wc -l < $INPUT_FILE`
remaining=$((totalLines - end + 1))

head -${prelude} $INPUT_FILE > $OUTPUT_FILE
cat $TEMPORAL_FILE >> $OUTPUT_FILE
tail -${remaining} $INPUT_FILE >> $OUTPUT_FILE

# Clean up
#sed -i.b 's#^[ 	]*class[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*;#__FORWARD_CLASS(\1);#' $OUTPUT_FILE
#sed -i.b 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g' $OUTPUT_FILE
#sed -i.b 's/static[ 	]inline[ 	]/inline /g' $OUTPUT_FILE
#sed -i.b 's/inline[ 	]static[ 	]/inline /g' $OUTPUT_FILE
sed 's#^[ 	]*class[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*;#__FORWARD_CLASS(\1);#; s#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g; s/static[ 	]inline[ 	]/inline /g; s/inline[ 	]static[ 	]/inline /g' $OUTPUT_FILE > $OUTPUT_FILE.tmp && mv -f $OUTPUT_FILE.tmp $OUTPUT_FILE

echo "Writing class hierarchy on caller $CALLER"  >> $CLASS_LOG_FILE

tryToLock $CLASSES_HIERARCHY_FILE hierarchy
#tryToLock $CLASSES_HIERARCHY_FILE hierarchy
# save new hierarchy
touch $CLASSES_HIERARCHY_FILE
#sed -i.b "s#^$className:.*##g" $CLASSES_HIERARCHY_FILE
#sed -i.b '/^[[:space:]]*$/d' $CLASSES_HIERARCHY_FILE
sed "s#^$className:.*##g; /^[[:space:]]*$/d" $CLASSES_HIERARCHY_FILE > $CLASSES_HIERARCHY_FILE.tmp && mv -f $CLASSES_HIERARCHY_FILE.tmp $CLASSES_HIERARCHY_FILE
echo "$className:$baseClassesNamesHelper:$classModifiers" >> $CLASSES_HIERARCHY_FILE
# replace any previous entry
# Clean it
releaseLock $CLASSES_HIERARCHY_FILE
echo "Done on caller $CALLER"  >> $CLASS_LOG_FILE

clean_up

if [ ! "$isExtensionClass" = true ];
then
	echo "Preprocessed class: $className"
else
	echo "Preprocessed extension for: $className"
fi

releaseLocks

echo "Released locks on caller $CALLER"  >> $CLASS_LOG_FILE

#echo "Finished preprocessing of $className" >> $WORKING_FOLDER/traces/preprocessing.txt
