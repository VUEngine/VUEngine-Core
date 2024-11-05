/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PHYSICAL_WORLD_H_
#define PHYSICAL_WORLD_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>
#include <Body.h>
#include <SpatialObject.h>
#include <Clock.h>


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __PHYSICS_TIME_ELAPSED_STEP			__FIX7_9_EXT_DIV(__1I_FIX7_9_EXT, __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(__TARGET_FPS), __I_TO_FIX7_9_EXT(__PHYSICS_TIME_ELAPSED_DIVISOR)))


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class PhysicalWorld
///
/// Inherits from Object
///
/// Manages the physical bodies in the game states.
/// @ingroup physics
class PhysicalWorld : Object
{
	/// @protectedsection

	/// Linked list of registered bodies
	VirtualList	bodies;

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

	/// If true, a body is pending destruction
	bool dirty;

	/// @publicsection

	/// Retrieve the time that passes between each physical simulation step.
	/// @return The time that passes between each physical simulation step
	static fixed_t getElapsedTimeStep();

	/// Class' constructor
	void constructor();

	/// Reset the manager's state.
	void reset();

	/// Update the registered bodies by advancing the physics simulations.
	void update();

	/// Create a body with the provided spec.
	/// @param owner: Object to which the body will attach to
	/// @param physicalProperties: Spec to use to create the body
	/// @param axisSubjectToGravity: Flag of axis on which the body is subject to gravity
	/// @return Created body
	Body createBody(SpatialObject owner, const PhysicalProperties* physicalProperties, uint16 axisSubjectToGravity);

	/// Destroy the provided body.
	/// @param collider: Body to destroy
	void destroyBody(Body body);

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
