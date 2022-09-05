/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_H_
#define OBJECT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Oop.h>
#include <Constants.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Object;
typedef Object (*AllocatorPointer)();



//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// Base class for all other classes in the engine, it derives from nothing but itself
/// @ingroup base
abstract class Object : Object
{
	// Pointer to the class's virtual table.
	void* vTable;

	/// @publicsection
	static Object getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod);
	void constructor();
	bool evolveTo(const void* targetClass);
	const void* getVTable();
}


#endif
