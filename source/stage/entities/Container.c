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

/**
 * @class	Container
 * @extends SpatialObject
 * @ingroup stage-entities
 */
implements Container : SpatialObject;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Container::applyEnvironmentToPosition(Container this, const Transformation* environmentTransform);
static void Container::applyEnvironmentToRotation(Container this, const Transformation* environmentTransform);
static void Container::applyEnvironmentToScale(Container this, const Transformation* environmentTransform);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Container, const char* const name)
__CLASS_NEW_END(Container, name);


/**
 * Class constructor
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 * @param name
 */
void Container::constructor(Container this, const char* const name)
{
	ASSERT(this, "Container::constructor: null this");

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
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::destructor(Container this)
{
	ASSERT(this, "Container::destructor: null this");

	// first remove any children removed
	if(this->removedChildren)
	{
		__DELETE(this->removedChildren);
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
			Container child = __SAFE_CAST(Container, node->data);
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
			__DELETE(child);
		}

		// delete children list
		__DELETE(this->children);
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
		__DELETE_BASIC(this->name);
	}

	if(this->events)
	{
		Object::fireEvent(__SAFE_CAST(Object, this), kEventContainerDeleted);
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Safe call to delete entities within a normal stage
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::deleteMyself(Container this)
{
	ASSERT(this, "Container::deleteMyself: null this");
	ASSERT(__IS_OBJECT_ALIVE(this), "Container::deleteMyself: deleted this");
	ASSERT(__IS_OBJECT_ALIVE(this->parent), "Container::deleteMyself: deleted parent");

	if(__IS_OBJECT_ALIVE(this->parent))
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

/**
 *
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::iAmDeletingMyself(Container this __attribute__ ((unused)))
{
	ASSERT(this, "Container::iAmDeletingMyself: null this");
}

/**
 * Add a child Container
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 * @param child	Child Container to add
 */
void Container::addChild(Container this, Container child)
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
 * @memberof			Container
 * @public
 *
 * @param this			Function scope
 * @param child			Child Container to remove
 * @param deleteChild
 */
void Container::removeChild(Container this, Container child, bool deleteChild)
{
	ASSERT(this, "Container::removeChild: null this");
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
		this->removedChildren = __NEW(VirtualList);
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

/**
 *
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::setupGraphics(Container this __attribute__ ((unused)))
{
	ASSERT(this, "Container::setupGraphics: null this");

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

/**
 *
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::releaseGraphics(Container this)
{
	ASSERT(this, "Container::releaseGraphics: null this");

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
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::purgeChildren(Container this)
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
			Printing::setDebugMode(Printing::getInstance());
			Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
			Printing::hex(Printing::getInstance(), (u32)this, 18, 15, 8, NULL);
			Printing::text(Printing::getInstance(), "Object's type: ", 1, 16, NULL);
			Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

			NM_ASSERT(false, "Container::processRemovedChildren: deleted children");
		}
#endif
		Container child = __SAFE_CAST(Container, node->data);

		VirtualList::removeElement(this->children, child);

		if(child->deleteMe)
		{
			child->parent = NULL;
			__DELETE(child);
		}
	}

	__DELETE(this->removedChildren);
	this->removedChildren = NULL;
}

/**
 * Update each Container's child
 *
 * @memberof			Container
 * @public
 *
 * @param this			Function scope
 * @param elapsedTime
 */
void Container::update(Container this, u32 elapsedTime)
{
	ASSERT(this, "Container::update: null this");

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
 * @memberof			Container
 * @public
 *
 * @param this			Function scope
 * @param elapsedTime
 *
 * @return				Environment Transformation
 */
Transformation Container::getEnvironmentTransform(Container this)
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

/**
 * Contatenate transformation
 *
 * @memberof							Container
 * @public
 *
 * @param this							Function scope
 * @param concatenatedTransformation
 * @param transformation
 */
void Container::concatenateTransform(Container this __attribute__ ((unused)), Transformation* concatenatedTransformation, Transformation* transformation)
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
 * @memberof					Container
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 */
void Container::changeEnvironment(Container this, Transformation* environmentTransform)
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

	Container::setLocalPosition(this, &localPosition);
	Container::setLocalRotation(this, &localRotation);
	Container::setLocalScale(this, &localScale);

	// force global position calculation on the next transformation cycle
	Container::invalidateGlobalTransformation(this);
}

/**
 * Initial transformation
 *
 * @memberof					Container
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 * @param recursive
 */
void Container::initialTransform(Container this, const Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "Container::initialTransform: null this");

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
			Container child = __SAFE_CAST(Container, node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			 Container::initialTransform(child, &this->transformation, true);
		}
	}
}

/**
 *
 *
 * @memberof					Container
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 */
void Container::applyEnvironmentToTransformation(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::applyEnvironmentToTranformation: null this");

	Container::applyEnvironmentToPosition(this, environmentTransform);
	Container::applyEnvironmentToRotation(this, environmentTransform);
	Container::applyEnvironmentToScale(this, environmentTransform);
}

/**
 *
 *
 * @memberof					Container
 * @private
 *
 * @param this					Function scope
 * @param environmentTransform
 */
inline static void Container::applyEnvironmentToPosition(Container this, const Transformation* environmentTransform)
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


/**
 *
 *
 * @memberof					Container
 * @private
 *
 * @param this					Function scope
 * @param environmentTransform
 */
inline static void Container::applyEnvironmentToRotation(Container this, const Transformation* environmentTransform)
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


/**
 *
 *
 * @memberof					Container
 * @private
 *
 * @param this					Function scope
 * @param environmentTransform
 */
inline static void Container::applyEnvironmentToScale(Container this, const Transformation* environmentTransform)
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

/**
 * Initial transformation but without calling the virtual method
 *
 * @memberof					Container
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 */
void Container::transformNonVirtual(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::transformNonVirtual: null this");

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
			Container child = __SAFE_CAST(Container, node->data);

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
 * @memberof							Container
 * @public
 *
 * @param this							Function scope
 * @param environmentTransform
 * @param invalidateTransformationFlag
 */
void Container::transform(Container this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "Container::transform: null this");
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
			Container child = __SAFE_CAST(Container, node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			 Container::transform(child, &this->transformation, invalidateTransformationFlag);
		}
	}

	// don't update position on next transformation cycle
	this->invalidateGlobalTransformation = 0;
}

/**
 *
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::synchronizeGraphics(Container this)
{
	ASSERT(this, "Container::synchronizeGraphics: null this");

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
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 *
 * @return		Pointer to Transformation
 */
Transformation* Container::getTransform(Container this)
{
	ASSERT(this, "Container::getTransform: null this");

	return &this->transformation;
}

/**
 * Retrieve global position
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 *
 * @return		Pointer to global position
 */
const Vector3D* Container::getGlobalPosition(Container this)
{
	ASSERT(this, "Container::getGlobalPosition: null this");

	return &this->transformation.globalPosition;
}

/**
 * Retrieve local position
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 *
 * @return		Pointer to local position
 */
const Vector3D* Container::getLocalPosition(Container this)
{
	ASSERT(this, "Container::getLocalPosition: null this");

	return &this->transformation.localPosition;
}

/**
 * Set local position
 *
 * @memberof		Container
 * @public
 *
 * @param this		Function scope
 * @param position	Pointer to position
 */
void Container::setLocalPosition(Container this, const Vector3D* position)
{
	ASSERT(this, "Container::setLocalPosition: null this");

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
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 *
 * @return		Pointer to local Rotation
 */
const Rotation* Container::getLocalRotation(Container this)
{
	ASSERT(this, "Container::getLocalRotation: null this");

	return &this->transformation.localRotation;
}

/**
 * Set local rotation
 *
 * @memberof		Container
 * @public
 *
 * @param this		Function scope
 * @param rotation	Pointer to Rotation
 */
void Container::setLocalRotation(Container this, const Rotation* rotation)
{
	ASSERT(this, "Container::setLocalRotation: null this");

	this->transformation.localRotation = *rotation;

	Container::invalidateGlobalRotation(this);
}

/**
 * Retrieve local scale
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 *
 * @return		Pointer to local Scale
 */
const Scale* Container::getLocalScale(Container this)
{
	ASSERT(this, "Container::getLocalScale: null this");

	return &this->transformation.localScale;
}

/**
 * Set local scale
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 * @param scale	Pointer to Scale
 */
void Container::setLocalScale(Container this, const Scale* scale)
{
	ASSERT(this, "Container::invalidateGlobalTransformation: null this");

	this->transformation.localScale = *scale;

	Container::invalidateGlobalScale(this);
}

/**
 * Invalidate global transformation
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::invalidateGlobalTransformation(Container this)
{
	ASSERT(this, "Container::invalidateGlobalTransformation: null this");

	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalTransformation(__SAFE_CAST(Container, node->data));
		}
	}
}

/**
 * Invalidate global position
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::invalidateGlobalPosition(Container this)
{
	ASSERT(this, "Container::invalidateGlobalPosition: null this");

	this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalPosition(__SAFE_CAST(Container, node->data));
		}
	}
}

/**
 * Invalidate global rotation
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::invalidateGlobalRotation(Container this)
{
	ASSERT(this, "Container::invalidateGlobalRotation: null this");

	this->invalidateGlobalTransformation |= __INVALIDATE_ROTATION;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalRotation(__SAFE_CAST(Container, node->data));
		}
	}
}

/**
 * Invalidate global scale
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::invalidateGlobalScale(Container this)
{
	ASSERT(this, "Container::invalidateGlobalScale: null this");

	this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalScale(__SAFE_CAST(Container, node->data));
		}
	}
}

/**
 * Propagate a message to the child wrapper
 *
 * @memberof						Container
 * @public
 *
 * @param this						Function scope
 * @param propagatedMessageHandler
 * @param args						va_list of propagated message parameters

 * @return							Result
 */
int Container::propagateMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), ...)
{
	ASSERT(this, "Container::propagateMessage: null this");
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
 * @memberof						Container
 * @public
 *
 * @param this						Function scope
 * @param propagatedMessageHandler
 * @param args						va_list of propagated message parameters

 * @return							Result
 */
int Container::passMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args)
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
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 * @param args	va_list of propagated message parameters

 * @return		Result
 */
int Container::onPropagatedMessage(Container this, va_list args)
{
	ASSERT(this, "Container::onPropagatedMessage: null this");

	int message = va_arg(args, int);
	return  Container::handlePropagatedMessage(this, message);
}

/**
 * Process message
 *
 * @memberof		Container
 * @public
 *
 * @param this		Function scope
 * @param message	Message

 * @return			Result
 */
bool Container::handlePropagatedMessage(Container this __attribute__ ((unused)), int message __attribute__ ((unused)))
{
	ASSERT(this, "Container::handlePropagatedMessage: null this");

	return false;
}

/**
 * Retrieve parent
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope

 * @return		Parent Container
 */
Container Container::getParent(Container this)
{
	ASSERT(this, "Container::getParent: null this");

	return this->parent;
}

/**
 * Retrieve children count
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope

 * @return		Children count
 */
int Container::getChildCount(Container this)
{
	ASSERT(this, "Container::getChildCount: null this");

	return this->children ? VirtualList::getSize(this->children) : 0;
}

/**
 * Set name
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 * @param name	Name
 */
void Container::setName(Container this, const char* const name)
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

/**
 * Retrieve name
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 *
 * @return		Name
 */
char* Container::getName(Container this)
{
	ASSERT(this, "Container::getName: null this");

	return this->name;
}

/**
 * Find child by name in given list
 *
 * @memberof		Container
 * @public
 *
 * @param this		Function scope
 * @param children	List to search
 * @param childName	Name of child to search for
 * @param recursive	Whether to search recursively
 *
 * @return			Child Container
 */
static Container Container::findChildByName(Container this, VirtualList children, char* childName, bool recursive)
{
	ASSERT(this, "Container::findChildByName: null this");

	if(this->deleteMe)
	{
		return NULL;
	}

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
 * @memberof		Container
 * @public
 *
 * @param this		Function scope
 * @param childName	Name of child to search for
 * @param recursive	Whether to search recursively
 *
 * @return			Child Container
 */
Container Container::getChildByName(Container this, char* childName, bool recursive)
{
	ASSERT(this, "Container::getChildByName: null this");

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
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::suspend(Container this)
{
	ASSERT(this, "Container::suspend: null this");

	if(this->children)
	{
		Container::purgeChildren(this);

		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			 Container::suspend(child);
		}
	}
}

/**
 * Resume after pause
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::resume(Container this)
{
	ASSERT(this, "Container::resume: null this");

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			Container child = __SAFE_CAST(Container, node->data);

			 Container::resume(child);
		}
	}

	// force translation recalculations
	Container::invalidateGlobalTransformation(this);
}

/**
 *
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::show(Container this)
{
	ASSERT(this, "Container::show: null this");

	this->hidden = false;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			 Container::show(__SAFE_CAST(Container, node->data));
		}
	}

	Container::invalidateGlobalTransformation(this);
}

/**
 *
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 */
void Container::hide(Container this)
{
	ASSERT(this, "Container::hide: null this");

	this->hidden = true;

	if(this->children)
	{
		VirtualNode node = this->children->head;

		for(; node; node = node->next)
		{
			 Container::hide(__SAFE_CAST(Container, node->data));
		}
	}
}

/**
 *
 *
 * @memberof	Container
 * @public
 *
 * @param this	Function scope
 *
 * @return		Where Container is hidden
 */
bool Container::isHidden(Container this)
{
	ASSERT(this, "Container::isHidden: null this");

	return this->hidden;
}
