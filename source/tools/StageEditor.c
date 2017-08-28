/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StageEditor.h>
#include <Game.h>
#include <Screen.h>
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
//											 CLASS' MACROS
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
//											CLASS' DEFINITION
//---------------------------------------------------------------------------------------------------------

#define StageEditor_ATTRIBUTES																			\
		Object_ATTRIBUTES																				\
		/**
		 * @var GameState		gameState
		 * @brief				Current game state
		 * @memberof			StageEditor
		 */																								\
		GameState gameState;																			\
		/**
		 * @var VirtualNode		currentEntityNode
		 * @brief				Current in game entity
		 * @memberof			StageEditor
		 */																								\
		VirtualNode currentEntityNode;																	\
		/**
		 * @var Shape			shape
		 * @brief				Current entity's shape
		 * @memberof			StageEditor
		 */																								\
		Shape shape;																					\
		/**
		 * @var int				mode
		 * @brief				Mode
		 * @memberof			StageEditor
		 */																								\
		int mode;																						\
		/**
		 * @var OptionsSelector userObjectsSelector
		 * @brief				Actors selector
		 * @memberof			StageEditor
		 */																								\
		OptionsSelector userObjectsSelector;															\
		/**
		 * @var int				translationStepSize
		 * @brief				Translation step size
		 * @memberof			StageEditor
		 */																								\
		int translationStepSize;																		\
		/**
		 * @var Sprite			userObjectSprite
		 * @brief				Current user's object's sprite
		 * @memberof			StageEditor
		 */																								\
		Sprite userObjectSprite;																		\

/**
 * @class	StageEditor
 * @extends Object
 * @ingroup tools
 * @brief	In-game stage editor for debug and productivity purposes
 */
__CLASS_DEFINITION(StageEditor, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);
__CLASS_FRIEND_DEFINITION(Sprite);

/**
 * The different modes of the StageEditor
 *
 * @memberof	AnimationInspector
 */
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
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern UserObject _userObjects[];

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
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			StageEditor_getInstance()
 * @memberof	StageEditor
 * @public
 *
 * @return		StageEditor instance
 */
__SINGLETON(StageEditor);

/**
 * Class constructor
 *
 * @memberof	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) StageEditor_constructor(StageEditor this)
{
	ASSERT(__SAFE_CAST(StageEditor, this), "StageEditor::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->currentEntityNode = NULL;
	this->gameState = NULL;
	this->userObjectSprite = NULL;
	this->mode = kFirstMode + 1;
	this->shape = NULL;
	this->userObjectsSelector = __NEW(OptionsSelector, 2, 12, NULL);

	VirtualList userObjects = __NEW(VirtualList);

	int i = 0;
	for(;  _userObjects[i].entityDefinition; i++)
	{
		Option* option = __NEW_BASIC(Option);
		option->value = _userObjects[i].name;
		option->type = kString;
		VirtualList_pushBack(userObjects, option);
	}

	if(VirtualList_getSize(userObjects))
	{
		OptionsSelector_setOptions(this->userObjectsSelector, userObjects);
	}

	__DELETE(userObjects);

	this->translationStepSize = 8;
}

/**
 * Class destructor
 *
 * @memberof	StageEditor
 * @public
 *
 * @param this	Function scope
 */
void StageEditor_destructor(StageEditor this)
{
	ASSERT(__SAFE_CAST(StageEditor, this), "StageEditor::destructor: null this");

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
 * @memberof	StageEditor
 * @public
 *
 * @param this	Function scope
 */
void StageEditor_update(StageEditor this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(StageEditor, this), "StageEditor::update: null this");
}

/**
 * Show editor
 *
 * @memberof		StageEditor
 * @public
 *
 * @param this		Function scope
 * @param gameState Current game state
 */
void StageEditor_show(StageEditor this, GameState gameState)
{
	ASSERT(__SAFE_CAST(StageEditor, this), "StageEditor::start: null this");
	ASSERT(gameState, "StageEditor::start: gameState this");

	this->gameState = gameState;
	this->mode = kFirstMode + 1;
	this->userObjectSprite = NULL;

	StageEditor_releaseShape(this);
	StageEditor_setupMode(this);
}

/**
 * Hide editor
 *
 * @memberof	StageEditor
 * @public
 *
 * @param this	Function scope
 */
void StageEditor_hide(StageEditor this)
{
	ASSERT(__SAFE_CAST(StageEditor, this), "StageEditor::stop: null this");

	CollisionManager_hideShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	StageEditor_removePreviousSprite(this);
	StageEditor_releaseShape(this);
	this->currentEntityNode = NULL;
}

/**
 * Process user input
 *
 * @memberof			StageEditor
 * @public
 *
 * @param this			Function scope
 * @param pressedKey	User input
 */
void StageEditor_processUserInput(StageEditor this, u16 pressedKey)
{
	ASSERT(__SAFE_CAST(StageEditor, this), "StageEditor::handleMessage: null this");

	if(!this->gameState)
	{
		return;
	}

	if(pressedKey & K_SEL)
	{
		this->mode++;

		if(kLastMode <= this->mode)
		{
			this->mode = kFirstMode + 1;
		}

		StageEditor_setupMode(this);
		return;
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

/**
 * Print header
 *
 * @memberof	StageEditor
 * @private
 *
 * @param this	Function scope
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
 * @memberof	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_setupMode(StageEditor this)
{
	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
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
 * @memberof	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_releaseShape(StageEditor this)
{
	if(this->currentEntityNode)
	{
		Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));

		VirtualList shapes = __VIRTUAL_CALL(Entity, getShapes, entity);
		VirtualNode node = shapes->head;

		for(; node; node = node->next)
		{
			if(this->shape == __SAFE_CAST(Shape, node->data))
			{
				break;
			}
		}

		if(this->shape && !node)
		{
			__DELETE(this->shape);
		}
		else if(this->shape)
		{
			__VIRTUAL_CALL(Shape, hide, this->shape);
		}

		this->shape = NULL;
	}
}

/**
 * Get shape
 *
 * @memberof	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_getShape(StageEditor this)
{
	if(!this->currentEntityNode)
	{
		return;
	}

	Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));
	VirtualList shapes = __VIRTUAL_CALL(Entity, getShapes, entity);

	this->shape = shapes ? __SAFE_CAST(Shape, VirtualList_front(shapes)) : NULL;

	if(!this->shape)
	{
		this->shape = __SAFE_CAST(Shape, __NEW(Cuboid, __SAFE_CAST(SpatialObject, entity)));

		Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));
		VBVec3D displacement = {0, 0, 0};
		Size size = {Entity_getWidth(entity), Entity_getHeight(entity), Entity_getDepth(entity)};

		__VIRTUAL_CALL(Shape, setup, this->shape, Entity_getPosition(entity), &size, &displacement, false);
	}

	Shape_setReady(this->shape, false);
}

/**
 * Position shape
 *
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_positionShape(StageEditor this)
{
	if(!this->currentEntityNode || !this->shape)
	{
		return;
	}

	Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(this->currentEntityNode));

	if(__VIRTUAL_CALL(Entity, moves, entity))
	{
		__VIRTUAL_CALL(Shape, position, this->shape, Entity_getPosition(entity), false);
	}

	if(this->shape)
	{
		__VIRTUAL_CALL(Shape, show, this->shape);
	}
}

/**
 * Highlight entity
 *
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
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
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
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
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
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
 * @memberof 			StageEditor
 * @private
 *
 * @param this			Function scope
 * @param pressedKey	The controller button pressed by the user
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
 *
 * @param this			Function scope
 * @param pressedKey	The controller button pressed by the user
 */
static void StageEditor_changeProjection(StageEditor this, u32 pressedKey)
{
	Optical optical = *_optical;

	if(pressedKey & K_LL)
	{
		optical.horizontalViewPointCenter -= __HVPC_STEP;
	}
	else if(pressedKey & K_LR)
	{
		optical.horizontalViewPointCenter += __HVPC_STEP;
	}
	else if(pressedKey & K_LU)
	{
		optical.verticalViewPointCenter -= __VERTICAL_VIEW_POINT_CENTER_STEP;
	}
	else if(pressedKey & K_LD)
	{
		optical.verticalViewPointCenter += __VERTICAL_VIEW_POINT_CENTER_STEP;
	}
	else if(pressedKey & K_RL)
	{
		optical.distanceEyeScreen -= __DISTANCE_EYE_SCREEN_STEP;
	}
	else if(pressedKey & K_RR)
	{
		optical.distanceEyeScreen += __DISTANCE_EYE_SCREEN_STEP;
	}
	else if(pressedKey & K_RU)
	{
		optical.maximumViewDistancePower += __MAXIMUM_VIEW_DISTACE_STEP;
	}
	else if(pressedKey & K_RD)
	{
		optical.maximumViewDistancePower -= __MAXIMUM_VIEW_DISTACE_STEP;

		if(0 >= optical.maximumViewDistancePower)
		{
			optical.maximumViewDistancePower = 1;
		}
	}
	else if(pressedKey & K_LT)
	{
		optical.baseDistance -= __BASE_DISTACE_STEP;
	}
	else if(pressedKey & K_RT)
	{
		optical.baseDistance += __BASE_DISTACE_STEP;
	}

	Screen_setOptical(Screen_getInstance(), optical);

	// this hack forces the Entity to recalculate its sprites' value.
	// must hack this global, otherwise will need another variable which most likely will only
	// take up the previous RAM, or another branching computation in the Entity's render method.
	Screen_forceDisplacement(Screen_getInstance(), true);

	StageEditor_printProjectionValues(this);
	GameState_transform(this->gameState);
	GameState_synchronizeGraphics(this->gameState);
}

/**
 * Translate an entity
 *
 * @memberof 			StageEditor
 * @private
 *
 * @param this			Function scope
 * @param pressedKey	The controller button pressed by the user
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
 * @memberof 			StageEditor
 * @private
 *
 * @param this			Function scope
 * @param translation	Translation vector
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
		Container_invalidateGlobalPosition(container);

		// this hack forces the Entity to recalculate its sprites' value.
		// must hack this global, otherwise will need another variable which most likely will only
		// take up the previous RAM, or another branching computation in the Entity's render method.
		Screen_forceDisplacement(Screen_getInstance(), true);

		GameState_transform(this->gameState);
		GameState_synchronizeGraphics(this->gameState);

		StageEditor_positionShape(this);

		StageEditor_printEntityPosition(this);

		SpriteManager_sortLayers(SpriteManager_getInstance());

		StageEditor_printTranslationStepSize(this);
	}
}

/**
 * Remove previous sprite
 *
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_removePreviousSprite(StageEditor this)
{
	if(this->userObjectSprite)
	{
		__DELETE(this->userObjectSprite);
		this->userObjectSprite = NULL;
	}

	SpriteManager_sortLayers(SpriteManager_getInstance());
}

/**
 * Show selected user object
 *
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_showSelectedUserObject(StageEditor this)
{
	StageEditor_removePreviousSprite(this);

	SpriteDefinition* spriteDefinition = (SpriteDefinition*)_userObjects[OptionsSelector_getSelectedOption(this->userObjectsSelector)].entityDefinition->spriteDefinitions[0];

	if(spriteDefinition)
	{
		this->userObjectSprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this));
		ASSERT(this->userObjectSprite, "AnimationInspector::createSprite: null animatedSprite");
		ASSERT(Sprite_getTexture(__SAFE_CAST(Sprite, this->userObjectSprite)), "AnimationInspector::createSprite: null texture");

		VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, __SAFE_CAST(Sprite, this->userObjectSprite));
		spritePosition.x = ITOFIX19_13((__HALF_SCREEN_WIDTH) - (Texture_getCols(Sprite_getTexture(__SAFE_CAST(Sprite, this->userObjectSprite))) << 2));
		spritePosition.y = ITOFIX19_13((__HALF_SCREEN_HEIGHT) - (Texture_getRows(Sprite_getTexture(__SAFE_CAST(Sprite, this->userObjectSprite))) << 2));

		Rotation spriteRotation = {0, 0, 0};
		Scale spriteScale = {__1I_FIX7_9, __1I_FIX7_9};
		__VIRTUAL_CALL(Sprite, setPosition, this->userObjectSprite, &spritePosition);
		__VIRTUAL_CALL(Sprite, rotate, this->userObjectSprite, &spriteRotation);
		__VIRTUAL_CALL(Sprite, resize, this->userObjectSprite, spriteScale, spritePosition.z);
		__VIRTUAL_CALL(Sprite, calculateParallax, this->userObjectSprite, spritePosition.z);

		this->userObjectSprite->writeAnimationFrame = true;
		SpriteManager_writeTextures(SpriteManager_getInstance());
		SpriteManager_sortLayers(SpriteManager_getInstance());
		SpriteManager_deferParamTableEffects(SpriteManager_getInstance(), false);
		SpriteManager_render(SpriteManager_getInstance());
		SpriteManager_deferParamTableEffects(SpriteManager_getInstance(), true);
	}
}

/**
 * Select user object
 *
 * @memberof 			StageEditor
 * @private
 *
 * @param this			Function scope
 * @param pressedKey	The controller button pressed by the user
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

		position.x += ITOFIX19_13(__HALF_SCREEN_WIDTH);
		position.y += ITOFIX19_13(__HALF_SCREEN_HEIGHT);
		position.z += ITOFIX19_13(0);

		PositionedEntity DUMMY_ENTITY =
		{
			(EntityDefinition*)_userObjects[OptionsSelector_getSelectedOption(this->userObjectsSelector)].entityDefinition,
			position,
			0,
			NULL,
			NULL,
			NULL,
			false
		};

		Stage_addChildEntity(GameState_getStage(this->gameState), &DUMMY_ENTITY, false);
		SpriteManager_sortLayers(SpriteManager_getInstance());

		VirtualList stageEntities = Container_getChildren(__SAFE_CAST(Container, GameState_getStage(this->gameState)));
		this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;

		// select the added entity
		this->mode = kTranslateEntities;
		StageEditor_setupMode(this);

		StageEditor_removePreviousSprite(this);
		SpriteManager_sortLayers(SpriteManager_getInstance());
		SpriteManager_writeTextures(SpriteManager_getInstance());
		SpriteManager_deferParamTableEffects(SpriteManager_getInstance(), false);
		SpriteManager_render(SpriteManager_getInstance());
		SpriteManager_deferParamTableEffects(SpriteManager_getInstance(), true);
	}
}

/**
 * Print entity position
 *
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
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
		Printing_int(Printing_getInstance(), Entity_getInternalId(__SAFE_CAST(Entity, entity)), x + 6, y, NULL);
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
		Printing_text(Printing_getInstance(), Entity_isVisible(entity, 16, true) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 13, y, NULL);
		Printing_text(Printing_getInstance(), "Children:                  ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Container_getChildCount(__SAFE_CAST(Container, entity)), x + 13, y, NULL);
	}
}

/**
 * Apply a translation to the screen
 *
 * @memberof 			StageEditor
 * @private
 *
 * @param this			Function scope
 * @param translation   Translation vector
 */
static void StageEditor_applyTranslationToScreen(StageEditor this, VBVec3D translation)
{
	Screen_move(Screen_getInstance(), translation, true);
	GameState_transform(this->gameState);
	GameState_synchronizeGraphics(this->gameState);
	StageEditor_printScreenPosition(this);
	Stage_streamAll(GameState_getStage(this->gameState));
	CollisionManager_processRemovedShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
	PhysicalWorld_processRemovedBodies(GameState_getPhysicalWorld(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
}

/**
 * Print the screen position
 *
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
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
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
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
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_printUserObjects(StageEditor this)
{
	Printing_text(Printing_getInstance(), "ADD OBJECTS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	Printing_text(Printing_getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing_text(Printing_getInstance(), "Accept  \x13", 38, 2, NULL);

	OptionsSelector_printOptions(this->userObjectsSelector, 1, 4);
}

/**
 * Print translation step size
 *
 * @memberof 	StageEditor
 * @private
 *
 * @param this	Function scope
 */
static void StageEditor_printTranslationStepSize(StageEditor this)
{
	Printing_text(Printing_getInstance(), "Step  \x1F\x1C\x1D", 38, 5, NULL);
	Printing_text(Printing_getInstance(), "+     ", 38, 6, NULL);
	Printing_int(Printing_getInstance(), this->translationStepSize, 39, 6, NULL);
}
