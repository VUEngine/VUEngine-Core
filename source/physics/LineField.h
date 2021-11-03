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

#include <Shape.h>
#include <Polyhedron.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __LINE_FIELD_VERTEXES		2

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class LineField : Shape
{
	Vector3D normal;
	Vector3D a;
	Vector3D b;
	fix10_6 normalLength;

	/// @publicsection
	static void project(Vector3D center, fix10_6 radius, Vector3D vector, fix10_6* min, fix10_6* max);
	void constructor(SpatialObject owner);
	void getVertexes(Vector3D vertexes[__LINE_FIELD_VERTEXES]);
	void addDisplacement(fix10_6 displacement);
	override void position(const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
	override CollisionInformation testForCollision(Shape shape, Vector3D displacement, fix10_6 sizeIncrement);
	override Vector3D getPosition();
	override Vector3D getNormal();
	override void configureWireframe();
	override void print(int32 x, int32 y);
}


#endif
