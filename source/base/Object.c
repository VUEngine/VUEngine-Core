/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Printing.h>
#include <DebugConfig.h>

#include "Object.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Object::constructor()
{
}

/**
 * Class destructor
 */
void Object::destructor()
{
	// free the memory
#ifndef __BYPASS_MEMORY_MANAGER_WHEN_DELETING
	MemoryPool::free((void*)((uint32)this - __DYNAMIC_STRUCT_PAD));
#else
	*((uint32*)((uint32)this - __DYNAMIC_STRUCT_PAD)) = __MEMORY_FREE_BLOCK_FLAG;
#endif

	this = NULL;
}

/**
 * Casts an object to base class
 *
 * @param targetClassGetClassMethod
 * @param baseClassGetClassMethod
 * @return								Casted Object
 */
static Object Object::getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod)
{
#ifdef __BYPASS_CAST
	return object;
#endif

#ifndef __DEBUG
	HardwareManager::suspendInterrupts();
#endif

	static int32 lp = -1;
	static int32 sp = -1;

	if(-1 == lp && -1 == sp)
	{
		asm
		(
			"mov	sp, %0"
			: "=r" (sp)
		);

		asm
		(
			"mov	lp, %0"
			: "=r" (lp)
		);
	}

#ifdef __DEBUG
	static int counter = 0;

	if(100 < ++counter)
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (uint32)object, 18, 15, 8, NULL);
	
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: infinite callback");
	}
#endif

	if(NULL == object)
	{
		lp = -1;
		sp = -1;
#ifdef __DEBUG
		counter = 0;
#else
		HardwareManager::resumeInterrupts();
#endif
		return NULL;
	}

#ifndef __SHIPPING
#ifndef __RELEASE
	if(isDeleted(object))
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (uint32)object, 18, 15, 8, NULL);
	
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: deleted object");
	}

	if(NULL == __VIRTUAL_CALL_ADDRESS(Object, getClassName, object))
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (uint32)object, 18, 15, 8, NULL);
	
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: null getClassName");
	}

	if(NULL == __VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object))
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (uint32)object, 18, 15, 8, NULL);
	
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: null getBaseClass");
	}
#endif
#endif

	if(NULL == baseClassGetClassMethod)
	{
		if(targetClassGetClassMethod == (ClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object))
		{
			lp = -1;
			sp = -1;
#ifdef __DEBUG
			counter = 0;
#else
			HardwareManager::resumeInterrupts();
#endif
			return object;
		}

		// make my own virtual call, otherwise the macro will cause an infinite recursive call because of the
		// ObjectClass::getCast check
		baseClassGetClassMethod = (ClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object)(object);
	}

	if(NULL == baseClassGetClassMethod || ((ClassPointer)&Object_getBaseClass == baseClassGetClassMethod && (ClassPointer)&Object_getBaseClass != targetClassGetClassMethod))
	{
		lp = -1;
		sp = -1;

#ifdef __DEBUG
		counter = 0;
#else
		HardwareManager::resumeInterrupts();
#endif
		return NULL;
	}

	if(targetClassGetClassMethod == baseClassGetClassMethod)
	{
		lp = -1;
		sp = -1;

#ifdef __DEBUG
		counter = 0;
#else
		HardwareManager::resumeInterrupts();
#endif
		return object;
	}

#ifndef __DEBUG
	HardwareManager::resumeInterrupts();
#endif

	return Object::getCast((Object)object, targetClassGetClassMethod, (ClassPointer)baseClassGetClassMethod(object));
}

/**
 * Get an Object's vTable
 *
 * @return		vTable pointer
 */
const void* Object::getVTable()
{
	return this->vTable;
}

/**
 * Get an Object's vTable
 *
 * @return		vTable pointer
 */
bool Object::evolveTo(const void* targetClass)
{
	const struct Object_vTable* targetClassVTable = (const struct Object_vTable*)targetClass;
	const struct Object_vTable* thisVTable = (const struct Object_vTable*)this->vTable;
	
	if(targetClassVTable == thisVTable)
	{
		return true;
	}

	ClassPointer baseClassGetClassMethod = (ClassPointer)targetClassVTable->getBaseClass(NULL);

	while((ClassPointer)Object::getBaseClass != (ClassPointer)baseClassGetClassMethod)
	{
		if((ClassPointer)thisVTable->getBaseClass == baseClassGetClassMethod)
		{
			this->vTable = (void*)targetClassVTable;
			return true;
		}

		baseClassGetClassMethod = (ClassPointer)baseClassGetClassMethod(NULL);
	}

	baseClassGetClassMethod = (ClassPointer)thisVTable->getBaseClass(NULL);
	
	while((ClassPointer)Object::getBaseClass != (ClassPointer)baseClassGetClassMethod)
	{
		if((ClassPointer)targetClassVTable->getBaseClass == baseClassGetClassMethod)
		{
			this->vTable = (void*)targetClassVTable;
			return true;

		}

		baseClassGetClassMethod = (ClassPointer)baseClassGetClassMethod(NULL);
	}

#ifndef __RELEASE
	char errorMessage [200] = "Object::evolve: trying to convert a ";
	strcat(errorMessage, __GET_CLASS_NAME(this));
	strcat(errorMessage, " into ");
	strcat(errorMessage, targetClassVTable->getClassName(NULL));
	Error::triggerException(errorMessage, NULL);		
#endif

	return false;
}

