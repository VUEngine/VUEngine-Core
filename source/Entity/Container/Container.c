/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <Body.h>
#include <Printer.h>
#include <VirtualList.h>

#include "Container.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::constructor(int16 internalId, const char* const name)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->internalId = internalId;

	this->localTransformation.position = Vector3D::zero();
	this->localTransformation.rotation = Rotation::zero();
	this->localTransformation.scale = Scale::unit();

	// Force global position calculation on the next transformation cycle
	this->transformation.invalid = __NON_TRANSFORMED;
	this->localTransformation.invalid = __VALID_TRANSFORMATION;

	this->parent = NULL;
	this->children = NULL;
	this->deleteMe = false;
	this->ready = false;
	this->dontStreamOut = false;
	this->hidden = false;
	this->axisForSynchronizationWithBody = __ALL_AXIS;

	this->name = NULL;
	Container::setName(this, name);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::destructor()
{
#ifndef __RELEASE
	if(!this->deleteMe || NULL != this->parent)
	{
		Printer::setDebugMode();
		Printer::clear();
		Printer::text("Me: ", 20, 12, NULL);
		Printer::text(__GET_CLASS_NAME(this), 24, 12, NULL);
		Printer::text("Parent: ", 20, 15, NULL);
		Printer::hex((uint32)this->parent, 29, 15, 8, NULL);

		NM_ASSERT(false, "Container::destructor: illegal destruction of a Container");
	}
#endif
	
	// If I have children
	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

#ifndef __RELEASE
			if(NULL != child->parent && child->parent != this)
			{
				Printer::setDebugMode();
				Printer::clear();
				Printer::text("Me: ", 20, 12, NULL);
				Printer::text(__GET_CLASS_NAME(this), 24, 12, NULL);
				Printer::text("It: ", 20, 13, NULL);
				Printer::text(child ? __GET_CLASS_NAME(child) : "NULL", 24, 13, NULL);
				Printer::hex((uint32)child, 29, 14, 8, NULL);
				Printer::text("Parent: ", 20, 15, NULL);
				Printer::hex((uint32)child->parent, 29, 15, 8, NULL);

				NM_ASSERT(false, "Container::destructor: deleting a child of not mine");
			}
#endif

			child->parent = NULL;
			child->deleteMe = true;
			delete child;
		}

		// Delete children list
		delete this->children;
		this->children = NULL;

	}

	// First remove from parent
	if(this->parent)
	{
		ASSERT(this != this->parent, "Container::destructor: I'm my own father");
		// Don't allow my parent to try to delete me again
		Container::removeChild(this->parent, this, false);
	}

	// Delete name
	if(!isDeleted(this->name))
	{
		delete this->name;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::show()
{
	Base::show(this);

	Container::invalidateTransformation(this);

	if(!isDeleted(this->children))
	{
		Container::propagateCommand(this, kMessageShow);
	}

	this->hidden = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::hide()
{
	Base::hide(this);
	
	if(!isDeleted(this->children))
	{
		Container::propagateCommand(this, kMessageHide);
	}

	this->hidden = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setTransparency(uint8 transparency)
{
	Base::setTransparency(this, transparency);

	if(!isDeleted(this->children))
	{
		Container::propagateCommand(this, kMessageSetTransparency, transparency);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setPosition(const Vector3D* position)
{
	Vector3D displacement = Vector3D::sub(*position, this->transformation.position);

	Base::setPosition(this, position);

	Container::translate(this, &displacement);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setRotation(const Rotation* rotation)
{
	Rotation displacement = Rotation::sub(*rotation, this->transformation.rotation);

	Base::setRotation(this, rotation);

	Container::rotate(this, &displacement);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setScale(const Scale* scale)
{
	Scale factor = Scale::division(*scale, this->transformation.scale);

	Base::setScale(this, &factor);

	Container::scale(this, &factor);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setDirection(const Vector3D* direction)
{
	if(NULL == direction)
	{
		return;
	}

	int8 axisForSynchronizationWithBody = this->axisForSynchronizationWithBody;

	if(NULL != this->body)
	{
		axisForSynchronizationWithBody = Body::getAxisForSynchronizationWithBody(this->body);
	}

	if((int8)__LOCK_AXIS == axisForSynchronizationWithBody)
	{
		return;
	}
		
	if(__NO_AXIS == axisForSynchronizationWithBody)
	{
		NormalizedDirection normalizedDirection = Container::getNormalizedDirection(this);

		if(0 > direction->x)
		{
			normalizedDirection.x = __LEFT;
		}
		else if(0 < direction->x)
		{
			normalizedDirection.x = __RIGHT;
		}

		if(0 > direction->y)
		{
			normalizedDirection.y = __UP;
		}
		else if(0 < direction->y)
		{
			normalizedDirection.y = __DOWN;
		}

		if(0 > direction->z)
		{
			normalizedDirection.z = __NEAR;
		}
		else if(0 < direction->z)
		{
			normalizedDirection.z = __FAR;
		}

		Container::setNormalizedDirection(this, normalizedDirection);
	}
	else
	{
		Rotation localRotation = Container::getRotationFromDirection(this, direction, axisForSynchronizationWithBody);
		Container::setLocalRotation(this, &localRotation);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setNormalizedDirection(NormalizedDirection normalizedDirection)
{
	NormalizedDirection currentNormalizedDirection = Container::getNormalizedDirection(this);

	// If directions XOR is 0, they are equal
	if
	(
		!(
			(currentNormalizedDirection.x ^ normalizedDirection.x) |
			(currentNormalizedDirection.y ^ normalizedDirection.y) |
			(currentNormalizedDirection.z ^ normalizedDirection.z)
		)
	)
	{
		return;
	}

	Rotation rotation =
	{
		__UP == normalizedDirection.y ? 
			__HALF_ROTATION_DEGREES 
			: 
			__DOWN == normalizedDirection.y ? 0 : this->localTransformation.rotation.x,
		__LEFT == normalizedDirection.x ? 
			__HALF_ROTATION_DEGREES 
			: 
			__RIGHT == normalizedDirection.x ? 0 : this->localTransformation.rotation.y,
		//__NEAR == direction.z ? __HALF_ROTATION_DEGREES : __FAR == direction.z ? 0 : this->localTransformation.rotation.z,
		this->localTransformation.rotation.z,
	};

	Container::setLocalRotation(this, &rotation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

NormalizedDirection Container::getNormalizedDirection()
{
	NormalizedDirection normalizedDirection =
	{
		__RIGHT, __DOWN, __FAR
	};

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.rotation.y))
	{
		normalizedDirection.x = __LEFT;
	}

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.rotation.x))
	{
		normalizedDirection.y = __UP;
	}

	if(__QUARTER_ROTATION_DEGREES < __ABS(this->transformation.rotation.z))
	{
		normalizedDirection.z = __NEAR;
	}

	return normalizedDirection;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::deleteMyself()
{
	ASSERT(!isDeleted(this), "Container::deleteMyself: deleted this");

	if(!isDeleted(this->parent))
	{
		Container::removeChild(this->parent, this, true);
	}
	else if(!this->deleteMe)
	{
		this->deleteMe = true;
		delete this;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 Container::getInternalId()
{
	return this->internalId;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const char* Container::getName()
{
	return this->name;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::streamOut(bool streamOut)
{
	this->dontStreamOut = !streamOut;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Container Container::getParent()
{
	return this->parent;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::addChild(Container child)
{
	// Check if child is valid
	if(isDeleted(child))
	{
		NM_ASSERT(false, "Container::addChild: adding null child");
		return;
	}

	// If don't have any child yet
	if(NULL == this->children)
	{
		// Create children list
		this->children = new VirtualList();
	}

	// First remove from previous parent
	if(this != child->parent)
	{
		Transformation environmentTransformation = Container::getEnvironmentTransform(this);
		Container::concatenateTransform(this, &environmentTransformation, &this->transformation);

		if(NULL != child->parent)
		{
			Container::removeChild(child->parent, child, false);			
			Container::changeEnvironment(child, &environmentTransformation);
		}

		// Set new parent
		child->parent = this;

		// Add to the children list
		VirtualList::pushBack(this->children, (void*)child);

		if(__NON_TRANSFORMED == child->transformation.invalid || __INVALIDATE_TRANSFORMATION == child->transformation.invalid)
		{
			Container::transform(child, &environmentTransformation, __INVALIDATE_TRANSFORMATION);
		}

		Container::createComponents(child, NULL);

		//NM_ASSERT(!child->ready, "Container::addChild: child is ready");

		if(!child->ready)
		{
			Container::ready(child, true);
		}

		child->ready = true;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::removeChild(Container child, bool deleteChild)
{
	ASSERT(this == child->parent, "Container::removeChild: not my child");

	if(isDeleted(child) || this != child->parent || isDeleted((this->children)))
	{
		return;
	}

	if(!deleteChild)
	{
		if(VirtualList::removeData(this->children, child))
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
		Printer::setDebugMode();
		Printer::text("Container's address: ", 1, 15, NULL);
		Printer::hex((uint32)this, 18, 15, 8, NULL);
		Printer::text("Container's type: ", 1, 16, NULL);
		Printer::text(__GET_CLASS_NAME(this), 18, 16, NULL);

		NM_ASSERT(false, "Container::removeChild: not my child");
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
			Printer::setDebugMode();
			Printer::text("ListenerObject's address: ", 1, 15, NULL);
			Printer::hex((uint32)this, 18, 15, 8, NULL);
			Printer::text("ListenerObject's type: ", 1, 16, NULL);
			Printer::text(__GET_CLASS_NAME(this), 18, 16, NULL);

			NM_ASSERT(false, "Container::purgeChildren: deleted children");
		}
#endif

		if(child->deleteMe)
		{
			VirtualList::removeNode(this->children, node);
			child->parent = NULL;
			child->deleteMe = true;
			delete child;
		}
	}

	if(NULL == this->children->head)
	{
		delete this->children;
		this->children = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

	return 0 < VirtualList::getCount(children);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Container Container::getChildById(int16 id)
{
	if(this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node ; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->internalId == id)
			{
				return !isDeleted(child) && !child->deleteMe ? child : NULL;
			}
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Container Container::getChildByName(const char* childName, bool recursive)
{
	Container foundChild = NULL;

	if(!this->deleteMe && childName && this->children)
	{
		// Search through direct children
		foundChild = Container::findChildByName(this, this->children, childName, false);

		// If no direct child could be found, do a recursive search, if applicable
		if(!foundChild && recursive)
		{
			foundChild = Container::findChildByName(this, this->children, childName, true);
		}
	}

	return !isDeleted(foundChild) && !foundChild->deleteMe ? foundChild : NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Container Container::getChildAtPosition(int16 position)
{
	if(isDeleted(this->children))
	{
		return NULL;
	}

	return Container::safeCast(VirtualList::getDataAtIndex(this->children, position));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 Container::getChildrenCount()
{
	return NULL != this->children ? VirtualList::getCount(this->children) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Container Container::getRelativeByName(const char* relativeName)
{
	Container topAncestor = this->parent;

	while(NULL != topAncestor->parent)
	{
		topAncestor = topAncestor->parent;
	}

	return Container::getChildByName(topAncestor, relativeName, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::updateChildren()
{
	// If I have children
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
			Printer::setDebugMode();
			Printer::text("ListenerObject's address: ", 1, 15, NULL);
			Printer::hex((uint32)this, 18, 15, 8, NULL);
			Printer::text("ListenerObject's type: ", 1, 16, NULL);
			Printer::text(__GET_CLASS_NAME(this), 18, 16, NULL);

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

		if(!Container::overrides(child, update) && NULL == child->children)
		{
			continue;
		}

		Container::update(child);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::invalidateTransformation()
{
	this->transformation.invalid = __INVALIDATE_TRANSFORMATION;

	if(!isDeleted(this->children))
	{
		// Update each child
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			// Make sure child recalculates its global position
			Container::invalidateTransformation(node->data);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

		if(child->deleteMe)
		{
			continue;
		}

		if(__VALID_TRANSFORMATION == invalidateTransformationFlag)
		{
			if(!Container::overrides(child, transform) && NULL == child->children && __VALID_TRANSFORMATION == child->transformation.invalid)
			{
				continue;
			}
		}

		if(Container::overrides(child, transform))
		{
			Container::transform(child, &this->transformation, invalidateTransformationFlag);
		}
		else
		{
			/// Force non virtual call
			Container::doTransform(child, &this->transformation, invalidateTransformationFlag);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::propagateCommand(uint32 command, ...)
{
	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			if(child->deleteMe)
			{
				continue;
			}

			va_list args;
			va_start(args, command);

			Container::handleCommand(child, command, args);
			
			va_end(args);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Container::propagateMessage(bool (*propagatedMessageHandler)(void*, va_list), ...)
{
	ASSERT(propagatedMessageHandler, "Container::propagateMessage: null propagatedMessageHandler");

	va_list args;
	va_start(args, propagatedMessageHandler);
	bool result = Container::propagateArguments(this, propagatedMessageHandler, args);
	va_end(args);

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Container::onPropagatedMessage(va_list args)
{
	int32 message = va_arg(args, int32);
	return  Container::handlePropagatedMessage(this, message);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Container::propagateString(bool (*propagatedStringHandler)(void*, va_list), ...)
{
	ASSERT(propagatedStringHandler, "Container::propagateMessage: null propagatedStringHandler");

	va_list args;
	va_start(args, propagatedStringHandler);
	bool result = Container::propagateArguments(this, propagatedStringHandler, args);
	va_end(args);

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Container::onPropagatedString(va_list args)
{
	const char* string = va_arg(args, char*);
	return  Container::handlePropagatedString(this, string);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::translate(const Vector3D* translation)
{
	Vector3D position = Vector3D::sum(this->localTransformation.position, *translation);

	if(this->localTransformation.position.z != position.z)
	{
		this->transformation.invalid |= __INVALIDATE_POSITION;
		this->transformation.invalid |= __INVALIDATE_SCALE;
	}
	else if(this->localTransformation.position.x != position.x)
	{
		this->transformation.invalid |= __INVALIDATE_POSITION;
	}
	else if(this->localTransformation.position.y != position.y)
	{
		this->transformation.invalid |= __INVALIDATE_POSITION;
	}

	this->localTransformation.position = position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::rotate(const Rotation* rotation)
{
	if(0 != rotation->x || 0 != rotation->y || 0 != rotation->z)
	{
		this->transformation.invalid |= __INVALIDATE_ROTATION;
	}

	this->localTransformation.rotation = Rotation::sum(this->localTransformation.rotation, *rotation);	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::scale(const Scale* scale)
{
	this->transformation.invalid |= __INVALIDATE_SCALE;

	this->localTransformation.scale = Scale::product(this->localTransformation.scale, *scale);	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* Container::getLocalPosition()
{
	return &this->localTransformation.position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Rotation* Container::getLocalRotation()
{
	return &this->localTransformation.rotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Scale* Container::getLocalScale()
{
	return &this->localTransformation.scale;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setLocalPosition(const Vector3D* position)
{
	Vector3D displacement = this->localTransformation.position;

	// Force global position calculation on the next transformation cycle
	if(position == &this->localTransformation.position)
	{
		this->transformation.invalid |= __INVALIDATE_POSITION;
		this->transformation.invalid |= __INVALIDATE_SCALE;
	}
	else
	{
		if(this->localTransformation.position.z != position->z)
		{
			this->transformation.invalid |= __INVALIDATE_POSITION;
			this->transformation.invalid |= __INVALIDATE_SCALE;
		}
		else if(this->localTransformation.position.x != position->x)
		{
			this->transformation.invalid |= __INVALIDATE_POSITION;
		}
		else if(this->localTransformation.position.y != position->y)
		{
			this->transformation.invalid |= __INVALIDATE_POSITION;
		}

		this->localTransformation.position = *position;
	}

	displacement = Vector3D::sub(displacement, this->localTransformation.position);

	this->transformation.position = Vector3D::sub(this->transformation.position, displacement);

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, &this->transformation.position, Entity::safeCast(this));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setLocalRotation(const Rotation* rotation)
{
	Rotation auxRotation = Rotation::clamp(rotation->x, rotation->y, rotation->z);

	if(this->localTransformation.rotation.z != auxRotation.z)
	{
		this->transformation.invalid |= __INVALIDATE_ROTATION;
	}
	else if(this->localTransformation.rotation.x != auxRotation.x)
	{
		this->transformation.invalid |= __INVALIDATE_ROTATION;
	}
	else if(this->localTransformation.rotation.y != auxRotation.y)
	{
		this->transformation.invalid |= __INVALIDATE_ROTATION;
	}

	this->localTransformation.rotation = auxRotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::setLocalScale(const Scale* scale)
{
	if(scale == &this->localTransformation.scale)
	{
		this->transformation.invalid |= __INVALIDATE_SCALE;
	}
	else
	{
		if(this->localTransformation.scale.z != scale->z)
		{
			this->transformation.invalid |= __INVALIDATE_SCALE;
		}
		else if(this->localTransformation.scale.x != scale->x)
		{
			this->transformation.invalid |= __INVALIDATE_SCALE;
		}
		else if(this->localTransformation.scale.y != scale->y)
		{
			this->transformation.invalid |= __INVALIDATE_SCALE;
		}

		this->localTransformation.scale = *scale;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::transform(const Transformation* environmentTransformation, uint8 invalidateTransformationFlag)
{
	Container::doTransform(this, environmentTransformation, invalidateTransformationFlag);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::update()
{
	Container::updateChildren(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::resume()
{
	Container::invalidateTransformation(this);

	if(NULL == this->children)
	{
		return;
	}

	for(VirtualNode node = this->children->head; NULL != node; node = node->next)
	{
		Container child = Container::safeCast(node->data);

		Container::resume(child);
	}

	if(this->hidden)
	{
		// Force syncronization even if hidden
		this->hidden = false;
		Container::hide(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::handleCommand(int32 command __attribute__ ((unused)), va_list args  __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Container::handlePropagatedMessage(int32 message __attribute__ ((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Container::handlePropagatedString(const char* string __attribute__ ((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::changeEnvironment(Transformation* environmentTransformation)
{
	Vector3D localPosition = Vector3D::sub(this->transformation.position, environmentTransformation->position);
	Rotation localRotation = Rotation::sub(this->transformation.rotation, environmentTransformation->rotation);
	Scale localScale = Scale::division(this->transformation.scale, environmentTransformation->scale);

	Container::setLocalPosition(this, &localPosition);
	Container::setLocalRotation(this, &localRotation);
	Container::setLocalScale(this, &localScale);

	// Force global position calculation on the next transformation cycle
	Container::invalidateTransformation(this);

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, &this->transformation.position, Entity::safeCast(this));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Container Container::findChildByName(VirtualList children, const char* childName, bool recursive)
{
	if(this->deleteMe)
	{
		return NULL;
	}

	// Look through all children
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Transformation Container::getEnvironmentTransform()
{
	if(NULL == this->parent)
	{
		Transformation environmentTransformation =
		{
			// Spatial position
			{0, 0, 0},
		
			// Spatial rotation
			{0, 0, 0},
		
			// Spatial scale
			{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},

			// Invalidity
			__VALID_TRANSFORMATION
		};

		return environmentTransformation;
	}

	return this->parent->transformation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::concatenateTransform(Transformation* concatenatedTransformation, Transformation* localTransformation)
{
	ASSERT(concatenatedTransformation, "Container::concatenateTransform: null concatenatedTransformation");
	ASSERT(localTransformation, "Container::concatenateTransform: null localTransformation");

	// Tranlate position
	concatenatedTransformation->position = Vector3D::sum(concatenatedTransformation->position, localTransformation->position);

	// Propagate rotation
	concatenatedTransformation->rotation = Rotation::sum(concatenatedTransformation->rotation, localTransformation->rotation);

	// Propagate scale
	concatenatedTransformation->scale = Scale::product(concatenatedTransformation->scale, localTransformation->scale);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void Container::applyEnvironmentToRotation(const Transformation* environmentTransformation)
{
	this->transformation.rotation = Rotation::sum(environmentTransformation->rotation, this->localTransformation.rotation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void Container::applyEnvironmentToScale(const Transformation* environmentTransformation)
{
	this->transformation.scale = Scale::product(environmentTransformation->scale, this->localTransformation.scale);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Container::doTransform(const Transformation* environmentTransformation, uint8 invalidateTransformationFlag)
{
	ASSERT(environmentTransformation, "Container::transform: null environmentTransformation");

	uint8 invalidateTransformationFlagHelper = (invalidateTransformationFlag | this->transformation.invalid);
	
	if(0 != (__INVALIDATE_SCALE & invalidateTransformationFlagHelper))
	{
		Container::applyEnvironmentToScale(this, environmentTransformation);
	}

	if(0 != (__INVALIDATE_ROTATION & invalidateTransformationFlagHelper))
	{
		Container::applyEnvironmentToRotation(this, environmentTransformation);
	}

	if(0 != ((__INVALIDATE_POSITION | __INVALIDATE_ROTATION) & invalidateTransformationFlagHelper))
	{
		Container::applyEnvironmentToPosition(this, environmentTransformation);
	}

	Container::transformChildren(this, invalidateTransformationFlagHelper);

	// Don't update position on next transformation cycle
	this->transformation.invalid = __VALID_TRANSFORMATION;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Container::propagateArguments(bool (*propagationHandler)(void*, va_list), va_list args)
{
	if(NULL == propagationHandler)
	{
		return false;
	}

	if(NULL != this->children)
	{
		for(VirtualNode node = this->children->head; NULL != node; node = node->next)
		{
			Container child = Container::safeCast(node->data);

			// Pass message to each child
			if(!child->deleteMe && Container::propagateArguments(child, propagationHandler, args))
			{
				return true;
			}
		}
	}

	// If no child processed the message, I process it
	return propagationHandler(this, args);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
