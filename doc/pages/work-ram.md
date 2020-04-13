Work RAM
========

To be able to efficiently allocate memory to dynamically created objects, VUEngine makes use of a MemoryPool construct rather than working with WRAM directly.

Basically, what the engine does is to divide the available WRAM into a number of differently sized pools to fill in objects based on their amount of required memory.

To optimally suit their game's needs, users can freely configure memory pools in a number of ways:

1. The total number of pools: there can be as many as needed
2. The block size of each pool
3. The number of objects that each pool can hold


Definition example
------------------

The following example shows a very basic definition of available memory pools and their sizes. These macros go in the config.h file of your game.

	#define 	__MEMORY_POOLS		3

	#define 	__MEMORY_POOL_ARRAYS						\
			__BLOCK_DEFINITION(256, 32)					\
			__BLOCK_DEFINITION(128, 64)					\
			__BLOCK_DEFINITION(64, 64)					\

	#define 	__SET_MEMORY_POOL_ARRAYS					\
			__SET_MEMORY_POOL_ARRAY(256)					\
			__SET_MEMORY_POOL_ARRAY(128)					\
			__SET_MEMORY_POOL_ARRAY(64)					\

This configuration will create 3 pools; one that holds up to 64 objects with sizes of 0 up to 64 bytes, another that holds up to 64 objects of sizes between 68 and 128 bytes, and finally, one that holds up to 32 objects of sizes between 132 and 256 bytes.

The size of each pool is the product of both numbers passed as arguments to the __BLOCK_DEFINITION macro. In this example that amounts to:

	256 * 32 	= 	8,192 bytes
	128 * 64 	= 	8,192 bytes
	64 * 64 	= 	4,096 bytes
	**TOTAL		= 	20,480 bytes**

Please note that the __BLOCK_DEFINITONs have to be sorted by size in descending order. Furthermore, the sizes always have to be multiples of 4.


Maximum size
------------

Under the best possible scenario, that is, when the class virtual tables are allocated in DRAM, the required memory by VUEngine to show an empty stage without any entities or preloaded textures is about 4 KB of the Virtual Boy's 64 KB of WRAM.

A minimum of 2 KB are needed for the program’s stack. Therefore, the absolute maximum amount of WRAM that can be reserved for the MemoryPool before overflowing the stack is about **62 KB**. However, the more the game's stages are populated, the more memory will be required by the stack and lower the MemoryPool's maximum size.


Practical application
---------------------

A generically configured MemoryPool as shown in the previous example should be sufficient for small projects with few objects. However, it is strongly  recommended to configure the MemoryPool to suit your game's needs as accurate as possible to not waste memory.

Imagine that your game's hero requires 200 bytes of memory. In the previous example configuration, it would be put in the 256 byte pool, effectively wasting 56 bytes of memory. So, what you should do is define an additional pool of the exact size of your hero, which can hold a single object, since there will never be more than one hero instanciated. The definition would look like the following: __BLOCK_DEFINITION(200, 1). Following the same pattern, you should create pools for all kinds of objects that appear in your game.

During the development of your game, you will want to constantly refine your MemoryPool's configuration to account for changes in the memory requirements of your game's objects.


Determining object sizes
------------------------

The most convenient way of finding out the exact memory footprints of your game's objects is to use the engine's debug tools. Slate 2 of the Debug System (accessed with LT+RT+RU) shows detailed information about the individual pools' usage, the MemoryPool's total size and usage as well as the memory requirements (in Bytes) of all of VUEngine's as well as user defined classes.

For a programmatic approach, you can use `sizeof(ObjectClass)` if you know the object's class.
