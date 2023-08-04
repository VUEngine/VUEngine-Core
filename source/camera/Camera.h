/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CAMERA_H_
#define CAMERA_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Entity.h>
#include <CameraMovementManager.h>
#include <CameraEffectManager.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __CAMERA_VIEWING_ANGLE									(56)

//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

extern const Vector3D* _cameraPosition;
extern const Vector3D* _cameraPreviousPosition;
extern const CameraFrustum* _cameraFrustum;
extern const Rotation* _cameraRotation;
extern const Rotation* _cameraInvertedRotation;
extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup camera
singleton class Camera : ListenerObject
{
	// Optical values used in projection values
	Optical optical;
	Optical opticalBackup;
	// Camera position
	Vector3D position;
	// Backup of Camera position
	Vector3D positionBackup;
	// Rotation
	Rotation rotation;
	// Backup of Camera rotation
	Rotation rotationBackup;
	Rotation invertedRotation;
	// Camera position displacement manager
	CameraMovementManager cameraMovementManager;
	// Camera effect manager
	CameraEffectManager cameraEffectManager;
	// Camera position displacement
	Vector3D focusEntityPositionDisplacement;
	// Actor to center the camera around
	Entity focusEntity;
	// Position of actor to center the camera around
	const Vector3D* focusEntityPosition;
	const Rotation* focusEntityRotation;
	// Stage's size in pixels
	Size stageSize;
	// Camera frustum
	CameraFrustum cameraFrustum;
	// Transformation flags
	uint8 transformationFlags;
	// Flag raised when synchronizing UI graphics
	bool synchronizingUIGraphics;
	// Counter to keep track of concurrent calls to suspend UI synchronization
	int8 UISynchronizationInterruptions;

	/// @publicsection
	static Camera getInstance();
	void capPosition();
	void focus(uint32 checkIfFocusEntityIsMoving);
	CameraFrustum getCameraFrustum();
	Entity getFocusEntity();
	Vector3D getLastDisplacement();
	Optical getOptical();
	Vector3D getPosition();
	Rotation getRotation();
	Size getStageSize();
	void translate(Vector3D, int32 cap);
	void onFocusEntityDeleted(Entity actor);
	void startUIGraphicsSynchronization();
	void stopUIGraphicsSynchronization();
	void suspendUIGraphicsSynchronization();
	void resumeUIGraphicsSynchronization();
	void reset();
	void resetCameraFrustum();
	void setCameraEffectManager(CameraEffectManager cameraEffectManager);
	void setCameraMovementManager(CameraMovementManager cameraMovementManager);
	void setFocusEntityPositionDisplacement(Vector3D focusEntityPositionDisplacement);
	void setFocusGameEntity(Entity focusEntity);
	void setOptical(Optical optical);
	void setup(PixelOptical pixelOptical, CameraFrustum cameraFrustum);
	void setPosition(Vector3D position, bool cap);
	void setRotation(Rotation rotation);
	void rotate(Rotation rotation);
	void setStageSize(Size size);
	void startEffect(int32 effect, ...);
	void stopEffect(int32 effect);
	void unsetFocusEntity();
	Vector3D getFocusEntityPosition();
	Vector3D getFocusEntityPositionDisplacement();
	Rotation getFocusEntityRotation();
	uint8 getTransformationFlags();
	void print(int32 x, int32 y, bool inPixels);
}


#endif
