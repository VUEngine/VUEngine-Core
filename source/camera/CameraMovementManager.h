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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Camera;
class Entity;

/// @ingroup camera
singleton class CameraMovementManager : ListenerObject
{
	Vector3D focusEntityPositionDisplacement;
	Entity focusEntity;
	const Vector3D* focusEntityPosition;
	const Rotation* focusEntityRotation;
	
	/// @publicsection
	static CameraMovementManager getInstance();
	void constructor();
	void reset();
	Entity getFocusEntity();
	void setFocusEntity(Entity focusEntity);
	const Vector3D* getFocusEntityPositionDisplacement();
	void setFocusEntityPositionDisplacement(const Vector3D* focusEntityPositionDisplacement);
	Vector3D getLastCameraDisplacement();
	virtual Vector3D focus(Camera camera, bool checkIfFocusEntityIsMoving);
}

#endif
