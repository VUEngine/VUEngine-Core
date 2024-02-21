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

#include <DirectDraw.h>
#include <DebugUtilities.h>
#include <Math.h>
#include <Optics.h>
#include <WireframeManager.h>

#include "Line.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Line::constructor(SpatialObject owner, LineSpec* lineSpec)
{
	// construct base object
	Base::constructor(owner, &lineSpec->wireframeSpec);

	this->a = PixelVector::zero();
	this->b = PixelVector::zero();
}

/**
 * Class destructor
 */
void Line::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Render
 */
bool Line::render()
{
	Vector3D position = Vector3D::sum(this->transformation->position, ((LineSpec*)this->componentSpec)->wireframeSpec.displacement);

	Vector3D relativePosition = Vector3D::sub(position, _previousCameraPosition);
	Sphere::setupRenderingMode(this, &relativePosition);

	if(__COLOR_BLACK == this->color)
	{
		return false;
	}

	Vector3D a = Vector3D::sum(relativePosition, ((LineSpec*)this->componentSpec)->a);
	a = Vector3D::rotate(a, _previousCameraInvertedRotation);
	this->a = Vector3D::projectToPixelVector(a, Optics::calculateParallax(a.z));

	Vector3D b = Vector3D::sum(relativePosition, ((LineSpec*)this->componentSpec)->b);
	b = Vector3D::rotate(b, _previousCameraInvertedRotation);
	this->b = Vector3D::projectToPixelVector(b, Optics::calculateParallax(b.z));

	return true;
}

/**
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
bool Line::draw()
{
	return DirectDraw::drawColorLine
	(
		this->a,
		this->b,
		this->color,
		0,
		false
	);
/*
	DirectDraw::drawColorLine
	(
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::intermediate(((LineSpec*)this->componentSpec)->a, ((LineSpec*)this->componentSpec)->b)), 0),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::sum(Vector3D::intermediate(((LineSpec*)this->componentSpec)->a, ((LineSpec*)this->componentSpec)->b), this->normal)), 0),
		__COLOR_BRIGHT_RED,
		0,
		false
	);
	*/
}
