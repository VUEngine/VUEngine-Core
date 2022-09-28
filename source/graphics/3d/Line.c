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

#include <Line.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VIPManager.h>
#include <WireframeManager.h>
#include <Math.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Line::constructor(LineSpec* lineSpec)
{
	// construct base object
	Base::constructor(&lineSpec->wireframeSpec);

	((LineSpec*)this->wireframeSpec)->a = lineSpec->a;
	((LineSpec*)this->wireframeSpec)->b = lineSpec->b;

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
void Line::render()
{
	extern Vector3D _previousCameraPosition;
	extern Rotation _previousCameraInvertedRotation;

	Vector3D position = Vector3D::intermediate(((LineSpec*)this->wireframeSpec)->a, ((LineSpec*)this->wireframeSpec)->b);

	Vector3D relativePosition = Vector3D::sub(position, _previousCameraPosition);
	Sphere::setupRenderingMode(this, &relativePosition);

	if(__COLOR_BLACK == this->color)
	{
		return;
	}

	relativePosition = Vector3D::sub(((LineSpec*)this->wireframeSpec)->a, _previousCameraPosition);
	relativePosition = Vector3D::rotate(relativePosition, _previousCameraInvertedRotation);
	this->a = Vector3D::projectToPixelVector(relativePosition, Optics::calculateParallax(relativePosition.z));

	relativePosition = Vector3D::sub(((LineSpec*)this->wireframeSpec)->b, _previousCameraPosition);
	relativePosition = Vector3D::rotate(relativePosition, _previousCameraInvertedRotation);
	this->b = Vector3D::projectToPixelVector(relativePosition, Optics::calculateParallax(relativePosition.z));
}

/**
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
void Line::draw()
{
	DirectDraw::drawColorLine(
		this->a,
		this->b,
		this->color,
		0,
		false
	);
/*
	DirectDraw::drawColorLine(
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::intermediate(((LineSpec*)this->wireframeSpec)->a, ((LineSpec*)this->wireframeSpec)->b)), 0),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::sum(Vector3D::intermediate(((LineSpec*)this->wireframeSpec)->a, ((LineSpec*)this->wireframeSpec)->b), this->normal)), 0),
		__COLOR_BRIGHT_RED,
		0,
		false
	);
	*/
}
