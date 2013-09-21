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

#define __REPLACE(x)	x

/*--------------------------------------------------------------------------------------------------------------------------------*/
// concatenate two strigs
#define __MAKE_CONCAT(str_1,str_2) str_1 ## str_2
#define __CONCAT(str_1,str_2) __MAKE_CONCAT(str_1,str_2)


/*--------------------------------------------------------------------------------------------------------------------------------*/
// helpers' declarations
#define SUPER_METHODS __CONCAT(SUPER_CLASS, _METHODS)
#define SUPER_SET_VTABLE __CONCAT(SUPER_CLASS, _SET_VTABLE)
#define SUPER_ATTRIBUTES __CONCAT(SUPER_CLASS, _ATTRIBUTES)

/*--------------------------------------------------------------------------------------------------------------------------------*/
// call to this method only once
#define __CALL_ONCE(MethodName, ...)							\
																\
	{															\
		/* a static flag */										\
		static int __CONCAT(MethodName, __callFlag) = false;	\
																\
		/* check if not called */								\
		if(!__CONCAT(MethodName, __callFlag)){					\
																\
			__CONCAT(MethodName, __callFlag) = true;			\
																\
			/* call method */									\
			MethodName(__VA_ARGS__);							\
		}														\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// to support in run time abstract class instantiation and debug
#define __DEFINE_CHECK_VTABLE													\
																				\
	/* define the checking method */											\
	void __REPLACE(CLASS)  ## _checkVTable(){									\
																				\
		/* check that each entry in the table is not NULL */					\
		int i = 0;																\
		for(; i < sizeof(__REPLACE(CLASS) ## _vTable) / sizeof(void*); i++){	\
																				\
			/* check each entry */												\
			NM_ASSERT(((void**)&__REPLACE(CLASS) ## _vTable)[i],				\
					__REPLACE(CLASS) ## : is abstract);							\
		}																		\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// call to check that the vtable doesn't have null pointers
#define __CHECK_VTABLE										\
															\
	/* setup the class's vtable on first call only */		\
	__CALL_ONCE(__REPLACE(CLASS)  ## _checkVTable);


/*--------------------------------------------------------------------------------------------------------------------------------*/
// define the class's method which allocates the instances
#define __ALLOCATOR_DEFINITION												\
																			\
	/* define allocator */													\
	__REPLACE(CLASS) __REPLACE(CLASS) ## _allocator(){						\
																			\
		/* allocate object */												\
		__REPLACE(CLASS) this = (__REPLACE(CLASS)) 							\
						MemoryPool_allocate(MemoryPool_getInstance(), 		\
						sizeof(__REPLACE(CLASS) ## _str));					\
																			\
		/* initialize the class */											\
		__SET_CLASS;														\
																			\
		/* abstract classes can't be instantiated */						\
		__CHECK_VTABLE;														\
																			\
		/* return constructed object's address */							\
		return this;														\
																			\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// define the class's allocator declaration
#define __CLASS_NEW_DECLARE(...)									\
																	\
	/* define the method */											\
	__REPLACE(CLASS) __CONCAT(CLASS, _new(int dummy __VA_ARGS__))


/*--------------------------------------------------------------------------------------------------------------------------------*/
// define the class's allocator
#define __CLASS_NEW_DEFINITION(...)											\
																			\
	/* define vtable checker */												\
	__DEFINE_CHECK_VTABLE													\
																			\
	/* define allocator */													\
	__ALLOCATOR_DEFINITION													\
																			\
	/* define the method */													\
	__REPLACE(CLASS) __REPLACE(CLASS) ## _new(int dummy __VA_ARGS__){		\
																			\
		/* create the object and allocate memory for it */					\
		__REPLACE(CLASS) this = __REPLACE(CLASS) ## _allocator();			\
																			\
		/* check if properly created */										\
		if(!this) return NULL;												\


/*--------------------------------------------------------------------------------------------------------------------------------*/
// end class's allocator
#define __CLASS_NEW_END(...)										\
																	\
		/* set the vtable pointer */								\
		this->vTable = &__REPLACE(CLASS) ## _vTable;				\
																	\
		/* construct the object */									\
		__REPLACE(CLASS) ## _constructor(this __VA_ARGS__);			\
																	\
		/* just for safety set it again */							\
		this->vTable = &__REPLACE(CLASS) ## _vTable;				\
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
#define __CONSTRUCT_BASE(...)											\
																		\
	/* make sure the this pointer is initialized */						\
	ASSERT(this, __REPLACE(SUPER_CLASS) ## _constructor);				\
																		\
	/* call super constructor */										\
	__REPLACE(SUPER_CLASS) ## _constructor((SUPER_CLASS)this __VA_ARGS__));				


/*--------------------------------------------------------------------------------------------------------------------------------*/
// must always call base class's destructor
#define __DESTROY_BASE												\
																	\
	/* call super class's destructor */								\
	__REPLACE(SUPER_CLASS) ## _destructor((SUPER_CLASS)this);		\
																	\
	/* if dynamically created */									\
	if(this->dynamic){												\
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
				" VMethod null: "  ## __MAKE_STRING(ClassName ## _ ##  MethodName))			\
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
#define __VIRTUAL_SET(ClassName, MethodName)											\
																						\
	/* set the virtual method's address in its correspoiding vtable offset */			\
	__REPLACE(CLASS) ## _vTable.MethodName = ClassName ## _ ## MethodName


/*--------------------------------------------------------------------------------------------------------------------------------*/
// call configure class's vtable method
#define __SET_CLASS											\
															\
	/* setup the class's vtable on first call only */		\
	__CALL_ONCE(__REPLACE(CLASS) ## _setVTable)

/*--------------------------------------------------------------------------------------------------------------------------------*/
// configure class's vtable
#define __SET_VTABLE													\
																		\
	/* define the static method */										\
	static void __REPLACE(CLASS) ## _setVTable(){						\
																		\
		/* set the class's virtual methods */							\
		__CONCAT(SUPER_CLASS, _SET_VTABLE)								\
																		\
		/* set the class's virtual methods */							\
		__CONCAT(CLASS, _SET_VTABLE)									\
																		\
		/* always set the destructor at the end */						\
		__VIRTUAL_SET(__REPLACE(CLASS), destructor);					\
	}

/*--------------------------------------------------------------------------------------------------------------------------------*/
// class's vtable declaration and instantiation
#define __VTABLE											\
															\
	/* declare the vtable struct */							\
	struct __REPLACE(CLASS) ## _vTable{						\
															\
		/* all destructors are virtual */					\
		__VIRTUAL_DEC(destructor);							\
															\
		/* insert class's virtual methods names */			\
		__REPLACE(__CONCAT(CLASS, _METHODS));				\
															\
	/* create the vtable instance */						\
	}__REPLACE(CLASS) ## _vTable;

/*--------------------------------------------------------------------------------------------------------------------------------*/
// declare a class
#define __CLASS															\
																		\
	/* declare a const pointer */										\
	typedef struct __REPLACE(CLASS) ##	_str* ## __REPLACE(CLASS);		\
																		\
	/* declare vtable */												\
	__VTABLE;															\
																		\
	/* declare getSize method */										\
	int __REPLACE(CLASS) ## _getObjectSize()

/*--------------------------------------------------------------------------------------------------------------------------------*/
// define a class
#define __CLASS_DEFINITION										\
																\
	typedef struct __REPLACE(CLASS)##_str {						\
																\
		/* class attributes */									\
		__CONCAT(SUPER_CLASS, _ATTRIBUTES)						\
																\
		/* class attributes */									\
		__CONCAT(CLASS, _ATTRIBUTES)							\
																\
		/* end definition */									\
	}__REPLACE(CLASS) ## _str;									\
																\
	/* define class's getSize method */							\
	int __REPLACE(CLASS) ## _getObjectSize(){					\
																\
		return sizeof(__REPLACE(CLASS) ## _str);				\
	}															\
																\
	/* now add the function which will set the vtable */		\
	__SET_VTABLE							

																		
/*--------------------------------------------------------------------------------------------------------------------------------*/
// declare an object type
#define __TYPE(ClassName)	ClassName ## _new

/*--------------------------------------------------------------------------------------------------------------------------------*/
// defines a singleton (unique instance of a class)
#define __SINGLETON																	\
																					\
	/* declare the static instance */												\
	static __REPLACE(CLASS) ## _str _instance ## __REPLACE(CLASS);					\
																					\
	/* a flag to know when to allow constructs */									\
	static int _singletonConstructed = false;										\
																					\
	/* define get instance method */												\
	__REPLACE(CLASS) __REPLACE(CLASS) ## _getInstance(){							\
																					\
		/* set the vtable */														\
		__SET_CLASS;																\
																					\
		/* first check if not constructed yet */									\
		if(!_singletonConstructed){													\
																					\
			/* call constructor */													\
			__REPLACE(CLASS) ## _constructor(&_instance ## __REPLACE(CLASS));		\
																					\
			/* set the vtable pointer */											\
			_instance ## __REPLACE(CLASS).vTable = &__REPLACE(CLASS) ## _vTable;	\
																					\
			/* don't allow more constructs */										\
			_singletonConstructed = true;											\
		}																			\
																					\
		/* return the created singleton */											\
		return &_instance ## __REPLACE(CLASS);										\
	}


/*--------------------------------------------------------------------------------------------------------------------------------*/
// must always call base class's destructor
#define __SINGLETON_DESTROY											\
																	\
	/* destroy super object */										\
	__DESTROY_BASE;													\
																	\
	/* allow new constructs */										\
	_singletonConstructed = false;



/*--------------------------------------------------------------------------------------------------------------------------------*/
// defines a dynamic singleton (unique instance of a class)
#define __SINGLETON_DYNAMIC														\
																				\
	/* declare the static pointer to instance */								\
	static __REPLACE(CLASS) _instance ## __REPLACE(CLASS);						\
																				\
	/* define allocator */														\
	__CLASS_NEW_DEFINITION();													\
	__CLASS_NEW_END();															\
																				\
																				\
	/* a flag to know when to allow constructs */								\
	static int _singletonConstructed = false;									\
																				\
	/* define get instance method */											\
	__REPLACE(CLASS) __REPLACE(CLASS) ## _getInstance(){						\
																				\
		/* set the vtable */													\
		__SET_CLASS;															\
																				\
		/* first check if not constructed yet */								\
		if(!_singletonConstructed){												\
																				\
			/* allocate */														\
			_instance ## __REPLACE(CLASS) = __REPLACE(CLASS) ## _new(0);		\
																				\
			/* don't allow more constructs */									\
			_singletonConstructed = true;										\
		}																		\
																				\
		/* return the created singleton */										\
		return _instance ## __REPLACE(CLASS);									\
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
