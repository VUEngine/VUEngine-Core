#!/bin/bash
#

if [ -z "$1" ];
then
        echo " Provide a file path"
        exit 0
fi

FILE=$1
FILE_BASENAME=`basename $FILE`

#echo " Cleaning $FILE_BASENAME"

if [ ! -f $FILE ];
then
	echo " File doesn't exist"
	exit 0
fi

hasTiles=`grep -e "Tiles\[" $FILE`
if [ -z "$hasTiles" ];
then
	echo " Doesn't have tiles"
	exit 0
fi

hasMap=`grep -e "Map\[" $FILE`
if [ -z "$hasMap" ];
then
	echo " Doesn't have a map"
	exit 0
fi

usesEmptyChar=`grep -w "0x0000" $FILE`

if [ ! -z "$usesEmptyChar" ];
then
	echo " Did not clean $FILE_BASENAME - empty char is used"
	exit 0
fi

sed -e 's/.*$/<NEWLINE>&/g' $FILE | tr -d "\r\n" > $FILE.tmp
mv $FILE.tmp $FILE

sed -e 's/\({<NEWLINE>[[:space:]]*\)0x00000000,0x00000000,0x00000000,0x00000000,/\1/' $FILE > $FILE.tmp
mv $FILE.tmp $FILE

# Put back new lines
sed -e 's/<NEWLINE>/\'$'\n/g' $FILE > $FILE.tmp
mv $FILE.tmp $FILE

mapDeclarationStartLine=`grep -m1 -n -e "Map\[" $FILE | cut -d ":" -f1`
mapDeclarationTotalLines=`tail -n +$mapDeclarationStartLine $FILE | grep -m 1 -n "};" | cut -d: -f1`
mapDeclarationEndLine=$((mapDeclarationStartLine + mapDeclarationTotalLines))
charIndexesStartLine=$((mapDeclarationStartLine + 2))
charIndexesEndLine=$((mapDeclarationEndLine - 2))
#echo mapDeclarationStartLine $mapDeclarationStartLine
#echo mapDeclarationEndLine $mapDeclarationEndLine

cp $FILE $FILE.bak

mapIndexesBlock=`cat $FILE | sed ''"$charIndexesStartLine"','"$charIndexesEndLine"'!d' | sed -e 's/,/\'$'\n/g'`
newMapIndexesBlock="	"
counter=0
for index in $mapIndexesBlock
do
	correctedIndex=$((index - 1))

	if [ 256 -gt "$correctedIndex" ]; then

		if [ 16 -gt "$correctedIndex" ]; then
			indexToPrint=0x000`printf '%x\n' $correctedIndex`
		else
			indexToPrint=0x00`printf '%x\n' $correctedIndex`
		fi
	else
		indexToPrint=0x0`printf '%x\n' $correctedIndex`
	fi

	#echo $index = $indexToPrint

	counter=$((counter + 1))

	if [ "$counter" -gt 8 ]; then
		newMapIndexesBlock="${newMapIndexesBlock}"$'\n'"	${indexToPrint},"
		counter=1
	else
		newMapIndexesBlock="$newMapIndexesBlock$indexToPrint",
	fi
done

cat $FILE | sed ''"1"','"$mapDeclarationStartLine"'!d' > $FILE.clean
echo "{" >> $FILE.clean
echo "$newMapIndexesBlock" >> $FILE.clean
echo "};" >> $FILE.clean

# Append anything else
tail -n +$mapDeclarationEndLine $FILE.bak >> $FILE.clean
mv $FILE.clean $FILE

# Fixing reported number of chars
numberOfChars=`grep -e "tiles .*" $FILE | cut -d " " -f2`
numberOfChars=$((numberOfChars - 1))
sed -e 's/+ [0-9][0-9]* \(tiles .*\)/+ '$numberOfChars' \1/g' $FILE > $FILE.clean
mv $FILE.clean $FILE

rm -f $FILE.bak

echo " Cleaned $FILE_BASENAME"
