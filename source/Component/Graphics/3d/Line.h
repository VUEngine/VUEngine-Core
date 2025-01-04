/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef LINE_H_
#define LINE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Wireframe.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A Line spec
/// @memberof Line
typedef struct LineSpec
{
	// Wireframe spec
	WireframeSpec wireframeSpec;

	/// Starting point
	Vector3D a;

	/// End point
	Vector3D b;

} LineSpec;

/// A Line spec that is stored in ROM
/// @memberof Line
typedef const LineSpec LineROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Line
///
/// Inherits from Wireframe
///
/// Draws a line.
class Line : Wireframe
{
	/// @protectedsection

	/// Starting point
	PixelVector a;

	/// End point
	PixelVector b;

	/// @publicsection

	/// Class' constructor
	/// @param owner: GameObject to which the wireframe attaches to
	/// @param lineSpec: Specification that determines how to configure the wireframe
	void constructor(GameObject owner, const LineSpec* lineSpec);

	/// Prepare the wireframe for drawing.
	/// @param relativePosition: Position relative to the camera's
	override void render(Vector3D relativePosition);

	/// Draw the wireframe to the frame buffers.
	/// @return True if at least one pixel is drawn; false otherwise
	override bool draw();
}

#endif
