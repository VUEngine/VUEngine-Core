/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CAMERA_MOVEMENT_MANAGER_H_
#define CAMERA_MOVEMENT_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Telegram.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup camera
singleton class CameraMovementManager : ListenerObject
{
	/// @publicsection
	static CameraMovementManager getInstance();
	void constructor();
	virtual void focus(uint32 checkIfFocusEntityIsMoving);
}


#endif
