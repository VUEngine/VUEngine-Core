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
// FORWARD DECLARATIONS
//=========================================================================================================

typedef ComponentSpec VisualComponentSpec;


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

	/// Class' constructor
	/// @param owner: SpatialObject that this component attaches to
	/// @param visualComponentSpec: Pointer to the spec that defines how to initialize the visual component
	void constructor(SpatialObject owner, const VisualComponentSpec* visualComponentSpec);

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
