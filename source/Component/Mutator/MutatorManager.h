/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MUTATOR_MANAGER_H_
#define MUTATOR_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Clock.h>
#include <ComponentManager.h>
#include <Mutator.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class MutatorManager
///
/// Inherits from ComponentManager
///
/// Manages all the mutator instances.
class MutatorManager : ComponentManager
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

	/// Create a mutator with the provided spec.
	/// @param owner: Object to which the mutator will attach to
	/// @param mutatorSpec: Spec to use to create the mutator
	/// @return Created mutator
	override Mutator create(Entity owner, const MutatorSpec* mutatorSpec);

	/// Destroy the provided mutator
	/// Reset the manager's state
	void reset();

	/// Update the registered bodies by advancing the physics simulations.
	void update();
}

#endif
