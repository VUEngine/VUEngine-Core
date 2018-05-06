if ! [ -z "$1" ]
then
	echo "Cleaning VIRTUAL_CALLS from $1"
	find $1 -name "*.c" -exec sed -i -e "s:__VIRTUAL_CALL(\([A-z0-9]*\), *\([A-z0-9]*\), *\(.*\)*:\1_\2(\3:g" {} \;
	echo "Cleaning CALL_BASE_METHOD from $1"
	find $1 -name "*.c" -exec sed -i -e "s:__CALL_BASE_METHOD([A-z0-9]*, *\([A-z0-9]* *\), *\([A-z0-9]*\), *:Base_\1(\2, :g" {} \;
	echo "Cleaning CONSTRUCT_BASE from $1"
	find $1 -name "*.c" -exec sed -i -e "s:.*__CONSTRUCT_BASE([A-z0-9]\+,\(.*\):	Base_constructor(this,\1:g" {} \;
	echo "Cleaning DESTROY_BASE from $1"
	find $1 -name "*.c" -exec sed -i -e "s:__DESTROY_BASE:Base_destructor():g" {} \;
fi