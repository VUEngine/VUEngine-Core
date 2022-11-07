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

#include <Shape.h>
#include <Sphere.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class Ball : Shape
{
	// the radius of the ball
	fixed_t radius;
	// the center of the ball
	Vector3D center;

	/// @publicsection
	static void project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max);
	
	void constructor(SpatialObject owner, const ShapeSpec* shapeSpec);
	override void position(const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
	override void setPosition(const Vector3D* position);
	override CollisionInformation testForCollision(Shape shape, Vector3D displacement, fixed_t sizeIncrement);
	override Vector3D getPosition();
	override void configureWireframe();
	override void print(int32 x, int32 y);
}


#endif
