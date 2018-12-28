#!/bin/bash
#

INPUT_FILE=
OUTPUT_FILE=
WORKING_FOLDER=build/compiler/preprocessor
PRINT_DEBUG_OUTPUT=
HELPER_FILES_PREFIX=engine
CLASSES_HIERARCHY_FILE=$WORKING_FOLDER/classesHierarchy.txt

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
		-p)
		HELPER_FILES_PREFIX="$2"
		shift # past argument
		;;
		-d)
		PRINT_DEBUG_OUTPUT="true"
		;;
		-c)
		CLASSES_HIERARCHY_FILE="$2"
		shift # past argument
		;;
	esac

	shift
done

classDeclaration=`grep -n -e "^[ 	]*[A-z0-9]*[ 	]*class[ 	]\+[A-Z][A-z0-9]*[ 	]*:[ 	]*[A-Z][A-z0-9]*" $INPUT_FILE`
line=`cut -d: -f1 <<< "$classDeclaration"`
cleanClassDeclaration=`cut -d: -f2,3 <<< "$classDeclaration"`
classModifiers=`sed -e 's#^\(.*\)class .*#\1#' <<< "$cleanClassDeclaration"`
className=`sed -e 's#^.*class \([A-z][A-z0-9]*\)[ 	]*\:.*#\1#' <<< "$cleanClassDeclaration"`
baseClassName=`cut -d: -f2 <<< "$cleanClassDeclaration" | sed -e 's/[^[:alnum:]_-]//g'`

if [ -z "$className" ];
then
	cp -f $INPUT_FILE $OUTPUT_FILE
    exit 0
fi

# replace any previous entry
if [ -f $CLASSES_HIERARCHY_FILE ];
then
	sed -i -e "s#^$className:.*##g" $CLASSES_HIERARCHY_FILE
fi

# save new hierarchy
echo "$className:$baseClassName:$classModifiers" >> $CLASSES_HIERARCHY_FILE

if [ "$className" = "Object" ];
then
	baseClassName=
fi

#echo "classDeclaration: $classDeclaration"
#echo "cleanClassDeclaration: $cleanClassDeclaration"
#echo "line: $line"
#echo "Modifiers: $classModifiers"

#if [ -z "$classModifiers" ];
#then
#	echo "$className inherits from $baseClassName"
#else
#	echo "$className inherits from $baseClassName ( is $classModifiers)"
#fi

#sed -e "s#$classDeclaration#__CLASS($className);#g" <<< "$classDeclaration"
#echo attributes $attributes

end=`tail -n +$line $INPUT_FILE | grep -m 1 -n "}" | cut -d: -f1`
end=$((line + end))
line=$((line + 1))

classDeclarationBlock=`cat $INPUT_FILE | sed ''"$line"','"$end"'!d' | grep -v -e '^[ 	]*[\*//]\+.*' | sed -e 's#[{}]#\'$'\n#' | tr -d "\r\n"  | sed -e 's/;/;\'$'\n/g'`
#echo "$classDeclarationBlock"
methods=`grep -v -e '^[ 	\*A-z0-9]\+[ 	]*([ 	]*\*' <<< "$classDeclarationBlock" | grep -e '(.*)[ 	=0]*;[ 	]*'`
attributes=`grep -v -e '^[ 	\*A-z0-9]\+[ 	]*([ 	]*[^\*]' <<< "$classDeclarationBlock" | grep -e ';' | sed -e 's#.*;#&\\\#' `

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

VIRTUAL_METHODS_FILE=$WORKING_FOLDER/$HELPER_FILES_PREFIX"VirtualMethods.txt"

# Process each method to generate the final header
methodDeclarations=

# check if necessary files already exist
if [ ! -f $VIRTUAL_METHODS_FILE ];
then
	touch $VIRTUAL_METHODS_FILE
fi

separator=
virtualMethodsFileHasContents=`cat $VIRTUAL_METHODS_FILE`
if [ ! -z "$virtualMethodsFileHasContents" ];
then
	separator="\|"
fi

while IFS= read -r method;
do
    if [ -z "$method" ];
    then
        continue;
    fi

    methodPrelude=`cut -d "(" -f1 <<< "$method"`
    methodName=`sed 's/.* //g' <<< "$methodPrelude"`
    methodType=`sed 's/'$methodName'//g' <<< "$methodPrelude"`
    methodParameters=`cut -d "(" -f2- <<< "$method" | rev | cut -d ")" -f2- | rev`

    nonModifiedMethodType=`sed -e 's#virtual##;s#override##;s#static##' <<< "$methodType"`
    methodIsAbstract=false

    #echo
    #echo "method $method"
    #echo "methodType $methodType"
    #echo "methodName $methodName"
    #echo "methodParameters $methodParameters"
    #echo nonModifiedMethodType $nonModifiedMethodType

	if [ ! -z "$methodType" ] && [ -z "${methodType##*virtual *}" ];
	then

		methodCall="$className""_""$methodName"
		hasMethod=`grep -e "|$methodCall(" $VIRTUAL_METHODS_FILE`
		if [ -z "$hasMethod" ];
		then
			echo "$separator\<$methodCall[ 	]*(.*" >> $VIRTUAL_METHODS_FILE
			separator="\|"
		fi

        if [ ! -z "$methodParameters" ];
        then
#			echo "method $method"
#			echo "methodParameters $methodParameters"
            virtualMethodDeclarations=$virtualMethodDeclarations"\\
__VIRTUAL_DEC(ClassName, "$nonModifiedMethodType", "$methodName", "$methodParameters");"
        else
            virtualMethodDeclarations=$virtualMethodDeclarations"\\
__VIRTUAL_DEC(ClassName, "$nonModifiedMethodType", "$methodName");"
        fi

        abstractMark=`sed -n -E 's#\)[    ]*=[    ]*0[    ]*;##p' <<< "$method"`
        if [ -z "$abstractMark" ];
		then
            #echo $methodName is not abstract
            virtualMethodOverrides=$virtualMethodOverrides"\\
__VIRTUAL_SET(ClassName, "$className", "$methodName");"
        else
			isAbstractClass=true
			methodIsAbstract=true
#            echo $methodName is abstract
        fi
    else
		if [ ! -z "$methodType" ] && [ -z "${methodType##*override *}" ];
		then

            methodType=`sed -e 's#override##' <<< "$methodType"`

            virtualMethodOverrides=$virtualMethodOverrides"\\
__VIRTUAL_SET(ClassName, "$className", "$methodName");"

			methodCall="$className""_""$methodName"
			hasMethod=`grep -e "|$methodCall(" $VIRTUAL_METHODS_FILE`
			if [ -z "$hasMethod" ];
			then
				echo "$separator\<$methodCall[ 	]*(.*" >> $VIRTUAL_METHODS_FILE
				separator="\|"
			fi
        fi
    fi

	methodDeclaration=
	if [ ! -z "$methodParameters" ];
	then
		if [ ! -z "$methodType" ] && [ -z "${methodType##*static *}" ];
		then
			methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"("$methodParameters");"
		else
			methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"(void* _this, "$methodParameters");"
		fi
	else

		if [ ! -z "$methodType" ] && [ -z "${methodType##*static *}" ];
		then
			methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"();"
		else
			methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"(void* _this);"
		fi
	fi

	methodDeclarations=$methodDeclarations"
"$methodDeclaration

done <<< "$methods"

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
echo "" > $TEMPORAL_FILE
if [ ! "$isStaticClass" = true ];
then
	echo "$virtualMethodDeclarations" >> $TEMPORAL_FILE
	echo "" >> $TEMPORAL_FILE
	echo "$virtualMethodOverrides" >> $TEMPORAL_FILE
	echo "" >> $TEMPORAL_FILE

	if [ ! "$isFinalClass" = true ];
	then
		if [ ! -z "$baseClassName" ];
		then
			attributes="#define "$className"_ATTRIBUTES \\
		"$baseClassName"_ATTRIBUTES \\
		$attributes"

			virtualMethodDeclarations=$virtualMethodDeclarations" "$baseClassName"_METHODS(ClassName) "
			virtualMethodOverrides=$virtualMethodOverrides" "$baseClassName"_SET_VTABLE(ClassName) "
		else
			attributes="#define "$className"_ATTRIBUTES \\
		$attributes"
		fi

		echo "$attributes" >> $TEMPORAL_FILE
	fi
fi

echo "" >> $TEMPORAL_FILE
if [ ! "$isStaticClass" = true ];
then
	echo "__CLASS($className);" >> $TEMPORAL_FILE
fi

echo "" >> $TEMPORAL_FILE
sed -e 's#static[ 	]\+##g' <<< "$methodDeclarations" >> $TEMPORAL_FILE
echo >> $TEMPORAL_FILE

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

rm -f $TEMPORAL_FILE