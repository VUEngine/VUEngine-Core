/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef WIREFRAME_MANAGER_H_
#define WIREFRAME_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ComponentManager.h>
#include <Wireframe.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;

extern Vector3D _cameraDirection __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Vector3D _previousCameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Vector3D _previousCameraPositionBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Rotation _previousCameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern Rotation _previousCameraInvertedRotationBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class WireframeManager
///
/// Inherits from ComponentManager
///
/// Manages instances of Wireframe.
class WireframeManager : ComponentManager
{
	/// @protectedsection

	/// Flag used to break the rendering due to high frame time
	volatile bool stopRendering;

	/// Flag used to break the drawing due to high frame time
	volatile bool stopDrawing;

	/// Flag to distinguish between even and odd game frames
	bool evenFrame;

	/// Number of rendered wireframes during the last game cycle
	uint8 renderedWireframes;

	/// Number of drawing wireframes during the last game cycle
	uint8 drawnWireframes;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Process an event that the instance is listen for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// Retrieve the compoment type that the manager manages.
	/// @return Component type
	override uint32 getType();

	/// Enable the manager.
	override void enable();

	/// Disable the manager.
	override void disable();

	/// Create a wireframe with the provided spec.
	/// @param owner: Object to which the wireframe will attach to
	/// @param wireframeSpec: Spec to use to create the wireframe
	/// @return Created wireframe
	override Wireframe instantiateComponent(Entity owner, const WireframeSpec* wireframeSpec);

	/// Check if at least of the sprites that attach to the provided owner is visible.
	/// @param owner: Object to which the sprites attach to
	/// @return True if at least of the sprites that attach to the provided owner is visible
	override bool isAnyVisible(Entity owner);

	/// Enable wireframe rendering and drawing.
	void enable();

	/// Disable wireframe rendering and drawing.
	void disable();

	/// Register a wireframe to be managed
	/// @param wireframe: Wireframe to be managed
	/// @return True if the wireframe was successfully registered; false otherwise
	bool registerWireframe(Wireframe wireframe);

	/// Unregister a wireframe to be managed
	/// @param wireframe: Wireframe to no longer manage
	/// @return True if the wireframe was successfully unregistered; false otherwise
	bool unregisterWireframe(Wireframe wireframe);

	/// Prepare wireframe's graphical data for drawing
	void render();

	/// Draw wireframes to the frame buffers
	void draw();

	/// Show all wireframes (available only when __TOOLS is defined).
	void showAllWireframes();
	
	/// Hide all wireframes (available only when __TOOLS is defined).
	void hideAllWireframes();

	/// Check if there are any registered wireframes.
	/// @return True if there are any registered wireframes; false otherwise
	bool hasWireframes();	

	/// Print the manager's state.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);
}

#endif
