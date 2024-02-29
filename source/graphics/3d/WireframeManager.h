/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef WIREFRAME_MANAGER_H_
#define WIREFRAME_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class VirtualList;

/// @ingroup graphics-3d

singleton class WireframeManager : ListenerObject
{
	volatile bool stopRendering;
	volatile bool stopDrawing;
	bool evenFrame;
	bool disabled;
	uint8 renderedWireframes;
	uint8 drawnWireframes;
	
	// Wireframes
	VirtualList wireframes;

	/// @publicsection
	static WireframeManager getInstance();

	void draw();
	void render();
	void print(int32 x, int32 y);
	Wireframe createWireframe(WireframeSpec* wireframeSpec, SpatialObject owner);
	void destroyWireframe(Wireframe wireframe);
	Wireframe registerWireframe(Wireframe wireframe);
	Wireframe unregisterWireframe(Wireframe wireframe);
	void reset();
	void enable();
	void disable();
	void hideWireframes();
	void showWireframes();	
}

extern Vector3D _cameraDirection __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Vector3D _previousCameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Vector3D _previousCameraPositionBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Rotation _previousCameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Rotation _previousCameraInvertedRotationBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;


#endif
