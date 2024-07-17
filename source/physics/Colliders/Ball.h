/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BALL_H_
#define BALL_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Collider.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class Ball : Collider
{
	// the radius of the ball
	fixed_t radius;

	/// @publicsection
	static void project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max);
	
	void constructor(SpatialObject owner, const ColliderSpec* colliderSpec);
	override void testForCollision(Collider collider, fixed_t sizeIncrement, CollisionInformation* collisionInformation);
	override void configureWireframe();
	override void print(int32 x, int32 y);
}


#endif
