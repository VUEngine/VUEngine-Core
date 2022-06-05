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


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Line::constructor(Vector3D a, Vector3D b, Vector3D normal, uint8 color)
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
	DirectDraw::drawColorLine(
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(this->a), 0),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(this->b), 0),
		this->color,
		0
	);

	DirectDraw::drawColorLine(
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::intermediate(this->a, this->b)), 0),
		PixelVector::getFromVector3D(Vector3D::getRelativeToCamera(Vector3D::sum(Vector3D::intermediate(this->a, this->b), this->normal)), 0),
		__COLOR_BRIGHT_RED,
		0
	);
}
