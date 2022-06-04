
/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MESH_H_
#define MESH_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a shape
typedef struct MeshSpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// color
	uint8 color;

	/// segments
	PixelVector (*segments)[2];

} MeshSpec;

typedef struct Vertex
{
	Vector3D vector;
	PixelVector pixelVector;

} Vertex;


typedef struct MeshSegment
{
	Vertex* fromVertex;
	Vertex* toVertex;
	uint8 bufferIndex;

} MeshSegment;

typedef const MeshSpec MeshROMSpec;

/// @ingroup graphics-3d
class Mesh : Wireframe
{
	MeshSpec* meshSpec;
	VirtualList segments;
	VirtualList vertices;

	/// @publicsection
	void constructor(MeshSpec* meshSpec);
	VirtualList getVertices();

	override void draw(bool calculateParallax);
	override void render();
}


#endif
