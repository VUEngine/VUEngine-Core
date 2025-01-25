/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BALL_H_
#define BALL_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Collider.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Ball
///
/// Inherits from Collider
///
/// Defines a collider with the shape of a ball.
class Ball : Collider
{
	/// @protectedsection

	/// The radius of the ball
	fixed_t radius;

	/// @publicsection

	/// Project the diameter of a circle defined by its centers and radius onto the provided vector.
	/// @param center: Center of the circle whose diameter is projected
	/// @param radius: Radius of the circle whose diameter is projected
	/// @param vector: Vector onto which to project the circle's diameter
	/// @param min min: Variable to store the value of the lowest value of the projection
	/// @param max max: Variable to store the value of the biggest value of the projection
	static void project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max);

	/// Class' constructor
	void constructor(Entity owner, const ColliderSpec* colliderSpec);

	/// Resize the colliders add the provided increment.
	/// @param sizeDelta: Delta to add to the collider's size
	override void resize(fixed_t sizeDelta);

	/// Configure the wireframe used to show the collider.
	override void configureWireframe();

	/// Print collider's state.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	override void print(int32 x, int32 y);
}

#endif
