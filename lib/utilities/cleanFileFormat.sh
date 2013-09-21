FILES=`find ../ -name *.h`
for file in $FILES ; do
	sed -i 's///g' $file 
done

FILES=`find ../ -name *.c`
for file in $FILES ; do
	sed -i 's///g' $file 
done

