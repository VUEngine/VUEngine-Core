/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CAMERA_MOVEMENT_MANAGER_H_
#define CAMERA_MOVEMENT_MANAGER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class Camera;
class Entity;

//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class CameraMovementManager
///
/// Inherits from ListenerObject
///
/// Manages camera's movement effects, like shaking, etc.
/// @ingroup camera
singleton class CameraMovementManager : ListenerObject
{
	/// Displacement to the focus entity's position to focus
	/// the camera on
	Vector3D focusEntityPositionDisplacement;
	
	/// Entity to focus on the camera
	Entity focusEntity;

	/// Cache of the focus entity's position pointer
	const Vector3D* focusEntityPosition;

	/// Cache of the focus entity's rotation pointer
	const Rotation* focusEntityRotation;
	
	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return Camera singleton
	static CameraMovementManager getInstance();

	/// Class' constructor
	void constructor();

	/// Reset the manager's state
	void reset();

	/// Register the entity that the camera must follow.
	/// @param focusEntity: Entity to follow
	void setFocusEntity(Entity focusEntity);

	/// Retrieve the entity that the camera is following.
	/// @return focusEntity: Entity being followed
	Entity getFocusEntity();

	/// Register a displacement to be added to the camera's position 
	/// relative to the focus entity's position.
	/// @param focusEntityPositionDisplacement: Displacement vector
	void setFocusEntityPositionDisplacement(const Vector3D* focusEntityPositionDisplacement);

	/// Retrieve the displacement that is added to the camera's position 
	/// relative to the focus entity's position.
	/// @return Displacement vector
	const Vector3D* getFocusEntityPositionDisplacement();

	/// Retrieve the camera's change of position in the last game cycle
	/// @return Camera's change of position in the last game cycle
	Vector3D getLastCameraDisplacement();

	/// Focus the camera on the focus entity if any.
	/// @param camera: Camera to focus
	virtual Vector3D focus(Camera camera);
}

#endif
