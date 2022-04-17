#!/bin/bash
vbwavOSX $1 $3
name=`echo $1 | sed -e "s@.wav@@g"`
output=`echo $1 | sed -e "s@.wav@.h@g"`
finalDestination=`echo $1 | sed -e "s@.wav@.c@g" | sed -e "s@sample@@g"`
mv $output Binary/$finalDestination
sed -i -e "s@sample_@@g" Binary/$finalDestination
sed -i -e "s@static@@g" Binary/$finalDestination
length=`grep _LEN Binary/$finalDestination | sed -E "s@.*_LEN@@g"`
echo $length
sed -i -e 's@#define '"$name"'Length.*@#define '"$name"'Length '"$length"'@g' $2|grep Length
rm Binary/*-e

