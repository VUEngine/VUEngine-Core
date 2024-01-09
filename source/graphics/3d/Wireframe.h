/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef WIREFRAME_H_
#define WIREFRAME_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <VisualComponent.h>

//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA							__FIXED_EXT_INFINITY


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class VirtualList;

typedef struct WireframeSpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// displacement
	Vector3D displacement;

	/// color
	uint8 color;

	/// transparent
	uint8 transparent;

	/// interlaced
	bool interlaced;

} WireframeSpec;

typedef const WireframeSpec WireframeROMSpec;


/// @ingroup graphics-3d
abstract class Wireframe : VisualComponent
{
	Vector3D displacement;
	fixed_ext_t squaredDistanceToCamera;
	bool drawn;
	bool interlaced;
	uint8 color;
	uint8 bufferIndex;

	/// @publicsection
	void constructor(SpatialObject owner, WireframeSpec* wireframeSpec);
	void setupRenderingMode(const Vector3D* relativePosition);
	void setDisplacement(const Vector3D* displacement);
	bool isVisible();
	PixelVector getPixelPosition();

	virtual void draw() = 0;
	virtual void render();
	virtual VirtualList getVertices();
	virtual PixelRightBox getPixelRightBox();
}


#endif
