
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

	uint16 radius;

} SphereSpec;

/// @ingroup graphics-3d
class Sphere : Wireframe
{
	// Vertices
	PixelVector center;
	// Radious
	fix10_6 radius;
	uint16 scaledRadius;

	/// @publicsection
	void constructor(SphereSpec* sphereSpec);
	PixelVector getCenter();
	fix10_6 getRadius();
	void setCenter(PixelVector center);
	void setRadius(fix10_6 radius);
	override void render();
	override void draw();
}


#endif
