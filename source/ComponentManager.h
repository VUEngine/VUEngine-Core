/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COMPONENT_MANAGER_H_
#define COMPONENT_MANAGER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <stdarg.h>
#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class SpatialObject;
class VirtualList;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class ComponentManager
///
/// Inherits from Object
///
/// Manages component instances.
singleton class ComponentManager : Object
{
	/// @protectedsection

	/// List of components
	VirtualList components;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Propagate a command to the sprites.
	/// @param command: Command to propagate to all the sprites
	/// @param owner: Owner of the sprites to command (all if NULL)
	/// @param ...: Variable arguments list depending on the command
	void propagateCommand(int32 command, SpatialObject owner, ...);

	/// Retrieve the number of components belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @return Number of components belonging to the provided owner
	uint16 getCount(SpatialObject owner);

	/// Check if at least of the components that attach to the provided owner is visible.
	/// @param owner: Object to which the components attach to
	/// @return True if at least of the components that attach to the provided owner is visible
	virtual bool isAnyVisible(SpatialObject owner);
}


#endif
