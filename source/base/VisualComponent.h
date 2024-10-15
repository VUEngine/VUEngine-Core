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
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class VirtualNode
///
/// Inherits from Object
///
/// Implements an element of linked lists.
/// @ingroup base
abstract class VisualComponent : Component 
{
	/// Transparent mode (__TRANSPARENT_NONE, __TRANSPARENT_EVEN or __TRANSPARENT_ODD)
	uint8 transparent;

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

	/// Retrieve the transparent mode
	/// @return Transparent mode
	uint8 getTransparent();

	/// Set the transparent mode
	/// @param value: Transparent mode (__TRANSPARENT_NONE, __TRANSPARENT_EVEN or __TRANSPARENT_ODD)
	void setTransparent(uint8 value);
}


#endif
