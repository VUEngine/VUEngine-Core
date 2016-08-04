/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef OOP_H_
#define OOP_H_


#define __MAKE_STRING(a) #a

// concatenate two strings
#define __MAKE_CONCAT(str_1,str_2) str_1 ## str_2
#define __CSTOM_CONCAT(str_1,str_2) __MAKE_CONCAT(str_1,str_2)

// call to this method only once
#define __CALL_ONCE(MethodName, ...)															        \
                                                                                                        \
        {																							    \
            /* a static flag */																		    \
            static bool __notCalledFlag = true;                                             		    \
                                                                                                        \
            /* check if not called */																    \
            if(__notCalledFlag)																		    \
            {																						    \
                /* call method */																	    \
                __notCalledFlag = false;															    \
                                                                                                        \
                /* call method */																	    \
                MethodName(__VA_ARGS__);															    \
            }																						    \
        }

// to support in run time abstract class instantiation and debug
#define __DEFINE_CHECK_VTABLE(ClassName)														        \
                                                                                                        \
        /* define the checking method */															    \
        void __attribute__ ((noinline)) ClassName  ## _checkVTable()									\
        {																							    \
            /* check that each entry in the table is not NULL */									    \
            int i = 0;																				    \
            for(; i < sizeof(ClassName ## _vTable) / sizeof(void*); i++)							    \
            {																						    \
                /* check each entry */																    \
                NM_ASSERT(((void**)&ClassName ## _vTable)[i],										    \
                        ClassName ##  is abstract);													    \
            }																						    \
        }

#ifdef __CHECK_ABSTRACT_CLASS_INSTANTIATION
// call to check that the vtable doesn't have null pointers
#define __CHECK_VTABLE(ClassName)																        \
                                                                                                        \
        /* setup the class's vtable on first call only */											    \
        __CALL_ONCE(ClassName  ## _checkVTable);
#else
#define __CHECK_VTABLE(ClassName)
#endif

// define the class's allocator declaration
#define __CLASS_NEW_DECLARE(ClassName, ...)														        \
                                                                                                        \
        /* define the method */																		    \
        ClassName ClassName ## _new(__VA_ARGS__)


// define the class's allocator
#define __CLASS_NEW_DEFINITION(ClassName, ...)													        \
                                                                                                        \
        /* define the method */																		    \
        ClassName ClassName ## _new(__VA_ARGS__)													    \
        {																							    \
            /* setup the class's vtable on first call only */										    \
            __SET_CLASS(ClassName);																	    \
                                                                                                        \
            /* abstract classes can't be instantiated */											    \
            __CHECK_VTABLE(ClassName);																    \
                                                                                                        \
            /* allocate object */																	    \
            ClassName this = (ClassName) 															    \
                            MemoryPool_allocate(MemoryPool_getInstance(), 							    \
                            sizeof(ClassName ## _str));												    \
                                                                                                        \
            /* check if properly created */															    \
            if(!this) return NULL;																	    \


// end class's allocator
#define __CLASS_NEW_END(ClassName, ...)															        \
                                                                                                        \
            /* set the vtable pointer */															    \
            this->vTable = (void*)&ClassName ## _vTable;												\
                                                                                                        \
            /* construct the object */																    \
            ClassName ## _constructor(this, ##__VA_ARGS__);											    \
                                                                                                        \
            ASSERT(this->vTable == &ClassName ## _vTable,                                               \
               __MAKE_STRING(ClassName) "::new: vTable not set properly");                              \
                                                                                                        \
            /* return the created object */															    \
            return this;																			    \
        }                                                                                               \
                                                                                                        \
        /* dummy redeclaration to avoid warning when compiling with -pedantic */                        \
        void ClassName ## dummyMethodClassNew()

#define	__DYNAMIC_STRUCT_PAD	4

// like new in C++
#define __NEW(ClassName, ...)																	        \
                                                                                                        \
        /* call class's new implementation */														    \
        ClassName ## _new(__VA_ARGS__)																    \


// like delete in C++ (calls virtual destructor)
#define __DELETE(object)																		        \
                                                                                                        \
        /* since the destructor is the first element in the virtual table */						    \
        ASSERT(object && *(u32*)object, "Deleting null object");									    \
        /* ((void (*)(void*))((void***)object)[0][0])(object); */										\
		((((struct Object ## _vTable*)((*((void**)object))))->destructor))((Object)object)              \

// like new in C++
#define __NEW_BASIC(ClassName)																	        \
                                                                                                        \
        /* allocate data */																			    \
        (ClassName*)(MemoryPool_allocate(MemoryPool_getInstance(),									    \
            sizeof(ClassName) + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);						    \

// like delete in C++ (calls virtual destructor)
#define __DELETE_BASIC(object)																	        \
                                                                                                        \
        /* free the memory */																		    \
        ASSERT(object && *(u32*)((u32)object - __DYNAMIC_STRUCT_PAD), 								    \
                "Oop: deleting null basic object");													    \
        MemoryPool_free(MemoryPool_getInstance(), (BYTE*)object - __DYNAMIC_STRUCT_PAD)				    \

// construct the base object
#define __CONSTRUCT_BASE(BaseClass, ...)														        \
                                                                                                        \
        /* call base's constructor */																    \
        BaseClass ## _constructor(__SAFE_CAST(BaseClass, this), ##__VA_ARGS__);							\

// must always call base class's destructor
#define __DESTROY_BASE																			        \
                                                                                                        \
        /* since the base destructor is the second element in the virtual table */					    \
        _baseDestructor(__SAFE_CAST(Object, this));														\
                                                                                                        \
        /* free the memory */																		    \
        MemoryPool_free(MemoryPool_getInstance(), (void*)this);										    \

// retrieve virtual method's address
#define __VIRTUAL_CALL_ADDRESS(ClassName, MethodName, object)                                           \
                                                                                                        \
        /* call derived implementation */															    \
        (((struct ClassName ## _vTable*)((*((void**)object))))->MethodName)							    \

// call a virtual method (in debug a check is performed to assert that the method isn't null)
#define __VIRTUAL_CALL(ClassName, MethodName, object, ...)							                    \
		/* release implementation */															        \
		((((struct ClassName ## _vTable*)((*((void**)object))))->MethodName))					        \
			(																					        \
				__SAFE_CAST(ClassName, object), ##__VA_ARGS__									        \
			)																					        \


#ifdef __DEBUG
#define __SAFE_CAST(ClassName, object)															        \
																								        \
		/* try to up cast object */																        \
		(ClassName)Object_getCast((Object)object, (ObjectBaseClassPointer)&ClassName ## _getBaseClass, NULL)
#else
#define __SAFE_CAST(ClassName, object) (ClassName)object
#endif

#define __GET_CAST(ClassName, object)															        \
																								        \
		/* try to up cast object */																        \
		(ClassName)Object_getCast((Object)object, (ObjectBaseClassPointer)&ClassName ## _getBaseClass, NULL)	\

// declare a virtual method
#define __VIRTUAL_DEC(ClassName, ReturnType, MethodName, ...)									        \
                                                                                                        \
        /* define a virtual method pointer */														    \
        ReturnType (*MethodName)(ClassName, ##__VA_ARGS__)                                              \

// override a virtual method
#define __VIRTUAL_SET(ClassVTable, ClassName, MethodName)										        \
		{																						        \
		    /* use a temporal pointer to avoid illegal cast between pointers to data and functions */	\
            void (*(*tempPointer))() = (void (*(*))())&ClassVTable ## _vTable.MethodName;               \
            *(tempPointer) = (void (*)())&ClassName ## _ ## MethodName;                                 \
        }

// call configure class's vtable method
#define __SET_CLASS(ClassName)																	        \
                                                                                                        \
        /* setup the class's vtable only if destructor is NULL */									    \
        if(!ClassName ## _vTable.destructor)                                                		    \
        {																							    \
            ClassName ## _setVTable();                                                      		    \
        }																							    \

// configure class's vtable
#define __SET_VTABLE(ClassName, BaseClassName)													        \
                                                                                                        \
        /* define the static method */																    \
        void __attribute__ ((noinline)) ClassName ## _setVTable()								        \
        {																							    \
            /* set the base class's virtual methods */												    \
            if(&ClassName ## _setVTable != &BaseClassName ## _setVTable)                                \
            {																						    \
                BaseClassName ## _setVTable();														    \
            }																						    \
                                                                                                        \
            /* set the class's virtual methods */													    \
            ClassName ## _SET_VTABLE(ClassName)														    \
                                                                                                        \
            /* set the destructor */							                					    \
            __VIRTUAL_SET(ClassName, ClassName, destructor);										    \
                                                                                                        \
            /* set the getBaseClass method */					                					    \
            __VIRTUAL_SET(ClassName, ClassName, getBaseClass);										    \
                                                                                                        \
            /* set the getClassName method */					                					    \
            __VIRTUAL_SET(ClassName, ClassName, getClassName);										    \
        }                                                                                               \

// class's vtable declaration and instantiation
#define __VTABLE(ClassName)									                					        \
                                                                                                        \
        /* declare the vtable struct */																    \
        struct ClassName ## _vTable 					        									    \
        {																							    \
            /* all destructors are virtual */														    \
            __VIRTUAL_DEC(ClassName, void, destructor);		    									    \
                                                                                                        \
            /* get super class method */															    \
            __VIRTUAL_DEC(ClassName, ObjectBaseClassPointer, getBaseClass);									\
                                                                                                        \
            /* all destructors are virtual */														    \
            __VIRTUAL_DEC(ClassName, char*, getClassName);											    \
                                                                                                        \
            /* insert class's virtual methods names */												    \
            ClassName ## _METHODS(ClassName)							                			    \
        }							                                                        		    \

// declare a class
#define __CLASS(ClassName)              														        \
                                                                                                        \
        /* declare a const pointer */																    \
        typedef struct ClassName ## _str* ClassName;												    \
                                                                                                        \
        /* typedef for RTTI */																            \
        typedef void* (*(*ClassName ## BaseClassPointer)(Object))(Object);                               \
                                                                                                        \
        /* declare vtable */																		    \
        __VTABLE(ClassName);								            							    \
                                                                                                        \
        /* declare vtable */																		    \
        void ClassName ## _setVTable();																    \
                                                                                                        \
        /* declare getSize method */																    \
        int ClassName ## _getObjectSize();													            \
                                                                                                        \
        /* declare getBaseClass method */															    \
        ObjectBaseClassPointer ClassName ## _getBaseClass(Object);									        \
                                                                                                        \
        /* declare getClass name method */															    \
        char* ClassName ## _getClassName(ClassName)


// to being able to friend a class
#define __CLASS_FRIEND_DEFINITION(ClassName)													        \
        typedef struct ClassName ## _str															    \
        {																							    \
            /* class attributes */																	    \
            ClassName ## _ATTRIBUTES																    \
                                                                                                        \
            /* end definition */																	    \
        } ClassName ## _str 																		    \

// define a class
#define __CLASS_DEFINITION(ClassName, BaseClassName)											        \
                                                                                                        \
        typedef struct ClassName ## _str															    \
        {																							    \
            /* class attributes */																	    \
            ClassName ## _ATTRIBUTES            													    \
                                                                                                        \
            /* end definition */																	    \
        } ClassName ## _str;																		    \
                                                                                                        \
        /* class' vtable's definition */								    		    			    \
        struct ClassName ## _vTable ClassName ## _vTable __VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE;       \
                                                                                                        \
        static void (* const _baseDestructor)(Object) =                                     		    \
        /* class' base's destructor */					    		    	                		    \
            (void (*)(Object))&BaseClassName ## _destructor;				    	        		    \
                                                                                                        \
        /* define class's getSize method */															    \
        int ClassName ## _getObjectSize()												                \
        {																							    \
            return sizeof(ClassName ## _str);														    \
        }																							    \
                                                                                                        \
        /* define class's getBaseClass method */													    \
        ObjectBaseClassPointer ClassName ## _getBaseClass(Object this)				                		\
        {																							    \
            return (ObjectBaseClassPointer)&BaseClassName ## _getBaseClass;									\
        }																							    \
                                                                                                        \
        /* define class's getSize method */															    \
        char* ClassName ## _getClassName(ClassName this)										        \
        {																							    \
            return #ClassName;																		    \
        }																							    \
                                                                                                        \
        /* now add the function which will set the vtable */										    \
        __SET_VTABLE(ClassName, BaseClassName)														    \
                                                                                                        \
        /* dummy redeclaration to avoid warning when compiling with -pedantic */                        \
        void ClassName ## _dummyMethodClassDefinition()

// retrieves object's class' name
#define __GET_CLASS_NAME(object)																        \
                                                                                                        \
        __VIRTUAL_CALL(Object, getClassName, (Object)object)

// retrieves object's class' name
#define __GET_CLASS_NAME_UNSAFE(object)                                                                 \
                                                                                                        \
        __VIRTUAL_CALL(Object, getClassName, (Object)object)

// declare an object type
#define __TYPE(ClassName)	                            (AllocatorPointer)&ClassName ## _new
#define __ALLOCATOR_TYPE(allocatorPointer)	            (AllocatorPointer)allocatorPointer


#define __SINGLETON_NOT_CONSTRUCTED			0
#define __SINGLETON_BEING_CONSTRUCTED		1
#define __SINGLETON_CONSTRUCTED				2

// defines a singleton (unique instance of a class)
#define __SINGLETON(ClassName, ...)																        \
                                                                                                        \
        /* declare the static instance */															    \
        static ClassName ## _str _instance ## ClassName __STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE;     \
                                                                                                        \
        /* a flag to know when to allow construction */												    \
        static s8 _singletonConstructed __INITIALIZED_DATA_SECTION_ATTRIBUTE =                          \
                __SINGLETON_NOT_CONSTRUCTED;                                                            \
                                                                                                        \
        /* define get instance method */															    \
        static void __attribute__ ((noinline)) ClassName ## _instantiate()								\
        {																							    \
            NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,                           \
                ClassName get instance during construction);						                    \
                                                                                                        \
            /* set the vtable */																	    \
            __SET_CLASS(ClassName);																	    \
                                                                                                        \
            _singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;								        \
                                                                                                        \
            /* set the vtable pointer */														        \
            _instance ## ClassName.vTable = &ClassName ## _vTable;								        \
                                                                                                        \
            /* call constructor */																        \
            ClassName ## _constructor(&_instance ## ClassName);									        \
                                                                                                        \
            /* set the vtable pointer */														        \
            _instance ## ClassName.vTable = &ClassName ## _vTable;								        \
                                                                                                        \
            /* don't allow more constructs */													        \
            _singletonConstructed = __SINGLETON_CONSTRUCTED;									        \
        }																						        \
                                                                                                        \
        /* define get instance method */															    \
        ClassName ClassName ## _getInstance()														    \
        {																							    \
            /* first check if not constructed yet */												    \
            if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)								    \
            {																						    \
                ClassName ## _instantiate();                                                            \
            }																						    \
                                                                                                        \
            /* return the created singleton */														    \
            return &_instance ## ClassName;															    \
        }                                                                                               \
                                                                                                        \
        /* dummy redeclaration to avoid warning when compiling with -pedantic */                        \
        void ClassName ## dummyMethodSingleton()

// must always call base class's destructor
#define __SINGLETON_DESTROY																		        \
                                                                                                        \
        /* destroy super object */																	    \
        __DESTROY_BASE;																				    \
                                                                                                        \
        /* allow new constructs */																	    \
        _singletonConstructed = __SINGLETON_NOT_CONSTRUCTED;										    \


// defines a dynamic singleton (unique instance of a class)
#define __SINGLETON_DYNAMIC(ClassName)															        \
																								        \
        /* declare the static pointer to instance */												    \
        static ClassName _instance ## ClassName __NON_INITIALIZED_DATA_SECTION_ATTRIBUTE;               \
                                                                                                        \
        /* define allocator */																		    \
        __CLASS_NEW_DEFINITION(ClassName)															    \
        __CLASS_NEW_END(ClassName);																	    \
                                                                                                        \
        /* a flag to know when to allow construction */												    \
        static s8 _singletonConstructed __INITIALIZED_DATA_SECTION_ATTRIBUTE =                          \
            __SINGLETON_NOT_CONSTRUCTED;                                                                \
                                                                                                        \
        /* define get instance method */															    \
        static void __attribute__ ((noinline)) ClassName ## _instantiate()								\
        {																							    \
            NM_ASSERT(__SINGLETON_BEING_CONSTRUCTED != _singletonConstructed,                           \
                ClassName get instance during construction);						                    \
                                                                                                        \
            /* set the vtable */																	    \
            __SET_CLASS(ClassName);																	    \
                                                                                                        \
            _singletonConstructed = __SINGLETON_BEING_CONSTRUCTED;								        \
                                                                                                        \
            /* allocate */																		        \
            _instance ## ClassName = ClassName ## _new();										        \
                                                                                                        \
            /* don't allow more constructs */													        \
            _singletonConstructed = __SINGLETON_CONSTRUCTED;									        \
        }																							    \
                                                                                                        \
        /* define get instance method */															    \
        ClassName ClassName ## _getInstance()														    \
        {																							    \
            /* first check if not constructed yet */												    \
            if(__SINGLETON_NOT_CONSTRUCTED == _singletonConstructed)								    \
            {																						    \
                ClassName ## _instantiate();                                                            \
            }																						    \
                                                                                                        \
            /* return the created singleton */														    \
            return _instance ## ClassName;															    \
        }                                                                                               \
                                                                                                        \
        /* dummy redeclaration to avoid warning when compiling with -pedantic */                        \
        void ClassName ## dummyMethodSingletonNew()


#endif
