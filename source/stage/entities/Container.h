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

#ifndef CONTAINER_H_
#define CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <HardwareManager.h>
#include <MiscStructs.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

/*
// position
	   1 X = 01
	  10 Y = 02
	 100 Z = 04
// rotation
	1000 X = 08
   10000 Y = 10
  100000 Z = 20
//scale
 1000000 X = 40
10000000 Y = 80
*/

#define __INVALIDATE_TRANSFORMATION			0x0F
#define __INVALIDATE_POSITION				0x01
#define __INVALIDATE_ROTATION				0x02
#define __INVALIDATE_SCALE					0x04
#define __INVALIDATE_PROJECTION				0x08


#define __MAX_CONTAINER_NAME_LENGTH			8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Container : SpatialObject
{
	/**
	* @var Transformation 	transformation
	* @brief				3D transformation
	* @memberof			Container
	*/
	Transformation transformation;
	/**
	* @var VirtualList 	children
	* @brief				Children list
	* @memberof			Container
	*/
	VirtualList children;
	/**
	* @var VirtualList 	removedChildren
	* @brief				Removed children list
	* @memberof			Container
	*/
	VirtualList removedChildren;
	/**
	* @var Container 		parent
	* @brief				Parent
	* @memberof			Container
	*/
	Container parent;
	/**
	* @var char* 			name
	* @brief				Name
	* @memberof			Container
	*/
	char* name;
	/**
	* @var u8 				deleteMe
	* @brief				Flag for parent to know to delete it
	* @memberof			Container
	*/
	u8 deleteMe;
	/**
	* @var u8 				hidden
	* @brief				Flag to hide the entity
	* @memberof			Container
	*/
	u8 hidden;
	/**
	* @var u8 				invalidateGlobalTransformation
	* @brief				Flag to recalculate global transformations
	* @memberof			Container
	*/
	u8 invalidateGlobalTransformation;

	void constructor(Container this, const char* const name);
	void applyEnvironmentToTransformation(Container this, const Transformation* environmentTransform);
	void concatenateTransform(Container this, Transformation *environmentTransform, Transformation* transformation);
	void deleteMyself(Container this);
	int doKeyHold(Container this, int pressedKey);
	int doKeyPressed(Container this, int pressedKey);
	int doKeyUp(Container this, int pressedKey);
	Container getChildByName(Container this, char* childName, bool recursive);
	int getChildCount(Container this);
	Transformation getEnvironmentTransform(Container this);
	const Vector3D* getGlobalPosition(Container this);
	s16 getId(Container this);
	const Vector3D* getLocalPosition(Container this);
	const Rotation* getLocalRotation(Container this);
	const Scale* getLocalScale(Container this);
	char* getName(Container this);
	Container getParent(Container this);
	Transformation* getTransform(Container this);
	void invalidateGlobalPosition(Container this);
	void invalidateGlobalRotation(Container this);
	void invalidateGlobalScale(Container this);
	void invalidateGlobalTransformation(Container this);
	bool isHidden(Container this);
	int onPropagatedMessage(Container this, va_list args);
	int propagateMessage(Container this, int (*propagatedMessageHandler)(Container, va_list), ...);
	void purgeChildren(Container this);
	void setLocalScale(Container this, const Scale* scale);
	void setName(Container this, const char* const name);
	void transformNonVirtual(Container this, const Transformation* environmentTransform);
	virtual void iAmDeletingMyself(Container this);
	virtual void update(Container this, u32 elapsedTime);
	virtual void setupGraphics(Container this);
	virtual void releaseGraphics(Container this);
	virtual void transform(Container this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	virtual void synchronizeGraphics(Container this);
	virtual void initialTransform(Container this, const Transformation* environmentTransform, u32 recursive);
	virtual void setLocalPosition(Container this, const Vector3D* position);
	virtual void setLocalRotation(Container this, const Rotation* rotation);
	virtual bool handlePropagatedMessage(Container this, int message);
	virtual void addChild(Container this, Container child);
	virtual void removeChild(Container this, Container child, bool deleteChild);
	virtual void changeEnvironment(Container this, Transformation* environmentTransform);
	virtual void suspend(Container this);
	virtual void resume(Container this);
	virtual void show(Container this);
	virtual void hide(Container this);
	virtual int passMessage(Container this, int (*propagatedMessageHandler)(Container, va_list), va_list args);
}


#endif
