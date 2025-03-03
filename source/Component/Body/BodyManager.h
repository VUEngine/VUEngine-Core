/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BODY_WORLD_H_
#define BODY_WORLD_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Body.h>
#include <ComponentManager.h>
#include <Entity.h>
#include <Clock.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class BodyManager
///
/// Inherits from ComponentManager
///
/// Manages the physical bodies in the game states.
class BodyManager : ComponentManager
{
	/// @protectedsection

	/// Gravity in the current physical world
	Vector3D gravity;

	/// Friction coefficient in the current physical world
	fixed_t frictionCoefficient;

	/// Time scale for time step on each call to update
	fixed_t timeScale;

	/// Cycle of physical simulation during the current second
	uint8 cycle;

	/// Number of cycles to skip physical simulations to slow down physics
	uint8 skipCycles;

	/// Number of pending cycles to skip until the next physical simulation cycle
	uint8 remainingSkipCycles;

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

	/// Create a body with the provided spec.
	/// @param owner: Object to which the body will attach to
	/// @param bodySpec: Spec to use to create the body
	/// @return Created body
	override Body create(Entity owner, const BodySpec* bodySpec);

	/// Reset the manager's state.
	void reset();

	/// Update the registered bodies by advancing the physics simulations.
	void update();

	/// Set the time scale for time step on each call to update.
	/// @param timeScale: Time scale for time step on each call to update
	void setTimeScale(fixed_t timeScale);

	/// Retrieve the time scale for time step on each call to update.
	/// @return Time scale for time step on each call to update
	uint32 getTimeScale();

	/// Set the physical world's gravity.
	/// @param gravity: Gravity to set in the current physical world
	void setGravity(Vector3D gravity);

	/// Retrieve the physical world's gravity.
	/// @return Gravity in the current physical world
	Vector3D getGravity();

	/// Set the physical world's friction coefficient.
	/// @param frictionCoefficient: Friction coefficient to set in the current physical world
	void setFrictionCoefficient(fixed_t frictionCoefficient);

	/// Retrieve the physical world's friction coefficient.
	/// @return Friction coefficient to set in the current physical world
	fixed_t getFrictionCoefficient();

	/// Print the manager's statistics.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);
}

#endif
