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
extern const Vector3D* _cameraDisplacement;
extern const CameraFrustum* _cameraFrustum;
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
	Vector3D previousPosition;
	// Backup of Camera position
	Vector3D positionBackup;
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
	// World's camera's last displacement
	Vector3D lastDisplacement;
	// Stage's size in pixels
	Size stageSize;
	// Camera frustum
	CameraFrustum cameraFrustum;

	/// @publicsection
	static Camera getInstance();
	void capPosition();
	void doneUITransform();
	void focus(uint32 checkIfFocusEntityIsMoving);
	void forceDisplacement(int32 flag);
	CameraFrustum getCameraFrustum();
	Entity getFocusEntity();
	Vector3D getLastDisplacement();
	Optical getOptical();
	Vector3D getPosition();
	Size getStageSize();
	void move(Vector3D translation, int32 cap);
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
	void setPosition(Vector3D position);
	void setStageSize(Size size);
	void startEffect(int32 effect, ...);
	void stopEffect(int32 effect);
	void unsetFocusEntity();
	Vector3D getFocusEntityPosition();
	Vector3D getFocusEntityPositionDisplacement();
	void print(int32 x, int32 y, bool inPixels);
}


#endif
