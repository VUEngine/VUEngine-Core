/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COLLISION_TESTER_H_
#define COLLISION_HELPER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Collider.h>
#include <ListenerObject.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class Collider;
struct CollisionInformation;



//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class CollisionTester
///
/// Inherits from Object
///
/// Implements collisions tests between two colliders.
/// @ingroup physics
static class CollisionTester : Object
{
	/// @publicsection
	static void testOverlaping(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation, fixed_t sizeDelta);
}


#endif
