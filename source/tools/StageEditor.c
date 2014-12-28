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

#ifdef __STAGE_EDITOR

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StageEditor.h>
#include <Game.h>
#include <Optics.h>
#include <Entity.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <SpriteManager.h>
#include <GameState.h>
#include <Stage.h>
#include <Shape.h>
#include <Screen.h>
#include <Cuboid.h>
#include <OptionsSelector.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __USER_OBJECT_SHOW_ROW 	6
#define __MAX_TRANSLATION_STEP	8 * 4
#define __SCREEN_X_TRANSLATION_STEP		__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP		__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP		__SCREEN_HEIGHT / 4

#define __HVPC_STEP						ITOFIX19_13(8)
#define __VVPC_STEP						ITOFIX19_13(8)
#define __DISTANCE_EYE_SCREEN_STEP		ITOFIX19_13(8)
#define __MAXIMUM_VIEW_DISTACE_STEP		ITOFIX19_13(8)
#define __BASE_DISTACE_STEP				ITOFIX19_13(8)

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define StageEditor_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* current in game entity */												\
	GameState gameState;														\
																				\
	/* current in game entity */												\
	VirtualNode currentEntityNode;												\
																				\
	/* current entity's shape */												\
	Shape shape;																\
																				\
	/* mode */																	\
	int mode;																	\
																				\
	/* actors selector */														\
	OptionsSelector userObjectsSelector;										\
																				\
	/* translation step size */													\
	int translationStepSize;													\

// define the StageEditor
__CLASS_DEFINITION(StageEditor);

enum Modes {
		kFirstMode = 0,
		kMoveScreen,
		kChangeProjection,
		kTranslateEntities,
		kAddObjects,

		kLastMode
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern Optical* _optical;
extern UserObject _userObjects[];
const extern VBVec3D* _screenDisplacement;

static void StageEditor_constructor(StageEditor this);
static void StageEditor_setupMode(StageEditor this);
static void StageEditor_releaseShape(StageEditor this);
static void StageEditor_getShape(StageEditor this);
static void StageEditor_positioneShape(StageEditor this);
static void StageEditor_highLightEntity(StageEditor this);
static void StageEditor_selectPreviousEntity(StageEditor this);
static void StageEditor_selectNextEntity(StageEditor this);
static void StageEditor_traslateEntity(StageEditor this, u16 pressedKey);
static void StageEditor_moveScreen(StageEditor this, u16 pressedKey);
static void StageEditor_changeProjection(StageEditor this, u16 pressedKey);
static void StageEditor_applyTraslationToEntity(StageEditor this, VBVec3D translation);
static void StageEditor_applyTraslationToScreen(StageEditor this, VBVec3D translation);
static void StageEditor_printEntityPosition(StageEditor this);
static void StageEditor_printScreenPosition(StageEditor this);
static void StageEditor_printProjectionValues(StageEditor this);
static void StageEditor_printUserObjects(StageEditor this);
static void StageEditor_selectUserObject(StageEditor this, u16 pressedKey);
static void StageEditor_printTranslationStepSize(StageEditor this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(StageEditor);

// class's constructor
static void StageEditor_constructor(StageEditor this)
{
	ASSERT(this, "StageEditor::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->currentEntityNode = NULL;

	this->gameState = NULL;

	this->mode = kFirstMode + 1;

	this->userObjectsSelector = __NEW(OptionsSelector, __ARGUMENTS(2, 12, ">", kString));

	VirtualList userObjects = __NEW(VirtualList);

	int i = 0;
	for (;  _userObjects[i].entityDefinition; i++)
	{
		VirtualList_pushBack(userObjects, _userObjects[i].name);
	}

	OptionsSelector_setOptions(this->userObjectsSelector, userObjects);
	__DELETE(userObjects);

	this->translationStepSize = 8;
}

// class's destructor
void StageEditor_destructor(StageEditor this)
{
	ASSERT(this, "StageEditor::destructor: null this");

	if (this->userObjectsSelector)
	{
		__DELETE(this->userObjectsSelector);
	}

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// update
void StageEditor_update(StageEditor this)
{
	ASSERT(this, "StageEditor::update: null this");

	if (this->gameState && this->shape)
	{
		__VIRTUAL_CALL(void, Shape, draw, this->shape);
	}
}

// show debug screens
void StageEditor_start(StageEditor this, GameState gameState)
{
	ASSERT(this, "StageEditor::start: null this");
	ASSERT(gameState, "StageEditor::start: gameState this");

	this->gameState = gameState;
	this->mode = kFirstMode + 1;
	StageEditor_releaseShape(this);
	StageEditor_setupMode(this);
}

// hide debug screens
void StageEditor_stop(StageEditor this)
{
	ASSERT(this, "StageEditor::stop: null this");

	CollisionManager_flushShapesDirectDrawData(CollisionManager_getInstance());
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	StageEditor_releaseShape(this);
	this->currentEntityNode = NULL;
}

// process a telegram
bool StageEditor_handleMessage(StageEditor this, Telegram telegram)
{
	ASSERT(this, "StageEditor::handleMessage: null this");

	if (!this->gameState)
	{
		return false;
	}

	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            if (pressedKey & K_SEL)
            {
                this->mode++;

                if (kLastMode <= this->mode)
                {
                    this->mode = kFirstMode + 1;
                }

                StageEditor_setupMode(this);
                break;
            }

            switch (this->mode)
            {
                case kMoveScreen:

                    StageEditor_moveScreen(this, pressedKey);
                    break;

                case kChangeProjection:

                    StageEditor_changeProjection(this, pressedKey);
                    break;

                case kTranslateEntities:

                    StageEditor_traslateEntity(this, pressedKey);
                    break;

                case kAddObjects:

                    StageEditor_selectUserObject(this, pressedKey);
                    break;
            }
        }
        break;
	}
	return true;
}

// print title
static void StageEditor_setupMode(StageEditor this)
{
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	Printing_text("LEVEL EDITOR", 17, 0);

	switch (this->mode)
	{
		case kMoveScreen:

			StageEditor_releaseShape(this);
			StageEditor_printScreenPosition(this);
			break;

		case kChangeProjection:

			StageEditor_releaseShape(this);
			StageEditor_printProjectionValues(this);
			break;

		case kTranslateEntities:

			if (!this->currentEntityNode)
	        {
				StageEditor_selectNextEntity(this);
			}
			else
	        {
				StageEditor_getShape(this);
				StageEditor_highLightEntity(this);
			}

			StageEditor_printEntityPosition(this);
			StageEditor_printTranslationStepSize(this);
			break;

		case kAddObjects:

			StageEditor_releaseShape(this);
			StageEditor_printUserObjects(this);
			break;
	}
}

static void StageEditor_releaseShape(StageEditor this)
{
	if (this->currentEntityNode)
	{
		Entity entity = (Entity)VirtualNode_getData(this->currentEntityNode);

		if (this->shape && this->shape != __VIRTUAL_CALL_UNSAFE(Shape, Entity, getShape, (Entity)entity))
	    {
			__DELETE(this->shape);
		}

		this->shape = NULL;
	}
}

static void StageEditor_getShape(StageEditor this)
{
	if (!this->currentEntityNode)
	{
		return;
	}

	Entity entity = (Entity)VirtualNode_getData(this->currentEntityNode);

	this->shape = __VIRTUAL_CALL_UNSAFE(Shape, Entity, getShape, (Entity)entity);

	if (!this->shape)
	{
		switch (__VIRTUAL_CALL(int, Entity, getShapeType, entity))
	    {
			case kCircle:

				//VirtualList_pushBack(this->shapes, (void*)__NEW(Circle, __ARGUMENTS(owner)));
				break;

			case kCuboid:

				this->shape = (Shape)__NEW(Cuboid, __ARGUMENTS(entity));
				break;
		}
	}
}

static void StageEditor_positioneShape(StageEditor this)
{
	if (!this->currentEntityNode || !this->shape)
	{
		return;
	}

	Entity entity = (Entity)VirtualNode_getData(this->currentEntityNode);

	__VIRTUAL_CALL(void, Shape, setup, this->shape);

	Shape_setReady(this->shape, false);

	if (__VIRTUAL_CALL(int, Entity, moves, entity))
	{
		__VIRTUAL_CALL(void, Shape, positione, this->shape);
	}
}

static void StageEditor_highLightEntity(StageEditor this)
{
	if (this->currentEntityNode)
	{
		StageEditor_printEntityPosition(this);
		StageEditor_positioneShape(this);
	}
	else
	{
		Printing_text("No entities in stage", 1, 4);
	}
}

// select the next entity
static void StageEditor_selectPreviousEntity(StageEditor this)
{
	StageEditor_releaseShape(this);

	VirtualList stageEntities = Container_getChildren((Container)GameState_getStage(this->gameState));

	if (!this->currentEntityNode)
	{
		this->currentEntityNode = stageEntities ? VirtualList_end(stageEntities) : NULL;
	}
	else
	{
		this->currentEntityNode = VirtualNode_getPrevious(this->currentEntityNode);

		if (!this->currentEntityNode)
    	{
			this->currentEntityNode = stageEntities ? VirtualList_end(stageEntities) : NULL;
		}
	}

	if (this->currentEntityNode)
	{
		StageEditor_getShape(this);
		StageEditor_highLightEntity(this);
	}
}

// select the next entity
static void StageEditor_selectNextEntity(StageEditor this)
{
	StageEditor_releaseShape(this);

	VirtualList stageEntities = Container_getChildren((Container)GameState_getStage(this->gameState));

	if (!this->currentEntityNode)
	{
		this->currentEntityNode = stageEntities ? VirtualList_begin(stageEntities) : NULL;
	}
	else
	{
		Printing_text("StageEditor", 1, 10);
		this->currentEntityNode = VirtualNode_getNext(this->currentEntityNode);

		if (!this->currentEntityNode)
	    {
			this->currentEntityNode = stageEntities ? VirtualList_begin(stageEntities) : NULL;
		}
	}

	if (this->currentEntityNode)
	{
		StageEditor_getShape(this);
		StageEditor_highLightEntity(this);
	}
}

// move screen
static void StageEditor_moveScreen(StageEditor this, u16 pressedKey)
{
	if (pressedKey & K_LL)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(-__SCREEN_X_TRANSLATION_STEP),
            0,
            0
		};

		StageEditor_applyTraslationToScreen(this, translation);
	}
	else if (pressedKey & K_LR)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(__SCREEN_X_TRANSLATION_STEP),
            0,
            0
		};

		StageEditor_applyTraslationToScreen(this, translation);
	}
	else if (pressedKey & K_LU)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(-__SCREEN_Y_TRANSLATION_STEP),
            0
		};

		StageEditor_applyTraslationToScreen(this, translation);
	}
	else if (pressedKey & K_LD)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(__SCREEN_Y_TRANSLATION_STEP),
            0
		};

		StageEditor_applyTraslationToScreen(this, translation);
	}
	else if (pressedKey & K_RU)
	{
		VBVec3D translation =
	    {
            0,
            0,
            ITOFIX19_13(__SCREEN_Z_TRANSLATION_STEP),
		};

		StageEditor_applyTraslationToScreen(this, translation);
	}
	else if (pressedKey & K_RD)
	{
		VBVec3D translation =
	    {
            0,
            0,
            ITOFIX19_13(-__SCREEN_Z_TRANSLATION_STEP),
		};

		StageEditor_applyTraslationToScreen(this, translation);
	}
}


// modify projection values
static void StageEditor_changeProjection(StageEditor this, u16 pressedKey)
{
	if (pressedKey & K_LL)
	{
		_optical->horizontalViewPointCenter -= __HVPC_STEP;
	}
	else if (pressedKey & K_LR)
	{
		_optical->horizontalViewPointCenter += __HVPC_STEP;
	}
	else if (pressedKey & K_LU)
	{
		_optical->verticalViewPointCenter -= __VVPC_STEP;
	}
	else if (pressedKey & K_LD)
	{
		_optical->verticalViewPointCenter += __VVPC_STEP;
	}
	else if (pressedKey & K_RL)
	{
		_optical->distanceEyeScreen -= __DISTANCE_EYE_SCREEN_STEP;
	}
	else if (pressedKey & K_RR)
	{
		_optical->distanceEyeScreen += __DISTANCE_EYE_SCREEN_STEP;
	}
	else if (pressedKey & K_RU)
	{
		_optical->maximunViewDistance += __MAXIMUM_VIEW_DISTACE_STEP;
	}
	else if (pressedKey & K_RD)
	{
		_optical->maximunViewDistance -= __MAXIMUM_VIEW_DISTACE_STEP;
	}
	else if (pressedKey & K_LT)
	{
		_optical->baseDistance -= __BASE_DISTACE_STEP;
	}
	else if (pressedKey & K_RT)
	{
		_optical->baseDistance += __BASE_DISTACE_STEP;
	}

	// this hack forces the Entity to recalculate
	// its sprites' value
	// must hack this global, otherwise will need
	// another variable which most likely will only
	// take up the previous RAM, or another branching
	// computation in the Entity's render method
	Screen_forceDisplacement(Screen_getInstance(), true);

	StageEditor_printProjectionValues(this);
	GameState_transform(this->gameState);
}

// translate entity
static void StageEditor_traslateEntity(StageEditor this, u16 pressedKey)
{
	if (pressedKey & K_LL)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(-this->translationStepSize),
            0,
            0
		};

		StageEditor_applyTraslationToEntity(this, translation);
	}
	else if (pressedKey & K_LR)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(this->translationStepSize),
            0,
            0
		};

		StageEditor_applyTraslationToEntity(this, translation);
	}
	else if (pressedKey & K_LU)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(-this->translationStepSize),
            0
		};

		StageEditor_applyTraslationToEntity(this, translation);
	}
	else if (pressedKey & K_LD)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(this->translationStepSize),
            0
		};

		StageEditor_applyTraslationToEntity(this, translation);
	}
	else if (pressedKey & K_RR)
	{
		if (__MAX_TRANSLATION_STEP < ++this->translationStepSize)
	    {
			this->translationStepSize = __MAX_TRANSLATION_STEP;
		}

		StageEditor_printTranslationStepSize(this);
	}
	else if (pressedKey & K_RL)
	{
		if (1 > --this->translationStepSize)
    	{
			this->translationStepSize = 1;
		}

		StageEditor_printTranslationStepSize(this);
	}
	else if (pressedKey & K_RU)
	{
		VBVec3D translation =
    	{
            0,
            0,
            ITOFIX19_13(this->translationStepSize),
		};

		StageEditor_applyTraslationToEntity(this, translation);
	}
	else if (pressedKey & K_RD)
	{
		VBVec3D translation =
	    {
            0,
            0,
            ITOFIX19_13(-this->translationStepSize),
		};

		StageEditor_applyTraslationToEntity(this, translation);
	}
	else if (pressedKey & K_LT)
	{
		StageEditor_selectPreviousEntity(this);
	}
	else if (pressedKey & K_RT)
	{
		StageEditor_selectNextEntity(this);
	}
}

static void StageEditor_applyTraslationToEntity(StageEditor this, VBVec3D translation)
{
	if (this->currentEntityNode && this->shape)
	{
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
		Screen_forceDisplacement(Screen_getInstance(), true);
		
		GameState_transform(this->gameState);

		StageEditor_positioneShape(this);

		StageEditor_printEntityPosition(this);

		SpriteManager_sortLayers(SpriteManager_getInstance(), false);

		StageEditor_printTranslationStepSize(this);

		// should work
		//__VIRTUAL_CALL(void, Shape, positione, this->shape);
	}
}

static void StageEditor_selectUserObject(StageEditor this, u16 pressedKey)
{
	if (pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->userObjectsSelector);
	}
	else if (pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->userObjectsSelector);
	}
	else if (pressedKey & K_A)
	{
		if (1 >= SpriteManager_getFreeLayer(SpriteManager_getInstance()))
	    {
			Printing_text("No more WORLDs", 48 - 15, 4);
			Printing_text("available     ", 48 - 15, 5);
			return;
		}

		VBVec3D position = Screen_getPosition(Screen_getInstance());

		position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
		position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
		position.z += ITOFIX19_13(__SCREEN_WIDTH >> 2);

		Entity entity = Stage_addEntity(GameState_getStage(this->gameState), _userObjects[OptionsSelector_getSelectedOption(this->userObjectsSelector)].entityDefinition, &position, NULL, false);
		SpriteManager_sortLayers(SpriteManager_getInstance(), false);

		GameState_transform(this->gameState);
		__VIRTUAL_CALL(void, Container, setLocalPosition, (Container)entity, __ARGUMENTS(position));

		VirtualList stageEntities = Container_getChildren((Container)GameState_getStage(this->gameState));
		this->currentEntityNode = stageEntities ? VirtualList_end(stageEntities) : NULL;

		// select the added entity
		this->mode = kTranslateEntities;
		StageEditor_setupMode(this);
	}
}

static void StageEditor_printEntityPosition(StageEditor this)
{
	int y = 2;
	int x = 1;
	Printing_text("MOVE OBJECT", x, y++);
	Printing_text("Nav.     (SEL)", 48 - 14, 0);
	Printing_text("Next   (LT/RT)", 48 - 14, 1);
	Printing_text("Move (LU/LD/LL", 48 - 14, 2);
	Printing_text("/LR/RU/LD)", 48 - 10, 3);

	if (this->currentEntityNode)
	{
		Container container = (Container)VirtualNode_getData(this->currentEntityNode);
		VBVec3D globalPosition = Container_getGlobalPosition(container);

		Printing_text("ID: ", x, ++y);
		Printing_int(Container_getId(container), x + 6, y);
		Printing_text("Type:                                  ", x, ++y);
		Printing_text(__GET_CLASS_NAME(container), x + 6, y);
		Printing_text("Position:                  ", x, ++y);
		Printing_int(FIX19_13TOI(globalPosition.x), x + 10, y);
		Printing_int(FIX19_13TOI(globalPosition.y), x + 15, y);
		Printing_int(FIX19_13TOI(globalPosition.z), x + 20, y);
	}
}


static void StageEditor_applyTraslationToScreen(StageEditor this, VBVec3D translation)
{
	Screen_move(Screen_getInstance(), translation, true);
	GameState_transform(this->gameState);
	StageEditor_printScreenPosition(this);
	Stage_streamAll(GameState_getStage(this->gameState));
	CollisionManager_processRemovedShapes(CollisionManager_getInstance());
	PhysicalWorld_processRemovedBodies(PhysicalWorld_getInstance());
}

static void StageEditor_printScreenPosition(StageEditor this)
{
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

static void StageEditor_printProjectionValues(StageEditor this)
{
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

static void StageEditor_printUserObjects(StageEditor this)
{
	Printing_text("ADD OBJECTS", 1, 2);
	Printing_text("                       ", 1, 3);
	Printing_text("Nav. (SEL)", 48 - 10, 0);
	Printing_text("Accept (A)", 48 - 10, 1);

	OptionsSelector_showOptions(this->userObjectsSelector, 1, 4);
}

static void StageEditor_printTranslationStepSize(StageEditor this)
{
	Printing_text("Step   (RL/RR)", 48 - 14, 4);
	Printing_text("+     ", 48 - 12, 5);
	Printing_int(this->translationStepSize, 48 - 11, 5);
}


#endif