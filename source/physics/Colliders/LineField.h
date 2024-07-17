/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef LINE_FIELD_H_
#define LINE_FIELD_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Collider.h>
#include <Line.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __LINE_FIELD_VERTEXES		2

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class LineField : Collider
{
	LineSpec* lineSpec;
	Vector3D normal;
	Vector3D a;
	Vector3D b;
	fixed_t normalLength;

	/// @publicsection
	static void project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max);

	void constructor(SpatialObject owner, const ColliderSpec* colliderSpec);
	void getVertexes(Vector3D vertexes[__LINE_FIELD_VERTEXES]);
	void addDisplacement(fixed_t displacement);
	Vector3D getCenter();

	override Vector3D getNormal();
	override void configureWireframe();
	override void print(int32 x, int32 y);
}


#endif
