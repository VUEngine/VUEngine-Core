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

classDeclaration=`grep -n -e "^[ 	]*[A-z0-9]*[ 	]*class[ 	]\+[A-Z][A-z0-9]*[ 	]*:[ 	]*[A-Z][A-z0-9]*" $INPUT_FILE`
line=`cut -d: -f1 <<< "$classDeclaration"`
cleanClassDeclaration=`cut -d: -f2,3 <<< "$classDeclaration"`
classModifiers=`sed -e 's#\(^.*\)class[ 	]\+.*#\1#' <<< "$cleanClassDeclaration"`
className=`sed -e 's#^.*class \([A-z][A-z0-9]*\)[ 	]*\:.*#\1#' <<< "$cleanClassDeclaration"`
baseClassName=`cut -d: -f2 <<< "$cleanClassDeclaration" | sed -e 's#[  ]##g'`

if [ -z "$className" ];
then
	cat $INPUT_FILE >> $OUTPUT_FILE
    exit 0
fi

if [ "$className" = "Object" ];
then
	baseClassName=
fi

#echo "classDeclaration: $classDeclaration"
#echo "cleanClassDeclaration: $cleanClassDeclaration"
#echo "line: $line"
#echo "Modifiers: $classModifiers"
echo "$className inherits from $baseClassName"

#sed -e "s#$classDeclaration#__CLASS($className);#g" <<< "$classDeclaration"
#echo attributes $attributes

end=`tail -n +$line $INPUT_FILE | grep -m 1 -n "}" | cut -d: -f1`
end=$((line + end))
line=$((line + 1))
classDeclarationBlock=`sed ''"$line"','"$end"'!d' $INPUT_FILE | grep -v -e '^[ 	][\*/]' | sed -e 's#[{}]#\n#' | tr -d '\n' | sed -e 's#;#;\n#g'`

#echo "$classDeclarationBlock"
methods=`grep -v -e '^[ 	\*A-z0-9]\+[ 	]*([ 	]*\*' <<< "$classDeclarationBlock" | grep -e '(.*)[ 	=0]*;[ 	]*$'`
attributes=`grep -v -e '^[ 	\*A-z0-9]\+[ 	]*([ 	]*[^\*]' <<< "$classDeclarationBlock" | grep -v -e '^[ 	][\*/]' | grep -e ';' | sed -e 's#.*;#&\\\#' `


#echo
#echo "methods 
#$methods"
#echo
#echo "attributes 
#$attributes"

isSingletonClass=false
isAbstractClass=false
isFinal=false


virtualMethodDeclarations="#define "$className"_METHODS(ClassName)"
virtualMethodOverrides="#define "$className"_SET_VTABLE(ClassName)"

if [ ! -z "$baseClassName" ];
then
	virtualMethodDeclarations=$virtualMethodDeclarations" "$baseClassName"_METHODS(ClassName) "
	virtualMethodOverrides=$virtualMethodOverrides" "$baseClassName"_SET_VTABLE(ClassName) "
fi
	
#echo
#echo methods
#echo $methods

# Process each method to generate the final header
while IFS= read -r method;
do
    if [ -z "$method" ];
    then
        continue;
    fi

	methodPrelude=`cut -d "(" -f1 <<< "$method"`
    methodType=`sed -e 's#\(^.*\)[ \t]\+[a-z][A-z0-9]\+[ \t]*$#\1#' <<< "$methodPrelude"`
    methodName=`sed -e 's#^.*[ \t]\+\([a-z][A-z0-9]\+[ \t]*$\)#\1#' <<< "$methodPrelude"`
    methodParamenters=`cut -d "(" -f2- <<< "$method" | rev | cut -d ")" -f2- | rev`

	
#   echo
#   echo "method $method"
#   echo "methodType $methodType"
#   echo "methodName $methodName"
#   echo "methodParamenters $methodParamenters"

    nonModifiedMethodType=`sed -e 's#virtual##' -e 's#override##' -e 's#static##' <<< "$methodType"`
	methodIsAbstract=false
	
    if [[ $methodType = *"virtual "* ]]; then

        if [ ! -z "$methodParamenters" ];
        then
			#echo "method $method"
			#echo "methodParamenters $methodParamenters"
            virtualMethodDeclarations=$virtualMethodDeclarations"\\
__VIRTUAL_DEC(ClassName, "$nonModifiedMethodType", "$methodName", "$methodParamenters");"
        else
            virtualMethodDeclarations=$virtualMethodDeclarations"\\
__VIRTUAL_DEC(ClassName, "$nonModifiedMethodType", "$methodName");"
        fi

        abstractMark=`sed -n -E 's#\)[    ]*=[    ]*0[    ]*;##p' <<< "$method"`
        if [[ -z "$abstractMark" ]]; then

            #echo $methodName is not abstract
            virtualMethodOverrides=$virtualMethodOverrides"\\
__VIRTUAL_SET(ClassName, "$className", "$methodName");"
        else
			isAbstractClass=true
			methodIsAbstract=true
#            echo $methodName is abstract
        fi
    else
        if [[ $methodType = *"override "* ]]; then

            methodType=`sed -e 's#override##' <<< "$methodType"`

            virtualMethodOverrides=$virtualMethodOverrides"\\
__VIRTUAL_SET(ClassName, "$className", "$methodName");"
        fi
    fi

    if [ ! "$methodIsAbstract" = true ];
    then
		methodDeclaration=
        if [ ! -z "$methodParamenters" ];
        then
			if [[ $methodType = *"static "* ]]; then
				methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"("$methodParamenters");"				
			else
				methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"("$className" this, "$methodParamenters");"
			fi
        else

			if [[ $methodType = *"static "* ]]; then
				methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"();"
			else
				methodDeclaration=$nonModifiedMethodType" "$className"_"$methodName"("$className" this);"
			fi
        fi

		methodDeclarations=$methodDeclarations"
"$methodDeclaration
    fi
done <<< "$methods"

while IFS= read -r classModifier;
do
    if [ -z "$classModifier" ];
    then
        continue;
    fi

    if [[ $classModifier = *"abstract "* ]]; then

		isAbstractClass=true
	fi
    
	if [[ $classModifier = *"final "* ]]; then

		isFinal=true;
	fi

	if [[ $classModifier = *"singleton "* ]]; then

		isSingletonClass=true
		
		if [ -z "$attributes" ];
		then
			isFinal=true;
		fi
	fi
	

done <<< "$classModifiers"

if [ "$isAbstractClass" = true ];
then
	echo $className is abstract
fi

if [ "$isSingletonClass" = true ];
then
	echo $className is singleton
fi

if [ "$isFinal" = true ];
then
	echo $className is final
fi

# Add destructor declaration
methodDeclarations="	void "$className"_destructor("$className" this);
"$methodDeclarations

# Add allocator if it is not abstract nor a singleton class
if [ ! "$isAbstractClass" = true ] && [ ! "$isSingletonClass" = true ] ;
then

	constructor=`grep -m 1 -e "void[ 	]\+"$className"_constructor[ 	]*(.*);" <<< "$methodDeclarations"`
#	echo constructor $constructor
#	echo "Adding allocator based on constructor $constructor"

	if [ -z "$constructor" ];
	then
		echo "Error: no constructor defined for $className : $baseClassName"
		exit 0
	else
		echo "Added allocator"
#		echo constructor $constructor
#		parameters=`cut -d\( -f2 <<< $constructor | cut -d\) -f1 | cut -d, -f2- | sed -e 's#^.*this##'`
		parameters=`cut -d\( -f2 <<< $constructor | cut -d\) -f1 | cut -d, -f2- | sed -e 's#^.*this##'`
#		echo parameters $parameters
		allocator="	"$className" "$className"_new("$parameters");"
		methodDeclarations="$allocator
"$methodDeclarations
	fi
fi



TEMPORAL_FILE=$WORKING_FOLDER/temporal.txt
echo "" > $TEMPORAL_FILE
echo "$virtualMethodDeclarations" >> $TEMPORAL_FILE
echo "" >> $TEMPORAL_FILE
echo "$virtualMethodOverrides" >> $TEMPORAL_FILE
echo "" >> $TEMPORAL_FILE
if [ ! "$isFinal" = true ];
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
echo "" >> $TEMPORAL_FILE
echo "__CLASS($className);" >> $TEMPORAL_FILE
echo "" >> $TEMPORAL_FILE
echo "$methodDeclarations" >> $TEMPORAL_FILE
echo >> $TEMPORAL_FILE

prelude=$((line - 2))
totalLines=`wc -l < $INPUT_FILE`
remaining=$((totalLines - end))

head -${prelude} $INPUT_FILE > $OUTPUT_FILE
cat $TEMPORAL_FILE >> $OUTPUT_FILE
tail -${remaining} $INPUT_FILE >> $OUTPUT_FILE
sed -i -e 's#^[ 	]*class[ 	]\+\([A-Z[A-z0-9]*\)[ 	]*;#__FORWARD_CLASS(\1)#' $OUTPUT_FILE

rm -f $WORKING_FOLDER/*.txt
echo


