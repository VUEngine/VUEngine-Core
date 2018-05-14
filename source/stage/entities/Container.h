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

	void constructor(const char* const name);
	void applyEnvironmentToTransformation(const Transformation* environmentTransform);
	void concatenateTransform(Transformation *environmentTransform, Transformation* transformation);
	void deleteMyself();
	int doKeyHold(int pressedKey);
	int doKeyPressed(int pressedKey);
	int doKeyUp(int pressedKey);
	Container getChildByName(char* childName, bool recursive);
	int getChildCount();
	Transformation getEnvironmentTransform();
	const Vector3D* getGlobalPosition();
	s16 getId();
	const Vector3D* getLocalPosition();
	const Rotation* getLocalRotation();
	const Scale* getLocalScale();
	char* getName();
	Container getParent();
	Transformation* getTransform();
	void invalidateGlobalPosition();
	void invalidateGlobalRotation();
	void invalidateGlobalScale();
	void invalidateGlobalTransformation();
	bool isHidden();
	int onPropagatedMessage(va_list args);
	int propagateMessage(int (*propagatedMessageHandler)(Container, va_list), ...);
	void purgeChildren();
	void setLocalScale(const Scale* scale);
	void setName(const char* const name);
	void transformNonVirtual(const Transformation* environmentTransform);
	virtual void iAmDeletingMyself();
	virtual void update(u32 elapsedTime);
	virtual void setupGraphics();
	virtual void releaseGraphics();
	virtual void transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	virtual void synchronizeGraphics();
	virtual void initialTransform(const Transformation* environmentTransform, u32 recursive);
	virtual void setLocalPosition(const Vector3D* position);
	virtual void setLocalRotation(const Rotation* rotation);
	virtual bool handlePropagatedMessage(int message);
	virtual void addChild(Container child);
	virtual void removeChild(Container child, bool deleteChild);
	virtual void changeEnvironment(Transformation* environmentTransform);
	virtual void suspend();
	virtual void resume();
	virtual void show();
	virtual void hide();
	virtual int passMessage(int (*propagatedMessageHandler)(Container, va_list), va_list args);
}


#endif
