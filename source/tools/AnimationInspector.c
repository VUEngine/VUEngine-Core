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

#include <AnimationInspector.h>
#include <Game.h>
#include <Optics.h>
#include <SpriteManager.h>
#include <GameState.h>
#include <Stage.h>
#include <Screen.h>
#include <string.h>
#include <OptionsSelector.h>
#include <Texture.h>
#include <AnimationController.h>
#include <BgmapTextureManager.h>
#include <KeyPadManager.h>
#include <Printing.h>


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

#define AnimationInspector_ATTRIBUTES																		\
		Object_ATTRIBUTES																				\
		/**
		 * @var GameState 				gameState
		 * @brief						current in game gameState
		 * @memberof					AnimationInspector
		 */																								\
		GameState gameState;																			\
		/**
		 * @var Sprite 					animatedSprite
		 * @brief						current animated sprite
		 * @memberof					AnimationInspector
		 */																								\
		Sprite animatedSprite;																			\
		/**
		 * @var AnimationDescription* 	animationDescription
		 * @brief						current animation description
		 * @memberof					AnimationInspector
		 */																								\
		AnimationDescription* animationDescription;														\
		/**
		 * @var AnimationFunction 		animationFunction
		 * @brief						current animation function
		 * @memberof					AnimationInspector
		 */																								\
		AnimationFunction animationFunction;															\
		/**
		 * @var OptionsSelector 		animatedInGameEntitySelector
		 * @brief						animated in game entity selector
		 * @memberof					AnimationInspector
		 */																								\
		OptionsSelector animatedInGameEntitySelector;													\
		/**
		 * @var OptionsSelector 		spriteSelector
		 * @brief						animated sprite selector
		 * @memberof					AnimationInspector
		 */																								\
		OptionsSelector spriteSelector;																	\
		/**
		 * @var OptionsSelector 		animationsSelector
		 * @brief						animations selector
		 * @memberof					AnimationInspector
		 */																								\
		OptionsSelector animationsSelector;																\
		/**
		 * @var OptionsSelector 		animationEditionSelector
		 * @brief						animation edition selector
		 * @memberof					AnimationInspector
		 */																								\
		OptionsSelector animationEditionSelector;														\
		/**
		 * @var OptionsSelector 		frameEditionSelector
		 * @brief						frame edition selector
		 * @memberof					AnimationInspector
		 */																								\
		OptionsSelector frameEditionSelector;															\
		/**
		 * @var int mode
		 * @brief						mode
		 * @memberof					AnimationInspector
		 */																								\
		int mode;																						\

/**
 * @class	AnimationInspector
 * @extends Object
 * @ingroup tools
 */
__CLASS_DEFINITION(AnimationInspector, Object);
__CLASS_FRIEND_DEFINITION(Sprite);

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

extern UserAnimatedInGameEntity _userAnimatedInGameEntities[];
extern const VBVec3D* _screenPosition;

AnimationController Sprite_getAnimationController(Sprite this);

static void AnimationInspector_constructor(AnimationInspector this);
static void AnimationInspector_setupMode(AnimationInspector this);
static void AnimationInspector_printUserAnimatedInGameEntities(AnimationInspector this);
static void AnimationInspector_printSprites(AnimationInspector this);
static void AnimationInspector_printAnimatedInGameEntityAnimations(AnimationInspector this);
static void AnimationInspector_printAnimationConfig(AnimationInspector this);
static void AnimationInspector_selectAnimatedInGameEntity(AnimationInspector this, u32 pressedKey);
static void AnimationInspector_selectSprite(AnimationInspector this, u32 pressedKey);
static void AnimationInspector_removePreviousSprite(AnimationInspector this);
static void AnimationInspector_selectAnimation(AnimationInspector this, u32 pressedKey);
static void AnimationInspector_editAnimation(AnimationInspector this, u32 pressedKey);
static void AnimationInspector_loadAnimationFunction(AnimationInspector this);
static void AnimationInspector_createSprite(AnimationInspector this);
static void AnimationInspector_createSpriteSelector(AnimationInspector this);
static void AnimationInspector_createAnimationsSelector(AnimationInspector this);
static void AnimationInspector_createAnimationEditionSelector(AnimationInspector this);
static void AnimationInspector_createFrameEditionSelector(AnimationInspector this);
static void AnimationInspector_onAnimationComplete(AnimationInspector this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationInspector_getInstance()
 * @memberof	AnimationInspector
 * @public
 *
 * @return		AnimationInspector instance
 */
__SINGLETON(AnimationInspector);

/**
 * Class constructor
 *
 * @memberof	AnimationInspector
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) AnimationInspector_constructor(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->animatedSprite = NULL;
	this->gameState = NULL;
	this->animatedInGameEntitySelector = NULL;
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
void AnimationInspector_destructor(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::destructor: null this");

	if(this->animatedInGameEntitySelector)
	{
		__DELETE(this->animatedInGameEntitySelector);
	}

	if(this->animatedInGameEntitySelector)
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
void AnimationInspector_update(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::update: null this");

	if(this->gameState && this->animatedSprite)
	{
		Sprite_updateAnimation(this->animatedSprite);
		Sprite_update(this->animatedSprite);
		__VIRTUAL_CALL(Sprite, applyAffineTransformations, this->animatedSprite);
		__VIRTUAL_CALL(Sprite, applyHbiasEffects, this->animatedSprite);
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
void AnimationInspector_show(AnimationInspector this, GameState gameState)
{
	ASSERT(this, "AnimationInspector::start: null this");
	ASSERT(gameState, "AnimationInspector::start: null gameState");

	this->gameState = gameState;
	this->animatedSprite = NULL;

	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->animatedInGameEntitySelector = __NEW(OptionsSelector, 2, 16, NULL);

	VirtualList animatedInGameEntitiesNames = __NEW(VirtualList);

	int i = 0;
	for(; _userAnimatedInGameEntities[i].animatedInGameEntityDefinition; i++)
	{
		ASSERT(_userAnimatedInGameEntities[i].name, "AnimationInspector::start: push null name");

		Option* option = __NEW_BASIC(Option);
		option->value = (char*)(_userAnimatedInGameEntities[i].name);
		option->type = kString;
		VirtualList_pushBack(animatedInGameEntitiesNames, option);
	}

	ASSERT(animatedInGameEntitiesNames, "AnimationInspector::start: null animatedInGameEntitiesNames");
	ASSERT(VirtualList_getSize(animatedInGameEntitiesNames), "AnimationInspector::start: empty animatedInGameEntitiesNames");

	OptionsSelector_setOptions(this->animatedInGameEntitySelector, animatedInGameEntitiesNames);
	__DELETE(animatedInGameEntitiesNames);

	this->mode = kFirstMode + 1;
	AnimationInspector_setupMode(this);
	SpriteManager_showLayer(SpriteManager_getInstance(), SpriteManager_getFreeLayer(SpriteManager_getInstance()));

	// make sure all textures are written right now
	SpriteManager_writeTextures(SpriteManager_getInstance());
	SpriteManager_deferAffineTransformations(SpriteManager_getInstance(), false);
}

/**
 * Hide editor
 *
 * @memberof		AnimationInspector
 * @public
 *
 * @param this		Function scope
 */
void AnimationInspector_hide(AnimationInspector this)
{
	ASSERT(this, "AnimationInspector::stop: null this");

	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);

	AnimationInspector_removePreviousSprite(this);

	if(this->animatedInGameEntitySelector)
	{
		__DELETE(this->animatedInGameEntitySelector);
		this->animatedInGameEntitySelector = NULL;
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
	SpriteManager_writeTextures(SpriteManager_getInstance());
	SpriteManager_recoverLayers(SpriteManager_getInstance());
	SpriteManager_deferAffineTransformations(SpriteManager_getInstance(), true);
}

/**
 * Setup editor's current page
 *
 * @memberof		AnimationInspector
 * @private
 *
 * @param this		Function scope
 */
static void AnimationInspector_setupMode(AnimationInspector this)
{
	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	Printing_text(Printing_getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing_text(Printing_getInstance(), " ANIMATION INSPECTOR ", 1, 0, NULL);
	Printing_text(Printing_getInstance(), " Accept \x13  ", 38, 1, NULL);
	Printing_text(Printing_getInstance(), " Cancel \x14  ", 38, 2, NULL);

	switch(this->mode)
	{
		case kSelectActor:

			AnimationInspector_printUserAnimatedInGameEntities(this);
			break;

		case kSelectSprite:

			AnimationInspector_createSpriteSelector(this);
			AnimationInspector_printSprites(this);
			AnimationInspector_createSprite(this);
			Sprite_pause(this->animatedSprite, true);
			break;

		case kSelectAnimation:

			AnimationInspector_createAnimationsSelector(this);
			Sprite_pause(this->animatedSprite, true);
			AnimationInspector_printAnimatedInGameEntityAnimations(this);
			break;

		case kEditAnimation:

			AnimationInspector_loadAnimationFunction(this);
			AnimationInspector_createAnimationEditionSelector(this);
			AnimationInspector_createFrameEditionSelector(this);
			AnimationController_playAnimationFunction(Sprite_getAnimationController(this->animatedSprite), &this->animationFunction);
			__VIRTUAL_CALL(Sprite, writeAnimation, this->animatedSprite);
			Sprite_pause(this->animatedSprite, true);
			Sprite_pause(this->animatedSprite, false);
			AnimationInspector_printAnimationConfig(this);
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
void AnimationInspector_processUserInput(AnimationInspector this, u16 pressedKey)
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
			AnimationInspector_setupMode(this);
		}

		return;
	}

	switch(this->mode)
	{
		case kSelectActor:

			AnimationInspector_selectAnimatedInGameEntity(this, pressedKey);
			break;

		case kSelectSprite:

			AnimationInspector_selectSprite(this, pressedKey);
			break;

		case kSelectAnimation:

			AnimationInspector_selectAnimation(this, pressedKey);
			break;

		case kEditAnimation:

			AnimationInspector_editAnimation(this, pressedKey);
			break;
	}
}

/**
 * Select AnimatedInGameEntity to work on
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationInspector_selectAnimatedInGameEntity(AnimationInspector this, u32 pressedKey)
{
	int userAnimatedInGameEntitiesCount = 0;
	for(; _userAnimatedInGameEntities[userAnimatedInGameEntitiesCount].animatedInGameEntityDefinition; userAnimatedInGameEntitiesCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->animatedInGameEntitySelector);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->animatedInGameEntitySelector);
	}
	else if(pressedKey & K_A)
	{
		// select the added entity
		this->mode = kSelectSprite;
		AnimationInspector_setupMode(this);
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
static void AnimationInspector_selectSprite(AnimationInspector this, u32 pressedKey)
{
	int userAnimatedInGameEntitiesCount = 0;
	for(; _userAnimatedInGameEntities[userAnimatedInGameEntitiesCount].animatedInGameEntityDefinition; userAnimatedInGameEntitiesCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->spriteSelector);
		AnimationInspector_createSprite(this);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->spriteSelector);
		AnimationInspector_createSprite(this);
	}
	else if(pressedKey & K_A)
	{
		// select the added entity
		this->mode = kSelectAnimation;
		AnimationInspector_setupMode(this);
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
static void AnimationInspector_removePreviousSprite(AnimationInspector this)
{
	if(this->animatedSprite)
	{
		__DELETE(this->animatedSprite);
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
static void AnimationInspector_selectAnimation(AnimationInspector this, u32 pressedKey)
{
	this->animationDescription = _userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->animationDescription;

	int animationsCount = 0;
	for(; this->animationDescription->animationFunctions[animationsCount]; animationsCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->animationsSelector);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->animationsSelector);
	}
	else if(pressedKey & K_A)
	{
		this->mode = kEditAnimation;
		AnimationInspector_setupMode(this);
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
static void AnimationInspector_editAnimation(AnimationInspector this, u32 pressedKey)
{
	if(pressedKey & K_A)
	{
		if(Sprite_isPlaying(this->animatedSprite))
		{
			Sprite_pause(this->animatedSprite, true);

		}
		else
		{
			Sprite_pause(this->animatedSprite, false);
		}
	}
	else if((pressedKey & K_LU))
	{
		OptionsSelector_selectPrevious(this->animationEditionSelector);
	}
	else if((pressedKey & K_LD))
	{
		OptionsSelector_selectNext(this->animationEditionSelector);
	}
	else if((pressedKey & K_LL))
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kNumberOfFrames:

				if(0 == --this->animationFunction.numberOfFrames)
				{
					this->animationFunction.numberOfFrames = 1;
				}

				AnimationInspector_createFrameEditionSelector(this);
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
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kNumberOfFrames:

				if(__MAX_FRAMES_PER_ANIMATION_FUNCTION < ++this->animationFunction.numberOfFrames)
				{
					this->animationFunction.numberOfFrames = __MAX_FRAMES_PER_ANIMATION_FUNCTION;
				}

				AnimationInspector_createFrameEditionSelector(this);

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
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kFrames:

				OptionsSelector_selectPrevious(this->frameEditionSelector);
				break;
		}
	}
	else if(pressedKey & K_RD)
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch(selectedProperty)
		{
			case kFrames:

				OptionsSelector_selectNext(this->frameEditionSelector);
				break;
		}
	}
	else if(pressedKey & K_RL)
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		int selectedFrame = OptionsSelector_getSelectedOption(this->frameEditionSelector);

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
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		int selectedFrame = OptionsSelector_getSelectedOption(this->frameEditionSelector);

		switch(selectedProperty)
		{
			case kFrames:
				{
					NM_ASSERT(this->animatedSprite, "AnimationInspector::selectAnimation: null animatedSprite");

					Texture texture = Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite));
					NM_ASSERT(texture, "AnimationInspector::selectAnimation: null texture");

					TextureDefinition* textureDefinition = Texture_getTextureDefinition(texture);
					NM_ASSERT(textureDefinition, "AnimationInspector::selectAnimation: null textureDefinition");

					if(++this->animationFunction.frames[selectedFrame] >= textureDefinition->numberOfFrames)
					{
						this->animationFunction.frames[selectedFrame] = textureDefinition->numberOfFrames - 1;
					}
				}
				break;
		}
	}

	AnimationInspector_printAnimationConfig(this);
}

/**
 * Print the list of user AnimatedInGameEntities
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector_printUserAnimatedInGameEntities(AnimationInspector this)
{
	Printing_text(Printing_getInstance(), "ACTORS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->animatedInGameEntitySelector, 1, 4);
}

/**
 * Print available sprites for the selected AnimatedInGameEntity
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector_printSprites(AnimationInspector this)
{
	Printing_text(Printing_getInstance(), "SPRITES", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->spriteSelector, 1, 4);
}

/**
 * Print a list of animation for the selected AnimatedInGameEntity
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector_printAnimatedInGameEntityAnimations(AnimationInspector this)
{
	Printing_text(Printing_getInstance(), "AVAILABLE ANIMATIONS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->animationsSelector, 1, 4);
}

/**
 * Print selected animation' values
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector_printAnimationConfig(AnimationInspector this)
{
	int x = 1;
	int y = 2;

	Printing_text(Printing_getInstance(), "Animation: ", x, y, NULL);
	Printing_text(Printing_getInstance(), this->animationFunction.name, x + 12, y++, NULL);
	OptionsSelector_printOptions(this->animationEditionSelector, x, ++y);

	Printing_int(Printing_getInstance(), this->animationFunction.numberOfFrames, x + 20, y++, NULL);
	Printing_int(Printing_getInstance(), this->animationFunction.delay, x + 20, y++, NULL);
	Printing_text(Printing_getInstance(), this->animationFunction.loop ? "true" : "false", x + 20, y++, NULL);

	OptionsSelector_printOptions(this->frameEditionSelector, x, ++y + 1);

	Printing_text(Printing_getInstance(), " Cancel   \x14 ", 36, 1, NULL);
	if(!Sprite_isPlaying(this->animatedSprite))
	{
		Printing_text(Printing_getInstance(), " Play     \x13 ", 36, 2, NULL);
	}
	else
	{
		Printing_text(Printing_getInstance(), " Pause    \x13 ", 36, 2, NULL);
	}
	Printing_text(Printing_getInstance(), " Select \x14 ", 36, 3, NULL);
	Printing_text(Printing_getInstance(), " Modify \x14 ", 36, 4, NULL);
	Printing_text(Printing_getInstance(), " Select \x1E\x1A\x1B", 36, 3, NULL);
	Printing_text(Printing_getInstance(), " Modify \x1E\x1C\x1D", 36, 4, NULL);

	int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);

	switch(selectedProperty)
	{
		case kFrames:

			Printing_text(Printing_getInstance(), " Select \x1F\x1A\x1B", 36, 6, NULL);
			Printing_text(Printing_getInstance(), " Modify \x1F\x1C\x1D", 36, 7, NULL);
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
static void AnimationInspector_loadAnimationFunction(AnimationInspector this)
{
	this->animationDescription = _userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->animationDescription;

	AnimationFunction* animationFunction = this->animationDescription->animationFunctions[OptionsSelector_getSelectedOption(this->animationsSelector)];

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
static void AnimationInspector_createSprite(AnimationInspector this)
{
	AnimationInspector_removePreviousSprite(this);

	VBVec3D position = *_screenPosition;

	position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
	position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
	position.z -= 10;

	SpriteDefinition* spriteDefinition = (SpriteDefinition*)_userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->inGameEntityDefinition.entityDefinition.spritesDefinitions[OptionsSelector_getSelectedOption(this->spriteSelector)];

	NM_ASSERT(spriteDefinition, "AnimationInspector::createSprite: null spriteDefinition");

	this->animatedSprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this));
	ASSERT(this->animatedSprite, "AnimationInspector::createSprite: null animatedSprite");
	ASSERT(Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite)), "AnimationInspector::createSprite: null texture");

	VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, __SAFE_CAST(Sprite, this->animatedSprite));
	spritePosition.x = ITOFIX19_13((__SCREEN_WIDTH >> 1) - (Texture_getCols(Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite))) << 2));
	spritePosition.y = ITOFIX19_13((__SCREEN_HEIGHT >> 1) - (Texture_getRows(Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite))) << 2));

	__VIRTUAL_CALL(Sprite, setPosition, __SAFE_CAST(Sprite, this->animatedSprite), &spritePosition);
	__VIRTUAL_CALL(Sprite, applyAffineTransformations, __SAFE_CAST(Sprite, this->animatedSprite));
	SpriteManager_showLayer(SpriteManager_getInstance(), __VIRTUAL_CALL(Sprite, getWorldLayer, __SAFE_CAST(Sprite, this->animatedSprite)));

	Rotation spriteRotation = {0, 0, 0};
	Scale spriteScale = {__1I_FIX7_9, __1I_FIX7_9};

	__VIRTUAL_CALL(Sprite, setPosition, this->animatedSprite, &spritePosition);
	__VIRTUAL_CALL(Sprite, rotate, this->animatedSprite, &spriteRotation);
	__VIRTUAL_CALL(Sprite, resize, this->animatedSprite, spriteScale, spritePosition.z);
	__VIRTUAL_CALL(Sprite, calculateParallax, this->animatedSprite, spritePosition.z);

	this->animatedSprite->writeAnimationFrame = true;
	SpriteManager_writeTextures(SpriteManager_getInstance());
	SpriteManager_sortLayers(SpriteManager_getInstance());
	SpriteManager_render(SpriteManager_getInstance());
}

/**
 * Create OptionSelector for sprites
 *
 * @memberof				AnimationInspector
 * @private
 *
 * @param this				Function scope
 */
static void AnimationInspector_createSpriteSelector(AnimationInspector this)
{
	if(this->spriteSelector)
	{
		__DELETE(this->spriteSelector);
	}

	this->spriteSelector = __NEW(OptionsSelector, (__SCREEN_WIDTH >> 3) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION >> 1, NULL);

	VirtualList spriteIndexes = __NEW(VirtualList);

	int i = 0;
	while(_userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->inGameEntityDefinition.entityDefinition.spritesDefinitions[i])
	{
		Option* option = __NEW_BASIC(Option);
		option->value = &i;
		option->type = kInt;
		VirtualList_pushBack(spriteIndexes, option);

		i++;
	}

	OptionsSelector_setOptions(this->spriteSelector, spriteIndexes);
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
static void AnimationInspector_createAnimationsSelector(AnimationInspector this)
{
	this->animationDescription = _userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->animationDescription;

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
			VirtualList_pushBack(animationsNames, option);
		}

		OptionsSelector_setOptions(this->animationsSelector, animationsNames);
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
static void AnimationInspector_createAnimationEditionSelector(AnimationInspector this)
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
	VirtualList_pushBack(optionsNames, option);

	option = __NEW_BASIC(Option);
	option->value = "Cycle delay:";
	option->type = kString;
	VirtualList_pushBack(optionsNames, option);

	option = __NEW_BASIC(Option);
	option->value = "Loop:";
	option->type = kString;
	VirtualList_pushBack(optionsNames, option);

	option = __NEW_BASIC(Option);
	option->value = "Frames:";
	option->type = kString;
	VirtualList_pushBack(optionsNames, option);

	OptionsSelector_setOptions(this->animationEditionSelector, optionsNames);
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
static void AnimationInspector_createFrameEditionSelector(AnimationInspector this)
{
	if(this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
	}

	this->frameEditionSelector = __NEW(OptionsSelector, 4, 16, NULL);
	OptionsSelector_setColumnWidth(this->frameEditionSelector, 2);

	VirtualList framesIndexes = __NEW(VirtualList);

	int i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION && i < this->animationFunction.numberOfFrames; i++)
	{
		Option* option = __NEW_BASIC(Option);
		option->value = &this->animationFunction.frames[i];
		option->type = kInt;
		VirtualList_pushBack(framesIndexes, option);
	}

	OptionsSelector_setOptions(this->frameEditionSelector, framesIndexes);
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
static void AnimationInspector_onAnimationComplete(AnimationInspector this, Object eventFirer __attribute__ ((unused)))
{
	if(!this->animationFunction.loop)
	{
		Printing_text(Printing_getInstance(), " Play     \x13 ", 36, 2, NULL);
	}
}
