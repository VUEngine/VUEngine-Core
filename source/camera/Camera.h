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


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __CAMERA_VIEWING_ANGLE									(56)

//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

extern const Vector3D* _cameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Vector3D* _cameraPreviousPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const CameraFrustum* _cameraFrustum __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Rotation* _cameraRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Rotation* _cameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern const Optical* _optical __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Entity;
class CameraMovementManager;
class CameraEffectManager;


/// @ingroup camera
singleton class Camera : ListenerObject
{
	// Optical values used in projection values
	Optical optical;
	// Camera position
	Vector3D position;
	// Camera position displacement
	Vector3D displacement;
	// Rotation
	Rotation rotation;
	// Backup of Camera rotation
	Rotation invertedRotation;
	// Camera position displacement manager
	CameraMovementManager cameraMovementManager;
	// Camera effect manager
	CameraEffectManager cameraEffectManager;
	// Stage's size in pixels
	Size stageSize;
	// Camera frustum
	CameraFrustum cameraFrustum;
	// Transformation flags
	uint8 transformationFlags;

	/// @publicsection
	static Camera getInstance();
	void capPosition();
	void focus(bool checkIfFocusEntityIsMoving);
	CameraFrustum getCameraFrustum();
	Entity getFocusEntity();
	Vector3D getLastDisplacement();
	Optical getOptical();
	Vector3D getPosition();
	Rotation getRotation();
	Size getStageSize();
	void translate(Vector3D, int32 cap);
	void onFocusEntityDeleted(Entity actor);
	void suspendUIGraphicsSynchronization();
	void resumeUIGraphicsSynchronization();
	void reset();
	void resetCameraFrustum();
	void setCameraEffectManager(CameraEffectManager cameraEffectManager);
	void setCameraMovementManager(CameraMovementManager cameraMovementManager);
	Vector3D getFocusEntityPositionDisplacement();
	void setFocusEntityPositionDisplacement(Vector3D focusEntityPositionDisplacement);
	void setFocusEntity(Entity focusEntity);
	void setOptical(Optical optical);
	void setup(PixelOptical pixelOptical, CameraFrustum cameraFrustum);
	void setPosition(Vector3D position, bool cap);
	void setRotation(Rotation rotation);
	void rotate(Rotation rotation);
	void setStageSize(Size size);
	void startEffect(int32 effect, ...);
	void stopEffect(int32 effect);
	void unsetFocusEntity();
	uint8 getTransformationFlags();
	void print(int32 x, int32 y, bool inPixels);
}


#endif
