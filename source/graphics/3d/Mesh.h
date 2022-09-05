
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

typedef struct MeshSpec
{
	// Wireframe
	WireframeSpec wireframeSpec;

	/// segments
	PixelVector (*segments)[2];

} MeshSpec;

typedef const MeshSpec MeshROMSpec;

typedef struct Vertex
{
	Vector3D vector;
	PixelVector pixelVector;

} Vertex;


typedef struct MeshSegment
{
	Vertex* fromVertex;
	Vertex* toVertex;

} MeshSegment;


/// @ingroup graphics-3d
class Mesh : Wireframe
{
	VirtualList segments;
	VirtualList vertices;

	static PixelRightBox getPixelRightBoxFromSpec(MeshSpec* meshSpec);

	/// @publicsection
	void constructor(MeshSpec* meshSpec);
	
	override PixelRightBox getPixelRightBox();
	override VirtualList getVertices();
	override void draw();
	void drawInterlaced();
	override void render();
}


#endif
