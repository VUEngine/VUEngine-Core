/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <Printing.h>
#include <DebugConfig.h>

#include "Object.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Object Object::getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod)
{
#ifdef __BYPASS_CAST
	return object;
#endif

	HardwareManager::suspendInterrupts();

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

	if(10 < ++counter)
	{
		Printing::setDebugMode();
		Printing::text("Object's address: ", 1, 15, NULL);
		Printing::text(__GET_CLASS_NAME(object), 18, 15, NULL);
		Printing::text("Object's address: ", 1, 16, NULL);
		Printing::hex((uint32)object, 18, 16, 8, NULL);

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
#endif

		HardwareManager::resumeInterrupts();
		return NULL;
	}

#ifndef __SHIPPING
#ifndef __RELEASE
	if(isDeleted(object))
	{
		Printing::setDebugMode();
		Printing::text("Object's address: ", 1, 15, NULL);
		Printing::hex((uint32)object, 18, 15, 8, NULL);
	
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: deleted object");
	}

	if(NULL == __VIRTUAL_CALL_ADDRESS(Object, getClassName, object))
	{
		Printing::setDebugMode();
		Printing::text("Object's address: ", 1, 15, NULL);
		Printing::hex((uint32)object, 18, 15, 8, NULL);
	
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: null getClassName");
	}

	if(NULL == __VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object))
	{
		Printing::setDebugMode();
		Printing::text("Object's address: ", 1, 15, NULL);
		Printing::hex((uint32)object, 18, 15, 8, NULL);
	
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
#endif

			HardwareManager::resumeInterrupts();
			return object;
		}

		// Make my own virtual call, otherwise the macro will cause an infinite recursive call because of the
		// ObjectClass::getCast check
		baseClassGetClassMethod = (ClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object)(object);
	}

	if
	(
		NULL == baseClassGetClassMethod 
		|| 
		(
			(ClassPointer)&Object_getBaseClass == baseClassGetClassMethod 
			&&
			(ClassPointer)&Object_getBaseClass != targetClassGetClassMethod
		)
	)
	{
		lp = -1;
		sp = -1;

#ifdef __DEBUG
		counter = 0;
		Printing::setDebugMode();
		Printing::text("Object's address: ", 1, 15, NULL);
		Printing::text(__GET_CLASS_NAME(object), 18, 15, NULL);
		Printing::text("Target class: ", 1, 16, NULL);
		Printing::hex((uint32)targetClassGetClassMethod, 18, 16, 8, NULL);
	
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: failed cast");
#endif

		HardwareManager::resumeInterrupts();
		return NULL;
	}

	if(targetClassGetClassMethod == baseClassGetClassMethod)
	{
		lp = -1;
		sp = -1;

#ifdef __DEBUG
		counter = 0;
#endif

		HardwareManager::resumeInterrupts();
		return object;
	}

	HardwareManager::resumeInterrupts();

	return Object::getCast((Object)object, targetClassGetClassMethod, (ClassPointer)baseClassGetClassMethod(object));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Object::constructor()
{
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Object::destructor()
{
	// Free the memory
#ifndef __BYPASS_MEMORY_MANAGER_WHEN_DELETING
	MemoryPool::free((void*)((uint32)this - __DYNAMIC_STRUCT_PAD));
#else
	*((uint32*)((uint32)this - __DYNAMIC_STRUCT_PAD)) = __MEMORY_FREE_BLOCK_FLAG;
#endif

	this = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const void* Object::getVTable()
{
	return this->vTable;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Object::mutateTo(const void* targetClass)
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
		if
		(
			(ClassPointer)thisVTable->getBaseClass == baseClassGetClassMethod
			||
			(ClassPointer)thisVTable->getBaseClass(NULL) == baseClassGetClassMethod
		)
		{
			this->vTable = (void*)targetClassVTable;
			return true;
		}

		baseClassGetClassMethod = (ClassPointer)baseClassGetClassMethod(NULL);
	}

	baseClassGetClassMethod = (ClassPointer)thisVTable->getBaseClass(NULL);
	
	while((ClassPointer)Object::getBaseClass != (ClassPointer)baseClassGetClassMethod)
	{
		if
		(
			(ClassPointer)targetClassVTable->getBaseClass == baseClassGetClassMethod
			||
			(ClassPointer)targetClassVTable->getBaseClass(NULL) == baseClassGetClassMethod
		)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
