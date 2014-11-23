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

#define __LEVEL_EDITOR

#ifdef __LEVEL_EDITOR

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <LevelEditor.h>
#include <Game.h>
#include <Optics.h>
#include <Globals.h>
#include <Entity.h>
#include <CollisionManager.h>
#include <SpriteManager.h>
#include <Level.h>
#include <Stage.h>
#include <Shape.h>
#include <Screen.h>
#include <Cuboid.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __USER_OBJECT_SHOW_ROW 	6

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define LevelEditor_ATTRIBUTES				\
											\
	/* super's attributes */				\
	Object_ATTRIBUTES;						\
											\
	/* current in game entity */			\
	Level level;							\
											\
	/* current in game entity */			\
	VirtualNode currentEntityNode;			\
											\
	/* current entity's shape */			\
	Shape shape;							\
											\
	/* mode */								\
	int mode;								\
											\
	/* user object index */					\
	int userObjectIndex;					\


// define the LevelEditor
__CLASS_DEFINITION(LevelEditor);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												  MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __TRANSLATION_STEP	8
#define __SCREEN_X_TRANSLATION_STEP		__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP		__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP		__SCREEN_HEIGHT / 4


enum Modes {
		kFirstMode = 0,
		kMoveScreen,
		kTranslateEntities,
		kAddObjects,
		
		kLastMode
};

extern UserObject _userObjects[];

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

static void LevelEditor_constructor(LevelEditor this);

static void LevelEditor_setupMode(LevelEditor this);
static void LevelEditor_releaseShape(LevelEditor this);
static void LevelEditor_getShape(LevelEditor this);
static void LevelEditor_positioneShape(LevelEditor this);
static void LevelEditor_highLightEntity(LevelEditor this);
static void LevelEditor_selectPreviousEntity(LevelEditor this);
static void LevelEditor_selectNextEntity(LevelEditor this);
static void LevelEditor_traslateEntity(LevelEditor this, u16 pressedKey);
static void LevelEditor_moveScreen(LevelEditor this, u16 pressedKey);
static void LevelEditor_applyTraslationToEntity(LevelEditor this, VBVec3D translation);
static void LevelEditor_applyTraslationToScreen(LevelEditor this, VBVec3D translation);
static void LevelEditor_printEntityPosition(LevelEditor this);
static void LevelEditor_printScreenPosition(LevelEditor this);
static void LevelEditor_printUserObjects(LevelEditor this);
static void LevelEditor_selectUserObject(LevelEditor this, u16 pressedKey);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
__SINGLETON(LevelEditor);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void LevelEditor_constructor(LevelEditor this){

	ASSERT(this, "LevelEditor::constructor: null this");

	__CONSTRUCT_BASE(Object);
	
	this->currentEntityNode = NULL;

	this->level = NULL;
	
	this->mode = kFirstMode + 1;
	this->userObjectIndex = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void LevelEditor_destructor(LevelEditor this){
	
	ASSERT(this, "LevelEditor::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update
void LevelEditor_update(LevelEditor this){
	
	if(this->shape) {
		
		__VIRTUAL_CALL(void, Shape, draw, this->shape);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show debug screens
void LevelEditor_start(LevelEditor this){
	
	LevelEditor_setupMode(this);
	this->level = Game_getLevel(Game_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print title
static void LevelEditor_setupMode(LevelEditor this) {
	
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	Printing_text("LEVEL EDITOR", 17, 0);
	Printing_text("Navigate (B)", 33, 0);
	Printing_text("Use (L/R pads)", 33, 1);
	Printing_text("Position: x    y    z", 1, 3);

	switch(this->mode) {
	
		case kMoveScreen:

			LevelEditor_printScreenPosition(this);
			break;

		case kTranslateEntities:

			if(!this->currentEntityNode) {

				LevelEditor_selectNextEntity(this);
			}
			else{

				LevelEditor_getShape(this);
				LevelEditor_highLightEntity(this);
			}

			LevelEditor_printEntityPosition(this);
			break;
			
		case kAddObjects:

			this->userObjectIndex = 0;
			LevelEditor_printUserObjects(this);

			//Entity Entity_load(EntityDefinition* entityDefinition, VBVec3D* position, int ID, void* extraInfo){
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// hide debug screens
void LevelEditor_stop(LevelEditor this){

	CollisionManager_flushShapesDirectDrawData(CollisionManager_getInstance());
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	LevelEditor_releaseShape(this);
	this->currentEntityNode = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int LevelEditor_handleMessage(LevelEditor this, Telegram telegram){
	
	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:	
			{
				u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

				if(pressedKey & K_B){
					
					this->mode++;
					
					if(kLastMode <= this->mode) {
						
						this->mode = kFirstMode + 1;
					}
					
					LevelEditor_setupMode(this);
										
					break;
				}
				
				switch(this->mode) {
				
					case kMoveScreen:

						LevelEditor_moveScreen(this, pressedKey);
						break;

					case kTranslateEntities:

						LevelEditor_traslateEntity(this, pressedKey);
						break;
					
					case kAddObjects:
						
						LevelEditor_selectUserObject(this, pressedKey);
						break;
				}
			}
			break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_releaseShape(LevelEditor this) {

	if(this->currentEntityNode) {
		
		Entity entity = (Entity)VirtualNode_getData(this->currentEntityNode);
	
		if(this->shape && this->shape != __VIRTUAL_CALL_UNSAFE(Shape, Entity, getShape, (Entity)entity)) {
			
			__DELETE(this->shape);
			
			this->shape = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_getShape(LevelEditor this) {
	
	if(!this->currentEntityNode){
	
		return;
	}
	
	Entity entity = (Entity)VirtualNode_getData(this->currentEntityNode);

	this->shape = __VIRTUAL_CALL_UNSAFE(Shape, Entity, getShape, (Entity)entity);
	
	if(!this->shape) {
		
		switch(__VIRTUAL_CALL(int, Entity, getShapeType, entity)){
		
			case kCircle:
				
				//VirtualList_pushBack(this->shapes, (void*)__NEW(Circle, __ARGUMENTS(owner)));			
				break;

			case kCuboid:

				this->shape = (Shape)__NEW(Cuboid, __ARGUMENTS(entity));
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_positioneShape(LevelEditor this) {
	
	if(!this->currentEntityNode || !this->shape){
	
		return;
	}
	
	Entity entity = (Entity)VirtualNode_getData(this->currentEntityNode);

	__VIRTUAL_CALL(void, Shape, setup, this->shape);
	
	Shape_setReady(this->shape, false);
	
	if(__VIRTUAL_CALL(int, Entity, moves, entity)) {
	
		__VIRTUAL_CALL(void, Shape, positione, this->shape);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_highLightEntity(LevelEditor this){
	
	if(this->currentEntityNode){
		
		LevelEditor_printEntityPosition(this);
		LevelEditor_positioneShape(this);
	}
	else {
		
		Printing_text("No entities in stage", 1, 4);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// select the next entity
static void LevelEditor_selectPreviousEntity(LevelEditor this){
	
	LevelEditor_releaseShape(this);

	VirtualList stageEntities = Container_getChildren((Container)Level_getStage(this->level));

	if(!this->currentEntityNode) {
		
		this->currentEntityNode = stageEntities? VirtualList_end(stageEntities): NULL;
	}
	else {

		this->currentEntityNode = VirtualNode_getPrevious(this->currentEntityNode);

		if(!this->currentEntityNode) {
			
			this->currentEntityNode = stageEntities? VirtualList_end(stageEntities): NULL;
		}
	}
	
	if(this->currentEntityNode) {
		
		LevelEditor_getShape(this);
		LevelEditor_highLightEntity(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// select the next entity
static void LevelEditor_selectNextEntity(LevelEditor this){
	
	LevelEditor_releaseShape(this);

	VirtualList stageEntities = Container_getChildren((Container)Level_getStage(this->level));

	if(!this->currentEntityNode) {
		
		this->currentEntityNode = stageEntities? VirtualList_begin(stageEntities): NULL;
	}
	else {
		
		this->currentEntityNode = VirtualNode_getNext(this->currentEntityNode);
		
		if(!this->currentEntityNode) {
			
			this->currentEntityNode = stageEntities? VirtualList_begin(stageEntities): NULL;
		}
	}

	if(this->currentEntityNode) {
		
		LevelEditor_getShape(this);
		LevelEditor_highLightEntity(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// move screen
static void LevelEditor_moveScreen(LevelEditor this, u16 pressedKey){
	
	if(pressedKey & K_LL){
		
		VBVec3D translation = {
				
				ITOFIX19_13(-__SCREEN_X_TRANSLATION_STEP),
				0, 
				0
		};

		LevelEditor_applyTraslationToScreen(this, translation);
	}
	else if(pressedKey & K_LR){
		
		VBVec3D translation = {
				
				ITOFIX19_13(__SCREEN_X_TRANSLATION_STEP),
				0, 
				0
		};

		LevelEditor_applyTraslationToScreen(this, translation);
	}
	else if(pressedKey & K_LU){
		
		VBVec3D translation = {
				0,
				ITOFIX19_13(-__SCREEN_Y_TRANSLATION_STEP),
				0
		};

		LevelEditor_applyTraslationToScreen(this, translation);
	}
	else if(pressedKey & K_LD){
		
		VBVec3D translation = {
				0,
				ITOFIX19_13(__SCREEN_Y_TRANSLATION_STEP),
				0
		};

		LevelEditor_applyTraslationToScreen(this, translation);
	}
	else if(pressedKey & K_RU){
		
		VBVec3D translation = {
				0,
				0,
				ITOFIX19_13(__SCREEN_Z_TRANSLATION_STEP),
		};

		LevelEditor_applyTraslationToScreen(this, translation);
	}
	else if(pressedKey & K_RD){
		
		VBVec3D translation = {
				0,
				0,
				ITOFIX19_13(-__SCREEN_Z_TRANSLATION_STEP),
		};

		LevelEditor_applyTraslationToScreen(this, translation);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// translate entity
static void LevelEditor_traslateEntity(LevelEditor this, u16 pressedKey){
	
	if(pressedKey & K_LL){
		
		VBVec3D translation = {
				
				ITOFIX19_13(-__TRANSLATION_STEP),
				0, 
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_LR){
		
		VBVec3D translation = {
				
				ITOFIX19_13(__TRANSLATION_STEP),
				0, 
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_LU){
		
		VBVec3D translation = {
				0,
				ITOFIX19_13(-__TRANSLATION_STEP),
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_LD){
		
		VBVec3D translation = {
				0,
				ITOFIX19_13(__TRANSLATION_STEP),
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_RL){

		// displace entity
	}
	else if(pressedKey & K_RR){
		
	}
	else if(pressedKey & K_RU){
		
		VBVec3D translation = {
				0,
				0,
				ITOFIX19_13(__TRANSLATION_STEP),
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_RD){
		
		VBVec3D translation = {
				0,
				0,
				ITOFIX19_13(-__TRANSLATION_STEP),
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_LT) {
		
		LevelEditor_selectPreviousEntity(this);
	}
	else if(pressedKey & K_RT) {

		LevelEditor_selectNextEntity(this);
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_applyTraslationToEntity(LevelEditor this, VBVec3D translation){
	
	if(this->currentEntityNode && this->shape){
		
		Container container = (Container)VirtualNode_getData(this->currentEntityNode);
		VBVec3D localPosition = Container_getLocalPosition(container);
		
		localPosition.x += translation.x;
		localPosition.y += translation.y;
		localPosition.z += translation.z;

		__VIRTUAL_CALL(void, Container, setLocalPosition, container, __ARGUMENTS(localPosition));
		
		Level_transform(this->level);

		// synchronize container's shape
		LevelEditor_positioneShape(this);

		LevelEditor_printEntityPosition(this);
		
		SpriteManager_sortAllLayers(SpriteManager_getInstance());
		
		// should work
		//__VIRTUAL_CALL(void, Shape, positione, this->shape);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_selectUserObject(LevelEditor this, u16 pressedKey) {
	
	int userObjecstCount = 0;
	for(; _userObjects[userObjecstCount].entityDefinition; userObjecstCount++);

	if(pressedKey & K_LU){
	
		Printing_text(" ", 1, __USER_OBJECT_SHOW_ROW + this->userObjectIndex);

		if(0 > --this->userObjectIndex) {
			
			this->userObjectIndex = userObjecstCount - 1;
		}
		
		Printing_text("*", 1, __USER_OBJECT_SHOW_ROW + this->userObjectIndex);
	}
	else if(pressedKey & K_LD){
		
		Printing_text(" ", 1, __USER_OBJECT_SHOW_ROW + this->userObjectIndex);

		if(userObjecstCount <= ++this->userObjectIndex) {
			
			this->userObjectIndex = 0;
		}
		
		Printing_text("*", 1, __USER_OBJECT_SHOW_ROW + this->userObjectIndex);
	}
	else if(pressedKey & K_A) {
		
		VBVec3D position = Screen_getPosition(Screen_getInstance());
		
		position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
		position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
		position.z += ITOFIX19_13(__SCREEN_WIDTH >> 2);
		
		Entity entity = Stage_addEntity(Level_getStage(this->level), _userObjects[this->userObjectIndex].entityDefinition, &position, -1, NULL);
		SpriteManager_sortAllLayers(SpriteManager_getInstance());

		Level_transform(this->level);
		__VIRTUAL_CALL(void, Container, setLocalPosition, (Container)entity, __ARGUMENTS(position));
					
		VirtualList stageEntities = Container_getChildren((Container)Level_getStage(this->level));
		this->currentEntityNode = stageEntities? VirtualList_end(stageEntities): NULL;

		// select the added entity
		this->mode = kTranslateEntities;
		LevelEditor_setupMode(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_printEntityPosition(LevelEditor this){
	
	Printing_text("Entity's type:                       ", 1, 2);

	if(this->currentEntityNode){
		
		Container container = (Container)VirtualNode_getData(this->currentEntityNode);
		Printing_text(__GET_CLASS_NAME(container), 16, 2);
		VBVec3D globalPosition = Container_getGlobalPosition(container);
		Printing_text("                   ", 11, 4);
		Printing_int(FIX19_13TOI(globalPosition.x), 11, 4);
		Printing_int(FIX19_13TOI(globalPosition.y), 16, 4);
		Printing_int(FIX19_13TOI(globalPosition.z), 21, 4);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_applyTraslationToScreen(LevelEditor this, VBVec3D translation){
	
	this->currentEntityNode = NULL;

	Screen_move(Screen_getInstance(), translation);
	Level_transform(this->level);

	LevelEditor_printScreenPosition(this);

	Stage_streamAll(Level_getStage(this->level));

	SpriteManager_sortAllLayers(SpriteManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_printScreenPosition(LevelEditor this){
	
	Printing_text("Screen's  ", 1, 2);
	VBVec3D position = Screen_getPosition(Screen_getInstance());
	Printing_text("                   ", 11, 4);
	Printing_int(FIX19_13TOI(position.x), 11, 4);
	Printing_int(FIX19_13TOI(position.y), 16, 4);
	Printing_int(FIX19_13TOI(position.z), 21, 4);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_printUserObjects(LevelEditor this){

	Printing_text("User's objects  ", 1, 2);
	Printing_text("                       ", 1, 3);
	Printing_text("Add (A)", 33, 0);

	int y = 3;
	
	Printing_text("Name", 1, ++y);
	y++;
	
	int i = 0;
	
	y = __USER_OBJECT_SHOW_ROW;
	
	for(; _userObjects[i].entityDefinition; i++) {
		
		Printing_text(_userObjects[i].name, 2, y++);
	}
	
	Printing_text("*", 1, __USER_OBJECT_SHOW_ROW + this->userObjectIndex);
	
}

#endif 
