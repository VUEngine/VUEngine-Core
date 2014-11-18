/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Container.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__CLASS_DEFINITION(Container);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// pass event to children recursively
static int Container_passEvent(Container this, int (*event)(Container this, va_list args), va_list args);

// process removed children
static void Container_processRemovedChildren(Container this);
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(Container, __PARAMETERS(int ID))
__CLASS_NEW_END(Container, __ARGUMENTS(ID));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's conctructor
void Container_constructor(Container this, int ID){
	
	ASSERT(this, "Container::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);
	
	// set ID
	this->ID = ID;
	
	// set position
	this->transform.localPosition.x = 0;
	this->transform.localPosition.y = 0;
	this->transform.localPosition.z = 0;
	
	this->transform.globalPosition.x = 0;
	this->transform.globalPosition.y = 0;
	this->transform.globalPosition.z = 0;	
	
	// set scale
	this->transform.scale.x = 1;
	this->transform.scale.y = 1;
	
	// set rotation
	this->transform.rotation.x = 0;
	this->transform.rotation.y = 0;
	this->transform.rotation.z = 0;
	
	// force global position calculation on the next transform cycle
	this->invalidateGlobalPosition.x = true;
	this->invalidateGlobalPosition.y = true;
	this->invalidateGlobalPosition.z = true;
	
	this->parent = NULL;
	this->children = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Container_destructor(Container this){

	ASSERT(this, "Container::destructor: null this");

	// first remove any children removed
	Container_processRemovedChildren(this);
	
	// if I have children
	if(this->children){
		
		// create children list
		VirtualList childrenToDelete = __NEW(VirtualList);

		VirtualNode node = VirtualList_begin(this->children);
	
		// move each child to a temporary list
		for(; node ; node = VirtualNode_getNext(node)){
			
			Container child = (Container)VirtualNode_getData(node);
			
			VirtualList_pushBack(childrenToDelete, (void*)child);
		}	
		
		// delete children list
		__DELETE(this->children);
		this->children = NULL;

		node = VirtualList_begin(childrenToDelete);

		// destroy each child
		for(; node ; node = VirtualNode_getNext(node)){
			
			Container child = (Container)VirtualNode_getData(node);
			
			__DELETE(child);
		}	

		__DELETE(childrenToDelete);
	}
	
	// first remove from parent
	if(this->parent){
		
		Container_removeChild(this->parent, this);
	}
	
	// destroy the super Container
	__DESTROY_BASE(Object);								
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add a child Container
void Container_addChild(Container this, Container child){

	ASSERT(this, "Container::addChild: null this");

	// check if child is valid
	if(child){

		// if don't have any child yet
		if(!this->children){
			
			// create children list
			this->children = __NEW(VirtualList);
		}

		// first remove from previous parent
		if(child->parent){
			
			Container_removeChild(child->parent, child);
		}
		
		// add to the children list
		VirtualList_pushBack(this->children, (void*)child);
		//VirtualList_pushFront(this->children, (void*)child);
		
		// set new parent
		child->parent = this;
	}
	else{
		
		ASSERT(false, "Container::addChild: adding null child");
	}
}

static void Container_processRemovedChildren(Container this){
	
	ASSERT(this, "Container::processRemovedChildren: null this");

	if(this->children && this->removedChildren){
		
		VirtualNode node = VirtualList_begin(this->removedChildren);
	
		// remove each child
		for(; node ; node = VirtualNode_getNext(node)){
			
			Container child = (Container)VirtualNode_getData(node);

			VirtualList_removeElement(this->children, child);
		}	

		__DELETE(this->removedChildren);
		
		this->removedChildren = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove child Container
void Container_removeChild(Container this, Container child){
	
	ASSERT(this, "Container::removeChild: null this");

	// check if child is valid and if I'm its parent
	if(child && this == child->parent && this->children){

		// if don't have any children to remove yet
		if(!this->removedChildren){
			
			// create children list
			this->removedChildren = __NEW(VirtualList);
		}

		// register for removing
		VirtualList_pushBack(this->removedChildren, (void*)child);

		// set no parent
		child->parent = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update each Container's child
void Container_update(Container this){	
	
	ASSERT(this, "Container::update: null this");

	// first remove children
	Container_processRemovedChildren(this);
	
	// if I have children
	if(this->children){
	
		VirtualNode node = VirtualList_begin(this->children);
	
		// update each child
		for(; node ; node = VirtualNode_getNext(node)){
			
			__VIRTUAL_CALL(void, Container, update, (Container)VirtualNode_getData(node));
		}
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// contatenate transform
void Container_concatenateTransform(Transformation *environmentTransform, Transformation* transform){

	ASSERT(environmentTransform, "Container::concatenateTransform: null environmentTransform");
	ASSERT(transform, "Container::concatenateTransform: null transform");

	// tranlate position
	environmentTransform->globalPosition.x += transform->localPosition.x;
	environmentTransform->globalPosition.y += transform->localPosition.y;
	environmentTransform->globalPosition.z += transform->localPosition.z;
	
	// propagate scale
	environmentTransform->scale.x *= transform->scale.x;
	environmentTransform->scale.y *= transform->scale.y;
	
	// propagate rotation
	environmentTransform->rotation.x += transform->rotation.x;
	environmentTransform->rotation.y += transform->rotation.y;
	environmentTransform->rotation.z += transform->rotation.z;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//transform class
void Container_transform(Container this, Transformation* environmentTransform){

	ASSERT(this, "Container::transform: null this");

	Transformation environmentTransformCopy = {
		// local position
		{
			environmentTransform->localPosition.x, 
			environmentTransform->localPosition.y, 
			environmentTransform->localPosition.z
		},
		// global position
		{
			environmentTransform->globalPosition.x, 
			environmentTransform->globalPosition.y, 
			environmentTransform->globalPosition.z
		},
		// scale
		{
			environmentTransform->scale.x, 
			environmentTransform->scale.y
		},
		// rotation
		{
			environmentTransform->rotation.x, 
			environmentTransform->rotation.y, 
			environmentTransform->rotation.z
		}
	};
	
	// concaenate environment transform
	Container_concatenateTransform(&environmentTransformCopy, &this->transform);

	// save new global position
	this->transform.globalPosition = environmentTransformCopy.globalPosition;
	
	// if I have children
	if(this->children){
		
		VirtualNode node = VirtualList_begin(this->children);
	
		// update each child
		for(; node ; node = VirtualNode_getNext(node)){
			
			Container child = (Container)VirtualNode_getData(node);
			
			child->invalidateGlobalPosition = child->invalidateGlobalPosition.x || child->invalidateGlobalPosition.y || child->invalidateGlobalPosition.z? child->invalidateGlobalPosition: this->invalidateGlobalPosition;
			
			// transform each entity
			__VIRTUAL_CALL(void, Container, transform, child, __ARGUMENTS(&environmentTransformCopy));		
		}	
	}
	
	// don't update position on next transform cycle
	this->invalidateGlobalPosition.x = false;
	this->invalidateGlobalPosition.y = false;
	this->invalidateGlobalPosition.z = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve global position
VBVec3D Container_getGlobalPosition(Container this){

	ASSERT(this, "Container::getGlobalPosition: null this");

	return this->transform.globalPosition;  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve local position
VBVec3D Container_getLocalPosition(Container this){

	ASSERT(this, "Container::getLocalPosition: null this");

	return this->transform.localPosition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set class's local position
void Container_setLocalPosition(Container this, VBVec3D position){

	ASSERT(this, "Container::setLocalPosition: null this");

	// force global position calculation on the next transform cycle
	this->invalidateGlobalPosition.x = this->transform.localPosition.x != position.x;
	this->invalidateGlobalPosition.y = this->transform.localPosition.y != position.y;
	this->invalidateGlobalPosition.z = this->transform.localPosition.z != position.z;

	this->transform.localPosition = position;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// propagate an event to the children wrapper
void Container_propagateEvent(Container this, int (*event)(Container this, va_list args), ...){

	ASSERT(this, "Container::propagateEvent: null this");
	ASSERT(event, "Container::propagateEvent: null event");

	va_list args;
    va_start(args, event);
    Container_passEvent(this, event, args);
    va_end(args);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pass event to children recursively
static int Container_passEvent(Container this, int (*event)(Container this, va_list args), va_list args){
	
	ASSERT(this, "Container::passEvent: null this");
	
	// if event is valid
	if(event){
		
		// propagate if I have children
		if(this->children){
			
			VirtualNode node = VirtualList_begin(this->children);
			
			// update each child
			for(; node ; node = VirtualNode_getNext(node)){
				
				// pass event to each child
				if(Container_passEvent((Container)VirtualNode_getData(node), event, args)){
	
					return true;
				}
			}
		}
		
		// if no child processed the event, I process it
		return event(this, args);
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_onKeyPressed(Container this, va_list args){
	
	ASSERT(this, "Container::onKeyPressed: null this");

	int pressedKey = 0;
	pressedKey = va_arg(args, int);
	return __VIRTUAL_CALL(int, Container, doKeyPressed, this, __ARGUMENTS(pressedKey));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_onKeyUp(Container this, va_list args){

	ASSERT(this, "Container::onKeyUp: null this");

	int pressedKey = 0;
	pressedKey = va_arg(args, int);	
	return __VIRTUAL_CALL(int, Container, doKeyUp, this, __ARGUMENTS(pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_onKeyHold(Container this, va_list args){
	
	ASSERT(this, "Container::onKeyHold: null this");

	int pressedKey = 0;
	pressedKey = va_arg(args, int);	
	return __VIRTUAL_CALL(int, Container, doKeyHold, this, __ARGUMENTS(pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_doKeyPressed(Container this, int pressedKey){

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_doKeyUp(Container this, int pressedKey){
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_doKeyHold(Container this, int pressedKey){
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//retrieve class's in game index
int Container_getID(Container this){
	
	return this->ID;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve child count
int Container_getChildCount(Container this){

	return VirtualList_getSize(this->children);
}