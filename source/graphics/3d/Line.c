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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Line.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VIPManager.h>
#include <WireframeManager.h>
#include <Math.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Line::constructor(Vector3D a, Vector3D b, Vector3D normal, u8 color)
{
	// construct base object
	Base::constructor(color);

	this->a = a;
	this->b = b;
	this->normal = normal;
}

/**
 * Class destructor
 */
void Line::destructor()
{
	Wireframe::hide(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
void Line::draw(bool calculateParallax __attribute__((unused)))
{
	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(this->a), 0),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(this->b), 0),
		this->color
	);

	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::intermediate(this->a, this->b)), 0),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::sum(Vector3D::intermediate(this->a, this->b), this->normal)), 0),
		__COLOR_BRIGHT_RED
	);
}
