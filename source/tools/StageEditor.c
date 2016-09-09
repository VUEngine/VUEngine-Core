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
#include <KeyPadManager.h>
#include <BgmapTextureManager.h>
#include <Printing.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __USER_OBJECT_SHOW_ROW 				6
#define __MAX_TRANSLATION_STEP				8 * 4
#define __SCREEN_X_TRANSLATION_STEP			__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP			__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP			__SCREEN_HEIGHT / 4

#define __HVPC_STEP							ITOFIX19_13(8)
#define __VERTICAL_VIEW_POINT_CENTER_STEP	ITOFIX19_13(8)
#define __DISTANCE_EYE_SCREEN_STEP			ITOFIX19_13(8)
#define __MAXIMUM_VIEW_DISTACE_STEP			1
#define __BASE_DISTACE_STEP					ITOFIX19_13(8)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class StageEditor
 * @extends Object
 * @brief In-game stage editor for debug and productivity purposes
 *
 * @var GameState       gameState
 * @brief Current game state
 * @memberof StageEditor
 *
 * @var VirtualNode     currentEntityNode
 * @brief Current in game entity
 * @memberof StageEditor
 *
 * @var Shape           shape
 * @brief Current entity's shape
 * @memberof StageEditor
 *
 * @var int             mode
 * @brief Mode
 * @memberof StageEditor
 *
 * @var OptionsSelector userObjectsSelector
 * @brief Actors selector
 * @memberof StageEditor
 *
 * @var int             translationStepSize
 * @brief Translation step size
 * @memberof StageEditor
 *
 * @var Sprite          userObjectSprite
 * @brief Current user's object's sprite
 * @memberof StageEditor
 */

#define StageEditor_ATTRIBUTES																			\
        Object_ATTRIBUTES																				\
        GameState gameState;																			\
        VirtualNode currentEntityNode;																	\
        Shape shape;																					\
        int mode;																						\
        OptionsSelector userObjectsSelector;															\
        int translationStepSize;																		\
        Sprite userObjectSprite;																		\

__CLASS_DEFINITION(StageEditor, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);

enum Modes
{
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
extern const VBVec3D* _screenDisplacement;

void Container_invalidateGlobalPosition(Container this, u8 axisToInvalidate);

static void StageEditor_constructor(StageEditor this);
static void StageEditor_setupMode(StageEditor this);
static void StageEditor_releaseShape(StageEditor this);
static void StageEditor_getShape(StageEditor this);
static void StageEditor_positionShape(StageEditor this);
static void StageEditor_highLightEntity(StageEditor this);
static void StageEditor_selectPreviousEntity(StageEditor this);
static void StageEditor_selectNextEntity(StageEditor this);
static void StageEditor_translateEntity(StageEditor this, u32 pressedKey);
static void StageEditor_moveScreen(StageEditor this, u32 pressedKey);
static void StageEditor_changeProjection(StageEditor this, u32 pressedKey);
static void StageEditor_applyTranslationToEntity(StageEditor this, VBVec3D translation);
static void StageEditor_applyTranslationToScreen(StageEditor this, VBVec3D translation);
static void StageEditor_printEntityPosition(StageEditor this);
static void StageEditor_printScreenPosition(StageEditor this);
static void StageEditor_printProjectionValues(StageEditor this);
static void StageEditor_printUserObjects(StageEditor this);
static void StageEditor_selectUserObject(StageEditor this, u32 pressedKey);
static void StageEditor_printTranslationStepSize(StageEditor this);
static void StageEditor_removePreviousSprite(StageEditor this);
static void StageEditor_showSelectedUserObject(StageEditor this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(StageEditor);

/**
 * Class constructor
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void __attribute__ ((noinline)) StageEditor_constructor(StageEditor this)
{
	ASSERT(this, "StageEditor::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->currentEntityNode = NULL;
	this->gameState = NULL;
	this->userObjectSprite = NULL;
	this->mode = kFirstMode + 1;
	this->shape = NULL;
	this->userObjectsSelector = __NEW(OptionsSelector, 2, 12, "\x0B", kString);

	VirtualList userObjects = __NEW(VirtualList);

	int i = 0;
	for(;  _userObjects[i].entityDefinition; i++)
	{
		VirtualList_pushBack(userObjects, _userObjects[i].name);
	}

	OptionsSelector_setOptions(this->userObjectsSelector, userObjects);
	__DELETE(userObjects);

	this->translationStepSize = 8;
}

/**
 * Class destructor
 *
 * @memberof StageEditor
 * @public
 * @param this  Function scope
 */
void StageEditor_destructor(StageEditor this)
{
	ASSERT(this, "StageEditor::destructor: null this");

	if(this->userObjectsSelector)
	{
		__DELETE(this->userObjectsSelector);
	}

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Update
 *
 * @memberof StageEditor
 * @public
 * @param this  Function scope
 */
void StageEditor_update(StageEditor this)
{
	ASSERT(this, "StageEditor::update: null this");

	if(this->gameState && this->shape)
	{
		__VIRTUAL_CALL(Shape, draw, this->shape);
	}
}

/**
 * Show debug screens
 *
 * @memberof StageEditor
 * @public
 * @param this      Function scope
 * @param gameState Current game state
 */
void StageEditor_start(StageEditor this, GameState gameState)
{
	ASSERT(this, "StageEditor::start: null this");
	ASSERT(gameState, "StageEditor::start: gameState this");

	this->gameState = gameState;
	this->mode = kFirstMode + 1;
	this->userObjectSprite = NULL;

	StageEditor_releaseShape(this);
	StageEditor_setupMode(this);
}

/**
 * Hide debug screens
 *
 * @memberof StageEditor
 * @public
 * @param this  Function scope
 */
void StageEditor_stop(StageEditor this)
{
	ASSERT(this, "StageEditor::stop: null this");

	CollisionManager_flushShapesDirectDrawData(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
	VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	StageEditor_removePreviousSprite(this);
	StageEditor_releaseShape(this);
	this->currentEntityNode = NULL;
}

/**
 * Handles incoming messages
 *
 * @memberof StageEditor
 * @public
 * @param this      Function scope
 * @param telegram  The received message
 * @return True if successful, false otherwise
 */
bool StageEditor_handleMessage(StageEditor this, Telegram telegram)
{
	ASSERT(this, "StageEditor::handleMessage: null this");

	if(!this->gameState)
	{
		return false;
	}

	switch(Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u32 pressedKey = *((u32*)Telegram_getExtraInfo(telegram));

            if(pressedKey & K_SEL)
            {
                this->mode++;

                if(kLastMode <= this->mode)
                {
                    this->mode = kFirstMode + 1;
                }

                StageEditor_setupMode(this);
                break;
            }

            switch(this->mode)
            {
                case kMoveScreen:

                    StageEditor_moveScreen(this, pressedKey);
                    break;

                case kChangeProjection:

                    StageEditor_changeProjection(this, pressedKey);
                    break;

                case kTranslateEntities:

                    StageEditor_translateEntity(this, pressedKey);
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

/**
 * Print header
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_printHeader(StageEditor this)
{
	Printing_text(Printing_getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
    Printing_text(Printing_getInstance(), " LEVEL EDITOR ", 1, 0, NULL);
    Printing_text(Printing_getInstance(), "  /  ", 16, 0, NULL);
    Printing_int(Printing_getInstance(), this->mode, 17, 0, NULL);
    Printing_int(Printing_getInstance(), kLastMode - 1, 19, 0, NULL);
}

/**
 * Print title
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_setupMode(StageEditor this)
{
	VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
    StageEditor_printHeader(this);
	StageEditor_removePreviousSprite(this);

	switch(this->mode)
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

			if(!this->currentEntityNode)
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
			StageEditor_showSelectedUserObject(this);
			break;
	}
}

/**
 * Release shape
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_releaseShape(StageEditor this)
{
	if(this->currentEntityNode)
	{
		Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));

		if(this->shape && this->shape != __VIRTUAL_CALL(Entity, getShape, entity))
	    {
			__DELETE(this->shape);
		}
		else if(this->shape)
		{
			__VIRTUAL_CALL(Shape, deleteDirectDrawData, this->shape);
		}

		this->shape = NULL;
	}
}

/**
 * Get shape
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_getShape(StageEditor this)
{
	if(!this->currentEntityNode)
	{
		return;
	}

	Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));

	this->shape = __VIRTUAL_CALL(Entity, getShape, entity);

	if(!this->shape)
	{
		switch(__VIRTUAL_CALL(SpatialObject, getShapeType, entity))
	    {
			case kCircle:

				//VirtualList_pushBack(this->shapes, (void*)__NEW(Circle, owner));
				break;

			case kCuboid:

				this->shape = __SAFE_CAST(Shape, __NEW(Cuboid, __SAFE_CAST(SpatialObject, entity)));
				break;
		}
	}
}

/**
 * Position shape
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_positionShape(StageEditor this)
{
	if(!this->currentEntityNode || !this->shape)
	{
		return;
	}

	Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));

	__VIRTUAL_CALL(Shape, setup, this->shape);

	Shape_setReady(this->shape, false);

	if(__VIRTUAL_CALL(Entity, moves, entity))
	{
		__VIRTUAL_CALL(Shape, position, this->shape);
	}
}

/**
 * Highlight entity
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_highLightEntity(StageEditor this)
{
	if(this->currentEntityNode)
	{
		StageEditor_printEntityPosition(this);
		StageEditor_positionShape(this);
	}
	else
	{
		Printing_text(Printing_getInstance(), "No entities in stage", 1, 4, NULL);
	}
}

/**
 * Select the previous entity
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_selectPreviousEntity(StageEditor this)
{
	StageEditor_releaseShape(this);

	VirtualList stageEntities = Container_getChildren(__SAFE_CAST(Container, GameState_getStage(this->gameState)));

	if(!this->currentEntityNode)
	{
		this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;
	}
	else
	{
		this->currentEntityNode = VirtualNode_getPrevious(this->currentEntityNode);

		if(!this->currentEntityNode)
    	{
			this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;
		}
	}

	if(this->currentEntityNode)
	{
		StageEditor_getShape(this);
		StageEditor_highLightEntity(this);
	}
}

/**
 * Select the next entity
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_selectNextEntity(StageEditor this)
{
	StageEditor_releaseShape(this);

	VirtualList stageEntities = Container_getChildren(__SAFE_CAST(Container, GameState_getStage(this->gameState)));

	if(!this->currentEntityNode)
	{
		this->currentEntityNode = stageEntities ? stageEntities->head : NULL;
	}
	else
	{
		this->currentEntityNode = this->currentEntityNode->next;

		if(!this->currentEntityNode)
	    {
			this->currentEntityNode = stageEntities ? stageEntities->head : NULL;
		}
	}

	if(this->currentEntityNode)
	{
		StageEditor_getShape(this);
		StageEditor_highLightEntity(this);
	}
}

/**
 * Move the screen
 *
 * @memberof StageEditor
 * @private
 * @param this          Function scope
 * @param pressedKey    The controller button pressed by the user
 */
static void StageEditor_moveScreen(StageEditor this, u32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(-__SCREEN_X_TRANSLATION_STEP),
            0,
            0
		};

		StageEditor_applyTranslationToScreen(this, translation);
	}
	else if(pressedKey & K_LR)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(__SCREEN_X_TRANSLATION_STEP),
            0,
            0
		};

		StageEditor_applyTranslationToScreen(this, translation);
	}
	else if(pressedKey & K_LU)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(-__SCREEN_Y_TRANSLATION_STEP),
            0
		};

		StageEditor_applyTranslationToScreen(this, translation);
	}
	else if(pressedKey & K_LD)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(__SCREEN_Y_TRANSLATION_STEP),
            0
		};

		StageEditor_applyTranslationToScreen(this, translation);
	}
	else if(pressedKey & K_RU)
	{
		VBVec3D translation =
	    {
            0,
            0,
            ITOFIX19_13(__SCREEN_Z_TRANSLATION_STEP),
		};

		StageEditor_applyTranslationToScreen(this, translation);
	}
	else if(pressedKey & K_RD)
	{
		VBVec3D translation =
	    {
            0,
            0,
            ITOFIX19_13(-__SCREEN_Z_TRANSLATION_STEP),
		};

		StageEditor_applyTranslationToScreen(this, translation);
	}
}

/**
 * Modify projection values
 *
 * @memberof StageEditor
 * @private
 * @param this          Function scope
 * @param pressedKey    The controller button pressed by the user
 */
static void StageEditor_changeProjection(StageEditor this, u32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		_optical->horizontalViewPointCenter -= __HVPC_STEP;
	}
	else if(pressedKey & K_LR)
	{
		_optical->horizontalViewPointCenter += __HVPC_STEP;
	}
	else if(pressedKey & K_LU)
	{
		_optical->verticalViewPointCenter -= __VERTICAL_VIEW_POINT_CENTER_STEP;
	}
	else if(pressedKey & K_LD)
	{
		_optical->verticalViewPointCenter += __VERTICAL_VIEW_POINT_CENTER_STEP;
	}
	else if(pressedKey & K_RL)
	{
		_optical->distanceEyeScreen -= __DISTANCE_EYE_SCREEN_STEP;
	}
	else if(pressedKey & K_RR)
	{
		_optical->distanceEyeScreen += __DISTANCE_EYE_SCREEN_STEP;
	}
	else if(pressedKey & K_RU)
	{
		_optical->maximumViewDistancePower += __MAXIMUM_VIEW_DISTACE_STEP;
	}
	else if(pressedKey & K_RD)
	{
		_optical->maximumViewDistancePower -= __MAXIMUM_VIEW_DISTACE_STEP;

		if(0 >= _optical->maximumViewDistancePower)
		{
			_optical->maximumViewDistancePower = 1;
		}
	}
	else if(pressedKey & K_LT)
	{
		_optical->baseDistance -= __BASE_DISTACE_STEP;
	}
	else if(pressedKey & K_RT)
	{
		_optical->baseDistance += __BASE_DISTACE_STEP;
	}

	// this hack forces the Entity to recalculate its sprites' value.
	// must hack this global, otherwise will need another variable which most likely will only
	// take up the previous RAM, or another branching computation in the Entity's render method.
	Screen_forceDisplacement(Screen_getInstance(), true);

	StageEditor_printProjectionValues(this);
	GameState_transform(this->gameState);
}

/**
 * Translate an entity
 *
 * @memberof StageEditor
 * @private
 * @param this          Function scope
 * @param pressedKey    The controller button pressed by the user
 */
static void StageEditor_translateEntity(StageEditor this, u32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(-this->translationStepSize),
            0,
            0
		};

		StageEditor_applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LR)
	{
		VBVec3D translation =
	    {
            ITOFIX19_13(this->translationStepSize),
            0,
            0
		};

		StageEditor_applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LU)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(-this->translationStepSize),
            0
		};

		StageEditor_applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LD)
	{
		VBVec3D translation =
	    {
            0,
            ITOFIX19_13(this->translationStepSize),
            0
		};

		StageEditor_applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_RR)
	{
		if(__MAX_TRANSLATION_STEP < ++this->translationStepSize)
	    {
			this->translationStepSize = __MAX_TRANSLATION_STEP;
		}

		StageEditor_printTranslationStepSize(this);
	}
	else if(pressedKey & K_RL)
	{
		if(1 > --this->translationStepSize)
    	{
			this->translationStepSize = 1;
		}

		StageEditor_printTranslationStepSize(this);
	}
	else if(pressedKey & K_RU)
	{
		VBVec3D translation =
    	{
            0,
            0,
            ITOFIX19_13(this->translationStepSize),
		};

		StageEditor_applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_RD)
	{
		VBVec3D translation =
	    {
            0,
            0,
            ITOFIX19_13(-this->translationStepSize),
		};

		StageEditor_applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LT)
	{
		StageEditor_selectPreviousEntity(this);
	}
	else if(pressedKey & K_RT)
	{
		StageEditor_selectNextEntity(this);
	}
}

/**
 * Apply a translation to an entity
 *
 * @memberof StageEditor
 * @private
 * @param this          Function scope
 * @param translation   Translation vector
 */
static void StageEditor_applyTranslationToEntity(StageEditor this, VBVec3D translation)
{
	if(this->currentEntityNode && this->shape)
	{
		Container container = __SAFE_CAST(Container, this->currentEntityNode->data);
		VBVec3D localPosition = *Container_getLocalPosition(container);

		localPosition.x += translation.x;
		localPosition.y += translation.y;
		localPosition.z += translation.z;

		__VIRTUAL_CALL(Container, setLocalPosition, container, &localPosition);
		Container_invalidateGlobalPosition(container, __XAXIS | __YAXIS | __ZAXIS);

		// this hack forces the Entity to recalculate its sprites' value.
		// must hack this global, otherwise will need another variable which most likely will only
		// take up the previous RAM, or another branching computation in the Entity's render method.
		Screen_forceDisplacement(Screen_getInstance(), true);

		GameState_transform(this->gameState);
		GameState_updateVisuals(this->gameState);

		StageEditor_positionShape(this);

		StageEditor_printEntityPosition(this);

		SpriteManager_sortLayers(SpriteManager_getInstance());

		StageEditor_printTranslationStepSize(this);

		// should work
		//__VIRTUAL_CALL(Shape, position, this->shape);
	}
}

/**
 * Remove previous sprite
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_removePreviousSprite(StageEditor this)
{
	if(this->userObjectSprite)
	{
		__DELETE(this->userObjectSprite);
		this->userObjectSprite = NULL;
	}
}

/**
 * Show selected user object
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_showSelectedUserObject(StageEditor this)
{
	StageEditor_removePreviousSprite(this);

	SpriteDefinition* spriteDefinition = (SpriteDefinition*)_userObjects[OptionsSelector_getSelectedOption(this->userObjectsSelector)].entityDefinition->spritesDefinitions[0];

	if(spriteDefinition)
	{
		this->userObjectSprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this));
		ASSERT(this->userObjectSprite, "AnimationEditor::createSprite: null animatedSprite");
		ASSERT(Sprite_getTexture(__SAFE_CAST(Sprite, this->userObjectSprite)), "AnimationEditor::createSprite: null texture");

		VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, __SAFE_CAST(Sprite, this->userObjectSprite));
		spritePosition.x = ITOFIX19_13((__SCREEN_WIDTH >> 1) - (Texture_getCols(Sprite_getTexture(__SAFE_CAST(Sprite, this->userObjectSprite))) << 2));
		spritePosition.y = ITOFIX19_13((__SCREEN_HEIGHT >> 1) - (Texture_getRows(Sprite_getTexture(__SAFE_CAST(Sprite, this->userObjectSprite))) << 2));

		__VIRTUAL_CALL(Sprite, setPosition, __SAFE_CAST(Sprite, this->userObjectSprite), &spritePosition);
		__VIRTUAL_CALL(Sprite, applyAffineTransformations, __SAFE_CAST(Sprite, this->userObjectSprite));
		__VIRTUAL_CALL(Sprite, render, __SAFE_CAST(Sprite, this->userObjectSprite));
	}
}

/**
 * Select user object
 *
 * @memberof StageEditor
 * @private
 * @param this          Function scope
 * @param pressedKey    The controller button pressed by the user
 */
static void StageEditor_selectUserObject(StageEditor this, u32 pressedKey)
{
	if(pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->userObjectsSelector);
		StageEditor_showSelectedUserObject(this);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->userObjectsSelector);
		StageEditor_showSelectedUserObject(this);
	}
	else if(pressedKey & K_A)
	{
		if(1 >= SpriteManager_getFreeLayer(SpriteManager_getInstance()))
	    {
			Printing_text(Printing_getInstance(), "No more WORLDs", 48 - 15, 5, NULL);
			Printing_text(Printing_getInstance(), "available     ", 48 - 15, 6, NULL);
			return;
		}

		VBVec3D position = Screen_getPosition(Screen_getInstance());

		position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
		position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
		position.z += ITOFIX19_13(0);

		Stage_addEntity(GameState_getStage(this->gameState), _userObjects[OptionsSelector_getSelectedOption(this->userObjectsSelector)].entityDefinition, NULL, &position, NULL, false);
		SpriteManager_sortLayers(SpriteManager_getInstance());

		VirtualList stageEntities = Container_getChildren(__SAFE_CAST(Container, GameState_getStage(this->gameState)));
		this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;

		// select the added entity
		this->mode = kTranslateEntities;
		StageEditor_setupMode(this);

		StageEditor_removePreviousSprite(this);
	}
}

/**
 * Print entity position
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_printEntityPosition(StageEditor this)
{
	int x = 1;
	int y = 2;

	Printing_text(Printing_getInstance(), "MOVE OBJECT", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing_text(Printing_getInstance(), "Next   \x17\x18", 38, 2, NULL);
	Printing_text(Printing_getInstance(), "Move\x1E\x1A\x1B\x1C\x1D", 38, 3, NULL);
	Printing_text(Printing_getInstance(), "      \x1F\x1A\x1B", 38, 4, NULL);

	if(this->currentEntityNode)
	{
		Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));
		const VBVec3D* globalPosition = Container_getGlobalPosition(__SAFE_CAST(Container, entity));
		char* entityName = Container_getName(__SAFE_CAST(Container, entity));

		Printing_text(Printing_getInstance(), "ID: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Container_getId(__SAFE_CAST(Container, entity)), x + 6, y, NULL);
		Printing_text(Printing_getInstance(), "Type:                                  ", x, ++y, NULL);
		Printing_text(Printing_getInstance(), __GET_CLASS_NAME_UNSAFE(entity), x + 6, y, NULL);
		Printing_text(Printing_getInstance(), "Name:                                  ", x, ++y, NULL);
		Printing_text(Printing_getInstance(), entityName ? entityName : "-", x + 6, y, NULL);
		Printing_text(Printing_getInstance(), "Pos. (x,y,z):                  ", x, ++y, NULL);
		Printing_float(Printing_getInstance(), FIX19_13TOF(globalPosition->x), x + 13, y, NULL);
		Printing_float(Printing_getInstance(), FIX19_13TOF(globalPosition->y), x + 22, y, NULL);
		Printing_float(Printing_getInstance(), FIX19_13TOF(globalPosition->z), x + 31, y, NULL);
		Printing_text(Printing_getInstance(), "Size (w,h,d):                  ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Entity_getWidth(entity), x + 13, y, NULL);
		Printing_int(Printing_getInstance(), Entity_getHeight(entity), x + 20, y, NULL);
		Printing_int(Printing_getInstance(), Entity_getDepth(entity), x + 27, y, NULL);
		Printing_text(Printing_getInstance(), "Is visible:                  ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Entity_isVisible(entity, 16, true), x + 13, y, NULL);
	}
}

/**
 * Apply a translation to the screen
 *
 * @memberof StageEditor
 * @private
 * @param this          Function scope
 * @param translation   Translation vector
 */
static void StageEditor_applyTranslationToScreen(StageEditor this, VBVec3D translation)
{
	Screen_move(Screen_getInstance(), translation, true);
	GameState_transform(this->gameState);
	StageEditor_printScreenPosition(this);
	Stage_streamAll(GameState_getStage(this->gameState));
	CollisionManager_processRemovedShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
	PhysicalWorld_processRemovedBodies(GameState_getPhysicalWorld(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
}

/**
 * Print the screen position
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_printScreenPosition(StageEditor this __attribute__ ((unused)))
{
	int x = 1;
	int y = 2;

	VBVec3D position = Screen_getPosition(Screen_getInstance());

	Printing_text(Printing_getInstance(), "MOVE SCREEN", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing_text(Printing_getInstance(), "Move\x1E\x1A\x1B\x1C\x1D", 38, 2, NULL);
	Printing_text(Printing_getInstance(), "      \x1F\x1A\x1B", 38, 3, NULL);
	Printing_text(Printing_getInstance(), "Position:               ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(position.x), x + 10, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(position.y), x + 15, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(position.z), x + 20, y, NULL);
}

/**
 * Print projection values
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_printProjectionValues(StageEditor this __attribute__ ((unused)))
{
	int x = 1;
	int y = 2;

	Printing_text(Printing_getInstance(), "PROJECTION VALUES", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing_text(Printing_getInstance(), "HVPC  \x1E\x1C\x1D", 38, 2, NULL);
	Printing_text(Printing_getInstance(), "VVPC  \x1E\x1A\x1B", 38, 3, NULL);
	Printing_text(Printing_getInstance(), "DES   \x1F\x1C\x1D", 38, 4, NULL);
	Printing_text(Printing_getInstance(), "MVD   \x1F\x1A\x1B", 38, 5, NULL);
	Printing_text(Printing_getInstance(), "BD     \x17\x18", 38, 6, NULL);

	Printing_text(Printing_getInstance(), "H. view point center:            ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(_optical->horizontalViewPointCenter), x + 22, y, NULL);
	Printing_text(Printing_getInstance(), "V. view point center:            ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(_optical->verticalViewPointCenter), x + 22, y, NULL);
	Printing_text(Printing_getInstance(), "Distance Eye Screen:            ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(_optical->distanceEyeScreen), x + 22, y, NULL);
	Printing_text(Printing_getInstance(), "Maximum View Screen:            ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), (1 << _optical->maximumViewDistancePower), x + 22, y, NULL);
	Printing_text(Printing_getInstance(), "Base Distance:                  ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(_optical->baseDistance), x + 22, y, NULL);
}

/**
 * Print user objects
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_printUserObjects(StageEditor this)
{
	Printing_text(Printing_getInstance(), "ADD OBJECTS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	Printing_text(Printing_getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing_text(Printing_getInstance(), "Accept  \x13", 38, 2, NULL);

	OptionsSelector_showOptions(this->userObjectsSelector, 1, 4);
}

/**
 * Print translation step size
 *
 * @memberof StageEditor
 * @private
 * @param this  Function scope
 */
static void StageEditor_printTranslationStepSize(StageEditor this)
{
	Printing_text(Printing_getInstance(), "Step  \x1F\x1C\x1D", 38, 5, NULL);
	Printing_text(Printing_getInstance(), "+     ", 38, 6, NULL);
	Printing_int(Printing_getInstance(), this->translationStepSize, 39, 6, NULL);
}


#endif
