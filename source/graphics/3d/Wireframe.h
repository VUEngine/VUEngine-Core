/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef WIREFRAME_H_
#define WIREFRAME_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <VisualComponent.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA							__FIXED_EXT_INFINITY


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A Wireframe spec
/// @memberof Wireframe
typedef struct WireframeSpec
{
	/// Class' allocator
	AllocatorPointer allocator;

	/// Displacement relative to the owner's spatial position
	Vector3D displacement;

	/// Color for the wireframe
	uint8 color;

	/// Transparency mode
	/// (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	uint8 transparency;

	/// Flag to render the wireframe in interlaced mode
	bool interlaced;

} WireframeSpec;

/// A Wireframe spec that is stored in ROM
/// @memberof Wireframe
typedef const WireframeSpec WireframeROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Wireframe
///
/// Inherits from VisualComponent
///
/// Draws 3D shapes to the frame buffers.
abstract class Wireframe : VisualComponent
{
	/// Displacement relative to the owner's spatial position
	Vector3D displacement;

	/// Wireframe's squared distance to the camera's position
	fixed_ext_t squaredDistanceToCamera;

	/// Flag that indicates that the wireframe has been drawn
	bool drawn;

	/// Flag to render the wireframe in interlaced mode
	bool interlaced;

	/// Color for the wireframe
	uint8 color;

	/// Index of the last frame buffer used in interlaced mode 
	uint8 bufferIndex;

	/// @publicsection
	/// Class' constructor
	/// @param owner: SpatialObject to which the wireframe attaches to
	/// @param wireframeSpec: Specification that determines how to configure the wireframe
	void constructor(SpatialObject owner, const WireframeSpec* wireframeSpec);

	/// Set the displacement relative to the owner's spatial position
	/// @param displacement: Displacement relative to the owner's spatial position
	void setDisplacement(Vector3D displacement);

	/// Check if the wireframe is visible.
	/// @return True if the wireframe is visible; false otherwise
	bool isVisible();

	/// Configure the wireframe to be drawn.
	/// @param relativePosition relativePosition: Wireframe's position relative to the camera's position 
	/// @return True if the wireframe is visible within the camera's frustum; false otherwise
	bool prepareForRender(Vector3D* relativePosition);

	/// Retrieve the list of vertices that compose the mesh.
	/// @return Linked list of vertices
	virtual VirtualList getVertices();

	/// Prepare the wireframe for drawing.
	/// @param relativePosition: Position relative to the camera's
	virtual void render(Vector3D relativePosition);

	/// Draw the wireframe to the frame buffers.
	/// @return True if at least one pixel is drawn; false otherwise
	virtual bool draw() = 0;
}


#endif
