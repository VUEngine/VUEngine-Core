/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BOX_H_
#define BOX_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Collider.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __BOX_VERTEXES	8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class Box : Collider
{
	// the normals of the box
	VertexProjection vertexProjections[__COLLIDER_NORMALS];
	// the rectangle
	RightBox rightBox;
	// for collision detection purposes
	Normals* normals;
	// for rotation purposes
	Vector3D rotationVertexDisplacement;

	/// @publicsection
	static void project(Vector3D vertexes[__BOX_VERTEXES], Vector3D vector, fixed_t* min, fixed_t* max);

	void constructor(SpatialObject owner, const ColliderSpec* colliderSpec);
	void getVertexes(Vector3D vertexes[__BOX_VERTEXES]);
	void computeNormals(Vector3D vertexes[__BOX_VERTEXES]);
	void projectOntoItself();
	override void resize(fixed_t sizeIncrement);
	override void configureWireframe();
	override void print(int32 x, int32 y);
}


#endif
