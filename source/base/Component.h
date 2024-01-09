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
	const ComponentSpec* componentSpec;
	SpatialObject owner;
	const Transformation* transformation;
	uint8 show;
	bool rendered;
	uint8 transparent;
	PixelVector center;

	/// @publicsection
	void constructor(SpatialObject owner, const ComponentSpec* componentSpec);
	void hide();
	void show();
	uint8 getTransparent();
	void setTransparent(uint8 value);
	Vector3D getPosition();
	Rotation getRotation();
	Scale getScale();
}


#endif
