/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class SpatialObject;

typedef const void ComponentSpec;

abstract class Component : ListenerObject 
{
	SpatialObject owner;
	const ComponentSpec* componentSpec;
	const Vector3D* position;
	const Rotation* rotation;
	const Scale* scale;
	uint8 show;
	bool draw;
	bool positioned;
	uint8 transparent;
	// Flag to avoid rewriting DRAM's cache if not needed (helps a lot in menus)
 	bool renderFlag;
	PixelVector center;

	/// @publicsection
	void constructor(SpatialObject owner, const ComponentSpec* componentSpec);
	void hide();
	void show();
	uint8 getTransparent();
	void setTransparent(uint8 value);
}


#endif
