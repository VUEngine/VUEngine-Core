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

	/// Add a component to the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param componentSpec: Spec to initialize the new component
	/// @return Added component
	static Component addComponent(SpatialObject owner, ComponentSpec* componentSpec);

	/// Remove a component from the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param component: Component to remove
	static void removeComponent(SpatialObject owner, Component component);

	/// Add components to the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param componentSpecs: Specs to initialize the new components
	/// @param componentType: Type of components to add
	static void addComponents(SpatialObject owner, ComponentSpec** componentSpecs, uint32 componentType);

	/// Remove components from the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to remove
	static void removeComponents(SpatialObject owner, uint32 componentType);

	/// Retrieve a component of the given type at the desired position.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to add
	/// @param componentIndex: Component's index according to their order of creation
	/// @return Component at the provided index position
	static Component getComponentAtIndex(SpatialObject owner, uint32 componentType, int16 componentIndex);

	/// Retrieve a list with the components of the provided type belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to add
	/// @return Linked list of components of the type provided that attach to the provided owner
	static VirtualList getComponents(SpatialObject owner, uint32 componentType);

	/// Retrieve the linked list of components that are instances of the provided class.
	/// @param owner: Object to which the components attach to
	/// @param classPointer: Pointer to the class to use as search criteria. Usage: typeofclass(ClassName)
	/// @param components: Linked list to be filled with the behaviors that meed the search criteria 
	/// (it is externally allocated and must be externally deleted)
	/// @param componentType: Type of components to retrieve
	/// @return True if one or more behaviors met the search criteria; false otherwise
	static bool getComponentsOfClass(SpatialObject owner, ClassPointer classPointer, VirtualList components, uint32 componentType);

	/// Retrieve the number of components belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to count
	/// @return Number of components belonging to the provided owner
	static uint16 getComponentsCount(SpatialObject owner, uint32 componentType);

	/// Class' constructor
	void constructor();

	/// Propagate a command to the components.
	/// @param command: Command to propagate to all the components
	/// @param owner: Owner of the components to command (all if NULL)
	/// @param ...: Variable arguments list depending on the command
	void propagateCommand(int32 command, SpatialObject owner, ...);

	/// Retrieve the number of components belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @return Number of components belonging to the provided owner
	uint16 getCount(SpatialObject owner);

	/// Create a component with the provided spec.
	/// @param owner: Object to which the component will attach to
	/// @param componentSpec: Spec to use to initialize the component
	/// @return Created component
	virtual Component createComponent(SpatialObject owner, const ComponentSpec* componentSpec);

	/// Destroy the provided component.
	/// @param owner: Object to which the component will attach to
	/// @param component: Comoponent to destroy
	virtual void destroyComponent(SpatialObject owner, Component component) ;

	/// Retrieve a list with the components of the provided type belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @return Linked list of components of the type provided that attach to the provided owner
	VirtualList doGetComponents(SpatialObject owner, VirtualList components);

	/// Check if at least of the components that attach to the provided owner is visible.
	/// @param owner: Object to which the components attach to
	/// @return True if at least of the components that attach to the provided owner is visible
	virtual bool isAnyVisible(SpatialObject owner);
}


#endif
