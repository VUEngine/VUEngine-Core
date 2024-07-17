/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapTextureManager.h>
#include <Box.h>
#include <Camera.h>
#include <CollisionManager.h>
#include <Debug.h>
#include <Entity.h>
#include <GameState.h>
#include <KeypadManager.h>
#include <Optics.h>
#include <OptionsSelector.h>
#include <PhysicalWorld.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <Collider.h>
#include <Stage.h>
#include <StateMachine.h>
#include <VirtualList.h>
#include <VUEngine.h>

#include "StageEditor.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __DEFAULT_TRANSLATION_STEP			8
#define __MAX_TRANSLATION_STEP				__PIXELS_TO_METERS(32)
#define __MAXIMUM_VIEW_DISTANCE_STEP		1


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
	this->collider = NULL;
	this->userObjectsSelector = new OptionsSelector(2, 12, NULL, NULL, NULL);

	VirtualList userObjects = new VirtualList();

	int32 i = 0;
	for(;  _userObjects[i].entitySpec; i++)
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

	this->translationStepSize = __DEFAULT_TRANSLATION_STEP;
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
void StageEditor::show()
{
	this->mode = kFirstMode + 1;
	this->userObjectSprite = NULL;

	StageEditor::releaseCollider(this);
	StageEditor::setupMode(this);

	StageEditor::dimmGame(this);
}

/**
 * Hide editor
 */
void StageEditor::hide()
{
	CollisionManager::hideColliders(GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(VUEngine::getStateMachine(VUEngine::getInstance())))));
	Printing::clear(Printing::getInstance());
	StageEditor::removePreviousSprite(this);
	StageEditor::releaseCollider(this);
	this->currentEntityNode = NULL;

	Tool::lightUpGame(this);
}

/**
 * Process  input
 *
 * @param pressedKey	User input
 */
void StageEditor::processUserInput(uint16 pressedKey)
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
	Printing::int32(Printing::getInstance(), this->mode, 17, 0, NULL);
	Printing::int32(Printing::getInstance(), kLastMode - 1, 19, 0, NULL);
}

/**
 * Print title
 *
 * @private
 */
void StageEditor::setupMode()
{
	Printing::clear(Printing::getInstance());
	StageEditor::printHeader(this);
	StageEditor::removePreviousSprite(this);

	switch(this->mode)
	{
		case kAddObjects:

			if(OptionsSelector::getNumberOfOptions(this->userObjectsSelector))
			{
				StageEditor::releaseCollider(this);
				StageEditor::printUserObjects(this);
				StageEditor::showSelectedUserObject(this);
				break;
			}

			this->mode = kMoveCamera;

		case kMoveCamera:

			StageEditor::releaseCollider(this);
			StageEditor::printCameraPosition(this);
			StageEditor::printTranslationStepSize(this, 38, 7);
			break;

		case kChangeProjection:

			StageEditor::releaseCollider(this);
			StageEditor::printProjectionValues(this);
			StageEditor::printTranslationStepSize(this, 38, 10);
			break;

		case kTranslateEntities:

			if(!this->currentEntityNode)
			{
				StageEditor::selectNextEntity(this);
			}
			else
			{
				StageEditor::getCollider(this);
				StageEditor::highLightEntity(this);
			}

			StageEditor::printEntityPosition(this);
			StageEditor::printTranslationStepSize(this, 38, 8);
			break;
	}
}

/**
 * Release collider
 *
 * @private
 */
void StageEditor::releaseCollider()
{
	if(this->currentEntityNode)
	{
		Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
		Entity::hideColliders(entity);

		if(!Entity::hasColliders(entity) && this->collider)
		{
			delete this->collider;
		}

		this->collider = NULL;
	}
}

/**
 * Get collider
 *
 * @private
 */
void StageEditor::getCollider()
{
	if(!this->currentEntityNode)
	{
		return;
	}

	Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
	Entity::showColliders(entity);

	if(!Entity::hasColliders(entity))
	{
		this->collider = Collider::safeCast(new Box(SpatialObject::safeCast(entity), NULL));

/*
		Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
		Size size = {Entity::getWidth(entity), Entity::getHeight(entity), 0};

		Collider::transform(this->collider, Entity::getPosition(entity), Entity::getRotation(entity), Entity::getScale(entity), &size);
*/
		Collider::setReady(this->collider, false);
	}
}

/**
 * Position collider
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::positionCollider()
{
	if(!this->currentEntityNode)
	{
		return;
	}

	Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
	Entity::showColliders(entity);
/*
	if(!Entity::hasColliders(entity) && this->collider)
	{
		Size size = {Entity::getWidth(entity), Entity::getHeight(entity), 0};
		Collider::transform(this->collider, Entity::getPosition(entity), Entity::getRotation(entity), Entity::getScale(entity), &size);
		Collider::show(this->collider);
	}
*/
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
		StageEditor::positionCollider(this);
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
	StageEditor::releaseCollider(this);

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
		StageEditor::getCollider(this);
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
	StageEditor::releaseCollider(this);

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
		StageEditor::getCollider(this);
		StageEditor::highLightEntity(this);
	}
}

/**
 * Move the camera
 *
 * @memberof 			StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the 
 */
void StageEditor::moveCamera(uint32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		Vector3D translation =
		{
			__PIXELS_TO_METERS(-this->translationStepSize),
			0,
			0
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_LR)
	{
		Vector3D translation =
		{
			__PIXELS_TO_METERS(this->translationStepSize),
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
			__PIXELS_TO_METERS(-this->translationStepSize),
			0
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_LD)
	{
		Vector3D translation =
		{
			0,
			__PIXELS_TO_METERS(this->translationStepSize),
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
			__PIXELS_TO_METERS(this->translationStepSize),
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_RD)
	{
		Vector3D translation =
		{
			0,
			0,
			__PIXELS_TO_METERS(-this->translationStepSize),
		};

		StageEditor::applyTranslationToCamera(this, translation);
	}
	else if(pressedKey & K_RR)
	{
		if(__MAX_TRANSLATION_STEP < ++this->translationStepSize)
		{
			this->translationStepSize = __MAX_TRANSLATION_STEP;
		}

		StageEditor::printTranslationStepSize(this, 38, 7);
	}
	else if(pressedKey & K_RL)
	{
		if(1 > --this->translationStepSize)
		{
			this->translationStepSize = 1;
		}

		StageEditor::printTranslationStepSize(this, 38, 7);
	}
}

/**
 * Modify projection values
 *
 * @memberof StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the 
 */
void StageEditor::changeProjection(uint32 pressedKey)
{
	Optical optical = *_optical;

	if(pressedKey & K_LL)
	{
		optical.horizontalViewPointCenter -= __PIXELS_TO_METERS(this->translationStepSize);
	}
	else if(pressedKey & K_LR)
	{
		optical.horizontalViewPointCenter += __PIXELS_TO_METERS(this->translationStepSize);
	}
	else if(pressedKey & K_LU)
	{
		optical.verticalViewPointCenter -= __PIXELS_TO_METERS(this->translationStepSize);
	}
	else if(pressedKey & K_LD)
	{
		optical.verticalViewPointCenter += __PIXELS_TO_METERS(this->translationStepSize);
	}
	else if(pressedKey & K_A)
	{
		optical.maximumXViewDistancePower += __MAXIMUM_VIEW_DISTANCE_STEP;
		optical.maximumYViewDistancePower += __MAXIMUM_VIEW_DISTANCE_STEP;
	}
	else if(pressedKey & K_B)
	{
		optical.maximumXViewDistancePower -= __MAXIMUM_VIEW_DISTANCE_STEP;
		optical.maximumYViewDistancePower -= __MAXIMUM_VIEW_DISTANCE_STEP;

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
		optical.baseDistance -= __PIXELS_TO_METERS(this->translationStepSize);
	}
	else if(pressedKey & K_RT)
	{
		optical.baseDistance += __PIXELS_TO_METERS(this->translationStepSize);
	}
	else if(pressedKey & K_RR)
	{
		if(__MAX_TRANSLATION_STEP < ++this->translationStepSize)
		{
			this->translationStepSize = __MAX_TRANSLATION_STEP;
		}

		StageEditor::printTranslationStepSize(this, 38, 10);
	}
	else if(pressedKey & K_RL)
	{
		if(1 > --this->translationStepSize)
		{
			this->translationStepSize = 1;
		}

		StageEditor::printTranslationStepSize(this, 38, 10);
	}

	Camera::setOptical(Camera::getInstance(), optical);

	StageEditor::printProjectionValues(this);
	GameState::transform(this->gameState);
}

/**
 * Translate an entity
 *
 * @memberof 			StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the 
 */
void StageEditor::translateEntity(uint32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		Vector3D translation =
		{
			__PIXELS_TO_METERS(-this->translationStepSize),
			0,
			0
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LR)
	{
		Vector3D translation =
		{
			__PIXELS_TO_METERS(this->translationStepSize),
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
			__PIXELS_TO_METERS(-this->translationStepSize),
			0
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_LD)
	{
		Vector3D translation =
		{
			0,
			__PIXELS_TO_METERS(this->translationStepSize),
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

		StageEditor::printTranslationStepSize(this, 38, 8);
	}
	else if(pressedKey & K_RL)
	{
		if(1 > --this->translationStepSize)
		{
			this->translationStepSize = 1;
		}

		StageEditor::printTranslationStepSize(this, 38, 8);
	}
	else if(pressedKey & K_RU)
	{
		Vector3D translation =
		{
			0,
			0,
			__PIXELS_TO_METERS(this->translationStepSize),
		};

		StageEditor::applyTranslationToEntity(this, translation);
	}
	else if(pressedKey & K_RD)
	{
		Vector3D translation =
		{
			0,
			0,
			__PIXELS_TO_METERS(-this->translationStepSize),
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
	if(this->currentEntityNode)
	{
		Container container = Container::safeCast(this->currentEntityNode->data);
		Vector3D localPosition = *Container::getLocalPosition(container);

		localPosition.x += translation.x;
		localPosition.y += translation.y;
		localPosition.z += translation.z;

		Container::setLocalPosition(container, &localPosition);
		Container::invalidateGlobalPosition(container);

		GameState::transform(this->gameState);

		StageEditor::positionCollider(this);

		StageEditor::printEntityPosition(this);

		SpriteManager::sort(SpriteManager::getInstance());

		StageEditor::printTranslationStepSize(this, 38, 8);
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

	SpriteManager::sort(SpriteManager::getInstance());
}

/**
 * Show selected  object
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::showSelectedUserObject()
{
	StageEditor::removePreviousSprite(this);

	SpriteSpec* spriteSpec = (SpriteSpec*)_userObjects[OptionsSelector::getSelectedOption(this->userObjectsSelector)].entitySpec->spriteSpecs[0];

	if(spriteSpec)
	{
		this->userObjectSprite = ((Sprite (*)(SpatialObject, SpriteSpec*)) spriteSpec->allocator)(NULL, (SpriteSpec*)spriteSpec);
		ASSERT(this->userObjectSprite, "AnimationInspector::createSprite: null animatedSprite");
		ASSERT(Sprite::getTexture(this->userObjectSprite), "AnimationInspector::createSprite: null texture");

		PixelVector spritePosition = Sprite::getDisplacedPosition(this->userObjectSprite);
		spritePosition.x = __I_TO_FIXED((__HALF_SCREEN_WIDTH) - (Texture::getCols(Sprite::getTexture(this->userObjectSprite)) << 2));
		spritePosition.y = __I_TO_FIXED((__HALF_SCREEN_HEIGHT) - (Texture::getRows(Sprite::getTexture(this->userObjectSprite)) << 2));

		Rotation spriteRotation = {0, 0, 0};
		PixelScale spriteScale = {1, 1};
		Sprite::setPosition(this->userObjectSprite, &spritePosition);
		Sprite::setRotation(this->userObjectSprite, &spriteRotation);
		Sprite::setScale(this->userObjectSprite, &spriteScale);
		Sprite::calculateParallax(this->userObjectSprite, spritePosition.z);

		this->userObjectSprite->writeAnimationFrame = true;
		SpriteManager::writeTextures(SpriteManager::getInstance());
		SpriteManager::sort(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
		SpriteManager::render(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
	}
}

/**
 * Select  object
 *
 * @memberof 			StageEditor
 * @private
 * @param pressedKey	The controller button pressed by the 
 */
void StageEditor::selectUserObject(uint32 pressedKey)
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

		PositionedEntity DUMMY_ENTITY =
		{
			(EntitySpec*)_userObjects[OptionsSelector::getSelectedOption(this->userObjectsSelector)].entitySpec,
			{__METERS_TO_PIXELS(cameraPosition.x) + __HALF_SCREEN_WIDTH, __METERS_TO_PIXELS(cameraPosition.y) + __HALF_SCREEN_HEIGHT, __METERS_TO_PIXELS(cameraPosition.z)},
			{0, 0, 0},
			{1, 1, 1},
			0,
			NULL,
			NULL,
			NULL,
			false
		};

		Stage::addChildEntity(GameState::getStage(this->gameState), &DUMMY_ENTITY, false);
		SpriteManager::sort(SpriteManager::getInstance());

		VirtualList stageEntities = (Container::safeCast(GameState::getStage(this->gameState)))->children;
		this->currentEntityNode = stageEntities ? stageEntities->tail : NULL;

		// select the added entity
		this->mode = kTranslateEntities;
		StageEditor::setupMode(this);

		StageEditor::removePreviousSprite(this);
		SpriteManager::sort(SpriteManager::getInstance());
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
	int32 x = 1;
	int32 y = 2;
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;

	Printing::text(Printing::getInstance(), "MOVE OBJECT", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text(Printing::getInstance(), "Next   \x17\x18", controlsXPos, controlsYPos++, NULL);
	Printing::text(Printing::getInstance(), "Move\x1E\x1A\x1B\x1C\x1D", controlsXPos, controlsYPos++, NULL);
	Printing::text(Printing::getInstance(), "      \x1F\x1A\x1B", controlsXPos, controlsYPos++, NULL);

	if(this->currentEntityNode)
	{
		Entity entity = Entity::safeCast(VirtualNode::getData(this->currentEntityNode));
		const Vector3D* globalPosition =  SpatialObject::getPosition(entity);
		const Rotation* globalRotation =  SpatialObject::getRotation(entity);
		const Scale* globalScale =  SpatialObject::getScale(entity);
		const char* entityName = Container::getName(entity);

		Printing::text(Printing::getInstance(),		"ID:                             ", 			x, 		++y, 	NULL);
		Printing::int32(Printing::getInstance(), 		Entity::getInternalId(entity), 					x + 10, y, 		NULL);
		Printing::text(Printing::getInstance(),		"Type:                           ", 			x, 		++y, 	NULL);
		Printing::text(Printing::getInstance(),		__GET_CLASS_NAME_UNSAFE(entity), 				x + 10, y, 		NULL);
		Printing::text(Printing::getInstance(),		"Name:                           ", 			x, 		++y, 	NULL);
		Printing::text(Printing::getInstance(),		entityName ? entityName : "-", 					x + 10, y, 		NULL);
		Printing::text(Printing::getInstance(),		"          X      Y      Z       ", 			x, 		++y, 	NULL);
		Printing::text(Printing::getInstance(),		"Position:                       ", 			x, 		++y, 	NULL);
		Printing::int32(Printing::getInstance(), 		__METERS_TO_PIXELS(globalPosition->x), 			x + 10, y, 		NULL);
		Printing::int32(Printing::getInstance(), 		__METERS_TO_PIXELS(globalPosition->y), 			x + 17, y, 		NULL);
		Printing::int32(Printing::getInstance(), 		__METERS_TO_PIXELS(globalPosition->z), 			x + 24, y, 		NULL);
		Printing::text(Printing::getInstance(),		"Rotation:                       ", 			x, 		++y, 	NULL);
		Printing::float(Printing::getInstance(), 		__FIXED_TO_F(globalRotation->x), 								x + 10, y, 		2, NULL);
		Printing::float(Printing::getInstance(), 		__FIXED_TO_F(globalRotation->y), 								x + 17, y, 		2, NULL);
		Printing::float(Printing::getInstance(), 		__FIXED_TO_F(globalRotation->z), 								x + 24, y, 		2, NULL);
		Printing::text(Printing::getInstance(),		"Scale:                          ", 			x, 		++y, 	NULL);
		Printing::float(Printing::getInstance(), 	__FIX7_9_TO_F(globalScale->x), 					x + 10, y, 		2, NULL);
		Printing::float(Printing::getInstance(), 	__FIX7_9_TO_F(globalScale->y), 					x + 17, y, 		2, NULL);
		Printing::float(Printing::getInstance(), 	__FIX7_9_TO_F(globalScale->z), 					x + 24, y, 		2, NULL);
		Printing::text(Printing::getInstance(),		"Size:                           ", 			x, 		++y, 	NULL);
		Printing::int32(Printing::getInstance(), 		__METERS_TO_PIXELS(Entity::getWidth(entity)), 	x + 10, y, 		NULL);
		Printing::int32(Printing::getInstance(), 		__METERS_TO_PIXELS(Entity::getHeight(entity)), 	x + 17, y, 		NULL);
		Printing::int32(Printing::getInstance(), 		__METERS_TO_PIXELS(Entity::getDepth(entity)), 	x + 24, y++, 	NULL);
		Printing::text(Printing::getInstance(),		"Children:                       ", 			x, 		++y, 	NULL);
		Printing::int32(Printing::getInstance(), 		Container::getChildCount(entity), 				x + 10, y, 		NULL);
		Printing::text(Printing::getInstance(),		"Sprites:                       ", 			x, 		++y, 	NULL);
		Printing::int32(Printing::getInstance(), 		Entity::getSprites(entity) ? VirtualList::getSize(Entity::getSprites(entity)) : 0, 				x + 10, y, 		NULL);
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
	Camera::translate(Camera::getInstance(), translation, true);
	GameState::transform(this->gameState);
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
	Camera::print(Camera::getInstance(), 1, 2, true);
}

/**
 * Print projection values
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printProjectionValues()
{
	int32 x = 1;
	int32 y = 2;
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;

	Printing::text(Printing::getInstance(), "Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text(Printing::getInstance(), "HVPC  \x1E\x1C\x1D", controlsXPos, controlsYPos++, NULL);
	Printing::text(Printing::getInstance(), "VVPC  \x1E\x1A\x1B", controlsXPos, controlsYPos++, NULL);
	Printing::text(Printing::getInstance(), "DETC  \x1F\x1A\x1B", controlsXPos, controlsYPos++, NULL);
	Printing::text(Printing::getInstance(), "MVD    \x13\x14", controlsXPos, controlsYPos++, NULL);
	Printing::text(Printing::getInstance(), "BD     \x17\x18", controlsXPos, controlsYPos++, NULL);

	Printing::text(Printing::getInstance(), "PROJECTION VALUES", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Horz. view point center:        ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(_optical->horizontalViewPointCenter), x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Vert. view point center:        ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(_optical->verticalViewPointCenter), x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Maximum X View Distance:        ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _optical->maximumXViewDistancePower, x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Maximum Y View Distance:        ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _optical->maximumYViewDistancePower, x + 25, y, NULL);
	Printing::text(Printing::getInstance(), "Base Distance:                  ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(_optical->baseDistance), x + 25, y, NULL);
}

/**
 * Print  objects
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printUserObjects()
{
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;

	Printing::text(Printing::getInstance(), "ADD OBJECTS", 1, 2, NULL);
	Printing::text(Printing::getInstance(), "                       ", 1, 3, NULL);

	Printing::text(Printing::getInstance(), "Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text(Printing::getInstance(), "Accept  \x13", controlsXPos, controlsYPos++, NULL);

	OptionsSelector::printOptions(this->userObjectsSelector, 1, 4, kOptionsAlignLeft, 0);
}

/**
 * Print translation step size
 *
 * @memberof 	StageEditor
 * @private
 */
void StageEditor::printTranslationStepSize(uint8 x, uint8 y)
{
	Printing::text(Printing::getInstance(), "Step  \x1F\x1C\x1D", x, y, NULL);
	Printing::text(Printing::getInstance(), "+     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->translationStepSize, x + 1, y, NULL);
}

#endif
