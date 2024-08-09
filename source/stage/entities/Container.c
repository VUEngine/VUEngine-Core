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
	this->localTransformation.position = Vector3D::zero();

	// set rotation
	this->localTransformation.rotation = Rotation::zero();

	// set scale
	this->localTransformation.scale = Scale::unit();

	// force global position calculation on the next transformation cycle
	this->transformation.invalid = __NON_TRANSFORMED;
	this->localTransformation.invalid = __VALID_TRANSFORMATION;

	this->parent = NULL;
	this->children = NULL;
	this->deleteMe = false;
	this->hidden = false;
	this->inheritEnvironment = __INHERIT_TRANSFORMATION;
	this->dontStreamOut = false;
	this->ready = false;

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

void Container::deleteAllChildren()
{
	if(NULL == this->children)
	{
		return;
	}


	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{			
		Container child = Container::safeCast(node->data);
		child->deleteMe = true;
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
		NM_ASSERT(false, "Container::addChild: adding null child");
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
		Transformation environmentTransformation = Container::getEnvironmentTransform(this);
		Container::concatenateTransform(this, &environmentTransformation, &this->transformation);

		if(NULL != child->parent)
		{
			Container::removeChild(child->parent, child, false);			
			Container::changeEnvironment(child, &environmentTransformation);
		}

		// set new parent
		child->parent = this;

		// add to the children list
		VirtualList::pushBack(this->children, (void*)child);


		this->update = this->update || Container::overrides(child, update);
		this->transform = this->transform || Container::overrides(child, transform);

		if(__NON_TRANSFORMED == child->transformation.invalid || __INVALIDATE_TRANSFORMATION == child->transformation.invalid)
		{
			Container::transform(child, &environmentTransformation, __INVALIDATE_TRANSFORMATION);
		}

		Container::createComponents(child);

		//NM_ASSERT(!child->ready, "Container::addChild: child is ready");

		if(!child->ready)
		{
			Container::ready(child, true);
		}

		child->ready = true;
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
	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		Container::destroyComponents(child);
	}
}

/**
 * Process removed children
 */
void Container::purgeChildren()
{
	if(NULL == this->children)
	{
		return;
	}

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

	if(NULL == this->children->head)
	{
		delete this->children;
		this->children = NULL;
	}
}

/**
 * Container is ready
 *
 * @param recursive
 */
void Container::ready(bool recursive)
{
	if(!recursive || NULL == this->children)
	{
		return;
	}

	for(VirtualNode childNode = this->children->head; childNode; childNode = childNode->next)
	{
		Container::ready(childNode->data, recursive);
	}

	if(this->hidden)
	{
		Container::hide(this);
	}
}

/**
 * Update container
 *
 */
void Container::update()
{
	Container::updateChildren(this);
}

/**
 * Update container's children
 *
 */
void Container::updateChildren()
{
	// if I have children
	if(NULL == this->children)
	{
		return;
	}

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

		if(!child->update && NULL == child->children)
		{
			continue;
		}

		Container::update(child);
	}
}

/**
 * Retrieve environment transformation
 *
 * @return				Environment Transformation
 */
Transformation Container::getEnvironmentTransform()
{
	if(NULL == this->parent)
	{
		Transformation environmentTransformation =
		{
			// spatial position
			{0, 0, 0},
		
			// spatial rotation
			{0, 0, 0},
		
			// spatial scale
			{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},

			// invalidity
			__VALID_TRANSFORMATION
		};

		return environmentTransformation;
	}

	return this->parent->transformation;
}

/**
 * Contatenate transformation
 *
 * @param concatenatedTransformation
 * @param transformation
 */
void Container::concatenateTransform(Transformation* concatenatedTransformation, Transformation* localTransformation)
{
	ASSERT(concatenatedTransformation, "Container::concatenateTransform: null concatenatedTransformation");
	ASSERT(localTransformation, "Container::concatenateTransform: null localTransformation");

	// tranlate position
	concatenatedTransformation->position = Vector3D::sum(concatenatedTransformation->position, localTransformation->position);

	// propagate rotation
	concatenatedTransformation->rotation = Rotation::sum(concatenatedTransformation->rotation, localTransformation->rotation);

	// propagate scale
	concatenatedTransformation->scale = Scale::product(concatenatedTransformation->scale, localTransformation->scale);
}

/**
 * Change environment
 *
 * @param environmentTransformation
 */
void Container::changeEnvironment(Transformation* environmentTransformation)
{
	Vector3D localPosition = Vector3D::sub(this->transformation.position, environmentTransformation->position);
	Rotation localRotation = Rotation::sub(this->transformation.rotation, environmentTransformation->rotation);
	Scale localScale = Scale::division(this->transformation.scale, environmentTransformation->scale);

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
 * @param environmentTransformation
 */
inline void Container::applyEnvironmentToPosition(const Transformation* environmentTransformation)
{
	Vector3D localPosition = this->localTransformation.position;

	if(0 != (environmentTransformation->rotation.x | environmentTransformation->rotation.y | environmentTransformation->rotation.z))
	{
		localPosition = Vector3D::rotate(localPosition, environmentTransformation->rotation);
	}

	if(0 != (environmentTransformation->scale.x | environmentTransformation->scale.y | environmentTransformation->scale.z))
	{
		localPosition = Vector3D::scale(localPosition, environmentTransformation->scale);
	}

	this->transformation.position = Vector3D::sum(environmentTransformation->position, localPosition);
}

/**
 *
 *
 * @private
 * @param environmentTransformation
 */
inline void Container::applyEnvironmentToRotation(const Transformation* environmentTransformation)
{
	this->transformation.rotation = Rotation::sum(environmentTransformation->rotation, this->localTransformation.rotation);
}

/**
 *
 *
 * @private
 * @param environmentTransformation
 */
inline void Container::applyEnvironmentToScale(const Transformation* environmentTransformation)
{
	this->transformation.scale = Scale::product(environmentTransformation->scale, this->localTransformation.scale);
}

/**
 * Propagate call to createComponents
 *
 */
void Container::createComponents()
{
	if(isDeleted(this->children))
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		Container::createComponents(child);
	}
}

/**
 * Initial transformation
 *
 * @param environmentTransformation
 * @param invalidateTransformationFlag
 */
void Container::transform(const Transformation* environmentTransformation, uint8 invalidateTransformationFlag)
{
	ASSERT(environmentTransformation, "Container::transform: null environmentTransformation");

	if(0 != (__INVALIDATE_SCALE & this->transformation.invalid))
	{
		if(0 != (__INHERIT_SCALE & this->inheritEnvironment))
		{
			Container::applyEnvironmentToScale(this, environmentTransformation);
		}
	}

	if(0 != (__INVALIDATE_ROTATION & this->transformation.invalid))
	{
		if(0 != (__INHERIT_ROTATION & this->inheritEnvironment))
		{
			Container::applyEnvironmentToRotation(this, environmentTransformation);
		}
	}

	if(0 != ((__INVALIDATE_POSITION | __INVALIDATE_ROTATION) & this->transformation.invalid))
	{
		// apply environment transformation
		if(0 != (__INHERIT_POSITION & this->inheritEnvironment))
		{
			Container::applyEnvironmentToPosition(this, environmentTransformation);
		}
	}

	// Check since the call is virtual
	Container::transformChildren(this, invalidateTransformationFlag);

	// don't update position on next transformation cycle
	this->transformation.invalid = __VALID_TRANSFORMATION;
}

void Container::transformChildren(uint8 invalidateTransformationFlag)
{
	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		child->transformation.invalid |= this->transformation.invalid;

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

		if(!child->transform && NULL == child->children && __VALID_TRANSFORMATION == child->transformation.invalid)
		{
			continue;
		}

		if(Container::overrides(child, transform))
		{
			Container::transform(child, &this->transformation, invalidateTransformationFlag);
		}
		else
		{
			// TODO: fix this hack
			// This should be a call to Container_transform, but the preprocessor makes the virtual call
			if(0 != (__INVALIDATE_SCALE & child->transformation.invalid))
			{
				if(0 != (__INHERIT_SCALE & child->inheritEnvironment))
				{
					Container::applyEnvironmentToScale(child, &this->transformation);
				}
			}

			if(0 != (__INVALIDATE_ROTATION & child->transformation.invalid))
			{
				if(0 != (__INHERIT_ROTATION & child->inheritEnvironment))
				{
					Container::applyEnvironmentToRotation(child, &this->transformation);
				}
			}

			if(0 != ((__INVALIDATE_POSITION | __INVALIDATE_ROTATION) & child->transformation.invalid))
			{
				// apply environment transformation
				if(0 != (__INHERIT_POSITION & child->inheritEnvironment))
				{
					Container::applyEnvironmentToPosition(child, &this->transformation);
				}
			}

			// Check since the call is virtual
			Container::transformChildren(child, invalidateTransformationFlag);

			// don't update position on next transformation cycle
			child->transformation.invalid = __VALID_TRANSFORMATION;
		}
	}
}

/**
 * Retrieve global position
 *
 * @return		Pointer to global position
 */
const Vector3D* Container::getGlobalPosition()
{
	return &this->transformation.position;
}

/**
 * Retrieve local position
 *
 * @return		Pointer to local position
 */
const Vector3D* Container::getLocalPosition()
{
	return &this->localTransformation.position;
}

/**
 * Set global position
 *
 * @param position	Pointer to position
 */
void Container::setPosition(const Vector3D* position)
{
	Vector3D displacement = Vector3D::get(this->transformation.position, *position);

	this->localTransformation.position = Vector3D::sum(this->localTransformation.position, displacement);
	this->transformation.position = *position;

	this->transformation.invalid |= __INVALIDATE_POSITION;

	if(displacement.z)
	{
		this->transformation.invalid |= __INVALIDATE_SCALE;
	}
}

void Container::setRotation(const Rotation* rotation)
{
	Rotation displacement = Rotation::sub(this->transformation.rotation, *rotation);

	this->transformation.rotation = Rotation::clamp(rotation->x, rotation->y, rotation->z);
	this->localTransformation.rotation = Rotation::sub(this->transformation.rotation, displacement);

	this->transformation.invalid |= __INVALIDATE_POSITION | __INVALIDATE_ROTATION;
}

void Container::setScale(const Scale* scale)
{
	Scale factor = Scale::division(this->transformation.scale, *scale);

	this->localTransformation.scale = Scale::product(this->transformation.scale, factor);	
	this->transformation.scale = *scale;

	this->transformation.invalid |= __INVALIDATE_SCALE;
}

/**
 * Set local position
 *
 * @param position	Pointer to position
 */
void Container::setLocalPosition(const Vector3D* position)
{
	// force global position calculation on the next transformation cycle
	if(position == &this->localTransformation.position)
	{
		Container::invalidateGlobalPosition(this);
		Container::invalidateGlobalScale(this);
	}
	else
	{
		if(this->localTransformation.position.z != position->z)
		{
			Container::invalidateGlobalPosition(this);
			Container::invalidateGlobalScale(this);
		}
		else if(this->localTransformation.position.x != position->x)
		{
			Container::invalidateGlobalPosition(this);
		}
		else if(this->localTransformation.position.y != position->y)
		{
			Container::invalidateGlobalPosition(this);
		}

		this->localTransformation.position = *position;
	}
}

/**
 * Retrieve local rotation
 *
 * @return		Pointer to local Rotation
 */
const Rotation* Container::getLocalRotation()
{
	return &this->localTransformation.rotation;
}

/**
 * Set local rotation
 *
 * @param rotation	Pointer to Rotation
 */
void Container::setLocalRotation(const Rotation* rotation)
{
	Rotation auxRotation = Rotation::clamp(rotation->x, rotation->y, rotation->z);

	if(this->localTransformation.rotation.z != auxRotation.z)
	{
		Container::invalidateGlobalRotation(this);
	}
	else if(this->localTransformation.rotation.x != auxRotation.x)
	{
		Container::invalidateGlobalRotation(this);
	}
	else if(this->localTransformation.rotation.y != auxRotation.y)
	{
		Container::invalidateGlobalRotation(this);
	}

	this->localTransformation.rotation = auxRotation;
}

/**
 * Retrieve local scale
 *
 * @return		Pointer to local Scale
 */
const Scale* Container::getLocalScale()
{
	return &this->localTransformation.scale;
}

/**
 * Set local scale
 *
 * @param scale	Pointer to Scale
 */
void Container::setLocalScale(const Scale* scale)
{
	if(scale == &this->localTransformation.scale)
	{
		Container::invalidateGlobalScale(this);
	}
	else
	{
		if(this->localTransformation.scale.z != scale->z)
		{
			Container::invalidateGlobalRotation(this);
		}
		else if(this->localTransformation.scale.x != scale->x)
		{
			Container::invalidateGlobalRotation(this);
		}
		else if(this->localTransformation.scale.y != scale->y)
		{
			Container::invalidateGlobalRotation(this);
		}

		this->localTransformation.scale = *scale;
	}
}

/**
 * Translate 
 *
 * @param translation 	Pointer to a Vector3D
 */
void Container::translate(const Vector3D* translation)
{
	Vector3D localPosition = Vector3D::sum(this->localTransformation.position, *translation);
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

	this->localTransformation.rotation = Rotation::sum(this->localTransformation.rotation, *rotation);	
}

/**
 * Scale 
 *
 * @param translation 	Pointer to a Vector3D
 */
void Container::scale(const Scale* scale)
{
	Container::invalidateGlobalScale(this);

	this->localTransformation.scale = Scale::product(this->localTransformation.scale, *scale);	
}

/**
 * Invalidate global transformation
 */
void Container::invalidateGlobalTransformation()
{
	this->transformation.invalid = __INVALIDATE_TRANSFORMATION;

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
	this->transformation.invalid |= __INVALIDATE_POSITION;

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container::invalidateGlobalPosition(node->data);
	}
}

/**
 * Invalidate global rotation
 */
void Container::invalidateGlobalRotation()
{
	this->transformation.invalid |= __INVALIDATE_ROTATION;

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container::invalidateGlobalRotation(node->data);
	}
}

/**
 * Invalidate global scale
 */
void Container::invalidateGlobalScale()
{
	this->transformation.invalid |= __INVALIDATE_SCALE;

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container::invalidateGlobalScale(node->data);
	}
}

/**
 * Set transparency 
 *
 * @param transparent 	Transparency flag
 */
void Container::setTransparent(uint8 transparent)
{
	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		Container::setTransparent(child, transparent);
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
	Container::purgeChildren(this);

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		Container::suspend(child);
	}
}

/**
 * Resume after pause
 */
void Container::resume()
{
	Container::invalidateGlobalTransformation(this);

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		Container::resume(child);
	}
}

void Container::show()
{
	this->hidden = false;

	Container::invalidateGlobalTransformation(this);

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		Container::show(child);
	}
}

void Container::hide()
{
	this->hidden = true;

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container::hide(node->data);
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
	if(isDeleted(this->children) || isDeleted(children))
	{
		return false;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		if(NULL == classPointer || Object::getCast(child, classPointer, NULL))
		{
			VirtualList::pushBack(children, child);
		}
	}

	return 0 < VirtualList::getSize(children);
}

bool Container::isTransformed()
{
	return __VALID_TRANSFORMATION == this->transformation.invalid;
}

Rotation Container::getRotationFromDirection(const Vector3D* direction, uint8 axis)
{
	Rotation rotation = this->localTransformation.rotation;

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