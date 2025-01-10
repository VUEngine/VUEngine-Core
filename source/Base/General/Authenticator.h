/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef AUTHENTICATOR_H_
#define MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Authenticator
///
/// Inherits from Object
///
/// Provides an interface for managers.
class Authenticator : Object
{
	/// @protectedsection

	/// Array of authorized callers
	VirtualList authorizedRequesters;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Initialize the authorized requesters array. Only has effect the first time it is called.
	/// @param requesterClasses: Array of class pointers, must be NULL terminated
	void initialize(ClassPointer* requesterClasses);

	/// Check if the requester's class is authorized to call this manager.
	/// @param requestClass: Class of the requester
	/// @return true if authorized; false otherwise
	bool authorize(ClassPointer requesterClass);
}

#endif
