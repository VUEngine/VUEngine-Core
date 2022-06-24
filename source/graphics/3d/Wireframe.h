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

} WireframeSpec;


/// @ingroup graphics-3d
abstract class Wireframe : ListenerObject
{
	WireframeSpec* wireframeSpec;
	const Vector3D* position;
	const Rotation* rotation;
	bool interlaced;
	uint8 color;

	/// @publicsection
	void constructor(WireframeSpec* wireframeSpec);
	void hide();
	void show();
	void setup(const Vector3D* position, const Rotation* rotation, const Scale* scale);
	void setupRenderingMode(fix10_6_ext distanceToCamera);

	virtual void draw(bool calculateParallax) = 0;
	virtual void render();
	virtual VirtualList getVertices();
}


#endif
