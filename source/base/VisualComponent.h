/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VISUAL_COMPONENT_H_
#define VISUAL_COMPONENT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Component.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef ComponentSpec VisualComponentSpec;

abstract class VisualComponent : Component 
{
	PixelVector center;
	uint8 transparent;
	uint8 show;
	bool rendered;

	/// @publicsection
	void constructor(SpatialObject owner, const VisualComponentSpec* visualComponentSpec);

	void show();
	void hide();
	uint8 getTransparent();
	void setTransparent(uint8 value);
}


#endif
