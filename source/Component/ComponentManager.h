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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <stdarg.h>
#include <Component.h>
#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Entity;
class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ComponentManager
///
/// Inherits from Object
///
/// Manages component instances.
abstract class ComponentManager : Object
{
	/// @protectedsection

	/// List of components
	VirtualList components;

	/// @publicsection

	/// Create a component with the specified owner.
	/// @param owner: Owner of the component (can be NULL)
	/// @param componentSpec: Spec to initialize the new component
	/// @return Created component
	static Component createComponent(Entity owner, const ComponentSpec* componentSpec);

	/// Destroy a component from the specified owner.
	/// @param owner: Owner of the component (can be NULL), must match the component's owner
	/// @param component: Component to destroy
	static void destroyComponent(Entity owner, Component component);

	/// Add a component to the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param componentSpec: Spec to initialize the new component
	/// @return Added component
	static Component addComponent(Entity owner, const ComponentSpec* componentSpec);

	/// Remove a component from the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param component: Component to remove
	static void removeComponent(Entity owner, Component component);

	/// Add components to the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param componentSpecs: Specs to initialize the new components
	/// @param componentType: Type of components to add
	static void addComponents(Entity owner, ComponentSpec** componentSpecs, uint32 componentType);

	/// Remove components from the specified owner.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to remove
	static void removeComponents(Entity owner, uint32 componentType);

	/// Retrieve a component of the given type at the desired position.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to add
	/// @param componentIndex: Component's index according to their order of creation
	/// @return Component at the provided index position
	static Component getComponentAtIndex(Entity owner, uint32 componentType, int16 componentIndex);

	/// Retrieve a list with the components of the provided type belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to add
	/// @return Linked list of components of the type provided that attach to the provided owner
	static VirtualList getComponents(Entity owner, uint32 componentType);

	/// Retrieve the linked list of components that are instances of the provided class.
	/// @param owner: Object to which the components attach to
	/// @param classPointer: Pointer to the class to use as search criteria. Usage: typeofclass(ClassName)
	/// @param components: Linked list to be filled with the behaviors that meed the search criteria 
	/// (it is externally allocated and must be externally deleted)
	/// @param componentType: Type of components to retrieve
	/// @return True if one or more behaviors met the search criteria; false otherwise
	static bool getComponentsOfClass(Entity owner, ClassPointer classPointer, VirtualList components, uint32 componentType);

	/// Retrieve the number of components belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to count
	/// @return Number of components belonging to the provided owner
	static uint16 getComponentsCount(Entity owner, uint32 componentType);

	/// Propagate a command to the components of the provided type.
	/// @param command: Command to propagate to all the components of the provided tyep
	/// @param owner: Owner of the components to command (all if NULL)
	/// @param componentType: Type of components towards which to propagate the command
	/// @param ...: Variable arguments list depending on the command
	static void propagateCommand(int32 command, Entity owner, uint32 componentType, ...);

	/// Retrieve the number of components of the a type belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @param componentType: Type of components to count
	/// @return Number of components belonging to the provided owner
	static uint16 getCount(Entity owner, uint32 componentType);

	/// Compute the rightbox for the owner in base of its visual components.
	/// @param owner: Entity that the components attaches to
	/// @param rightBox: Rightbox to configure
	/// @return True if the owner has visual components; false otherwise
	static bool calculateRightBox(Entity owner, RightBox* rightBox);

	/// Check if at least of the visual components that attach to the provided owner is visible.
	/// @param owner: Object to which the visual components attach to
	/// @return True if at least of the visual components that attach to the provided owner is visible
	static bool isAnyCompomentVisible(Entity owner);

	/// Class' constructor
	void constructor();

	/// Create a component with the provided spec.
	/// @param owner: Object to which the component will attach to
	/// @param componentSpec: Spec to use to initialize the component
	/// @return Created component
	virtual Component instantiateComponent(Entity owner, const ComponentSpec* componentSpec);

	/// Destroy the provided component.
	/// @param owner: Object to which the component will attach to
	/// @param component: Comoponent to destroy
	virtual void deinstantiateComponent(Entity owner, Component component) ;

	/// Retrieve a list with the components of the provided type belonging to the provided owner.
	/// @param owner: Object to which the components attach to
	/// @return Linked list of components of the type provided that attach to the provided owner
	VirtualList doGetComponents(Entity owner, VirtualList components);

	/// Check if at least of the components that attach to the provided owner is visible.
	/// @param owner: Object to which the components attach to
	/// @return True if at least of the components that attach to the provided owner is visible
	virtual bool isAnyVisible(Entity owner);
}

#endif
