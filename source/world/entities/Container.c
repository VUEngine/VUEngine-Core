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
	
	// force global position calculation on the next render cycle
	this->invalidateGlobalPosition = true;
	
	this->parent = NULL;
	this->children = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Container_destructor(Container this){

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
		
		ASSERT(false, Container: adding NULL child);
	}
}

static void Container_processRemovedChildren(Container this){
	
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
//render class
void Container_render(Container this, Transformation environmentTransform){

	// concaenate environment transform
	Container_concatenateTransform(&environmentTransform, &this->transform);

	// save new global position
	this->transform.globalPosition = environmentTransform.globalPosition;
	
	// if I have children
	if(this->children){
		
		VirtualNode node = VirtualList_begin(this->children);
	
		// update each child
		for(; node ; node = VirtualNode_getNext(node)){
			
			Container child = (Container)VirtualNode_getData(node);
			
			child->invalidateGlobalPosition = child->invalidateGlobalPosition? child->invalidateGlobalPosition: this->invalidateGlobalPosition;
			
			// render each entity
			__VIRTUAL_CALL(void, Container, render, child, __ARGUMENTS(environmentTransform));		
		}	
	}
	
	// don't update position on next render cycle
	this->invalidateGlobalPosition = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve global position
VBVec3D Container_getGlobalPosition(Container this){

	return this->transform.globalPosition;  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve local position
VBVec3D Container_getLocalPosition(Container this){

	return this->transform.localPosition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set class's local position
void Container_setLocalPosition(Container this, VBVec3D position){

	this->transform.localPosition = position;
	
	// force global position calculation on the next render cycle
	this->invalidateGlobalPosition = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// propagate an event to the children wrapper
void Container_propagateEvent(Container this, int (*event)(Container this, va_list args), ...){

	va_list args;
    va_start(args, event);
    Container_passEvent(this, event, args);
    va_end(args);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pass event to children recursively
static int Container_passEvent(Container this, int (*event)(Container this, va_list args), va_list args){

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
	
	int pressedKey = 0;
	pressedKey = va_arg(args, int);
	return __VIRTUAL_CALL(int, Container, doKeyPressed, this, __ARGUMENTS(pressedKey));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_onKeyUp(Container this, va_list args){

	int pressedKey = 0;
	pressedKey = va_arg(args, int);	
	return __VIRTUAL_CALL(int, Container, doKeyUp, this, __ARGUMENTS(pressedKey));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process user input
int Container_onKeyHold(Container this, va_list args){
	
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
