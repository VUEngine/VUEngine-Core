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


//---------------------------------------------------------------------------------------------------------
//											FORWARD DECLARATIONS
//---------------------------------------------------------------------------------------------------------

class Object;
typedef Object (*AllocatorPointer)();


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

///
/// Class Object
///
/// Inherits from Object
///
/// Base class for all other classes in the engine, it inherits from nothing but itself.
/// @ingroup base
abstract class Object : Object
{
	/// Pointer to the class's virtual table
	void* vTable;

	/// @publicsection

	/// Run time type checking
	/// @param object: object to cast
	/// @param targetClassGetClassMethod: pointer to the target class' identifier method
	/// @param baseClassGetClassMethod: pointer to the object's base class' identifier method
	/// @return Pointer to the object if the cast succeeds, NULL otherwhise.
	static Object getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod);

	/// Class' constructor
	void constructor();

	/// Class' destructor
	void destructor();

 	/// Retrieve the object's virtual table pointer
	/// @return	Pointer to the object's virtual table pointer
	const void* getVTable();

 	/// Converts the object into an instance of the target class if object's class is in the hierarchy of the target class.
	/// @param targetClass: pointer to the target class' virtual table
	/// @return	True if successful
	bool evolveTo(const void* targetClass);
}


#endif
