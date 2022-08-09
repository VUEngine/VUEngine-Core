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

#include <Cross.h>
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
void Cross::constructor(CrossSpec* sphereSpec)
{
	// construct base object
	Base::constructor(&sphereSpec->wireframeSpec);

	this->center = PixelVector::zero();
	this->length = __ABS(sphereSpec->length);
	this->scaledLength = this->length;
}

/**
 * Class destructor
 */
void Cross::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Render
 */
void Cross::render()
{
	if(NULL == this->position)
	{
		return;
	}

	extern Vector3D _previousCameraPosition;
	extern Rotation _previousCameraInvertedRotation;

	Vector3D relativePosition = Vector3D::sub(*this->position, _previousCameraPosition);
	Cross::setupRenderingMode(this, &relativePosition);

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
void Cross::draw()
{
	if(NULL == this->position)
	{
		return;
	}

	if(__COLOR_BLACK == this->color)
	{
		return;
	}

	PixelVector fromPixelVector = this->center;
	PixelVector toPixelVector = this->center;

	static bool flag = false;

	flag = !flag;

	if(flag)
	{
		fromPixelVector.x -= this->scaledLength;
		fromPixelVector.y -= this->scaledLength;
		toPixelVector.x += this->scaledLength;
		toPixelVector.y += this->scaledLength;

		DirectDraw::drawColorLine(fromPixelVector, toPixelVector, this->color, this->bufferIndex, this->interlaced);

		fromPixelVector = this->center;
		toPixelVector = this->center;
		
		fromPixelVector.x += this->scaledLength;
		fromPixelVector.y -= this->scaledLength;
		toPixelVector.x -= this->scaledLength;
		toPixelVector.y += this->scaledLength;

		DirectDraw::drawColorLine(fromPixelVector, toPixelVector, this->color, this->bufferIndex, this->interlaced);
	}
	else		
	{
		fromPixelVector = this->center;
		toPixelVector = this->center;
		
		fromPixelVector.x -= this->scaledLength;
		toPixelVector.x += this->scaledLength;

		DirectDraw::drawColorLine(fromPixelVector, toPixelVector, this->color, this->bufferIndex, this->interlaced);

		fromPixelVector = this->center;
		toPixelVector = this->center;
		
		fromPixelVector.y -= this->scaledLength;
		toPixelVector.y += this->scaledLength;

		DirectDraw::drawColorLine(fromPixelVector, toPixelVector, this->color, this->bufferIndex, this->interlaced);
		this->bufferIndex = !this->bufferIndex;
	}
}
