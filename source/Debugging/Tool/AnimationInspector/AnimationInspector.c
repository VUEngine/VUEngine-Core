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

#include <string.h>

#include <AnimationController.h>
#include <BgmapTextureManager.h>
#include <Camera.h>
#include <GameState.h>
#include <KeypadManager.h>
#include <Optics.h>
#include <OptionsSelector.h>
#include <Printer.h>
#include <Singleton.h>
#include <Stage.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "AnimationInspector.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Sprite;

extern UserActor _userAnimatedActors[];

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// @memberof AnimationInspector
enum AnimationInspectorStates
{
	kFirstState = 0,
	kSelectActor,
	kSelectSprite,
	kSelectAnimation,
	kEditAnimation,
	kLastState
};

/// @memberof AnimationInspector
enum AnimationProperties
{
	kNumberOfFrames = 0,
	kDelay,
	kLoop,
	kFrames
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::update()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::show()
{
	Printer::clear();

	if(NULL == _userAnimatedActors[0].actorSpec)
	{
		Printer::text
		(
			
			"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
			"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL
		);

		Printer::text(" ANIMATION INSPECTOR ", 1, 0, NULL);
		Printer::text("             ", 39, 2, NULL);
		Printer::text("             ", 39, 3, NULL);
		Printer::text("No animations found", 1, 4, NULL);
		Printer::text("Define some in _userAnimatedActors global variable", 1, 6, NULL);
		return;
	}

	this->sprite = NULL;

	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->actorSelector = new OptionsSelector(2, 16, NULL, NULL, NULL);

	VirtualList animatedActorsNames = new VirtualList();

	int32 i = 0;
	for(; _userAnimatedActors[i].actorSpec; i++)
	{
		ASSERT(_userAnimatedActors[i].name, "AnimationInspector::start: push null name");

		Option* option = new Option;
		option->value = (char*)(_userAnimatedActors[i].name);
		option->type = kString;
		VirtualList::pushBack(animatedActorsNames, option);
	}

	ASSERT(animatedActorsNames, "AnimationInspector::start: null animatedActorsNames");
	ASSERT(VirtualList::getCount(animatedActorsNames), "AnimationInspector::start: empty animatedActorsNames");

	OptionsSelector::setOptions(this->actorSelector, animatedActorsNames);
	delete animatedActorsNames;

	this->state = kFirstState + 1;
	AnimationInspector::configureState(this);
	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(ToolState::getCurrentGameState(this->toolState), kSpriteComponent));

	SpriteManager::hideAllSprites(spriteManager, NULL, false);
	
	// Make sure all textures are written right now
	SpriteManager::writeTextures(spriteManager);
	SpriteManager::deferParamTableEffects(spriteManager, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::hide()
{
	if(NULL == _userAnimatedActors[0].actorSpec)
	{
		Printer::text("No animations found", 1, 4, NULL);
		Printer::text("Define some in _userAnimatedActors global variable", 1, 6, NULL);
		return;
	}

	Printer::clear();

	AnimationInspector::removePreviousSprite(this);

	if(this->actorSelector)
	{
		delete this->actorSelector;
		this->actorSelector = NULL;
	}

	if(this->animationsSelector)
	{
		delete this->animationsSelector;
		this->animationsSelector = NULL;
	}

	if(this->animationEditionSelector)
	{
		delete this->animationEditionSelector;
		this->animationEditionSelector = NULL;
	}

	if(this->frameEditionSelector)
	{
		delete this->frameEditionSelector;
		this->frameEditionSelector = NULL;
	}

	// Make sure all textures are written right now
	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(ToolState::getCurrentGameState(this->toolState), kSpriteComponent));

	SpriteManager::writeTextures(spriteManager);
	//SpriteManager::showAllSprites(spriteManager, NULL, true);
	SpriteManager::deferParamTableEffects(spriteManager, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::processUserInput(uint16 pressedKey)
{
	if(NULL == _userAnimatedActors[0].actorSpec)
	{
		return;
	}

	if(pressedKey & K_B)
	{
		this->state--;

		if(kFirstState >= this->state)
		{
			this->state = kFirstState + 1;
		}
		else
		{
			AnimationInspector::configureState(this);
		}

		return;
	}

	switch(this->state)
	{
		case kSelectActor:

			AnimationInspector::selectActor(this, pressedKey);
			break;

		case kSelectSprite:

			AnimationInspector::selectSprite(this, pressedKey);
			break;

		case kSelectAnimation:

			AnimationInspector::selectAnimation(this, pressedKey);
			break;

		case kEditAnimation:

			AnimationInspector::editAnimation(this, pressedKey);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->sprite = NULL;
	this->actorSelector = NULL;
	this->spriteSelector = NULL;
	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->state = kFirstState + 1;

	int32 i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION; i++)
	{
		this->animationFunction.frames[i] = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::destructor()
{
	if(this->actorSelector)
	{
		delete this->actorSelector;
	}

	if(this->actorSelector)
	{
		delete this->spriteSelector;
	}

	if(this->animationsSelector)
	{
		delete this->animationsSelector;
	}

	if(this->animationEditionSelector)
	{
		delete this->animationEditionSelector;
	}

	if(this->frameEditionSelector)
	{
		delete this->frameEditionSelector;
	}

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::configureState()
{
	Printer::clear();
	Printer::text
	(
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL
	);

	Printer::text(" ANIMATION INSPECTOR ", 1, 0, NULL);
	Printer::text("             ", 39, 2, NULL);
	Printer::text("             ", 39, 3, NULL);

	switch(this->state)
	{
		case kSelectActor:

			AnimationInspector::removePreviousSprite(this);
			Printer::text("Select \x13  ", 39, 2, NULL);
			AnimationInspector::printUserAnimatedActors(this);
			break;

		case kSelectSprite:

			Printer::text("Select \x13  ", 39, 2, NULL);
			Printer::text("Back   \x14  ", 39, 3, NULL);
			AnimationInspector::createSpriteSelector(this);
			AnimationInspector::printSprites(this);
			AnimationInspector::createSprite(this);
			Sprite::pause(this->sprite, true);
			break;

		case kSelectAnimation:

			Printer::text("Select \x13  ", 39, 2, NULL);
			Printer::text("Back   \x14  ", 39, 3, NULL);
			AnimationInspector::createAnimationsSelector(this);
			Sprite::pause(this->sprite, true);
			AnimationInspector::printActorAnimations(this);
			break;

		case kEditAnimation:

			AnimationInspector::loadAnimationFunction(this);
			AnimationInspector::createAnimationEditionSelector(this);
			AnimationInspector::createFrameEditionSelector(this);
			AnimationController::playAnimationFunction(Sprite::getAnimationController(this->sprite), &this->animationFunction, NULL);
			Sprite::updateAnimation(this->sprite);
			Sprite::pause(this->sprite, true);
			Sprite::pause(this->sprite, false);
			AnimationInspector::printAnimationConfig(this);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::selectActor(uint32 pressedKey)
{
	int32 userAnimatedActorsCount = 0;

	for(; NULL != _userAnimatedActors[userAnimatedActorsCount].actorSpec; userAnimatedActorsCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector::selectPrevious(this->actorSelector);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector::selectNext(this->actorSelector);
	}
	else if(pressedKey & K_A)
	{
		// Select the added actor
		this->state = kSelectSprite;
		AnimationInspector::configureState(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::selectSprite(uint32 pressedKey)
{
	int32 userAnimatedActorsCount = 0;

	for(; NULL != _userAnimatedActors[userAnimatedActorsCount].actorSpec; userAnimatedActorsCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector::selectPrevious(this->spriteSelector);
		AnimationInspector::createSprite(this);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector::selectNext(this->spriteSelector);
		AnimationInspector::createSprite(this);
	}
	else if(pressedKey & K_A)
	{
		// Select the added actor
		this->state = kSelectAnimation;
		AnimationInspector::configureState(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::removePreviousSprite()
{
	if(!isDeleted(this->sprite))
	{
		ComponentManager::destroyComponent(NULL, Component::safeCast(this->sprite));
		this->sprite = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::selectAnimation(uint32 pressedKey)
{
	this->animationFunctions = 
		_userAnimatedActors[OptionsSelector::getSelectedOption(this->actorSelector)].actorSpec->animationFunctions;

	int32 animationsCount = 0;
	for(; this->animationFunctions[animationsCount]; animationsCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector::selectPrevious(this->animationsSelector);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector::selectNext(this->animationsSelector);
	}
	else if(pressedKey & K_A)
	{
		this->state = kEditAnimation;
		AnimationInspector::configureState(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::editAnimation(uint32 pressedKey)
{
	if(pressedKey & K_A)
	{
		if(Sprite::isPlaying(this->sprite))
		{
			Sprite::pause(this->sprite, true);
		}
		else
		{
			AnimationController::playAnimationFunction(Sprite::getAnimationController(this->sprite), &this->animationFunction, NULL);
		}
	}
	else if((pressedKey & K_LU))
	{
		OptionsSelector::selectPrevious(this->animationEditionSelector);
	}
	else if((pressedKey & K_LD))
	{
		OptionsSelector::selectNext(this->animationEditionSelector);
	}
	else if((pressedKey & K_LL))
	{
		int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kNumberOfFrames:

				if(0 == --this->animationFunction.numberOfFrames)
				{
					this->animationFunction.numberOfFrames = 1;
				}

				AnimationInspector::createFrameEditionSelector(this);
				break;

			case kDelay:

				this->animationFunction.delay -= 1;

				if(0 > (int8)this->animationFunction.delay)
				{
					this->animationFunction.delay = 0;
				}
				break;

			case kLoop:

				this->animationFunction.loop = !this->animationFunction.loop;
				break;

			case kFrames:

				break;
		}
	}
	else if(pressedKey & K_LR)
	{
		int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kNumberOfFrames:

				if(__MAX_FRAMES_PER_ANIMATION_FUNCTION < ++this->animationFunction.numberOfFrames)
				{
					this->animationFunction.numberOfFrames = __MAX_FRAMES_PER_ANIMATION_FUNCTION;
				}

				AnimationInspector::createFrameEditionSelector(this);

				break;

			case kDelay:

				this->animationFunction.delay += 1;
				break;

			case kLoop:

				this->animationFunction.loop = !this->animationFunction.loop;
				break;

			case kFrames:

				break;
		}
	}
	else if(pressedKey & K_RU)
	{
		int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kFrames:

				OptionsSelector::selectPrevious(this->frameEditionSelector);
				break;
		}
	}
	else if(pressedKey & K_RD)
	{
		int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kFrames:

				OptionsSelector::selectNext(this->frameEditionSelector);
				break;
		}
	}
	else if(pressedKey & K_RL)
	{
		int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		int32 selectedFrame = OptionsSelector::getSelectedOption(this->frameEditionSelector);

		switch(selectedProperty)
		{
			case kFrames:

				if(0 < this->animationFunction.frames[selectedFrame])
				{
					this->animationFunction.frames[selectedFrame]--;
				}

				break;
		}
	}
	else if(pressedKey & K_RR)
	{
		int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		int32 selectedFrame = OptionsSelector::getSelectedOption(this->frameEditionSelector);

		switch(selectedProperty)
		{
			case kFrames:
				{
					NM_ASSERT(this->sprite, "AnimationInspector::selectAnimation: null sprite");

					Texture texture = Sprite::getTexture(this->sprite);
					NM_ASSERT(texture, "AnimationInspector::selectAnimation: null texture");

					const TextureSpec* textureSpec = Texture::getSpec(texture);
					NM_ASSERT(textureSpec, "AnimationInspector::selectAnimation: null textureSpec");

					if
					(
						++this->animationFunction.frames[selectedFrame] >= textureSpec->numberOfFrames 
						&& 
						1 < textureSpec->numberOfFrames
					)
					{
						this->animationFunction.frames[selectedFrame] = textureSpec->numberOfFrames - 1;
					}
				}
				break;
		}
	}

	AnimationInspector::printAnimationConfig(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::printUserAnimatedActors()
{
	Printer::text("OBJECTS", 1, 2, NULL);
	Printer::text("                       ", 1, 3, NULL);
	OptionsSelector::print(this->actorSelector, 1, 4, kOptionsAlignLeft, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::printSprites()
{
	Printer::text("SPRITES", 1, 2, NULL);
	Printer::text("                       ", 1, 3, NULL);
	OptionsSelector::print(this->spriteSelector, 1, 4, kOptionsAlignLeft, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::printActorAnimations()
{
	Printer::text("AVAILABLE ANIMATIONS", 1, 2, NULL);
	Printer::text("                       ", 1, 3, NULL);
	OptionsSelector::print(this->animationsSelector, 1, 4, kOptionsAlignLeft, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::printAnimationConfig()
{
	int32 x = 1;
	int32 y = 2;

	Printer::text("Animation: ", x, y, NULL);
	Printer::text(this->animationFunction.name, x + 11, y++, NULL);
	OptionsSelector::print(this->animationEditionSelector, x, ++y, kOptionsAlignLeft, 0);

	Printer::int32(this->animationFunction.numberOfFrames, x + 19, y++, NULL);
	Printer::int32(this->animationFunction.delay, x + 19, y++, NULL);
	Printer::text(this->animationFunction.loop ? "true" : "false", x + 19, y++, NULL);

	OptionsSelector::print(this->frameEditionSelector, x, ++y + 1, kOptionsAlignLeft, 0);

	Printer::text("Back     \x14 ", 37, 2, NULL);
	if(!Sprite::isPlaying(this->sprite))
	{
		Printer::text("Play     \x13 ", 37, 3, NULL);
	}
	else
	{
		Printer::text("Pause    \x13 ", 37, 3, NULL);
	}
	Printer::text("Select \x1E\x1A\x1B", 37, 4, NULL);
	Printer::text("Modify \x1E\x1C\x1D", 37, 5, NULL);

	int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);

	switch(selectedProperty)
	{
		case kFrames:

			Printer::text("Select \x1F\x1A\x1B", 37, 7, NULL);
			Printer::text("Modify \x1F\x1C\x1D", 37, 8, NULL);
			break;

		default:

			Printer::text("                   ", 37, 7, NULL);
			Printer::text("                   ", 37, 8, NULL);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::loadAnimationFunction()
{
	if(NULL == _userAnimatedActors[0].actorSpec)
	{
		return;
	}

	this->animationFunctions = 
		_userAnimatedActors[OptionsSelector::getSelectedOption(this->actorSelector)].actorSpec->animationFunctions;

	const AnimationFunction* animationFunction = 
		this->animationFunctions[OptionsSelector::getSelectedOption(this->animationsSelector)];

	int32 i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION; i++)
	{
		this->animationFunction.frames[i] = animationFunction->frames[i];
	}

	strcpy(this->animationFunction.name, animationFunction->name);
	this->animationFunction.numberOfFrames = animationFunction->numberOfFrames;
	this->animationFunction.delay = animationFunction->delay;
	this->animationFunction.loop = animationFunction->loop;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::createSprite()
{
	if(NULL == _userAnimatedActors[0].actorSpec)
	{
		return;
	}

	AnimationInspector::removePreviousSprite(this);

	Vector3D position = *_cameraPosition;

	position.x += __I_TO_FIXED(__HALF_SCREEN_WIDTH);
	position.y += __I_TO_FIXED(__HALF_SCREEN_HEIGHT);
	position.z -= 10;

	SpriteSpec* spriteSpec = 
		(SpriteSpec*)_userAnimatedActors[OptionsSelector::getSelectedOption(this->actorSelector)].
			actorSpec->componentSpecs[OptionsSelector::getSelectedOption(this->spriteSelector)];

	NM_ASSERT(spriteSpec, "AnimationInspector::createSprite: null spriteSpec");

	this->sprite = Sprite::safeCast(ComponentManager::createComponent(NULL, (ComponentSpec*)spriteSpec));
	ASSERT(this->sprite, "AnimationInspector::createSprite: null sprite");
	ASSERT(Sprite::getTexture(this->sprite), "AnimationInspector::createSprite: null texture");

	PixelVector spritePosition = {__SCREEN_WIDTH / 2, __SCREEN_HEIGHT / 2, 1, 2};
	Rotation spriteRotation = {0, 0, 0};
	PixelScale spriteScale = {__F_TO_FIX7_9(1.0f), __F_TO_FIX7_9(1.0f)};

	Sprite::setPosition(this->sprite, &spritePosition);
	Sprite::setRotation(this->sprite, &spriteRotation);
	Sprite::setScale(this->sprite, &spriteScale);
	Sprite::processEffects(this->sprite, -1);

	this->sprite->updateAnimationFrame = true;

	SpriteManager spriteManager = 
		SpriteManager::safeCast(ToolState::getComponentManager(ToolState::getCurrentGameState(this->toolState), kSpriteComponent));

	//SpriteManager::hideAllSprites(spriteManager, this->sprite, false);
	SpriteManager::prepareAll(spriteManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::createSpriteSelector()
{
	if(!isDeleted(this->spriteSelector))
	{
		delete this->spriteSelector;
	}

	this->spriteSelector = 
		new OptionsSelector((__SCREEN_WIDTH_IN_CHARS) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION >> 1, NULL, NULL, NULL);

	VirtualList spriteIndexes = new VirtualList();

	int32 i = 0;
	
	while
	(
		NULL 
		!= 
		_userAnimatedActors[OptionsSelector::getSelectedOption(this->actorSelector)].
			actorSpec->componentSpecs[i]
	)
	{
		Option* option = new Option;
		option->value = NULL;
		option->type = kInt;
		VirtualList::pushBack(spriteIndexes, option);

		i++;
	}

	OptionsSelector::setOptions(this->spriteSelector, spriteIndexes);
	delete spriteIndexes;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::createAnimationsSelector()
{
	if(NULL == _userAnimatedActors[0].actorSpec)
	{
		return;
	}

	this->animationFunctions = 
		_userAnimatedActors[OptionsSelector::getSelectedOption(this->actorSelector)].actorSpec->animationFunctions;

	if(this->animationFunctions)
	{
		if(this->animationsSelector)
		{
			delete this->animationsSelector;
		}

		this->animationsSelector = new OptionsSelector(2, 16, NULL, NULL, NULL);

		VirtualList animationsNames = new VirtualList();

		int32 i = 0;
		for(i = 0; this->animationFunctions[i]; i++)
		{
			Option* option = new Option;
			option->value = (void*)this->animationFunctions[i]->name;
			option->type = kString;
			VirtualList::pushBack(animationsNames, option);
		}

		OptionsSelector::setOptions(this->animationsSelector, animationsNames);
		delete animationsNames;
	}
	else
	{
		//TODO
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::createAnimationEditionSelector()
{
	if(this->animationEditionSelector)
	{
		delete this->animationEditionSelector;
	}

	this->animationEditionSelector = new OptionsSelector(1, 4, NULL, NULL, NULL);

	VirtualList optionsNames = new VirtualList();
	Option* option = NULL;

	option = new Option;
	option->value = "Number of frames:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	option = new Option;
	option->value = "Cycle delay:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	option = new Option;
	option->value = "Loop:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	option = new Option;
	option->value = "Frames:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	OptionsSelector::setOptions(this->animationEditionSelector, optionsNames);
	delete optionsNames;

	this->state = kEditAnimation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspector::createFrameEditionSelector()
{
	if(this->frameEditionSelector)
	{
		delete this->frameEditionSelector;
	}

	this->frameEditionSelector = new OptionsSelector(4, 16, NULL, NULL, NULL);
	OptionsSelector::setColumnWidth(this->frameEditionSelector, 2);

	VirtualList framesIndexes = new VirtualList();

	int32 i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION && i < this->animationFunction.numberOfFrames; i++)
	{
		Option* option = new Option;
		option->value = &this->animationFunction.frames[i];
		option->type = kChar;
		VirtualList::pushBack(framesIndexes, option);
	}

	OptionsSelector::setOptions(this->frameEditionSelector, framesIndexes);
	delete framesIndexes;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool AnimationInspector::onAnimationComplete(ListenerObject eventFirer __attribute__ ((unused)))
{
	if(!this->animationFunction.loop)
	{
		Printer::text("Play     \x13 ", 37, 3, NULL);
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
