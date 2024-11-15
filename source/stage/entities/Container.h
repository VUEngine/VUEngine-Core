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

#define __MAX_CONTAINER_NAME_LENGTH			16


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class VirtualList;

/// @ingroup stage-entities
class Container : SpatialObject
{
	// whether to inherit position, rotation and scale from environment (parents)
	uint8 inheritEnvironment;
	// 3D transformation
	Transformation localTransformation;
	// Children list
	VirtualList children;
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
	// Flag to prevent the same entity from getting ready multiple times
	bool ready;

	/// @publicsection
	void constructor(const char* const name);
	void deleteMyself();

	void streamOut(bool streamOut);
	Container getChildByName(const char* childName, bool recursive);
	Container getChildAtPosition(int16 position);
	int32 getChildCount();
	const Vector3D* getGlobalPosition();
	const Vector3D* getLocalPosition();
	const Rotation* getLocalRotation();
	const Scale* getLocalScale();
	const char* getName();
	Container getParent();
	void invalidateTransformation();
	bool isHidden();
	int32 onPropagatedMessage(va_list args);
	int32 onPropagatedString(va_list args);
	int32 propagateMessage(int32 (*propagatedMessageHandler)(void*, va_list), ...);
	int32 propagateString(int32 (*propagatedMessageHandler)(void*, va_list), ...);
	void purgeChildren();

	void setName(const char* const name);
	void setInheritEnvironment(uint8 inheritEnvironment);
	void translate(const Vector3D* translation);
	void rotate(const Rotation* rotation);
	void scale(const Scale* scale);
	Rotation getRotationFromDirection(const Vector3D* direction, uint8 axis);

	// Use: typeofclass(ClassName)
	bool getChildren(ClassPointer classPointer, VirtualList children);
	void updateChildren();
	void transformChildren(uint8 invalidateTransformationFlag);
	
	void addChild(Container child);
	void removeChild(Container child, bool deleteChild);

	virtual void createComponents();

	virtual void destroyComponents();
	
	virtual void ready(bool recursive);

	/// Make this instance visible.
	virtual void show();

	/// Make this instance invisible.
	virtual void hide();

	/// Update this instance's logic.
	virtual void update();

	/// Prepare to suspend this instance's logic.
	virtual void suspend();

	/// Prepare to resume this instance's logic.
	virtual void resume();

	virtual void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);

	/// Set this instance's transparency effects.
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	virtual void setTransparency(uint8 transparency);

	virtual void setLocalPosition(const Vector3D* position);
	virtual void setLocalRotation(const Rotation* rotation);
	virtual void setLocalScale(const Scale* scale);

	virtual bool handlePropagatedMessage(int32 message);
	virtual bool handlePropagatedString(const char* string);


	virtual void changeEnvironment(Transformation* environmentTransform);

	override void setPosition(const Vector3D* position);
	override void setRotation(const Rotation* rotation);
	override void setScale(const Scale* scale);
}


#endif
