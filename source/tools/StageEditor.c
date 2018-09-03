/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Camera.h>
#include <Optics.h>
#include <Entity.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <SpriteManager.h>
#include <GameState.h>
#include <Stage.h>
#include <Shape.h>
#include <Camera.h>
#include <Box.h>
#include <OptionsSelector.h>
#include <KeypadManager.h>
#include <BgmapTextureManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __USER_OBJECT_SHOW_ROW 				6
#define __MAX_TRANSLATION_STEP				__PIXELS_TO_METERS(8 * 4)
#define __SCREEN_X_TRANSLATION_STEP			__PIXELS_TO_METERS(__SCREEN_WIDTH / 4)
#define __SCREEN_Y_TRANSLATION_STEP			__PIXELS_TO_METERS(__SCREEN_HEIGHT / 4)
#define __SCREEN_Z_TRANSLATION_STEP			__PIXELS_TO_METERS(__SCREEN_HEIGHT / 4)

#define __HVPC_STEP							__PIXELS_TO_METERS(8)
#define __VERTICAL_VIEW_POINT_CENTER_STEP	__PIXELS_TO_METERS(8)
#define __DISTANCE_EYE_SCREEN_STEP			__PIXELS_TO_METERS(8)
#define __MAXIMUM_VIEW_DISTACE_STEP			1
#define __BASE_DISTACE_STEP					__PIXELS_TO_METERS(8)


//---------------------------------------------------------------------------------------------------------
//											CLASS' DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;
friend class Sprite;
friend class Container;


//---------------------------------------------------------------------------------------------------------
//												ENUMS
//---------------------------------------------------------------------------------------------------------

/**
 * The different modes of the StageEditor
 *
 * @memberof	AnimationInspector
 */
enum Modes
{
	kFirstMode = 0,
	kMoveCamera,
	kChangeProjection,
	kTranslateEntities,
	kAddObjects,

	kLastMode
};


//---------------------------------------------------------------------------------------------------------
//												GLOBALS
//---------------------------------------------------------------------------------------------------------

// globals
extern UserObject _userObjects[];


//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			StageEditor::getInstance()
 * @memberof	StageEditor
 * @public
 * @return		StageEditor instance
 */


/**
 * Class constructor
 *
 * @private
 */
void StageEditor::constructor()
{
	Base::constructor();

	this->currentEntityNode = NULL;
	this->gameState = NULL;
	this->userObjectSprite = NULL;
	this->mode = kFirstMode + 1;
	this->shape = NULL;
	this->userObjectsSelector = new OptionsSelector(2, 12, NULL);

	VirtualList userObjects = new VirtualList();

	int i = 0;
	for(;  _userObjects[i].entityDefinition; i++)
	{
		Option* option = new Option;
		option->value = _userObjects[i].name;
		option->type = kString;
		VirtualList::pushBack(userObjects, option);
	}

	if(VirtualList::getSize(userObjects))
	{
		OptionsSelector::setOptions(this->userObjectsSelector, userObjects);
	}

	delete userObjects;

	this->translationStepSize = 1;
}

/**
 * Class destructor
 */
void StageEditor::destructor()
{
	if(this->userObjectsSelector)
	{
		delete this->userObjectsSelector;
	}

	// allow a new construct
	Base::destructor();
}

/**
 * Update
 */
void StageEditor::update()
{}

/**
 * Show editor
 *
 * @param gameState Current game state
 */
void StageEditor::show(GameState gameState)
{
	ASSERT(gameState, "StageEditor::start: gameState this");

	this->gameState = gameState;
	this->mode = kFirstMode + 1;
	this->userObjectSprite = NULL;

	StageEditor::releaseShape(this);
	StageEditor::setupMode(this);
}

/**
 * Hide editor
 */
void StageEditor::hide()
{
	CollisionManager::hideShapes(GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance())))));
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	StageEditor::removePreviousSprite(this);
	StageEditor::releaseShape(this);
	this->currentEntityNode = NULL;
}

/**
 * Process user input
 *
 * @param pressedKey	User input
 */
void StageEditor::processUserInput(u16 pressedKey)
{
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

		StageEditor::setupMode(this);
		return;
	}

	switch(this->mode)
	{
		case kMoveCamera:

			StageEditor::moveCamera(this, pressedKey);
			break;

		case kChangeProjection:

			StageEditor::changeProjection(this, pressedKey);
			break;

		case kTranslateEntities:

			StageEditor::translateEntity(this, pressedKey);
			break;

		case kAddObjects:

			StageEditor::selectUserObject(this, pressedKey);
			break;
	}
}

/**
 * Print header
 *
 * @private
 */
void StageEditor::printHeader()
{
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing::text(Printing::getInstance(), " LEVEL EDITOR ", 1, 0, NULL);
	Printing::text(Printing::getInstance(), "  /  ", 16, 0, NULL);
	Printing::int(Printing::getInstance(), this->mode, 17, 0, NULL);
	Printing::int(Printing::getInstance(), kLastMode - 1, 19, 0, NULL);
}

/**
 * Print title
 *
 * @private
 */
void StageEditor::setupMode()
{
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	StageEditor::printHeader(this);
	StageEditor::removePreviousSprite(this);

	switch(this->mode)
	{
		case kAddObjects:

			if(OptionsSelector::getNumberOfOptions(this->userObjectsSelector))
			{
				StageEditor::releaseShape(this);
				StageEditor::printUserObjects(this);
				StageEditor::showSelectedUserObject(this);
				break;
			}

			this->mode = kMoveCamera;

		case kMoveCamera:

			StageEditor::releaseShape(this);
			StageEditor::printCameraPosition(this);
			break;

		case kChangeProjection:

			StageEditor::releaseShape(this);
			StageEditor::printProjectionValues(this);
			break;

		case kTranslateEntities:

			if(!this->currentEntityNode)
			{
				StageEditor::selectNextEntity(this);
			}
			else
			{
				StageEditor::getShape(this);
				StageEditor::highLightEntity(this);
			}

			StageEditor::printEntityPosition(this);
			StageEditor::printTranslationStepSize(this);
			break;
	}
}

/**
 * Release shape
 *
 * @private
 */
void StageEditor::releaseShape()
{
	if(this->currentEntityNode)
	{
		Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));

		VirtualList shapes =  Entity::getShapes(entity);

		if(shapes)
		{
			VirtualNode node = shapes->head;

			for(; node; node = node->next)
			{
				if(this->shape == Shape::safeCast(node->data))
				{
					break;
				}
			}

			if(this->shape && !node)
			{
				delete this->shape;
			}
			else if(this->shape)
			{
				Shape::hide(this->shape);
			}
		}
		else if(this->shape)
		{
			delete this->shape;
		}

		this->shape = NULL;
	}
}

/**
 * Get shape
 *
 * @private
 */
void StageEditor::getShape()
{
	if(!this->currentEntityNode)
	{
		return;
	}

	Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
	VirtualList shapes =  Entity::getShapes(entity);

	this->shape = shapes ? Shape::safeCast(VirtualList::front(shapes)) : NULL;

	if(!this->shape)
	{
		this->shape = Shape::safeCast(new Box(SpatialObject::safeCast(entity)));

		Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
		Size size = {Entity::getWidth(entity), Entity::getHeight(entity), 0};

			Shape::position(this->shape, Entity::getPosition(entity), Entity::getRotation(entity), Entity::getScale(entity), &size);
	}

	Shape::setReady(this->shape, false);
}

/**
 * Position shape
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::positionShape()
{
	if(!this->currentEntityNode || !this->shape)
	{
		return;
	}

	Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
	Size size = {Entity::getWidth(entity), Entity::getHeight(entity), 0};

		Shape::position(this->shape, Entity::getPosition(entity), Entity::getRotation(entity), Entity::getScale(entity), &size);

	if(this->shape)
	{
		Shape::show(this->shape);
	}
}

/**
 * Highlight entity
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::highLightEntity()
{
	if(this->currentEntityNode)
	{
		StageEditor::printEntityPosition(this);
		StageEditor::positionShape(this);
	}
	else
	{
		Printing::text(Printing::getInstance(), "No entities in stage", 1, 4, NULL);
	}
}

/**
 * Select the previous entity
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::selectPreviousEntity()
{
	StageEditor::releaseShape(this);

	VirtualList stageEntities = (Container::safeCast(GameState::getStage(this->gameState)))->children;

	if(!this->currentEntityNode)
	{
		this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;
	}
	else
	{
		this->currentEntityNode = VirtualNode::getPrevious(this->currentEntityNode);

		if(!this->currentEntityNode)
		{
			this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;
		}
	}

	if(this->currentEntityNode)
	{
		StageEditor::getShape(this);
		StageEditor::highLightEntity(this);
	}
}

/**
 * Select the next entity
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::selectNextEntity()
{
	StageEditor::releaseShape(this);

	VirtualList stageEntities = (Container::safeCast(GameState::getStage(this->gameState)))->children;

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
		StageEditor::getShape(this);
		StageEditor::highLightEntity(this);
	}
}

/**
 * Move the camera
 *
 * @memberof 			StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the user
 */
void StageEditor::moveCamera(u32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		Vector3D translation =
		{
			-__SCREEN_X_TRANSLATION_STEP,
			0,
			0
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_LR)
	{
		Vector3D translation =
		{
			__SCREEN_X_TRANSLATION_STEP,
			0,
			0
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_LU)
	{
		Vector3D translation =
		{
			0,
			-__SCREEN_Y_TRANSLATION_STEP,
			0
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_LD)
	{
		Vector3D translation =
		{
			0,
			__SCREEN_Y_TRANSLATION_STEP,
			0
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_RU)
	{
		Vector3D translation =
		{
			0,
			0,
			__SCREEN_Z_TRANSLATION_STEP,
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_RD)
	{
		Vector3D translation =
		{
			0,
			0,
			-__SCREEN_Z_TRANSLATION_STEP,
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
}

/**
 * Modify projection values
 *
 * @memberof StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the user
 */
void StageEditor::changeProjection(u32 pressedKey)
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
		optical.maximumXViewDistancePower += __MAXIMUM_VIEW_DISTACE_STEP;
		optical.maximumYViewDistancePower += __MAXIMUM_VIEW_DISTACE_STEP;
	}
	else if(pressedKey & K_RD)
	{
		optical.maximumXViewDistancePower -= __MAXIMUM_VIEW_DISTACE_STEP;
		optical.maximumYViewDistancePower -= __MAXIMUM_VIEW_DISTACE_STEP;

		if(0 >= optical.maximumXViewDistancePower)
		{
			optical.maximumXViewDistancePower = 1;
		}

		if(0 >= optical.maximumYViewDistancePower)
		{
			optical.maximumYViewDistancePower = 1;
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

	Camera::setOptical(Camera::getInstance(), optical);

	// this hack forces the Entity to recalculate its sprites' value.
	// must hack this global, otherwise will need another variable which most likely will only
	// take up the previous RAM, or another branching computation in the Entity's render method.
	Camera::forceDisplacement(Camera::getInstance(), true);

	StageEditor::printProjectionValues(this);
	GameState::transform(this->gameState);
	GameState::synchronizeGraphics(this->gameState);
}

/**
 * Translate an entity
 *
 * @memberof 			StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the user
 */
void StageEditor::translateEntity(u32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		Vector3D translation =
		{
			__I_TO_FIX10_6(-this->translationStepSize),
			0,
			0
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LR)
	{
		Vector3D translation =
		{
			__I_TO_FIX10_6(this->translationStepSize),
			0,
			0
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LU)
	{
		Vector3D translation =
		{
			0,
			__I_TO_FIX10_6(-this->translationStepSize),
			0
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LD)
	{
		Vector3D translation =
		{
			0,
			__I_TO_FIX10_6(this->translationStepSize),
			0
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_RR)
	{
		if(__MAX_TRANSLATION_STEP < ++this->translationStepSize)
		{
			this->translationStepSize = __MAX_TRANSLATION_STEP;
		}

		StageEditor::printTranslationStepSize(this);
	}
	else if(pressedKey & K_RL)
	{
		if(1 > --this->translationStepSize)
		{
			this->translationStepSize = 1;
		}

		StageEditor::printTranslationStepSize(this);
	}
	else if(pressedKey & K_RU)
	{
		Vector3D translation =
		{
			0,
			0,
			__I_TO_FIX10_6(this->translationStepSize),
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_RD)
	{
		Vector3D translation =
		{
			0,
			0,
			__I_TO_FIX10_6(-this->translationStepSize),
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LT)
	{
		StageEditor::selectPreviousEntity(this);
	}
	else if(pressedKey & K_RT)
	{
		StageEditor::selectNextEntity(this);
	}
}

/**
 * Apply a translation to an entity
 *
 * @memberof 			StageEditor
 * @private
 * @param translation	Translation vector
 */
void StageEditor::applyTranslationToEntity(Vector3D translation)
{
	if(this->currentEntityNode && this->shape)
	{
		Container container = Container::safeCast(this->currentEntityNode->data);
		Vector3D localPosition = *Container::getLocalPosition(container);

		localPosition.x += translation.x;
		localPosition.y += translation.y;
		localPosition.z += translation.z;

			Container::setLocalPosition(container, &localPosition);
		Container::invalidateGlobalPosition(container);

		// this hack forces the Entity to recalculate its sprites' value.
		// must hack this global, otherwise will need another variable which most likely will only
		// take up the previous RAM, or another branching computation in the Entity's render method.
		Camera::forceDisplacement(Camera::getInstance(), true);

		GameState::transform(this->gameState);
		GameState::synchronizeGraphics(this->gameState);

		StageEditor::positionShape(this);

		StageEditor::printEntityPosition(this);

		SpriteManager::sortLayers(SpriteManager::getInstance());

		StageEditor::printTranslationStepSize(this);
	}
}

/**
 * Remove previous sprite
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::removePreviousSprite()
{
	if(this->userObjectSprite)
	{
		delete this->userObjectSprite;
		this->userObjectSprite = NULL;
	}

	SpriteManager::sortLayers(SpriteManager::getInstance());
}

/**
 * Show selected user object
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::showSelectedUserObject()
{
	StageEditor::removePreviousSprite(this);

	SpriteDefinition* spriteDefinition = (SpriteDefinition*)_userObjects[OptionsSelector::getSelectedOption(this->userObjectsSelector)].entityDefinition->spriteDefinitions[0];

	if(spriteDefinition)
	{
		this->userObjectSprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, Object::safeCast(this));
		ASSERT(this->userObjectSprite, "AnimationInspector::createSprite: null animatedSprite");
		ASSERT(Sprite::getTexture(this->userObjectSprite), "AnimationInspector::createSprite: null texture");

		PixelVector spritePosition = Sprite::getDisplacedPosition(this->userObjectSprite);
		spritePosition.x = __I_TO_FIX10_6((__HALF_SCREEN_WIDTH) - (Texture::getCols(Sprite::getTexture(this->userObjectSprite)) << 2));
		spritePosition.y = __I_TO_FIX10_6((__HALF_SCREEN_HEIGHT) - (Texture::getRows(Sprite::getTexture(this->userObjectSprite)) << 2));

		Rotation spriteRotation = {0, 0, 0};
		Scale spriteScale = {__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
			Sprite::setPosition(this->userObjectSprite, &spritePosition);
		Sprite::rotate(this->userObjectSprite, &spriteRotation);
		Sprite::resize(this->userObjectSprite, spriteScale, spritePosition.z);
		Sprite::calculateParallax(this->userObjectSprite, spritePosition.z);

		this->userObjectSprite->writeAnimationFrame = true;
		SpriteManager::writeTextures(SpriteManager::getInstance());
		SpriteManager::sortLayers(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
		SpriteManager::render(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
	}
}

/**
 * Select user object
 *
 * @memberof 			StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the user
 */
void StageEditor::selectUserObject(u32 pressedKey)
{
	if(pressedKey & K_LU)
	{
		OptionsSelector::selectPrevious(this->userObjectsSelector);
		StageEditor::showSelectedUserObject(this);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector::selectNext(this->userObjectsSelector);
		StageEditor::showSelectedUserObject(this);
	}
	else if(pressedKey & K_A)
	{
		if(1 >= SpriteManager::getFreeLayer(SpriteManager::getInstance()))
		{
			Printing::text(Printing::getInstance(), "No more WORLDs", 48 - 15, 5, NULL);
			Printing::text(Printing::getInstance(), "available     ", 48 - 15, 6, NULL);
			return;
		}

		Vector3D cameraPosition = Camera::getPosition(Camera::getInstance());

		ScreenPixelVector position =
		{
			__METERS_TO_PIXELS(cameraPosition.x) + __HALF_SCREEN_WIDTH,
			__METERS_TO_PIXELS(cameraPosition.y) + __HALF_SCREEN_HEIGHT,
			__METERS_TO_PIXELS(cameraPosition.z),
			0
		};

		PositionedEntity DUMMY_ENTITY =
		{
			(EntityDefinition*)_userObjects[OptionsSelector::getSelectedOption(this->userObjectsSelector)].entityDefinition,
			position,
			0,
			NULL,
			NULL,
			NULL,
			false
		};

		Stage::addChildEntity(GameState::getStage(this->gameState), &DUMMY_ENTITY, false);
		SpriteManager::sortLayers(SpriteManager::getInstance());

		VirtualList stageEntities = (Container::safeCast(GameState::getStage(this->gameState)))->children;
		this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;

		// select the added entity
		this->mode = kTranslateEntities;
		StageEditor::setupMode(this);

		StageEditor::removePreviousSprite(this);
		SpriteManager::sortLayers(SpriteManager::getInstance());
		SpriteManager::writeTextures(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
		SpriteManager::render(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
	}
}

/**
 * Print entity position
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printEntityPosition()
{
	int x = 1;
	int y = 2;

	Printing::text(Printing::getInstance(), "MOVE OBJECT", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing::text(Printing::getInstance(), "Next   \x17\x18", 38, 2, NULL);
	Printing::text(Printing::getInstance(), "Move\x1E\x1A\x1B\x1C\x1D", 38, 3, NULL);
	Printing::text(Printing::getInstance(), "      \x1F\x1A\x1B", 38, 4, NULL);

	if(this->currentEntityNode)
	{
		Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
		const Vector3D* globalPosition =  SpatialObject::getPosition(entity);
		const Rotation* globalRotation =  SpatialObject::getRotation(entity);
		const Scale* globalScale =  SpatialObject::getScale(entity);
		char* entityName = Container::getName(entity);

		Printing::text(Printing::getInstance(), "ID: ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), Entity::getInternalId(entity), x + 6, y, NULL);
		Printing::text(Printing::getInstance(), "Type:                                  ", x, ++y, NULL);
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(entity), x + 6, y, NULL);
		Printing::text(Printing::getInstance(), "Name:                                  ", x, ++y, NULL);
		Printing::text(Printing::getInstance(), entityName ? entityName : "-", x + 6, y, NULL);
		Printing::text(Printing::getInstance(), "Pos. (x,y,z):                  ", x, ++y, NULL);
		Printing::float(Printing::getInstance(), __FIX10_6_TO_F(globalPosition->x), x + 13, y, NULL);
		Printing::float(Printing::getInstance(), __FIX10_6_TO_F(globalPosition->y), x + 25, y, NULL);
		Printing::float(Printing::getInstance(), __FIX10_6_TO_F(globalPosition->z), x + 31, y, NULL);
		Printing::text(Printing::getInstance(), "Rot. (x,y,z):                  ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), globalRotation->x, x + 13, y, NULL);
		Printing::int(Printing::getInstance(), globalRotation->y, x + 25, y, NULL);
		Printing::int(Printing::getInstance(), globalRotation->z, x + 31, y, NULL);
		Printing::text(Printing::getInstance(), "Scl. (x,y,z):                  ", x, ++y, NULL);
		Printing::float(Printing::getInstance(), __FIX7_9_TO_F(globalScale->x), x + 13, y, NULL);
		Printing::float(Printing::getInstance(), __FIX7_9_TO_F(globalScale->y), x + 25, y, NULL);
		Printing::float(Printing::getInstance(), __FIX7_9_TO_F(globalScale->z), x + 31, y, NULL);
		Printing::text(Printing::getInstance(), "Size (w,h,d):                  ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(Entity::getWidth(entity)), x + 13, y, NULL);
		Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(Entity::getHeight(entity)), x + 20, y, NULL);
		Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(Entity::getDepth(entity)), x + 27, y, NULL);
		Printing::text(Printing::getInstance(), "Is visible:                  ", x, ++y, NULL);
		Printing::text(Printing::getInstance(), Entity::isVisible(entity, 16, true) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 13, y, NULL);
		Printing::text(Printing::getInstance(), "Children:                  ", x, ++y, NULL);
		Printing::int(Printing::getInstance(), Container::getChildCount(entity), x + 13, y, NULL);
	}
}

/**
 * Apply a translation to the camera
 *
 * @memberof 			StageEditor
 * @private
 * @param translation   Translation vector
 */
void StageEditor::applyTranslationToCamera(Vector3D translation)
{
	Camera::move(Camera::getInstance(), translation, true);
	GameState::transform(this->gameState);
	GameState::synchronizeGraphics(this->gameState);
	StageEditor::printCameraPosition(this);
	Stage::streamAll(GameState::getStage(this->gameState));
}

/**
 * Print the camera position
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printCameraPosition()
{
	Camera::print(Camera::getInstance(), 1, 2);
}

/**
 * Print projection values
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printProjectionValues()
{
	int x = 1;
	int y = 2;

	Printing::text(Printing::getInstance(), "PROJECTION VALUES", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing::text(Printing::getInstance(), "HVPC  \x1E\x1C\x1D", 38, 2, NULL);
	Printing::text(Printing::getInstance(), "VVPC  \x1E\x1A\x1B", 38, 3, NULL);
	Printing::text(Printing::getInstance(), "DES   \x1F\x1C\x1D", 38, 4, NULL);
	Printing::text(Printing::getInstance(), "MVD   \x1F\x1A\x1B", 38, 5, NULL);
	Printing::text(Printing::getInstance(), "BD     \x17\x18", 38, 6, NULL);

	Printing::text(Printing::getInstance(), "H. view point center:            ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(_optical->horizontalViewPointCenter), x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "V. view point center:            ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(_optical->verticalViewPointCenter), x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Distance Eye Camera:            ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(_optical->distanceEyeScreen), x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Maximum X View Distance:            ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), _optical->maximumXViewDistancePower, x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Maximum Y View Distance:            ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), _optical->maximumYViewDistancePower, x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Base Distance:                  ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(_optical->baseDistance), x + 25, y, NULL);
}

/**
 * Print user objects
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printUserObjects()
{
	Printing::text(Printing::getInstance(), "ADD OBJECTS", 1, 2, NULL);
	Printing::text(Printing::getInstance(), "                       ", 1, 3, NULL);
	Printing::text(Printing::getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing::text(Printing::getInstance(), "Accept  \x13", 38, 2, NULL);

	OptionsSelector::printOptions(this->userObjectsSelector, 1, 4);
}

/**
 * Print translation step size
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printTranslationStepSize()
{
	Printing::text(Printing::getInstance(), "Step  \x1F\x1C\x1D", 38, 5, NULL);
	Printing::text(Printing::getInstance(), "+     ", 38, 6, NULL);
	Printing::int(Printing::getInstance(), this->translationStepSize, 39, 6, NULL);
}
