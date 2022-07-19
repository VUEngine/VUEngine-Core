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

	// By default, save on calls to main methods.
	this->update = Container::overrides(this, update);
	this->transform = Container::overrides(this, transform);

	this->transformed = false;

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
	if(this->children)
	{
		for(VirtualNode node = this->children->head; node ; node = node->next)
		{
			Container child = Container::safeCast(node->data);

#ifndef __RELEASE
			if(child->parent != this)
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
			}
#endif
			NM_ASSERT(child->parent == this, "Container::destructor: deleting a child of not mine");

			child->parent = NULL;
			delete child;
		}

		// delete children list
		delete this->children;
		this->children = NULL;

	}

	if(this->behaviors)
	{
		VirtualNode node = this->behaviors->head;

		// destroy each child
		for(; node ; node = node->next)
		{
			delete node->data;
		}

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
	if(this->name)
	{
		delete this->name;
	}

	if(this->events)
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

	if(!isDeleted(this->parent))
	{
		Container::hide(this);
		Container::removeChild(this->parent, this, true);
		Container::iAmDeletingMyself(this);
	}
	else
	{
		delete this;
	}
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

			Transformation environmentTransform = Container::getEnvironmentTransform(this);
			Container::concatenateTransform(this, &environmentTransform, &this->transformation);
			Container::changeEnvironment(child, &environmentTransform);
		}

		// set new parent
		child->parent = this;

		// add to the children list
		VirtualList::pushBack(this->children, (void*)child);

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

	if(VirtualList::find(this->children, child))
	{
		child->parent = NULL;
		child->deleteMe = deleteChild;
	}
#ifndef __RELEASE
	else
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "ListenerObject's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (uint32)this, 18, 15, 8, NULL);
		Printing::text(Printing::getInstance(), "ListenerObject's type: ", 1, 16, NULL);
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 18, 16, NULL);

		NM_ASSERT(false, "Container::removeChild: not my child");
	}
#endif
}

void Container::setupShapes()
{
	// if I have children
	if(this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

			Container::setupShapes(child);
		}
	}
}

/**
 * Process removed children
 */
void Container::purgeChildren()
{
	// if I have children
	if(this->children)
	{
		// update each child
		for(VirtualNode node = this->children->head, nextNode = NULL; node ; node = nextNode)
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

				NM_ASSERT(false, "Container::processRemovedChildren: deleted children");
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
		VirtualNode node = this->behaviors->head;

		for(; node ; node = node->next)
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
		// call ready method on children
		VirtualNode childNode = this->children->head;

		for(; childNode; childNode = childNode->next)
		{
			Container::ready(childNode->data, recursive);
		}
	}
}

/**
 * Update container
 *
 * @param elapsedTime
 */
void Container::update(uint32 elapsedTime)
{
	Container::updateBehaviors(this, elapsedTime);
	Container::updateChildren(this, elapsedTime);
}

/**
 * Update container's behaviors
 *
 * @param elapsedTime
 */
void Container::updateBehaviors(uint32 elapsedTime)
{
	if(this->behaviors)
	{
		VirtualNode node = this->behaviors->head;

		for(; node ; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(Behavior::isEnabled(behavior))
			{
				Behavior::update(behavior, this, elapsedTime);
			}
		}
	}

}

/**
 * Update container's children
 *
 * @param elapsedTime
 */
void Container::updateChildren(uint32 elapsedTime)
{
	// if I have children
	if(this->children)
	{
		Container::purgeChildren(this);

		// update each child
		for(VirtualNode node = this->children->head; node ; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

			if(!child->update && NULL == child->children && NULL == this->behaviors)
			{
				continue;
			}

			Container::update(child, elapsedTime);
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
	if(environmentTransform->globalRotation.x || environmentTransform->globalRotation.y || environmentTransform->globalRotation.z)
	{
		this->transformation.globalPosition = Vector3D::sum(environmentTransform->globalPosition, Vector3D::rotate(this->transformation.localPosition, environmentTransform->globalRotation));
	}
	else
	{
		this->transformation.globalPosition = Vector3D::sum(environmentTransform->globalPosition, this->transformation.localPosition);
	}
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
 * Initial transformation
 *
 * @param environmentTransform
 * @param recursive
 */
void Container::initialTransform(const Transformation* environmentTransform, uint32 recursive)
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
	if(recursive && this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

			child->invalidateGlobalTransformation |= this->invalidateGlobalTransformation;

			Container::initialTransform(child, &this->transformation, true);
		}
	}

	this->transformed = !this->hidden;
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

	if(__INHERIT_SCALE & this->inheritEnvironment)
	{
		if(__INVALIDATE_SCALE & this->invalidateGlobalTransformation)
		{
			Container::applyEnvironmentToScale(this, environmentTransform);
		}
	}

	if(__INHERIT_ROTATION & this->inheritEnvironment)
	{
		if(__INVALIDATE_ROTATION & this->invalidateGlobalTransformation)
		{
			Container::applyEnvironmentToRotation(this, environmentTransform);
		}
	}

	if(__INHERIT_POSITION & this->inheritEnvironment)
	{
		// apply environment transformation
		if((__INHERIT_POSITION | __INHERIT_ROTATION) & this->invalidateGlobalTransformation)
		{
			Container::applyEnvironmentToPosition(this, environmentTransform);
		}
	}

	// Check since the call is virtual
	if(this->children)
	{
		Container::transformChildren(this, invalidateTransformationFlag);
	}

	// don't update position on next transformation cycle
	this->invalidateGlobalTransformation = false;
	this->transformed = true;
}

void Container::transformChildren(uint8 invalidateTransformationFlag)
{
	// if I have children
	if(this->children)
	{
		uint8 invalidateGraphics = (__INVALIDATE_POSITION & invalidateTransformationFlag) | (__INVALIDATE_ROTATION & invalidateTransformationFlag) | (__INVALIDATE_SCALE & invalidateTransformationFlag) | (__INVALIDATE_PROJECTION & invalidateTransformationFlag);

		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

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

			if(!child->transform && NULL == child->children && !child->invalidateGlobalTransformation)
			{
				continue;
			}

			Container::transform(child, &this->transformation, invalidateTransformationFlag);
		}
	}
}

void Container::synchronizeGraphics()
{
	Container::synchronizeChildrenGraphics(this);
}

void Container::synchronizeChildrenGraphics()
{
	// if I have children
	if(this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

			if(child->hidden)
			{
				continue;
			}

			if(!child->transformed)
			{
				return;
			}

			if(!child->invalidateGraphics && NULL == child->children)
			{
				continue;
			}

			Container::synchronizeGraphics(child);
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
 * Retrieve position
 *
 * @return		Global position
 */
const Vector3D* Container::getPosition()
{
	return &this->transformation.globalPosition;
}

/**
 * Set local position
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

//	this->transformed = false;
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

	if(this->children)
	{
		VirtualNode node = this->children->head;

		// update each child
		for(; NULL != node; node = node->next)
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
		for(; NULL != node; node = node->next)
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
		for(; NULL != node; node = node->next)
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
		for(; NULL != node; node = node->next)
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
int32 Container::propagateMessage(int32 (*propagatedMessageHandler)(void*, va_list), ...)
{
	ASSERT(propagatedMessageHandler, "Container::propagateMessage: null propagatedMessageHandler");

	va_list args;
	va_start(args, propagatedMessageHandler);
	int32 result =  Container::passMessage(this, propagatedMessageHandler, args);
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
int32 Container::passMessage(int32 (*propagatedMessageHandler)(void*, va_list), va_list args)
{
	// if message is valid
	if(!propagatedMessageHandler)
	{
		return false;
	}

	// propagate if I have children
	if(this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

			// pass message to each child
			if( Container::passMessage(child, propagatedMessageHandler, args))
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
int32 Container::onPropagatedMessage(va_list args)
{
	int32 message = va_arg(args, int32);
	return  Container::handlePropagatedMessage(this, message);
}

/**
 * Process message
 *
 * @param message	Message

 * @return			Result
 */
bool Container::handlePropagatedMessage(int32 message __attribute__ ((unused)))
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
 * Suspend for pause
 */
void Container::suspend()
{
	if(this->behaviors)
	{
		VirtualNode node = this->behaviors->head;

		for(; node ; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(Behavior::isEnabled(behavior))
			{
				Behavior::pause(behavior, this);
			}
		}
	}

	if(this->children)
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
		VirtualNode node = this->behaviors->head;

		for(; node ; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(Behavior::isEnabled(behavior))
			{
				Behavior::resume(behavior, this);
			}
		}
	}

	if(this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

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
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}
			
			Container::show(child);
		}
	}

	Container::invalidateGlobalTransformation(this);
	this->transformed = false;
}

void Container::hide()
{
	this->hidden = true;

	if(this->children)
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
	if(this->children && !isDeleted(children))
	{
		VirtualNode node = this->children->head;

		for(; node ; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(!classPointer || Object::getCast((ListenerObject)child, classPointer, NULL))
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
		VirtualNode node = this->behaviors->head;

		for(; node ; node = node->next)
		{
			Behavior behavior = Behavior::safeCast(node->data);

			if(!classPointer || Object::getCast((ListenerObject)behavior, classPointer, NULL))
			{
				VirtualList::pushBack(behaviors, behavior);
			}
		}

		if(VirtualList::getSize(behaviors))
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