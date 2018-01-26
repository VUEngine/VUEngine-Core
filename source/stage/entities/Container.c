/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <string.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Container
 * @extends SpatialObject
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(Container, SpatialObject);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Container_applyEnvironmentToPosition(Container this, const Transformation* environmentTransform);
static void Container_applyEnvironmentToRotation(Container this, const Transformation* environmentTransform);
static void Container_applyEnvironmentToScale(Container this, const Transformation* environmentTransform);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Container, const char* const name)
__CLASS_NEW_END(Container, name);

// class's constructor
void Container_constructor(Container this, const char* const name)
{
	ASSERT(this, "Container::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(SpatialObject);

	// set position
	this->transformation.localPosition = (Vector3D){0, 0, 0};
	this->transformation.globalPosition = (Vector3D){0, 0, 0};

	// set rotation
	this->transformation.localRotation = (Rotation){0, 0, 0};
	this->transformation.globalRotation = (Rotation){0, 0, 0};

	// set scale
	this->transformation.localScale = (Scale){__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
	this->transformation.globalScale = (Scale){__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};

	// force global position calculation on the next transformation cycle
	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;

	this->parent = NULL;
	this->children = NULL;
	this->removedChildren = NULL;
	this->deleteMe = false;
	this->hidden = false;

	this->name = NULL;
	Container_setName(this, name);
}

// class's destructor
void Container_destructor(Container this)
{
	ASSERT(this, "Container::destructor: null this");

	// first remove any children removed
	Container_processRemovedChildren(this);

	// if I have children
	if(this->children)
	{
		// create a temporary children list
		VirtualNode node = this->children->head;

		// destroy each child
		for(; node ; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

#ifdef __DEBUG
			if(child->parent != this)
			{
				Printing_text(Printing_getInstance(), "Me: ", 1, 15, NULL);
				Printing_text(Printing_getInstance(), __GET_CLASS_NAME(this), 5, 15, NULL);
				Printing_text(Printing_getInstance(), "It: ", 1, 16, NULL);
				Printing_text(Printing_getInstance(), child ? __GET_CLASS_NAME(child) : "NULL", 5, 16, NULL);
			}
#endif
			ASSERT(child->parent == this, "Container::destructor: deleting a child of not mine");
			child->parent = NULL;
			__DELETE(child);
		}

		// delete children list
		__DELETE(this->children);
		this->children = NULL;

	}

	// first remove from parent
	if(this->parent)
	{
		// don't allow my parent to try to delete me again
		Container_removeChild(this->parent, this, false);
	}

	// delete name
	if(this->name)
	{
		__DELETE_BASIC(this->name);
	}

	if(this->events)
	{
		Object_fireEvent(__SAFE_CAST(Object, this), kEventContainerDeleted);
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// safe call to delete entities within a normal stage
void Container_deleteMyself(Container this)
{
	if(this->parent)
	{
		this->deleteMe = true;
		__VIRTUAL_CALL(Container, removeChild, this->parent, this);
		__VIRTUAL_CALL(Container, releaseGraphics, this);
	}
	else
	{
		Printing_text(Printing_getInstance(), __GET_CLASS_NAME_UNSAFE(this), 1, 15, NULL);
		NM_ASSERT(false, "Container::deleteMyself: I'm orphan");
	}
}

// add a child Container
void Container_addChild(Container this, Container child)
{
	ASSERT(this, "Container::addChild: null this");

	// check if child is valid
	if(!child)
	{
		ASSERT(false, "Container::addChild: adding null child");
		return;
	}

	// if don't have any child yet
	if(!this->children)
	{
		// create children list
		this->children = __NEW(VirtualList);
	}

	// first remove from previous parent
	if(this != child->parent)
	{
		if(child->parent)
		{
			Container_removeChild(child->parent, child, false);

			__VIRTUAL_CALL(Container, changeEnvironment, child, &this->transformation);
		}

		// set new parent
		child->parent = this;

		// add to the children list
		VirtualList_pushBack(this->children, (void*)child);

		if(this->removedChildren)
		{
			// make sure it is not up for removal
			VirtualList_removeElement(this->removedChildren, child);
		}

		Container_invalidateGlobalTransformation(child);
	}
}

// remove child Container
void Container_removeChild(Container this, Container child, bool deleteChild)
{
	ASSERT(this, "Container::removeChild: null this");

	// check if child is valid and if I'm its parent
	if(!(child && this == child->parent && this->children))
	{
		return;
	}

	// if don't have any children to remove yet
	if(!this->removedChildren)
	{
		// create children list
		this->removedChildren = __NEW(VirtualList);
	}

	if(!VirtualList_find(this->removedChildren, child))
	{
		// register for removing
		VirtualList_pushBack(this->removedChildren, child);

		// set no parent
		child->parent = NULL;
		child->deleteMe = deleteChild;
	}
}

void Container_setupGraphics(Container this __attribute__ ((unused)))
{
	ASSERT(this, "Container::setupGraphics: null this");

	// if I have children
	if(this->children)
	{
		// first remove children
		Container_processRemovedChildren(this);

		VirtualNode node = this->children->head;

		// update each child
		for(; node ; node = node->next)
		{
			__VIRTUAL_CALL(Container, setupGraphics, node->data);
		}
	}
}

void Container_releaseGraphics(Container this)
{
	ASSERT(this, "Container::releaseGraphics: null this");

	// if I have children
	if(this->children)
	{
		// first remove children
		Container_processRemovedChildren(this);

		VirtualNode node = this->children->head;

		// update each child
		for(; node ; node = node->next)
		{
			__VIRTUAL_CALL(Container, releaseGraphics, node->data);
		}
	}
}

// process removed children
void Container_processRemovedChildren(Container this)
{
	ASSERT(this, "Container::processRemovedChildren: null this");

	if(!this->removedChildren)
	{
		return;
	}

	ASSERT(this->children, "Container::processRemovedChildren: null children list");

	VirtualNode node = this->removedChildren->head;

	// remove each child
	for(; node ; node = node->next)
	{
#ifndef __RELEASE
		if(!__IS_OBJECT_ALIVE(node->data))
		{
			Printing_setDebugMode(Printing_getInstance());
			Printing_text(Printing_getInstance(), "Object's address: ", 1, 15, NULL);
			Printing_hex(Printing_getInstance(), (u32)this, 18, 15, 8, NULL);
			Printing_text(Printing_getInstance(), "Object's type: ", 1, 16, NULL);
			Printing_text(Printing_getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

			NM_ASSERT(false, "Container::processRemovedChildren: deleted children");
		}
#endif
		Container child = __SAFE_CAST(Container, node->data);

		VirtualList_removeElement(this->children, child);

		if(child->deleteMe)
		{
			child->parent = NULL;
			__DELETE(child);
		}
	}

	__DELETE(this->removedChildren);
	this->removedChildren = NULL;
}

// update each Container's child
void Container_update(Container this, u32 elapsedTime)
{
	ASSERT(this, "Container::update: null this");

	// if I have children
	if(this->children)
	{
		// first remove children
		Container_processRemovedChildren(this);

		VirtualNode node = this->children->head;

		// update each child
		for(; node ; node = node->next)
		{
			__VIRTUAL_CALL(Container, update, node->data, elapsedTime);
		}
	}
}

// retrieve environment transformation
Transformation Container_getEnvironmentTransform(Container this)
{
	ASSERT(this, "Container::getEnvironmentTransform: null this");

	if(!this->parent)
	{
		Transformation environmentTransform =
		{
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// local rotation
				{0, 0, 0},
				// global rotation
				{0, 0, 0},
				// local scale
				{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},
				// global scale
				{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9}
		};

		return environmentTransform;
	}

	return this->parent->transformation;
}

// contatenate transformation
void Container_concatenateTransform(Transformation* concatenatedTransformation, Transformation* transformation)
{
	ASSERT(concatenatedTransformation, "Container::concatenateTransform: null concatenatedTransformation");
	ASSERT(transformation, "Container::concatenateTransform: null transformation");

	// tranlate position
	concatenatedTransformation->globalPosition.x += transformation->localPosition.x;
	concatenatedTransformation->globalPosition.y += transformation->localPosition.y;
	concatenatedTransformation->globalPosition.z += transformation->localPosition.z;

	// propagate rotation
	concatenatedTransformation->globalRotation.x += transformation->localRotation.x;
	concatenatedTransformation->globalRotation.y += transformation->localRotation.y;
	concatenatedTransformation->globalRotation.z += transformation->localRotation.z;

	// propagate scale
	concatenatedTransformation->globalScale.x = __FIX7_9_MULT(concatenatedTransformation->globalScale.x, transformation->localScale.x);
	concatenatedTransformation->globalScale.y = __FIX7_9_MULT(concatenatedTransformation->globalScale.y, transformation->localScale.y);
	concatenatedTransformation->globalScale.z = __FIX7_9_MULT(concatenatedTransformation->globalScale.z, transformation->localScale.z);
}

// change environment
void Container_changeEnvironment(Container this, Transformation* environmentTransform)
{
	ASSERT(this, "Container::changeEnvironment: null this");

	Vector3D localPosition =
	{
		this->transformation.globalPosition.x - environmentTransform->globalPosition.x,
		this->transformation.globalPosition.y - environmentTransform->globalPosition.y,
		this->transformation.globalPosition.z - environmentTransform->globalPosition.z,
	};

	Rotation localRotation =
	{
		this->transformation.globalRotation.x - environmentTransform->globalRotation.x,
		this->transformation.globalRotation.y - environmentTransform->globalRotation.y,
		this->transformation.globalRotation.z - environmentTransform->globalRotation.z,
	};

	Scale localScale =
	{
		__FIX7_9_DIV(this->transformation.globalScale.x, environmentTransform->globalScale.x),
		__FIX7_9_DIV(this->transformation.globalScale.y, environmentTransform->globalScale.y),
		__FIX7_9_DIV(this->transformation.globalScale.z, environmentTransform->globalScale.z),
	};

	Container_setLocalPosition(this, &localPosition);
	Container_setLocalRotation(this, &localRotation);
	Container_setLocalScale(this, &localScale);

	// force global position calculation on the next transformation cycle
	Container_invalidateGlobalTransformation(this);
}

// initial transformation
void Container_initialTransform(Container this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "Container::initialTransform: null this");

	// concatenate transformation
	Container_applyEnvironmentToPosition(this, environmentTransform);
	Container_applyEnvironmentToRotation(this, environmentTransform);
	Container_applyEnvironmentToScale(this, environmentTransform);

	Container_invalidateGlobalTransformation(this);

	// if I have children
	if(recursive && this->children)
	{
		// first remove children
		Container_processRemovedChildren(this);

		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			__VIRTUAL_CALL(Container, initialTransform, child, &this->transformation, true);
		}
	}
}

void Container_applyEnvironmentToTransformation(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::applyEnvironmentToTranformation: null this");

	Container_applyEnvironmentToPosition(this, environmentTransform);
	Container_applyEnvironmentToRotation(this, environmentTransform);
	Container_applyEnvironmentToScale(this, environmentTransform);
}

inline static void Container_applyEnvironmentToPosition(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::applyEnvironmentToTranformation: null this");

	Vector3D globalPosition = environmentTransform->globalPosition;
	Vector3D localPosition = this->transformation.localPosition;

	// propagate position
	globalPosition.x += localPosition.x;
	globalPosition.y += localPosition.y;
	globalPosition.z += localPosition.z;

	this->transformation.globalPosition = globalPosition;
}

inline static void Container_applyEnvironmentToRotation(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::applyEnvironmentToRotation: null this");

	Rotation globalRotation = environmentTransform->globalRotation;
	Rotation localRotation = this->transformation.localRotation;

	// propagate position
	globalRotation.x += localRotation.x;
	globalRotation.y += localRotation.y;
	globalRotation.z += localRotation.z;

	this->transformation.globalRotation = globalRotation;
}

inline static void Container_applyEnvironmentToScale(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::applyEnvironmentToScale: null this");

	Scale globalScale = environmentTransform->globalScale;
	Scale localScale = this->transformation.localScale;

	// propagate scale
	globalScale.x = __FIX7_9_MULT(globalScale.x, localScale.x);
	globalScale.y = __FIX7_9_MULT(globalScale.y, localScale.y);
	globalScale.z = __FIX7_9_MULT(globalScale.z, localScale.z);

	this->transformation.globalScale = globalScale;
}

// initial transformation but don't call the virtual method
void Container_transformNonVirtual(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::transformNonVirtual: null this");

	// apply environment transformation
	if(__INVALIDATE_POSITION & this->invalidateGlobalTransformation)
	{
		Container_applyEnvironmentToPosition(this, environmentTransform);
	}

	if(__INVALIDATE_ROTATION & this->invalidateGlobalTransformation)
	{
		Container_applyEnvironmentToRotation(this, environmentTransform);
	}

	if(__INVALIDATE_SCALE & this->invalidateGlobalTransformation)
	{
		Container_applyEnvironmentToScale(this, environmentTransform);
	}

	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			Container_transformNonVirtual(child, &this->transformation);
		}
	}

	// don't update position on next transformation cycle
	this->invalidateGlobalTransformation = 0;
}

// initial transformation
void Container_transform(Container this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "Container::transform: null this");
	ASSERT(environmentTransform, "Container::transform: null environmentTransform");

	// apply environment transformation
	if(__INVALIDATE_POSITION & this->invalidateGlobalTransformation)
	{
		Container_applyEnvironmentToPosition(this, environmentTransform);
	}

	if(__INVALIDATE_ROTATION & this->invalidateGlobalTransformation)
	{
		Container_applyEnvironmentToRotation(this, environmentTransform);
	}

	if(__INVALIDATE_SCALE & this->invalidateGlobalTransformation)
	{
		Container_applyEnvironmentToScale(this, environmentTransform);
	}

	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			__VIRTUAL_CALL(Container, transform, child, &this->transformation, invalidateTransformationFlag);
		}
	}

	// don't update position on next transformation cycle
	this->invalidateGlobalTransformation = 0;
}

void Container_synchronizeGraphics(Container this)
{
	ASSERT(this, "Container::synchronizeGraphics: null this");

	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(Container, synchronizeGraphics, node->data);
		}
	}
}

// retrieve transformation
Transformation* Container_getTransform(Container this)
{
	ASSERT(this, "Container::getTransform: null this");

	return &this->transformation;
}

// retrieve global position
const Vector3D* Container_getGlobalPosition(Container this)
{
	ASSERT(this, "Container::getGlobalPosition: null this");

	return &this->transformation.globalPosition;
}

// retrieve local position
const Vector3D* Container_getLocalPosition(Container this)
{
	ASSERT(this, "Container::getLocalPosition: null this");

	return &this->transformation.localPosition;
}

//set class's local position
void Container_setLocalPosition(Container this, const Vector3D* position)
{
	ASSERT(this, "Container::setLocalPosition: null this");

	// force global position calculation on the next transformation cycle
	if(this->transformation.localPosition.z != position->z)
	{
		Container_invalidateGlobalPosition(this);
		Container_invalidateGlobalScale(this);
	}
	else if(this->transformation.localPosition.x != position->x)
	{
		Container_invalidateGlobalPosition(this);
	}
	else if(this->transformation.localPosition.y != position->y)
	{
		Container_invalidateGlobalPosition(this);
	}

	this->transformation.localPosition = *position;
}

const Rotation* Container_getLocalRotation(Container this)
{
	ASSERT(this, "Container::getLocalRotation: null this");

	return &this->transformation.localRotation;
}

//set class's local position
void Container_setLocalRotation(Container this, const Rotation* rotation)
{
	ASSERT(this, "Container::setLocalRotation: null this");

	this->transformation.localRotation = *rotation;

	Container_invalidateGlobalRotation(this);
}

const Scale* Container_getLocalScale(Container this)
{
	ASSERT(this, "Container::getLocalScale: null this");

	return &this->transformation.localScale;
}

//set class's local position
void Container_setLocalScale(Container this, const Scale* scale)
{
	ASSERT(this, "Container::invalidateGlobalTransformation: null this");

	this->transformation.localScale = *scale;

	Container_invalidateGlobalScale(this);
}

void Container_invalidateGlobalTransformation(Container this)
{
	ASSERT(this, "Container::invalidateGlobalTransformation: null this");

	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure children recalculates its global position
			Container_invalidateGlobalTransformation(__SAFE_CAST(Container, node->data));
		}
	}
}

// invalidate global position
void Container_invalidateGlobalPosition(Container this)
{
	ASSERT(this, "Container::invalidateGlobalPosition: null this");

	this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure children recalculates its global position
			Container_invalidateGlobalPosition(__SAFE_CAST(Container, node->data));
		}
	}
}

// invalidate global rotation
void Container_invalidateGlobalRotation(Container this)
{
	ASSERT(this, "Container::invalidateGlobalRotation: null this");

	this->invalidateGlobalTransformation |= __INVALIDATE_ROTATION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure children recalculates its global position
			Container_invalidateGlobalRotation(__SAFE_CAST(Container, node->data));
		}
	}
}

// invalidate global scale
void Container_invalidateGlobalScale(Container this)
{
	ASSERT(this, "Container::invalidateGlobalScale: null this");

	this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure children recalculates its global position
			Container_invalidateGlobalScale(__SAFE_CAST(Container, node->data));
		}
	}
}

// propagate a message to the children wrapper
int Container_propagateMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), ...)
{
	ASSERT(this, "Container::propagateMessage: null this");
	ASSERT(propagatedMessageHandler, "Container::propagateMessage: null propagatedMessageHandler");

	va_list args;
	va_start(args, propagatedMessageHandler);
	int result = __VIRTUAL_CALL(Container, passMessage, this, propagatedMessageHandler, args);
	va_end(args);

	return result;
}

// pass message to children recursively
int Container_passMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args)
{
	ASSERT(this, "Container::passMessage: null this");

	// if message is valid
	if(!propagatedMessageHandler)
	{
		return false;
	}

	// propagate if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node ; node = node->next)
		{
			// pass message to each child
			if(__VIRTUAL_CALL(Container, passMessage, node->data, propagatedMessageHandler, args))
			{
				return true;
			}
		}
	}

	// if no child processed the message, I process it
	return propagatedMessageHandler(this, args);
}

// process user input
int Container_onPropagatedMessage(Container this, va_list args)
{
	ASSERT(this, "Container::onPropagatedMessage: null this");

	int message = va_arg(args, int);
	return __VIRTUAL_CALL(Container, handlePropagatedMessage, this, message);
}

// process message
bool Container_handlePropagatedMessage(Container this __attribute__ ((unused)), int message __attribute__ ((unused)))
{
	ASSERT(this, "Container::handlePropagatedMessage: null this");

	return false;
}

// retrieve parent
Container Container_getParent(Container this)
{
	ASSERT(this, "Container::getParent: null this");

	return this->parent;
}

// retrieve child count
int Container_getChildCount(Container this)
{
	ASSERT(this, "Container::getChildCount: null this");

	return this->children ? VirtualList_getSize(this->children) : 0;
}

// retrieve children
VirtualList Container_getChildren(Container this)
{
	ASSERT(this, "Container::getChildren: null this");

	return this->children;
}

// set name
void Container_setName(Container this, const char* const name)
{
	ASSERT(this, "Container::setName: null this");

	if(this->name)
	{
		__DELETE_BASIC(this->name);
	}

	if(!name)
	{
		return;
	}

	typedef struct NameWrapper
	{
		char name[__MAX_CONTAINER_NAME_LENGTH];

	} NameWrapper;

	NameWrapper* nameWrapper = (NameWrapper*)__NEW_BASIC(NameWrapper);
	this->name = nameWrapper->name;

	strncpy(this->name, name, __MAX_CONTAINER_NAME_LENGTH);
}

// get name
char* Container_getName(Container this)
{
	ASSERT(this, "Container::getName: null this");

	return this->name;
}

// find child by name in given list
static Container Container_findChildByName(Container this, VirtualList children, char* childName, bool recursive)
{
	ASSERT(this, "Container::findChildByName: null this");

	if(this->deleteMe)
	{
		return NULL;
	}

	// first remove children
	Container_processRemovedChildren(this);

	Container child, grandChild;
	VirtualNode node = children->head;

	// look through all children
	for(; node ; node = node->next)
	{
		child = __SAFE_CAST(Container, node->data);

		if(child->name && !strncmp(childName, child->name, __MAX_CONTAINER_NAME_LENGTH))
		{
			return child;
		}
		else if(recursive && child->children)
		{
			grandChild = Container_findChildByName(this, child->children, childName, recursive);
			if(grandChild)
			{
				return grandChild;
			}
		}
	}

	return NULL;
}

// get child by name
Container Container_getChildByName(Container this, char* childName, bool recursive)
{
	ASSERT(this, "Container::getChildByName: null this");

	Container foundChild = NULL;

	if(!this->deleteMe && childName && this->children)
	{
		// search through direct children
		foundChild = Container_findChildByName(this, this->children, childName, false);

		// if no direct child could be found, do a recursive search, if applicable
		if(!foundChild && recursive)
		{
			foundChild = Container_findChildByName(this, this->children, childName, true);
		}
	}

	return foundChild;
}



// suspend for pause
void Container_suspend(Container this)
{
	ASSERT(this, "Container::suspend: null this");

	if(this->children)
	{
		Container_processRemovedChildren(this);

		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			__VIRTUAL_CALL(Container, suspend, child);
		}
	}
}

// resume after pause
void Container_resume(Container this)
{
	ASSERT(this, "Container::resume: null this");

	if(this->children)
	{
		Container_processRemovedChildren(this);

		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			__VIRTUAL_CALL(Container, resume, child);
		}
	}

	// force translation recalculations
	Container_invalidateGlobalTransformation(this);
}

void Container_show(Container this)
{
	ASSERT(this, "Container::show: null this");

	this->hidden = false;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			__VIRTUAL_CALL(Container, show, child);
		}
	}

	Container_invalidateGlobalTransformation(this);
}

void Container_hide(Container this)
{
	ASSERT(this, "Container::hide: null this");

	this->hidden = true;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(Container, hide, node->data);
		}
	}
}

bool Container_isHidden(Container this)
{
	ASSERT(this, "Container::isHidden: null this");

	return this->hidden;
}
