/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SPHERE_H_
#define SPHERE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Wireframe.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A Sphere spec
/// @memberof Sphere
typedef struct SphereSpec
{
	/// Wireframe spec
	WireframeSpec wireframeSpec;

	/// Radius
	fixed_t radius;

	/// Flag to control the drawing of the sphere's center point
	bool drawCenter;

} SphereSpec;

/// A Sphere spec that is stored in ROM
/// @memberof Sphere
typedef const SphereSpec SphereROMSpec;


/// @ingroup graphics-3d
class Sphere : Wireframe
{
	/// @protectedsection

	/// Position at which to draw the wireframe
	PixelVector position;

	/// Radius of the sphere
	fixed_t radius;

	/// Used radius of the sphere's to draw it
	uint16 scaledRadius;

	/// Flag to control the drawing of the sphere's center point
	bool drawCenter;

	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the wireframe attaches to
	/// @param asteriskSpec: Specification that determines how to configure the wireframe
	void constructor(SpatialObject owner, const SphereSpec* sphereSpec);

	/// Set the sphere's radius.
	/// @param radius: Sphere's radius
	void setRadius(fixed_t radius);

	/// Retrieve the sphere's radius.
	/// @return Sphere's radius
	fixed_t getRadius();
	
	/// Prepare the wireframe for drawing.
	/// @param relativePosition: Position relative to the camera's
	override void render(Vector3D relativePosition);

	/// Draw the wireframe to the frame buffers.
	/// @return True if at least one pixel is drawn; false otherwise
	override bool draw();
}


#endif
