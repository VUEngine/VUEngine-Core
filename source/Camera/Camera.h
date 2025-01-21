/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CAMERA_H_
#define CAMERA_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Actor;
class CameraMovementManager;
class CameraEffectManager;

extern const Vector3D* _cameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Vector3D* _cameraPreviousPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const CameraFrustum* _cameraFrustum __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Rotation* _cameraRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Rotation* _cameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Optical* _optical __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __CAMERA_VIEWING_ANGLE									(56)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Camera
///
/// Inherits from ListenerObject
///
/// Represents the player's view point.
singleton class Camera : ListenerObject
{
	/// Optical configuration values used for projections
	Optical optical;

	/// Camera's transformation
	Transformation transformation;

	/// Displacement applied to the position when it changes
	Vector3D displacement;

	/// Saves the camera's change of position in the last game cycle
	Vector3D lastDisplacement;

	/// Camera's rotation's complement
	Rotation invertedRotation;

	/// Camera's movement manager
	CameraMovementManager cameraMovementManager;

	/// Camera's special effects manager
	CameraEffectManager cameraEffectManager;

	/// Cached stage's size used to optionally limit the camera's movement
	Size stageSize;

	/// Camera's frustum configuration
	CameraFrustum cameraFrustum;

	/// Transformation flags to keep track of changes in the camera's position 
	/// and rotation during the current game cycle
	uint8 transformationFlags;

	/// @publicsection
	
	/// Reset the camera's state.
	void reset();

	/// Setup the camera's optical and frustum configuration that determine
	/// the results from 3D to 2D projection.
	/// @param pixelOptical: Configuration struct for the projection functions
	/// @param cameraFrustum: Player's point of view configuration
	void setup(PixelOptical pixelOptical, CameraFrustum cameraFrustum);

	/// Set the manager of the camera's properties.
	/// @param cameraMovementManager: Movement manager
	void setCameraMovementManager(CameraMovementManager cameraMovementManager);

	/// Retrieve the camera's current movement manager
	/// @return Camera's current movement manager
	CameraMovementManager getCameraMovementManager();

	/// Set the manager of the camera's special effects.
	/// @param cameraEffectManager: Special effects manager
	void setCameraEffectManager(CameraEffectManager cameraEffectManager);

	/// Retrieve the camera's current special effects manager
	/// @return Camera's current special effects manager
	CameraEffectManager getCameraEffectManager();

	/// Save the stage's size. 
	/// @param size: Stage's size
	void setStageSize(Size size);

	/// Retrieve the stage's cached size.
	/// @return Stage's size
	Size getStageSize();

	/// Register the actor that the camera must follow.
	/// @param focusActor: Actor to follow
	void setFocusActor(Actor focusActor);

	/// Retrieve the actor that the camera is following.
	/// @return focusActor: Actor being followed
	Actor getFocusActor();

	/// Stop following any actor.
	void unsetFocusActor();

	/// Register a displacement to be added to the camera's position 
	/// relative to the focus actor's position.
	/// @param focusActorPositionDisplacement: Displacement vector
	void setFocusActorPositionDisplacement(Vector3D focusActorPositionDisplacement);

	/// Retrieve the displacement that is added to the camera's position 
	/// relative to the focus actor's position.
	/// @return Displacement vector
	Vector3D getFocusActorPositionDisplacement();
	
	/// Set a constant displacement to be added to the camera's position.
	/// @param displacement: Displacement vector
	void setDisplacement(Vector3D);

	/// Retrieve the constant displacement that is added to the camera's position.
	/// @return Displacement vector
	Vector3D geDisplacement();

	/// Set the optical configuration values used for projections.
	/// @param optical: configuration struct with the values used for projections
	void setOptical(Optical optical);

	/// Retrieve the optical configuration values used for projections.
	/// @return Optical struct with the configuration values used for projections
	Optical getOptical();

	/// Set the camera's transformation.
	/// @param transformation: New transformation
	void setTransformation(Transformation transformation, bool cap);

	/// Retrieve the camera's transformation.
	/// @return Camera's transformation
	Transformation getTransformation();

	/// Set the camera's position.
	/// @param position: 3D vector
	/// @param cap: Cap the camera's position within the stage's size if true
	void setPosition(Vector3D position, bool cap);
	
	/// Add a displacement the camera's current position.
	/// @param displacement: Displacement vector
	/// @param cap: Cap the camera's position within the stage's size if true
	void translate(Vector3D displacement, int32 cap);

	/// Retrieve the camera's position.
	/// @return Camera's position
	Vector3D getPosition();

	/// Set the camera's rotation.
	/// @param rotation: Rotation to assign to the camera
	void setRotation(Rotation rotation);

	/// Add a rotation the camera's current rotation.
	/// @param rotation: Rotation delta
	void rotate(Rotation rotation);

	/// Retrieve the camera's rotation.
	/// @return Camera's rotation
	Rotation getRotation();

	/// Retrieve the camera's frustum configuration.
	/// @return Camera's frustum configuration
	CameraFrustum getCameraFrustum();

	/// Retrieve the camera's change of position in the last game cycle
	/// @return Camera's change of position in the last game cycle
	Vector3D getLastDisplacement();

	/// Retrieve the transformation flags that keep track of changes in the camera's position 
	/// and rotation during the current game cycle.
	/// @return Transformation flags
	uint8 getTransformationFlags();

	/// Focus the camera on the focus actor if any.
	void focus();

	/// Start a camera effect.
	/// @param effect: Code of the effect to start
	/// @param ...: Variable arguments list depending on the effect to start
	void startEffect(int32 effect, ...);

	/// Stop a camera effect.
	/// @param effect: Code of the effect to stop
	void stopEffect(int32 effect);

	/// Print the camera's status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param inPixels: If true, the spatial data is printed in pixel units; in meter, otherwise
	void print(int32 x, int32 y, bool inPixels);
}

#endif
