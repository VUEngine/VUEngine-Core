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
	override Wireframe create(Entity owner, const WireframeSpec* wireframeSpec);

	/// Retrieve information regarding if the components are visile.
	/// @return True if the components managed are visual; false otherwise
	override bool areComponentsVisual();

	/// Enable wireframe rendering and drawing.
	void enable();

	/// Disable wireframe rendering and drawing.
	void disable();

	/// Prepare wireframe's graphical data for drawing
	void render();

	/// Invalidate the rendering status of all wireframes so they re-render again in the next cycle.
	void invalidateRendering();

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
