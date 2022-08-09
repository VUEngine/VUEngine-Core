
/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CROSS_H_
#define CROSS_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct CrossSpec
{
	// Wireframe
	WireframeSpec wireframeSpec;

	// length
	fixed_t length;

} CrossSpec;

typedef const CrossSpec CrossROMSpec;


/// @ingroup graphics-3d
class Cross : Wireframe
{
	// Vertices
	PixelVector center;
	// Radious
	fixed_t length;
	uint16 scaledLength;

	/// @publicsection
	void constructor(CrossSpec* sphereSpec);
	override void render();
	override void draw();
}


#endif
