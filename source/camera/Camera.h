/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//											CLASS'S DECLARATION
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


singleton class Camera : Object
{
	/**
	* @var Optical					optical
	* @brief						Optical values used in projection values
	* @memberof					Camera
	*/
	Optical optical;
	/**
	* @var Vector3D				position
	* @brief						Camera position
	* @memberof					Camera
	*/
	Vector3D position;
	/**
	* @var Vector3D				previousPosition
	* @brief						Backup of Camera position
	* @memberof					Camera
	*/
	Vector3D previousPosition;
	/**
	* @var Vector3D				positionBackup
	* @brief						Backup of Camera position
	* @memberof					Camera
	*/
	Vector3D positionBackup;
	/**
	* @var CameraMovementManager	cameraMovementManager
	* @brief						Camera position displacement manager
	* @memberof					Camera
	*/
	CameraMovementManager cameraMovementManager;
	/**
	* @var CameraEffectManager		cameraEffectManager
	* @brief						Camera effect manager
	* @memberof					Camera
	*/
	CameraEffectManager cameraEffectManager;
	/**
	* @var Vector3D				focusEntityPositionDisplacement
	* @brief						Camera position displacement
	* @memberof					Camera
	*/
	Vector3D focusEntityPositionDisplacement;
	/**
	* @var Entity					focusEntity
	* @brief						Actor to center the camera around
	* @memberof					Camera
	*/
	Entity focusEntity;
	/**
	* @var const					focusEntityPosition
	* @brief						Position of actor to center the camera around
	* @memberof					Camera
	*/
	const Vector3D* focusEntityPosition;
	/**
	* @var Vector3D				lastDisplacement
	* @brief						World's camera's last displacement
	* @memberof					Camera
	*/
	Vector3D lastDisplacement;
	/**
	* @var Size					stageSize
	* @brief						Stage's size in pixels
	* @memberof					Camera
	*/
	Size stageSize;
	/**
	* @var CameraFrustum			cameraFrustum
	* @brief						Camera frustum
	* @memberof					Camera
	*/
	CameraFrustum cameraFrustum;

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
	void prepareForUITransform();
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
	void print(int x, int y);
}


#endif
