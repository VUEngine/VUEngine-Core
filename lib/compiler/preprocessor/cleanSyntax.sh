if [ ! -z "$1" ] && [ ! -z "$2" ] && [ ! -z "$3" ]
then
	VUENGINE_HOME=$1
	SOURCE_FOLDER=$2
	WORKING_FOLDER=$3
	echo "Cleaning $SOURCE_FOLDER"
#	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__VIRTUAL_CALL(\([A-z0-9]*\), *\([A-z0-9]*\), *\(.*\)*:\1_\2(\3:g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__CALL_BASE_METHOD([A-z0-9]*, *\([A-z0-9]* *\), *\([A-z0-9]*\), *:Base_\1(\2, :g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#__CONSTRUCT_BASE(\(.*\)#Base::constructor(\1)#" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__DESTROY_BASE:Base_destructor():g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__CLASS_DEFINITION.*::g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__CLASS_NEW_.*::g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#\([A-Z][A-z0-9]*\)_\([a-z][A-z0-9]*\)#\1::\2#g" {} \;
	find $SOURCE_FOLDER -name "*.h" -exec $VUENGINE_HOME/lib/compiler/preprocessor/portHeader.sh {} $WORKING_FOLDER \;
#	find $SOURCE_FOLDER -name "*.h" -exec cp -f {}.txt {} \;

fi