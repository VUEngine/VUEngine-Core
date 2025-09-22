/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Singletons' construction state flags
#define __SINGLETON_NOT_CONSTRUCTED			0
#define __SINGLETON_BEING_CONSTRUCTED		1
#define __SINGLETON_CONSTRUCTED				2

/// Checks that the provided authorization array lives in non volatile memory.
/// @param ClassName: Class to verity
/// @return Implementation of a check for the validity of the authorization array
#ifndef __SHIPPING
#define __SINGLETON_SECURITY_CHECKER(ClassName)																							\
																																		\
	extern uint32 _textStart __attribute__((unused));																					\
	extern uint32 _dataLma __attribute__((unused));																						\
																																		\
	if(!(&_textStart < (uint32*)requesterClasses && (uint32*)requesterClasses < &_dataLma))												\
	{																																	\
		Printer_setDebugMode();																											\
		Printer_clear();																												\
		Printer_text(#ClassName, 44, 25, NULL);																							\
		Printer_hex((uint32)requesterClasses, 44, 26, 8, NULL);																			\
		NM_ASSERT(false, ClassName ## initialize: the provided array lives in WRAM);													\
	}
#else
#define __SINGLETON_SECURITY_CHECKER(ClassName)
#endif

/// Checks that the provided authorization array lives in non volatile memory.
/// @param ClassName: Class to verity
/// @return Implementation of a check for the validity of the authorization array
#ifndef __RELEASE
#define __SINGLETON_AUTHORIZATION_CHECK(ClassName)																						\
																																		\
	_authorized = true;																													\
																																		\
	if(NULL != _authorizedRequesters && !ClassName ## _authorize(requesterClass))														\
	{																																	\
		_authorized = false;																											\
	}
#else
#define __SINGLETON_AUTHORIZATION_CHECK(ClassName)
#endif

#define __SINGLETON_ACCESS(ClassName)																									\
																																		\
	/* Array of authorized callers */																									\
	static ClassPointer const (*_authorizedRequesters)[] = NULL;																		\
																																		\
	/* Flag to authorize access to secure methods */																					\
	static bool _authorized __attribute__((unused)) = true;																				\
																																		\
	/* Define get instance method */																									\
	bool ClassName ## _authorize(ClassPointer requesterClass)																			\
	{																																	\
		for(int16 i = 0; NULL != (*_authorizedRequesters)[i]; i++)																		\
		{																																\
			if(requesterClass == (*_authorizedRequesters)[i])																			\
			{																															\
				return true;																											\
			}																															\
		}																																\
																																		\
		return typeofclass(ClassName) == requesterClass;																				\
	}																																	\
																																		\
	/* Define secure */																													\
	void ClassName ## _secure (ClassPointer const (*requesterClasses)[])																\
	{																																	\
		/* Check the validity of the provided array */																					\
		__SINGLETON_SECURITY_CHECKER(ClassName)																							\
																																		\
		if(NULL != _authorizedRequesters)																								\
		{																																\
			return;																														\
		}																																\
																																		\
		/* Register the provided array */																								\
		_authorizedRequesters = requesterClasses;																						\
	}
	
/// Defines a singleton class' fundamental methods.
/// @param ClassName: Singleton class' name to define
/// @return Implementation of a singleton class' fundamental methods
#define __SINGLETON(ClassName, MemorySection)																							\
																																		\
	/* declare the static instance */																									\
	typedef struct SingletonWrapper ## ClassName																						\
	{																																	\
		/* footprint to differentiate between objects and structs */																	\
		uint32 objectMemoryFootprint;																									\
		/* declare the static instance */																								\
		ClassName ## _str instance;																										\
	} SingletonWrapper ## ClassName;																									\
																																		\
	static SingletonWrapper ## ClassName _singletonWrapper ## ClassName 																\
			MemorySection;																												\
																																		\
	/* a flag to know when to allow construction */																						\
	static int8 _singletonConstructed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE														\
									= __SINGLETON_NOT_CONSTRUCTED;																		\
																																		\
	/* Define get instance method */																									\
	static void __attribute__ ((noinline)) ClassName ## _instantiate()																	\
	{																																	\
		NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,																\
			ClassName get instance during construction);																				\
																																		\
		_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;																			\
																																		\
		/* make sure that the class is properly set */																					\
		__CALL_CHECK_VTABLE(ClassName);																									\
																																		\
		/*  */																															\
		ClassName instance = &_singletonWrapper ## ClassName.instance;																	\
		_singletonWrapper ## ClassName.objectMemoryFootprint = 																			\
			(__OBJECT_MEMORY_FOOT_PRINT << 16) | -1;																					\
																																		\
		/* set the vtable pointer */																									\
		instance->vTable = &ClassName ## _vTable;																						\
																																		\
		/* call constructor */																											\
		ClassName ## _constructor(instance);																							\
																																		\
		/* set the vtable pointer */																									\
		instance->vTable = &ClassName ## _vTable;																						\
																																		\
		/* don't allow more constructs */																								\
		_singletonConstructed = __SINGLETON_CONSTRUCTED;																				\
	}																																	\
																																		\
	__SINGLETON_ACCESS(ClassName)																										\
																																		\
	/* Define get instance method */																									\
	__attribute__((unused)) ClassName ClassName ## _getInstance (ClassPointer requesterClass __attribute__((unused)))					\
	{																																	\
		/* Check if the call has higher privileges */																					\
		__SINGLETON_AUTHORIZATION_CHECK(ClassName)																						\
																																		\
		/* first check if not constructed yet */																						\
		if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)																		\
		{																																\
			ClassName ## _instantiate();																								\
		}																																\
																																		\
		/* return the created singleton */																								\
		return &_singletonWrapper ## ClassName.instance;																				\
	}																																	\
																																		\
	/* dummy redeclaration to avoid warning when compiling with -pedantic */															\
	void ClassName ## dummyMethodSingleton()

/// Call's the singleton's base class' destructor and allows a new instantiation.
/// @return Code to call the singleton class' base's destructor and to pull down the flag that prevents
/// a new instance being created
#define __SINGLETON_DESTROY																												\
																																		\
	/* destroy super object */																											\
	__DESTROY_BASE;																														\
																																		\
	/* allow new constructs */																											\
	_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;																				\

/// Defines a dynamically allocated singleton class' fundamental methods.
/// @param ClassName: Singleton class' name to define
/// @return Implementation of a singleton class' fundamental methods
#define __SINGLETON_DYNAMIC(ClassName)																									\
																																		\
	/* declare the static pointer to instance */																						\
	static ClassName _instance ## ClassName __NON_INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;											\
																																		\
	/* Define allocator */																												\
	__CLASS_NEW_DEFINITION(ClassName, void)																								\
	__CLASS_NEW_END(ClassName, this);																									\
																																		\
	/* a flag to know when to allow construction */																						\
	static int8 _singletonConstructed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE														\
									= __SINGLETON_NOT_CONSTRUCTED;																		\
																																		\
	/* Define get instance method */																									\
	static void __attribute__ ((noinline)) ClassName ## _instantiate()																	\
	{																																	\
		NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,																\
			ClassName get instance during construction);																				\
																																		\
		_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;																			\
																																		\
		/* allocate */																													\
		_instance ## ClassName = ClassName ## _new();																					\
																																		\
		/* don't allow more constructs */																								\
		_singletonConstructed = __SINGLETON_CONSTRUCTED;																				\
	}																																	\
																																		\
	__SINGLETON_ACCESS(ClassName)																										\
																																		\
	/* Define get instance method */																									\
	__attribute__((unused)) ClassName ClassName ## _getInstance (ClassPointer requesterClass __attribute__((unused)))					\
	{																																	\
		/* Check if the call has higher privileges */																					\
		__SINGLETON_AUTHORIZATION_CHECK(ClassName)																						\
																																		\
		/* first check if not constructed yet */																						\
		if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)																		\
		{																																\
			ClassName ## _instantiate();																								\
		}																																\
																																		\
		/* return the created singleton */																								\
		return _instance ## ClassName;																									\
	}																																	\
																																		\
	/* dummy redeclaration to avoid warning when compiling with -pedantic */															\
	void ClassName ## dummyMethodSingletonNew()

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Singleton
///
/// Inherits from Object
///
/// Catches assertions and hardware exceptions.
static class Singleton : Object
{
	/// @publicsection

	/// Secure the singleton classes.
	static void secure();
}

#endif
