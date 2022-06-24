
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
	Vector3D normal;

} LineSpec;

/// @ingroup graphics-3d
class Line : Wireframe
{
	// Vertices
	Vector3D normal;
	Vector3D a;
	Vector3D b;

	/// @publicsection
	void constructor(LineSpec* lineSpec);
	override void draw(bool calculateParallax);
}


#endif
