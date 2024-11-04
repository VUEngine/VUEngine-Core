/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COLLISION_MANAGER_H_
#define COLLISION_MANAGER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Collider.h>
#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

#define __COLLISION_ALL_LAYERS		0x7FFFFFFF


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

class SpatialObject;
class Clock;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class CollisionManager
///
/// Inherits from Object
///
/// Manages the colliders in the game.
/// @ingroup physics
class CollisionManager : Object
{
	/// @protectedsection

	/// Linked list of registered colliders
	VirtualList	colliders;

	/// If false, colliders out of camera's range are culled of from collision testing
	bool checkCollidersOutOfCameraRange;

	/// If true, the list of registered colliders was modified in the mist of processing collisions
	bool dirty;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Reset the manager's state
	void reset();

	/// Purge destroyed colliders.
	void purgeDestroyedColliders();

	/// Update colliders and test collisions.
	uint32 update();

	/// Create a collider with the provided spec.
	/// @param spriteSpec: Spec to use to create the collider
	/// @param owner: Object to which the collider will attach to
	/// @return Created collider
	Collider createCollider(const ColliderSpec* colliderSpec, SpatialObject owner);

	/// Destroy the provided collider.
	/// @param collider: Collider to destroy
	void destroyCollider(Collider collider);

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
