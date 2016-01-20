##
## ensure the class name starts with an uppercase character
#set ($NAME = $NAME.substring(0,1).toUpperCase() + $NAME.substring(1))
##
## set super class to object if it is not set,
## otherwise ensure the class name starts with an uppercase character
#if (!$Super_class_name || $Super_class_name == "")
    #set ($SuperClassClean = "Object")
#else
    #set ($SuperClassClean = $Super_class_name.substring(0,1).toUpperCase() + $Super_class_name.substring(1))
#end
##
## derive instance name from class name, starting with a lowercase character
#set ($InstanceName = $NAME.substring(0,1).toLowerCase() + $NAME.substring(1))
##
## include default engine file header
#parse("VBJaEngine Header.c")


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

\#include <${SuperClassClean}.h>
\#include "${NAME}.h"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(${NAME}, ${SuperClassClean});


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------
#if (${Is_singleton} && ${Is_singleton} == 1)

// Only one instance
${NAME} ${InstanceName} = NULL;

// there can only be one ${InstanceName} instantiated
${NAME} ${NAME}_getInstance()
{
	return ${InstanceName};
}

void ${NAME}_setInstance(${NAME} instance)
{
	ASSERT(!${InstanceName}, "${NAME}::setInstance: already instantiated");

	${InstanceName} = instance;
}
#end

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(${NAME}, ${SuperClassClean}Definition* definition, int id, const char* const name)
__CLASS_NEW_END(${NAME}, definition, id, name);

// class's constructor
void ${NAME}_constructor(${NAME} this, ${SuperClassClean}Definition* definition, int id, const char* const name)
{
	ASSERT(this, "${NAME}::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(definition, id, name);
}

// class's destructor
void ${NAME}_destructor(${NAME} this)
{
	ASSERT(this, "${NAME}::destructor: null this");
#if (${Is_singleton} && ${Is_singleton} == 1)
	ASSERT(${InstanceName}, "${NAME}::destructor: already deleted");
	ASSERT(${InstanceName} == this, "${NAME}::destructor: more than one instance");
#end

	// delete the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}
