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

#include <AnimationInspector.h>
#include <Game.h>
#include <Optics.h>
#include <SpriteManager.h>
#include <GameState.h>
#include <Stage.h>
#include <Camera.h>
#include <string.h>
#include <OptionsSelector.h>
#include <Texture.h>
#include <AnimationController.h>
#include <BgmapTextureManager.h>
#include <KeyPadManager.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __USER_ACTOR_SHOW_ROW 			6

#define __TRANSLATION_STEP				8
#define __SCREEN_X_TRANSLATION_STEP		__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP		__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP		__SCREEN_HEIGHT / 4


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	AnimationInspector
 * @extends Object
 * @ingroup tools
 */

friend class Sprite;

/**
 * The different modes of the AnimationInspector
 *
 * @memberof	AnimationInspector
 */
enum Modes
{
	kFirstMode = 0,
	kSelectActor,
	kSelectSprite,
	kSelectAnimation,
	kEditAnimation,
	kLastMode
};

/**
 * Properties of a Animation
 *
 * @memberof	AnimationInspector
 */
enum AnimationProperties
{
	kNumberOfFrames = 0,
	kDelay,
	kLoop,
	kFrames
};


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern UserAnimatedEntity _userAnimatedEntities[];

AnimationController Sprite::getAnimationController(Sprite this);

void AnimationInspector::constructor(AnimationInspector this);
static void AnimationInspector::setupMode(AnimationInspector this);
static void AnimationInspector::printUserAnimatedEntities(AnimationInspector this);
static void AnimationInspector::printSprites(AnimationInspector this);
static void AnimationInspector::printAnimatedEntityAnimations(AnimationInspector this);
static void AnimationInspector::printAnimationConfig(AnimationInspector this);
static void AnimationInspector::selectAnimatedEntity(AnimationInspector this, u32 pressedKey);
static void AnimationInspector::selectSprite(AnimationInspector this, u32 pressedKey);
static void AnimationInspector::removePreviousSprite(AnimationInspector this);
static void AnimationInspector::selectAnimation(AnimationInspector this, u32 pressedKey);
static void AnimationInspector::editAnimation(AnimationInspector this, u32 pressedKey);
static void AnimationInspector::loadAnimationFunction(AnimationInspector this);
static void AnimationInspector::createSprite(AnimationInspector this);
static void AnimationInspector::createSpriteSelector(AnimationInspector this);
static void AnimationInspector::createAnimationsSelector(AnimationInspector this);
static void AnimationInspector::createAnimationEditionSelector(AnimationInspector this);
static void AnimationInspector::createFrameEditionSelector(AnimationInspector this);
static void AnimationInspector::onAnimationComplete(AnimationInspector this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationInspector::getInstance()
 * @memberof	AnimationInspector
 * @public
 *
 * @return		AnimationInspector instance
 */


/**
 * Class constructor
 *
 * @memberof	AnimationInspector
 * @private
 *
 * @param this	Function scope
 */
void __attribute__ ((noinline)) AnimationInspector::constructor(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::constructor: null this");

	Base::constructor();

	this->animatedSprite = NULL;
	this->gameState = NULL;
	this->animatedEntitySelector = NULL;
	this->spriteSelector = NULL;
	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->mode = kFirstMode + 1;
}

/**
 * Class destructor
 *
 * @memberof	AnimationInspector
 * @public
 *
 * @param this	Function scope
 */
void AnimationInspector::destructor(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::destructor: null this");

	if(this->animatedEntitySelector)
	{
		__DELETE(this->animatedEntitySelector);
	}

	if(this->animatedEntitySelector)
	{
		__DELETE(this->spriteSelector);
	}

	if(this->animationsSelector)
	{
		__DELETE(this->animationsSelector);
	}

	if(this->animationEditionSelector)
	{
		__DELETE(this->animationEditionSelector);
	}

	if(this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
	}

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Update
 *
 * @memberof	AnimationInspector
 * @public
 *
 * @param this	Function scope
 */
void AnimationInspector::update(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::update: null this");

	if(this->gameState && this->animatedSprite)
	{
		Sprite::updateAnimation(this->animatedSprite);
		Sprite::update(this->animatedSprite);
		 Sprite::applyAffineTransformations(this->animatedSprite);
		 Sprite::applyHbiasEffects(this->animatedSprite);
	}
}

/**
 * Show editor
 *
 * @memberof		AnimationInspector
 * @public
 *
 * @param this		Function scope
 * @param gameState Current game state
 */
void AnimationInspector::show(AnimationInspector this, GameState gameState)
{
	ASSERT(this, "AnimationInspector::start: null this");
	ASSERT(gameState, "AnimationInspector::start: null gameState");

	this->gameState = gameState;
	this->animatedSprite = NULL;

	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->animatedEntitySelector = __NEW(OptionsSelector, 2, 16, NULL);

	VirtualList animatedEntitiesNames = __NEW(VirtualList);

	int i = 0;
	for(; _userAnimatedEntities[i].animatedEntityDefinition; i++)
	{
		ASSERT(_userAnimatedEntities[i].name, "AnimationInspector::start: push null name");

		Option* option = __NEW_BASIC(Option);
		option->value = (char*)(_userAnimatedEntities[i].name);
		option->type = kString;
		VirtualList::pushBack(animatedEntitiesNames, option);
	}

	ASSERT(animatedEntitiesNames, "AnimationInspector::start: null animatedEntitiesNames");
	ASSERT(VirtualList::getSize(animatedEntitiesNames), "AnimationInspector::start: empty animatedEntitiesNames");

	OptionsSelector::setOptions(this->animatedEntitySelector, animatedEntitiesNames);
	__DELETE(animatedEntitiesNames);

	this->mode = kFirstMode + 1;
	AnimationInspector::setupMode(this);
	SpriteManager::showLayer(SpriteManager::getInstance(), SpriteManager::getFreeLayer(SpriteManager::getInstance()));

	// make sure all textures are written right now
	SpriteManager::writeTextures(SpriteManager::getInstance());
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
}

/**
 * Hide editor
 *
 * @memberof		AnimationInspector
 * @public
 *
 * @param this		Function scope
 */
void AnimationInspector::hide(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::stop: null this");

	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);

	AnimationInspector::removePreviousSprite(this);

	if(this->animatedEntitySelector)
	{
		__DELETE(this->animatedEntitySelector);
		this->animatedEntitySelector = NULL;
	}

	if(this->animationsSelector)
	{
		__DELETE(this->animationsSelector);
		this->animationsSelector = NULL;
	}

	if(this->animationEditionSelector)
	{
		__DELETE(this->animationEditionSelector);
		this->animationEditionSelector = NULL;
	}

	if(this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
		this->frameEditionSelector = NULL;
	}

	// make sure all textures are written right now
	SpriteManager::writeTextures(SpriteManager::getInstance());
	SpriteManager::recoverLayers(SpriteManager::getInstance());
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
}

/**
 * Setup editor's current page
 *
 * @memberof		AnimationInspector
 * @private
 *
 * @param this		Function scope
 */
static void AnimationInspector::setupMode(AnimationInspector this)
{
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing::text(Printing::getInstance(), " ANIMATION INSPECTOR ", 1, 0, NULL);
	Printing::text(Printing::getInstance(), " Accept \x13  ", 38, 1, NULL);
	Printing::text(Printing::getInstance(), " Cancel \x14  ", 38, 2, NULL);

	switch(this->mode)
	{
		case kSelectActor:

			AnimationInspector::printUserAnimatedEntities(this);
			break;

		case kSelectSprite:

			AnimationInspector::createSpriteSelector(this);
			AnimationInspector::printSprites(this);
			AnimationInspector::createSprite(this);
			Sprite::pause(this->animatedSprite, true);
			break;

		case kSelectAnimation:

			AnimationInspector::createAnimationsSelector(this);
			Sprite::pause(this->animatedSprite, true);
			AnimationInspector::printAnimatedEntityAnimations(this);
			break;

		case kEditAnimation:

			AnimationInspector::loadAnimationFunction(this);
			AnimationInspector::createAnimationEditionSelector(this);
			AnimationInspector::createFrameEditionSelector(this);
			AnimationController::playAnimationFunction(Sprite::getAnimationController(this->animatedSprite), &this->animationFunction);
			 Sprite::writeAnimation(this->animatedSprite);
			Sprite::pause(this->animatedSprite, true);
			Sprite::pause(this->animatedSprite, false);
			AnimationInspector::printAnimationConfig(this);
			break;
	}
}

/**
 * Process user input
 *
 * @memberof			AnimationInspector
 * @public
 *
 * @param this			Function scope
 * @param pressedKey	User input
 */
void AnimationInspector::processUserInput(AnimationInspector this, u16 pressedKey)
{
	ASSERT(this, "AnimationInspector::processUserInput: null this");

	if(!this->gameState)
	{
		return;
	}

	if(pressedKey & K_B)
	{
		this->mode--;

		if(kFirstMode >= this->mode)
		{
			this->mode = kFirstMode + 1;
		}
		else
		{
			AnimationInspector::setupMode(this);
		}

		return;
	}

	switch(this->mode)
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

/**
 * Select AnimatedEntity to work on
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationInspector::selectAnimatedEntity(AnimationInspector this, u32 pressedKey)
{
	int userAnimatedEntitiesCount = 0;
	for(; _userAnimatedEntities[userAnimatedEntitiesCount].animatedEntityDefinition; userAnimatedEntitiesCount++);

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
		this->mode = kSelectSprite;
		AnimationInspector::setupMode(this);
	}
}

/**
 * Select the Sprite to work on
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationInspector::selectSprite(AnimationInspector this, u32 pressedKey)
{
	int userAnimatedEntitiesCount = 0;
	for(; _userAnimatedEntities[userAnimatedEntitiesCount].animatedEntityDefinition; userAnimatedEntitiesCount++);

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
		this->mode = kSelectAnimation;
		AnimationInspector::setupMode(this);
	}
}

/**
 * Discard previous selected Sprite
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::removePreviousSprite(AnimationInspector this)
{
	if(this->animatedSprite)
	{
		SpriteManager::disposeSprite(SpriteManager::getInstance(), this->animatedSprite);
		this->animatedSprite = NULL;
	}
}

/**
 * Select the animation to work on
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationInspector::selectAnimation(AnimationInspector this, u32 pressedKey)
{
	this->animationDescription = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntityDefinition->animationDescription;

	int animationsCount = 0;
	for(; this->animationDescription->animationFunctions[animationsCount]; animationsCount++);

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
		this->mode = kEditAnimation;
		AnimationInspector::setupMode(this);
	}
}

/**
 * Start editing the selected animation
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationInspector::editAnimation(AnimationInspector this, u32 pressedKey)
{
	if(pressedKey & K_A)
	{
		if(Sprite::isPlaying(this->animatedSprite))
		{
			Sprite::pause(this->animatedSprite, true);

		}
		else
		{
			Sprite::pause(this->animatedSprite, false);
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
		int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
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

				if(0 > this->animationFunction.delay)
				{
					this->animationFunction.delay = 0;
				}
				break;

			case kLoop:

				this->animationFunction.loop = false;
				break;

			case kFrames:

				break;
		}
	}
	else if(pressedKey & K_LR)
	{
		int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
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

				if(1000 < this->animationFunction.delay)
				{
					this->animationFunction.delay = 1000;
				}
				break;

			case kLoop:

				this->animationFunction.loop = true;
				break;

			case kFrames:

				break;
		}
	}
	else if(pressedKey & K_RU)
	{
		int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kFrames:

				OptionsSelector::selectPrevious(this->frameEditionSelector);
				break;
		}
	}
	else if(pressedKey & K_RD)
	{
		int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kFrames:

				OptionsSelector::selectNext(this->frameEditionSelector);
				break;
		}
	}
	else if(pressedKey & K_RL)
	{
		int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		int selectedFrame = OptionsSelector::getSelectedOption(this->frameEditionSelector);

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
		int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);
		int selectedFrame = OptionsSelector::getSelectedOption(this->frameEditionSelector);

		switch(selectedProperty)
		{
			case kFrames:
				{
					NM_ASSERT(this->animatedSprite, "AnimationInspector::selectAnimation: null animatedSprite");

					Texture texture = Sprite::getTexture(__SAFE_CAST(Sprite, this->animatedSprite));
					NM_ASSERT(texture, "AnimationInspector::selectAnimation: null texture");

					TextureDefinition* textureDefinition = Texture::getTextureDefinition(texture);
					NM_ASSERT(textureDefinition, "AnimationInspector::selectAnimation: null textureDefinition");

					if(++this->animationFunction.frames[selectedFrame] >= textureDefinition->numberOfFrames)
					{
						this->animationFunction.frames[selectedFrame] = textureDefinition->numberOfFrames - 1;
					}
				}
				break;
		}
	}

	AnimationInspector::printAnimationConfig(this);
}

/**
 * Print the list of user AnimatedEntities
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::printUserAnimatedEntities(AnimationInspector this)
{
	Printing::text(Printing::getInstance(), "ACTORS", 1, 2, NULL);
	Printing::text(Printing::getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector::printOptions(this->animatedEntitySelector, 1, 4);
}

/**
 * Print available sprites for the selected AnimatedEntity
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::printSprites(AnimationInspector this)
{
	Printing::text(Printing::getInstance(), "SPRITES", 1, 2, NULL);
	Printing::text(Printing::getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector::printOptions(this->spriteSelector, 1, 4);
}

/**
 * Print a list of animation for the selected AnimatedEntity
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::printAnimatedEntityAnimations(AnimationInspector this)
{
	Printing::text(Printing::getInstance(), "AVAILABLE ANIMATIONS", 1, 2, NULL);
	Printing::text(Printing::getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector::printOptions(this->animationsSelector, 1, 4);
}

/**
 * Print selected animation' values
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::printAnimationConfig(AnimationInspector this)
{
	int x = 1;
	int y = 2;

	Printing::text(Printing::getInstance(), "Animation: ", x, y, NULL);
	Printing::text(Printing::getInstance(), this->animationFunction.name, x + 12, y++, NULL);
	OptionsSelector::printOptions(this->animationEditionSelector, x, ++y);

	Printing::int(Printing::getInstance(), this->animationFunction.numberOfFrames, x + 20, y++, NULL);
	Printing::int(Printing::getInstance(), this->animationFunction.delay, x + 20, y++, NULL);
	Printing::text(Printing::getInstance(), this->animationFunction.loop ? "true" : "false", x + 20, y++, NULL);

	OptionsSelector::printOptions(this->frameEditionSelector, x, ++y + 1);

	Printing::text(Printing::getInstance(), " Cancel   \x14 ", 36, 1, NULL);
	if(!Sprite::isPlaying(this->animatedSprite))
	{
		Printing::text(Printing::getInstance(), " Play     \x13 ", 36, 2, NULL);
	}
	else
	{
		Printing::text(Printing::getInstance(), " Pause    \x13 ", 36, 2, NULL);
	}
	Printing::text(Printing::getInstance(), " Select \x14 ", 36, 3, NULL);
	Printing::text(Printing::getInstance(), " Modify \x14 ", 36, 4, NULL);
	Printing::text(Printing::getInstance(), " Select \x1E\x1A\x1B", 36, 3, NULL);
	Printing::text(Printing::getInstance(), " Modify \x1E\x1C\x1D", 36, 4, NULL);

	int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);

	switch(selectedProperty)
	{
		case kFrames:

			Printing::text(Printing::getInstance(), " Select \x1F\x1A\x1B", 36, 6, NULL);
			Printing::text(Printing::getInstance(), " Modify \x1F\x1C\x1D", 36, 7, NULL);
			break;
	}
}

/**
 * Load the selected animation function to edit
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::loadAnimationFunction(AnimationInspector this)
{
	this->animationDescription = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntityDefinition->animationDescription;

	AnimationFunction* animationFunction = this->animationDescription->animationFunctions[OptionsSelector::getSelectedOption(this->animationsSelector)];

	int i = 0;
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

/**
 * Create a Sprite to work on
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::createSprite(AnimationInspector this)
{
	AnimationInspector::removePreviousSprite(this);

	Vector3D position = *_cameraPosition;

	position.x += __I_TO_FIX10_6(__HALF_SCREEN_WIDTH);
	position.y += __I_TO_FIX10_6(__HALF_SCREEN_HEIGHT);
	position.z -= 10;

	SpriteDefinition* spriteDefinition = (SpriteDefinition*)_userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntityDefinition->entityDefinition.spriteDefinitions[OptionsSelector::getSelectedOption(this->spriteSelector)];

	NM_ASSERT(spriteDefinition, "AnimationInspector::createSprite: null spriteDefinition");

	this->animatedSprite = __SAFE_CAST(Sprite, SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this)));
	ASSERT(this->animatedSprite, "AnimationInspector::createSprite: null animatedSprite");
	ASSERT(Sprite::getTexture(__SAFE_CAST(Sprite, this->animatedSprite)), "AnimationInspector::createSprite: null texture");

	PixelVector spritePosition = Sprite::getDisplacedPosition(__SAFE_CAST(Sprite, this->animatedSprite));
	spritePosition.x = ((__HALF_SCREEN_WIDTH) - (Texture::getCols(Sprite::getTexture(__SAFE_CAST(Sprite, this->animatedSprite))) << 2));
	spritePosition.y = ((__HALF_SCREEN_HEIGHT) - (Texture::getRows(Sprite::getTexture(__SAFE_CAST(Sprite, this->animatedSprite))) << 2));

	 Sprite::setPosition(this->animatedSprite, &spritePosition);
	 Sprite::applyAffineTransformations(this->animatedSprite);

	Rotation spriteRotation = {0, 0, 0};
	Scale spriteScale = {__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};

	 Sprite::setPosition(this->animatedSprite, &spritePosition);
	 Sprite::rotate(this->animatedSprite, &spriteRotation);
	 Sprite::resize(this->animatedSprite, spriteScale, spritePosition.z);
	 Sprite::calculateParallax(this->animatedSprite, spritePosition.z);

	this->animatedSprite->writeAnimationFrame = true;
	SpriteManager::writeTextures(SpriteManager::getInstance());
	SpriteManager::sortLayers(SpriteManager::getInstance());
	SpriteManager::render(SpriteManager::getInstance());
}

/**
 * Create OptionSelector for sprites
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::createSpriteSelector(AnimationInspector this)
{
	if(this->spriteSelector)
	{
		__DELETE(this->spriteSelector);
	}

	this->spriteSelector = __NEW(OptionsSelector, (__SCREEN_WIDTH_IN_CHARS) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION >> 1, NULL);

	VirtualList spriteIndexes = __NEW(VirtualList);

	int i = 0;
	while(_userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntityDefinition->entityDefinition.spriteDefinitions[i])
	{
		Option* option = __NEW_BASIC(Option);
		option->value = &i;
		option->type = kInt;
		VirtualList::pushBack(spriteIndexes, option);

		i++;
	}

	OptionsSelector::setOptions(this->spriteSelector, spriteIndexes);
	__DELETE(spriteIndexes);
}

/**
 * Create OptionSelector for animations
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::createAnimationsSelector(AnimationInspector this)
{
	this->animationDescription = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntityDefinition->animationDescription;

	if(this->animationDescription)
	{
		if(this->animationsSelector)
		{
			__DELETE(this->animationsSelector);
		}

		this->animationsSelector = __NEW(OptionsSelector, 2, 16, NULL);

		VirtualList animationsNames = __NEW(VirtualList);

		int i = 0;
		for(i = 0; this->animationDescription->animationFunctions[i]; i++)
		{
			Option* option = __NEW_BASIC(Option);
			option->value = this->animationDescription->animationFunctions[i]->name;
			option->type = kString;
			VirtualList::pushBack(animationsNames, option);
		}

		OptionsSelector::setOptions(this->animationsSelector, animationsNames);
		__DELETE(animationsNames);
	}
	else
	{
		//TODO
	}
}

/**
 * Create OptionSelector for editing the animation
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::createAnimationEditionSelector(AnimationInspector this)
{
	if(this->animationEditionSelector)
	{
		__DELETE(this->animationEditionSelector);
	}

	this->animationEditionSelector = __NEW(OptionsSelector, 1, 4, NULL);

	VirtualList optionsNames = __NEW(VirtualList);
	Option* option = NULL;

	option = __NEW_BASIC(Option);
	option->value = "Number of frames:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	option = __NEW_BASIC(Option);
	option->value = "Cycle delay:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	option = __NEW_BASIC(Option);
	option->value = "Loop:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	option = __NEW_BASIC(Option);
	option->value = "Frames:";
	option->type = kString;
	VirtualList::pushBack(optionsNames, option);

	OptionsSelector::setOptions(this->animationEditionSelector, optionsNames);
	__DELETE(optionsNames);

	this->mode = kEditAnimation;
}

/**
 * Create OptionSelector for the frames of the current animation
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector::createFrameEditionSelector(AnimationInspector this)
{
	if(this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
	}

	this->frameEditionSelector = __NEW(OptionsSelector, 4, 16, NULL);
	OptionsSelector::setColumnWidth(this->frameEditionSelector, 2);

	VirtualList framesIndexes = __NEW(VirtualList);

	int i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION && i < this->animationFunction.numberOfFrames; i++)
	{
		Option* option = __NEW_BASIC(Option);
		option->value = &this->animationFunction.frames[i];
		option->type = kInt;
		VirtualList::pushBack(framesIndexes, option);
	}

	OptionsSelector::setOptions(this->frameEditionSelector, framesIndexes);
	__DELETE(framesIndexes);
}

/**
 * Callback for when animation completes its playback
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 * @param eventFirer		AnimationController
 */
static void AnimationInspector::onAnimationComplete(AnimationInspector this, Object eventFirer __attribute__ ((unused)))
{
	if(!this->animationFunction.loop)
	{
		Printing::text(Printing::getInstance(), " Play     \x13 ", 36, 2, NULL);
	}
}
