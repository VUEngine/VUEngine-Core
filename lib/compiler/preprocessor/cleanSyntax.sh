if [ ! -z "$1" ] && [ ! -z "$2" ] && [ ! -z "$3" ]
then
	VUENGINE_HOME=$1
	SOURCE_FOLDER=$2
	WORKING_FOLDER=$3

	if [ ! -d "$WORKING_FOLDER/preprocessor" ];
	then
		mkdir -p $WORKING_FOLDER/preprocessor
	fi

	echo "Cleaning $SOURCE_FOLDER"
#	find $SOURCE_FOLDER -name "*.h" -exec cp -f {}.txt {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__VIRTUAL_CALL(\([A-z0-9]*\), *\([A-z0-9]*\), *\(.*\)*:\1_\2(\3:g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__CALL_BASE_METHOD([A-z0-9]*, *\([A-z0-9]* *\), *\([A-z0-9]*\), *:Base_\1(\2, :g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#__CONSTRUCT_BASE(\(.*\)#Base::constructor(\1)#" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__DESTROY_BASE:Base_destructor():g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__CLASS_DEFINITION.*::g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__CLASS_NEW_.*::g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__SINGLETON(.*::g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s:__SINGLETON_.*(.*::g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#\([A-Z][A-z0-9]*\)_\([a-z][A-z0-9]*\)#\1::\2#g" {} \;
	find $SOURCE_FOLDER -name "*.h" -exec $VUENGINE_HOME/lib/compiler/preprocessor/portHeader.sh {} $WORKING_FOLDER \;

	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#__SAFE_CAST(\([A-Z][A-z0-9]*\), #\1::safeCast(#g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#([A-Z][A-z0-9]* this)#()#g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#([A-Z][A-z0-9]* this, #(#g" {} \;
	find $SOURCE_FOLDER -name "*.h" -exec sed -i -e "s#([A-Z][A-z0-9]* this)#()#g" {} \;
	find $SOURCE_FOLDER -name "*.h" -exec sed -i -e "s#([A-Z][A-z0-9]* this, #(#g" {} \;

	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#__NEW(\([A-Z][A-z0-9]*\),#new \1(#g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#__NEW_BASIC(\([A-Z][A-z0-9]*\))#new \1#g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#__DELETE(\([A-Z][A-z0-9]*\));#delete \1;#g" {} \;
	find $SOURCE_FOLDER -name "*.c" -exec sed -i -e "s#__DELETE_BASIC(\([A-Z][A-z0-9]*\));#delete \1;#g" {} \;

fi