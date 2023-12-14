/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COLLISION_HELPER_H_
#define COLLISION_HELPER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Collider.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-3d
static class CollisionHelper : ListenerObject
{
	/// @publicsection
	static void checkIfOverlap(Collider shapeA, Collider shapeB, CollisionInformation* collisionInformation);
}


#endif
