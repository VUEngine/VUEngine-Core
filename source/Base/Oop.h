/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OOP_H_
#define OOP_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// OOP MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// This file contains the core of the OOP support

/// Convert a literal into a string.
/// @param a: Literal to stringify
#define __MAKE_STRING(a) #a

/// Concatenate two strings
/// @param str_1: String to concatenate at the start
/// @param str_2: String to concatenate at the end
/// @return Concatenated string: str_1str_2
#define __MAKE_CONCAT(str_1,str_2) str_1 ## str_2

/// Concatenate two strings
/// @param str_1: String to concatenate at the start
/// @param str_2: String to concatenate at the end
/// @return Concatenated string: str_1str_2
#define __CUSTOM_CONCAT(str_1,str_2) __MAKE_CONCAT(str_1,str_2)

/// Remove class name printing in shipping releases.
/// @param value: Class' name string
/// @return Stringified value
#ifdef __SHIPPING
#define __OBFUSCATE_NAME(value)			("ClassName")
#else
#define __OBFUSCATE_NAME(value)			(#value)
#endif

/// Optimized memory free in release mode,
#ifndef __RELEASE
#undef __BYPASS_MEMORY_MANAGER_WHEN_DELETING
#else
#define __BYPASS_MEMORY_MANAGER_WHEN_DELETING
#endif

/// Flag to mark a memory block as used by an object, as opposed to a simple struct
#define __OBJECT_MEMORY_FOOT_PRINT		(uint16)(__MEMORY_USED_BLOCK_FLAG + sizeof(uint16) * 8)

/// Define the provided class' new method
/// @param ClassName: Name of the class to define
/// @param ...: Class' new allocator's parameters
/// @return Method definitions of the provided class
#define __CLASS_NEW_DEFINITION(ClassName, ...)																							\
																																		\
		/* Define the method */																											\
		ClassName ClassName ## _new(__VA_ARGS__)																						\
		{																																\
			/* make sure that the class is properly set */																				\
			__CALL_CHECK_VTABLE(ClassName);																								\
																																		\
			/* allocate object */																										\
			uint16* memoryBlock = (uint16*)MemoryPool_allocate(				 															\
							sizeof(ClassName ## _str) + __DYNAMIC_STRUCT_PAD);															\
																																		\
			/* mark memory block as used by an object */																				\
			*memoryBlock = __OBJECT_MEMORY_FOOT_PRINT;																					\
																																		\
			/* this pointer lives __DYNAMIC_STRUCT_PAD ahead */																			\
			ClassName this = (ClassName)((uint32)memoryBlock + __DYNAMIC_STRUCT_PAD);													\
																																		\
			/* check if properly created */																								\
			ASSERT(this, __MAKE_STRING(ClassName) "::new: not allocated");																\

/// End class's new method's definition
/// @param ClassName: Name of the class to define
/// @param ...: Class' new allocator's arguments
/// @return Method definitions of the provided class
#define __CLASS_NEW_END(ClassName, ...)																									\
																																		\
			/* set the vtable pointer */																								\
			this->vTable = (void*)&ClassName ## _vTable;																				\
																																		\
			/* construct the object */																									\
			ClassName ## _constructor(__VA_ARGS__);																						\
																																		\
			ASSERT(this->vTable == &ClassName ## _vTable,																				\
				__MAKE_STRING(ClassName) "::new: vTable not set properly");																\
																																		\
			/* return the created object */																								\
			return this;																												\
		}																																\
																																		\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */														\
		void ClassName ## dummyMethodClassNew()

/// Padding to be adding to dynamic memory allocation requests
#define	__DYNAMIC_STRUCT_PAD	sizeof(uint32)

/// Our version of C++' new for object allocation
/// @param ClassName: Name of the class to instantiate
/// @param ...: Class' constructor arguments
/// @return Call to ClassName::new 
#define __NEW(ClassName, ...)																											\
																																		\
		/* call class's new implementation */																							\
		ClassName ## _new(__VA_ARGS__)																									\

/// Our version of C++' new for dynamic struct allocation
/// @param StructName: Name of the struct to define
/// @param ...: Class' new allocator's parameters
/// @return Call to allocation request to the MemoryPool 
#define __NEW_BASIC(StructName)																											\
																																		\
		/* allocate data */																												\
		(StructName*)((uint32)MemoryPool_allocate(																						\
			sizeof(StructName) + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);															\

/// Our version of C++' delete (calls virtual destructor)
/// @param objectToDelete: Object to delete
/// @return Call to ClassName::destructor or deallocation request to MemoryPool 
#ifndef __BYPASS_MEMORY_MANAGER_WHEN_DELETING
#define __DELETE(objectToDelete)																										\
	{																																	\
		void* object = (void*)objectToDelete;																							\
																																		\
		HardwareManager_suspendInterrupts();																							\
																																		\
		if(__OBJECT_MEMORY_FOOT_PRINT == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))												\
		{																																\
			/* since the destructor is the first element in the virtual table */														\
			ASSERT(object && *(uint32*)object, "Deleting null object");																	\
			((((struct Object ## _vTable*)((*((void**)object))))->destructor))((Object)object);											\
		}																																\
		else if(__MEMORY_USED_BLOCK_FLAG == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))											\
		{																																\
			ASSERT(object && *(uint32*)((uint32)object - __DYNAMIC_STRUCT_PAD), 														\
				"Oop: deleting null basic object");																						\
			MemoryPool_free((BYTE*)((uint32)object - __DYNAMIC_STRUCT_PAD));															\
		}																																\
		else 																															\
		{																																\
			NM_ASSERT(false, "Oop: deleting something not dynamically allocated");														\
		}																																\
																																		\
		HardwareManager_resumeInterrupts();																								\
																																		\
	}
#else
#define __DELETE(objectToDelete)																										\
	{																																	\
		void* object = (void*)objectToDelete;																							\
																																		\
		HardwareManager_suspendInterrupts();																							\
																																		\
		if(__OBJECT_MEMORY_FOOT_PRINT == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))												\
		{																																\
			((((struct Object ## _vTable*)((*((void**)object))))->destructor))((Object)object);											\
		}																																\
		else if(__MEMORY_USED_BLOCK_FLAG == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))											\
		{																																\
			*(uint32*)((uint32)object - __DYNAMIC_STRUCT_PAD) = __MEMORY_FREE_BLOCK_FLAG;												\
		}																																\
		else 																															\
		{																																\
			NM_ASSERT(false, "Oop: deleting something not dynamically allocated");														\
		}																																\
																																		\
		HardwareManager_resumeInterrupts();																								\
																																		\
	}
#endif

/// Call the base class' constructor.
/// @param BaseClass: Name of base class whose constructor must be called
/// @param ...: Base class' constructor's parameters
/// @return Call to the base class' constructor
#define __CONSTRUCT_BASE(BaseClass, ...)																								\
																																		\
		BaseClass ## _constructor((BaseClass)__VA_ARGS__);																				\

/// Call the base class' destructor.
/// @return Call to the class' destructor
#define __DESTROY_BASE																													\
																																		\
		_baseDestructor(__SAFE_CAST(Object, this));																						\

/// Retrieve the virtual method's address of a method according to the provided object.
/// @param ClassName: Class' name
/// @param MethodName: Method whose address is to be retrieved
/// @param object: Object on which to call the method
/// @return Address of the virtual method
#define __VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object)																			\
																																		\
		/* call derived implementation */																								\
		(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName)																\

/// Helper macros to work around the extra comma in variadic macros
/// @param object: Object to rewrite
/// @param ...: Arguments to throw away
/// @return Sanitized arguments for a virtual method call
#define __VIRTUAL_CALL_FIRST_HELPER(object, ...) object
#define __VIRTUAL_CALL_OBJECT(...) __VIRTUAL_CALL_FIRST_HELPER(__VA_ARGS__, throwaway)

/// Call a virtual method.
/// @param ClassName: Class name
/// @param MethodName: Method to call
/// @param ...: Arguments to pass to the method
/// @return Call to virtual method
#define __VIRTUAL_CALL(ClassName, MethodName, ...)																						\
																																		\
		((((struct ClassName ## _vTable*)((*((void**)__VIRTUAL_CALL_OBJECT(__VA_ARGS__)))))->MethodName))								\
		(																																\
			(ClassName)__VA_ARGS__																										\
		)

/// Call the method's base implementation.
/// @param BaseClassName: Base class' name
/// @param MethodName: Method to call
/// @param ...: Arguments to pass to the method
/// @return Call to the method's base implementation
#define __CALL_BASE_METHOD(BaseClassName, MethodName, ...)																				\
																																		\
		BaseClassName ## _vTable.MethodName																								\
		(																																\
			(BaseClassName)__VA_ARGS__																									\
		)

/// Cast an object to the provided class. It decays to an usafe C cast in non debug build modes.
/// @param ClassName: Class to cast to
/// @param object: Object to cast
/// @return Cast result: NULL or valid pointer
#ifdef __DEBUG
#define __SAFE_CAST(ClassName, object)																									\
																																		\
		(ClassName)Object_getCast((Object)object, (ClassPointer)&ClassName ## _getBaseClass, NULL)
#else
#define __SAFE_CAST(ClassName, object) (ClassName)object
#endif

/// Check if an object is an instance of a class that overrides the provided method.
/// This is used to bypass late dispatching when performance is at premium.
/// @param ClassName: Class name
/// @param object: Oject to check if overrides the method
/// @param MethodName: Method to check if overrode
/// @return True if the object's class overrides the provided method; false otherwise
#define __OVERRIDES_METHOD(ClassName, object, MethodName) 																				\
																																		\
		((void (*)())&ClassName ## _ ## MethodName != 																					\
			(void (*)())__VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object))

/// Check if an object is not destroyed.
/// @param object: Oject to check
/// @return True if the object is not NULL nor destroyed; false otherwise
#define __IS_OBJECT_ALIVE(object)																										\
																																		\
		(object && (__MEMORY_FREE_BLOCK_FLAG != *(uint32*)((uint32)object - __DYNAMIC_STRUCT_PAD)))										\

/// Check if an object is not destroyed.
/// @param object: Oject to check
/// @return True if the object is not NULL nor destroyed; false otherwise
#define isDeleted(object)					(!__IS_OBJECT_ALIVE(object))

/// Retrieve the class pointer of the class that he provided object is an instance of. 
/// @param ClassName: Class name to retrieve its type
/// @return Pointer to the class' identifying function pointer
#define typeofclass(ClassName)		((ClassPointer)&ClassName ## _getBaseClass)

/// Try to cast an object to the  provided class.
/// @param ClassName: Class to cast to
/// @param object: Oject to cast
/// @return Cast result: NULL or valid pointer
#define __GET_CAST(ClassName, object)																									\
																																		\
		(ClassName)Object_getCast((Object)object,																						\
			(ClassPointer)&ClassName ## _getBaseClass, NULL)																			\

/// Check if an object is an instance of the provided class by comparing virtual table pointers.
/// @param ClassName: Class to check
/// @param object: Oject to check
/// @return True if the object is an instance of the provided class
#define __IS_INSTANCE_OF(ClassName, object)																								\
																																		\
		(void*)&ClassName ## _vTable == (void*)*((void**)object)																		\

/// Declare a method as virtual.
/// @param ClassName: Class that owns the method
/// @param ReturnType: Method's return type
/// @param MethodName: Method to declare as virtual
/// @param ...: Method's parameters
/// @return Virtual method declaration
#define __VIRTUAL_DEC(ClassName, ReturnType, MethodName, ...)																			\
																																		\
		ReturnType (*MethodName)(__VA_ARGS__)																							\

/// Override a virtual method
/// @param ClassVTable: Class' virtual table's pointer
/// @param ClassName: Class that owns the method
/// @param MethodName: Method to override
/// @return Virtual method override
#define __VIRTUAL_SET(ClassVTable, ClassName, MethodName)																				\
		{																																\
			/* Use a temporary pointer to avoid illegal cast between pointers to data and functions */									\
			void (*(*tempPointer))() = (void (*(*))())&ClassVTable ## _vTable.MethodName;												\
			*(tempPointer) = (void (*)())&ClassName ## _ ## MethodName;																	\
		}

/// Mutate a class' method.
/// @param ClassName: Class that owns the method
/// @param MethodName: Method to mutate
/// @param NewMethod: Method's new implementation
#define __CLASS_MUTATE_METHOD(ClassName, MethodName, NewMethod)																			\
		{																																\
			/* Use a temporary pointer to avoid illegal cast between pointers to data and functions */									\
			void (*(*tempPointer))() = (void (*(*))())&ClassName ## _vTable.MethodName;													\
			*(tempPointer) = (void (*)())NewMethod;																						\
		}

/// Check a class's vtable's integrity.
/// @param ClassName: Class whose virtual table is to being checked
/// @return Implementation for the class' virtual table's check
#define __CHECK_VTABLE_DEFINITION(ClassName)																							\
																																		\
		void __attribute__ ((noinline)) ClassName ## _checkVTable()																		\
		{																																\
			/* setup the class's vtable only if destructor is NULL */																	\
			NM_ASSERT(ClassName ## _vTable.destructor, ClassName ## vTable not properly set. 											\
			Delete the GAME/build/working/setupClasses.c file);																			\
																																		\
			/* check that no method is null */																							\
			uint32 i = 0;																												\
			for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)														\
			{																															\
				NM_ASSERT(((void (*(*))())&ClassName ## _vTable)[i], 																	\
					__MAKE_STRING(ClassName) " is abstract");																			\
			}																															\
		}
#ifndef __RELEASE
#define __CALL_CHECK_VTABLE(ClassName)		ClassName ## _checkVTable()
#else
#define __CALL_CHECK_VTABLE(ClassName)
#endif

/// Configure a class's vtable
/// @param ClassName: Class whose virtual table is to being configured
/// @param BaseClassName: Base class' from which the class inherits
/// @return Implementation of the class' virtual table's configuration method
#define __SET_VTABLE_DEFINITION(ClassName, BaseClassName)																				\
																																		\
		/* Define the static method */																									\
		void __attribute__ ((noinline)) ClassName ## _setVTable(bool force)																\
		{																																\
			/* setup the class's vtable only if destructor is NULL */																	\
			if(!force && ClassName ## _vTable.destructor)																				\
			{																															\
				return;																													\
			}																															\
																																		\
			/* clean up the vtable */																									\
			uint32 i = 0;																												\
			for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)														\
			{																															\
				((void (*(*))())&ClassName ## _vTable)[i] = NULL;																		\
			}																															\
																																		\
			/* set the class's virtual methods */																						\
			ClassName ## _SET_VTABLE(ClassName)																							\
																																		\
			/* set the destructor */																									\
			__VIRTUAL_SET(ClassName, ClassName, destructor);																			\
																																		\
			/* set the getBaseClass method */																							\
			__VIRTUAL_SET(ClassName, ClassName, getBaseClass);																			\
																																		\
			/* set the getClassName method */																							\
			__VIRTUAL_SET(ClassName, ClassName, getClassName);																			\
		}																																\

/// Declare a class's vtable.
/// @param ClassName: Class whose virtual table is to being declared
/// @return Class' virtual table's declaration
#define __VTABLE(ClassName)																												\
																																		\
		/* declare the vtable struct */																									\
		struct ClassName ## _vTable 																									\
		{																																\
			/* all destructors are virtual */																							\
			__VIRTUAL_DEC(ClassName, void, destructor, ClassName);																		\
																																		\
			/* get super class method */																								\
			__VIRTUAL_DEC(ClassName, ClassPointer, getBaseClass, ClassName);															\
																																		\
			/* all destructors are virtual */																							\
			__VIRTUAL_DEC(ClassName, const char*, getClassName, ClassName);																\
																																		\
			/* insert class's virtual methods names */																					\
			ClassName ## _METHODS(ClassName)																							\
		};																																\
																																		\
		/* declare virtual table for external reference */																				\
		extern struct ClassName ## _vTable ClassName ## _vTable																			\

/// Forward declare a class.
/// @param ClassName: Class being forward declared
/// @return Class' forward declaration
#define __FORWARD_CLASS(ClassName)																										\
		/* declare a pointer */																											\
		typedef struct ClassName ## _str* ClassName																						\

/// typedef for RTTI
typedef void* (*(*ClassPointer)(void*))(void*);

/// Declare a class.
/// @param ClassName: Class being declared
/// @return Class' declaration
#define __CLASS(ClassName)																												\
																																		\
		/* declare vtable */																											\
		__VTABLE(ClassName);																											\
																																		\
		/* declare vtable */																											\
		void ClassName ## _setVTable(bool force);																						\
																																		\
		/* get class */																													\
		const void* ClassName ## _getClass();																							\
																																		\
		/* declare getSize method */																									\
		int32 ClassName ## _getObjectSize();																							\
																																		\
		/* declare getBaseClass method */																								\
		ClassPointer ClassName ## _getBaseClass(void*);																					\
																																		\
		/* declare getClass name method */																								\
		const char* ClassName ## _getClassName(ClassName);																				\
																																		\
		/* declare restoreMethods name method */																						\
		void ClassName ## _restoreMethods()																								\

/// Copy a class' declaration to make its member accessible to a compilation unit.
/// @param ClassName: Class being friended
/// @return Class' struct declaration
#define __CLASS_FRIEND_DEFINITION(ClassName)																							\
		typedef struct ClassName ## _str																								\
		{																																\
			/* class attributes */																										\
			ClassName ## _ATTRIBUTES																									\
																																		\
			/* end spec */																												\
		} ClassName ## _str 																											\

/// Retrieve the memory footprint of a give class' instances.
/// @param ClassName: Class whose instances' size is to be retrieved
/// @return Implementation of the class' method to retrive the size of its instances
#ifndef __RELEASE
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)																						\
																																		\
		int32 ClassName ## _getObjectSize()																								\
		{																																\
			return sizeof(ClassName ## _str);																							\
		}
#else
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)
#endif

/// Define the fundamental methods of a class.
/// @param ClassName: Class whose methods are defined
/// @param BaseClassName: Base class' from which the class inherits
/// @return Class' fundamental method's definition
#define __CLASS_DEFINITION(ClassName, BaseClassName)																					\
																																		\
		typedef struct ClassName ## _str																								\
		{																																\
			/* class attributes */																										\
			ClassName ## _ATTRIBUTES																									\
																																		\
			/* end spec */																												\
		} ClassName ## _str;																											\
																																		\
		/* class' vtable's spec */																										\
		struct ClassName ## _vTable ClassName ## _vTable __VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE;										\
																																		\
		static void (* const _baseDestructor)(Object) =																					\
		/* class' base's destructor */																									\
			(void (*)(Object))&BaseClassName ## _destructor;																			\
																																		\
		/* Define class's getSize method */																								\
		__GET_INSTANCE_SIZE_DEFINITION(ClassName)																						\
																																		\
		/* Define class's getBaseClass method */																						\
		ClassPointer ClassName ## _getBaseClass(void* this __attribute__ ((unused)))													\
		{																																\
			ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,														\
					"Wrong class spec: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "												\
					__MAKE_STRING(BaseClassName) ")");																					\
			return (ClassPointer)&BaseClassName ## _getBaseClass;																		\
		}																																\
																																		\
		/* Define class's getSize method */																								\
		const char* ClassName ## _getClassName(ClassName this __attribute__ ((unused)))													\
		{																																\
			ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,														\
					"Wrong class spec: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "												\
					__MAKE_STRING(BaseClassName) ")");																					\
			return (char*)__OBFUSCATE_NAME(ClassName);																					\
		}																																\
																																		\
		/* restore class vTable */																										\
		void ClassName ## _restoreMethods()																								\
		{																																\
			/* must prevent any interrupt from calling methods on unstable vtables */													\
			HardwareManager_suspendInterrupts();																						\
																																		\
			ClassName ## _setVTable(true);																								\
																																		\
			/* resume interrupts */																										\
			HardwareManager_resumeInterrupts();																							\
		}																																\
																																		\
		const void* ClassName ## _getClass()																							\
		{																																\
			return (const void*)&ClassName ## _vTable;																					\
		}																																\
																																		\
		/* now add the function which will handle the vtable */																			\
		__SET_VTABLE_DEFINITION(ClassName, BaseClassName)																				\
		__CHECK_VTABLE_DEFINITION(ClassName)																							\

/// Retrieve the name of the class of an object.
/// @param object: Object from which to retrieve its class' name
/// @return Call to the method to retrieve the object's class' name
#define __GET_CLASS_NAME(object)																										\
																																		\
		__VIRTUAL_CALL(Object, getClassName, (Object)object)

/// Replace a class name by the pointer of its instances allocator.
/// @param ClassName: Class from which to retrieve its allocator's address
/// @return Address of the class' allocator
#define __TYPE(ClassName)								(AllocatorPointer)&ClassName ## _new

/// Cast the provided pointer to AllocatorPointer.
/// @param allocatorPointer: Pointer to be casted
/// @return Casted pointer
#define __ALLOCATOR_TYPE(allocatorPointer)				(AllocatorPointer)allocatorPointer

/// Singletons' construction state flags
#define __SINGLETON_NOT_CONSTRUCTED			0
#define __SINGLETON_BEING_CONSTRUCTED		1
#define __SINGLETON_CONSTRUCTED				2

/// Checks that the provided authorization array lives in non volatile memory.
/// @param ClassName: Class to verity
/// @return Implementation of a check for the validity of the authorization array
#ifdef __SHIPPING
#define __SINGLETON_SECURITY_CHECKER(ClassName)																							\
																																		\
	extern uint32 _textStart __attribute__((unused));																					\
	extern uint32 _dataLma __attribute__((unused));																						\
																																		\
	if(!(&_textStart < (uint32*)requesterClasses && (uint32*)requesterClasses < &_dataLma))												\
	{																																	\
		Printing_setDebugMode();																										\
		Printing_clear();																												\
		Printing_text(#ClassName, 44, 25, NULL);																						\
		Printing_hex((WORD)requesterClasses, 44, 26, 8, NULL);																			\
		NM_ASSERT(false, ClassName ## initialize: the provided array lives in WRAM);													\
	}
#else
#define __SINGLETON_SECURITY_CHECKER(ClassName)
#endif

#define __SINGLETON_ACCESS(ClassName, GetInstantLinkage)																				\
																																		\
		/* Define get instance method */																								\
		bool ClassName ## _authorize(ClassPointer requesterClass)																		\
		{																																\
			for(int16 i = 0; NULL != (*_authorizedRequesters)[i]; i++)																	\
			{																															\
				if(requesterClass == (*_authorizedRequesters)[i])																		\
				{																														\
					return true;																										\
				}																														\
			}																															\
																																		\
			return false;																												\
		}																																\
																																		\
		/* Define get instance method */																								\
		GetInstantLinkage __attribute__((unused)) ClassName ClassName ## _getInstance (ClassPointer requesterClass)						\
		{																																\
			if(NULL != _authorizedRequesters && !ClassName ## _authorize(requesterClass))												\
			{																															\
				NM_ASSERT(false, Unauthorized access to ClassName singleton);															\
				return NULL;																											\
			}																															\
																																		\
			/* first check if not constructed yet */																					\
			if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)																	\
			{																															\
				ClassName ## _instantiate();																							\
			}																															\
																																		\
			/* return the created singleton */																							\
			return &_singletonWrapper ## ClassName.instance;																			\
		}																																\
																																		\
		/* Define secure */																												\
		void ClassName ## _secure (ClassPointer const (*requesterClasses)[])															\
		{																																\
			/* Check the validity of the provided array */																				\
			__SINGLETON_SECURITY_CHECKER(ClassName)																						\
																																		\
			if(NULL != _authorizedRequesters)																							\
			{																															\
				return;																													\
			}																															\
																																		\
			/* Register the provided array */																							\
			_authorizedRequesters = requesterClasses;																					\
		}																																\


/// Defines a singleton class' fundamental methods.
/// @param ClassName: Singleton class' name to define
/// @return Implementation of a singleton class' fundamental methods
#define __SINGLETON(ClassName, ...)																										\
																																		\
		/* declare the static instance */																								\
		typedef struct SingletonWrapper ## ClassName																					\
		{																																\
			/* footprint to differentiate between objects and structs */																\
			uint32 objectMemoryFootprint;																								\
			/* declare the static instance */																							\
			ClassName ## _str instance;																									\
		} SingletonWrapper ## ClassName;																								\
																																		\
		/* Array of authorized callers */																								\
		static ClassPointer const (*_authorizedRequesters)[] = NULL;																	\
																																		\
		static SingletonWrapper ## ClassName _singletonWrapper ## ClassName 															\
				__STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE;																				\
																																		\
		/* a flag to know when to allow construction */																					\
		static int8 _singletonConstructed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE													\
										= __SINGLETON_NOT_CONSTRUCTED;																	\
																																		\
		/* Define get instance method */																								\
		static void __attribute__ ((noinline)) ClassName ## _instantiate()																\
		{																																\
			NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,															\
				ClassName get instance during construction);																			\
																																		\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;																		\
																																		\
			/* make sure that the class is properly set */																				\
			__CALL_CHECK_VTABLE(ClassName);																								\
																																		\
			/*  */																														\
			ClassName instance = &_singletonWrapper ## ClassName.instance;																\
			_singletonWrapper ## ClassName.objectMemoryFootprint = 																		\
				(__OBJECT_MEMORY_FOOT_PRINT << 16) | -1;																				\
																																		\
			/* set the vtable pointer */																								\
			instance->vTable = &ClassName ## _vTable;																					\
																																		\
			/* call constructor */																										\
			ClassName ## _constructor(instance);																						\
																																		\
			/* set the vtable pointer */																								\
			instance->vTable = &ClassName ## _vTable;																					\
																																		\
			/* don't allow more constructs */																							\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;																			\
		}																																\
																																		\
		__SINGLETON_ACCESS(ClassName, __VA_ARGS__)																						\
																																		\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */														\
		void ClassName ## dummyMethodSingleton()

/// Call's the singleton's base class' destructor and allows a new instantiation.
/// @return Code to call the singleton class' base's destructor and to pull down the flag that prevents
/// a new instance being created
#define __SINGLETON_DESTROY																												\
																																		\
		/* destroy super object */																										\
		__DESTROY_BASE;																													\
																																		\
		/* allow new constructs */																										\
		_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;																			\

/// Defines a dynamically allocated singleton class' fundamental methods.
/// @param ClassName: Singleton class' name to define
/// @return Implementation of a singleton class' fundamental methods
#define __SINGLETON_DYNAMIC(ClassName)																									\
																																		\
		/* declare the static pointer to instance */																					\
		static ClassName _instance ## ClassName __NON_INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;										\
																																		\
		/* Define allocator */																											\
		__CLASS_NEW_DEFINITION(ClassName, void)																							\
		__CLASS_NEW_END(ClassName, this);																								\
																																		\
		/* a flag to know when to allow construction */																					\
		static int8 _singletonConstructed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE													\
										= __SINGLETON_NOT_CONSTRUCTED;																	\
																																		\
		/* Define get instance method */																								\
		static void __attribute__ ((noinline)) ClassName ## _instantiate()																\
		{																																\
			NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,															\
				ClassName get instance during construction);																			\
																																		\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;																		\
																																		\
			/* allocate */																												\
			_instance ## ClassName = ClassName ## _new();																				\
																																		\
			/* don't allow more constructs */																							\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;																			\
		}																																\
																																		\
		/* Define get instance method */																								\
		__attribute__((unused)) ClassName ClassName ## _getInstance (ClassPointer requesterClass)										\
		{																																\
			/* first check if not constructed yet */																					\
			if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)																	\
			{																															\
				ClassName ## _instantiate();																							\
			}																															\
																																		\
			/* return the created singleton */																							\
			return _instance ## ClassName;																								\
		}																																\
																																		\
		__SINGLETON_ACCESS(ClassName, __VA_ARGS__)																						\
																																		\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */														\
		void ClassName ## dummyMethodSingletonNew()

#endif
