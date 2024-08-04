
/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SPHERE_H_
#define SPHERE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct SphereSpec
{
	// Wireframe
	WireframeSpec wireframeSpec;

	// radius
	fixed_t radius;

	// draw center
	bool drawCenter;

} SphereSpec;

typedef const SphereSpec SphereROMSpec;


/// @ingroup graphics-3d
class Sphere : Wireframe
{
	PixelVector center;
	fixed_t radius;
	uint16 scaledRadius;
	bool drawCenter;

	/// @publicsection
	void constructor(SpatialObject owner, const SphereSpec* sphereSpec);
	PixelVector getCenter();
	fixed_t getRadius();
	void setCenter(PixelVector center);
	void setRadius(fixed_t radius);
	void setRadiusScale(fixed_t radiusScale);

	override bool render();
	override bool draw();
}


#endif
