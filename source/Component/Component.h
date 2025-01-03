/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <stdarg.h>

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class GameObject;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Component commands
/// @memberof Component
enum ComponentCommands
{
	cComponentCommandEnable = 0,
	cComponentCommandDisable,
	cComponentCommandReset,
	cComponentCommandLast
};

/// Component types
/// @memberof Component
enum ComponentTypes
{
	kColliderComponent = 0,
	kSpriteComponent,
	kWireframeComponent,
	kBehaviorComponent,
	kPhysicsComponent,

	// Limmiter
	kComponentTypes,
};

/// A Component Spec
/// @memberof Component
typedef struct ComponentSpec
{
	/// Class' allocator
	AllocatorPointer allocator;

	/// Component type
	uint16 componentType;

} ComponentSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Component
///
/// Inherits from ListenerObject
///
/// Serves as the base class for components of entities.
abstract class Component : ListenerObject
{
	/// @protectedsection

	/// Object to which this component attaches to
	GameObject owner;

	/// Pointer to the spec that defines how to initialize the component
	const ComponentSpec* componentSpec;

	/// Pointer to the transformation that the component attaches to
	const Transformation* transformation;

	/// @publicsection

	/// Class' constructor
	/// @param owner: GameObject to which the component attaches to
	/// @param componentSpec: Pointer to the spec that defines how to initialize the component
	void constructor(GameObject owner, const ComponentSpec* componentSpec);

	/// Class' destructor
	void destructor();

	/// Retrieve the spec pointer that defined how to initialized the component
	/// @return Component spec pointer
	ComponentSpec* getSpec();

	/// Retrieve the collider's owner
	GameObject getOwner();

	/// Retrieve the component's type
	/// @return Component's type'
	uint32 getType();

	/// Handle a command.
	/// @param command: Command to handle
	/// @param args: Variable arguments list depending on the command to handle
	virtual void handleCommand(int32 command, va_list args);
}

#endif
