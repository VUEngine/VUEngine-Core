/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#define __ACTIVE 		(int)0x1
#define __PASSIVE		(int)0x0


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
	s16 x0;
	/// y0 frustum
	s16 y0;
	/// z0 frustum
	s16 z0;
	/// x1 frustum
	s16 x1;
	/// y1 frustum
	s16 y1;
	/// z1 frustum
	s16 z1;

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
	void focus(u32 checkIfFocusEntityIsMoving);
	void forceDisplacement(int flag);
	CameraFrustum getCameraFrustum();
	Entity getFocusEntity();
	Vector3D getLastDisplacement();
	Optical getOptical();
	Vector3D getPosition();
	Size getStageSize();
	void move(Vector3D translation, int cap);
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
	void startEffect(int effect, ...);
	void stopEffect(int effect);
	void unsetFocusEntity();
	Vector3D getFocusEntityPosition();
	Vector3D getFocusEntityPositionDisplacement();
	void print(int x, int y, bool inPixels);
}


#endif
