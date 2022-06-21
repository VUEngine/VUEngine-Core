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

/// @ingroup graphics-3d
abstract class Wireframe : ListenerObject
{
	const Vector3D* position;
	const Rotation* rotation;
	uint8 color;

	/// @publicsection
	void constructor(uint8 color);
	void hide();
	void show();
	void setup(const Vector3D* position, const Rotation* rotation, const Scale* scale);
	virtual void draw(bool calculateParallax) = 0;
	virtual void render();
}


#endif
