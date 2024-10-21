/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MESH_H_
#define MESH_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Wireframe.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A Mesh spec
/// @memberof Mesh
typedef struct MeshSpec
{
	/// Wireframe spec
	WireframeSpec wireframeSpec;

	/// Segments that compose the mesh
	PixelVector (*segments)[2];

} MeshSpec;

/// A Mesh spec that is stored in ROM
/// @memberof Mesh
typedef const MeshSpec MeshROMSpec;

/// A Vexter struct
/// @memberof Mesh
typedef struct Vertex
{
	/// 3D vector
	Vector3D vector;

	/// Pixel vector
	PixelVector pixelVector;

} Vertex;

/// A segment struct
/// @memberof Mesh
typedef struct MeshSegment
{
	/// Starting vertex
	Vertex* fromVertex;

	/// End vertex
	Vertex* toVertex;

} MeshSegment;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Mesh
///
/// Inherits from Wireframe
///
/// Draws the segments that compose the mesh.
/// @ingroup graphics-3d
class Mesh : Wireframe
{
	/// @protectedsection

	/// List of segments
	VirtualList segments;

	/// List of vertices
	VirtualList vertices;

	/// Retrieve the bounding box in pixel units defined by the provided mesh spec's values.
	/// @return Bounding box of the resulting mesh
	static PixelRightBox getPixelRightBoxFromSpec(MeshSpec* meshSpec);

	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the wireframe attaches to
	/// @param asteriskSpec: Specification that determines how to configure the wireframe
	void constructor(SpatialObject owner, const MeshSpec* meshSpec);
	
	/// Create new segments from the provided array.
	/// @param segments: Array of segments holding their spatial data
	/// @displacement: Displacement to add to the segments' vertices
	void addSegments(PixelVector (*segments)[2], Vector3D displacement);
	
	/// Create a new segment from provided vectors.
	/// @param startVector: New segment's starting point
	/// @param endVector: New segment's end point
	void addSegment(Vector3D startVector, Vector3D endVector);

	/// Retrieve the mesh's bounding box in pixel units.
	/// @return Bounding box of the mesh
	override PixelRightBox getPixelRightBox();

	/// Retrieve the list of vertices that compose the mesh.
	/// @return Linked list of vertices
	override VirtualList getVertices();

	/// Prepare the wireframe for drawing.
	/// @param relativePosition: Position relative to the camera's
	override void render(Vector3D relativePosition);

	/// Draw the wireframe to the frame buffers.
	/// @return True if at least one pixel is drawn; false otherwise
	override bool draw();
}


#endif
