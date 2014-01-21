/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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
#define __PARAMETERS(...)  ,__VA_ARGS__
#define __ARGUMENTS(...)  ,__VA_ARGS__

/*--------------------------------------------------------------------------------------------------------------------------------*/
// concatenate two strigs
#define __MAKE_CONCAT(str_1,str_2) str_1 ## str_2
#define __CONCAT(str_1,str_2) __MAKE_CONCAT(str_1,str_2)


/*--------------------------------------------------------------------------------------------------------------------------------*/
// call to this method only once
#define __CALL_ONCE(MethodName, ...)				\
													\
	{												\
		/* a static flag */							\
		static u8 __callFlag = false;				\
													\
		/* check if not called */					\
		if(!__callFlag){							\
													\
			/* call method */						\
			MethodName(__VA_ARGS__);				\
		}											\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// to support in run time abstract class instantiation and debug
#define __DEFINE_CHECK_VTABLE(ClassName)								\
																		\
	/* define the checking method */									\
	void ClassName  ## _checkVTable(){									\
																		\
		/* check that each entry in the table is not NULL */			\
		int i = 0;														\
		for(; i < sizeof(ClassName ## _vTable) / sizeof(void*); i++){	\
																		\
			/* check each entry */										\
			NM_ASSERT(((void**)&ClassName ## _vTable)[i],				\
					ClassName ## is abstract);							\
		}																\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// call to check that the vtable doesn't have null pointers
#define __CHECK_VTABLE(ClassName)							\
															\
	/* setup the class's vtable on first call only */		\
	__CALL_ONCE(ClassName  ## _checkVTable);


/*--------------------------------------------------------------------------------------------------------------------------------*/
// define the class's method which allocates the instances
#define __ALLOCATOR_DEFINITION(ClassName)									\
																			\
	/* define allocator */													\
	ClassName ClassName ## _allocator(){									\
																			\
		/* allocate object */												\
		ClassName this = (ClassName) 										\
						MemoryPool_allocate(MemoryPool_getInstance(), 		\
						sizeof(ClassName ## _str));							\
																			\
		/* initialize the class */											\
		__SET_CLASS(ClassName);												\
																			\
		/* abstract classes can't be instantiated */						\
		__CHECK_VTABLE(ClassName);											\
																			\
		/* return constructed object's address */							\
		return this;														\
																			\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// define the class's allocator declaration
#define __CLASS_NEW_DECLARE(ClassName, ...)						\
																	\
	/* define the method */											\
	ClassName ClassName ## _new(int dummy __VA_ARGS__)


/*--------------------------------------------------------------------------------------------------------------------------------*/
// define the class's allocator
#define __CLASS_NEW_DEFINITION(ClassName, ...)						\
																	\
	/* define the method */											\
	ClassName ClassName ## _new(int dummy __VA_ARGS__){				\
																	\
		/* create the object and allocate memory for it */			\
		ClassName this = ClassName ## _allocator();					\
																	\
		/* check if properly created */								\
		if(!this) return NULL;										\


/*--------------------------------------------------------------------------------------------------------------------------------*/
// end class's allocator
#define __CLASS_NEW_END(ClassName, ...)							\
																	\
		/* set the vtable pointer */								\
		this->vTable = &ClassName ## _vTable;						\
																	\
		/* construct the object */									\
		ClassName ## _constructor(this __VA_ARGS__);				\
																	\
		/* just for safety set it again */							\
		this->vTable = &ClassName ## _vTable;						\
																	\
		/* set dynamic flag */										\
		this->dynamic = true;										\
																	\
		/* return the created object */								\
		return this;												\
	}
	

/*--------------------------------------------------------------------------------------------------------------------------------*/
// like new in C++
#define __NEW(ClassName, ...)							\
														\
	/* call class's new implementation */				\
	ClassName ## _new(0 __VA_ARGS__)		
	

/* ---------------------------------------------------------------------------------------------------------*/
// like delete in C++ (calls virtual destructor)
#define __DELETE(Object)													\
																			\
	/* since the destructor is the first element in the virtual table */	\
	((void (*)(void*))((void***)Object)[0][0])(Object);		


/*--------------------------------------------------------------------------------------------------------------------------------*/
// like new in C++
#define __NEW_BASIC(ClassName)											\
																		\
	/* allocate data */													\
	(ClassName*)MemoryPool_allocate(MemoryPool_getInstance(),			\
		sizeof(ClassName));
		
	

/* ---------------------------------------------------------------------------------------------------------*/
// like delete in C++ (calls virtual destructor)
#define __DELETE_BASIC(Object)												\
																			\
	/* free the memory */													\
	MemoryPool_free(MemoryPool_getInstance(), (void*)Object)

/*--------------------------------------------------------------------------------------------------------------------------------*/
// construct the base object
#define __CONSTRUCT_BASE(ClassName, ...)								\
																		\
	/* make sure the this pointer is initialized */						\
	ASSERT(this, __MAKE_STRING(ClassName ## _constructor));				\
																		\
	/* call super constructor */										\
	ClassName ## _constructor((ClassName)this __VA_ARGS__);				


/*--------------------------------------------------------------------------------------------------------------------------------*/
// must always call base class's destructor
#define __DESTROY_BASE(SuperClass)									\
																	\
	/* call super class's destructor */								\
	SuperClass ## _destructor((SuperClass)this);					\
																	\
	/* if dynamically created */									\
	if(this->dynamic){												\
																	\
		/*  */														\
		this->dynamic = false;										\
																	\
		/* free the memory */										\
		MemoryPool_free(MemoryPool_getInstance(), (void*)this);		\
	}
				


/*--------------------------------------------------------------------------------------------------------------------------------*/
// retrieve virtual method's address
#define __VIRTUAL_CALL_ADDRESS(ClassName, MethodName, Object, ...)					\
																					\
	/* call derived implementation */												\
	(((struct ClassName ## _vTable*)((*((void**)Object))))->MethodName)

/*--------------------------------------------------------------------------------------------------------------------------------*/
// call a virtual method (in debug a check is performed to assert that the method isn't null)
#ifdef __DEBUG																		
#define __VIRTUAL_CALL(ReturnType, ClassName, MethodName, Object, ...)						\
		(																					\
			(__VIRTUAL_CALL_ADDRESS(ClassName, MethodName, Object, ...))?					\
			/* call derived implementation */												\
			((ReturnType (*)(ClassName, ...))												\
			(((struct ClassName ## _vTable*)((*((void**)Object))))->MethodName))			\
				(																			\
						Object __VA_ARGS__													\
				):																			\
			/* call base implementation */													\
			(ReturnType)Error_triggerException(Error_getInstance(),							\
				 /*"VMethod null "  ## __MAKE_STRING(ClassName ## _ ##  MethodName))*/			\
				 __MAKE_STRING(ClassName ## _ ##  MethodName))			\
		)


#else
#define __VIRTUAL_CALL(ReturnType, ClassName, MethodName, Object, ...)					\
		/* call derived implementation */												\
		((ReturnType (*)(ClassName, ...))												\
		(((struct ClassName ## _vTable*)((*((void**)Object))))->MethodName))			\
			(																			\
					Object __VA_ARGS__													\
			)																			\

#endif

/*--------------------------------------------------------------------------------------------------------------------------------*/
// cast macro
#define __GET_CAST(ClassName, Object)																\
		(																							\
			/* check if object's destructor matches class' destructor */							\
			((void*)ClassName ## _destructor == ((void (*)(void*))((void***)Object)[0][0]))?		\
																									\
			/* cast is safe */																		\
			(ClassName)Object																		\
																									\
			/* otherwise */																			\
			:																						\
			/* cast is null */																		\
			NULL																					\
		)
	

/*--------------------------------------------------------------------------------------------------------------------------------*/
// declare a virtual method
#define __VIRTUAL_DEC(MethodName)					\
													\
	/* define a virtual method pointer */			\
	void* MethodName


/*--------------------------------------------------------------------------------------------------------------------------------*/
// override a virtual method
#define __VIRTUAL_SET(ClassVTable, ClassName, MethodName)								\
																						\
	/* set the virtual method's address in its correspoiding vtable offset */			\
	ClassVTable ## _vTable.MethodName = ClassName ## _ ## MethodName


/*--------------------------------------------------------------------------------------------------------------------------------*/
// call configure class's vtable method
#define __SET_CLASS(ClassName)								\
															\
	/* setup the class's vtable on first call only */		\
	__CALL_ONCE(ClassName ## _setVTable)

/*--------------------------------------------------------------------------------------------------------------------------------*/
// configure class's vtable
#define __SET_VTABLE(ClassName)											\
																		\
	/* define the static method */										\
	static void ClassName ## _setVTable() 	{							\
																		\
		/* set the class's virtual methods */							\
		ClassName ## _SET_VTABLE(ClassName)								\
																		\
		/* always set the destructor at the end */						\
		__VIRTUAL_SET(ClassName, ClassName, destructor);				\
																		\
		/* always set the class name methos at the end */				\
		__VIRTUAL_SET(ClassName, ClassName, getClassName);				\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// class's vtable declaration and instantiation
#define __VTABLE(ClassName)									\
															\
	/* declare the vtable struct */							\
	struct ClassName ## _vTable{							\
															\
		/* all destructors are virtual */					\
		__VIRTUAL_DEC(destructor);							\
															\
		/* all destructors are virtual */					\
		__VIRTUAL_DEC(getClassName);						\
															\
		/* insert class's virtual methods names */			\
		ClassName ## _METHODS;								\
															\
	/* create the vtable instance */						\
	}ClassName ## _vTable;									\
															\


/*--------------------------------------------------------------------------------------------------------------------------------*/
// declare a class
#define __CLASS(ClassName)												\
																		\
	/* declare a const pointer */										\
	typedef struct ClassName ## _str* ClassName;						\
																		\
	/* declare vtable */												\
	__VTABLE(ClassName);												\
																		\
	/* declare getSize method */										\
	int ClassName ## _getObjectSize();									\
																		\
	/* declare getClass name method */									\
	char* ClassName ## _getClassName()

/*--------------------------------------------------------------------------------------------------------------------------------*/
// define a class
#define __CLASS_DEFINITION(ClassName)							\
																\
	typedef struct ClassName ## _str {							\
																\
		/* class attributes */									\
		ClassName ## _ATTRIBUTES								\
																\
		/* end definition */									\
	}ClassName ## _str;											\
																\
	/* define class's getSize method */							\
	int ClassName ## _getObjectSize(){							\
																\
		return sizeof(ClassName ## _str);						\
	}															\
																\
	/* define class's getSize method */							\
	char* ClassName ## _getClassName(){							\
																\
		return #ClassName;										\
	}															\
																\
	/* now add the function which will set the vtable */		\
	__SET_VTABLE(ClassName)										\
																\
	/* define vtable checker */									\
	__DEFINE_CHECK_VTABLE(ClassName)							\
																\
	/* define allocator */										\
	__ALLOCATOR_DEFINITION(ClassName)


/*--------------------------------------------------------------------------------------------------------------------------------*/
// declare an object type
#define __TYPE(ClassName)	ClassName ## _new

/*--------------------------------------------------------------------------------------------------------------------------------*/
// defines a singleton (unique instance of a class)
#define __SINGLETON(ClassName)										\
																	\
	/* declare the static instance */								\
	static ClassName ## _str _instance ## ClassName;				\
																	\
	/* a flag to know when to allow constructs */					\
	static u8 _singletonConstructed  ## ClassName = false;			\
																	\
	/* define get instance method */								\
	ClassName ClassName ## _getInstance(){							\
																	\
		/* set the vtable */										\
		__SET_CLASS(ClassName);										\
																	\
		/* first check if not constructed yet */					\
		if(!_singletonConstructed ## ClassName){					\
																	\
			/* call constructor */									\
			ClassName ## _constructor(&_instance ## ClassName);		\
																	\
			/* set the vtable pointer */							\
			_instance ## ClassName.vTable = &ClassName ## _vTable;	\
																	\
			/* don't allow more constructs */						\
			_singletonConstructed ## ClassName = true;				\
		}															\
																	\
		/* return the created singleton */							\
		return &_instance ## ClassName;								\
	}


/*--------------------------------------------------------------------------------------------------------------------------------*/
// must always call base class's destructor
#define __SINGLETON_DESTROY(SuperClass)								\
																	\
	/* destroy super object */										\
	__DESTROY_BASE(SuperClass);										\
																	\
	/* allow new constructs */										\
	//_singletonConstructed = false;



/*--------------------------------------------------------------------------------------------------------------------------------*/
// defines a dynamic singleton (unique instance of a class)
#define __SINGLETON_DYNAMIC(ClassName)								\
																	\
	/* declare the static pointer to instance */					\
	static ClassName _instance ## ClassName;						\
																	\
	/* define allocator */											\
	__CLASS_NEW_DEFINITION(ClassName);								\
	__CLASS_NEW_END(ClassName);										\
																	\
																	\
	/* a flag to know when to allow constructs */					\
	static u8 _singletonConstructed ## ClassName = false;			\
																	\
	/* define get instance method */								\
	ClassName ClassName ## _getInstance(){							\
																	\
		/* set the vtable */										\
		__SET_CLASS(ClassName);										\
																	\
		/* first check if not constructed yet */					\
		if(!_singletonConstructed ## ClassName){					\
																	\
			/* allocate */											\
			_instance ## ClassName = ClassName ## _new(0);			\
																	\
			/* don't allow more constructs */						\
			_singletonConstructed ## ClassName = true;				\
		}															\
																	\
		/* return the created singleton */							\
		return _instance ## ClassName;								\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// gcc has a bug, it doesn't move back the sp register after returning
// from a variadic call
#define __CALL_VARIADIC(VariadicFunctionCall)						\
																	\
	/* variadic function call */									\
	VariadicFunctionCall;											\
																	\
	/* sp fix displacement */										\
	asm("addi	20, sp, sp")


#endif /*OOP_H_*/
