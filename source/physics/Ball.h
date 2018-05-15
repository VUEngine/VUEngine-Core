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

class Ball : Shape
{
	/**
	* @var fix10_6*	radius
	* @brief			the radius of the ball
	* @memberof 		Box
	*/
	fix10_6 radius;
	/**
	* @var Vector3D		center
	* @brief			the center of the ball
	* @memberof 		Box
	*/
	Vector3D center;

	static void project(Vector3D center, fix10_6 radius, Vector3D vector, fix10_6* min, fix10_6* max);
	void constructor(Ball this, SpatialObject owner);
	override void position(Ball this, const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
	override CollisionInformation testForCollision(Ball this, Shape shape, Vector3D displacement, fix10_6 sizeIncrement);
	override Vector3D getPosition(Ball this);
	override RightBox getSurroundingRightBox(Ball this);
	override void configureWireframe(Ball this);
	override void print(Ball this, int x, int y);
}


#endif
