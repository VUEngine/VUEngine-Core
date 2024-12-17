/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VISUAL_COMPONENT_H_
#define VISUAL_COMPONENT_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Component.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

typedef ComponentSpec VisualComponentSpec;

enum VisualeComponentCommands
{
	cVisualComponentLastCommand = cComponentLastCommand + 1,
	cVisualComponentCommandShow,
	cVisualComponentCommandHide,
	cVisualComponentCommandSetTransparency
};


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class VirtualNode
///
/// Inherits from Object
///
/// Implements an element of linked lists.
abstract class VisualComponent : Component 
{
	/// @protectedsection

	/// Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	uint8 transparency;

	/// Show state flag (__HIDE, __SHOW_NEXT_FRAME, __SHOW)
	uint8 show;

	/// Rendering status flag
	bool rendered;

	/// @publicsection

	/// Propagate a command to the sprites.
	/// @param command: Command to propagate to all the sprites
	/// @param owner: Owner of the sprites to command (all if NULL)
	/// @param ...: Variable arguments list depending on the command
	static void propagateCommand(int32 command, SpatialObject owner, ...);

	/// Compute the rightbox for the owner in base of its visual components.
	/// @param owner: SpatialObject that the components attaches to
	/// @param rightBox: Rightbox to configure
	/// @return True if the owner has visual components; false otherwise
	static bool calculateRightBox(SpatialObject owner, RightBox* rightBox);

	/// Check if at least of the visual components that attach to the provided owner is visible.
	/// @param owner: Object to which the visual components attach to
	/// @return True if at least of the visual components that attach to the provided owner is visible
	static bool isAnyVisible(SpatialObject owner);

	/// Class' constructor
	/// @param owner: SpatialObject that this component attaches to
	/// @param visualComponentSpec: Pointer to the spec that defines how to initialize the visual component
	void constructor(SpatialObject owner, const VisualComponentSpec* visualComponentSpec);

	/// Handle a command.
	/// @param command: Command to handle
	/// @param args: Variable arguments list depending on the command to handle
	override void handleCommand(int32 command, va_list args);

	/// Make the visual component visible.
	void show();

	/// Make the visual component invisible.
	void hide();

	/// Retrieve the transparency mode
	/// @return Transparecy effect
	uint8 getTransparent();

	/// Set the transparency mode
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	void setTransparency(uint8 transparency);

	/// Retrieve the mesh's bounding box.
	/// @return Bounding box of the mesh
	virtual RightBox getRightBox();
}


#endif
