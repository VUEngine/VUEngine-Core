/**
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

#include <Asterisk.h>
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
void Asterisk::constructor(AsteriskSpec* asteriskSpec)
{
	// construct base object
	Base::constructor(&asteriskSpec->wireframeSpec);

	this->center = PixelVector::zero();
	this->length = __ABS(asteriskSpec->length);
	this->scaledLength = this->length;
	this->renderCycle = false;
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
void Asterisk::render()
{
	NM_ASSERT(NULL != this->position, "Asterisk::render: NULL position");

	extern Vector3D _previousCameraPosition;
	extern Rotation _previousCameraInvertedRotation;

	Vector3D relativePosition = Vector3D::sub(*this->position, _previousCameraPosition);
	Asterisk::setupRenderingMode(this, &relativePosition);

	if(__COLOR_BLACK == this->color)
	{
		return;
	}

	relativePosition = Vector3D::rotate(relativePosition, _previousCameraInvertedRotation);
	this->center = Vector3D::projectToPixelVector(relativePosition, Optics::calculateParallax(relativePosition.z));
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
void Asterisk::draw()
{
	NM_ASSERT(NULL != this->position, "Asterisk::draw: NULL position");

	if(this->renderCycle)
	{
		DirectDraw::drawColorX(this->center, this->scaledLength, this->color, this->bufferIndex, this->interlaced);
	}
	else		
	{
		DirectDraw::drawColorCross(this->center, this->scaledLength, this->color, this->bufferIndex, this->interlaced);
	}

	this->renderCycle = !this->renderCycle;
}
