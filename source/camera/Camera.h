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

#include <Object.h>
#include <Telegram.h>
#include <Entity.h>
#include <CameraMovementManager.h>
#include <CameraEffectManager.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// state of movement
#define __ACTIVE 		(int32)0x1
#define __PASSIVE		(int32)0x0


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Camera frustum
 *
 * @memberof 	Camera
 */
typedef struct CameraFrustum
{
	/// x0 frustum
	int16 x0;
	/// y0 frustum
	int16 y0;
	/// z0 frustum
	int16 z0;
	/// x1 frustum
	int16 x1;
	/// y1 frustum
	int16 y1;
	/// z1 frustum
	int16 z1;

} CameraFrustum;


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
singleton class Camera : Object
{
	// Optical values used in projection values
	Optical optical;
	// Camera position
	Vector3D position;
	// Backup of Camera position
	Vector3D positionBackup;
	// Rotation
	Rotation rotation;
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

	/// @publicsection
	static Camera getInstance();
	void capPosition();
	void doneUITransform();
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
	void prepareForUI();
	void reset();
	void resetCameraFrustum();
	void setCameraEffectManager(CameraEffectManager cameraEffectManager);
	void setCameraFrustum(CameraFrustum cameraFrustum);
	void setCameraMovementManager(CameraMovementManager cameraMovementManager);
	void setFocusEntityPositionDisplacement(Vector3D focusEntityPositionDisplacement);
	void setFocusGameEntity(Entity focusEntity);
	void setOptical(Optical optical);
	void setOpticalFromPixelOptical(PixelOptical pixelOptical);
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
