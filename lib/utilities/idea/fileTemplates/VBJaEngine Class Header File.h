##
## ensure the class name starts with an uppercase character
#set ($NAME = $NAME.substring(0,1).toUpperCase() + $NAME.substring(1))
#set ($NameUpper = $NAME.toUpperCase())
##
## set super class to object if it is not set,
## otherwise ensure the class name starts with an uppercase character
#if (!$Super_class_name || $Super_class_name == "")
    #set ($SuperClassClean = "Object")
#else
    #set ($SuperClassClean = $Super_class_name.substring(0,1).toUpperCase() + $Super_class_name.substring(1))
#end
##
## include default engine file header
#parse("VBJaEngine Header.c")

#ifndef ${NameUpper}_H_
\#define ${NameUpper}_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

\#include <${SuperClassClean}.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

\#define ${NAME}_METHODS																					\
	${SuperClassClean}_METHODS;																						\

\#define ${NAME}_SET_VTABLE(ClassName)																		\
	${SuperClassClean}_SET_VTABLE(ClassName);																		\

__CLASS(${NAME});

\#define ${NAME}_ATTRIBUTES																					\
																										\
	/* it is derived from */																			\
	${SuperClassClean}_ATTRIBUTES																					\


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------
#if (${Is_singleton} && ${Is_singleton} == 1)

${NAME} ${NAME}_getInstance();
#end

__CLASS_NEW_DECLARE(${NAME}, ${SuperClassClean}Definition* definition, int id, const char* const name);

void ${NAME}_constructor(${NAME} this, ${SuperClassClean}Definition* definition, int id, const char* const name);
void ${NAME}_destructor(${NAME} this);


#endif