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

#include <AnimationEditor.h>
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

#define AnimationEditor_ATTRIBUTES																		\
		Object_ATTRIBUTES																				\
		/**
		 * @var GameState 				gameState
		 * @brief						current in game gameState
		 * @memberof					AnimationEditor
		 */																								\
		GameState gameState;																			\
		/**
		 * @var Sprite 					animatedSprite
		 * @brief						current animated sprite
		 * @memberof					AnimationEditor
		 */																								\
		Sprite animatedSprite;																			\
		/**
		 * @var AnimationDescription* 	animationDescription
		 * @brief						current animation description
		 * @memberof					AnimationEditor
		 */																								\
		AnimationDescription* animationDescription;														\
		/**
		 * @var AnimationFunction 		animationFunction
		 * @brief						current animation function
		 * @memberof					AnimationEditor
		 */																								\
		AnimationFunction animationFunction;															\
		/**
		 * @var OptionsSelector 		animatedInGameEntitySelector
		 * @brief						animated in game entity selector
		 * @memberof					AnimationEditor
		 */																								\
		OptionsSelector animatedInGameEntitySelector;													\
		/**
		 * @var OptionsSelector 		spriteSelector
		 * @brief						animated sprite selector
		 * @memberof					AnimationEditor
		 */																								\
		OptionsSelector spriteSelector;																	\
		/**
		 * @var OptionsSelector 		animationsSelector
		 * @brief						animations selector
		 * @memberof					AnimationEditor
		 */																								\
		OptionsSelector animationsSelector;																\
		/**
		 * @var OptionsSelector 		animationEditionSelector
		 * @brief						animation edition selector
		 * @memberof					AnimationEditor
		 */																								\
		OptionsSelector animationEditionSelector;														\
		/**
		 * @var OptionsSelector 		frameEditionSelector
		 * @brief						frame edition selector
		 * @memberof					AnimationEditor
		 */																								\
		OptionsSelector frameEditionSelector;															\
		/**
		 * @var int mode
		 * @brief						mode
		 * @memberof					AnimationEditor
		 */																								\
		int mode;																						\

/**
 * @class	AnimationEditor
 * @extends Object
 * @ingroup tools
 */
__CLASS_DEFINITION(AnimationEditor, Object);

/**
 * The different modes of the AnimationEditor
 *
 * @memberof	AnimationEditor
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
 * @memberof	AnimationEditor
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

static void AnimationEditor_constructor(AnimationEditor this);
static void AnimationEditor_setupMode(AnimationEditor this);
static void AnimationEditor_printUserAnimatedInGameEntities(AnimationEditor this);
static void AnimationEditor_printSprites(AnimationEditor this);
static void AnimationEditor_printAnimatedInGameEntityAnimations(AnimationEditor this);
static void AnimationEditor_printAnimationConfig(AnimationEditor this);
static void AnimationEditor_selectAnimatedInGameEntity(AnimationEditor this, u32 pressedKey);
static void AnimationEditor_selectSprite(AnimationEditor this, u32 pressedKey);
static void AnimationEditor_removePreviousSprite(AnimationEditor this);
static void AnimationEditor_selectAnimation(AnimationEditor this, u32 pressedKey);
static void AnimationEditor_editAnimation(AnimationEditor this, u32 pressedKey);
static void AnimationEditor_loadAnimationFunction(AnimationEditor this);
static void AnimationEditor_createSprite(AnimationEditor this);
static void AnimationEditor_createSpriteSelector(AnimationEditor this);
static void AnimationEditor_createAnimationsSelector(AnimationEditor this);
static void AnimationEditor_createAnimationEditionSelector(AnimationEditor this);
static void AnimationEditor_createFrameEditionSelector(AnimationEditor this);
static void AnimationEditor_onAnimationComplete(AnimationEditor this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationEditor_getInstance()
 * @memberof	AnimationEditor
 * @public
 *
 * @return		AnimationEditor instance
 */
__SINGLETON(AnimationEditor);

/**
 * Class constructor
 *
 * @memberof	AnimationEditor
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) AnimationEditor_constructor(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::constructor: null this");

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
 * @memberof	AnimationEditor
 * @public
 *
 * @param this	Function scope
 */
void AnimationEditor_destructor(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::destructor: null this");

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
 * @memberof	AnimationEditor
 * @public
 *
 * @param this	Function scope
 */
void AnimationEditor_update(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::update: null this");

	if(this->gameState && this->animatedSprite)
	{
		Sprite_updateAnimation(this->animatedSprite);
		Sprite_update(this->animatedSprite);
		__VIRTUAL_CALL(Sprite, applyAffineTransformations, this->animatedSprite);
		__VIRTUAL_CALL(Sprite, applyHbiasTransformations, this->animatedSprite);
	}
}

/**
 * Show editor
 *
 * @memberof		AnimationEditor
 * @public
 *
 * @param this		Function scope
 * @param gameState Current game state
 */
void AnimationEditor_show(AnimationEditor this, GameState gameState)
{
	ASSERT(this, "AnimationEditor::start: null this");
	ASSERT(gameState, "AnimationEditor::start: null gameState");

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
		ASSERT(_userAnimatedInGameEntities[i].name, "AnimationEditor::start: push null name");

		Option* option = __NEW_BASIC(Option);
		option->value = (char*)(_userAnimatedInGameEntities[i].name);
		option->type = kString;
		VirtualList_pushBack(animatedInGameEntitiesNames, option);
	}

	ASSERT(animatedInGameEntitiesNames, "AnimationEditor::start: null animatedInGameEntitiesNames");
	ASSERT(VirtualList_getSize(animatedInGameEntitiesNames), "AnimationEditor::start: empty animatedInGameEntitiesNames");

	OptionsSelector_setOptions(this->animatedInGameEntitySelector, animatedInGameEntitiesNames);
	__DELETE(animatedInGameEntitiesNames);

	this->mode = kFirstMode + 1;
	AnimationEditor_setupMode(this);
	SpriteManager_showLayer(SpriteManager_getInstance(), SpriteManager_getFreeLayer(SpriteManager_getInstance()));

	SpriteManager_deferTextureWriting(SpriteManager_getInstance(), false);
	SpriteManager_deferAffineTransformations(SpriteManager_getInstance(), false);
}

/**
 * Hide editor
 *
 * @memberof		AnimationEditor
 * @public
 *
 * @param this		Function scope
 */
void AnimationEditor_hide(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::stop: null this");

	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);

	AnimationEditor_removePreviousSprite(this);

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

	SpriteManager_recoverLayers(SpriteManager_getInstance());
	SpriteManager_deferTextureWriting(SpriteManager_getInstance(), true);
	SpriteManager_deferAffineTransformations(SpriteManager_getInstance(), true);
}

/**
 * Setup editor's current page
 *
 * @memberof		AnimationEditor
 * @private
 *
 * @param this		Function scope
 */
static void AnimationEditor_setupMode(AnimationEditor this)
{
	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	Printing_text(Printing_getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing_text(Printing_getInstance(), " ANIMATION INSPECTOR ", 1, 0, NULL);
	Printing_text(Printing_getInstance(), " Accept \x13  ", 38, 1, NULL);
	Printing_text(Printing_getInstance(), " Cancel \x14  ", 38, 2, NULL);

	switch(this->mode)
	{
		case kSelectActor:

			AnimationEditor_printUserAnimatedInGameEntities(this);
			break;

		case kSelectSprite:

			AnimationEditor_createSpriteSelector(this);
			AnimationEditor_printSprites(this);
			AnimationEditor_createSprite(this);
			Sprite_pause(this->animatedSprite, true);
			break;

		case kSelectAnimation:

			AnimationEditor_createAnimationsSelector(this);
			Sprite_pause(this->animatedSprite, true);
			AnimationEditor_printAnimatedInGameEntityAnimations(this);
			break;

		case kEditAnimation:

			AnimationEditor_loadAnimationFunction(this);
			AnimationEditor_createAnimationEditionSelector(this);
			AnimationEditor_createFrameEditionSelector(this);
			AnimationController_playAnimationFunction(Sprite_getAnimationController(this->animatedSprite), &this->animationFunction);
			__VIRTUAL_CALL(Sprite, writeAnimation, this->animatedSprite);
			Sprite_pause(this->animatedSprite, true);
			Sprite_pause(this->animatedSprite, false);
			AnimationEditor_printAnimationConfig(this);
			break;
	}
}

/**
 * Process messages
 *
 * @memberof			AnimationEditor
 * @public
 *
 * @param this			Function scope
 * @param telegram		Message wrapper
 *
 * @return				Result of message processing
 */
bool AnimationEditor_handleMessage(AnimationEditor this, Telegram telegram)
{
	ASSERT(this, "AnimationEditor::handleMessage: null this");

	if(!this->gameState)
	{
		return false;
	}

	switch(Telegram_getMessage(telegram))
	{
		case kKeyPressed:

			{
				u32 pressedKey = *((u32*)Telegram_getExtraInfo(telegram));

				if(pressedKey & K_B)
				{
					this->mode--;

					if(kFirstMode >= this->mode)
					{
						this->mode = kFirstMode + 1;
					}
					else
					{
						AnimationEditor_setupMode(this);
					}
					break;
				}

				switch(this->mode)
				{
					case kSelectActor:

						AnimationEditor_selectAnimatedInGameEntity(this, pressedKey);
						break;

					case kSelectSprite:

						AnimationEditor_selectSprite(this, pressedKey);
						break;

					case kSelectAnimation:

						AnimationEditor_selectAnimation(this, pressedKey);
						break;

					case kEditAnimation:

						AnimationEditor_editAnimation(this, pressedKey);
						break;
				}
			}
			break;
	}
	return true;
}

/**
 * Select AnimatedInGameEntity to work on
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationEditor_selectAnimatedInGameEntity(AnimationEditor this, u32 pressedKey)
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
		AnimationEditor_setupMode(this);
	}
}

/**
 * Select the Sprite to work on
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationEditor_selectSprite(AnimationEditor this, u32 pressedKey)
{
	int userAnimatedInGameEntitiesCount = 0;
	for(; _userAnimatedInGameEntities[userAnimatedInGameEntitiesCount].animatedInGameEntityDefinition; userAnimatedInGameEntitiesCount++);

	if(pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->spriteSelector);
		AnimationEditor_createSprite(this);
	}
	else if(pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->spriteSelector);
		AnimationEditor_createSprite(this);
	}
	else if(pressedKey & K_A)
	{
		// select the added entity
		this->mode = kSelectAnimation;
		AnimationEditor_setupMode(this);
	}
}

/**
 * Discard previous selected Sprite
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_removePreviousSprite(AnimationEditor this)
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
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationEditor_selectAnimation(AnimationEditor this, u32 pressedKey)
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
		AnimationEditor_setupMode(this);
	}
}

/**
 * Start editing the selected animation
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 * @param pressedKey		User input
 */
static void AnimationEditor_editAnimation(AnimationEditor this, u32 pressedKey)
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

				AnimationEditor_createFrameEditionSelector(this);
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

				AnimationEditor_createFrameEditionSelector(this);

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
					NM_ASSERT(this->animatedSprite, "AnimationEditor::selectAnimation: null animatedSprite");

					Texture texture = Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite));
					NM_ASSERT(texture, "AnimationEditor::selectAnimation: null texture");

					TextureDefinition* textureDefinition = Texture_getTextureDefinition(texture);
					NM_ASSERT(textureDefinition, "AnimationEditor::selectAnimation: null textureDefinition");

					if(++this->animationFunction.frames[selectedFrame] >= textureDefinition->numberOfFrames)
					{
						this->animationFunction.frames[selectedFrame] = textureDefinition->numberOfFrames - 1;
					}
				}
				break;
		}
	}

	AnimationEditor_printAnimationConfig(this);
}

/**
 * Print the list of user AnimatedInGameEntities
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_printUserAnimatedInGameEntities(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "ACTORS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->animatedInGameEntitySelector, 1, 4);
}

/**
 * Print available sprites for the selected AnimatedInGameEntity
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_printSprites(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "SPRITES", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->spriteSelector, 1, 4);
}

/**
 * Print a list of animation for the selected AnimatedInGameEntity
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_printAnimatedInGameEntityAnimations(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "AVAILABLE ANIMATIONS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->animationsSelector, 1, 4);
}

/**
 * Print selected animation' values
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_printAnimationConfig(AnimationEditor this)
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
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_loadAnimationFunction(AnimationEditor this)
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
	this->animationFunction.onAnimationComplete = (EventListener)&AnimationEditor_onAnimationComplete;
}

/**
 * Create a Sprite to work on
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_createSprite(AnimationEditor this)
{
	AnimationEditor_removePreviousSprite(this);

	VBVec3D position = *_screenPosition;

	position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
	position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
	position.z -= 10;

	SpriteDefinition* spriteDefinition = (SpriteDefinition*)_userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->inGameEntityDefinition.entityDefinition.spritesDefinitions[OptionsSelector_getSelectedOption(this->spriteSelector)];

	NM_ASSERT(spriteDefinition, "AnimationEditor::createSprite: null spriteDefinition");

	this->animatedSprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, __SAFE_CAST(Object, this));
	ASSERT(this->animatedSprite, "AnimationEditor::createSprite: null animatedSprite");
	ASSERT(Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite)), "AnimationEditor::createSprite: null texture");

	VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, __SAFE_CAST(Sprite, this->animatedSprite));
	spritePosition.x = ITOFIX19_13((__SCREEN_WIDTH >> 1) - (Texture_getCols(Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite))) << 2));
	spritePosition.y = ITOFIX19_13((__SCREEN_HEIGHT >> 1) - (Texture_getRows(Sprite_getTexture(__SAFE_CAST(Sprite, this->animatedSprite))) << 2));

	__VIRTUAL_CALL(Sprite, setPosition, __SAFE_CAST(Sprite, this->animatedSprite), &spritePosition);
	__VIRTUAL_CALL(Sprite, applyAffineTransformations, __SAFE_CAST(Sprite, this->animatedSprite));
	SpriteManager_showLayer(SpriteManager_getInstance(), __VIRTUAL_CALL(Sprite, getWorldLayer, __SAFE_CAST(Sprite, this->animatedSprite)));
	__VIRTUAL_CALL(Sprite, render, __SAFE_CAST(Sprite, this->animatedSprite));

	// must set the position after showing the sprite, otherwise
	// it will remain non initialized
	__VIRTUAL_CALL(Sprite, setPosition, __SAFE_CAST(Sprite, this->animatedSprite), &spritePosition);
}

/**
 * Create OptionSelector for sprites
 *
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_createSpriteSelector(AnimationEditor this)
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
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_createAnimationsSelector(AnimationEditor this)
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
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_createAnimationEditionSelector(AnimationEditor this)
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
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 */
static void AnimationEditor_createFrameEditionSelector(AnimationEditor this)
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
 * @memberof				AnimationEditor
 * @private
 *
 * @param this				Function scope
 * @param eventFirer		AnimationController
 */
static void AnimationEditor_onAnimationComplete(AnimationEditor this, Object eventFirer __attribute__ ((unused)))
{
	if(!this->animationFunction.loop)
	{
		Printing_text(Printing_getInstance(), " Play     \x13 ", 36, 2, NULL);
	}
}
