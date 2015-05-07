/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef OOP_H_
#define OOP_H_

#define __MAKE_STRING(a) #a

// concatenate two strigs
#define __MAKE_CONCAT(str_1,str_2) str_1 ## str_2
#define __CONCAT(str_1,str_2) __MAKE_CONCAT(str_1,str_2)


// call to this method only once
#define __CALL_ONCE(MethodName, ...)													\
																						\
	{																					\
		/* a static flag */																\
		static bool __callFlag = false;													\
																						\
		/* check if not called */														\
		if (!__callFlag)																\
		{																				\
			/* call method */															\
			MethodName(__VA_ARGS__);													\
		}																				\
	}

// to support in run time abstract class instantiation and debug
#define __DEFINE_CHECK_VTABLE(ClassName)												\
																						\
	/* define the checking method */													\
	void ClassName  ## _checkVTable()													\
	{																					\
		/* check that each entry in the table is not NULL */							\
		int i = 0;																		\
		for (; i < sizeof(ClassName ## _vTable) / sizeof(void*); i++)					\
		{																				\
			/* check each entry */														\
			NM_ASSERT(((void**)&ClassName ## _vTable)[i],								\
					ClassName ##  is abstract);											\
		}																				\
	}

// call to check that the vtable doesn't have null pointers
#define __CHECK_VTABLE(ClassName)														\
																						\
	/* setup the class's vtable on first call only */									\
	__CALL_ONCE(ClassName  ## _checkVTable);


// define the class's method which allocates the instances
#define __ALLOCATOR_DEFINITION(ClassName, BaseClassName)								\
																						\
	/* define allocator */																\
	ClassName ClassName ## _allocator()													\
	{																					\
		/* allocate object */															\
		ClassName this = (ClassName) 													\
						MemoryPool_allocate(MemoryPool_getInstance(), 					\
						sizeof(ClassName ## _str));										\
																						\
		/* abstract classes can't be instantiated */									\
		__CHECK_VTABLE(ClassName);														\
																						\
		/* return constructed object's address */										\
		return this;																	\
																						\
	}

// define the class's allocator declaration
#define __CLASS_NEW_DECLARE(ClassName, ...)												\
																						\
	/* define the method */																\
	ClassName ClassName ## _new(__VA_ARGS__)


// define the class's allocator
#define __CLASS_NEW_DEFINITION(ClassName, ...)											\
																						\
	/* define the method */																\
	ClassName ClassName ## _new(__VA_ARGS__)											\
	{																					\
		/* setup the class's vtable on first call only */								\
		__SET_CLASS(ClassName);															\
																						\
		/* create the object and allocate memory for it */								\
		ClassName this = ClassName ## _allocator();										\
																						\
		/* check if properly created */													\
		if (!this) return NULL;															\


// end class's allocator
#define __CLASS_NEW_END(ClassName, ...)													\
																						\
		/* set the vtable pointer */													\
		this->vTable = &ClassName ## _vTable;											\
																						\
		/* construct the object */														\
		ClassName ## _constructor(this, ##__VA_ARGS__);									\
																						\
		/* just for safety set it again */												\
		this->vTable = &ClassName ## _vTable;											\
																						\
		/* set dynamic flag */															\
		this->dynamic = true;															\
																						\
		/* return the created object */													\
		return this;																	\
	}


// like new in C++
#define __NEW(ClassName, ...)															\
																						\
	/* call class's new implementation */												\
	ClassName ## _new(__VA_ARGS__)


// like delete in C++ (calls virtual destructor)
#define __DELETE(object)																\
																						\
	/* since the destructor is the first element in the virtual table */				\
	((void (*)(void*))((void***)object)[0][0])(object);


// like new in C++
#define __NEW_BASIC(ClassName)															\
																						\
	/* allocate data */																	\
	(ClassName*)MemoryPool_allocate(MemoryPool_getInstance(),							\
		sizeof(ClassName));


// like delete in C++ (calls virtual destructor)
#define __DELETE_BASIC(object)															\
																						\
	/* free the memory */																\
	MemoryPool_free(MemoryPool_getInstance(), (void*)object)


// construct the base object
#define __CONSTRUCT_BASE(...)															\
																						\
	/* call base's constructor */														\
	_baseConstructor((void*)this, ##__VA_ARGS__);												\

// must always call base class's destructor
#define __DESTROY_BASE																	\
																						\
	/* since the base destructor is the second element in the virtual table */			\
	_baseDestructor((void*)this);																\
																						\
	/* if dynamically created */														\
	if (this->dynamic)																	\
	{																					\
		this->dynamic = false;															\
																						\
		/* free the memory */															\
		MemoryPool_free(MemoryPool_getInstance(), (void*)this);							\
	}


// retrieve virtual method's address
#define __VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object, ...)						\
																						\
	/* call derived implementation */													\
	(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName)

// call a virtual method (in debug a check is performed to assert that the method isn't null)
#ifdef __DEBUG
#define __VIRTUAL_CALL(ReturnType, ClassName, MethodName, object, ...)					\
		(																				\
			(__VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object, ...))?				\
			/* call derived implementation */											\
			((ReturnType (*)(ClassName, ...))											\
			(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName))		\
				(																		\
						__UPCAST(ClassName, object), ##__VA_ARGS__						\
				):																		\
			/* call base implementation */												\
			(ReturnType)Error_triggerException(Error_getInstance(),						\
				 __MAKE_STRING(ClassName ## _ ##  MethodName))							\
		)


#else
#define __VIRTUAL_CALL(ReturnType, ClassName, MethodName, object, ...)					\
		/* release implementation */													\
		((ReturnType (*)(ClassName, ...))												\
		(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName))			\
			(																			\
				__UPCAST(ClassName, object), ##__VA_ARGS__								\
			)																			\

#endif

#define __VIRTUAL_CALL_UNSAFE(ReturnType, ClassName, MethodName, object, ...)			\
		/* to bypass checking on DEBUG */												\
		((ReturnType (*)(ClassName, ...))												\
		(((struct ClassName ## _vTable*)((*((void**)object))))->MethodName))			\
			(																			\
					object, ##__VA_ARGS__												\
			)																			\

// cast macro
#define __GET_CAST(ClassName, object)													\
		(																				\
			/* check if object's destructor matches class' destructor */				\
			object && ((void*)ClassName ## _destructor == 								\
							((void (*)(void*))((void***)object)[0][0]))?				\
																						\
			/* cast is safe */															\
			(ClassName)object															\
																						\
			/* otherwise */																\
			:																			\
			/* cast is null */															\
			NULL																		\
		)

#ifdef __DEBUG
#define __UPCAST(ClassName, object)														\
																						\
		/* try to up cast object */														\
		(ClassName)Object_upcast((Object)object, ClassName ## _getBaseClass, NULL)
#else	
#define __UPCAST(ClassName, object) (ClassName)object
#endif


// declare a virtual method
#define __VIRTUAL_DEC(MethodName)														\
																						\
	/* define a virtual method pointer */												\
	void* MethodName


// override a virtual method
#define __VIRTUAL_SET(ClassVTable, ClassName, MethodName)								\
																						\
	/* set the virtual method's address in its correspoiding vtable offset */			\
	ClassVTable ## _vTable.MethodName = ClassName ## _ ## MethodName


// override a virtual method
#define __VIRTUAL_BASE_SET(ClassVTable, ClassName, MethodName, BaseClassMethodName)		\
																						\
	/* set the virtual method's address in its correspoiding vtable offset */			\
	ClassVTable ## _vTable.MethodName = ClassName ## _ ## BaseClassMethodName


// call configure class's vtable method
#define __SET_CLASS(ClassName)															\
																						\
	/* setup the class's vtable on first call only */									\
	__CALL_ONCE(ClassName ## _setVTable)

// configure class's vtable
#define __SET_VTABLE(ClassName, BaseClassName)											\
																						\
	/* define the static method */														\
	void ClassName ## _setVTable()														\
	{																					\
		/* set the base class's virtual methods */										\
		if(ClassName ## _setVTable != BaseClassName ## _setVTable)						\
		{																				\
			BaseClassName ## _setVTable();												\
		}																				\
																						\
		/* set the class's virtual methods */											\
		ClassName ## _SET_VTABLE(ClassName)												\
																						\
		/* always set the destructor at the end */										\
		__VIRTUAL_SET(ClassName, ClassName, destructor);								\
																						\
		/* always set the destructor at the end */										\
		__VIRTUAL_SET(ClassName, ClassName, getBaseClass);								\
																						\
		/* always set the class name methos at the end */								\
		__VIRTUAL_SET(ClassName, ClassName, getClassName);								\
																						\
		/* set base class methods */													\
		_baseConstructor = (void (*)(void*, ...))&BaseClassName ## _constructor;		\
		_baseDestructor = (void (*)(void*))&BaseClassName ## _destructor;				\
	}


// class's vtable declaration and instantiation
#define __VTABLE(ClassName)																\
																						\
	/* declare the vtable struct */														\
	struct ClassName ## _vTable 														\
	{																					\
		/* all destructors are virtual */												\
		__VIRTUAL_DEC(destructor);														\
																						\
		/* get super class method */													\
		__VIRTUAL_DEC(getBaseClass);													\
																						\
		/* all destructors are virtual */												\
		__VIRTUAL_DEC(getClassName);													\
																						\
		/* insert class's virtual methods names */										\
		ClassName ## _METHODS;															\
																						\
	/* create the vtable instance */													\
	}ClassName ## _vTable;																\


// declare a class
#define __CLASS(ClassName)																\
																						\
	/* declare a const pointer */														\
	typedef struct ClassName ## _str* ClassName;										\
																						\
	/* declare vtable */																\
	__VTABLE(ClassName);																\
																						\
	/* declare vtable */																\
	void ClassName ## _setVTable();														\
																						\
	/* declare getSize method */														\
	int ClassName ## _getObjectSize();													\
																						\
	/* declare getBaseClass method */													\
	void* ClassName ## _getBaseClass();													\
																						\
	/* declare getClass name method */													\
	char* ClassName ## _getClassName()

// to being able to friend a class
#define __CLASS_FRIEND_DEFINITION(ClassName)											\
	typedef struct ClassName ## _str													\
	{																					\
		/* class attributes */															\
		ClassName ## _ATTRIBUTES														\
																						\
		/* end definition */															\
	} ClassName ## _str;																\


// define a class
#define __CLASS_DEFINITION(ClassName, BaseClassName)									\
																						\
	typedef struct ClassName ## _str													\
	{																					\
		/* class attributes */															\
		ClassName ## _ATTRIBUTES														\
																						\
		/* end definition */															\
	} ClassName ## _str;																\
																						\
	static void (*_baseConstructor)(void*, ...) = NULL;									\
	static void (*_baseDestructor)(void*) = NULL;										\
																						\
	/* define class's getSize method */													\
	int ClassName ## _getObjectSize()													\
	{																					\
		return sizeof(ClassName ## _str);												\
	}																					\
																						\
	/* define class's getBaseClass method */											\
	void* ClassName ## _getBaseClass()													\
	{																					\
		return (void*)BaseClassName ## _getBaseClass;									\
	}																					\
																						\
	/* define class's getSize method */													\
	char* ClassName ## _getClassName()													\
	{																					\
		return #ClassName;																\
	}																					\
																						\
	/* now add the function which will set the vtable */								\
	__SET_VTABLE(ClassName, BaseClassName)												\
																						\
	/* define vtable checker */															\
	__DEFINE_CHECK_VTABLE(ClassName)													\
																						\
	/* define allocator */																\
	__ALLOCATOR_DEFINITION(ClassName, BaseClassName)


// retrieves object's class' name
#define __GET_CLASS_NAME(object)														\
																						\
	__VIRTUAL_CALL(char*, Object, getClassName, (Object)object)

// declare an object type
#define __TYPE(ClassName)	ClassName ## _new

#define __SINGLETON_NOT_CONSTRUCTED			0
#define __SINGLETON_BEING_CONSTRUCTED		1
#define __SINGLETON_CONSTRUCTED				2

// defines a singleton (unique instance of a class)
#define __SINGLETON(ClassName)															\
																						\
	/* declare the static instance */													\
	static ClassName ## _str _instance ## ClassName;									\
																						\
	/* a flag to know when to allow constructs */										\
	static s8 _singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;						\
																						\
	/* define get instance method */													\
	ClassName ClassName ## _getInstance()												\
	{																					\
		/* set the vtable */															\
		__SET_CLASS(ClassName);															\
																						\
		if (__SINGLETON_BEING_CONSTRUCTED == _singletonConstructed)						\
		{																				\
			NM_ASSERT(false, ClassName get instance during construction);				\
		}																				\
		/* first check if not constructed yet */										\
		if (__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)						\
		{																				\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;						\
																						\
			/* call constructor */														\
			ClassName ## _constructor(&_instance ## ClassName);							\
																						\
			/* set the vtable pointer */												\
			_instance ## ClassName.vTable = &ClassName ## _vTable;						\
																						\
			/* don't allow more constructs */											\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;							\
		}																				\
																						\
		/* return the created singleton */												\
		return &_instance ## ClassName;													\
	}

// must always call base class's destructor
#define __SINGLETON_DESTROY																\
																						\
	/* destroy super object */															\
	__DESTROY_BASE;																		\
																						\
	/* allow new constructs */															\
	_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;								\


// defines a dynamic singleton (unique instance of a class)
#define __SINGLETON_DYNAMIC(ClassName)													\
																						\
	/* declare the static pointer to instance */										\
	static ClassName _instance ## ClassName;											\
																						\
	/* define allocator */																\
	__CLASS_NEW_DEFINITION(ClassName);													\
	__CLASS_NEW_END(ClassName);															\
																						\
	/* a flag to know when to allow constructs */										\
	static s8 _singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;						\
																						\
	/* define get instance method */													\
	ClassName ClassName ## _getInstance()												\
	{																					\
		/* set the vtable */															\
		__SET_CLASS(ClassName);															\
																						\
		if (__SINGLETON_BEING_CONSTRUCTED == _singletonConstructed)						\
		{																				\
			NM_ASSERT(false, ClassName get instance during construction);				\
		}																				\
																						\
		/* first check if not constructed yet */										\
		if (__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)						\
		{																				\
			_singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;						\
																						\
			/* allocate */																\
			_instance ## ClassName = ClassName ## _new();								\
																						\
			/* don't allow more constructs */											\
			_singletonConstructed = __SINGLETON_CONSTRUCTED;							\
		}																				\
																						\
		/* return the created singleton */												\
		return _instance ## ClassName;													\
	}

// gcc has a bug, it doesn't move back the sp register after returning
// from a variadic call
#define __CALL_VARIADIC(VariadicFunctionCall)											\
																						\
	/* variadic function call */														\
	VariadicFunctionCall;																\
																						\
	/* sp fix displacement */															\
	asm("addi	20, sp, sp")


// MemoryPool's defines
#define __BLOCK_DEFINITION(BlockSize, Elements)									\
	BYTE pool ## BlockSize ## B[BlockSize * Elements]; 							\

#define __SET_MEMORY_POOL_ARRAY(BlockSize)										\
	this->poolLocation[pool] = this->pool ## BlockSize ## B;					\
	this->poolSizes[pool][ePoolSize] = sizeof(this->pool ## BlockSize ## B);	\
	this->poolSizes[pool++][eBlockSize] = BlockSize;							\


#endif