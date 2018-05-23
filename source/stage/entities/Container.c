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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <string.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param name
 */
void Container::constructor(const char* const name)
{
	// construct base object
	Base::constructor();

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
	Container::setName(this, name);
}

/**
 * Class destructor
 */
void Container::destructor()
{
	// first remove any children removed
	if(this->removedChildren)
	{
		delete this->removedChildren;
		this->removedChildren = NULL;
	}

	// if I have children
	if(this->children)
	{
		// create a temporary children list
		VirtualNode node = this->children->head;

		// destroy each child
		for(; node ; node = node->next)
		{
			Container child = Container::safeCast(node->data);
/*
#ifdef __DEBUG
			if(child->parent != this)
			{
				Printing::text(Printing::getInstance(), "Me: ", 1, 15, NULL);
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 5, 15, NULL);
				Printing::text(Printing::getInstance(), "It: ", 1, 16, NULL);
				Printing::text(Printing::getInstance(), child ? __GET_CLASS_NAME(child) : "NULL", 5, 16, NULL);
			}
#endif
			ASSERT(child->parent == this, "Container::destructor: deleting a child of not mine");
*/
			child->parent = NULL;
			delete child;
		}

		// delete children list
		delete this->children;
		this->children = NULL;

	}

	// first remove from parent
	if(this->parent)
	{
		ASSERT(this != this->parent, "Container::destructor: I'm my own father");
		// don't allow my parent to try to delete me again
		Container::removeChild(this->parent, this, false);
	}

	// delete name
	if(this->name)
	{
		delete this->name;
	}

	if(this->events)
	{
		Object::fireEvent(this, kEventContainerDeleted);
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Safe call to delete entities within a normal stage
 */
void Container::deleteMyself()
{
	ASSERT(!isDeleted(this), "Container::deleteMyself: deleted this");
	ASSERT(!isDeleted(this->parent), "Container::deleteMyself: deleted parent");

	if(!isDeleted(this->parent))
	{
		 Container::removeChild(this->parent, this, true);
		 Container::iAmDeletingMyself(this);
		 Container::releaseGraphics(this);
	}
	else
	{
		//Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this), 1, 15, NULL);
		NM_ASSERT(false, "Container::deleteMyself: I'm orphan");
	}
}

void Container::iAmDeletingMyself()
{}

/**
 * Add a child Container
 *
 * @param child	Child Container to add
 */
void Container::addChild(Container child)
{
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
		this->children = new VirtualList();
	}

	// first remove from previous parent
	if(this != child->parent)
	{
		if(child->parent)
		{
			Container::removeChild(child->parent, child, false);

			Container::changeEnvironment(child, &this->transformation);
		}

		// set new parent
		child->parent = this;

		// add to the children list
		VirtualList::pushBack(this->children, (void*)child);

		if(this->removedChildren)
		{
			// make sure it is not up for removal
			VirtualList::removeElement(this->removedChildren, child);
		}

		Container::invalidateGlobalTransformation(child);
	}
}

/**
 * Remove a child Container
 *
 * @param child			Child Container to remove
 * @param deleteChild
 */
void Container::removeChild(Container child, bool deleteChild)
{
	ASSERT(this == child->parent, "Container::removeChild: not my child");

	// check if child is valid and if I'm its parent
	if(!(child && this == child->parent && this->children))
	{
		return;
	}

	// if don't have any children to remove yet
	if(!this->removedChildren)
	{
		// create children list
		this->removedChildren = new VirtualList();
	}

	if(!VirtualList::find(this->removedChildren, child))
	{
		// register for removing
		VirtualList::pushBack(this->removedChildren, child);

		// set no parent
		child->parent = NULL;
		child->deleteMe = deleteChild;
	}
#ifndef __RELEASE
	else
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (u32)this, 18, 15, 8, NULL);
		Printing::text(Printing::getInstance(), "Object's type: ", 1, 16, NULL);
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

		NM_ASSERT(false, "Container::removeChild: removing child twice");
	}
#endif
}

void Container::setupGraphics()
{
	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node ; node = node->next)
		{
			 Container::setupGraphics(node->data);
		}
	}
}

void Container::releaseGraphics()
{
	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node ; node = node->next)
		{
			 Container::releaseGraphics(node->data);
		}
	}
}

/**
 * Process removed children
 */
void Container::purgeChildren()
{
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
		if(isDeleted(node->data))
		{
			Printing::setDebugMode(Printing::getInstance());
			Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
			Printing::hex(Printing::getInstance(), (u32)this, 18, 15, 8, NULL);
			Printing::text(Printing::getInstance(), "Object's type: ", 1, 16, NULL);
			Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

			NM_ASSERT(false, "Container::processRemovedChildren: deleted children");
		}
#endif
		Container child = Container::safeCast(node->data);

		VirtualList::removeElement(this->children, child);

		if(child->deleteMe)
		{
			child->parent = NULL;
			delete child;
		}
	}

	delete this->removedChildren;
	this->removedChildren = NULL;
}

/**
 * Update each Container's child
 *
 * @param elapsedTime
 */
void Container::update(u32 elapsedTime)
{
	// if I have children
	if(this->children)
	{
		// first remove children
		Container::purgeChildren(this);

		VirtualNode node = this->children->head;

		// update each child
		for(; node ; node = node->next)
		{
			 Container::update(node->data, elapsedTime);
		}
	}
}

/**
 * Retrieve environment transformation
 *
 * @param elapsedTime
 * @return				Environment Transformation
 */
Transformation Container::getEnvironmentTransform()
{
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

/**
 * Contatenate transformation
 *
 * @param concatenatedTransformation
 * @param transformation
 */
void Container::concatenateTransform(Transformation* concatenatedTransformation, Transformation* transformation)
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

/**
 * Change environment
 *
 * @param environmentTransform
 */
void Container::changeEnvironment(Transformation* environmentTransform)
{
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

	Container::setLocalPosition(this, &localPosition);
	Container::setLocalRotation(this, &localRotation);
	Container::setLocalScale(this, &localScale);

	// force global position calculation on the next transformation cycle
	Container::invalidateGlobalTransformation(this);
}

/**
 * Initial transformation
 *
 * @param environmentTransform
 * @param recursive
 */
void Container::initialTransform(const Transformation* environmentTransform, u32 recursive)
{
	// concatenate transformation
	Container::applyEnvironmentToPosition(this, environmentTransform);
	Container::applyEnvironmentToRotation(this, environmentTransform);
	Container::applyEnvironmentToScale(this, environmentTransform);

	Container::invalidateGlobalTransformation(this);

	// if I have children
	if(recursive && this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			 Container::initialTransform(child, &this->transformation, true);
		}
	}
}

/**
 *
 *
 * @param environmentTransform
 */
void Container::applyEnvironmentToTransformation(const Transformation* environmentTransform)
{
	Container::applyEnvironmentToPosition(this, environmentTransform);
	Container::applyEnvironmentToRotation(this, environmentTransform);
	Container::applyEnvironmentToScale(this, environmentTransform);
}

/**
 *
 *
 * @private
 * @param environmentTransform
 */
inline void Container::applyEnvironmentToPosition(const Transformation* environmentTransform)
{
	Vector3D globalPosition = environmentTransform->globalPosition;
	Vector3D localPosition = this->transformation.localPosition;

	// propagate position
	globalPosition.x += localPosition.x;
	globalPosition.y += localPosition.y;
	globalPosition.z += localPosition.z;

	this->transformation.globalPosition = globalPosition;
}


/**
 *
 *
 * @private
 * @param environmentTransform
 */
inline void Container::applyEnvironmentToRotation(const Transformation* environmentTransform)
{
	Rotation globalRotation = environmentTransform->globalRotation;
	Rotation localRotation = this->transformation.localRotation;

	// propagate position
	globalRotation.x += localRotation.x;
	globalRotation.y += localRotation.y;
	globalRotation.z += localRotation.z;

	this->transformation.globalRotation = globalRotation;
}


/**
 *
 *
 * @private
 * @param environmentTransform
 */
inline void Container::applyEnvironmentToScale(const Transformation* environmentTransform)
{
	Scale globalScale = environmentTransform->globalScale;
	Scale localScale = this->transformation.localScale;

	// propagate scale
	globalScale.x = __FIX7_9_MULT(globalScale.x, localScale.x);
	globalScale.y = __FIX7_9_MULT(globalScale.y, localScale.y);
	globalScale.z = __FIX7_9_MULT(globalScale.z, localScale.z);

	this->transformation.globalScale = globalScale;
}

/**
 * Initial transformation but without calling the virtual method
 *
 * @param environmentTransform
 */
void Container::transformNonVirtual(const Transformation* environmentTransform)
{
	// apply environment transformation
	if(__INVALIDATE_POSITION & this->invalidateGlobalTransformation)
	{
		Container::applyEnvironmentToPosition(this, environmentTransform);
	}

	if(__INVALIDATE_ROTATION & this->invalidateGlobalTransformation)
	{
		Container::applyEnvironmentToRotation(this, environmentTransform);
	}

	if(__INVALIDATE_SCALE & this->invalidateGlobalTransformation)
	{
		Container::applyEnvironmentToScale(this, environmentTransform);
	}

	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			Container::transformNonVirtual(child, &this->transformation);
		}
	}

	// don't update position on next transformation cycle
	this->invalidateGlobalTransformation = 0;
}

/**
 * Initial transformation
 *
 * @param environmentTransform
 * @param invalidateTransformationFlag
 */
void Container::transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(environmentTransform, "Container::transform: null environmentTransform");

	// apply environment transformation
	if(__INVALIDATE_POSITION & this->invalidateGlobalTransformation)
	{
		Container::applyEnvironmentToPosition(this, environmentTransform);
	}

	if(__INVALIDATE_ROTATION & this->invalidateGlobalTransformation)
	{
		Container::applyEnvironmentToRotation(this, environmentTransform);
	}

	if(__INVALIDATE_SCALE & this->invalidateGlobalTransformation)
	{
		Container::applyEnvironmentToScale(this, environmentTransform);
	}

	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			 Container::transform(child, &this->transformation, invalidateTransformationFlag);
		}
	}

	// don't update position on next transformation cycle
	this->invalidateGlobalTransformation = 0;
}

void Container::synchronizeGraphics()
{
	// if I have children
	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			 Container::synchronizeGraphics(node->data);
		}
	}
}

/**
 * Retrieve transformation
 *
 * @return		Pointer to Transformation
 */
Transformation* Container::getTransform()
{
	return &this->transformation;
}

/**
 * Retrieve global position
 *
 * @return		Pointer to global position
 */
const Vector3D* Container::getGlobalPosition()
{
	return &this->transformation.globalPosition;
}

/**
 * Retrieve local position
 *
 * @return		Pointer to local position
 */
const Vector3D* Container::getLocalPosition()
{
	return &this->transformation.localPosition;
}

/**
 * Set local position
 *
 * @param position	Pointer to position
 */
void Container::setLocalPosition(const Vector3D* position)
{
	// force global position calculation on the next transformation cycle
	if(this->transformation.localPosition.z != position->z)
	{
		Container::invalidateGlobalPosition(this);
		Container::invalidateGlobalScale(this);
	}
	else if(this->transformation.localPosition.x != position->x)
	{
		Container::invalidateGlobalPosition(this);
	}
	else if(this->transformation.localPosition.y != position->y)
	{
		Container::invalidateGlobalPosition(this);
	}

	this->transformation.localPosition = *position;
}

/**
 * Retrieve local rotation
 *
 * @return		Pointer to local Rotation
 */
const Rotation* Container::getLocalRotation()
{
	return &this->transformation.localRotation;
}

/**
 * Set local rotation
 *
 * @param rotation	Pointer to Rotation
 */
void Container::setLocalRotation(const Rotation* rotation)
{
	this->transformation.localRotation = *rotation;

	Container::invalidateGlobalRotation(this);
}

/**
 * Retrieve local scale
 *
 * @return		Pointer to local Scale
 */
const Scale* Container::getLocalScale()
{
	return &this->transformation.localScale;
}

/**
 * Set local scale
 *
 * @param scale	Pointer to Scale
 */
void Container::setLocalScale(const Scale* scale)
{
	this->transformation.localScale = *scale;

	Container::invalidateGlobalScale(this);
}

/**
 * Invalidate global transformation
 */
void Container::invalidateGlobalTransformation()
{
	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalTransformation(node->data);
		}
	}
}

/**
 * Invalidate global position
 */
void Container::invalidateGlobalPosition()
{
	this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalPosition(node->data);
		}
	}
}

/**
 * Invalidate global rotation
 */
void Container::invalidateGlobalRotation()
{
	this->invalidateGlobalTransformation |= __INVALIDATE_ROTATION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalRotation(node->data);
		}
	}
}

/**
 * Invalidate global scale
 */
void Container::invalidateGlobalScale()
{
	this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalScale(node->data);
		}
	}
}

/**
 * Propagate a message to the child wrapper
 *
 * @param propagatedMessageHandler
 * @param args						va_list of propagated message parameters

 * @return							Result
 */
int Container::propagateMessage(int (*propagatedMessageHandler)(void*, va_list), ...)
{
	ASSERT(propagatedMessageHandler, "Container::propagateMessage: null propagatedMessageHandler");

	va_list args;
	va_start(args, propagatedMessageHandler);
	int result =  Container::passMessage(this, propagatedMessageHandler, args);
	va_end(args);

	return result;
}

/**
 * Pass message to children recursively
 *
 * @param propagatedMessageHandler
 * @param args						va_list of propagated message parameters

 * @return							Result
 */
int Container::passMessage(int (*propagatedMessageHandler)(void*, va_list), va_list args)
{
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
			if( Container::passMessage(node->data, propagatedMessageHandler, args))
			{
				return true;
			}
		}
	}

	// if no child processed the message, I process it
	return propagatedMessageHandler(this, args);
}

/**
 * Process user input
 *
 * @param args	va_list of propagated message parameters

 * @return		Result
 */
int Container::onPropagatedMessage(va_list args)
{
	int message = va_arg(args, int);
	return  Container::handlePropagatedMessage(this, message);
}

/**
 * Process message
 *
 * @param message	Message

 * @return			Result
 */
bool Container::handlePropagatedMessage(int message __attribute__ ((unused)))
{
	return false;
}

/**
 * Retrieve parent
 *

 * @return		Parent Container
 */
Container Container::getParent()
{
	return this->parent;
}

/**
 * Retrieve children count
 *

 * @return		Children count
 */
int Container::getChildCount()
{
	return this->children ? VirtualList::getSize(this->children) : 0;
}

/**
 * Set name
 *
 * @param name	Name
 */
void Container::setName(const char* const name)
{
	if(this->name)
	{
		delete this->name;
	}

	if(!name)
	{
		return;
	}

	typedef struct NameWrapper
	{
		char name[__MAX_CONTAINER_NAME_LENGTH];

	} NameWrapper;

	NameWrapper* nameWrapper = (NameWrapper*)new NameWrapper;
	this->name = nameWrapper->name;

	strncpy(this->name, name, __MAX_CONTAINER_NAME_LENGTH);
}

/**
 * Retrieve name
 *
 * @return		Name
 */
char* Container::getName()
{
	return this->name;
}

/**
 * Find child by name in given list
 *
 * @param children	List to search
 * @param childName	Name of child to search for
 * @param recursive	Whether to search recursively
 * @return			Child Container
 */
Container Container::findChildByName(VirtualList children, char* childName, bool recursive)
{
	if(this->deleteMe)
	{
		return NULL;
	}

	Container child, grandChild;
	VirtualNode node = children->head;

	// look through all children
	for(; node ; node = node->next)
	{
		child = Container::safeCast(node->data);

		if(child->name && !strncmp(childName, child->name, __MAX_CONTAINER_NAME_LENGTH))
		{
			return child;
		}
		else if(recursive && child->children)
		{
			grandChild = Container::findChildByName(this, child->children, childName, recursive);
			if(grandChild)
			{
				return grandChild;
			}
		}
	}

	return NULL;
}

/**
 * Get child by name
 *
 * @param childName	Name of child to search for
 * @param recursive	Whether to search recursively
 * @return			Child Container
 */
Container Container::getChildByName(char* childName, bool recursive)
{
	Container foundChild = NULL;

	if(!this->deleteMe && childName && this->children)
	{
		// search through direct children
		foundChild = Container::findChildByName(this, this->children, childName, false);

		// if no direct child could be found, do a recursive search, if applicable
		if(!foundChild && recursive)
		{
			foundChild = Container::findChildByName(this, this->children, childName, true);
		}
	}

	return this->removedChildren && VirtualList::find(this->removedChildren, foundChild) ? NULL : foundChild;
}

/**
 * Suspend for pause
 */
void Container::suspend()
{
	if(this->children)
	{
		Container::purgeChildren(this);

		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			 Container::suspend(child);
		}
	}
}

/**
 * Resume after pause
 */
void Container::resume()
{
	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			 Container::resume(child);
		}
	}

	// force translation recalculations
	Container::invalidateGlobalTransformation(this);
}

void Container::show()
{
	this->hidden = false;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			 Container::show(node->data);
		}
	}

	Container::invalidateGlobalTransformation(this);
}

void Container::hide()
{
	this->hidden = true;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			 Container::hide(node->data);
		}
	}
}

/**
 *
 *
 * @return		Where Container is hidden
 */
bool Container::isHidden()
{
	return this->hidden;
}
