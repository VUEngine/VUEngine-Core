#!/bin/bash

echo $1
INPUT_FILE=$1
BACKUP_FILE=$1.txt
OUTPUT_FILE=$1.new

WORKING_FOLDER=$2
rm -f $WORKING_FOLDER/*.txt

CLEAN_INPUT_FILE=$WORKING_FOLDER/Clean.txt
ONLY_METHODS_INPUT_FILE=$WORKING_FOLDER/OnlyMethods.txt

sed -e "s#\\\##g" $INPUT_FILE > $CLEAN_INPUT_FILE
sed -e "s#.*\\\##g" $INPUT_FILE > $ONLY_METHODS_INPUT_FILE

classDeclarationBlockStartLine=`grep -n -m 1 "_METHODS(" $CLEAN_INPUT_FILE | cut -d: -f1`
classDeclaration=`grep -n "__CLASS(" $INPUT_FILE | sed -e "s#__CLASS(\(.*\));#\1#"`
classDeclarationBlockEndLine=`cut -d: -f1 <<< "$classDeclaration"`
cleanClassDeclaration=`cut -d: -f2,3 <<< "$classDeclaration"`
className=`grep "#define" $CLEAN_INPUT_FILE | grep "_METHODS" | cut -d_ -f1 | sed -e "s#\#define##"`
baseClassName=`grep -v "#define" $CLEAN_INPUT_FILE | grep "_METHODS" | cut -d_ -f1`

if [ -z "$className" ];
then
    exit 0
fi
echo Class $className
echo Base Class $baseClassName
echo Class declared betwee lines $classDeclarationBlockStartLine and $classDeclarationBlockEndLine

# PROCESS METHODS
#echo attributes $attributes
#attributes=`sed -n -e 's/.*[A-z0-9]* *[A-z0-9]*;/&\\\/p' <<< "$classDeclarationBlock" | grep -v -e "(.*)"`

virtualMethodDeclarations=`grep "__VIRTUAL_DEC" $CLEAN_INPUT_FILE | cut -d, -f3 | cut -d, -f1 | cut -d\) -f1 | sed -e 's#[ ]\+##g'`
virtualMethodOverrides=`grep "__VIRTUAL_SET" $CLEAN_INPUT_FILE | cut -d, -f3 | cut -d\) -f1 | sed -e 's#[ ]\+##g'`

#echo
#echo virtualMethodDeclarations
#echo "$virtualMethodDeclarations"
#echo
#echo virtualMethodOverrides
#echo "$virtualMethodOverrides"

VIRTUAL_METHODS_FILE=$WORKING_FOLDER/VirtualMethods.txt
touch $VIRTUAL_METHODS_FILE
#cat $ONLY_METHODS_INPUT_FILE
#echo
echo Processing virtual modifier
while IFS= read -r virtualMethod;
do
    if [ -z "$virtualMethod" ];
    then
        continue;
    fi

    methodDeclaration=`grep -m 1 $virtualMethod"(" $ONLY_METHODS_INPUT_FILE | sed 's#.*#virtual &#'`
	methodSearchPattern="_"$virtualMethod"("
    alreadyDeclared=`grep "$methodSearchPattern" $VIRTUAL_METHODS_FILE`

    if [ -z "$alreadyDeclared" ];
    then
#		echo virtualMethod $virtualMethod
#		echo methodDeclaration "$methodDeclaration"
        echo "	$methodDeclaration" >> $VIRTUAL_METHODS_FILE
    fi

done <<< "$virtualMethodDeclarations"

#echo
echo Processing override modifier
while IFS= read -r virtualMethod;
do
    if [ -z "$virtualMethod" ];
    then
        continue;
    fi

    methodDeclaration=`grep -m 1 $virtualMethod"(" $ONLY_METHODS_INPUT_FILE | sed 's#.*#override &#'`
	methodSearchPattern="_"$virtualMethod"("
    alreadyDeclared=`grep "$methodSearchPattern" $VIRTUAL_METHODS_FILE`

    if [ -z "$alreadyDeclared" ];
    then
#		echo virtualMethod $virtualMethod
#		echo methodDeclaration "$methodDeclaration"
        echo "	$methodDeclaration" >> $VIRTUAL_METHODS_FILE
    fi

done <<< "$virtualMethodOverrides"

STATIC_METHODS_FILE=$WORKING_FOLDER/StaticMethods.txt
touch $STATIC_METHODS_FILE

#echo
echo Processing non-virtual methods
while IFS= read -r methodDeclaration;
do
    if [ -z "$methodDeclaration" ];
    then
        continue;
    fi

    alreadyDeclared=`grep -F "$methodDeclaration" $VIRTUAL_METHODS_FILE`

    if [ -z "$alreadyDeclared" ];
    then
#		echo methodDeclaration $methodDeclaration
        echo "	$methodDeclaration" >> $STATIC_METHODS_FILE
    fi

done <<< "$(grep -e '.*[A-z][a-z0-9]*_[a-z][A-z0-9]*(.*);' $ONLY_METHODS_INPUT_FILE)"

# PROCESS ATTRIBUTES
attributes=
hasAttributes=`grep "_ATTRIBUTES" $INPUT_FILE`
if [ ! -z "$hasAttributes" ];
then
    HELPER_FILE=$WORKING_FOLDER/helper.txt
    sed -e "s#\\\#VUEMARK#g" $INPUT_FILE > $HELPER_FILE

    attributesBlockStartLine=`grep -n -v "#define" $CLEAN_INPUT_FILE | grep "_ATTRIBUTES" | cut -d: -f1`
    attributesBlockStartLine=$((attributesBlockStartLine + 0))

    totalLines=`wc -l < $CLEAN_INPUT_FILE`
    tail -$((totalLines - attributesBlockStartLine)) $CLEAN_INPUT_FILE > $HELPER_FILE
    attributesBlockEndLine=`grep -n "CLASS(" $HELPER_FILE | cut -d: -f1`
    attributesBlockEndLine=$((attributesBlockEndLine - 1))
    attributes=`head -${attributesBlockEndLine} $HELPER_FILE | sed -e 's#^[ 	]*#	#'`
fi

#echo attributes
#echo "$attributes"

# DO THE INJETION

READY_FOR_INJECTION_HELPER_FILE=$WORKING_FOLDER/ReadyForInjectionHelper.txt
READY_FOR_INJECTION_FILE=$WORKING_FOLDER/ReadyForInjection.txt

classDeclarationBlockStartLine=$((classDeclarationBlockStartLine - 1))
head -${classDeclarationBlockStartLine} $INPUT_FILE > $READY_FOR_INJECTION_HELPER_FILE

totalLines=`wc -l < $INPUT_FILE`
tailLines=$((totalLines - classDeclarationBlockEndLine))
tail -${tailLines} $INPUT_FILE >> $READY_FOR_INJECTION_HELPER_FILE

grep -v '\\' $READY_FOR_INJECTION_HELPER_FILE | grep -v -e '.*[A-z][a-z0-9]*_[a-z][A-z0-9]*(.*);' | grep -v '__CLASS' > $READY_FOR_INJECTION_FILE

lastEndIf=`grep -n "#endif" $READY_FOR_INJECTION_FILE | tail -1 | cut -d: -f1`

prelude=$((lastEndIf - 1))

head -${prelude} $READY_FOR_INJECTION_FILE > $OUTPUT_FILE

echo >> $OUTPUT_FILE
echo class $className : $baseClassName  >> $OUTPUT_FILE
echo "{" >> $OUTPUT_FILE
if [ ! -z "$attributes" ];
then
    echo "$attributes" >> $OUTPUT_FILE
fi
cat $STATIC_METHODS_FILE >> $OUTPUT_FILE
cat $VIRTUAL_METHODS_FILE >> $OUTPUT_FILE
echo "}" >> $OUTPUT_FILE
echo >> $OUTPUT_FILE
echo >> $OUTPUT_FILE
echo "#endif" >> $OUTPUT_FILE

className=`sed -e "s#[  ]*##g" <<< "$className"`
sed -i -e "s#$className this[ ]*,[ ]*##g" $OUTPUT_FILE
sed -i -e "s#$className this[ ]*##g" $OUTPUT_FILE
sed -i -e "s#$className\_##g" $OUTPUT_FILE


cat $OUTPUT_FILE > $INPUT_FILE
rm -f $OUTPUT_FILE

rm -f $WORKING_FOLDER/*.txt
