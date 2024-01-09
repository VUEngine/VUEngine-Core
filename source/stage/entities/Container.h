/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CONTAINER_H_
#define CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SpatialObject.h>
#include <Collider.h>
#include <stdarg.h>


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


#define __MAX_CONTAINER_NAME_LENGTH			16


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Behavior;
class VirtualList;

/// @ingroup stage-entities
class Container : SpatialObject
{
	// whether to inherit position, rotation and scale from environment (parents)
	uint8 inheritEnvironment;
	// Flag to update graphics' attributes
	uint8 invalidateGraphics;
	// Flag to recalculate global transformations
	uint8 invalidateGlobalTransformation;
	// 3D transformation
	Transformation transformation;
	// Children list
	VirtualList children;
	// Bahaviors list
	VirtualList behaviors;
	// Parent
	Container parent;
	// Name
	char* name;
	// Flag for parent to know to delete it
	bool deleteMe;
	// Flag to hide the entity
	bool hidden;
	// flag to enable calls to the update method
	bool update;
	// flag to enable calls to the transform method
	bool transform;
	// Flag to update sprites' attributes
	bool dontStreamOut;

	/// @publicsection
	void constructor(const char* const name);
	void concatenateTransform(Transformation *environmentTransform, Transformation* transformation);
	void streamOut(bool streamOut);
	void deleteMyself();
	void deleteAllChildren();
	void addBehavior(Behavior behavior);
	void removeBehavior(Behavior behavior);
	int32 doKeyHold(int32 pressedKey);
	int32 doKeyPressed(int32 pressedKey);
	int32 doKeyUp(int32 pressedKey);
	Container getChildByName(const char* childName, bool recursive);
	Container getChildAtPosition(int16 position);
	int32 getChildCount();
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
	int32 onPropagatedMessage(va_list args);
	int32 onPropagatedString(va_list args);
	int32 propagateMessage(int32 (*propagatedMessageHandler)(void*, va_list), ...);
	int32 propagateString(int32 (*propagatedMessageHandler)(void*, va_list), ...);
	void purgeChildren();
	void setLocalScale(const Scale* scale);
	void setName(const char* const name);
	void setInheritEnvironment(uint8 inheritEnvironment);
	void updateChildren();
	void updateChildren();
	void translate(const Vector3D* translation);
	void rotate(const Rotation* rotation);
	void scale(const Scale* scale);
	Rotation getRotationFromDirection(const Vector3D* direction, uint8 axis);

	// Use: typeofclass(ClassName)
	bool getBehaviors(ClassPointer classPointer, VirtualList behaviors);
	bool getChildren(ClassPointer classPointer, VirtualList children);
	void transformChildren(uint8 invalidateTransformationFlag);
	
	virtual void ready(bool recursive);
	virtual void update();
	virtual void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);
	virtual void createComponents();
	virtual void initialTransform(const Transformation* environmentTransform);
	virtual void setLocalPosition(const Vector3D* position);
	virtual void setLocalRotation(const Rotation* rotation);
	virtual void setTransparent(uint8 transparent);
	virtual bool handlePropagatedMessage(int32 message);
	virtual bool handlePropagatedString(const char* string);
	virtual void addChild(Container child);
	virtual void removeChild(Container child, bool deleteChild);
	virtual void changeEnvironment(Transformation* environmentTransform);
	virtual void suspend();
	virtual void resume();
	virtual void show();
	virtual void hide();
	virtual bool isTransformed();
	virtual void destroyComponents();

	override const Vector3D* getPosition();
	override void setPosition(const Vector3D* position);
	override const Rotation* getRotation();
	override void setRotation(const Rotation* rotation);
	override const Scale* getScale();
	override void setScale(const Scale* scale);
}


#endif
