/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DirectDraw.h>
#include <Math.h>
#include <Optics.h>
#include <WireframeManager.h>

#include "Asterisk.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Asterisk::constructor(SpatialObject owner, const AsteriskSpec* asteriskSpec)
{
	// construct base object
	Base::constructor(owner, &asteriskSpec->wireframeSpec);

	this->length = __ABS(asteriskSpec->length);
	this->scaledLength = this->length;
	this->renderCycle = false;
	this->center = PixelVector::zero();
}

/**
 * Class destructor
 */
void Asterisk::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Render
 */
void Asterisk::render(Vector3D relativePosition)
{
	NM_ASSERT(NULL != this->transformation, "Asterisk::render: NULL transformation");

	relativePosition = Vector3D::rotate(relativePosition, _previousCameraInvertedRotation);
	this->center = PixelVector::projectVector3D(relativePosition, Optics::calculateParallax(relativePosition.z));
	this->scaledLength = __METERS_TO_PIXELS(__FIXED_MULT(this->length, Vector3D::getScale(relativePosition.z, false)));
}

/**
 * Draw
 */

/**
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
bool Asterisk::draw()
{
	NM_ASSERT(NULL != this->transformation, "Asterisk::draw: NULL transformation");

	bool drawn = false;

	if(this->renderCycle)
	{
		drawn = DirectDraw::drawColorX(this->center, this->scaledLength, this->color, this->bufferIndex, this->interlaced);
	}
	else		
	{
		drawn = DirectDraw::drawColorCross(this->center, this->scaledLength, this->color, this->bufferIndex, this->interlaced);
	}

	this->bufferIndex = !this->bufferIndex;

	this->renderCycle = !this->renderCycle;
	
	return drawn;
}
