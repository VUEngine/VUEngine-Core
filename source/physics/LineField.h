/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
	override void print(int x, int y);
}


#endif
