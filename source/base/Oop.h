/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OOP_H_
#define OOP_H_


#define __MAKE_STRING(a) #a

// concatenate two strings
#define __MAKE_CONCAT(str_1,str_2) str_1 ## str_2
#define __CUSTOM_CONCAT(str_1,str_2) __MAKE_CONCAT(str_1,str_2)


// define the class's allocator declaration
#define __CLASS_NEW_DECLARE(ClassName, ...)																\
																										\
		/* define the method */																			\
		ClassName ClassName ## _new(__VA_ARGS__)


// define the class's allocator
#define __CLASS_NEW_DEFINITION(ClassName, ...)															\
																										\
		/* define the method */																			\
		ClassName ClassName ## _new(__VA_ARGS__)														\
		{																								\
			/* make sure that the class is properly set */												\
			ClassName ## _checkVTable();																\
																										\
			/* to speed things up */																	\
			extern MemoryPool _memoryPool;																\
																										\
			/* allocate object */																		\
			ClassName this = (ClassName) 																\
							MemoryPool_allocate(_memoryPool, 											\
							sizeof(ClassName ## _str));													\
																										\
			/* check if properly created */																\
			ASSERT(this, __MAKE_STRING(ClassName) "::new: not allocated");								\


// end class's allocator
#define __CLASS_NEW_END(ClassName, ...)																	\
																										\
			/* set the vtable pointer */																\
			this->vTable = (void*)&ClassName ## _vTable;													\
																										\
			/* construct the object */																	\
			ClassName ## _constructor(this, ##__VA_ARGS__);												\
																										\
			ASSERT(this->vTable == &ClassName ## _vTable,												\
				__MAKE_STRING(ClassName) "::new: vTable not set properly");								\
																										\
			/* return the created object */																\
			return this;																				\
		}																								\
																										\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */						\
		void ClassName ## dummyMethodClassNew()

#define	__DYNAMIC_STRUCT_PAD	sizeof(u32)

// like new in C++
#define __NEW(ClassName, ...)																			\
																										\
		/* call class's new implementation */															\
		ClassName ## _new(__VA_ARGS__)																	\


// like delete in C++ (calls virtual destructor)
#define __DELETE(object)																				\
																										\
		/* since the destructor is the first element in the virtual table */							\
		ASSERT(object && *(u32*)object, "Deleting null object");											\
		/* ((void (*)(void*))((void***)object)[0][0])(object); */										\
		((((struct Object ## _vTable*)((*((void**)object))))->destructor))((Object)object)				\

// like new in C++
#define __NEW_BASIC(ClassName)																			\
																										\
		/* allocate data */																				\
		(ClassName*)((u32)MemoryPool_allocate(MemoryPool_getInstance(),									\
			sizeof(ClassName) + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);							\

#ifdef __DEBUG
// like delete in C++ (calls virtual destructor)
#define __DELETE_BASIC(object)																			\
																										\
		/* free the memory */																			\
		ASSERT(object && *(u32*)((u32)object - __DYNAMIC_STRUCT_PAD), 									\
				"Oop: deleting null basic object");														\
																										\
		/* to speed things up */																		\
		extern MemoryPool _memoryPool;																	\
		MemoryPool_free(_memoryPool, (BYTE*)((u32)object - __DYNAMIC_STRUCT_PAD))
#else
#define __DELETE_BASIC(object)																			\
																										\
		/* free the memory */																			\
		*(u32*)((u32)object - __DYNAMIC_STRUCT_PAD) = __MEMORY_FREE_BLOCK_FLAG;
#endif

// construct the base object
#define __CONSTRUCT_BASE(BaseClass, ...)																\
																										\
		/* call base's constructor */																	\
		BaseClass ## _constructor(__SAFE_CAST(BaseClass, this), ##__VA_ARGS__);							\

// must always call base class's destructor
#define __DESTROY_BASE																					\
																										\
		/* since the base destructor is the second element in the virtual table */						\
		_baseDestructor(__SAFE_CAST(Object, this));														\

// retrieve virtual method's address
#define __VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object)											\
																										\
		/* call derived implementation */																\
		(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName)								\

// call a virtual method (in debug a check is performed to assert that the method isn't null)
#define __VIRTUAL_CALL(ClassName, MethodName, object, ...)												\
																										\
		((((struct ClassName ## _vTable*)((*((void**)object))))->MethodName))							\
		(																								\
			__SAFE_CAST(ClassName, object), ##__VA_ARGS__												\
		)

// call the base's method
#define __CALL_BASE_METHOD(BaseClassName, MethodName, object, ...)										\
																										\
		BaseClassName ## _vTable.MethodName																\
		(																								\
			__SAFE_CAST(BaseClassName, object), ##__VA_ARGS__											\
		)

#ifdef __DEBUG
#define __SAFE_CAST(ClassName, object)																	\
																										\
		/* try to up cast object */																		\
		(ClassName)Object_getCast((Object)object, (ObjectBaseClassPointer)&ClassName ## _getBaseClass, NULL)
#else
#define __SAFE_CAST(ClassName, object) (ClassName)object
#endif


#define __IS_OBJECT_ALIVE(object)																		\
																										\
		/* test if object has not been deleted */														\
		(object && *(u32*)object)																		\


#define __IS_BASIC_OBJECT_ALIVE(object)																	\
																										\
		/* test if object has not been deleted */														\
		(object && *(u32*)((u32)object - __DYNAMIC_STRUCT_PAD))											\


#define __GET_CAST(ClassName, object)																	\
																										\
		/* try to up cast object */																		\
		(ClassName)Object_getCast((Object)object,														\
			(ObjectBaseClassPointer)&ClassName ## _getBaseClass, NULL)									\

#define __IS_INSTANCE_OF(ClassName, object)																\
																										\
		/* try to up cast object */																		\
		(void*)&ClassName ## _vTable == (void*)*((void**)object)											\

// declare a virtual method
#define __VIRTUAL_DEC(ClassName, ReturnType, MethodName, ...)											\
																										\
		/* define a virtual method pointer */															\
		ReturnType (*MethodName)(ClassName, ##__VA_ARGS__)												\

// override a virtual method
#define __VIRTUAL_SET(ClassVTable, ClassName, MethodName)												\
		{																								\
			/* use a temporal pointer to avoid illegal cast between pointers to data and functions */	\
			void (*(*tempPointer))() = (void (*(*))())&ClassVTable ## _vTable.MethodName;				\
			*(tempPointer) = (void (*)())&ClassName ## _ ## MethodName;									\
		}

// configure class's vtable
#ifndef __RELEASE
#define __CHECK_VTABLE_DEFINITION(ClassName)															\
																										\
		/* define the static method */																	\
		void __attribute__ ((noinline)) ClassName ## _checkVTable()										\
		{																								\
			/* setup the class's vtable only if destructor is NULL */									\
			if(ClassName ## _vTable.destructor)															\
			{																							\
				return;																					\
			}																							\
			else																						\
			{																							\
				NM_ASSERT(false, ClassName ## vTable not properly set. 									\
				Delete the GAME/lib/compiler/setupClasses.c file);										\
			}																							\
																										\
			/* check that no method is null */															\
			u32 i = 0;																					\
			for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)							\
			{																							\
				NM_ASSERT(((void (*(*))())&ClassName ## _vTable)[i], ClassName ##	is abstract);		\
			}																							\
		}
#else
#define __CHECK_VTABLE_DEFINITION(ClassName)															\
																										\
		/* define the static method */																	\
		void __attribute__ ((noinline)) ClassName ## _checkVTable()										\
		{																								\
		}
#endif

// configure class's vtable
#define __SET_VTABLE_DEFINITION(ClassName, BaseClassName)												\
																										\
		/* define the static method */																	\
		void __attribute__ ((noinline)) ClassName ## _setVTable()										\
		{																								\
			/* setup the class's vtable only if destructor is NULL */									\
			if(ClassName ## _vTable.destructor)															\
			{																							\
				return;																					\
			}																							\
																										\
			/* clean up the vtable */																	\
			u32 i = 0;																					\
			for(; i < sizeof(ClassName ## _vTable) / sizeof(void (*(*))()); i++)							\
			{																							\
				((void (*(*))())&ClassName ## _vTable)[i] = NULL;										\
			}																							\
																										\
			/* set the base class's virtual methods */													\
			if(&ClassName ## _setVTable != &BaseClassName ## _setVTable)								\
			{																							\
				BaseClassName ## _setVTable();															\
			}																							\
																										\
			/* set the class's virtual methods */														\
			ClassName ## _SET_VTABLE(ClassName)															\
																										\
			/* set the destructor */																	\
			__VIRTUAL_SET(ClassName, ClassName, destructor);											\
																										\
			/* set the getBaseClass method */															\
			__VIRTUAL_SET(ClassName, ClassName, getBaseClass);											\
																										\
			/* set the getClassName method */															\
			__VIRTUAL_SET(ClassName, ClassName, getClassName);											\
		}																								\

// class's vtable declaration and instantiation
#define __VTABLE(ClassName)																				\
																										\
		/* declare the vtable struct */																	\
		struct ClassName ## _vTable 																	\
		{																								\
			/* all destructors are virtual */															\
			__VIRTUAL_DEC(ClassName, void, destructor);													\
																										\
			/* get super class method */																\
			__VIRTUAL_DEC(ClassName, ObjectBaseClassPointer, getBaseClass);								\
																										\
			/* all destructors are virtual */															\
			__VIRTUAL_DEC(ClassName, const char*, getClassName);											\
																										\
			/* insert class's virtual methods names */													\
			ClassName ## _METHODS(ClassName)															\
		};																								\
																										\
		/* declare virtual table for external reference */												\
		extern struct ClassName ## _vTable ClassName ## _vTable											\

// forward declare a class
#define __FORWARD_CLASS(ClassName)																		\
		/* declare a pointer */																			\
		typedef struct ClassName ## _str* ClassName;														\

// declare a class
#define __CLASS(ClassName)																				\
																										\
		/* declare a pointer */																			\
		typedef struct ClassName ## _str* ClassName;														\
																										\
		/* typedef for RTTI */																			\
		typedef void* (*(*ClassName ## BaseClassPointer)(Object))(Object);								\
																										\
		/* declare vtable */																			\
		__VTABLE(ClassName);																			\
																										\
		/* declare vtable */																			\
		void ClassName ## _setVTable();																	\
																										\
		/* declare getSize method */																	\
		int ClassName ## _getObjectSize();																\
																										\
		/* declare getBaseClass method */																\
		ObjectBaseClassPointer ClassName ## _getBaseClass(Object);										\
																										\
		/* declare getClass name method */																\
		const char* ClassName ## _getClassName(ClassName)												\


// to being able to friend a class
#define __CLASS_FRIEND_DEFINITION(ClassName)															\
		typedef struct ClassName ## _str																\
		{																								\
			/* class attributes */																		\
			ClassName ## _ATTRIBUTES																	\
																										\
			/* end definition */																		\
		} ClassName ## _str 																				\


// define method only when compiling with debugging tools
#ifdef __DEBUG_TOOLS
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)														\
																										\
		int ClassName ## _getObjectSize()																\
		{																								\
			return sizeof(ClassName ## _str);															\
		}
#else
#define __GET_INSTANCE_SIZE_DEFINITION(ClassName)
#endif

// define a class
#define __CLASS_DEFINITION(ClassName, BaseClassName)													\
																										\
		typedef struct ClassName ## _str																\
		{																								\
			/* class attributes */																		\
			ClassName ## _ATTRIBUTES																	\
																										\
			/* end definition */																		\
		} ClassName ## _str;																				\
																										\
		/* class' vtable's definition */																\
		struct ClassName ## _vTable ClassName ## _vTable __VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE;		\
																										\
		static void (* const _baseDestructor)(Object) =													\
		/* class' base's destructor */																	\
			(void (*)(Object))&BaseClassName ## _destructor;												\
																										\
		/* define class's getSize method */																\
		__GET_INSTANCE_SIZE_DEFINITION(ClassName)														\
																										\
		/* define class's getBaseClass method */														\
		ObjectBaseClassPointer ClassName ## _getBaseClass(Object this __attribute__ ((unused)))			\
		{																								\
			ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,						\
					"Wrong class definition: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "			\
					__MAKE_STRING(BaseClassName) ")");													\
			return (ObjectBaseClassPointer)&BaseClassName ## _getBaseClass;								\
		}																								\
																										\
		/* define class's getSize method */																\
		const char* ClassName ## _getClassName(ClassName this __attribute__ ((unused)))					\
		{																								\
			ASSERT(&BaseClassName ## _getBaseClass != &ClassName ## _getBaseClass,						\
					"Wrong class definition: __CLASS_DEFINITION(" __MAKE_STRING(ClassName) ", "			\
					__MAKE_STRING(BaseClassName) ")");													\
			return #ClassName;																			\
		}																								\
																										\
		/* now add the function which will handle the vtable */											\
		__SET_VTABLE_DEFINITION(ClassName, BaseClassName)												\
		__CHECK_VTABLE_DEFINITION(ClassName)															\

// retrieves object's class' name
#define __GET_CLASS_NAME(object)																		\
																										\
		__VIRTUAL_CALL(Object, getClassName, (Object)object)

// retrieves object's class' name
#define __GET_CLASS_NAME_UNSAFE(object)																	\
																										\
		__VIRTUAL_CALL(Object, getClassName, (Object)object)

// declare an object type
#define __TYPE(ClassName)								(AllocatorPointer)&ClassName ## _new
#define __ALLOCATOR_TYPE(allocatorPointer)				(AllocatorPointer)allocatorPointer


#define __SINGLETON_NOT_CONSTRUCTED			0
#define __SINGLETON_BEING_CONSTRUCTED		1
#define __SINGLETON_CONSTRUCTED				2

// defines a singleton (unique instance of a class)
#define __SINGLETON(ClassName, ...)																		\
																										\
		/* declare the static instance */																\
		static ClassName ## _str _instance ## ClassName __STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE;		\
																										\
		/* a flag to know when to allow construction */													\
		static s8 _singletonConstructed __INITIALIZED_DATA_SECTION_ATTRIBUTE							\
										= __SINGLETON_NOT_CONSTRUCTED;									\
																										\
		/* define get instance method */																\
		static void __attribute__ ((noinline)) ClassName ## _instantiate()								\
		{																								\
			NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,							\
				ClassName get instance during construction);											\
																										\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;										\
																										\
			/* make sure that the class is properly set */												\
			ClassName ## _checkVTable();																\
																										\
			/* set the vtable pointer */																\
			_instance ## ClassName.vTable = &ClassName ## _vTable;										\
																										\
			/* call constructor */																		\
			ClassName ## _constructor(&_instance ## ClassName);											\
																										\
			/* set the vtable pointer */																\
			_instance ## ClassName.vTable = &ClassName ## _vTable;										\
																										\
			/* don't allow more constructs */															\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;											\
		}																								\
																										\
		/* define get instance method */																\
		ClassName ClassName ## _getInstance()															\
		{																								\
			/* first check if not constructed yet */													\
			if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)									\
			{																							\
				ClassName ## _instantiate();															\
			}																							\
																										\
			/* return the created singleton */															\
			return &_instance ## ClassName;																\
		}																								\
																										\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */						\
		void ClassName ## dummyMethodSingleton()

// must always call base class's destructor
#define __SINGLETON_DESTROY																				\
																										\
		/* destroy super object */																		\
		__DESTROY_BASE;																					\
																										\
		/* allow new constructs */																		\
		_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;											\


// defines a dynamic singleton (unique instance of a class)
#define __SINGLETON_DYNAMIC(ClassName)																	\
																										\
		/* declare the static pointer to instance */													\
		static ClassName _instance ## ClassName __NON_INITIALIZED_DATA_SECTION_ATTRIBUTE;				\
																										\
		/* define allocator */																			\
		__CLASS_NEW_DEFINITION(ClassName)																\
		__CLASS_NEW_END(ClassName);																		\
																										\
		/* a flag to know when to allow construction */													\
		static s8 _singletonConstructed __INITIALIZED_DATA_SECTION_ATTRIBUTE							\
										= __SINGLETON_NOT_CONSTRUCTED;									\
																										\
		/* define get instance method */																\
		static void __attribute__ ((noinline)) ClassName ## _instantiate()								\
		{																								\
			NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,							\
				ClassName get instance during construction);											\
																										\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;										\
																										\
			/* allocate */																				\
			_instance ## ClassName = ClassName ## _new();												\
																										\
			/* don't allow more constructs */															\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;											\
		}																								\
																										\
		/* define get instance method */																\
		ClassName ClassName ## _getInstance()															\
		{																								\
			/* first check if not constructed yet */													\
			if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)									\
			{																							\
				ClassName ## _instantiate();															\
			}																							\
																										\
			/* return the created singleton */															\
			return _instance ## ClassName;																\
		}																								\
																										\
		/* dummy redeclaration to avoid warning when compiling with -pedantic */						\
		void ClassName ## dummyMethodSingletonNew()


#endif
