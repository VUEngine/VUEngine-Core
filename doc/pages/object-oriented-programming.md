Object Oriented Programming
===========================

Since the nature of games is better and more easily represented by the OOP paradigm than a structured one, because the former allows for better code re-usage and flexibility, and since the available compiler for the VB only supports C,a set of C MACROS have been created for this engine to simulate some of the most visible features provided by C++:

- Inheritance
- Polymorphism
- Encapsulation

In order to use these features, you must be comfortable using some MACRO calls which will be explained next.


Creating a Class
----------------

Every class in the engine and the game must inherit from a base class called Object or from another class which inherits from it.

Let's create a Hero class which inherits from the Character class provided with the engine. In the header file Hero.h the following macros must be placed. This will allow the Hero class to inherit the virtual methods from the Character class.

    #define Hero_METHODS                                        \
        Character_METHODS;

Next, it is necessary to inherit and/or redefine those method's definitions.

    #define Hero_SET_VTABLE(ClassName)                          \
        Character_SET_VTABLE(ClassName);                        \
        __VIRTUAL_SET(ClassName, Hero, die);

This tells the engine that Character has a virtual method called `die` (`Character_die` actually) and we want to redefine that method with our own version of `die`, thus allowing *Polymorphism*.

Then you must declare the class with the following line:

    __CLASS(Hero);

This will define a pointer Hero to a struct, this way the Hero class's implementation is hidden from client code, making it impossible to access private members, which provides *Encapsulation*.

Then you must declare Hero class's specific attributes, to do so, declare the following MACRO:

    #define Hero_ATTRIBUTES                             \
                                                        \
    /* it is derived from */                            \
    Character_ATTRIBUTES                                \
                                                        \
    /* hero has energy    */                            \
    u8 energy;


Notice that all of these macros have a backslash ("\") at the end of each line, you must be careful and always be sure that there are no blank or tab spaces after them. The reason to declare the class's attributes this way is because this allows to inherit methods/attributes, and at the same time allows to make the attributes private. Notice that in order for Hero to inherit Character's attributes to include `Character_ATTRIBUTES` in `Hero_ATTRIBUTES`.

The last thing to be done in the header file is to declare the following methods:

**Allocator:** 

All classes must follow the following format (the arguments are optional).

    Hero Hero_new(CharacterDefinition* animatedEntityDefinition, int ID);

**Constructor:**
 
The first argument is mandatory.

    void Hero_constructor(Hero this, CharacterDefinition* definition, int ID);

**Destructor:**
 
The argument is mandatory and must only be one in all cases.

    void Hero_destructor(Hero this);

Now it is time to define the class. In the source file Hero.c do as follows.
Include the header file which holds the class's declaration:

    #include "Hero.h"

Define the class:

    // A Hero! Which inherits from Character
    __CLASS_DEFINITION(Hero, Character);

Define the allocator:

    // always call these to macros next to each other
    __CLASS_NEW_DEFINITION(Hero, ActorDefinition* actorDefinition, int ID)
    __CLASS_NEW_END(Hero, this, actorDefinition, ID);

Define the constructor: must always call the parent class's constructor to properly initialize the object.

    // class's constructor
    void Hero_constructor(Hero this, ActorDefinition* actorDefinition, int ID)
    {
        __CONSTRUCT_BASE(this, actorDefinition, ID);
    
        this->energy = 1;
    
        ...
    } 

Define the destructor: must always destroy the parent class at the end of the method.

    // class's destructor
    void Hero_destructor(Hero this)
    {
        // free space allocated here
    
        // delete the super object
        __DEALLOCATE;
    } 


Virtual Calls
-------------

The purpose of having OOP features is to allow generic programming through the use of virtual calls to class methods through a base class pointer. For example, the Stage has a list of Entities (from which Character, Background and Image inherit) and it must be able to call the proper update and render methods on those classes. To do so, there are two possible ways:

- Using a switch statement to determine which type of object is being pointed to by the parent class pointer, which can mean having to store extra info on each object to hold the type, and can be quite cumbersome if we extend to have more kind of Entities.
- The other way is to use virtual calls to virtual methods, as shown below:

      // update each entity's internal state
      void Stage_update(Stage this)
      {
          VirtualNode node = VirtualList_begin(this->entities);
    
          for(; node ; node = VirtualNode_getNext(node)) 
          {
              __VIRTUAL_CALL(void, Entity, update,(Entity)VirtualNode_getData(node));
          }
      } 

As you can see, there is only one call to the method, which depends on the type of object that is currently being processed.


Abstract class
--------------

To define an abstract class, simply omit the `__VIRTUAL_SET` definition.

Trying to instantiate an abstract Entity will result in an exception in `__DEBUG` mode.


Friend class
------------

The engine supports friend classes through the use of the following macro:
 
	__CLASS_FRIEND_DEFINITION(ClassName)

This allow access to the friend class' attributes through a pointer to an instance of that class.

Singleton
---------

You can define your class as a singleton to restrict its instantiation to one object only. This is very useful for state objects as an example. 
To make your class a singleton, call the respective macro right after the class definition:

    __CLASS_DEFINITION(ExampleState, GameState);
    __SINGLETON(ExampleState);

There's two different ways to define a singleton: 
`__SINGLETON`, where an instance is a global variable that is allocated in the program's bss section, and
`__SINGLETON_DYNAMIC` where an instance is allocated in the memory pool. 
The purpose of `__SINGLETON_DYNAMIC()` is to being able to free up memory while not straining the engine. 
That means that states which are used a lot during gameplay (for example the main character's states) are not good candidates for `__SINGLETON_DYNAMIC` because it'd mean a lot of memory allocations and deallocations.

Rule of thumb: If the state is either used a lot during gameplay, or you need to keep a reference to it after the state has exited, use `__SINGLETON`, otherwise `__SINGLETON_DYNAMIC` should be the better choice.
  
In any case, a singleton's instance can get retrieved through the `[ClassName]_getInstance();` method.

A singleton instance is destroyed with a call to `__SINGLETON_DESTROY` in the class' destructor. 
In the case of a `__SINGLETON_DYNAMIC`, destroying the instance results in the next call to `getInstance()` to allocate a new instance in the memory pool. 
In the case of a `__SINGLETON`, the only thing that trying to delete it accomplishes is that the constructor gets called again during the next call to `getInstance()`.

Runtime type checking
---------------------

The engine supports runtime type checking through the following methods and macros:

#### Upcast and downcast

It is possible to upcast or downcast object pointers by using the following macro:

	__GET_CAST(ClassName, object)
	
If the cast success, the returned value is the same object pointer; if it fails, the returned value is NULL.

These cases should be sparingly used since the performance overhead that they produce can have a negative impact in the game's performance.

#### Get object's class' name

To get the name of the class of a given object use the following macro:

	__GET_CLASS_NAME(object)
	
This macro replaces the call:

	__VIRTUAL_CALL(Object, getClassName, (Object)object)

The call will return a pointer to const char* that holds the name of the object's class

#### Check if an object is an instance of a class

It is possible to check if an object is an instance of a given class without having to type cast it by using the following code:

	__IS_INSTANCE_OF(ClassName, object)
	
The drawback is that this check can only detect if the object was instantiated calling the given class' allocator, but cannot tell whether the object inherits from the class or not.
