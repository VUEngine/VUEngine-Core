/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <FrameBufferManager.h>
#include <WireframeManager.h>

#include "Line.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Line::constructor(Entity owner, const LineSpec* lineSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &lineSpec->wireframeSpec);

	this->a = PixelVector::zero();
	this->b = PixelVector::zero();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Line::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Line::render(Vector3D relativePosition)
{
	if(NULL == this->componentSpec)
	{
		return;
	}
	
	Vector3D a = Vector3D::sum(relativePosition, ((LineSpec*)this->componentSpec)->a);
	a = Vector3D::rotate(a, *_cameraInvertedRotation);
	this->a = PixelVector::projectVector3D(a, Optics::calculateParallax(a.z));

	Vector3D b = Vector3D::sum(relativePosition, ((LineSpec*)this->componentSpec)->b);
	b = Vector3D::rotate(b, *_cameraInvertedRotation);
	this->b = PixelVector::projectVector3D(b, Optics::calculateParallax(b.z));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Line::draw()
{
	bool drawn = FrameBufferManager::drawLine
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
