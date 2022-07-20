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

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct WireframeSpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// color
	uint8 color;

	/// transparent
	uint8 transparent;

} WireframeSpec;

typedef const WireframeSpec WireframeROMSpec;


/// @ingroup graphics-3d
abstract class Wireframe : ListenerObject
{
	WireframeSpec* wireframeSpec;
	const Vector3D* position;
	const Rotation* rotation;
	const Scale* scale;
	bool interlaced;
	uint8 color;
	uint8 bufferIndex;
	uint8 show;
	uint8 transparent;

	/// @publicsection
	void constructor(WireframeSpec* wireframeSpec);
	void setTransparent(bool transparent);
	void hide();
	void show();
	void setupRenderingMode(const Vector3D* relativePosition);

	virtual void draw() = 0;
	virtual void render();
	virtual VirtualList getVertices();
	virtual void setup(const Vector3D* position, const Rotation* rotation, const Scale* scale);
}


#endif
