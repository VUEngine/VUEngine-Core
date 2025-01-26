/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COLLIDER_MANAGER_H_
#define COLLIDER_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Collider.h>
#include <ComponentManager.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __COLLISION_ALL_LAYERS		0x7FFFFFFF

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Entity;
class Clock;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ColliderManager
///
/// Inherits from ComponentManager
///
/// Manages the colliders in the game states.
class ColliderManager : ComponentManager
{
	/// @protectedsection

	/// If false, colliders out of camera's range are culled of from collision testing
	bool checkCollidersOutOfCameraRange;

	/// If true, the list of registered colliders was modified in the mist of processing collisions
	bool dirty;

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

	/// Create a collider with the provided spec.
	/// @param owner: Object to which the collider will attach to
	/// @param colliderSpec: Spec to use to create the collider
	/// @return Created collider
	override Collider instantiateComponent(Entity owner, const ColliderSpec* colliderSpec);

	/// Destroy the provided collider.
	/// @param owner: Object to which the sprite will attach to
	/// @param collider: Collider to destroy
	override void deinstantiateComponent(Entity owner, Collider collider);

	/// Update colliders and test collisions.
	uint32 update();

	/// Set if the colliders out of camera's range are culled of from collision testing.
	/// @param value: If false, colliders out of camera's range are culled of from collision testing
	void setCheckCollidersOutOfCameraRange(bool value);

	/// Make the colliders visible by the usage of wireframes.
	void showColliders();

	/// Make the colliders invisible by hiding their wireframes.
	void hideColliders();

	/// Print the manager's statistics.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);
}

#endif
