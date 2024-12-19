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
#include <Component.h>
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

	/// Create the components for the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param componentSpecsDirectory: Array for the the component specs from
	static void createComponents(SpatialObject owner, ComponentSpec** componentSpecsDirectory[]);

	/// Destroy the components for the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param components: List of list of components
	/// @param componentType: Type of components to destroy
	static void destroyComponents(SpatialObject owner, VirtualList components[], uint32 componentType);

	/// Add a component to the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param components: List of list of components
	/// @param componentSpec: Spec to initialize the new component
	/// @return Added component
	static Component addComponent(SpatialObject owner, VirtualList components[], ComponentSpec* componentSpec);

	/// Remove a component from the specified owner.
	/// @param components: List of list of components
	/// @param component: Component to remove
	static void removeComponent(VirtualList components[], Component component);

	/// Add a components to the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param components: List of list of components
	/// @param componentSpecs: Specs to initialize the new components
	/// @param destroyOldComponents: If true, any previous component of the same type is destroyed
	static void addComponents(SpatialObject owner, VirtualList components[], ComponentSpec** componentSpecs, bool destroyOldComponents);

	/// Retrieve a list with the components of the provided type belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @param components: List of list of components
	/// @param componentType: Type of components to add
	/// @return Linked list of components of the type provided that attach to the provided owner
	static VirtualList getComponents(SpatialObject owner, VirtualList components[], uint32 componentType);

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

	/// Retrieve a list with the components of the provided type belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @param components: List of list of components
	/// @return Linked list of components of the type provided that attach to the provided owner
	VirtualList doGetComponents(SpatialObject owner, VirtualList components);

	/// Create a component with the provided spec.
	/// @param owner: Object to which the sprite will attach to
	/// @param componentSpec: Spec to use to initialize the component
	/// @return Created component
	virtual Component createComponent(SpatialObject owner, const ComponentSpec* componentSpec) = 0;

	/// Destroy the provided component.
	/// @param component: Sprite to destroy
	virtual void destroyComponent(Component component) = 0;

	/// Check if at least of the components that attach to the provided owner is visible.
	/// @param owner: Object to which the components attach to
	/// @return True if at least of the components that attach to the provided owner is visible
	virtual bool isAnyVisible(SpatialObject owner);
}


#endif
