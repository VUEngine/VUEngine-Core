/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef BOX_H_
#define BOX_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Shape.h>
#include <Polyhedron.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __BOX_VERTEXES	8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Box : Shape
{
	/**
	* @var Normals		normals
	* @brief			the normals of the box
	* @memberof 		Box
	*/
	VertexProjection vertexProjections[__SHAPE_NORMALS];
	/**
	* @var Normals*	normals
	* @brief			for collision detection purposes
	* @memberof 		Box
	*/
	Normals* normals;
	/**
	* @var Vector3D	rotationVertexDisplacement
	* @brief			for rotation purposes
	* @memberof 		Box
	*/
	Vector3D rotationVertexDisplacement;
	/**
	* @var RightBox	rightBox
	* @brief			the rectangle
	* @memberof 		Box
	*/
	RightBox rightBox;

	static void project(Vector3D vertexes[__BOX_VERTEXES], Vector3D vector, fix10_6* min, fix10_6* max);
	void constructor(SpatialObject owner);
	void getVertexes(Vector3D vertexes[__BOX_VERTEXES]);
	void computeNormals(Vector3D vertexes[__BOX_VERTEXES]);
	void projectOntoItself();
	override void position(const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
	override CollisionInformation testForCollision(Shape shape, Vector3D displacement, fix10_6 sizeIncrement);
	override Vector3D getPosition();
	override RightBox getSurroundingRightBox();
	override void configureWireframe();
	override void print(int x, int y);
}


#endif
