/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CONTAINER_H_
#define CONTAINER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <SpatialObject.h>
#include <Collider.h>
#include <stdarg.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __MAX_CONTAINER_NAME_LENGTH			16


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Container
///
/// Inherits from SpatialObject
///
/// Adds parenting cababilities to spatial objects.
class Container : SpatialObject
{
	/// @protectedsection

	/// Flags that determine whether to inherit position, rotation 
	/// and scale from environment (parents)
	uint8 inheritEnvironment;

	/// 3D local transformation
	Transformation localTransformation;

	/// Linked list of child containers
	VirtualList children;

	/// Container of which this one is a child
	Container parent;

	/// Container's name
	char* name;

	/// If true, the parent will delete this container when appropriate
	bool deleteMe;

	/// Flag to mark the container (and its children) as non visible
	bool hidden;

	/// Flag to allow/prohibit calls to the update method
	bool update;

	/// Flag to allow/prohibit calls to the transform method
	bool transform;

	// Flag to prevent the same container from making ready multiple times
	bool ready;

	// Flag to prevent the container from being streamed out when out of the camera's reach
	bool dontStreamOut;
	
	/// @publicsection

	/// Class' constructor
	/// @param name: Name to assign to the new instance
	void constructor(const char* const name);

	/// Set the container's position.
	/// @param position: 3D vector defining the object's new position
	override void setPosition(const Vector3D* position);

	/// Set the container's rotation.
	/// @param rotation: Rotation
	override void setRotation(const Rotation* rotation);

	/// Set the container's scale.
	/// @param scale: Scale
	override void setScale(const Scale* scale);

	/// Delete this container when appropriate.
	/// Containers must not be deleted nor created directly by the client code
	void deleteMyself();

	/// Set the container's name.
	/// @param name: Name to assign to the instance
	void setName(const char* const name);

	/// Retrive the container's name.
	/// @return Pointer to the container's name
	const char* getName();

	/// Set the streaming effects on this container.
	/// @param streamOut: If false, this container won't be streamed out when
	/// outside of the camera's reach
	void streamOut(bool streamOut);

	/// Retrieve this container's parent container.
	/// @return Parent container
	Container getParent();

	/// Add a container as a child.
	/// @param child: Container to be added as a child
	void addChild(Container child);

	/// Reomve a child container for this container's children list.
	/// @param child: Container to be remove as a child
	/// @param deleteChild: If true, the child will be deleted in the next game cycle
	void removeChild(Container child, bool deleteChild);

	/// Force the destruction of all children marked to be deleted.
	void purgeChildren();

	/// Retrieve the linked list of children that are instances of the provided class.
	/// @param classPointer: Pointer to the class to use as search criteria. Usage: typeofclass(ClassName)
	/// @param children: Linked list to be filled with the children that meed the search criteria 
	/// (it is externally allocated and must be externally deleted)
	/// @return True if one or more children met the search criteria; false otherwise
	bool getChildren(ClassPointer classPointer, VirtualList children);

	/// Find a child with the provided name.
	/// @param childName: Name to look for
	/// @param recursive: If true, the seach extends to grand children, grand grand children, etc.
	/// @return The first child container whose name equals the provided one 
	Container getChildByName(const char* childName, bool recursive);

	/// Retrieve the child at the provided position in the linked list of children.
	/// @param position: Position in the linked list of children
	/// @return The child container at the provided position if any
	Container getChildAtPosition(int16 position);

	/// Retrieve the amount of children of this container.
	/// @return Amount of children of this container
	int32 getChildrenCount();
	
	/// Update the children of this container.
	void updateChildren();

	/// Invalidate the transformation to force it to be recomputed in the next
	/// game cycle.
	void invalidateTransformation();

	/// Set the flags that determine which of the transformation's components must
	/// inherit those from this container's parent's
	/// @param inheritEnvironment: Transformation inheritance flags
	void setInheritEnvironment(uint8 inheritEnvironment);

	/// Apply the transformations to this container's children.
	/// @param invalidateTransformationFlag: Flag that determines which transfomation's components 
	/// must be recomputed
	void transformChildren(uint8 invalidateTransformationFlag);

	/// Propagate an integer message through the whole parenting hierarchy (children, grand children, etc.).
	/// @param propagatedMessageHandler: Method that handles the message
	/// @return The result that the provided message handler returns
	bool propagateMessage(bool (*propagatedMessageHandler)(void*, va_list), ...);

	/// Generic integer message propagator
	/// @param args: Variable list of propagated arguments
	bool onPropagatedMessage(va_list args);

	/// Propagate a string through the whole parenting hierarchy (children, grand children, etc.).
	/// @param propagatedMessageHandler: Method that handles the string
	/// @return The result that the provided string handler returns
	bool propagateString(bool (*propagatedMessageHandler)(void*, va_list), ...);

	/// Generic string propagator
	/// @param args: Variable list of propagated arguments
	bool onPropagatedString(va_list args);

	/// Displace the container.
	/// @param translation: Displacement to be added to the container's position
	void translate(const Vector3D* translation);

	/// Rotate the container.
	/// @param rotation: Rotation to be added to the container's rotation
	void rotate(const Rotation* rotation);

	/// Scale the container.
	/// @param scale: Scale to be applied to the container's scale
	void scale(const Scale* scale);

	/// Retrieve the local position.
	/// @return Pointer to the local transformation's position
	const Vector3D* getLocalPosition();

	/// Retrieve the local rotation.
	/// @return Pointer to the local transformation's rotation
	const Rotation* getLocalRotation();

	/// Retrieve the local scale.
	/// @return Pointer to the local transformation's scale
	const Scale* getLocalScale();

	/// Set the local position.
	/// @param position: New local position
	virtual void setLocalPosition(const Vector3D* position);

	/// Set the local rotation.
	/// @param rotation: New local rotation
	virtual void setLocalRotation(const Rotation* rotation);

	/// Set the local scale.
	/// @param scale: New local scale
	virtual void setLocalScale(const Scale* scale);

	/// Update the local transformation in function of the provided environment transform.
	/// @param environmentTransform: New reference environment for the local transformation
	virtual void changeEnvironment(Transformation* environmentTransform);

	/// Create the components that must attach to this container. 	
	virtual void createComponents();

	/// Destroy the components that attach to this container. 	
	virtual void destroyComponents();

	/// Make the container ready to start operating once it has been completely intialized.
	/// @param recursive: If true, the ready call is propagated to its children, grand children, etc.
	virtual void ready(bool recursive);

	/// Make this instance visible.
	virtual void show();

	/// Make this instance invisible.
	virtual void hide();

	/// Compute the container's global transformation.
	/// @param environmentTransform: Reference environment for the local transformation
	/// @param invalidateTransformationFlag: Flag that determines which transfomation's components 
	/// must be recomputed
	virtual void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);

	/// Update this instance's logic.
	virtual void update();

	/// Prepare to suspend this instance's logic.
	virtual void suspend();

	/// Prepare to resume this instance's logic.
	virtual void resume();

	/// Set this instance's transparency effects.
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	virtual void setTransparency(uint8 transparency);

	/// Default interger message handler for propagateMessage
	/// @param message: Propagated integer message
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	virtual bool handlePropagatedMessage(int32 message);

	/// Default string handler for propagateString
	/// @param string: Propagated string
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	virtual bool handlePropagatedString(const char* string);
}


#endif
