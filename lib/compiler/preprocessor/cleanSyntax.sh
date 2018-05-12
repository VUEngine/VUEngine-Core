if [ ! -z "$1" ] && [ ! -z "$2" ] && [ ! -z "$3" ]
then
	VUENGINE_HOME=$1
	SOURCE_FOLDER=$2
	WORKING_FOLDER=$3
#	echo "Cleaning VIRTUAL_CALLS from $2"
#	find $2 -name "*.c" -exec sed -i -e "s:__VIRTUAL_CALL(\([A-z0-9]*\), *\([A-z0-9]*\), *\(.*\)*:\1_\2(\3:g" {} \;
#	echo "Cleaning CALL_BASE_METHOD from $2"
#	find $2 -name "*.c" -exec sed -i -e "s:__CALL_BASE_METHOD([A-z0-9]*, *\([A-z0-9]* *\), *\([A-z0-9]*\), *:Base_\1(\2, :g" {} \;
#	echo "Cleaning CONSTRUCT_BASE from $2"
#	find $2 -name "*.c" -exec sed -i -e "s:.*__CONSTRUCT_BASE([A-z0-9]\+,\(.*\):	Base_constructor(this,\1:g" {} \;
#	echo "Cleaning DESTROY_BASE from $2"
#	find $2 -name "*.c" -exec sed -i -e "s:__DESTROY_BASE:Base_destructor():g" {} \;
	echo "Cleaning class declarations from $2"
	find $SOURCE_FOLDER -name "*.h" -exec $VUENGINE_HOME/lib/compiler/preprocessor/portHeader.sh {} $WORKING_FOLDER \;
fi