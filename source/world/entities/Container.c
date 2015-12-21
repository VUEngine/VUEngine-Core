/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <string.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(Container, SpatialObject);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static int Container_passEvent(Container this, int (*event)(Container this, va_list args), va_list args);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Container, s16 id, const char* const name)
__CLASS_NEW_END(Container, id, name);

// class's constructor
void Container_constructor(Container this, s16 id, const char* const name)
{
	ASSERT(this, "Container::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE();

	// set ID
	this->id = id;

	// set position
	this->transform.localPosition.x = 0;
	this->transform.localPosition.y = 0;
	this->transform.localPosition.z = 0;

	this->transform.globalPosition.x = 0;
	this->transform.globalPosition.y = 0;
	this->transform.globalPosition.z = 0;

	// set rotation
	this->transform.localRotation.x = 0;
	this->transform.localRotation.y = 0;
	this->transform.localRotation.z = 0;

	this->transform.globalRotation.x = 0;
	this->transform.globalRotation.y = 0;
	this->transform.globalRotation.z = 0;

	// set scale
	this->transform.localScale.x = ITOFIX7_9(1);
	this->transform.localScale.y = ITOFIX7_9(1);

	this->transform.globalScale.x = ITOFIX7_9(1);
	this->transform.globalScale.y = ITOFIX7_9(1);

	// force global position calculation on the next transform cycle
	this->invalidateGlobalPosition.x = true;
	this->invalidateGlobalPosition.y = true;
	this->invalidateGlobalPosition.z = true;

	this->parent = NULL;
	this->children = NULL;
	this->removedChildren = NULL;
	this->deleteMe = false;
	
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
		VirtualList childrenToDelete = __NEW(VirtualList);
		VirtualList_copy(childrenToDelete, this->children);
		
		// delete children list
		__DELETE(this->children);
		this->children = NULL;

		VirtualNode node = VirtualList_begin(childrenToDelete);

		// destroy each child
		for(; node ; node = VirtualNode_getNext(node))
	    {
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));

			ASSERT(child->parent == this, "Container::destructor: deleting a child of not mine");
			child->parent = NULL;
			__DELETE(child);
		}

		__DELETE(childrenToDelete);
	}

	// first remove from parent
	if(this->parent)
	{
		Container_removeChild(this->parent, this);
	}
	
	// delete name
	if(this->name)
	{
		__DELETE_BASIC(this->name);
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
		__VIRTUAL_CALL(void, Container, removeChild, this->parent, this);
	}
	else
	{
		NM_ASSERT(false, "Container::deleteMyself: I'm orphan");
	}
}

// add a child Container
void Container_addChild(Container this, Container child)
{
	ASSERT(this, "Container::addChild: null this");

	// check if child is valid
	if(child)
	{
		// if don't have any child yet
		if(!this->children)
	    {
			// create children list
			this->children = __NEW(VirtualList);
		}

		// first remove from previous parent
		if(child->parent)
	    {
			Container_removeChild(child->parent, child);
		}

		// add to the children list
		VirtualList_pushBack(this->children, (void*)child);

		// set new parent
		child->parent = this;
	}
	else
	{
		ASSERT(false, "Container::addChild: adding null child");
	}
}

// remove child Container
void Container_removeChild(Container this, Container child)
{
	ASSERT(this, "Container::removeChild: null this");

	// check if child is valid and if I'm its parent
	if(child && this == child->parent && this->children)
	{
		// if don't have any children to remove yet
		if(!this->removedChildren)
	    {
			// create children list
			this->removedChildren = __NEW(VirtualList);
		}

		// register for removing
		VirtualList_pushBack(this->removedChildren, (void*)child);

		// set no parent
		child->parent = NULL;
	}
}

// process removed children
void Container_processRemovedChildren(Container this)
{
	ASSERT(this, "Container::processRemovedChildren: null this");

	if(this->children && this->removedChildren)
	{
		VirtualNode node = VirtualList_begin(this->removedChildren);

		// remove each child
		for(; node ; node = VirtualNode_getNext(node))
	    {
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));

			VirtualList_removeElement(this->children, child);
			
			if(child->deleteMe)
			{
				__DELETE(child);
			}
		}

		__DELETE(this->removedChildren);

		this->removedChildren = NULL;
	}
}

// update each Container's child
void Container_update(Container this)
{
	ASSERT(this, "Container::update: null this");

	// if I have children
	if(this->children)
	{
		// first remove children
		Container_processRemovedChildren(this);

		VirtualNode node = VirtualList_begin(this->children);

		// update each child
		for(; node ; node = VirtualNode_getNext(node))
	    {
			__VIRTUAL_CALL(void, Container, update, VirtualNode_getData(node));
		}
	}
}

// retrieve environment transformation
Transformation Container_getEnvironmentTransform(Container this)
{
	ASSERT(this, "Container::getEnvironmentTransform: null this");

	if(this->parent)
	{
		Transformation transformation = Container_getEnvironmentTransform(this->parent);

		Container_concatenateTransform(&transformation, &this->transform);

		return transformation;
	}

	// static to avoid call to _memcpy
	static Transformation environmentTransform =
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
			{ITOFIX7_9(1), ITOFIX7_9(1)},
			// global scale
			{ITOFIX7_9(1), ITOFIX7_9(1)}
	};

	Container_concatenateTransform(&environmentTransform, &this->transform);

	return environmentTransform;
}

// contatenate transform
void Container_concatenateTransform(Transformation* environmentTransform, Transformation* transform)
{
	ASSERT(environmentTransform, "Container::concatenateTransform: null environmentTransform");
	ASSERT(transform, "Container::concatenateTransform: null transform");

	// tranlate position
	environmentTransform->globalPosition.x += transform->localPosition.x;
	environmentTransform->globalPosition.y += transform->localPosition.y;
	environmentTransform->globalPosition.z += transform->localPosition.z;

	// propagate rotation
	environmentTransform->globalRotation.x += transform->localRotation.x;
	environmentTransform->globalRotation.y += transform->localRotation.y;
	environmentTransform->globalRotation.z += transform->localRotation.z;
	
	// propagate scale
	environmentTransform->globalScale.x = FIX7_9_MULT(environmentTransform->globalScale.x, transform->localScale.x);
	environmentTransform->globalScale.y = FIX7_9_MULT(environmentTransform->globalScale.y, transform->localScale.y);
}

// initial transform
void Container_initialTransform(Container this, Transformation* environmentTransform)
{
	ASSERT(this, "Container::initialTransform: null this");
	
	// concatenate transform
	this->transform.globalPosition.x = environmentTransform->globalPosition.x + this->transform.localPosition.x;
	this->transform.globalPosition.y = environmentTransform->globalPosition.y + this->transform.localPosition.y;
	this->transform.globalPosition.z = environmentTransform->globalPosition.z + this->transform.localPosition.z;

	// propagate rotation
	this->transform.globalRotation.x = environmentTransform->globalRotation.x + this->transform.localRotation.x;
	this->transform.globalRotation.y = environmentTransform->globalRotation.x + this->transform.localRotation.y;
	this->transform.globalRotation.z = environmentTransform->globalRotation.x + this->transform.localRotation.z;
	
	// propagate scale
	this->transform.globalScale.x = FIX7_9_MULT(environmentTransform->globalScale.x, this->transform.localScale.x);
	this->transform.globalScale.y = FIX7_9_MULT(environmentTransform->globalScale.y, this->transform.localScale.y);

	// if I have children
	if(this->children)
	{
		// first remove children
		Container_processRemovedChildren(this);

		VirtualNode node = VirtualList_begin(this->children);

		// update each child
		for(; node; node = VirtualNode_getNext(node))
		{
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));

			child->invalidateGlobalPosition = child->invalidateGlobalPosition.x || child->invalidateGlobalPosition.y || child->invalidateGlobalPosition.z ? child->invalidateGlobalPosition : this->invalidateGlobalPosition;

			__VIRTUAL_CALL(void, Container, initialTransform, child, &this->transform);
		}
	}

	Container_invalidateGlobalPosition(this);
}

// initial transform but don't call the virtual method
void Container_transformNonVirtual(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::transform: null this");

	// concatenate transform
	this->transform.globalPosition.x = environmentTransform->globalPosition.x + this->transform.localPosition.x;
	this->transform.globalPosition.y = environmentTransform->globalPosition.y + this->transform.localPosition.y;
	this->transform.globalPosition.z = environmentTransform->globalPosition.z + this->transform.localPosition.z;

	// propagate rotation
	this->transform.globalRotation.x = environmentTransform->globalRotation.x + this->transform.localRotation.x;
	this->transform.globalRotation.y = environmentTransform->globalRotation.x + this->transform.localRotation.y;
	this->transform.globalRotation.z = environmentTransform->globalRotation.x + this->transform.localRotation.z;
	
	// propagate scale
	this->transform.globalScale.x = FIX7_9_MULT(environmentTransform->globalScale.x, this->transform.localScale.x);
	this->transform.globalScale.y = FIX7_9_MULT(environmentTransform->globalScale.y, this->transform.localScale.y);

	// if I have children
	if(this->children)
	{
		// first remove children
		//Container_processRemovedChildren(this);

		VirtualNode node = VirtualList_begin(this->children);

		// update each child
		for(; node; node = VirtualNode_getNext(node))
		{
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));

			child->invalidateGlobalPosition.x = child->invalidateGlobalPosition.x || this->invalidateGlobalPosition.x;
			child->invalidateGlobalPosition.y = child->invalidateGlobalPosition.y || this->invalidateGlobalPosition.y;
			child->invalidateGlobalPosition.z = child->invalidateGlobalPosition.z || this->invalidateGlobalPosition.z;

			Container_transformNonVirtual(child, &this->transform);
		}
	}

	// don't update position on next transform cycle
	this->invalidateGlobalPosition.x = false;
	this->invalidateGlobalPosition.y = false;
	this->invalidateGlobalPosition.z = false;
}

// initial transform
void Container_transform(Container this, const Transformation* environmentTransform)
{
	ASSERT(this, "Container::transform: null this");

	// concatenate transform
	this->transform.globalPosition.x = environmentTransform->globalPosition.x + this->transform.localPosition.x;
	this->transform.globalPosition.y = environmentTransform->globalPosition.y + this->transform.localPosition.y;
	this->transform.globalPosition.z = environmentTransform->globalPosition.z + this->transform.localPosition.z;

	// propagate rotation
	this->transform.globalRotation.x = environmentTransform->globalRotation.x + this->transform.localRotation.x;
	this->transform.globalRotation.y = environmentTransform->globalRotation.x + this->transform.localRotation.y;
	this->transform.globalRotation.z = environmentTransform->globalRotation.x + this->transform.localRotation.z;
	
	// propagate scale
	this->transform.globalScale.x = FIX7_9_MULT(environmentTransform->globalScale.x, this->transform.localScale.x);
	this->transform.globalScale.y = FIX7_9_MULT(environmentTransform->globalScale.y, this->transform.localScale.y);

	// if I have children
	if(this->children)
	{
		// first remove children
		//Container_processRemovedChildren(this);

		VirtualNode node = VirtualList_begin(this->children);

		// update each child
		for(; node; node = VirtualNode_getNext(node))
		{
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));

			child->invalidateGlobalPosition.x = child->invalidateGlobalPosition.x || this->invalidateGlobalPosition.x;
			child->invalidateGlobalPosition.y = child->invalidateGlobalPosition.y || this->invalidateGlobalPosition.y;
			child->invalidateGlobalPosition.z = child->invalidateGlobalPosition.z || this->invalidateGlobalPosition.z;

			__VIRTUAL_CALL(void, Container, transform, child, &this->transform);
		}
	}

	// don't update position on next transform cycle
	this->invalidateGlobalPosition.x = false;
	this->invalidateGlobalPosition.y = false;
	this->invalidateGlobalPosition.z = false;
}

// retrieve global position
const VBVec3D* Container_getGlobalPosition(Container this)
{
	ASSERT(this, "Container::getGlobalPosition: null this");

	return &this->transform.globalPosition;
}

// invalidate global position
static void Container_propagateInvalidateGlobalPosition(Container this)
{
	if(this->children)
	{
		VirtualNode node = VirtualList_begin(this->children);

		// update each child
		for(; node; node = VirtualNode_getNext(node))
		{
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));

			child->invalidateGlobalPosition.x |= this->invalidateGlobalPosition.x;
			child->invalidateGlobalPosition.y |= this->invalidateGlobalPosition.y;
			child->invalidateGlobalPosition.z |= this->invalidateGlobalPosition.z;

			// make sure children recalculates its global position
			Container_propagateInvalidateGlobalPosition(child);
		}
	}
}

// retrieve local position
const VBVec3D* Container_getLocalPosition(Container this)
{
	ASSERT(this, "Container::getLocalPosition: null this");

	return &this->transform.localPosition;
}

//set class's local position
void Container_setLocalPosition(Container this, const VBVec3D* position)
{
	ASSERT(this, "Container::setLocalPosition: null this");

	// force global position calculation on the next transform cycle
	this->invalidateGlobalPosition.x = this->transform.localPosition.x != position->x;
	this->invalidateGlobalPosition.y = this->transform.localPosition.y != position->y;
	this->invalidateGlobalPosition.z = this->transform.localPosition.z != position->z;

	this->transform.localPosition = *position;

	if(this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z)
	{
		Container_propagateInvalidateGlobalPosition(this);
	}
}

const Rotation* Container_getLocalRotation(Container this)
{
	ASSERT(this, "Container::getLocalRotation: null this");

	return &this->transform.localRotation;
}

//set class's local position
void Container_setLocalRotation(Container this, const Rotation* rotation)
{
	ASSERT(this, "Container::setLocalRotation: null this");

	this->transform.localRotation = *rotation;
	
	Container_invalidateGlobalPosition(this);
}

const Scale* Container_getLocalScale(Container this)
{
	ASSERT(this, "Container::getLocalScale: null this");

	return &this->transform.localScale;
}

//set class's local position
void Container_setLocalScale(Container this, const Scale* scale)
{
	ASSERT(this, "Container::invalidateGlobalPosition: null this");

	this->transform.localScale = *scale;
	
	Container_invalidateGlobalPosition(this);
}

// invalidate global position
void Container_invalidateGlobalPosition(Container this)
{
	ASSERT(this, "Container::invalidateGlobalPosition: null this");

	this->invalidateGlobalPosition.x = this->invalidateGlobalPosition.y = this->invalidateGlobalPosition.z = true;

	if(this->children)
	{
		VirtualNode node = VirtualList_begin(this->children);

		// update each child
		for(; node; node = VirtualNode_getNext(node))
		{
			// make sure children recalculates its global position
			Container_invalidateGlobalPosition(__SAFE_CAST(Container, VirtualNode_getData(node)));
		}
	}
}

// propagate an event to the children wrapper
int Container_propagateEvent(Container this, int (*event)(Container this, va_list args), ...)
{
	ASSERT(this, "Container::propagateEvent: null this");
	ASSERT(event, "Container::propagateEvent: null event");

	va_list args;
    va_start(args, event);
    int result = Container_passEvent(this, event, args);
    va_end(args);

    return result;
}

// pass event to children recursively
static int Container_passEvent(Container this, int (*event)(Container this, va_list args), va_list args)
{
	ASSERT(this, "Container::passEvent: null this");

	// if event is valid
	if(event)
	{
		// propagate if I have children
		if(this->children)
    	{
			// first remove children
			Container_processRemovedChildren(this);

			VirtualNode node = VirtualList_begin(this->children);

			// update each child
			for(; node ; node = VirtualNode_getNext(node))
	        {
				// pass event to each child
				if(Container_passEvent(__SAFE_CAST(Container, VirtualNode_getData(node)), event, args))
	            {
					return true;
				}
			}
		}

		// if no child processed the event, I process it
		return event(this, args);
	}

	return false;
}

// process user input
int Container_onMessage(Container this, va_list args)
{
	ASSERT(this, "Container::onKeyHold: null this");

	int message = va_arg(args, int);
	return __VIRTUAL_CALL(int, Container, doMessage, this, message);
}

// process message
int Container_doMessage(Container this, int message)
{
	ASSERT(this, "Container::doMessage: null this");

	return false;
}

//retrieve class's in game index
s16 Container_getId(Container this)
{
	ASSERT(this, "Container::doMessage: null this");

	return this->id;
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
		
	}NameWrapper;
	
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

    Container child, grandChild;
    VirtualNode node = VirtualList_begin(children);

    // first remove children
    Container_processRemovedChildren(this);

    // look through all children
    for(; node ; node = VirtualNode_getNext(node))
    {
        child = __SAFE_CAST(Container, VirtualNode_getData(node));

        if(child->name && !strncmp(childName, child->name, __MAX_CONTAINER_NAME_LENGTH))
        {
            return child;
        }
        else if(recursive && child->children)
        {
            grandChild = Container_findChildByName(this, child->children, childName, recursive);
            if(grandChild) {
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

	if(childName && this->children)
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

// get child by id
Container Container_getChildById(Container this, s16 id)
{
	ASSERT(this, "Container::getChildById: null this");

	if(this->children)
	{
		// first remove children
		Container_processRemovedChildren(this);

		VirtualNode node = VirtualList_begin(this->children);

		// look through all children
		for(; node ; node = VirtualNode_getNext(node))
        {
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));

			if(child->id == id)
			{
				return child;
			}
        }
	}
	
	return NULL;
}

// suspend for pause
void Container_suspend(Container this)
{
	ASSERT(this, "Container::suspend: null this");

	if(this->children)
	{
		Container_processRemovedChildren(this);

		VirtualNode node = VirtualList_begin(this->children);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));
			
			__VIRTUAL_CALL(void, Container, suspend, child);
		}
	}
}

// resume after pause
void Container_resume(Container this)
{
	ASSERT(this, "Container::resume: null this");

	if(this->children)
	{
		VirtualNode node = VirtualList_begin(this->children);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			Container child = __SAFE_CAST(Container, VirtualNode_getData(node));
			
			__VIRTUAL_CALL(void, Container, resume, child);
		}
	}
	
	// force translation recalculations
	Container_invalidateGlobalPosition(this);
}

