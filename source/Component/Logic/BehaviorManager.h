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
singleton class BehaviorManager : ComponentManager
{
	/// @protectedsection

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return BehaviorManager singleton
	static BehaviorManager getInstance();

	/// Create a behavior with the provided spec.
	/// @param owner: Object to which the behavior will attach to
	/// @param behaviorSpec: Spec to use to create the behavior
	/// @return Created behavior
	override Behavior instantiateComponent(Entity owner, const BehaviorSpec* behaviorSpec);

	/// Destroy the provided behavior.
	/// @param owner: Object to which the sprite will attach to
	/// @param behavior: Behavior to destroy
	override void deinstantiateComponent(Entity owner, Behavior behavior);

	/// Reset the manager's state
	static void reset();
}

#endif
