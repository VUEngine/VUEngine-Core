#!/bin/bash
#
clean_up() {
	# clean up
	sed -i -e 's/<%>//g' $OUTPUT_FILE
	sed -i -e 's/<[%]*DECLARATION>[ 	]*static[ 	][ 	]*/ /g' $OUTPUT_FILE
	sed -i -e 's/<[%]*DECLARATION>//g' $OUTPUT_FILE
	sed -i -e 's/!DECLARATION_MIDDLE!//g' $OUTPUT_FILE
	sed -i -e 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g' $OUTPUT_FILE
	sed -i -e 's/<START_BLOCK>//g' $OUTPUT_FILE
	sed -i -e 's/,<Â·>/,\'$'\n/g' $OUTPUT_FILE
	

	# Replace casts
    sed -i -e 's/\([A-Z][A-z0-9]*\)_safeCast[ 	]*(/__SAFE_CAST(\1, /g' $OUTPUT_FILE
}

INPUT_FILE=
OUTPUT_FILE=
WORKING_FOLDER=build/preprocessor
HELPER_FILES_PREFIXES=
PRINT_DEBUG_OUTPUT=
CLASSES_HIERARCHY_FILE=$WORKING_FOLDER/hierarchies/classesHierarchy.txt

while [ $# -gt 0 ]
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
		-c)
		CLASSES_HIERARCHY_FILE="$2"
		shift # past argument
		;;
		-d)
		PRINT_DEBUG_OUTPUT="true"
		;;
		-p|-output)
		;;
		*)
		HELPER_FILES_PREFIXES="$HELPER_FILES_PREFIXES $1"
		;;
	esac

	shift
done

#echo HELPER_FILES_PREFIXES $HELPER_FILES_PREFIXES
#echo WORKING_FOLDER $WORKING_FOLDER
#echo INPUT_FILE $INPUT_FILE
#echo OUTPUT_FILE $OUTPUT_FILE
cp $INPUT_FILE $OUTPUT_FILE

if [ ! -f "$INPUT_FILE" ];
then
	echo "File not found: $INPUT_FILE"
	exit 0
fi

if [ -z "$INPUT_FILE" ] || [ -z "${INPUT_FILE##*assets/*}" ];
then
	echo $INPUT_FILE | sed -e 's#^.*assets/\(.*$\)#Compiling asset: \1#g'
	exit 0
fi

className=`grep -m 1 -e '^.*::[ 	]*constructor[ 	]*(' $OUTPUT_FILE | sed -e 's#^.*[ 	][ 	]*\([A-Z][A-z0-9]*\)::.*#\1#'`
isStatic=false

if [ -z "$className" ];
then
# Maybe it is a static class
	isStatic=false
	className=`grep -o -m 1 -e '^.*[ 	][ 	]*[A-Z][A-z0-9]*[ 	]*::[ 	]*[a-z][A-z0-9]*[ 	]*(' $OUTPUT_FILE | sed -e 's/^.*[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*::.*/\1/'`
fi

# Prepare file by adding a blank space in front of each Class::method pattern
sed -i -e 's/\([A-z][A-z0-9]*::[a-z][A-z0-9]*\)/ \1/g' $OUTPUT_FILE

mark="@N@"
# Mark block starters
sed -i -e 's/{/{<START_BLOCK>/g' $OUTPUT_FILE

# Inline multiline declarations
sed -i -e 's/,[ 	]*$/,<Â·>/g' $OUTPUT_FILE
awk '{if ($0 ~ "<Â·>") printf "%s ", $0; else print;}' $OUTPUT_FILE > $OUTPUT_FILE.tmp && mv -f $OUTPUT_FILE.tmp $OUTPUT_FILE

# Identify static declarations
sed -i -e 's/.*static.*/&<%>/g' $OUTPUT_FILE
echo >> $OUTPUT_FILE

# Find method declarations
sed -e 's/.*/'"$mark"'&/g' $OUTPUT_FILE | tr -d "\r\n" | sed -e 's/'"$mark"'\([ 	]*[A-z0-9_ 	]*[A-z0-9_\*][A-z0-9_\*]*[ 	][ 	]*'"$className"'\)[ 	]*::\([ 	]*[a-z][A-z0-9]*[ 	]*([^{}]*{[ 	]*<START_BLOCK>\)/'"$mark"'<DECLARATION>\1!DECLARATION_MIDDLE!_\2<%DECLARATION>/g' > $OUTPUT_FILE.tmp && mv -f $OUTPUT_FILE.tmp $OUTPUT_FILE

# Add static qualifier to static methods block start
sed  -i -e 's/\(<DECLARATION>[^<]*\)<%>\([^{]*\)@N@{/\1@N@\2<%>{/g' $OUTPUT_FILE

# Inject _this parameter
sed -i -e 's/\(!DECLARATION_MIDDLE!_[^(]*\)(\([^%{]*{\)/\1(void* _this '"__attribute__ ((unused))"', \2/g' $OUTPUT_FILE

# Clean methods with no parameters declarations
sed -i -e 's/,[ 	]*)/)/g' $OUTPUT_FILE

# Put back line breaks
sed  -e 's/'"$mark"'/\'$'\n/g' $OUTPUT_FILE > $OUTPUT_FILE.tmp

referencedClassesNames=`grep -v -e '<DECLARATION>' $OUTPUT_FILE.tmp | grep "::" | sed -e 's/\([A-Z][A-z0-9]*::\)/<\1>\'$'\n/g' | grep "::>" | sed -e 's/.*<\([A-Z][A-z0-9]*\)::>/\1/g' | sort -u`
rm -f $OUTPUT_FILE.tmp

# Replace :: by _
sed -i -e 's#\([A-Z][A-z0-9]*\)::\([a-z][A-z0-9]*\)#\1_\2#g' $OUTPUT_FILE

prototypes=`sed -e 's/<DECLARATION>/\'$'\n<DECLARATION>/g' $OUTPUT_FILE | sed -e 's/<%DECLARATION>/<%DECLARATION>\'$'\n/g' | grep "DECLARATION>" | sed -e 's/<[%]*DECLARATION>//g' | sed -e 's/{<START_BLOCK>/;/g' |sed  -e 's/'"$mark"'//g' |sed  -e 's/<%>//g' | tr -d "\r\n" | sed -e 's/\([^A-z0-9]*\)static[ 	]/\1 /g'`

# Put back line breaks
sed  -i -e 's/'"$mark"'/\'$'\n/g'  $OUTPUT_FILE

# Clean up empty new line added at the start of file
tail -n +2 $OUTPUT_FILE > $OUTPUT_FILE.tmp
mv $OUTPUT_FILE.tmp $OUTPUT_FILE

# Inject this pointer
sed -i -e 's/<%>[ 	]*{[ 	]*<START_BLOCK>/{/g' $OUTPUT_FILE
sed -i -e 's/{[ 	]*<START_BLOCK>\(.*\)<%DECLARATION>/{'"$className"' this '"__attribute__ ((unused))"' = __SAFE_CAST('"$className"' , _this);\1/g' $OUTPUT_FILE

firstMethodDeclarationLine=`grep -m1 -n -e "^<DECLARATION>" $OUTPUT_FILE | cut -d ":" -f1`

if [ ! -s $OUTPUT_FILE ];
then
	echo "Error (1) processing file: $OUTPUT_FILE"
	exit 0
fi

#echo "prototypes $prototypes"
#echo "firstMethodDeclarationLine $firstMethodDeclarationLine"

if [ -z "$className" ];
then
	clean_up
	echo $INPUT_FILE | sed -e 's#^.*source[s]*/\(.*$\)#Compiling file: \1#g'
	exit 0
fi

if [ ! -s $OUTPUT_FILE ];
then
	echo "Error (2) processing file: $OUTPUT_FILE"
	exit 0
fi

if [ ! -f "$CLASSES_HIERARCHY_FILE" ];
then
	clean_up
	echo $INPUT_FILE | sed -e 's#^.*source[s]*/\(.*$\)#Compiling file: \1#g'
	exit 0
fi

baseClassName=`grep -m1 -e "^$className:" $CLASSES_HIERARCHY_FILE | cut -d ":" -f2`
if [ -z "$baseClassName" ];
then
	clean_up
	echo $INPUT_FILE | sed -e 's#^.*source[s]*/\(.*$\)#Compiling file: \1#g'
	exit 0
fi

if [ -z "$INPUT_FILE" ] || [ ! -z "${INPUT_FILE##*source/*}" ];
then
	exit 0
fi

# INJECTION OF ClassName _this into method declarations
echo "Compiling class: $className"

if [ ! -d $WORKING_FOLDER ];
then
	mkdir -p $WORKING_FOLDER
fi

# Move declaration mark to the end in preparation for virtual method call substitutions
sed -i -e 's/<DECLARATION>.*/&<DECLARATION>/g' $OUTPUT_FILE

anyMethodVirtualized=false

#echo HELPER_FILES_PREFIXES $HELPER_FILES_PREFIXES
# Replace calls to base class methods
CLASS_OWNED_METHODS_DICTIONARY=$WORKING_FOLDER/dictionaries/$className"MethodsOwned.txt"
classHasOwnMethods=`cat $CLASS_OWNED_METHODS_DICTIONARY`
if [ ! -z "$classHasOwnMethods" ];
then
	awk -f $VBDE/libs/vuengine/core/lib/compiler/preprocessor/normalMethodTraduction.awk $CLASS_OWNED_METHODS_DICTIONARY $OUTPUT_FILE > $OUTPUT_FILE.tmp
	mv $OUTPUT_FILE.tmp $OUTPUT_FILE
fi

VIRTUAL_METHODS_FILE=$WORKING_FOLDER/dictionaries/$className"MethodsVirtualToApply.txt"
if [ -f $VIRTUAL_METHODS_FILE ];
then
	rm -f $VIRTUAL_METHODS_FILE
fi

# Generate a dictionary of all virtual methods to replace on file
for referencedClassName in $referencedClassesNames
do
	REFERENCED_CLASS_VIRTUAL_METHODS_FILE=$WORKING_FOLDER/dictionaries/$referencedClassName"MethodsVirtual.txt"

	if [ ! -f "$REFERENCED_CLASS_VIRTUAL_METHODS_FILE" ];
	then
		continue;
	fi

	cat $REFERENCED_CLASS_VIRTUAL_METHODS_FILE >> $VIRTUAL_METHODS_FILE
done

if [ -f $VIRTUAL_METHODS_FILE ];
then

	classHasOwnMethods=`cat $VIRTUAL_METHODS_FILE`
	if [ ! -z "$classHasOwnMethods" ];
	then
		awk -f $VBDE/libs/vuengine/core/lib/compiler/preprocessor/virtualMethodTraduction.awk $VIRTUAL_METHODS_FILE $OUTPUT_FILE > $OUTPUT_FILE.tmp
		mv $OUTPUT_FILE.tmp $OUTPUT_FILE
	fi

	rm -f $VIRTUAL_METHODS_FILE
fi

if [ ! -s $OUTPUT_FILE ];
then
	echo "Error (3) processing file: $OUTPUT_FILE"
	exit 0
fi

# clean up
sed -i -e 's/<%>//g' $OUTPUT_FILE
sed -i -e 's/<[%]*DECLARATION>[ 	]*static[ 	][ 	]*/ /g' $OUTPUT_FILE
sed -i -e 's/<[%]*DECLARATION>//g' $OUTPUT_FILE
sed -i -e 's/<START_BLOCK>//g' $OUTPUT_FILE


classModifiers=`grep -m1 -e "^$className:" $CLASSES_HIERARCHY_FILE | cut -d ":" -f3`

if [ -z $classModifiers ];
then
	classModifiers="normal"
fi

if [ ! -z "${classModifiers##*static *}" ] ;
then

	classDefinition="__CLASS_DEFINITION($className, $baseClassName); $prototypes"

	# Add allocator if it is not abstract nor a singleton class
	if [ ! -z "${classModifiers##*singleton *}" ] && [ ! -z "${classModifiers##*static *}" ] && [ ! -z "${classModifiers##*abstract *}" ];
	then
	#	echo "Adding allocator"
		constructor=`grep -m 1 -e $className"!DECLARATION_MIDDLE!_constructor[ 	]*(.*)" $OUTPUT_FILE`
		constructorParameters=`sed -e 's#^.*(\(.*\))[ 	{]*$#\1#' <<< "$constructor"`
		#echo "constructorParameters $constructorParameters"
		allocatorParameters=`cut -d "," -f2- <<< "$constructorParameters,"`
		#echo "allocatorParameters $allocatorParameters"
		allocatorArguments=`sed -e 's#[ 	*][ 	*]*\([A-z0-9][A-z0-9]*[ 	]*,\)#<\1>\'$'\n#g' <<< "$allocatorParameters"| sed -e 's#.*<\(.*\)>.*#\1#g' | tr -d "\r\n" | sed -e 's#\(.*\),#\1#'`
		allocatorParameters=`sed -e 's#\(.*\),#\1#' <<< "$allocatorParameters"`

		if [ -z "$allocatorParameters" ];then
			classDefinition=$classDefinition"__CLASS_NEW_DEFINITION($className)"
			classDefinition=$classDefinition"__CLASS_NEW_END($className);"
		else
			classDefinition=$classDefinition"__CLASS_NEW_DEFINITION($className, $allocatorParameters)"
			classDefinition=$classDefinition"__CLASS_NEW_END($className, $allocatorArguments);"
		fi
	else
		if [ -z "${classModifiers##*singleton *}" ];
		then
			customSingletonDefinition=`grep -o -e '#define[ 	][ 	]*.*SINGLETON.*(' $OUTPUT_FILE`

			if [ -z "$customSingletonDefinition" ];
			then
				if [ -z "${classModifiers##*dynamic_singleton *}" ];
				then
					classDefinition=$classDefinition"__SINGLETON_DYNAMIC($className);"
				else
					classDefinition=$classDefinition"__SINGLETON($className);"
				fi
			else
				customSingletonDefinition=`sed -e 's@^.*[ 	][ 	]*\(.*SINGLETON.*\)(@\1@' <<< $customSingletonDefinition`
				classDefinition=$classDefinition"$customSingletonDefinition($className);"
			fi

			sed -i -e "s/Base_destructor();/_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED; Base_destructor();/" $OUTPUT_FILE

		fi
	fi
else
	classDefinition="$prototypes"
fi

if [ ! -s $OUTPUT_FILE ];
then
	echo "Error (4) processing file: $OUTPUT_FILE"
	exit 0
fi

#if [ -z "$classModifiers" ];
#then
#	echo "$className inherits from $baseClassName"
#else
#	echo "$className inherits from $baseClassName (is $classModifiers)"
#fi

if [ ! -z "$firstMethodDeclarationLine" ];
then

	firstMethodDeclarationLine=$((firstMethodDeclarationLine))
	orig=$'\n'; replace=$'\\\n'
	sed -i -e "${firstMethodDeclarationLine}s@.*@${classDefinition//$orig/$replace};&@" $OUTPUT_FILE
#	sed -i -e 's/<$>/\'$'\n/g' $OUTPUT_FILE
fi

sed -i -e 's#[ 	]*friend[ 	][ 	]*class[ 	][ 	]*\([A-z0-9][A-z0-9]*\)#__CLASS_FRIEND_DEFINITION(\1)#' $OUTPUT_FILE

# replace base method calls
sed -i -e "s#Base_constructor(\(.*\)#__CONSTRUCT_BASE($baseClassName,\1#g" $OUTPUT_FILE
sed -i -e 's#,[ 	]*);#);#' $OUTPUT_FILE
sed -i -e "s#Base_destructor()#__DESTROY_BASE#g" $OUTPUT_FILE
sed -i -e "s#Base_\([A-z][A-z0-0][A-z0-0]*\)(#__CALL_BASE_METHOD($baseClassName,\1, #g" $OUTPUT_FILE

clean_up

# Replace news and deletes
sed -i -e "s/\([^A-z0-9]\)new[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*(/\1\2_new(/g" $OUTPUT_FILE
sed -i -e "s/\([^A-z0-9]\)delete[ 	][ 	]*\(.*\);/\1__DELETE(\2);/g"  $OUTPUT_FILE
sed -i -e "s/\([^A-z0-9]\)new[ 	][ 	]*\([A-Z][A-z0-9]*\)[ 	]*;/\1__NEW_BASIC(\2);/g" $OUTPUT_FILE


if [ $PRINT_DEBUG_OUTPUT ] && [ "$anyMethodVirtualized" = true ];
then
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "*****************************************************************************************************" >> $WORKING_FOLDER/virtualizations.txt
	echo "FILE: $INPUT_FILE" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	grep -n CALL_BASE_METHOD $OUTPUT_FILE | sed -e "s#\([0-9][0-9]*:\).*\(__CALL_BASE_METHOD(.*\)#\1	\2#g" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
	grep -n VIRTUAL_CALL $OUTPUT_FILE | sed -e "s#\([0-9][0-9]*:\).*\(__VIRTUAL_CALL(.*\)#\1	\2#g" >> $WORKING_FOLDER/virtualizations.txt
	echo "" >> $WORKING_FOLDER/virtualizations.txt
fi

if [ ! -s $OUTPUT_FILE ];
then
	echo "Error (5) processing file: $OUTPUT_FILE"
	exit 0
fi
