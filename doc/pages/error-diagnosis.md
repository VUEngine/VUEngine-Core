Error diagnosis
===============

Since the engine makes heavy use of pointer's logic, it is really easy to trigger difficult to find bugs. In order to mitigate this issue, the engine provides the following aids:


Asserts
-------

Use the ASSERT macro to check every pointer or variable which can be troublesome; in particular, place an ASSERT checking that the "this" pointer passed to the class's methods is not NULL.

The engine provides two kinds of ASSERT macros, which check a given statement and throw an exception with a given error message if this statement returns false. These macros should be used throughout the code to make debugging easier. Since the engine relies on heavy pointer usage, it is common to operate on a NULL pointer and get lost.

### ASSERT

Only inserted when compiling under debug. It is used at the start of most of the engine's methods to check that the "this" pointer is not NULL. Since the MemoryPool writes a 0 in the first byte of a deleted pointer, this helps to assure that any memory slot within the MemoryPool's pools has a 0 when it has been deleted.

Another good use case for this would be to check an object's class against the expected class as shown below.

	ASSERT(__GET_CAST(ClassName, someObject), "ClassName::methodName: Wrong object class");

### NM_ASSERT

Inserted under any compilation type (NM stands for "non maskerable"). This macro is meant to be placed in sensible parts of the code. Here's a few examples of usage in VUEngine:

- **MemoryPool allocation**
  To let you know that the memory is full, otherwise extremely hard to track bugs occur
- **SpriteManager, registering a new Sprite**
  To let you know that there are no more WORLDs available
- **ParamTableManager, registering a new Sprite**
  To let you know that param memory is depleted



Initialize everything
---------------------

One of the most difficult, and common source of hard to diagnose bugs, are uninitialized variables; random crashes or completely strange behavior often are caused by not properly initialized variables. To aid the detection of such mistakes, in the config.h file, define the __MEMORY_POOL_CLEAN_UP macro, this will force the engine to put every memory pool's free block to 0 when the game changes its state, so, if the problem solves by defining such macro, the cause is, most likely, a variable not initialized.


MemoryPool size
-----------------

Whenever crashes appear more or less randomly with alternating exceptions, the cause will be, most likely, a stack overflow. Try to reduce the memory pool size to leave a bit more room for the stack. Since the safe minimum for the stack is about 2KB, your memory pool configuration should not exceed 62KB (depending on how deep the stack can grow because of nested function calls, this limit could be lower; this is specially the case when compiling under debug mode).


Cast everything
---------------

Because the engine implements class inheritance by accumulation of attributes' definitions within macros, it is necessary to cast every pointer of any given class to its base class in order to avoid compiler warnings when calling the base class' methods. This exposes the program to hard to identify errors. In order to mitigate this danger, cast every pointer before passing it to the base class' method by following this pattern:

	BaseClass::method(__SAFE_CAST(BaseClass, object), ...);

When compiling for release, the macro is replaced by a simple C type cast; while for debug, the Object_getCast method will be called, returning NULL if the object does not inherit from the given BaseClass, raising an exception in the method (which must check that the "this" pointer isn't NULL).


Exceptions
----------

When an exception is thrown in-game in debug mode, you're presented with some output that's meant to help you find the exact location that is causing the crash. These are last process, LP and SP as well as the exception message.

Looking for the message in both your game code as well as the engine would be the quickest thing to do but should give you only a rough idea of the problem's root in most cases.

The LP (linker pointer) value shows you the exact location in program where the crash occurred and will lead you to the function that has caused it. Set DUMP_ELF to 1 in the project's config.make file and recompile. The compiler will produce a file called "machine-{TYPE}.asm" in the project's build/{TYPE} folder. It contains a huge list of all functions, their ASM equivalent and memory locations. Search it for your LP value and it will lead you to the faulty function.

The SP (stack pointer) value becomes useful in the (seldom) case of a stack overflow. Since the check is performed during a timer interrupt, it is possible that an overflow occurs between interrupts. By checking the SP value against the __lastDataVariable address in the sections.txt file, you can guess that there was an overflow. As described for the machine.asm file above, activate generation of the sections.txt file in the makefile.


Debug Tools
-----------

### Debug System

[TODO]

### Stage Editor

[TODO]

### Animation Editor

[TODO]


Other useful macros
-------------------

### __GET_CLASS_NAME()

Get the class of an object using this macro.
