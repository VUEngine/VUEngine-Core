/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#ifdef __TOOLS


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
#include <KeypadManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Sprite;

extern UserAnimatedEntity _userAnimatedEntities[];


//---------------------------------------------------------------------------------------------------------
//												ENUMS
//---------------------------------------------------------------------------------------------------------

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
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationInspector::getInstance()
 * @memberof	AnimationInspector
 * @public
 * @return		AnimationInspector instance
 */


/**
 * Class constructor
 *
 * @private
 */
void AnimationInspector::constructor()
{
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
 */
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

/**
 * Update
 */
void AnimationInspector::update()
{
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
 * @param gameState Current game state
 */
void AnimationInspector::show()
{
	this->animatedSprite = NULL;

	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->animatedEntitySelector = new OptionsSelector(2, 16, NULL);

	VirtualList animatedEntitiesNames = new VirtualList();

	int i = 0;
	for(; _userAnimatedEntities[i].animatedEntitySpec; i++)
	{
		ASSERT(_userAnimatedEntities[i].name, "AnimationInspector::start: push null name");

		Option* option = new Option;
		option->value = (char*)(_userAnimatedEntities[i].name);
		option->type = kString;
		VirtualList::pushBack(animatedEntitiesNames, option);
	}

	ASSERT(animatedEntitiesNames, "AnimationInspector::start: null animatedEntitiesNames");
	ASSERT(VirtualList::getSize(animatedEntitiesNames), "AnimationInspector::start: empty animatedEntitiesNames");

	OptionsSelector::setOptions(this->animatedEntitySelector, animatedEntitiesNames);
	delete animatedEntitiesNames;

	this->mode = kFirstMode + 1;
	AnimationInspector::setupMode(this);
	SpriteManager::showLayer(SpriteManager::getInstance(), SpriteManager::getFreeLayer(SpriteManager::getInstance()));

	// make sure all textures are written right now
	SpriteManager::writeTextures(SpriteManager::getInstance());
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);
}

/**
 * Hide editor
 */
void AnimationInspector::hide()
{
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);

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
	SpriteManager::recoverLayers(SpriteManager::getInstance());
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
}

/**
 * Setup editor's current page
 *
 * @private
 */
void AnimationInspector::setupMode()
{
	Printing printing = Printing::getInstance();

	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	Printing::text(printing, "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing::text(printing, " ANIMATION INSPECTOR ", 1, 0, NULL);
	Printing::text(printing, "             ", 39, 2, NULL);
	Printing::text(printing, "             ", 39, 3, NULL);

	switch(this->mode)
	{
		case kSelectActor:

			Printing::text(printing, "Select \x13  ", 39, 2, NULL);
			AnimationInspector::printUserAnimatedEntities(this);
			break;

		case kSelectSprite:

			Printing::text(printing, "Select \x13  ", 39, 2, NULL);
			Printing::text(printing, "Back   \x14  ", 39, 3, NULL);
			AnimationInspector::createSpriteSelector(this);
			AnimationInspector::printSprites(this);
			AnimationInspector::createSprite(this);
			Sprite::pause(this->animatedSprite, true);
			break;

		case kSelectAnimation:

			Printing::text(printing, "Select \x13  ", 39, 2, NULL);
			Printing::text(printing, "Back   \x14  ", 39, 3, NULL);
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
 * @param pressedKey	User input
 */
void AnimationInspector::processUserInput(u16 pressedKey)
{
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
 * @private
 * @param pressedKey		User input
 */
void AnimationInspector::selectAnimatedEntity(u32 pressedKey)
{
	int userAnimatedEntitiesCount = 0;
	for(; _userAnimatedEntities[userAnimatedEntitiesCount].animatedEntitySpec; userAnimatedEntitiesCount++);

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
 * @private
 * @param pressedKey		User input
 */
void AnimationInspector::selectSprite(u32 pressedKey)
{
	int userAnimatedEntitiesCount = 0;
	for(; _userAnimatedEntities[userAnimatedEntitiesCount].animatedEntitySpec; userAnimatedEntitiesCount++);

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
 * @private
 */
void AnimationInspector::removePreviousSprite()
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
 * @private
 * @param pressedKey		User input
 */
void AnimationInspector::selectAnimation(u32 pressedKey)
{
	this->animationDescription = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->animationDescription;

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
 * @private
 * @param pressedKey		User input
 */
void AnimationInspector::editAnimation(u32 pressedKey)
{
	if(pressedKey & K_A)
	{
		if(Sprite::isPlaying(this->animatedSprite))
		{
			Sprite::pause(this->animatedSprite, true);
		}
		else
		{
			AnimationController::playAnimationFunction(Sprite::getAnimationController(this->animatedSprite), &this->animationFunction);
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

				this->animationFunction.loop = !this->animationFunction.loop;
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

				this->animationFunction.loop = !this->animationFunction.loop;
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

					Texture texture = Sprite::getTexture(this->animatedSprite);
					NM_ASSERT(texture, "AnimationInspector::selectAnimation: null texture");

					TextureSpec* textureSpec = Texture::getTextureSpec(texture);
					NM_ASSERT(textureSpec, "AnimationInspector::selectAnimation: null textureSpec");

					if(++this->animationFunction.frames[selectedFrame] >= textureSpec->numberOfFrames && textureSpec->charSetSpec->allocationType == __ANIMATED_MULTI)
					{
						this->animationFunction.frames[selectedFrame] = textureSpec->numberOfFrames - 1;
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
 * @private
 */
void AnimationInspector::printUserAnimatedEntities()
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "OBJECTS", 1, 2, NULL);
	Printing::text(printing, "                       ", 1, 3, NULL);
	OptionsSelector::printOptions(this->animatedEntitySelector, 1, 4);
}

/**
 * Print available sprites for the selected AnimatedEntity
 *
 * @private
 */
void AnimationInspector::printSprites()
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "SPRITES", 1, 2, NULL);
	Printing::text(printing, "                       ", 1, 3, NULL);
	OptionsSelector::printOptions(this->spriteSelector, 1, 4);
}

/**
 * Print a list of animation for the selected AnimatedEntity
 *
 * @private
 */
void AnimationInspector::printAnimatedEntityAnimations()
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "AVAILABLE ANIMATIONS", 1, 2, NULL);
	Printing::text(printing, "                       ", 1, 3, NULL);
	OptionsSelector::printOptions(this->animationsSelector, 1, 4);
}

/**
 * Print selected animation' values
 *
 * @private
 */
void AnimationInspector::printAnimationConfig()
{
	int x = 1;
	int y = 2;
	Printing printing = Printing::getInstance();

	Printing::text(printing, "Animation: ", x, y, NULL);
	Printing::text(printing, this->animationFunction.name, x + 11, y++, NULL);
	OptionsSelector::printOptions(this->animationEditionSelector, x, ++y);

	Printing::int(printing, this->animationFunction.numberOfFrames, x + 19, y++, NULL);
	Printing::int(printing, this->animationFunction.delay, x + 19, y++, NULL);
	Printing::text(printing, this->animationFunction.loop ? "true" : "false", x + 19, y++, NULL);

	OptionsSelector::printOptions(this->frameEditionSelector, x, ++y + 1);

	Printing::text(printing, "Back     \x14 ", 37, 2, NULL);
	if(!Sprite::isPlaying(this->animatedSprite))
	{
		Printing::text(printing, "Play     \x13 ", 37, 3, NULL);
	}
	else
	{
		Printing::text(printing, "Pause    \x13 ", 37, 3, NULL);
	}
	Printing::text(printing, "Select \x1E\x1A\x1B", 37, 4, NULL);
	Printing::text(printing, "Modify \x1E\x1C\x1D", 37, 5, NULL);

	int selectedProperty = OptionsSelector::getSelectedOption(this->animationEditionSelector);

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

/**
 * Load the selected animation function to edit
 *
 * @private
 */
void AnimationInspector::loadAnimationFunction()
{
	this->animationDescription = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->animationDescription;

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
 * @private
 */
void AnimationInspector::createSprite()
{
	AnimationInspector::removePreviousSprite(this);

	Vector3D position = *_cameraPosition;

	position.x += __I_TO_FIX10_6(__HALF_SCREEN_WIDTH);
	position.y += __I_TO_FIX10_6(__HALF_SCREEN_HEIGHT);
	position.z -= 10;

	SpriteSpec* spriteSpec = (SpriteSpec*)_userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->entitySpec.spriteSpecs[OptionsSelector::getSelectedOption(this->spriteSelector)];

	NM_ASSERT(spriteSpec, "AnimationInspector::createSprite: null spriteSpec");

	this->animatedSprite = Sprite::safeCast(SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteSpec*)spriteSpec, Object::safeCast(this)));
	ASSERT(this->animatedSprite, "AnimationInspector::createSprite: null animatedSprite");
	ASSERT(Sprite::getTexture(this->animatedSprite), "AnimationInspector::createSprite: null texture");

	PixelVector spritePosition = Sprite::getDisplacedPosition(this->animatedSprite);
	spritePosition.x = ((__HALF_SCREEN_WIDTH) - (Texture::getCols(Sprite::getTexture(this->animatedSprite)) << 2));
	spritePosition.y = ((__HALF_SCREEN_HEIGHT) - (Texture::getRows(Sprite::getTexture(this->animatedSprite)) << 2));

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
	SpriteManager::sort(SpriteManager::getInstance());
	SpriteManager::render(SpriteManager::getInstance());
}

/**
 * Create OptionSelector for sprites
 *
 * @private
 */
void AnimationInspector::createSpriteSelector()
{
	if(this->spriteSelector)
	{
		delete this->spriteSelector;
	}

	this->spriteSelector = new OptionsSelector((__SCREEN_WIDTH_IN_CHARS) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION >> 1, NULL);

	VirtualList spriteIndexes = new VirtualList();

	int i = 0;
	while(_userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->entitySpec.spriteSpecs[i])
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

/**
 * Create OptionSelector for animations
 *
 * @private
 */
void AnimationInspector::createAnimationsSelector()
{
	this->animationDescription = _userAnimatedEntities[OptionsSelector::getSelectedOption(this->animatedEntitySelector)].animatedEntitySpec->animationDescription;

	if(this->animationDescription)
	{
		if(this->animationsSelector)
		{
			delete this->animationsSelector;
		}

		this->animationsSelector = new OptionsSelector(2, 16, NULL);

		VirtualList animationsNames = new VirtualList();

		int i = 0;
		for(i = 0; this->animationDescription->animationFunctions[i]; i++)
		{
			Option* option = new Option;
			option->value = this->animationDescription->animationFunctions[i]->name;
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

/**
 * Create OptionSelector for editing the animation
 *
 * @private
 */
void AnimationInspector::createAnimationEditionSelector()
{
	if(this->animationEditionSelector)
	{
		delete this->animationEditionSelector;
	}

	this->animationEditionSelector = new OptionsSelector(1, 4, NULL);

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

	this->mode = kEditAnimation;
}

/**
 * Create OptionSelector for the frames of the current animation
 *
 * @private
 */
void AnimationInspector::createFrameEditionSelector()
{
	if(this->frameEditionSelector)
	{
		delete this->frameEditionSelector;
	}

	this->frameEditionSelector = new OptionsSelector(4, 16, NULL);
	OptionsSelector::setColumnWidth(this->frameEditionSelector, 2);

	VirtualList framesIndexes = new VirtualList();

	int i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION && i < this->animationFunction.numberOfFrames; i++)
	{
		Option* option = new Option;
		option->value = &this->animationFunction.frames[i];
		option->type = kInt;
		VirtualList::pushBack(framesIndexes, option);
	}

	OptionsSelector::setOptions(this->frameEditionSelector, framesIndexes);
	delete framesIndexes;
}

/**
 * Callback for when animation completes its playback
 *
 * @private
 * @param eventFirer		AnimationController
 */
void AnimationInspector::onAnimationComplete(Object eventFirer __attribute__ ((unused)))
{
	if(!this->animationFunction.loop)
	{
		Printing::text(Printing::getInstance(), "Play     \x13 ", 37, 3, NULL);
	}
}

#endif
