/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <BgmapTextureManager.h>
#include <Box.h>
#include <Camera.h>
#include <ColliderManager.h>
#include <Debug.h>
#include <Entity.h>
#include <GameState.h>
#include <KeypadManager.h>
#include <Mesh.h>
#include <Optics.h>
#include <OptionsSelector.h>
#include <BodyManager.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <Collider.h>
#include <Stage.h>
#include <StateMachine.h>
#include <VirtualList.h>
#include <VUEngine.h>

#include "StageEditor.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;
friend class Sprite;
friend class Container;

extern Transformation _neutralEnvironmentTransformation;
extern UserObject _userObjects[];


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __DEFAULT_TRANSLATION_STEP			8
#define __MAX_TRANSLATION_STEP				__PIXELS_TO_METERS(32)
#define __MAXIMUM_VIEW_DISTANCE_STEP		1


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// @memberof StageEditor
enum StageEditorStates
{
	kFirstState = 0,
	kMoveCamera,
	kChangeProjection,
	kTranslateEntities,
	kAddObjects,

	kLastState
};


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void StageEditor::update()
{}
//---------------------------------------------------------------------------------------------------------
void StageEditor::show()
{
	this->state = kFirstState + 1;
	this->userEntitySprite = NULL;

	StageEditor::releaseWireframe(this);
	StageEditor::configureState(this);

	StageEditor::dimmGame(this);
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::hide()
{
	ColliderManager::hideColliders(GameState::getColliderManager(GameState::safeCast(VUEngine::getPreviousState(VUEngine::getInstance()))));
	Printing::clear(Printing::getInstance());
	StageEditor::removePreviousSprite(this);
	StageEditor::releaseWireframe(this);
	this->entityNode = NULL;

	Tool::lightUpGame(this);
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::processUserInput(uint16 pressedKey)
{
	if(isDeleted(this->stage))
	{
		return;
	}

	if(pressedKey & K_SEL)
	{
		this->state++;

		if(kLastState <= this->state)
		{
			this->state = kFirstState + 1;
		}

		StageEditor::configureState(this);
		return;
	}

	switch(this->state)
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
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void StageEditor::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->entityNode = NULL;
	this->userEntitySprite = NULL;
	this->state = kFirstState + 1;
	this->wireframe = NULL;
	this->userEntitySelector = new OptionsSelector(2, 12, NULL, NULL, NULL);

	VirtualList userObjects = new VirtualList();

	int32 i = 0;
	for(;  _userObjects[i].entitySpec; i++)
	{
		Option* option = new Option;
		option->value = _userObjects[i].name;
		option->type = kString;
		VirtualList::pushBack(userObjects, option);
	}

	if(VirtualList::getCount(userObjects))
	{
		OptionsSelector::setOptions(this->userEntitySelector, userObjects);
	}

	delete userObjects;

	this->translationStepSize = __DEFAULT_TRANSLATION_STEP;
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::destructor()
{
	if(this->userEntitySelector)
	{
		delete this->userEntitySelector;
	}

	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::printHeader()
{
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing::text(Printing::getInstance(), " LEVEL EDITOR ", 1, 0, NULL);
	Printing::text(Printing::getInstance(), "  /  ", 16, 0, NULL);
	Printing::int32(Printing::getInstance(), this->state, 17, 0, NULL);
	Printing::int32(Printing::getInstance(), kLastState - 1, 19, 0, NULL);
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::configureState()
{
	Printing::clear(Printing::getInstance());
	StageEditor::printHeader(this);
	StageEditor::removePreviousSprite(this);

	switch(this->state)
	{
		case kAddObjects:

			if(OptionsSelector::getNumberOfOptions(this->userEntitySelector))
			{
				StageEditor::releaseWireframe(this);
				StageEditor::printUserObjects(this);
				StageEditor::showSelectedUserObject(this);
				break;
			}

			this->state = kMoveCamera;

		case kMoveCamera:

			StageEditor::releaseWireframe(this);
			StageEditor::printCameraPosition(this);
			StageEditor::printTranslationStepSize(this, 38, 7);
			break;

		case kChangeProjection:

			StageEditor::releaseWireframe(this);
			StageEditor::printProjectionValues(this);
			StageEditor::printTranslationStepSize(this, 38, 10);
			break;

		case kTranslateEntities:

			if(!this->entityNode)
			{
				StageEditor::selectNextEntity(this);
			}
			else
			{
				StageEditor::highLightEntity(this);
			}

			StageEditor::printEntityPosition(this);
			StageEditor::printTranslationStepSize(this, 38, 8);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::releaseWireframe()
{
	if(!isDeleted(this->wireframe))
	{
		WireframeManager::destroyWireframe(WireframeManager::getInstance(), this->wireframe);

		this->wireframe = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::highLightEntity()
{
	if(NULL != this->entityNode)
	{
		StageEditor::printEntityPosition(this);

		Entity entity = Entity::safeCast(VirtualNode::getData(this->entityNode));

		if(!isDeleted(entity))
		{
			int16 width = __METERS_TO_PIXELS(Entity::getWidth(entity)) << 2;
			int16 height = __METERS_TO_PIXELS(Entity::getHeight(entity)) << 2;
			fixed_t parallax = Optics::calculateParallax(Entity::getPosition(entity)->z);

			const PixelVector MeshesSegments[][2]=
			{
				{
					PixelVector::getFromVector3D((Vector3D){-width / 2, -height / 2, 0}, parallax),
					PixelVector::getFromVector3D((Vector3D){width / 2, -height / 2, 0}, parallax),
				},
				{
					PixelVector::getFromVector3D((Vector3D){-width / 2, height / 2, 0}, parallax),
					PixelVector::getFromVector3D((Vector3D){width / 2, height / 2, 0}, parallax),
				},
				{
					PixelVector::getFromVector3D((Vector3D){-width / 2, -height / 2, 0}, parallax),
					PixelVector::getFromVector3D((Vector3D){-width / 2, height / 2, 0}, parallax),
				},
				{
					PixelVector::getFromVector3D((Vector3D){width / 2, -height / 2, 0}, parallax),
					PixelVector::getFromVector3D((Vector3D){width / 2, height / 2, 0}, parallax),
				},

				// limiter
				{
					{0, 0, 0, 0}, 
					{0, 0, 0, 0}
				},
			};

			MeshSpec meshSpec =
			{
				{
					__TYPE(Mesh),

					/// Displacement
					{0, 0, 0},

					/// color
					__COLOR_BRIGHT_RED,

					/// Transparency mode (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
					__TRANSPARENCY_NONE,
				
					/// interlaced
					false
				},

				// segments
				(PixelVector(*)[2])MeshesSegments
			};


			this->wireframe = WireframeManager::createWireframe(WireframeManager::getInstance(), (WireframeSpec*)&meshSpec, GameObject::safeCast(entity));
		}
	}
	else
	{
		Printing::text(Printing::getInstance(), "No entities in stage", 1, 4, NULL);
	}
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::selectPreviousEntity()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	StageEditor::releaseWireframe(this);

	VirtualList stageEntities = (Container::safeCast(this->stage))->children;

	if(!this->entityNode)
	{
		this->entityNode = stageEntities ? stageEntities->tail : NULL;
	}
	else
	{
		this->entityNode = VirtualNode::getPrevious(this->entityNode);

		if(!this->entityNode)
		{
			this->entityNode = stageEntities ? stageEntities->tail : NULL;
		}
	}

	if(this->entityNode)
	{
		StageEditor::highLightEntity(this);
	}
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::selectNextEntity()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	StageEditor::releaseWireframe(this);

	VirtualList stageEntities = (Container::safeCast(this->stage))->children;

	if(!this->entityNode)
	{
		this->entityNode = stageEntities ? stageEntities->head : NULL;
	}
	else
	{
		this->entityNode = this->entityNode->next;

		if(!this->entityNode)
		{
			this->entityNode = stageEntities ? stageEntities->head : NULL;
		}
	}

	if(this->entityNode)
	{
		StageEditor::highLightEntity(this);
	}
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void StageEditor::changeProjection(uint32 pressedKey)
{
	if(isDeleted(this->stage))
	{
		return;
	}

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

	Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(Camera::getInstance()));
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void StageEditor::applyTranslationToEntity(Vector3D translation)
{
	if(isDeleted(this->stage))
	{
		return;
	}

	if(this->entityNode)
	{
		Container container = Container::safeCast(this->entityNode->data);
		Vector3D localPosition = *Container::getLocalPosition(container);

		localPosition.x += translation.x;
		localPosition.y += translation.y;
		localPosition.z += translation.z;

		Container::setLocalPosition(container, &localPosition);

		Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(Camera::getInstance()));

		StageEditor::printEntityPosition(this);

		SpriteManager::sortSprites(SpriteManager::getInstance());

		StageEditor::printTranslationStepSize(this, 38, 8);
	}
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::removePreviousSprite()
{
	if(this->userEntitySprite)
	{
		delete this->userEntitySprite;
		this->userEntitySprite = NULL;
	}

	SpriteManager::sortSprites(SpriteManager::getInstance());
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::showSelectedUserObject()
{
	StageEditor::removePreviousSprite(this);

	SpriteSpec* spriteSpec = (SpriteSpec*)_userObjects[OptionsSelector::getSelectedOption(this->userEntitySelector)].entitySpec->componentSpecs[0];

	if(spriteSpec)
	{
		this->userEntitySprite = ((Sprite (*)(GameObject, SpriteSpec*)) spriteSpec->allocator)(NULL, (SpriteSpec*)spriteSpec);
		ASSERT(this->userEntitySprite, "AnimationInspector::createSprite: null animatedSprite");
		ASSERT(Sprite::getTexture(this->userEntitySprite), "AnimationInspector::createSprite: null texture");

		PixelVector spritePosition = Sprite::getDisplacedPosition(this->userEntitySprite);
		spritePosition.x = __I_TO_FIXED((__HALF_SCREEN_WIDTH) - (Texture::getCols(Sprite::getTexture(this->userEntitySprite)) << 2));
		spritePosition.y = __I_TO_FIXED((__HALF_SCREEN_HEIGHT) - (Texture::getRows(Sprite::getTexture(this->userEntitySprite)) << 2));
		spritePosition.parallax = Optics::calculateParallax(spritePosition.z);

		Rotation spriteRotation = {0, 0, 0};
		PixelScale spriteScale = {1, 1};
		Sprite::setPosition(this->userEntitySprite, &spritePosition);
		Sprite::setRotation(this->userEntitySprite, &spriteRotation);
		Sprite::setScale(this->userEntitySprite, &spriteScale);

		this->userEntitySprite->updateAnimationFrame = true;
		SpriteManager::writeTextures(SpriteManager::getInstance());
		SpriteManager::sortSprites(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
		SpriteManager::render(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
	}
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::selectUserObject(uint32 pressedKey)
{
	if(isDeleted(this->stage))
	{
		return;
	}

	if(pressedKey & K_LU)
	{
		OptionsSelector::selectPrevious(this->userEntitySelector);
		StageEditor::showSelectedUserObject(this);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector::selectNext(this->userEntitySelector);
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
			(EntitySpec*)_userObjects[OptionsSelector::getSelectedOption(this->userEntitySelector)].entitySpec,
			{__METERS_TO_PIXELS(cameraPosition.x) + __HALF_SCREEN_WIDTH, __METERS_TO_PIXELS(cameraPosition.y) + __HALF_SCREEN_HEIGHT, __METERS_TO_PIXELS(cameraPosition.z)},
			{0, 0, 0},
			{1, 1, 1},
			0,
			NULL,
			NULL,
			NULL,
			false
		};

		Stage::spawnChildEntity(this->stage, &DUMMY_ENTITY, false);
		SpriteManager::sortSprites(SpriteManager::getInstance());

		VirtualList stageEntities = (Container::safeCast(this->stage))->children;
		this->entityNode = stageEntities ? stageEntities->tail : NULL;

		// select the added entity
		this->state = kTranslateEntities;
		StageEditor::configureState(this);

		StageEditor::removePreviousSprite(this);
		SpriteManager::sortSprites(SpriteManager::getInstance());
		SpriteManager::writeTextures(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
		SpriteManager::render(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
	}
}
//---------------------------------------------------------------------------------------------------------
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

	if(this->entityNode)
	{
		Entity entity = Entity::safeCast(VirtualNode::getData(this->entityNode));
		const Vector3D* globalPosition =  GameObject::getPosition(entity);
		const Rotation* globalRotation =  GameObject::getRotation(entity);
		const Scale* globalScale =  GameObject::getScale(entity);
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
		Printing::int32(Printing::getInstance(), 		Container::getChildrenCount(entity), 				x + 10, y, 		NULL);
	}
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::applyTranslationToCamera(Vector3D translation)
{
	if(isDeleted(this->stage))
	{
		return;
	}

	Camera::translate(Camera::getInstance(), translation, true);
	Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(Camera::getInstance()));
	StageEditor::printCameraPosition(this);
	Stage::streamAll(this->stage);
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::printCameraPosition()
{
	Camera::print(Camera::getInstance(), 1, 2, true);
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void StageEditor::printUserObjects()
{
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;

	Printing::text(Printing::getInstance(), "ADD OBJECTS", 1, 2, NULL);
	Printing::text(Printing::getInstance(), "                       ", 1, 3, NULL);

	Printing::text(Printing::getInstance(), "Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text(Printing::getInstance(), "Accept  \x13", controlsXPos, controlsYPos++, NULL);

	OptionsSelector::print(this->userEntitySelector, 1, 4, kOptionsAlignLeft, 0);
}
//---------------------------------------------------------------------------------------------------------
void StageEditor::printTranslationStepSize(uint8 x, uint8 y)
{
	Printing::text(Printing::getInstance(), "Step  \x1F\x1C\x1D", x, y, NULL);
	Printing::text(Printing::getInstance(), "+     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->translationStepSize, x + 1, y, NULL);
}
//---------------------------------------------------------------------------------------------------------

#endif
