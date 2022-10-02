
/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ASTERISK_H_
#define ASTERISK_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct AsteriskSpec
{
	// Wireframe
	WireframeSpec wireframeSpec;

	// length
	fixed_t length;

} AsteriskSpec;

typedef const AsteriskSpec AsteriskROMSpec;


/// @ingroup graphics-3d
class Asterisk : Wireframe
{
	// Vertices
	PixelVector center;
	fixed_t length;
	uint16 scaledLength;
	bool renderCycle;

	/// @publicsection
	void constructor(AsteriskSpec* asteriskSpec);
	override void render();
	override void draw();
}


#endif
