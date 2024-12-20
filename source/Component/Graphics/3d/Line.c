/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <DirectDraw.h>
#include <WireframeManager.h>

#include "Line.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Line::constructor(SpatialObject owner, const LineSpec* lineSpec)
{
	// construct base object
	Base::constructor(owner, &lineSpec->wireframeSpec);

	this->a = PixelVector::zero();
	this->b = PixelVector::zero();
}
//---------------------------------------------------------------------------------------------------------
void Line::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void Line::render(Vector3D relativePosition)
{
	Vector3D a = Vector3D::sum(relativePosition, ((LineSpec*)this->componentSpec)->a);
	a = Vector3D::rotate(a, _previousCameraInvertedRotation);
	this->a = PixelVector::projectVector3D(a, Optics::calculateParallax(a.z));

	Vector3D b = Vector3D::sum(relativePosition, ((LineSpec*)this->componentSpec)->b);
	b = Vector3D::rotate(b, _previousCameraInvertedRotation);
	this->b = PixelVector::projectVector3D(b, Optics::calculateParallax(b.z));
}
//---------------------------------------------------------------------------------------------------------
bool Line::draw()
{
	bool drawn = DirectDraw::drawLine
	(
		this->a,
		this->b,
		this->color,
		this->bufferIndex,
		this->interlaced
	);

	this->bufferIndex = !this->bufferIndex;

	return drawn;	
}
//---------------------------------------------------------------------------------------------------------
