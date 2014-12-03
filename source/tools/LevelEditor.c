/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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
#include <Entity.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <SpriteManager.h>
#include <Level.h>
#include <Stage.h>
#include <Shape.h>
#include <Screen.h>
#include <Cuboid.h>
#include <OptionsSelector.h>


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
	/* actors selector */					\
	OptionsSelector userObjectsSelector;	\
											\
	/* translation step size */				\
	int translationStepSize;				\


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

#define __MAX_TRANSLATION_STEP	8 * 4
#define __SCREEN_X_TRANSLATION_STEP		__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP		__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP		__SCREEN_HEIGHT / 4

#define __HVPC_STEP						ITOFIX19_13(8)
#define __VVPC_STEP						ITOFIX19_13(8)
#define __DISTANCE_EYE_SCREEN_STEP		ITOFIX19_13(8)
#define __MAXIMUM_VIEW_DISTACE_STEP		ITOFIX19_13(8)
#define __BASE_DISTACE_STEP				ITOFIX19_13(8)

enum Modes {
		kFirstMode = 0,
		kMoveScreen,
		kChangeProjection,
		kTranslateEntities,
		kAddObjects,
		
		kLastMode
};


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// globals
extern Optical* _optical;
extern UserObject _userObjects[];
extern MovementState* _screenMovementState;


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
static void LevelEditor_changeProjection(LevelEditor this, u16 pressedKey);
static void LevelEditor_applyTraslationToEntity(LevelEditor this, VBVec3D translation);
static void LevelEditor_applyTraslationToScreen(LevelEditor this, VBVec3D translation);
static void LevelEditor_printEntityPosition(LevelEditor this);
static void LevelEditor_printScreenPosition(LevelEditor this);
static void LevelEditor_printProjectionValues(LevelEditor this);
static void LevelEditor_printUserObjects(LevelEditor this);
static void LevelEditor_selectUserObject(LevelEditor this, u16 pressedKey);
static void LevelEditor_printTranslationStepSize(LevelEditor this);

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
	
	this->userObjectsSelector = __NEW(OptionsSelector, __ARGUMENTS(2, 12, ">", kString));
	
	VirtualList userObjects = __NEW(VirtualList);
	
	int i = 0;
	for(;  _userObjects[i].entityDefinition; i++) {
	
		VirtualList_pushBack(userObjects, _userObjects[i].name);
	}
	
	OptionsSelector_setOptions(this->userObjectsSelector, userObjects);
	__DELETE(userObjects);
	
	this->translationStepSize = 8;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void LevelEditor_destructor(LevelEditor this){
	
	ASSERT(this, "LevelEditor::destructor: null this");

	if(this->userObjectsSelector) {
		
		__DELETE(this->userObjectsSelector);
	}
	
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update
void LevelEditor_update(LevelEditor this){
	
	ASSERT(this, "LevelEditor::update: null this");

	//Stage_stream(Level_getStage(this->level));
	if(this->level && this->shape) {
		
		__VIRTUAL_CALL(void, Shape, draw, this->shape);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show debug screens
void LevelEditor_start(LevelEditor this, Level level){
	
	ASSERT(this, "LevelEditor::start: null this");
	ASSERT(level, "LevelEditor::start: level this");

	this->level = level;
	this->mode = kFirstMode + 1;
	LevelEditor_releaseShape(this);
	LevelEditor_setupMode(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// hide debug screens
void LevelEditor_stop(LevelEditor this){

	ASSERT(this, "LevelEditor::stop: null this");

	CollisionManager_flushShapesDirectDrawData(CollisionManager_getInstance());
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	LevelEditor_releaseShape(this);
	this->currentEntityNode = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int LevelEditor_handleMessage(LevelEditor this, Telegram telegram){
	
	ASSERT(this, "LevelEditor::handleMessage: null this");

	if(!this->level) {
		
		return false;
	}

	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:	
			{
				u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

				if(pressedKey & K_SEL){
					
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

					case kChangeProjection:

						LevelEditor_changeProjection(this, pressedKey);
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
// print title
static void LevelEditor_setupMode(LevelEditor this) {
	
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	Printing_text("LEVEL EDITOR", 17, 0);

	switch(this->mode) {
	
		case kMoveScreen:

			LevelEditor_releaseShape(this);
			LevelEditor_printScreenPosition(this);
			break;
			
		case kChangeProjection:

			LevelEditor_releaseShape(this);
			LevelEditor_printProjectionValues(this);
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
			LevelEditor_printTranslationStepSize(this);
			break;
			
		case kAddObjects:

			LevelEditor_releaseShape(this);
			LevelEditor_printUserObjects(this);
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_releaseShape(LevelEditor this) {

	if(this->currentEntityNode) {
		
		Entity entity = (Entity)VirtualNode_getData(this->currentEntityNode);
	
		if(this->shape && this->shape != __VIRTUAL_CALL_UNSAFE(Shape, Entity, getShape, (Entity)entity)) {
			
			__DELETE(this->shape);
		}

		this->shape = NULL;
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
// modify projection values
static void LevelEditor_changeProjection(LevelEditor this, u16 pressedKey){
	
	if(pressedKey & K_LL){
		
		_optical->horizontalViewPointCenter -= __HVPC_STEP;
	}
	else if(pressedKey & K_LR){
		
		_optical->horizontalViewPointCenter += __HVPC_STEP;
	}
	else if(pressedKey & K_LU){
		
		_optical->verticalViewPointCenter -= __VVPC_STEP;
	}
	else if(pressedKey & K_LD){
		
		_optical->verticalViewPointCenter += __VVPC_STEP;
	}
	else if(pressedKey & K_RL){
		
		_optical->distanceEyeScreen -= __DISTANCE_EYE_SCREEN_STEP;
	}
	else if(pressedKey & K_RR){
		
		_optical->distanceEyeScreen += __DISTANCE_EYE_SCREEN_STEP;
	}
	else if(pressedKey & K_RU){
		
		_optical->maximunViewDistance += __MAXIMUM_VIEW_DISTACE_STEP;
	}
	else if(pressedKey & K_RD){
		
		_optical->maximunViewDistance -= __MAXIMUM_VIEW_DISTACE_STEP;
	}
	else if(pressedKey & K_LT){
		
		_optical->baseDistance -= __BASE_DISTACE_STEP;
	}
	else if(pressedKey & K_RT){
		
		_optical->baseDistance += __BASE_DISTACE_STEP;
	}

	// this hack forces the Entity to recalculate
	// its sprites' value
	// must hack this global, otherwise will need
	// another variable which most likely will only
	// take up the previous RAM, or another branching
	// computation in the Entity's render method
	_screenMovementState->x = __ACTIVE;
	_screenMovementState->y = __ACTIVE;
	_screenMovementState->z = __ACTIVE;

	LevelEditor_printProjectionValues(this);
	Level_transform(this->level);

	// prevent any side effect
	_screenMovementState->x = __PASSIVE;
	_screenMovementState->y = __PASSIVE;
	_screenMovementState->z = __PASSIVE;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// translate entity
static void LevelEditor_traslateEntity(LevelEditor this, u16 pressedKey){
	
	if(pressedKey & K_LL){
		
		VBVec3D translation = {
				
				ITOFIX19_13(-this->translationStepSize),
				0, 
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_LR){
		
		VBVec3D translation = {
				
				ITOFIX19_13(this->translationStepSize),
				0, 
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_LU){
		
		VBVec3D translation = {
				0,
				ITOFIX19_13(-this->translationStepSize),
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_LD){
		
		VBVec3D translation = {
				0,
				ITOFIX19_13(this->translationStepSize),
				0
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_RR){

		if(__MAX_TRANSLATION_STEP < ++this->translationStepSize) {
			
			this->translationStepSize = __MAX_TRANSLATION_STEP;
		}
		
		LevelEditor_printTranslationStepSize(this);
	}
	else if(pressedKey & K_RL){
		
		if(1 > --this->translationStepSize) {
			
			this->translationStepSize = 1;
		}
		
		LevelEditor_printTranslationStepSize(this);
	}
	else if(pressedKey & K_RU){
		
		VBVec3D translation = {
				0,
				0,
				ITOFIX19_13(this->translationStepSize),
		};

		LevelEditor_applyTraslationToEntity(this, translation);
	}
	else if(pressedKey & K_RD){
		
		VBVec3D translation = {
				0,
				0,
				ITOFIX19_13(-this->translationStepSize),
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
		
		// this hack forces the Entity to recalculate
		// its sprites' value
		// must hack this global, otherwise will need
		// another variable which most likely will only
		// take up the previous RAM, or another branching
		// computation in the Entity's render method
		_screenMovementState->x = __ACTIVE;
		_screenMovementState->y = __ACTIVE;
		_screenMovementState->z = __ACTIVE;
		
		Level_transform(this->level);

		// prevent any side effect
		_screenMovementState->x = __PASSIVE;
		_screenMovementState->y = __PASSIVE;
		_screenMovementState->z = __PASSIVE;

		LevelEditor_positioneShape(this);

		LevelEditor_printEntityPosition(this);
		
		SpriteManager_sortAllLayers(SpriteManager_getInstance());
		
		LevelEditor_printTranslationStepSize(this);

		// should work
		//__VIRTUAL_CALL(void, Shape, positione, this->shape);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_selectUserObject(LevelEditor this, u16 pressedKey) {
	
	if(pressedKey & K_LU){
	
		OptionsSelector_selectPrevious(this->userObjectsSelector);
	}
	else if(pressedKey & K_LD){
		
		OptionsSelector_selectNext(this->userObjectsSelector);
	}
	else if(pressedKey & K_A) {
		
		VBVec3D position = Screen_getPosition(Screen_getInstance());
		
		position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
		position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
		position.z += ITOFIX19_13(__SCREEN_WIDTH >> 2);
		
		Entity entity = Stage_addEntity(Level_getStage(this->level), _userObjects[OptionsSelector_getSelectedOption(this->userObjectsSelector)].entityDefinition, &position, NULL, false);
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
	
	int y = 2;
	int x = 1;
	Printing_text("MOVE OBJECT", x, y++);
	Printing_text("Nav.     (SEL)", 48 - 14, 0);
	Printing_text("Next   (LT/RT)", 48 - 14, 1);
	Printing_text("Move (LU/LD/LL", 48 - 14, 2);
	Printing_text("/LR/RU/LD)", 48 - 10, 3);

	if(this->currentEntityNode){
		
		Container container = (Container)VirtualNode_getData(this->currentEntityNode);
		VBVec3D globalPosition = Container_getGlobalPosition(container);

		Printing_text("ID: ", x, ++y);
		Printing_int(Container_getID(container), x + 6, y);
		Printing_text("Type:                                  ", x, ++y);
		Printing_text(__GET_CLASS_NAME(container), x + 6, y);
		Printing_text("Position:                  ", x, ++y);
		Printing_int(FIX19_13TOI(globalPosition.x), x + 10, y);
		Printing_int(FIX19_13TOI(globalPosition.y), x + 15, y);
		Printing_int(FIX19_13TOI(globalPosition.z), x + 20, y);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_applyTraslationToScreen(LevelEditor this, VBVec3D translation){
	
	Screen_move(Screen_getInstance(), translation, true);
	Level_transform(this->level);

	LevelEditor_printScreenPosition(this);

	Stage_streamAll(Level_getStage(this->level));

	CollisionManager_processRemovedShapes(CollisionManager_getInstance());
	PhysicalWorld_processRemovedBodies(PhysicalWorld_getInstance());
	SpriteManager_sortAllLayers(SpriteManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_printScreenPosition(LevelEditor this){
	
	int x = 1;
	int y = 2;
	
	VBVec3D position = Screen_getPosition(Screen_getInstance());

	Printing_text("MOVE SCREEN", x, y++);
	Printing_text("Nav.     (SEL)", 48 - 14, 0);
	Printing_text("Move (LU/LD/LL", 48 - 14, 1);
	Printing_text("/LR/RU/LD)", 48 - 10, 2);
	Printing_text("Position:               ", x, ++y);
	Printing_int(FIX19_13TOI(position.x), x + 10, y);
	Printing_int(FIX19_13TOI(position.y), x + 15, y);
	Printing_int(FIX19_13TOI(position.z), x + 20, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_printProjectionValues(LevelEditor this){
	
	int x = 1;
	int y = 2;
	
	Printing_text("PROJECTION VALUES", x, y++);
	Printing_text("Nav.   (SEL)", 48 - 12, 0);
	Printing_text("HVPC (LD/LL)", 48 - 12, 1);
	Printing_text("VVPC (LU/LD)", 48 - 12, 2);
	Printing_text("DES  (RL/RR)", 48 - 12, 3);
	Printing_text("MVD  (RU/RD)", 48 - 12, 4);
	Printing_text("BD   (LT/RT)", 48 - 12, 5);
	
	Printing_text("H. view point center:            ", x, ++y);
	Printing_int(FIX19_13TOI(_optical->horizontalViewPointCenter), x + 22, y);
	Printing_text("V. view point center:            ", x, ++y);
	Printing_int(FIX19_13TOI(_optical->verticalViewPointCenter), x + 22, y);
	Printing_text("Distance Eye Screen:            ", x, ++y);
	Printing_int(FIX19_13TOI(_optical->distanceEyeScreen), x + 22, y);
	Printing_text("Maximum View Screen:            ", x, ++y);
	Printing_int(FIX19_13TOI(_optical->maximunViewDistance), x + 22, y);
	Printing_text("Base Distance:                  ", x, ++y);
	Printing_int(FIX19_13TOI(_optical->baseDistance), x + 22, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_printUserObjects(LevelEditor this){

	Printing_text("ADD OBJECTS", 1, 2);
	Printing_text("                       ", 1, 3);
	Printing_text("Nav. (SEL)", 48 - 10, 0);
	Printing_text("Accept (A)", 48 - 10, 1);

	OptionsSelector_showOptions(this->userObjectsSelector, 1, 4);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void LevelEditor_printTranslationStepSize(LevelEditor this) {
	
	Printing_text("Step   (RL/RR)", 48 - 14, 4);
	Printing_text("+     ", 48 - 12, 5);
	Printing_int(this->translationStepSize, 48 - 11, 5);
}

#endif 
