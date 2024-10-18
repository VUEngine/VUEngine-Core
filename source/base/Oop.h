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


//=========================================================================================================
// OOP MACROS
//=========================================================================================================

#define __MAKE_STRING(a) #a

// concatenate two strings
#define __MAKE_CONCAT(str_1,str_2) str_1 ## str_2
#define __CUSTOM_CONCAT(str_1,str_2) __MAKE_CONCAT(str_1,str_2)

#define __BASE	((void*)this)

#ifdef __SHIPPING
#define __OBFUSCATE_NAME(value)			("ClassName")
#else
#define __OBFUSCATE_NAME(value)			(#value)
#endif

#ifndef __RELEASE
#undef __BYPASS_MEMORY_MANAGER_WHEN_DELETING
#else
#define __BYPASS_MEMORY_MANAGER_WHEN_DELETING
#endif

#define __OBJECT_MEMORY_FOOT_PRINT		(uint16)(__MEMORY_USED_BLOCK_FLAG + sizeof(uint16) * 8)

// define the class's allocator
#define __CLASS_NEW_DEFINITION(ClassName, ...)																	\
																												\
		/* define the method */																					\
		ClassName ClassName ## _new(__VA_ARGS__)																\
		{																										\
			/* make sure that the class is properly set */														\
			__CALL_CHECK_VTABLE(ClassName);																		\
																												\
			/* allocate object */																				\
			uint16* memoryBlock = (uint16*)MemoryPool_allocate(				 									\
							sizeof(ClassName ## _str) + __DYNAMIC_STRUCT_PAD);									\
																												\
			/* mark memory block as used by an object */														\
			*memoryBlock = __OBJECT_MEMORY_FOOT_PRINT;															\
																												\
			/* this pointer lives __DYNAMIC_STRUCT_PAD ahead */													\
			ClassName this = (ClassName)((uint32)memoryBlock + __DYNAMIC_STRUCT_PAD);							\
																												\
			/* check if properly created */																		\
			ASSERT(this, __MAKE_STRING(ClassName) "::new: not allocated");										\


// end class's allocator
#define __CLASS_NEW_END(ClassName, ...)																			\
																												\
			/* set the vtable pointer */																		\
			this->vTable = (void*)&ClassName ## _vTable;														\
																												\
			/* construct the object */																			\
			ClassName ## _constructor(__VA_ARGS__);																\
																												\
			ASSERT(this->vTable == &ClassName ## _vTable,														\
				__MAKE_STRING(ClassName) "::new: vTable not set properly");										\
																												\
			/* return the created object */																		\
			return this;																						\
		}																										\
																												\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */								\
		void ClassName ## dummyMethodClassNew()

#define	__DYNAMIC_STRUCT_PAD	sizeof(uint32)

// like new in C++
#define __NEW(ClassName, ...)																					\
																												\
		/* call class's new implementation */																	\
		ClassName ## _new(__VA_ARGS__)																			\

// like new in C++
#define __NEW_BASIC(ClassName)																					\
																												\
		/* allocate data */																						\
		(ClassName*)((uint32)MemoryPool_allocate(																\
			sizeof(ClassName) + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);									\

// like delete in C++ (calls virtual destructor)
#ifndef __BYPASS_MEMORY_MANAGER_WHEN_DELETING
#define __DELETE(objectToDelete)																				\
	{																											\
		void* object = (void*)objectToDelete;																	\
																												\
		HardwareManager_suspendInterrupts();																	\
																												\
		if(__OBJECT_MEMORY_FOOT_PRINT == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))						\
		{																										\
			/* since the destructor is the first element in the virtual table */								\
			ASSERT(object && *(uint32*)object, "Deleting null object");											\
			((((struct Object ## _vTable*)((*((void**)object))))->destructor))((Object)object);					\
		}																										\
		else if(__MEMORY_USED_BLOCK_FLAG == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))					\
		{																										\
			ASSERT(object && *(uint32*)((uint32)object - __DYNAMIC_STRUCT_PAD), 								\
				"Oop: deleting null basic object");																\
			MemoryPool_free((BYTE*)((uint32)object - __DYNAMIC_STRUCT_PAD));									\
		}																										\
		else 																									\
		{																										\
			NM_ASSERT(false, "Oop: deleting something not dynamically allocated");								\
		}																										\
																												\
		HardwareManager_resumeInterrupts();																		\
																												\
	}
#else
#define __DELETE(objectToDelete)																				\
	{																											\
		void* object = (void*)objectToDelete;																	\
																												\
		HardwareManager_suspendInterrupts();																	\
																												\
		if(__OBJECT_MEMORY_FOOT_PRINT == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))						\
		{																										\
			((((struct Object ## _vTable*)((*((void**)object))))->destructor))((Object)object);					\
		}																										\
		else if(__MEMORY_USED_BLOCK_FLAG == *(uint16*)((uint32)object - __DYNAMIC_STRUCT_PAD))					\
		{																										\
			*(uint32*)((uint32)object - __DYNAMIC_STRUCT_PAD) = __MEMORY_FREE_BLOCK_FLAG;						\
		}																										\
		else 																									\
		{																										\
			NM_ASSERT(false, "Oop: deleting something not dynamically allocated");								\
		}																										\
																												\
		HardwareManager_resumeInterrupts();																		\
																												\
	}
#endif


// construct the base object
#define __CONSTRUCT_BASE(BaseClass, ...)																		\
																												\
		/* call base's constructor */																			\
		BaseClass ## _constructor((BaseClass)__VA_ARGS__);														\

// must always call base class's destructor
#define __DESTROY_BASE																							\
																												\
		/* since the base destructor is the second element in the virtual table */								\
		_baseDestructor(__SAFE_CAST(Object, this));																\

// retrieve virtual method's address
#define __VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object)													\
																												\
		/* call derived implementation */																		\
		(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName)										\

#define __VIRTUAL_CALL_FIRST_HELPER(object, ...) object
#define __VIRTUAL_CALL_OBJECT(...) __VIRTUAL_CALL_FIRST_HELPER(__VA_ARGS__, throwaway)

// call a virtual method (in debug a check is performed to assert that the method isn't null)
#define __VIRTUAL_CALL(ClassName, MethodName, ...)																\
																												\
		((((struct ClassName ## _vTable*)((*((void**)__VIRTUAL_CALL_OBJECT(__VA_ARGS__)))))->MethodName))		\
		(																										\
			(ClassName)__VA_ARGS__																				\
		)

// call the base's method
#define __CALL_BASE_METHOD(BaseClassName, MethodName, ...)														\
																												\
		BaseClassName ## _vTable.MethodName																		\
		(																										\
			(BaseClassName)__VA_ARGS__																			\
		)

#ifdef __DEBUG
#define __SAFE_CAST(ClassName, object)																			\
																												\
		/* try to up cast object */																				\
		(ClassName)Object_getCast((Object)object, (ClassPointer)&ClassName ## _getBaseClass, NULL)
#else
#define __SAFE_CAST(ClassName, object) (ClassName)object
#endif

#define __OVERRIDES_METHOD(ClassName, object, MethodName) 														\
																												\
		/* try to up cast object */																				\
		(void (*)())&ClassName ## _ ## MethodName != 															\
			(void (*)())__VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object)

#define __IS_OBJECT_ALIVE(object)																				\
																												\
		/* test if object has not been deleted */																\
		(object && (__MEMORY_FREE_BLOCK_FLAG != *(uint32*)((uint32)object - __DYNAMIC_STRUCT_PAD)))				\


#define isDeleted(object)					(!__IS_OBJECT_ALIVE(object))

#define typeofclass(ClassName)		((ClassPointer)&ClassName ## _getBaseClass)

#define __GET_CAST(ClassName, object)																			\
																												\
		/* try to up cast object */																				\
		(ClassName)Object_getCast((Object)object,																\
			(ClassPointer)&ClassName ## _getBaseClass, NULL)													\

#define __IS_INSTANCE_OF(ClassName, object)																		\
																												\
		/* try to up cast object */																				\
		(void*)&ClassName ## _vTable == (void*)*((void**)object)												\

// declare a virtual method
#define __VIRTUAL_DEC(ClassName, ReturnType, MethodName, ...)													\
																												\
		/* define a virtual method pointer */																	\
		ReturnType (*MethodName)(__VA_ARGS__)																	\

// override a virtual method
#define __VIRTUAL_SET(ClassVTable, ClassName, MethodName)														\
		{																										\
			/* use a temporary pointer to avoid illegal cast between pointers to data and functions */	\
			void (*(*tempPointer))() = (void (*(*))())&ClassVTable ## _vTable.MethodName;						\
			*(tempPointer) = (void (*)())&ClassName ## _ ## MethodName;											\
		}

// mutate class method
#define __CLASS_MUTATE_METHOD(ClassName, MethodName, NewMethod)													\
		{																										\
			/* use a temporary pointer to avoid illegal cast between pointers to data and functions */	\
			void (*(*tempPointer))() = (void (*(*))())&ClassName ## _vTable.MethodName;							\
			*(tempPointer) = (void (*)())NewMethod;																\
		}

// configure class's vtable
#define __CHECK_VTABLE_DEFINITION(ClassName)																	\
																												\
		/* define the static method */																			\
		void __attribute__ ((noinline)) ClassName ## _checkVTable()												\
		{																										\
			/* setup the class's vtable only if destructor is NULL */											\
			NM_ASSERT(ClassName ## _vTable.destructor, ClassName ## vTable not properly set. 					\
			Delete the GAME/build/working/setupClasses.c file);													\
																												\
			/* check that no method is null */																	\
			uint32 i = 0;																						\
			for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)								\
			{																									\
				NM_ASSERT(((void (*(*))())&ClassName ## _vTable)[i], 											\
					__MAKE_STRING(ClassName) " is abstract");													\
			}																									\
		}
#ifndef __RELEASE
#define __CALL_CHECK_VTABLE(ClassName)		ClassName ## _checkVTable()
#else
#define __CALL_CHECK_VTABLE(ClassName)
#endif

// configure class's vtable
#define __SET_VTABLE_DEFINITION(ClassName, BaseClassName)														\
																												\
		/* define the static method */																			\
		void __attribute__ ((noinline)) ClassName ## _setVTable(bool force)										\
		{																										\
			/* setup the class's vtable only if destructor is NULL */											\
			if(!force && ClassName ## _vTable.destructor)														\
			{																									\
				return;																							\
			}																									\
																												\
			/* clean up the vtable */																			\
			uint32 i = 0;																						\
			for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)								\
			{																									\
				((void (*(*))())&ClassName ## _vTable)[i] = NULL;												\
			}																									\
																												\
			/* set the class's virtual methods */																\
			ClassName ## _SET_VTABLE(ClassName)																	\
																												\
			/* set the destructor */																			\
			__VIRTUAL_SET(ClassName, ClassName, destructor);													\
																												\
			/* set the getBaseClass method */																	\
			__VIRTUAL_SET(ClassName, ClassName, getBaseClass);													\
																												\
			/* set the getClassName method */																	\
			__VIRTUAL_SET(ClassName, ClassName, getClassName);													\
		}																										\

// class's vtable declaration and instantiation
#define __VTABLE(ClassName)																						\
																												\
		/* declare the vtable struct */																			\
		struct ClassName ## _vTable 																			\
		{																										\
			/* all destructors are virtual */																	\
			__VIRTUAL_DEC(ClassName, void, destructor, ClassName);												\
																												\
			/* get super class method */																		\
			__VIRTUAL_DEC(ClassName, ClassPointer, getBaseClass, ClassName);									\
																												\
			/* all destructors are virtual */																	\
			__VIRTUAL_DEC(ClassName, const char*, getClassName, ClassName);										\
																												\
			/* insert class's virtual methods names */															\
			ClassName ## _METHODS(ClassName)																	\
		};																										\
																												\
		/* declare virtual table for external reference */														\
		extern struct ClassName ## _vTable ClassName ## _vTable													\

// forward declare a class
#define __FORWARD_CLASS(ClassName)																				\
		/* declare a pointer */																					\
		typedef struct ClassName ## _str* ClassName																\

/* typedef for RTTI */
typedef void* (*(*ClassPointer)(void*))(void*);

// declare a class
#define __CLASS(ClassName)																						\
																												\
		/* declare vtable */																					\
		__VTABLE(ClassName);																					\
																												\
		/* declare vtable */																					\
		void ClassName ## _setVTable(bool force);																\
																												\
		/* get class */																							\
		const void* ClassName ## _getClass();																	\
																												\
		/* declare getSize method */																			\
		int32 ClassName ## _getObjectSize();																	\
																												\
		/* declare getBaseClass method */																		\
		ClassPointer ClassName ## _getBaseClass(void*);															\
																												\
		/* declare getClass name method */																		\
		const char* ClassName ## _getClassName(ClassName);														\
																												\
		/* declare restoreMethods name method */																\
		void ClassName ## _restoreMethods()																		\



// to being able to friend a class
#define __CLASS_FRIEND_DEFINITION(ClassName)																	\
		typedef struct ClassName ## _str																		\
		{																										\
			/* class attributes */																				\
			ClassName ## _ATTRIBUTES																			\
																												\
			/* end spec */																						\
		} ClassName ## _str 																					\


// define method only when compiling with debugging tools
#ifndef __RELEASE
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)																\
																												\
		int32 ClassName ## _getObjectSize()																		\
		{																										\
			return sizeof(ClassName ## _str);																	\
		}
#else
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)
#endif

// define a class
#define __CLASS_DEFINITION(ClassName, BaseClassName)															\
																												\
		typedef struct ClassName ## _str																		\
		{																										\
			/* class attributes */																				\
			ClassName ## _ATTRIBUTES																			\
																												\
			/* end spec */																						\
		} ClassName ## _str;																					\
																												\
		/* class' vtable's spec */																				\
		struct ClassName ## _vTable ClassName ## _vTable __VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE;				\
																												\
		static void (* const _baseDestructor)(Object) =															\
		/* class' base's destructor */																			\
			(void (*)(Object))&BaseClassName ## _destructor;													\
																												\
		/* define class's getSize method */																		\
		__GET_INSTANCE_SIZE_DEFINITION(ClassName)																\
																												\
		/* define class's getBaseClass method */																\
		ClassPointer ClassName ## _getBaseClass(void* this __attribute__ ((unused)))							\
		{																										\
			ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,								\
					"Wrong class spec: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "						\
					__MAKE_STRING(BaseClassName) ")");															\
			return (ClassPointer)&BaseClassName ## _getBaseClass;												\
		}																										\
																												\
		/* define class's getSize method */																		\
		const char* ClassName ## _getClassName(ClassName this __attribute__ ((unused)))							\
		{																										\
			ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,								\
					"Wrong class spec: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "						\
					__MAKE_STRING(BaseClassName) ")");															\
			return (char*)__OBFUSCATE_NAME(ClassName);															\
		}																										\
																												\
		/* restore class vTable */																				\
		void ClassName ## _restoreMethods()																		\
		{																										\
			/* must prevent any interrupt from calling methods on unstable vtables */							\
			HardwareManager_suspendInterrupts();																\
																												\
			ClassName ## _setVTable(true);																		\
																												\
			/* resume interrupts */																				\
			HardwareManager_resumeInterrupts();																	\
		}																										\
																												\
		const void* ClassName ## _getClass()																	\
		{																										\
			return (const void*)&ClassName ## _vTable;															\
		}																										\
																												\
		/* now add the function which will handle the vtable */													\
		__SET_VTABLE_DEFINITION(ClassName, BaseClassName)														\
		__CHECK_VTABLE_DEFINITION(ClassName)																	\

// retrieves object's class' name
#define __GET_CLASS_NAME(object)																				\
																												\
		__VIRTUAL_CALL(Object, getClassName, (Object)object)

// retrieves object's class' name
#define __GET_CLASS_NAME_UNSAFE(object)																			\
																												\
		__VIRTUAL_CALL(Object, getClassName, (Object)object)

// declare an object type
#define __TYPE(ClassName)								(AllocatorPointer)&ClassName ## _new
#define __ALLOCATOR_TYPE(allocatorPointer)				(AllocatorPointer)allocatorPointer


#define __SINGLETON_NOT_CONSTRUCTED			0
#define __SINGLETON_BEING_CONSTRUCTED		1
#define __SINGLETON_CONSTRUCTED				2

// defines a singleton (unique instance of a class)
#define __SINGLETON(ClassName)																					\
																												\
		/* declare the static instance */																		\
		typedef struct SingletonWrapper ## ClassName															\
		{																										\
			/* footprint to differentiate between objects and structs */										\
			uint32 objectMemoryFootprint;																		\
			/* declare the static instance */																	\
			ClassName ## _str instance;																			\
		} SingletonWrapper ## ClassName;																		\
																												\
		static SingletonWrapper ## ClassName _singletonWrapper ## ClassName 									\
				__STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE;														\
																												\
		/* a flag to know when to allow construction */															\
		static int8 _singletonConstructed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE							\
										= __SINGLETON_NOT_CONSTRUCTED;											\
																												\
		/* define get instance method */																		\
		static void __attribute__ ((noinline)) ClassName ## _instantiate()										\
		{																										\
			NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,									\
				ClassName get instance during construction);													\
																												\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;												\
																												\
			/* make sure that the class is properly set */														\
			__CALL_CHECK_VTABLE(ClassName);																		\
																												\
			/*  */																								\
			ClassName instance = &_singletonWrapper ## ClassName.instance;										\
			_singletonWrapper ## ClassName.objectMemoryFootprint = 												\
				(__OBJECT_MEMORY_FOOT_PRINT << 16) | -1;														\
																												\
			/* set the vtable pointer */																		\
			instance->vTable = &ClassName ## _vTable;															\
																												\
			/* call constructor */																				\
			ClassName ## _constructor(instance);																\
																												\
			/* set the vtable pointer */																		\
			instance->vTable = &ClassName ## _vTable;															\
																												\
			/* don't allow more constructs */																	\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;													\
		}																										\
																												\
		/* define get instance method */																		\
		ClassName ClassName ## _getInstance()																	\
		{																										\
			/* first check if not constructed yet */															\
			if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)											\
			{																									\
				ClassName ## _instantiate();																	\
			}																									\
																												\
			/* return the created singleton */																	\
			return &_singletonWrapper ## ClassName.instance;													\
		}																										\
																												\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */								\
		void ClassName ## dummyMethodSingleton()

// must always call base class's destructor
#define __SINGLETON_DESTROY																						\
																												\
		/* destroy super object */																				\
		__DESTROY_BASE;																							\
																												\
		/* allow new constructs */																				\
		_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;													\


// defines a dynamic singleton (unique instance of a class)
#define __SINGLETON_DYNAMIC(ClassName)																			\
																												\
		/* declare the static pointer to instance */															\
		static ClassName _instance ## ClassName __NON_INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;				\
																												\
		/* define allocator */																					\
		__CLASS_NEW_DEFINITION(ClassName, void)																	\
		__CLASS_NEW_END(ClassName, this);																		\
																												\
		/* a flag to know when to allow construction */															\
		static int8 _singletonConstructed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE							\
										= __SINGLETON_NOT_CONSTRUCTED;											\
																												\
		/* define get instance method */																		\
		static void __attribute__ ((noinline)) ClassName ## _instantiate()										\
		{																										\
			NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,									\
				ClassName get instance during construction);													\
																												\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;												\
																												\
			/* allocate */																						\
			_instance ## ClassName = ClassName ## _new();														\
																												\
			/* don't allow more constructs */																	\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;													\
		}																										\
																												\
		/* define get instance method */																		\
		ClassName ClassName ## _getInstance()																	\
		{																										\
			/* first check if not constructed yet */															\
			if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)											\
			{																									\
				ClassName ## _instantiate();																	\
			}																									\
																												\
			/* return the created singleton */																	\
			return _instance ## ClassName;																		\
		}																										\
																												\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */								\
		void ClassName ## dummyMethodSingletonNew()


#endif
