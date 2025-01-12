/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapTextureManager.h>
#include <Box.h>
#include <Camera.h>
#include <ColliderManager.h>
#include <Debug.h>
#include <Actor.h>
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
#include <WireframeManager.h>

#include "StageEditor.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;
friend class Sprite;
friend class Container;

extern Transformation _neutralEnvironmentTransformation;
extern UserObject _userObjects[];

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __DEFAULT_TRANSLATION_STEP			8
#define __MAX_TRANSLATION_STEP				__PIXELS_TO_METERS(32)
#define __MAXIMUM_VIEW_DISTANCE_STEP		1

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// @memberof StageEditor
enum StageEditorStates
{
	kFirstState = 0,
	kMoveCamera,
	kChangeProjection,
	kTranslateActors,
	kAddObjects,

	kLastState
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::update()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::show()
{
	this->state = kFirstState + 1;
	this->userActorSprite = NULL;

	StageEditor::releaseWireframe(this);
	StageEditor::configureState(this);

	StageEditor::dimmGame(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::hide()
{
	ColliderManager::hideColliders
	(
		GameState::getColliderManager(GameState::safeCast(VUEngine::getPreviousState()))
	);

	Printing::clear();
	StageEditor::removePreviousSprite(this);
	StageEditor::releaseWireframe(this);
	this->actorNode = NULL;

	Tool::lightUpGame(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

		case kTranslateActors:

			StageEditor::translateActor(this, pressedKey);
			break;

		case kAddObjects:

			StageEditor::selectUserObject(this, pressedKey);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->actorNode = NULL;
	this->userActorSprite = NULL;
	this->state = kFirstState + 1;
	this->wireframe = NULL;
	this->userActorSelector = new OptionsSelector(2, 12, NULL, NULL, NULL);

	VirtualList userObjects = new VirtualList();

	int32 i = 0;
	for(;  _userObjects[i].actorSpec; i++)
	{
		Option* option = new Option;
		option->value = _userObjects[i].name;
		option->type = kString;
		VirtualList::pushBack(userObjects, option);
	}

	if(VirtualList::getCount(userObjects))
	{
		OptionsSelector::setOptions(this->userActorSelector, userObjects);
	}

	delete userObjects;

	this->translationStepSize = __DEFAULT_TRANSLATION_STEP;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::destructor()
{
	if(this->userActorSelector)
	{
		delete this->userActorSelector;
	}

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::printHeader()
{
	Printing::text
	(
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL
	);

	Printing::text(" LEVEL EDITOR ", 1, 0, NULL);
	Printing::text("  /  ", 16, 0, NULL);
	Printing::int32(this->state, 17, 0, NULL);
	Printing::int32(kLastState - 1, 19, 0, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::configureState()
{
	Printing::clear();
	StageEditor::printHeader(this);
	StageEditor::removePreviousSprite(this);

	switch(this->state)
	{
		case kAddObjects:

			if(OptionsSelector::getNumberOfOptions(this->userActorSelector))
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

		case kTranslateActors:

			if(!this->actorNode)
			{
				StageEditor::selectNextActor(this);
			}
			else
			{
				StageEditor::highLightActor(this);
			}

			StageEditor::printActorPosition(this);
			StageEditor::printTranslationStepSize(this, 38, 8);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::releaseWireframe()
{
	if(!isDeleted(this->wireframe))
	{
		WireframeManager::destroyWireframe(this->wireframe);

		this->wireframe = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::highLightActor()
{
	if(NULL != this->actorNode)
	{
		StageEditor::printActorPosition(this);

		Actor actor = Actor::safeCast(VirtualNode::getData(this->actorNode));

		if(!isDeleted(actor))
		{
			int16 width = __METERS_TO_PIXELS(Actor::getWidth(actor)) << 2;
			int16 height = __METERS_TO_PIXELS(Actor::getHeight(actor)) << 2;
			fixed_t parallax = Optics::calculateParallax(Actor::getPosition(actor)->z);

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

				// Limiter
				{
					{0, 0, 0, 0}, 
					{0, 0, 0, 0}
				},
			};

			MeshSpec meshSpec =
			{
				{
					{
						// Allocator
						__TYPE(Mesh),

						// Component type
						kWireframeComponent
					},

					/// Displacement
					{0, 0, 0},

					/// color
					__COLOR_BRIGHT_RED,

					/// Transparency mode (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
					__TRANSPARENCY_NONE,
				
					/// Flag to render the wireframe in interlaced mode
					false
				},

				// Segments
				(PixelVector(*)[2])MeshesSegments
			};

			this->wireframe = 
				WireframeManager::createWireframe
				(
					Entity::safeCast(actor), (WireframeSpec*)&meshSpec
				);
		}
	}
	else
	{
		Printing::text("No actors in stage", 1, 4, NULL);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::selectPreviousActor()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	StageEditor::releaseWireframe(this);

	VirtualList stageActors = (Container::safeCast(this->stage))->children;

	if(!this->actorNode)
	{
		this->actorNode = stageActors ? stageActors->tail : NULL;
	}
	else
	{
		this->actorNode = VirtualNode::getPrevious(this->actorNode);

		if(!this->actorNode)
		{
			this->actorNode = stageActors ? stageActors->tail : NULL;
		}
	}

	if(this->actorNode)
	{
		StageEditor::highLightActor(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::selectNextActor()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	StageEditor::releaseWireframe(this);

	VirtualList stageActors = (Container::safeCast(this->stage))->children;

	if(!this->actorNode)
	{
		this->actorNode = stageActors ? stageActors->head : NULL;
	}
	else
	{
		this->actorNode = this->actorNode->next;

		if(!this->actorNode)
		{
			this->actorNode = stageActors ? stageActors->head : NULL;
		}
	}

	if(this->actorNode)
	{
		StageEditor::highLightActor(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

	Camera::setOptical(optical);

	StageEditor::printProjectionValues(this);

	Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::translateActor(uint32 pressedKey)
{
	if(pressedKey & K_LL)
	{
		Vector3D translation =
		{
			__PIXELS_TO_METERS(-this->translationStepSize),
			0,
			0
		};

		StageEditor::applyTranslationToActor(this, translation);
	}
	else if(pressedKey & K_LR)
	{
		Vector3D translation =
		{
			__PIXELS_TO_METERS(this->translationStepSize),
			0,
			0
		};

		StageEditor::applyTranslationToActor(this, translation);
	}
	else if(pressedKey & K_LU)
	{
		Vector3D translation =
		{
			0,
			__PIXELS_TO_METERS(-this->translationStepSize),
			0
		};

		StageEditor::applyTranslationToActor(this, translation);
	}
	else if(pressedKey & K_LD)
	{
		Vector3D translation =
		{
			0,
			__PIXELS_TO_METERS(this->translationStepSize),
			0
		};

		StageEditor::applyTranslationToActor(this, translation);
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

		StageEditor::applyTranslationToActor(this, translation);
	}
	else if(pressedKey & K_RD)
	{
		Vector3D translation =
		{
			0,
			0,
			__PIXELS_TO_METERS(-this->translationStepSize),
		};

		StageEditor::applyTranslationToActor(this, translation);
	}
	else if(pressedKey & K_LT)
	{
		StageEditor::selectPreviousActor(this);
	}
	else if(pressedKey & K_RT)
	{
		StageEditor::selectNextActor(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::applyTranslationToActor(Vector3D translation)
{
	if(isDeleted(this->stage))
	{
		return;
	}

	if(this->actorNode)
	{
		Container container = Container::safeCast(this->actorNode->data);
		Vector3D localPosition = *Container::getLocalPosition(container);

		localPosition.x += translation.x;
		localPosition.y += translation.y;
		localPosition.z += translation.z;

		Container::setLocalPosition(container, &localPosition);

		Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags());

		StageEditor::printActorPosition(this);

		SpriteManager::sortSprites(SpriteManager::getInstance());

		StageEditor::printTranslationStepSize(this, 38, 8);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::removePreviousSprite()
{
	if(this->userActorSprite)
	{
		ComponentManager::destroyComponent(NULL, Component::safeCast(this->userActorSprite));
		this->userActorSprite = NULL;
	}

	SpriteManager::sortSprites(SpriteManager::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::showSelectedUserObject()
{
	StageEditor::removePreviousSprite(this);

	SpriteSpec* spriteSpec = 
		(SpriteSpec*)_userObjects[OptionsSelector::getSelectedOption(this->userActorSelector)].actorSpec->componentSpecs[0];

	if(spriteSpec)
	{
		this->userActorSprite = ComponentManager::createComponent(NULL, (ComponentSpec*)spriteSpec);
		ASSERT(this->userActorSprite, "AnimationInspector::createSprite: null animatedSprite");
		ASSERT(Sprite::getTexture(this->userActorSprite), "AnimationInspector::createSprite: null texture");

		PixelVector spritePosition = Sprite::getDisplacedPosition(this->userActorSprite);
		spritePosition.x = __I_TO_FIXED((__HALF_SCREEN_WIDTH) - (Texture::getCols(Sprite::getTexture(this->userActorSprite)) << 2));
		spritePosition.y = __I_TO_FIXED((__HALF_SCREEN_HEIGHT) - (Texture::getRows(Sprite::getTexture(this->userActorSprite)) << 2));
		spritePosition.parallax = Optics::calculateParallax(spritePosition.z);

		Rotation spriteRotation = {0, 0, 0};
		PixelScale spriteScale = {1, 1};
		Sprite::setPosition(this->userActorSprite, &spritePosition);
		Sprite::setRotation(this->userActorSprite, &spriteRotation);
		Sprite::setScale(this->userActorSprite, &spriteScale);

		this->userActorSprite->updateAnimationFrame = true;
		SpriteManager::writeTextures(SpriteManager::getInstance());
		SpriteManager::sortSprites(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
		SpriteManager::render(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::selectUserObject(uint32 pressedKey)
{
	if(isDeleted(this->stage))
	{
		return;
	}

	if(pressedKey & K_LU)
	{
		OptionsSelector::selectPrevious(this->userActorSelector);
		StageEditor::showSelectedUserObject(this);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector::selectNext(this->userActorSelector);
		StageEditor::showSelectedUserObject(this);
	}
	else if(pressedKey & K_A)
	{
		if(1 >= SpriteManager::getFreeLayer(SpriteManager::getInstance()))
		{
			Printing::text("No more WORLDs", 48 - 15, 5, NULL);
			Printing::text("available     ", 48 - 15, 6, NULL);
			return;
		}

		Vector3D cameraPosition = Camera::getPosition();

		PositionedActor DUMMY_ENTITY =
		{
			(ActorSpec*)_userObjects[OptionsSelector::getSelectedOption(this->userActorSelector)].actorSpec,
			{
				__METERS_TO_PIXELS(cameraPosition.x) + __HALF_SCREEN_WIDTH, 
				__METERS_TO_PIXELS(cameraPosition.y) + __HALF_SCREEN_HEIGHT, 
				__METERS_TO_PIXELS(cameraPosition.z)
			},
			{0, 0, 0},
			{1, 1, 1},
			0,
			NULL,
			NULL,
			NULL,
			false
		};

		Stage::spawnChildActor(this->stage, &DUMMY_ENTITY, false);
		SpriteManager::sortSprites(SpriteManager::getInstance());

		VirtualList stageActors = (Container::safeCast(this->stage))->children;
		this->actorNode = stageActors ? stageActors->tail : NULL;

		// Select the added actor
		this->state = kTranslateActors;
		StageEditor::configureState(this);

		StageEditor::removePreviousSprite(this);
		SpriteManager::sortSprites(SpriteManager::getInstance());
		SpriteManager::writeTextures(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
		SpriteManager::render(SpriteManager::getInstance());
		SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::printActorPosition()
{
	int32 x = 1;
	int32 y = 2;
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;

	Printing::text("MOVE OBJECT", x, y++, NULL);

	Printing::text("Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text("Next   \x17\x18", controlsXPos, controlsYPos++, NULL);
	Printing::text("Move\x1E\x1A\x1B\x1C\x1D", controlsXPos, controlsYPos++, NULL);
	Printing::text("      \x1F\x1A\x1B", controlsXPos, controlsYPos++, NULL);

	if(this->actorNode)
	{
		Actor actor = Actor::safeCast(VirtualNode::getData(this->actorNode));
		const Vector3D* globalPosition =  Entity::getPosition(actor);
		const Rotation* globalRotation =  Entity::getRotation(actor);
		const Scale* globalScale =  Entity::getScale(actor);
		const char* actorName = Container::getName(actor);

		Printing::text("ID:                             ", x, ++y, NULL);
		Printing::int32(Actor::getInternalId(actor), x + 10, y, NULL);
		Printing::text("Type:                           ", x, ++y, NULL);
		Printing::text(	__GET_CLASS_NAME(actor), x + 10, y, NULL);
		Printing::text("Name:                           ", x, ++y, NULL);
		Printing::text(	actorName ? actorName : "-", x + 10, y, NULL);
		Printing::text("          X      Y      Z       ", x, ++y, NULL);
		Printing::text("Position:                       ", x, ++y, NULL);
		Printing::int32(__METERS_TO_PIXELS(globalPosition->x), x + 10, y, NULL);
		Printing::int32(__METERS_TO_PIXELS(globalPosition->y), x + 17, y, NULL);
		Printing::int32(__METERS_TO_PIXELS(globalPosition->z), x + 24, y, NULL);
		Printing::text("Rotation:                       ", x, ++y, NULL);
		Printing::float(__FIXED_TO_F(globalRotation->x), x + 10, y, 2, NULL);
		Printing::float(__FIXED_TO_F(globalRotation->y), x + 17, y, 2, NULL);
		Printing::float(__FIXED_TO_F(globalRotation->z), x + 24, y, 2, NULL);
		Printing::text("Scale:                          ", x, ++y, NULL);
		Printing::float(__FIX7_9_TO_F(globalScale->x), x + 10, y, 2, NULL);
		Printing::float(__FIX7_9_TO_F(globalScale->y), x + 17, y, 2, NULL);
		Printing::float(__FIX7_9_TO_F(globalScale->z), x + 24, y, 2, NULL);
		Printing::text("Size:                           ", x, ++y, NULL);
		Printing::int32(__METERS_TO_PIXELS(Actor::getWidth(actor)), x + 10, y, NULL);
		Printing::int32(__METERS_TO_PIXELS(Actor::getHeight(actor)), x + 17, y, NULL);
		Printing::int32(__METERS_TO_PIXELS(Actor::getDepth(actor)), x + 24, y++, NULL);
		Printing::text("Children:                       ", x, ++y, NULL);
		Printing::int32(Container::getChildrenCount(actor), x + 10, y, NULL);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::applyTranslationToCamera(Vector3D translation)
{
	if(isDeleted(this->stage))
	{
		return;
	}

	Camera::translate(translation, true);
	Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags());
	StageEditor::printCameraPosition(this);
	Stage::streamAll(this->stage);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::printCameraPosition()
{
	Camera::print(1, 2, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::printProjectionValues()
{
	int32 x = 1;
	int32 y = 2;
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;

	Printing::text("Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text("HVPC  \x1E\x1C\x1D", controlsXPos, controlsYPos++, NULL);
	Printing::text("VVPC  \x1E\x1A\x1B", controlsXPos, controlsYPos++, NULL);
	Printing::text("DETC  \x1F\x1A\x1B", controlsXPos, controlsYPos++, NULL);
	Printing::text("MVD    \x13\x14", controlsXPos, controlsYPos++, NULL);
	Printing::text("BD     \x17\x18", controlsXPos, controlsYPos++, NULL);

	Printing::text("PROJECTION VALUES", x, y++, NULL);
	Printing::text("Horz. view point center:        ", x, ++y, NULL);
	Printing::int32(__METERS_TO_PIXELS(_optical->horizontalViewPointCenter), x + 25, y, NULL);
	Printing::text("Vert. view point center:        ", x, ++y, NULL);
	Printing::int32(__METERS_TO_PIXELS(_optical->verticalViewPointCenter), x + 25, y, NULL);
	Printing::text("Maximum X View Distance:        ", x, ++y, NULL);
	Printing::int32(_optical->maximumXViewDistancePower, x + 25, y, NULL);
	Printing::text("Maximum Y View Distance:        ", x, ++y, NULL);
	Printing::int32(_optical->maximumYViewDistancePower, x + 25, y, NULL);
	Printing::text("Base Distance:                  ", x, ++y, NULL);
	Printing::int32(__METERS_TO_PIXELS(_optical->baseDistance), x + 25, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::printUserObjects()
{
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;

	Printing::text("ADD OBJECTS", 1, 2, NULL);
	Printing::text("                       ", 1, 3, NULL);

	Printing::text("Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text("Accept  \x13", controlsXPos, controlsYPos++, NULL);

	OptionsSelector::print(this->userActorSelector, 1, 4, kOptionsAlignLeft, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StageEditor::printTranslationStepSize(uint8 x, uint8 y)
{
	Printing::text("Step  \x1F\x1C\x1D", x, y, NULL);
	Printing::text("+     ", x, ++y, NULL);
	Printing::int32(this->translationStepSize, x + 1, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
