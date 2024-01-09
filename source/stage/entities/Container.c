/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Behavior.h>
#include <DebugUtilities.h>
#include <Printing.h>
#include <VirtualList.h>

#include "Container.h"


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

	// By default, save on calls to main methods.
	this->update = Container::overrides(this, update);
	this->transform = Container::overrides(this, transform);

	// set position
	this->transformation.localPosition = Vector3D::zero();
	this->transformation.globalPosition = Vector3D::zero();

	// set rotation
	this->transformation.localRotation = Rotation::zero();
	this->transformation.globalRotation = Rotation::zero();

	// set scale
	this->transformation.localScale = Scale::unit();
	this->transformation.globalScale = Scale::unit();

	// force global position calculation on the next transformation cycle
	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;

	this->parent = NULL;
	this->children = NULL;
	this->behaviors = NULL;
	this->deleteMe = false;
	this->hidden = false;
	this->inheritEnvironment = __INHERIT_TRANSFORMATION;
	this->dontStreamOut = false;

	this->name = NULL;
	Container::setName(this, name);
}

/**
 * Class destructor
 */
void Container::destructor()
{
	// if I have children
	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

#ifndef __RELEASE
			if(NULL != child->parent && child->parent != this)
			{
				Printing::setDebugMode(Printing::getInstance());
				Printing::clear(Printing::getInstance());
				Printing::text(Printing::getInstance(), "Me: ", 20, 12, NULL);
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 24, 12, NULL);
				Printing::text(Printing::getInstance(), "It: ", 20, 13, NULL);
				Printing::text(Printing::getInstance(), child ? __GET_CLASS_NAME(child) : "NULL", 24, 13, NULL);
				Printing::hex(Printing::getInstance(), (uint32)child, 29, 14, 8, NULL);
				Printing::text(Printing::getInstance(), "Parent: ", 20, 15, NULL);
				Printing::hex(Printing::getInstance(), (uint32)child->parent, 29, 15, 8, NULL);

				NM_ASSERT(false, "Container::destructor: deleting a child of not mine");
			}
#endif

			child->parent = NULL;
			delete child;
		}

		// delete children list
		delete this->children;
		this->children = NULL;

	}

	if(!isDeleted(this->behaviors))
	{
		VirtualList::deleteData(this->behaviors);
		delete this->behaviors;
		this->behaviors = NULL;
	}

	// first remove from parent
	if(this->parent)
	{
		ASSERT(this != this->parent, "Container::destructor: I'm my own father");
		// don't allow my parent to try to delete me again
		Container::removeChild(this->parent, this, false);
	}

	// delete name
	if(!isDeleted(this->name))
	{
		delete this->name;
	}

	if(!isDeleted(this->events))
	{
		Container::fireEvent(this, kEventContainerDeleted);
		NM_ASSERT(!isDeleted(this), "Container::destructor: deleted this during kEventContainerDeleted");
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

	this->update = false;
	this->transform = false;

	if(!isDeleted(this->parent))
	{
		Container::hide(this);
		Container::removeChild(this->parent, this, true);
	}
	else if(!this->deleteMe)
	{
		delete this;
	}
}

void Container::streamOut(bool streamOut)
{
	this->dontStreamOut = !streamOut;
}

void Container::addBehavior(Behavior behavior)
{
	if(!isDeleted(behavior))
	{
		if(!this->behaviors)
		{
			this->behaviors = new VirtualList();
		}

		if(!VirtualList::find(this->behaviors, behavior))
		{
			VirtualList::pushBack(this->behaviors, behavior);
		}
	}
}

void Container::removeBehavior(Behavior behavior)
{
	if(this->behaviors)
	{
		VirtualList::removeElement(this->behaviors, behavior);
	}
}

void Container::deleteAllChildren()
{
	// if I have children
	if(NULL != this->children)
	{
		// update each child
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{			
			Container child = Container::safeCast(node->data);
			child->deleteMe = true;
		}
	}	
}

/**
 * Add a child Container
 *
 * @param child	Child Container to add
 */
void Container::addChild(Container child)
{
	// check if child is valid
	if(isDeleted(child))
	{
		ASSERT(false, "Container::addChild: adding null child");
		return;
	}

	// if don't have any child yet
	if(NULL == this->children)
	{
		// create children list
		this->children = new VirtualList();
	}

	// first remove from previous parent
	if(this != child->parent)
	{
		if(NULL != child->parent)
		{
			Container::removeChild(child->parent, child, false);

			Transformation environmentTransform = Container::getEnvironmentTransform(this);
			Container::concatenateTransform(this, &environmentTransform, &this->transformation);
			Container::changeEnvironment(child, &environmentTransform);
		}

		// set new parent
		child->parent = this;

		// add to the children list
		VirtualList::pushBack(this->children, (void*)child);

		Container::invalidateGlobalTransformation(child);

		this->update = this->update || Container::overrides(child, update);
		this->transform = this->transform || Container::overrides(child, transform);
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

	if(isDeleted(child) || this != child->parent || isDeleted((this->children)))
	{
		return;
	}

	if(!deleteChild)
	{
		if(VirtualList::removeElement(this->children, child))
		{
			child->parent = NULL;
		}
	}
#ifndef __RELEASE
	else if(NULL != VirtualList::find(this->children, child))
#else
	else
#endif
	{
		child->parent = NULL;
		child->deleteMe = deleteChild;

		if(deleteChild)
		{
			Container::discardAllMessages(child);
			Container::destroyComponents(child);
		}
	}
#ifndef __RELEASE
	else
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Container's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (uint32)this, 18, 15, 8, NULL);
		Printing::text(Printing::getInstance(), "Container's type: ", 1, 16, NULL);
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

		NM_ASSERT(false, "Container::removeChild: not my child");
	}
#endif
}

void Container::destroyComponents()
{
	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			Container::destroyComponents(child);
		}
	}
}

/**
 * Process removed children
 */
void Container::purgeChildren()
{
	// if I have children
	if(NULL != this->children)
	{
		// update each child
		for(VirtualNode node = this->children->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;
			
			Container child = Container::safeCast(node->data);

#ifndef __RELEASE
			if(isDeleted(child))
			{
				Printing::setDebugMode(Printing::getInstance());
				Printing::text(Printing::getInstance(), "ListenerObject's address: ", 1, 15, NULL);
				Printing::hex(Printing::getInstance(), (uint32)this, 18, 15, 8, NULL);
				Printing::text(Printing::getInstance(), "ListenerObject's type: ", 1, 16, NULL);
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

				NM_ASSERT(false, "Container::purgeChildren: deleted children");
			}
#endif

			if(child->deleteMe)
			{
				VirtualList::removeNode(this->children, node);
				child->parent = NULL;
				delete child;
			}
		}
	}
}

/**
 * Container is ready
 *
 * @param recursive
 */
void Container::ready(bool recursive)
{
	if(this->behaviors)
	{
		for(VirtualNode node = this->behaviors->head; NULL != node; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(Behavior::isEnabled(behavior))
			{
				Behavior::start(behavior, this);
			}
		}
	}

	if(recursive && this->children)
	{
		for(VirtualNode childNode = this->children->head; childNode; childNode = childNode->next)
		{
			Container::ready(childNode->data, recursive);
		}
	}
}

/**
 * Update container
 *
 */
void Container::update()
{
	Container::updateBehaviors(this);
	Container::updateChildren(this);
}

/**
 * Update container's behaviors
 *
 */
void Container::updateBehaviors()
{
	if(NULL != this->behaviors)
	{
		for(VirtualNode node = this->behaviors->head; NULL != node; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(Behavior::isEnabled(behavior))
			{
				Behavior::update(behavior, this);
			}
		}
	}
}

/**
 * Update container's children
 *
 */
void Container::updateChildren()
{
	// if I have children
	if(NULL != this->children)
	{
		bool hadChildren = NULL != this->children->head;

		for(VirtualNode node = this->children->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;
			
			Container child = Container::safeCast(node->data);

#ifndef __RELEASE
			if(isDeleted(child))
			{
				Printing::setDebugMode(Printing::getInstance());
				Printing::text(Printing::getInstance(), "ListenerObject's address: ", 1, 15, NULL);
				Printing::hex(Printing::getInstance(), (uint32)this, 18, 15, 8, NULL);
				Printing::text(Printing::getInstance(), "ListenerObject's type: ", 1, 16, NULL);
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

				NM_ASSERT(false, "Container::updateChildren: deleted children");
			}
#endif
			if(child->deleteMe)
			{
				VirtualList::removeNode(this->children, node);
				child->parent = NULL;
				delete child;
				continue;
			}

			if(!child->update)
			{
				continue;
			}

			Container::update(child);
		}

		if(hadChildren && NULL == this->children->head)
		{
			Container::fireEvent(this, kEventContainerAllChildrenDeleted);
		}
	}
}

/**
 * Retrieve environment transformation
 *
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
	concatenatedTransformation->globalPosition = Vector3D::sum(concatenatedTransformation->globalPosition, transformation->localPosition);

	// propagate rotation
	concatenatedTransformation->globalRotation = Rotation::sum(concatenatedTransformation->globalRotation, transformation->localRotation);

	// propagate scale
	concatenatedTransformation->globalScale = Scale::product(concatenatedTransformation->globalScale, transformation->localScale);
}

/**
 * Change environment
 *
 * @param environmentTransform
 */
void Container::changeEnvironment(Transformation* environmentTransform)
{
	Vector3D localPosition = Vector3D::sub(this->transformation.globalPosition, environmentTransform->globalPosition);
	Rotation localRotation = Rotation::sub(this->transformation.globalRotation, environmentTransform->globalRotation);
	Scale localScale = Scale::division(this->transformation.globalScale, environmentTransform->globalScale);

	Container::setLocalPosition(this, &localPosition);
	Container::setLocalRotation(this, &localRotation);
	Container::setLocalScale(this, &localScale);

	// force global position calculation on the next transformation cycle
	Container::invalidateGlobalTransformation(this);
}

/**
 *
 *
 * @private
 * @param environmentTransform
 */
inline void Container::applyEnvironmentToPosition(const Transformation* environmentTransform)
{
	Vector3D localPosition = this->transformation.localPosition;

	if(0 != (environmentTransform->globalRotation.x | environmentTransform->globalRotation.y | environmentTransform->globalRotation.z))
	{
		localPosition = Vector3D::rotate(localPosition, environmentTransform->globalRotation);
	}

	if(0 != (environmentTransform->globalScale.x | environmentTransform->globalScale.y | environmentTransform->globalScale.z))
	{
		localPosition = Vector3D::scale(localPosition, environmentTransform->globalScale);
	}

	this->transformation.globalPosition = Vector3D::sum(environmentTransform->globalPosition, localPosition);
}

/**
 *
 *
 * @private
 * @param environmentTransform
 */
inline void Container::applyEnvironmentToRotation(const Transformation* environmentTransform)
{
	this->transformation.globalRotation = Rotation::sum(environmentTransform->globalRotation, this->transformation.localRotation);
}

/**
 *
 *
 * @private
 * @param environmentTransform
 */
inline void Container::applyEnvironmentToScale(const Transformation* environmentTransform)
{
	this->transformation.globalScale = Scale::product(environmentTransform->globalScale, this->transformation.localScale);
}

/**
 * Propagate call to createComponents
 *
 */
void Container::createComponents()
{
	if(!isDeleted(this->children))
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			Container::createComponents(child);
		}
	}
}

/**
 * Initial transformation
 *
 * @param environmentTransform
 * @param recursive
 */
void Container::initialTransform(const Transformation* environmentTransform)
{
	// concatenate transformation
	if(__INHERIT_SCALE & this->inheritEnvironment)
	{
		Container::applyEnvironmentToScale(this, environmentTransform);
	}

	if(__INHERIT_ROTATION & this->inheritEnvironment)
	{
		Container::applyEnvironmentToRotation(this, environmentTransform);
	}

	if((__INHERIT_POSITION | __INHERIT_ROTATION) & this->inheritEnvironment)
	{
		Container::applyEnvironmentToPosition(this, environmentTransform);
	}

	Container::invalidateGlobalTransformation(this);

	// if I have children
	if(!isDeleted(this->children))
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			Container::initialTransform(child, &this->transformation);
		}
	}
}

/**
 * Initial transformation
 *
 * @param environmentTransform
 * @param invalidateTransformationFlag
 */
void Container::transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag)
{
	ASSERT(environmentTransform, "Container::transform: null environmentTransform");

	if(0 != (__INVALIDATE_SCALE & this->invalidateGlobalTransformation))
	{
		if(0 != (__INHERIT_SCALE & this->inheritEnvironment))
		{
			Container::applyEnvironmentToScale(this, environmentTransform);
		}
	}

	if(0 != (__INVALIDATE_ROTATION & this->invalidateGlobalTransformation))
	{
		if(0 != (__INHERIT_ROTATION & this->inheritEnvironment))
		{
			Container::applyEnvironmentToRotation(this, environmentTransform);
		}
	}

	if(0 != ((__INHERIT_POSITION | __INHERIT_ROTATION) & this->invalidateGlobalTransformation))
	{
		// apply environment transformation
		if(0 != (__INHERIT_POSITION & this->inheritEnvironment))
		{
			Container::applyEnvironmentToPosition(this, environmentTransform);
		}
	}

	// Check since the call is virtual
	Container::transformChildren(this, invalidateTransformationFlag);

	// don't update position on next transformation cycle
	this->invalidateGlobalTransformation = false;
}

void Container::transformChildren(uint8 invalidateTransformationFlag)
{
	// if I have children
	if(NULL != this->children)
	{
		uint8 invalidateGraphics = (__INVALIDATE_POSITION & invalidateTransformationFlag) | (__INVALIDATE_ROTATION & invalidateTransformationFlag) | (__INVALIDATE_SCALE & invalidateTransformationFlag) | (__INVALIDATE_PROJECTION & invalidateTransformationFlag);

		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;
			child->invalidateGraphics |= invalidateGraphics;

			// Do not enable this check to optimize things
			// It messes up child entities when you need to 
			// hide and then show them
			// Besides, the transformation should be valid
			// all the time
			/*
			if(child->hidden)
			{
				continue;
			}
			*/

			if(child->deleteMe)
			{
				continue;
			}

			if(!child->transform && NULL == child->children && !child->invalidateGlobalTransformation)
			{
				continue;
			}

			Container::transform(child, &this->transformation, invalidateTransformationFlag);
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
 * Retrieve global position
 *
 * @return		Global position
 */
const Vector3D* Container::getPosition()
{
	return &this->transformation.globalPosition;
}

/**
 * Set global position
 *
 * @param position	Pointer to position
 */
void Container::setPosition(const Vector3D* position)
{
	Vector3D displacement = Vector3D::get(this->transformation.globalPosition, *position);

	this->transformation.localPosition = Vector3D::sum(this->transformation.localPosition, displacement);
	this->transformation.globalPosition = *position;

	this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;

	if(displacement.z)
	{
		this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;
	}
}

/**
 * Retrieve global rotation
 *
 * @return		Global rotation
 */
const Rotation* Container::getRotation()
{
	return &this->transformation.globalRotation;
}

void Container::setRotation(const Rotation* rotation)
{
	Rotation displacement = Rotation::sub(this->transformation.globalRotation, *rotation);

	this->transformation.globalRotation = Rotation::clamp(rotation->x, rotation->y, rotation->z);
	this->transformation.localRotation = Rotation::sub(this->transformation.globalRotation, displacement);

	this->invalidateGlobalTransformation |= __INVALIDATE_POSITION | __INVALIDATE_ROTATION;
}

/**
 * Retrieve global scale
 *
 * @return		Global scale
 */
const Scale* Container::getScale()
{
	return &this->transformation.globalScale;
}

void Container::setScale(const Scale* scale)
{
	Scale factor = Scale::division(this->transformation.globalScale, *scale);

	this->transformation.localScale = Scale::product(this->transformation.globalScale, factor);	
	this->transformation.globalScale = *scale;

	this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;
}

/**
 * Set local position
 *
 * @param position	Pointer to position
 */
void Container::setLocalPosition(const Vector3D* position)
{
	// force global position calculation on the next transformation cycle
	if(position == &this->transformation.localPosition)
	{
		Container::invalidateGlobalPosition(this);
		Container::invalidateGlobalScale(this);
	}
	else
	{
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
	Rotation auxRotation = Rotation::clamp(rotation->x, rotation->y, rotation->z);

	if(this->transformation.localRotation.z != auxRotation.z)
	{
		Container::invalidateGlobalRotation(this);
	}
	else if(this->transformation.localRotation.x != auxRotation.x)
	{
		Container::invalidateGlobalRotation(this);
	}
	else if(this->transformation.localRotation.y != auxRotation.y)
	{
		Container::invalidateGlobalRotation(this);
	}

	this->transformation.localRotation = auxRotation;
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
	if(scale == &this->transformation.localScale)
	{
		Container::invalidateGlobalScale(this);
	}
	else
	{
		if(this->transformation.localScale.z != scale->z)
		{
			Container::invalidateGlobalRotation(this);
		}
		else if(this->transformation.localScale.x != scale->x)
		{
			Container::invalidateGlobalRotation(this);
		}
		else if(this->transformation.localScale.y != scale->y)
		{
			Container::invalidateGlobalRotation(this);
		}

		this->transformation.localScale = *scale;
	}
}

/**
 * Translate 
 *
 * @param translation 	Pointer to a Vector3D
 */
void Container::translate(const Vector3D* translation)
{

	Vector3D localPosition = Vector3D::sum(this->transformation.localPosition, *translation);
	Container::setLocalPosition(this, &localPosition);	
}

/**
 * Rotate 
 *
 * @param translation 	Pointer to a Vector3D
 */
void Container::rotate(const Rotation* rotation)
{
	if(0 != rotation->x || 0 != rotation->y || 0 != rotation->z)
	{
		Container::invalidateGlobalRotation(this);
	}

	this->transformation.localRotation = Rotation::sum(this->transformation.localRotation, *rotation);	
}

/**
 * Scale 
 *
 * @param translation 	Pointer to a Vector3D
 */
void Container::scale(const Scale* scale)
{
	Container::invalidateGlobalScale(this);

	this->transformation.localScale = Scale::product(this->transformation.localScale, *scale);	
}

/**
 * Invalidate global transformation
 */
void Container::invalidateGlobalTransformation()
{
	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;

	if(!isDeleted(this->children))
	{
		// update each child
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
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

	if(NULL != this->children)
	{
		// update each child
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
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

	if(NULL != this->children)
	{
		// update each child
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
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

	if(NULL != this->children)
	{
		// update each child
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			// make sure child recalculates its global position
			Container::invalidateGlobalScale(node->data);
		}
	}
}

/**
 * Set transparency 
 *
 * @param transparent 	Transparency flag
 */
void Container::setTransparent(uint8 transparent)
{
	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			Container::setTransparent(child, transparent);
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
int32 Container::propagateMessage(int32 (*propagatedMessageHandler)(void*, va_list), ...)
{
	ASSERT(propagatedMessageHandler, "Container::propagateMessage: null propagatedMessageHandler");

	va_list args;
	va_start(args, propagatedMessageHandler);
	int32 result = Container::propagateArguments(this, propagatedMessageHandler, args);
	va_end(args);

	return result;
}

/**
 * Propagate a string to the child wrapper
 *
 * @param propagatedMessageHandler
 * @param args						va_list of propagated string parameters

 * @return							Result
 */
int32 Container::propagateString(int32 (*propagatedStringHandler)(void*, va_list), ...)
{
	ASSERT(propagatedStringHandler, "Container::propagateMessage: null propagatedStringHandler");

	va_list args;
	va_start(args, propagatedStringHandler);
	int32 result = Container::propagateArguments(this, propagatedStringHandler, args);
	va_end(args);

	return result;
}


/**
 * Pass message to children recursively
 *
 * @param propagationHandler		Method used to actually propagate the arguments
 * @param args						va_list of propagated message parameters

 * @return							Result
 */
int32 Container::propagateArguments(int32 (*propagationHandler)(void*, va_list), va_list args)
{
	// if message is valid
	if(NULL == propagationHandler)
	{
		return false;
	}

	// propagate if I have children
	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			// pass message to each child
			if(!child->deleteMe && Container::propagateArguments(child, propagationHandler, args))
			{
				return true;
			}
		}
	}

	// if no child processed the message, I process it
	return propagationHandler(this, args);
}

/**
 * Process message
 *
 * @param args	va_list of propagated message parameters

 * @return		Result
 */
int32 Container::onPropagatedMessage(va_list args)
{
	int32 message = va_arg(args, int32);
	return  Container::handlePropagatedMessage(this, message);
}

/**
 * Process string
 *
 * @param args	va_list of propagated string parameters

 * @return		Result
 */
int32 Container::onPropagatedString(va_list args)
{
	const char* string = va_arg(args, char*);
	return  Container::handlePropagatedString(this, string);
}

/**
 * Handle propagated message
 *
 * @param message	Message

 * @return			Result
 */
bool Container::handlePropagatedMessage(int32 message __attribute__ ((unused)))
{
	return false;
}

/**
 * Handle propagated string
 *
 * @param message	Message

 * @return			Result
 */
bool Container::handlePropagatedString(const char* string __attribute__ ((unused)))
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
int32 Container::getChildCount()
{
	return NULL != this->children ? VirtualList::getSize(this->children) : 0;
}

/**
 * Set name
 *
 * @param name	Name
 */
void Container::setName(const char* const name)
{
	if(NULL != this->name)
	{
		delete this->name;
	}

	if(NULL == name)
	{
		return;
	}

	typedef struct NameWrapper
	{
		char name[__MAX_CONTAINER_NAME_LENGTH + 1];

	} NameWrapper;

	NameWrapper* nameWrapper = (NameWrapper*)new NameWrapper;
	this->name = nameWrapper->name;

	strncpy(this->name, name, __MAX_CONTAINER_NAME_LENGTH);
	this->name[__MAX_CONTAINER_NAME_LENGTH] = '\0';
}

/**
 * Retrieve name
 *
 * @return		Name
 */
const char* Container::getName()
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
Container Container::findChildByName(VirtualList children, const char* childName, bool recursive)
{
	if(this->deleteMe)
	{
		return NULL;
	}

	// look through all children
	for(VirtualNode node = children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		if(child->deleteMe)
		{
			continue;
		}

		if(NULL != child->name && !strncmp(childName, child->name, __MAX_CONTAINER_NAME_LENGTH))
		{
			return child;
		}
		else if(recursive && NULL != child->children)
		{
			Container grandChild = Container::findChildByName(this, child->children, childName, recursive);
			
			if(!isDeleted(grandChild))
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
Container Container::getChildByName(const char* childName, bool recursive)
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

	return !isDeleted(foundChild) && !foundChild->deleteMe ? foundChild : NULL;
}

/**
 * Get child by its position in my children list
 *
 * @param position	Index
 * @return			Child Container
 */
Container Container::getChildAtPosition(int16 position)
{
	if(isDeleted(this->children))
	{
		return NULL;
	}

	return Container::safeCast(VirtualList::getObjectAtPosition(this->children, position));
}

/**
 * Suspend for pause
 */
void Container::suspend()
{
	if(this->behaviors)
	{
		for(VirtualNode node = this->behaviors->head; NULL != node; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(Behavior::isEnabled(behavior))
			{
				Behavior::pause(behavior, this);
			}
		}
	}

	if(NULL != this->children)
	{
		Container::purgeChildren(this);

		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
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
	if(this->behaviors)
	{
		for(VirtualNode node = this->behaviors->head; NULL != node; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(Behavior::isEnabled(behavior))
			{
				Behavior::resume(behavior, this);
			}
		}
	}

	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
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

	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			Container::show(child);
		}
	}

	Container::invalidateGlobalTransformation(this);
}

void Container::hide()
{
	this->hidden = true;

	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container::hide(node->data);
		}
	}
}

bool Container::isHidden()
{
	return this->hidden;
}

void Container::setInheritEnvironment(uint8 inheritEnvironment)
{
	this->inheritEnvironment = inheritEnvironment;
}

bool Container::getChildren(ClassPointer classPointer, VirtualList children)
{
	if(!isDeleted(this->children) && !isDeleted(children))
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(NULL == classPointer || Object::getCast(child, classPointer, NULL))
			{
				VirtualList::pushBack(children, child);
			}
		}

		if(VirtualList::getSize(children))
		{
			return true;
		}
	}

	return false;
}

bool Container::getBehaviors(ClassPointer classPointer, VirtualList behaviors)
{
	if(this->behaviors && !isDeleted(behaviors))
	{
		for(VirtualNode node = this->behaviors->head; NULL != node; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(!classPointer || Object::getCast(behavior, classPointer, NULL))
			{
				VirtualList::pushBack(behaviors, behavior);
			}
		}

		if(NULL != behaviors->head)
		{
			return true;
		}
	}

	return false;
}

bool Container::isTransformed()
{
	return !this->invalidateGlobalTransformation;
}

Rotation Container::getRotationFromDirection(const Vector3D* direction, uint8 axis)
{
	Rotation rotation = this->transformation.localRotation;

	if(__X_AXIS & axis)
	{
		fixed_ext_t z = direction->z;

		if(direction->x)
		{
			z = Math::squareRootFixed(__FIXED_EXT_MULT(direction->x, direction->x) + __FIXED_EXT_MULT(direction->z, direction->z));

			z = 0 > direction->z ? -z : z;
		}

		rotation.x = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9(direction->y), __FIXED_TO_FIX7_9(z))) - __QUARTER_ROTATION_DEGREES;
	}
	
	if(__Y_AXIS & axis)
	{
		fixed_ext_t x = direction->x;

		if(direction->y)
		{
			x = Math::squareRootFixed(__FIXED_EXT_MULT(direction->y, direction->y) + __FIXED_EXT_MULT(direction->x, direction->x));

			x = 0 > direction->x ? -x : x;
		}

		rotation.y = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9((direction->z)), __FIXED_TO_FIX7_9(x)));
	}

	if(__Z_AXIS & axis)
	{
		fixed_ext_t y = direction->y;

		if(direction->z)
		{
			y = Math::squareRootFixed(__FIXED_EXT_MULT(direction->z, direction->z) + __FIXED_EXT_MULT(direction->y, direction->y));

			y = 0 > direction->y ? -y : y;
		}

		rotation.z = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9((direction->x)), __FIXED_TO_FIX7_9(y)));
	}

	if(__X_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.z)
		{
			rotation.x = rotation.x - __HALF_ROTATION_DEGREES;
		}
	}

	if(__Y_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.x)
		{
			rotation.y = rotation.y - __HALF_ROTATION_DEGREES;
		}
	}

	if(__Z_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.y)
		{
			rotation.z = rotation.z - __HALF_ROTATION_DEGREES;
		}
	}

	return Rotation::clamp(rotation.x, rotation.y, rotation.z);	
}