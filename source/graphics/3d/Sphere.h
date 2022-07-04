
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

} SphereSpec;

typedef const SphereSpec SphereROMSpec;


/// @ingroup graphics-3d
class Sphere : Wireframe
{
	// Vertices
	PixelVector center;
	// Radious
	fixed_t radius;
	uint16 scaledRadius;

	/// @publicsection
	void constructor(SphereSpec* sphereSpec);
	PixelVector getCenter();
	fixed_t getRadius();
	void setCenter(PixelVector center);
	void setRadius(fixed_t radius);
	override void render();
	override void draw();
}


#endif
