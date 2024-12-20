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


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class CollisionTester
///
/// Inherits from Object
///
/// Implements collisions tests between two colliders.
static class CollisionTester : Object
{
	/// @publicsection

	/// Check if the provided colliders are overlaping each other.
	/// @param requesterCollider: Collider asking for the test
	/// @param otherCollider: Collider against which to test the overlaping
	/// @param collisionInformation: Struct holding the information with the results of the test
	/// @param sizeDelta: Delta to add to the collider's size
	static void testOverlaping(Collider requesterCollider, Collider otherCollider, CollisionInformation* collisionInformation, fixed_t sizeDelta);
}


#endif
