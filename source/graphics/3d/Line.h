
/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef LINE_H_
#define LINE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct LineSpec
{
	// Wireframe
	WireframeSpec wireframeSpec;

	Vector3D a;
	Vector3D b;

} LineSpec;

/// @ingroup graphics-3d
class Line : Wireframe
{
	// Vertices
	PixelVector a;
	PixelVector b;

	/// @publicsection
	void constructor(LineSpec* lineSpec);
	override void render();
	override void draw();
}


#endif
