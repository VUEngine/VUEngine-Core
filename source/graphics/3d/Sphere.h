
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

	Vector3D center;
	fix10_6 radius;

} SphereSpec;

/// @ingroup graphics-3d
class Sphere : Wireframe
{
	// Vertices
	Vector3D center;
	Vector3D normalizedCenter3D;
	// Radious
	fix10_6 radius;

	/// @publicsection
	void constructor(SphereSpec* sphereSpec);
	Vector3D getCenter();
	fix10_6 getRadius();
	void setCenter(Vector3D center);
	void setRadius(fix10_6 radius);
	override void draw(bool calculateParallax);
}


#endif
