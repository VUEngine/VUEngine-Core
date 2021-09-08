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
#include <Behavior.h>


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

#define __INHERIT_TRANSFORMATION			0x0F
#define __INHERIT_NONE						0x00
#define __INHERIT_POSITION					0x01
#define __INHERIT_ROTATION					0x02
#define __INHERIT_SCALE						0x04


#define __MAX_CONTAINER_NAME_LENGTH			8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities
class Container : SpatialObject
{
	// 3D transformation
	Transformation transformation;
	// Children list
	VirtualList children;
	// Removed children list
	VirtualList removedChildren;
	// Bahaviors list
	VirtualList behaviors;
	// Parent
	Container parent;
	// Name
	char* name;
	// whether to inherit position, rotation and scale from environment (parents)
	uint8 inheritEnvironment;
	// Flag to update graphics' attributes
	bool invalidateGraphics;
	// Flag for parent to know to delete it
	uint8 deleteMe;
	// Flag to hide the entity
	uint8 hidden;
	// Flag to recalculate global transformations
	uint8 invalidateGlobalTransformation;
	// flag to enable calls to update method
	bool update;
	// flag to enable calls to update method
	bool transform;
	// Flag to update sprites' attributes
	bool dontStreamOut;
	// Raise flag when transformed to allow graphics sync
	bool transformed;

	/// @publicsection
	void constructor(const char* const name);
	void concatenateTransform(Transformation *environmentTransform, Transformation* transformation);
	void deleteMyself();
	void addBehavior(Behavior behavior);
	void removeBehavior(Behavior behavior);
	int doKeyHold(int pressedKey);
	int doKeyPressed(int pressedKey);
	int doKeyUp(int pressedKey);
	Container getChildByName(const char* childName, bool recursive);
	int getChildCount();
	Transformation getEnvironmentTransform();
	const Vector3D* getGlobalPosition();
	const Vector3D* getLocalPosition();
	const Rotation* getLocalRotation();
	const Scale* getLocalScale();
	const char* getName();
	Container getParent();
	Transformation* getTransform();
	void invalidateGlobalPosition();
	void invalidateGlobalRotation();
	void invalidateGlobalScale();
	void invalidateGlobalTransformation();
	bool isHidden();
	int onPropagatedMessage(va_list args);
	int propagateMessage(int (*propagatedMessageHandler)(void*, va_list), ...);
	void purgeChildren();
	void setLocalScale(const Scale* scale);
	void setName(const char* const name);
	void setInheritEnvironment(uint8 inheritEnvironment);
	void updateChildren(uint32 elapsedTime);
	void updateBehaviors(uint32 elapsedTime);
	void synchronizeChildrenGraphics();

	// Use: typeofclass(ClassName)
	bool getBehaviors(ClassPointer classPointer, VirtualList behaviors);
	bool getChildren(ClassPointer classPointer, VirtualList children);
	virtual void iAmDeletingMyself();
	virtual void ready(bool recursive);
	virtual void update(uint32 elapsedTime);
	virtual void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);
	virtual void transformChildren(uint8 invalidateTransformationFlag);
	virtual void synchronizeGraphics();
	virtual void initialTransform(const Transformation* environmentTransform, uint32 recursive);
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
	virtual int passMessage(int (*propagatedMessageHandler)(void*, va_list), va_list args);
	virtual bool isTransformed();

	override void setPosition(const Vector3D* position);
	override const Vector3D* getPosition();
}


#endif
