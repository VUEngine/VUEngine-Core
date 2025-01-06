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

	/// Method to retrieve the singleton instance
	/// @return Camera singleton
	static Camera getInstance();
	
	/// Reset the camera's state.
	static void reset();

	/// Setup the camera's optical and frustum configuration that determine
	/// the results from 3D to 2D projection.
	/// @param pixelOptical: Configuration struct for the projection functions
	/// @param cameraFrustum: Player's point of view configuration
	static void setup(PixelOptical pixelOptical, CameraFrustum cameraFrustum);

	/// Set the manager of the camera's properties.
	/// @param cameraMovementManager: Movement manager
	static void setCameraMovementManager(CameraMovementManager cameraMovementManager);

	/// Retrieve the camera's current movement manager
	/// @return Camera's current movement manager
	static CameraMovementManager getCameraMovementManager();

	/// Set the manager of the camera's special effects.
	/// @param cameraEffectManager: Special effects manager
	static void setCameraEffectManager(CameraEffectManager cameraEffectManager);

	/// Retrieve the camera's current special effects manager
	/// @return Camera's current special effects manager
	static CameraEffectManager getCameraEffectManager();

	/// Save the stage's size. 
	/// @param size: Stage's size
	static void setStageSize(Size size);

	/// Retrieve the stage's cached size.
	/// @return Stage's size
	static Size getStageSize();

	/// Register the actor that the camera must follow.
	/// @param focusActor: Actor to follow
	static void setFocusActor(Actor focusActor);

	/// Retrieve the actor that the camera is following.
	/// @return focusActor: Actor being followed
	static Actor getFocusActor();

	/// Stop following any actor.
	static void unsetFocusActor();

	/// Register a displacement to be added to the camera's position 
	/// relative to the focus actor's position.
	/// @param focusActorPositionDisplacement: Displacement vector
	static void setFocusActorPositionDisplacement(Vector3D focusActorPositionDisplacement);

	/// Retrieve the displacement that is added to the camera's position 
	/// relative to the focus actor's position.
	/// @return Displacement vector
	static Vector3D getFocusActorPositionDisplacement();
	
	/// Set a constant displacement to be added to the camera's position.
	/// @param displacement: Displacement vector
	static void setDisplacement(Vector3D);

	/// Retrieve the constant displacement that is added to the camera's position.
	/// @return Displacement vector
	static Vector3D geDisplacement();

	/// Set the optical configuration values used for projections.
	/// @param optical: configuration struct with the values used for projections
	static void setOptical(Optical optical);

	/// Retrieve the optical configuration values used for projections.
	/// @return Optical struct with the configuration values used for projections
	static Optical getOptical();

	/// Set the camera's transformation.
	/// @param transformation: New transformation
	static void setTransformation(Transformation transformation, bool cap);

	/// Retrieve the camera's transformation.
	/// @return Camera's transformation
	static Transformation getTransformation();

	/// Set the camera's position.
	/// @param position: 3D vector
	/// @param cap: Cap the camera's position within the stage's size if true
	static void setPosition(Vector3D position, bool cap);
	
	/// Add a displacement the camera's current position.
	/// @param displacement: Displacement vector
	/// @param cap: Cap the camera's position within the stage's size if true
	static void translate(Vector3D displacement, int32 cap);

	/// Retrieve the camera's position.
	/// @return Camera's position
	static Vector3D getPosition();

	/// Set the camera's rotation.
	/// @param rotation: Rotation to assign to the camera
	static void setRotation(Rotation rotation);

	/// Add a rotation the camera's current rotation.
	/// @param rotation: Rotation delta
	static void rotate(Rotation rotation);

	/// Retrieve the camera's rotation.
	/// @return Camera's rotation
	static Rotation getRotation();

	/// Retrieve the camera's frustum configuration.
	/// @return Camera's frustum configuration
	static CameraFrustum getCameraFrustum();

	/// Retrieve the camera's change of position in the last game cycle
	/// @return Camera's change of position in the last game cycle
	static Vector3D getLastDisplacement();

	/// Retrieve the transformation flags that keep track of changes in the camera's position 
	/// and rotation during the current game cycle.
	/// @return Transformation flags
	static uint8 getTransformationFlags();

	/// Focus the camera on the focus actor if any.
	static void focus();

	/// Start a camera effect.
	/// @param effect: Code of the effect to start
	/// @param ...: Variable arguments list depending on the effect to start
	static void startEffect(int32 effect, ...);

	/// Stop a camera effect.
	/// @param effect: Code of the effect to stop
	static void stopEffect(int32 effect);

	/// Print the camera's status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param inPixels: If true, the spatial data is printed in pixel units; in meter, otherwise
	static void print(int32 x, int32 y, bool inPixels);
}

#endif
