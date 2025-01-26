/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BEHAVIOR_MANAGER_H_
#define BEHAVIOR_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Clock.h>
#include <ComponentManager.h>
#include <Behavior.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class BehaviorManager
///
/// Inherits from ComponentManager
///
/// Manages all the behavior instances.
class BehaviorManager : ComponentManager
{
	/// @protectedsection

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Retrieve the compoment type that the manager manages.
	/// @return Component type
	override uint32 getType();

	/// Enable the manager.
	override void enable();

	/// Disable the manager.
	override void disable();

	/// Create a behavior with the provided spec.
	/// @param owner: Object to which the behavior will attach to
	/// @param behaviorSpec: Spec to use to create the behavior
	/// @return Created behavior
	override Behavior instantiateComponent(Entity owner, const BehaviorSpec* behaviorSpec);

	/// Destroy the provided behavior
	/// Reset the manager's state
	void reset();

	/// Update the registered bodies by advancing the physics simulations.
	void update();
}

#endif
