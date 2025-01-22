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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Camera;
class Actor;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class CameraMovementManager
///
/// Inherits from ListenerObject
///
/// Manages camera's movement effects, like shaking, etc.
singleton class CameraMovementManager : ListenerObject
{
	/// @protectedsection

	/// Displacement to the focus actor's position to focus
	/// the camera on
	Vector3D focusActorPositionDisplacement;
	
	/// Actor to focus on the camera
	Actor focusActor;

	/// Cache of the focus actor's position pointer
	const Vector3D* focusActorPosition;

	/// Cache of the focus actor's rotation pointer
	const Rotation* focusActorRotation;
	
	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Process an event that the instance is listen for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// Reset the manager's state
	void reset();

	/// Register the actor that the camera must follow.
	/// @param focusActor: Actor to follow
	void setFocusActor(Actor focusActor);

	/// Retrieve the actor that the camera is following.
	/// @return focusActor: Actor being followed
	Actor getFocusActor();

	/// Register a displacement to be added to the camera's position 
	/// relative to the focus actor's position.
	/// @param focusActorPositionDisplacement: Displacement vector
	void setFocusActorPositionDisplacement(const Vector3D* focusActorPositionDisplacement);

	/// Retrieve the displacement that is added to the camera's position 
	/// relative to the focus actor's position.
	/// @return Displacement vector
	const Vector3D* getFocusActorPositionDisplacement();

	/// Retrieve the camera's change of position in the last game cycle
	/// @return Camera's change of position in the last game cycle
	Vector3D getLastCameraDisplacement();

	/// Focus the camera on the focus actor if any.
	/// @param camera: Camera to focus
	virtual Vector3D focus(Camera camera);
}

#endif
