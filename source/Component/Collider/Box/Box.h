/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BOX_H_
#define BOX_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Collider.h>
#include <Mesh.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __BOX_VERTEXES	8

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Box
///
/// Inherits from Collider
///
/// Defines a collider with the shape of a box.
class Box : Collider
{
	/// @protectedsection

	/// The normals of the box
	VertexProjection vertexProjections[__COLLIDER_NORMALS];

	/// Mesh used to draw the collider
	MeshSpec* meshSpec;

	/// Bounding box
	RightBox rightBox;
	// for collision detection purposes

	/// The normals of the box
	Normals* normals;

	// For rotation purposes
	Vector3D rotationVertexDisplacement;

	/// @publicsection

	/// Project the vertexes onto the provided vector.
	/// @param vertexes: Array of vectors to project
	/// @param vector: Vector onto which make the projections
	/// @param min min: Variable to store the value of the lowest value of the projection
	/// @param max max: Variable to store the value of the biggest value of the projection
	static void project(Vector3D vertexes[__BOX_VERTEXES], Vector3D vector, fixed_t* min, fixed_t* max);

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

	/// Retrieve the vertexes that define the box.
	/// @param vertexes: Array of vectors that define the box
	void getVertexes(Vector3D vertexes[__BOX_VERTEXES]);

	/// Project the box's vertexes onto its normals.
	void projectOntoItself();
}

#endif
