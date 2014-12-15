#!/bin/bash
#
#Jorge Eremiev
#
#Delete SVN information
#
FILES=`find ../ -name *svn*`
for file in $FILES ; do
	rm -Rf $file
done
