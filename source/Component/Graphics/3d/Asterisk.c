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

#include <DirectDraw.h>
#include <WireframeManager.h>

#include "Asterisk.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Asterisk::constructor(GameObject owner, const AsteriskSpec* asteriskSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &asteriskSpec->wireframeSpec);

	this->length = __ABS(asteriskSpec->length);
	this->scaledLength = this->length;
	this->position = PixelVector::zero();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Asterisk::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Asterisk::render(Vector3D relativePosition)
{
	NM_ASSERT(NULL != this->transformation, "Asterisk::render: NULL transformation");

	relativePosition = Vector3D::rotate(relativePosition, _previousCameraInvertedRotation);
	this->position = PixelVector::projectVector3D(relativePosition, Optics::calculateParallax(relativePosition.z));
	this->scaledLength = __METERS_TO_PIXELS(__FIXED_MULT(this->length, Vector3D::getScale(relativePosition.z, false)));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Asterisk::draw()
{
	NM_ASSERT(NULL != this->transformation, "Asterisk::draw: NULL transformation");

	bool drawn = false;

	if(this->bufferIndex)
	{
		drawn = DirectDraw::drawX(this->position, this->scaledLength, this->color, this->bufferIndex, this->interlaced);
	}
	else		
	{
		drawn = DirectDraw::drawCross(this->position, this->scaledLength, this->color, this->bufferIndex, this->interlaced);
	}

	this->bufferIndex = !this->bufferIndex;

	return drawn;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

