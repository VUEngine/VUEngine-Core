/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ASTERISK_H_
#define ASTERISK_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Wireframe.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A Asterisk spec
/// @memberof Asterisk
typedef struct AsteriskSpec
{
	// Wireframe spec
	WireframeSpec wireframeSpec;

	// Length of the segments
	fixed_t length;

} AsteriskSpec;

/// A Asterisk spec that is stored in ROM
/// @memberof Asterisk
typedef const AsteriskSpec AsteriskROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Asterisk
///
/// Inherits from Wireframe
///
/// Draws an asterisk.
class Asterisk : Wireframe
{
	/// @protectedsection

	/// Position at which to draw the wireframe
	PixelVector position;

	/// Length of the asterisk's lines
	fixed_t length;

	/// Used length of the asterisk's lines to draw them
	uint16 scaledLength;

	/// @publicsection

	/// Class' constructor
	/// @param owner: Entity to which the wireframe attaches to
	/// @param asteriskSpec: Specification that determines how to configure the wireframe
	void constructor(Entity owner, const AsteriskSpec* asteriskSpec);

	/// Prepare the wireframe for drawing.
	/// @param relativePosition: Position relative to the camera's
	override void render(Vector3D relativePosition);

	/// Draw the wireframe to the frame buffers.
	/// @return True if at least one pixel is drawn; false otherwise
	override bool draw();
}

#endif
