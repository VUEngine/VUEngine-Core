if ! [ -z "$1" ]
then
	echo "Cleaning VIRTUAL_CALLS from $1"
	find $1 -name "*.c" -exec sed -i -e "s:__VIRTUAL_CALL(\([A-z0-9]*\), *\([A-z0-9]*\), *\(.*\)*:\1_\2(\3:g" {} \;
	echo "Cleaning CALL_BASE_METHOD from $1"
	find $1 -name "*.c" -exec sed -i -e "s:__CALL_BASE_METHOD([A-z0-9]*, *\([A-z0-9]* *\), *\([A-z0-9]*\), *:Base_\1(\2, :g" {} \;
fi