/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_H_
#define OBJECT_H_

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
	/* Define the method */																												\
	ClassName ClassName ## _new(__VA_ARGS__)																							\
	{																																	\
		/* make sure that the class is properly set */																					\
		__CALL_CHECK_VTABLE(ClassName);																									\
																																		\
		/* allocate object */																											\
		uint16* memoryBlock = (uint16*)MemoryPool_allocate(				 																\
						sizeof(ClassName ## _str) + __DYNAMIC_STRUCT_PAD);																\
																																		\
		/* mark memory block as used by an object */																					\
		*memoryBlock = __OBJECT_MEMORY_FOOT_PRINT;																						\
																																		\
		/* this pointer lives __DYNAMIC_STRUCT_PAD ahead */																				\
		ClassName this = (ClassName)((uint32)memoryBlock + __DYNAMIC_STRUCT_PAD);														\
																																		\
		/* check if properly created */																									\
		ASSERT(this, __MAKE_STRING(ClassName) "::new: not allocated");																	\

/// End class's new method's definition
/// @param ClassName: Name of the class to define
/// @param ...: Class' new allocator's arguments
/// @return Method definitions of the provided class
#define __CLASS_NEW_END(ClassName, ...)																									\
																																		\
		/* set the vtable pointer */																									\
		this->vTable = (void*)&ClassName ## _vTable;																					\
																																		\
		/* construct the object */																										\
		ClassName ## _constructor(__VA_ARGS__);																							\
																																		\
		ASSERT(this->vTable == &ClassName ## _vTable,																					\
			__MAKE_STRING(ClassName) "::new: vTable not set properly");																	\
																																		\
		/* return the created object */																									\
		return this;																													\
	}																																	\
																																		\
	/* dummy redeclaration to avoid warning when compiling with -pedantic */															\
	void ClassName ## dummyMethodClassNew()

/// Padding to be adding to dynamic memory allocation requests
#define	__DYNAMIC_STRUCT_PAD	sizeof(uint32)

/// Our version of C++' new for object allocation
/// @param ClassName: Name of the class to instantiate
/// @param ...: Class' constructor arguments
/// @return Call to ClassName::new 
#define __NEW(ClassName, ...)																											\
																																		\
	/* call class's new implementation */																								\
	ClassName ## _new(__VA_ARGS__)																										\

/// Our version of C++' new for dynamic struct allocation
/// @param StructName: Name of the struct to define
/// @param ...: Class' new allocator's parameters
/// @return Call to allocation request to the MemoryPool 
#define __NEW_BASIC(StructName)																											\
																																		\
	/* allocate data */																													\
	(StructName*)((uint32)MemoryPool_allocate(																							\
		sizeof(StructName) + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);																\

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
	BaseClass ## _constructor((BaseClass)__VA_ARGS__);																					\

/// Call the base class' destructor.
/// @return Call to the class' destructor
#define __DESTROY_BASE																													\
																																		\
	_baseDestructor(__SAFE_CAST(Object, this));																							\

/// Retrieve the virtual method's address of a method according to the provided object.
/// @param ClassName: Class' name
/// @param MethodName: Method whose address is to be retrieved
/// @param object: Object on which to call the method
/// @return Address of the virtual method
#define __VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object)																			\
																																		\
	/* call derived implementation */																									\
	(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName)																	\

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
	((((struct ClassName ## _vTable*)((*((void**)__VIRTUAL_CALL_OBJECT(__VA_ARGS__)))))->MethodName))									\
	(																																	\
		(ClassName)__VA_ARGS__																											\
	)

/// Call the method's base implementation.
/// @param BaseClassName: Base class' name
/// @param MethodName: Method to call
/// @param ...: Arguments to pass to the method
/// @return Call to the method's base implementation
#define __CALL_BASE_METHOD(BaseClassName, MethodName, ...)																				\
																																		\
	BaseClassName ## _vTable.MethodName																									\
	(																																	\
		(BaseClassName)__VA_ARGS__																										\
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
	((void (*)())&ClassName ## _ ## MethodName != 																						\
		(void (*)())__VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object))

/// Check if an object is not destroyed.
/// @param object: Oject to check
/// @return True if the object is not NULL nor destroyed; false otherwise
#define __IS_OBJECT_ALIVE(object)																										\
																																		\
	(object && (__MEMORY_FREE_BLOCK_FLAG != *(uint32*)((uint32)object - __DYNAMIC_STRUCT_PAD)))											\

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
	(ClassName)Object_getCast((Object)object,																							\
		(ClassPointer)&ClassName ## _getBaseClass, NULL)																				\

/// Check if an object is an instance of the provided class by comparing virtual table pointers.
/// @param ClassName: Class to check
/// @param object: Oject to check
/// @return True if the object is an instance of the provided class
#define __IS_INSTANCE_OF(ClassName, object)																								\
																																		\
	(void*)&ClassName ## _vTable == (void*)*((void**)object)																			\

/// Declare a method as virtual.
/// @param ClassName: Class that owns the method
/// @param ReturnType: Method's return type
/// @param MethodName: Method to declare as virtual
/// @param ...: Method's parameters
/// @return Virtual method declaration
#define __VIRTUAL_DEC(ClassName, ReturnType, MethodName, ...)																			\
																																		\
	ReturnType (*MethodName)(__VA_ARGS__)																								\

/// Override a virtual method
/// @param ClassVTable: Class' virtual table's pointer
/// @param ClassName: Class that owns the method
/// @param MethodName: Method to override
/// @return Virtual method override
#define __VIRTUAL_SET(ClassVTable, ClassName, MethodName)																				\
	{																																	\
		/* Use a temporary pointer to avoid illegal cast between pointers to data and functions */										\
		void (*(*tempPointer))() = (void (*(*))())&ClassVTable ## _vTable.MethodName;													\
		*(tempPointer) = (void (*)())&ClassName ## _ ## MethodName;																		\
	}

/// Mutate a class' method.
/// @param ClassName: Class that owns the method
/// @param MethodName: Method to mutate
/// @param NewMethod: Method's new implementation
#define __CLASS_MUTATE_METHOD(ClassName, MethodName, NewMethod)																			\
	{																																	\
		/* Use a temporary pointer to avoid illegal cast between pointers to data and functions */										\
		void (*(*tempPointer))() = (void (*(*))())&ClassName ## _vTable.MethodName;														\
		*(tempPointer) = (void (*)())NewMethod;																							\
	}

/// Check a class's vtable's integrity.
/// @param ClassName: Class whose virtual table is to being checked
/// @return Implementation for the class' virtual table's check
#define __CHECK_VTABLE_DEFINITION(ClassName)																							\
																																		\
	void __attribute__ ((noinline)) ClassName ## _checkVTable()																			\
	{																																	\
		/* setup the class's vtable only if destructor is NULL */																		\
		NM_ASSERT(ClassName ## _vTable.destructor, ClassName ## vTable not properly set. 												\
		Delete the GAME/build/working/setupClasses.c file);																				\
																																		\
		/* check that no method is null */																								\
		uint32 i = 0;																													\
		for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)															\
		{																																\
			NM_ASSERT(((void (*(*))())&ClassName ## _vTable)[i], 																		\
				__MAKE_STRING(ClassName) " is abstract");																				\
		}																																\
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
	/* Define the static method */																										\
	void __attribute__ ((noinline)) ClassName ## _setVTable(bool force)																	\
	{																																	\
		/* setup the class's vtable only if destructor is NULL */																		\
		if(!force && ClassName ## _vTable.destructor)																					\
		{																																\
			return;																														\
		}																																\
																																		\
		/* clean up the vtable */																										\
		uint32 i = 0;																													\
		for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)															\
		{																																\
			((void (*(*))())&ClassName ## _vTable)[i] = NULL;																			\
		}																																\
																																		\
		/* set the class's virtual methods */																							\
		ClassName ## _SET_VTABLE(ClassName)																								\
																																		\
		/* set the destructor */																										\
		__VIRTUAL_SET(ClassName, ClassName, destructor);																				\
																																		\
		/* set the getBaseClass method */																								\
		__VIRTUAL_SET(ClassName, ClassName, getBaseClass);																				\
																																		\
		/* set the getClassName method */																								\
		__VIRTUAL_SET(ClassName, ClassName, getClassName);																				\
	}																																	\

/// Declare a class's vtable.
/// @param ClassName: Class whose virtual table is to being declared
/// @return Class' virtual table's declaration
#define __VTABLE(ClassName)																												\
																																		\
	/* declare the vtable struct */																										\
	struct ClassName ## _vTable 																										\
	{																																	\
		/* all destructors are virtual */																								\
		__VIRTUAL_DEC(ClassName, void, destructor, ClassName);																			\
																																		\
		/* get super class method */																									\
		__VIRTUAL_DEC(ClassName, ClassPointer, getBaseClass, ClassName);																\
																																		\
		/* all destructors are virtual */																								\
		__VIRTUAL_DEC(ClassName, const char*, getClassName, ClassName);																	\
																																		\
		/* insert class's virtual methods names */																						\
		ClassName ## _METHODS(ClassName)																								\
	};																																	\
																																		\
	/* declare virtual table for external reference */																					\
	extern struct ClassName ## _vTable ClassName ## _vTable																				\

/// Forward declare a class.
/// @param ClassName: Class being forward declared
/// @return Class' forward declaration
#define __FORWARD_CLASS(ClassName)																										\
																																		\
	/* declare a pointer */																												\
	typedef struct ClassName ## _str* ClassName																							\

/// typedef for RTTI
typedef void* (*(*ClassPointer)(void*))(void*);

/// Declare a class' fundamental methods.
/// @param ClassName: Class being declared
/// @return Declaration of methods that all classes must have
#define __CLASS_FUNDAMENTAL_METHODS(ClassName)																							\
																																		\
	/* declare getBaseClass method */																									\
	ClassPointer ClassName ## _getBaseClass(void*)

/// Declare a class.
/// @param ClassName: Class being declared
/// @return Class' declaration
#define __CLASS(ClassName)																												\
																																		\
	/* Declare vtable */																												\
	__VTABLE(ClassName);																												\
																																		\
	/* Declare vtable */																												\
	void ClassName ## _setVTable(bool force);																							\
																																		\
	/* get class */																														\
	const void* ClassName ## _getClass();																								\
																																		\
	/* Declare restoreMethods name method */																							\
	void ClassName ## _restoreMethods();																								\
																																		\
	/* Declare getSize method */																										\
	int32 ClassName ## _getObjectSize();																								\
																																		\
	/* declare getClass name method */																									\
	const char* ClassName ## _getClassName(ClassName);																					\
																																		\
	/* Declare fundamental class methods */																								\
	__CLASS_FUNDAMENTAL_METHODS(ClassName)

/// Copy a class' declaration to make its member accessible to a compilation unit.
/// @param ClassName: Class being friended
/// @return Class' struct declaration
#define __CLASS_FRIEND_DEFINITION(ClassName)																							\
	typedef struct ClassName ## _str																									\
	{																																	\
		/* class attributes */																											\
		ClassName ## _ATTRIBUTES																										\
																																		\
		/* end spec */																													\
	} ClassName ## _str 																												\

/// Retrieve the memory footprint of a give class' instances.
/// @param ClassName: Class whose instances' size is to be retrieved
/// @return Implementation of the class' method to retrive the size of its instances
#ifndef __RELEASE
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)																						\
																																		\
	int32 ClassName ## _getObjectSize()																									\
	{																																	\
		return sizeof(ClassName ## _str);																								\
	}
#else
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)
#endif

/// Define the fundamental methods of a class.
/// @param ClassName: Class whose methods are defined
/// @param BaseClassName: Base class' from which the class inherits
/// @return Class' fundamental method's definition
#define __CLASS_FUNDAMENTAL_DEFINITION(ClassName, BaseClassName)																		\
																																		\
	/* Define class's getBaseClass method */																							\
	ClassPointer ClassName ## _getBaseClass(void* this __attribute__ ((unused)))														\
	{																																	\
		ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,															\
				"Wrong class spec: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "													\
				__MAKE_STRING(BaseClassName) ")");																						\
		return (ClassPointer)&BaseClassName ## _getBaseClass;																			\
	}

/// Define the methods of a class.
/// @param ClassName: Class whose methods are defined
/// @param BaseClassName: Base class' from which the class inherits
/// @return Class' fundamental method's definition
#define __CLASS_DEFINITION(ClassName, BaseClassName)																					\
																																		\
	typedef struct ClassName ## _str																									\
	{																																	\
		/* class attributes */																											\
		ClassName ## _ATTRIBUTES																										\
																																		\
		/* end spec */																													\
	} ClassName ## _str;																												\
																																		\
	/* class' vtable's spec */																											\
	struct ClassName ## _vTable ClassName ## _vTable __VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE;											\
																																		\
	static void (* const _baseDestructor)(Object) =																						\
	/* class' base's destructor */																										\
		(void (*)(Object))&BaseClassName ## _destructor;																				\
																																		\
	/* Define class's getSize method */																									\
	const void* ClassName ## _getClass()																								\
	{																																	\
		return (const void*)&ClassName ## _vTable;																						\
	}																																	\
																																		\
	/* Define class's getSize method */																									\
	const char* ClassName ## _getClassName(ClassName this __attribute__ ((unused)))														\
	{																																	\
		ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,															\
				"Wrong class spec: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "													\
				__MAKE_STRING(BaseClassName) ")");																						\
		return (char*)__OBFUSCATE_NAME(ClassName);																						\
	}																																	\
																																		\
	/* Define class's fundamental methods */																							\
	__CLASS_FUNDAMENTAL_DEFINITION(ClassName, BaseClassName)																			\
																																		\
	/* Define class's getSize method */																									\
	__GET_INSTANCE_SIZE_DEFINITION(ClassName)																							\
																																		\
	/* restore class vTable */																											\
	void ClassName ## _restoreMethods()																									\
	{																																	\
		/* must prevent any interrupt from calling methods on unstable vtables */														\
		HardwareManager_suspendInterrupts();																							\
																																		\
		ClassName ## _setVTable(true);																									\
																																		\
		/* resume interrupts */																											\
		HardwareManager_resumeInterrupts();																								\
	}																																	\
																																		\
	/* now add the function which will handle the vtable */																				\
	__SET_VTABLE_DEFINITION(ClassName, BaseClassName)																					\
	__CHECK_VTABLE_DEFINITION(ClassName)																								\

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Object;
typedef Object (*AllocatorPointer)();

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Object
///
/// Inherits from Object
///
/// Serves as the base class for all other classes in the engine.
abstract class Object : Object
{
	/// @protectedsection
	// The unusual order of the attributes in the rest of the classes 
	// aims to optimize data packing as much as possible.

	/// Pointer to the class's virtual table
	void* vTable;

	/// @publicsection

	/// Cast an object at runtime to a give class.
	/// @param object: Object to cast
	/// @param targetClassGetClassMethod: pointer to the target class' identifier method
	/// @param baseClassGetClassMethod: pointer to the object's base class' identifier method
	/// @return Pointer to the object if the cast succeeds, NULL otherwhise.
	static Object getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod);

	/// Class' constructor
	void constructor();

 	/// Retrieve the object's virtual table pointer
	/// @return	Pointer to the object's virtual table pointer
	const void* getVTable();

 	/// Converts the object into an instance of the target class if object's class is in the hierarchy of the target class.
	/// @param targetClass: pointer to the target class' virtual table
	/// @return	True if successful
	bool mutateTo(const void* targetClass);
}

#endif
