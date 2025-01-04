/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_H_
#define OBJECT_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Oop.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Object;
typedef Object (*AllocatorPointer)();


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Object
///
/// Inherits from Object
///
/// Serves as the base class for all other classes in the engine.
abstract class Object : Object
{
	/// @protectedsection
	// The unusual order of the attributes in the rest of the classes 
	// aims to optimize data packing as much as possible.

	/// Pointer to the class's virtual table
	void* vTable;

	/// @publicsection

	/// Cast an object at runtime to a give class.
	/// @param object: Object to cast
	/// @param targetClassGetClassMethod: pointer to the target class' identifier method
	/// @param baseClassGetClassMethod: pointer to the object's base class' identifier method
	/// @return Pointer to the object if the cast succeeds, NULL otherwhise.
	static Object getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod);

	/// Class' constructor
	void constructor();

 	/// Retrieve the object's virtual table pointer
	/// @return	Pointer to the object's virtual table pointer
	const void* getVTable();

 	/// Converts the object into an instance of the target class if object's class is in the hierarchy of the target class.
	/// @param targetClass: pointer to the target class' virtual table
	/// @return	True if successful
	bool evolveTo(const void* targetClass);
}


#endif
