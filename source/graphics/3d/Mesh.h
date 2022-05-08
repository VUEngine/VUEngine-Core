
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
	Vector3D (*segments)[2];

} MeshSpec;

typedef struct MeshPoint
{
	Vector3D vector;
	PixelVector pixelVector;
	bool projected;

} MeshPoint;


typedef struct MeshSegment
{
	MeshPoint* startPoint;
	MeshPoint* endPoint;

} MeshSegment;

typedef const MeshSpec MeshROMSpec;

/// @ingroup graphics-3d
class Mesh : Wireframe
{
	MeshSpec* meshSpec;
	VirtualList segments;
	const Vector3D* position;
	const Rotation* rotation;

	/// @publicsection
	void constructor(MeshSpec* meshSpec);
	override void draw(bool calculateParallax);
	override void render();
	override void setup(const Vector3D* position, const Rotation* rotation, const Scale* scale);
}


#endif
