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

#include <string.h>

#include <AnimationController.h>
#include <BgmapTextureManager.h>
#include <Camera.h>
#include <GameState.h>
#include <KeypadManager.h>
#include <Optics.h>
#include <OptionsSelector.h>
#include <Printing.h>
#include <Stage.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "AnimationInspector.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Sprite;

extern UserAnimatedEntity _userAnimatedEntities[];


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

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


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void AnimationInspector::update()
{}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::show()
{
	Printing::clear(Printing::getInstance());

	if(NULL == _userAnimatedEntities[0].animatedEntitySpec)
	{
		Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
		Printing::text(Printing::getInstance(), " ANIMATION INSPECTOR ", 1, 0, NULL);
		Printing::text(Printing::getInstance(), "             ", 39, 2, NULL);
		Printing::text(Printing::getInstance(), "             ", 39, 3, NULL);
		Printing::text(Printing::getInstance(), "No animations found", 1, 4, NULL);
		Printing::text(Printing::getInstance(), "Define some in _userAnimatedEntities global variable", 1, 6, NULL);
		return;
	}

	this->sprite = NULL;

	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->animatedEntitySelector = new OptionsSelector(2, 16, NULL, NULL, NULL);

	VirtualList animatedEntitiesNames = new VirtualList();

	int32 i = 0;
	for(; _userAnimatedEntities[i].animatedEntitySpec; i++)
	{
		ASSERT(_userAnimatedEntities[i].name, "AnimationInspector::start: push null name");

		Option* option = new Option;
		option->value = (char*)(_userAnimatedEntities[i].name);
		option->type = kString;
		VirtualList::pushBack(animatedEntitiesNames, option);
	}

	ASSERT(animatedEntitiesNames, "AnimationInspector::start: null animatedEntitiesNames");
	ASSERT(VirtualList::getCount(animatedEntitiesNames), "AnimationInspector::start: empty animatedEntitiesNames");

	OptionsSelector::setOptions(this->animatedEntitySelector, animatedEntitiesNames);
	delete animatedEntitiesNames;

	this->state = kFirstState + 1;
	AnimationInspector::configureState(this);
	SpriteManager::hideAllSprites(SpriteManager::getInstance(), NULL, false);
	
	// make sure all textures are written right now
	SpriteManager::writeTextures(SpriteManager::getInstance());
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::hide()
{
	if(NULL == _userAnimatedEntities[0].animatedEntitySpec)
	{
		Printing::text(Printing::getInstance(), "No animations found", 1, 4, NULL);
		Printing::text(Printing::getInstance(), "Define some in _userAnimatedEntities global variable", 1, 6, NULL);
		return;
	}

	Printing::clear(Printing::getInstance());

	AnimationInspector::removePreviousSprite(this);

	if(this->animatedEntitySelector)
	{
		delete this->animatedEntitySelector;
		this->animatedEntitySelector = NULL;
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

	// make sure all textures are written right now
	SpriteManager::writeTextures(SpriteManager::getInstance());
	SpriteManager::showAllSprites(SpriteManager::getInstance(), NULL, true);
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::processUserInput(uint16 pressedKey)
{
	if(NULL == _userAnimatedEntities[0].animatedEntitySpec)
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

			AnimationInspector::selectAnimatedEntity(this, pressedKey);
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
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void AnimationInspector::constructor()
{
	Base::constructor();

	this->sprite = NULL;
	this->animatedEntitySelector = NULL;
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
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::destructor()
{
	if(this->animatedEntitySelector)
	{
		delete this->animatedEntitySelector;
	}

	if(this->animatedEntitySelector)
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

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::configureState()
{
	Printing printing = Printing::getInstance();

	Printing::clear(Printing::getInstance());
	Printing::text(printing, "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing::text(printing, " ANIMATION INSPECTOR ", 1, 0, NULL);
	Printing::text(printing, "             ", 39, 2, NULL);
	Printing::text(printing, "             ", 39, 3, NULL);

	switch(this->state)
	{
		case kSelectActor:

			AnimationInspector::removePreviousSprite(this);
			Printing::text(printing, "Select \x13  ", 39, 2, NULL);
			AnimationInspector::printUserAnimatedEntities(this);
			break;

		case kSelectSprite:

			Printing::text(printing, "Select \x13  ", 39, 2, NULL);
			Printing::text(printing, "Back   \x14  ", 39, 3, NULL);
			AnimationInspector::createSpriteSelector(this);
			AnimationInspector::printSprites(this);
			AnimationInspector::createSprite(this);
			Sprite::pause(this->sprite, true);
			break;

		case kSelectAnimation:

			Printing::text(printing, "Select \x13  ", 39, 2, NULL);
			Printing::text(printing, "Back   \x14  ", 39, 3, NULL);
			AnimationInspector::createAnimationsSelector(this);
			Sprite::pause(this->sprite, true);
			AnimationInspector::printAnimatedEntityAnimations(this);
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
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::selectAnimatedEntity(uint32 pressedKey)
{
	int32 userAnimatedEntitiesCount = 0;

	for(; NULL != _userAnimatedEntities[userAnimatedEntitiesCount].animatedEntitySpec; userAnimatedEntitiesCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector::selectPrevious(this->animatedEntitySelector);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector::selectNext(this->animatedEntitySelector);
	}
	else if(pressedKey & K_A)
	{
		// select the added entity
		this->state = kSelectSprite;
		AnimationInspector::configureState(this);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::selectSprite(uint32 pressedKey)
{
	int32 userAnimatedEntitiesCount = 0;

	for(; NULL != _userAnimatedEntities[userAnimatedEntitiesCount].animatedEntitySpec; userAnimatedEntitiesCount++);

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
		// select the added entity
		this->state = kSelectAnimation;
		AnimationInspector::configureState(this);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::removePreviousSprite()
{
	if(!isDeleted(this->sprite))
	{
		SpriteManager::destroySprite(SpriteManager::getInstance(), this->sprite);
		this->sprite = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::selectAnimation(uint32 pressedKey)
{
	this->animationFunctions = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->animationFunctions;

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
//---------------------------------------------------------------------------------------------------------
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

					TextureSpec* textureSpec = Texture::getSpec(texture);
					NM_ASSERT(textureSpec, "AnimationInspector::selectAnimation: null textureSpec");

					if(++this->animationFunction.frames[selectedFrame] >= textureSpec->numberOfFrames && 1 < textureSpec->numberOfFrames)
					{
						this->animationFunction.frames[selectedFrame] = textureSpec->numberOfFrames - 1;
					}
				}
				break;
		}
	}

	AnimationInspector::printAnimationConfig(this);
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::printUserAnimatedEntities()
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "OBJECTS", 1, 2, NULL);
	Printing::text(printing, "                       ", 1, 3, NULL);
	OptionsSelector::print(this->animatedEntitySelector, 1, 4, kOptionsAlignLeft, 0);
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::printSprites()
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "SPRITES", 1, 2, NULL);
	Printing::text(printing, "                       ", 1, 3, NULL);
	OptionsSelector::print(this->spriteSelector, 1, 4, kOptionsAlignLeft, 0);
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::printAnimatedEntityAnimations()
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "AVAILABLE ANIMATIONS", 1, 2, NULL);
	Printing::text(printing, "                       ", 1, 3, NULL);
	OptionsSelector::print(this->animationsSelector, 1, 4, kOptionsAlignLeft, 0);
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::printAnimationConfig()
{
	int32 x = 1;
	int32 y = 2;
	Printing printing = Printing::getInstance();

	Printing::text(printing, "Animation: ", x, y, NULL);
	Printing::text(printing, this->animationFunction.name, x + 11, y++, NULL);
	OptionsSelector::print(this->animationEditionSelector, x, ++y, kOptionsAlignLeft, 0);

	Printing::int32(printing, this->animationFunction.numberOfFrames, x + 19, y++, NULL);
	Printing::int32(printing, this->animationFunction.delay, x + 19, y++, NULL);
	Printing::text(printing, this->animationFunction.loop ? "true" : "false", x + 19, y++, NULL);

	OptionsSelector::print(this->frameEditionSelector, x, ++y + 1, kOptionsAlignLeft, 0);

	Printing::text(printing, "Back     \x14 ", 37, 2, NULL);
	if(!Sprite::isPlaying(this->sprite))
	{
		Printing::text(printing, "Play     \x13 ", 37, 3, NULL);
	}
	else
	{
		Printing::text(printing, "Pause    \x13 ", 37, 3, NULL);
	}
	Printing::text(printing, "Select \x1E\x1A\x1B", 37, 4, NULL);
	Printing::text(printing, "Modify \x1E\x1C\x1D", 37, 5, NULL);

	int32 selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);

	switch(selectedProperty)
	{
		case kFrames:

			Printing::text(printing, "Select \x1F\x1A\x1B", 37, 7, NULL);
			Printing::text(printing, "Modify \x1F\x1C\x1D", 37, 8, NULL);
			break;

		default:

			Printing::text(printing, "                   ", 37, 7, NULL);
			Printing::text(printing, "                   ", 37, 8, NULL);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::loadAnimationFunction()
{
	if(NULL == _userAnimatedEntities[0].animatedEntitySpec)
	{
		return;
	}

	this->animationFunctions = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->animationFunctions;

	const AnimationFunction* animationFunction = this->animationFunctions[OptionsSelector::getSelectedOption(this->animationsSelector)];

	int32 i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION; i++)
	{
		this->animationFunction.frames[i] = animationFunction->frames[i];
	}

	strcpy(this->animationFunction.name, animationFunction->name);
	this->animationFunction.numberOfFrames = animationFunction->numberOfFrames;
	this->animationFunction.delay = animationFunction->delay;
	this->animationFunction.loop = animationFunction->loop;
	this->animationFunction.onAnimationComplete = (EventListener)&AnimationInspector_onAnimationComplete;
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::createSprite()
{
	if(NULL == _userAnimatedEntities[0].animatedEntitySpec)
	{
		return;
	}

	AnimationInspector::removePreviousSprite(this);

	Vector3D position = *_cameraPosition;

	position.x += __I_TO_FIXED(__HALF_SCREEN_WIDTH);
	position.y += __I_TO_FIXED(__HALF_SCREEN_HEIGHT);
	position.z -= 10;

	SpriteSpec* spriteSpec = (SpriteSpec*)_userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->entitySpec.componentSpecs[OptionsSelector::getSelectedOption(this->spriteSelector)];

	NM_ASSERT(spriteSpec, "AnimationInspector::createSprite: null spriteSpec");

	this->sprite = Sprite::safeCast(SpriteManager::createSprite(SpriteManager::getInstance(), NULL, (SpriteSpec*)spriteSpec));
	ASSERT(this->sprite, "AnimationInspector::createSprite: null sprite");
	ASSERT(Sprite::getTexture(this->sprite), "AnimationInspector::createSprite: null texture");

	PixelVector spritePosition = {__SCREEN_WIDTH / 2, __SCREEN_HEIGHT / 2, 1, 2};
	Rotation spriteRotation = {0, 0, 0};
	PixelScale spriteScale = {__F_TO_FIX7_9(1.0f), __F_TO_FIX7_9(1.0f)};

	Sprite::setPosition(this->sprite, &spritePosition);
	Sprite::setRotation(this->sprite, &spriteRotation);
	Sprite::setScale(this->sprite, &spriteScale);
	Sprite::processEffects(this->sprite);

	this->sprite->updateAnimationFrame = true;

	SpriteManager::hideAllSprites(SpriteManager::getInstance(), this->sprite, false);
	SpriteManager::prepareAll(SpriteManager::getInstance());
}
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::createSpriteSelector()
{
	if(!isDeleted(this->spriteSelector))
	{
		delete this->spriteSelector;
	}

	this->spriteSelector = new OptionsSelector((__SCREEN_WIDTH_IN_CHARS) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION >> 1, NULL, NULL, NULL);

	VirtualList spriteIndexes = new VirtualList();

	int32 i = 0;
	
	while(NULL != _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->entitySpec.componentSpecs[i])
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
//---------------------------------------------------------------------------------------------------------
void AnimationInspector::createAnimationsSelector()
{
	if(NULL == _userAnimatedEntities[0].animatedEntitySpec)
	{
		return;
	}

	this->animationFunctions = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->animationFunctions;

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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
bool AnimationInspector::onAnimationComplete(ListenerObject eventFirer __attribute__ ((unused)))
{
	if(!this->animationFunction.loop)
	{
		Printing::text(Printing::getInstance(), "Play     \x13 ", 37, 3, NULL);
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------

#endif
