/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifdef __ANIMATION_EDITOR


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
// 											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __USER_ACTOR_SHOW_ROW 	6
#define __OPTION_MARK	        "\x0B"
#define __FRAME_OPTION_MARK	    "\x0B"

#define __TRANSLATION_STEP	    8
#define __SCREEN_X_TRANSLATION_STEP		__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP		__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP		__SCREEN_HEIGHT / 4


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define AnimationEditor_ATTRIBUTES																		\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* current in game gameState */																	\
        GameState gameState;																			\
        /* current animated sprite */																	\
        Sprite animatedSprite;																			\
        /* current animation description */																\
        AnimationDescription* animationDescription;														\
        /* current animation function */																\
        AnimationFunction animationFunction;															\
        /* animated in game entity selector */															\
        OptionsSelector animatedInGameEntitySelector;													\
        /* animated sprite selector */																	\
        OptionsSelector spriteSelector;																	\
        /* animations selector */																		\
        OptionsSelector animationsSelector;																\
        /* animation edition selector */																\
        OptionsSelector animationEditionSelector;														\
        /* frame edition selector */																	\
        OptionsSelector frameEditionSelector;															\
        /* mode */																						\
        int mode;																						\

// define the AnimationEditor
__CLASS_DEFINITION(AnimationEditor, Object);

enum Modes
{
	kFirstMode = 0,
	kSelectActor,
	kSelectSprite,
	kSelectAnimation,
	kEditAnimation,
	kLastMode
};

enum AnimationProperties
{
	kNumberOfFrames = 0,
	kDelay,
	kLoop,
	kFrames
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern UserAnimatedInGameEntity _userAnimatedInGameEntities[];
AnimationController Sprite_getAnimationController(Sprite this);

static void AnimationEditor_constructor(AnimationEditor this);
static void AnimationEditor_setupMode(AnimationEditor this);
static void AnimationEditor_printUserAnimatedInGameEntities(AnimationEditor this);
static void AnimationEditor_printSprites(AnimationEditor this);
static void AnimationEditor_printActorAnimations(AnimationEditor this);
static void AnimationEditor_printAnimationConfig(AnimationEditor this);
static void AnimationEditor_selectActor(AnimationEditor this, u32 pressedKey);
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
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(AnimationEditor);

// class's constructor
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

// class's destructor
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

// update
void AnimationEditor_update(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::update: null this");

	if(this->gameState && this->animatedSprite)
	{
        Sprite_animate(this->animatedSprite);
		Sprite_update(this->animatedSprite);
    	__VIRTUAL_CALL(Sprite, applyAffineTransformations, this->animatedSprite);
    	__VIRTUAL_CALL(Sprite, applyHbiasTransformations, this->animatedSprite);
	}
}

// start editor
void AnimationEditor_start(AnimationEditor this, GameState gameState)
{
	ASSERT(this, "AnimationEditor::start: null this");
	ASSERT(gameState, "AnimationEditor::start: null gameState");

	this->gameState = gameState;
	this->animatedSprite = NULL;

	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->animatedInGameEntitySelector = __NEW(OptionsSelector, 2, 16, __OPTION_MARK, kString, NULL);

	VirtualList animatedInGameEntitiesNames = __NEW(VirtualList);

	int i = 0;
	for(; _userAnimatedInGameEntities[i].animatedInGameEntityDefinition; i++)
	{
		ASSERT(_userAnimatedInGameEntities[i].name, "AnimationEditor::start: push null name");
		VirtualList_pushBack(animatedInGameEntitiesNames, _userAnimatedInGameEntities[i].name);
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

// hide editor screens
void AnimationEditor_stop(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::stop: null this");

	VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);

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

// print title
static void AnimationEditor_setupMode(AnimationEditor this)
{
	VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
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
			AnimationEditor_printActorAnimations(this);
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

// process a telegram
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

						AnimationEditor_selectActor(this, pressedKey);
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

static void AnimationEditor_selectActor(AnimationEditor this, u32 pressedKey)
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

static void AnimationEditor_removePreviousSprite(AnimationEditor this)
{
	if(this->animatedSprite)
	{
		__DELETE(this->animatedSprite);
		this->animatedSprite = NULL;
	}
}

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

static void AnimationEditor_printUserAnimatedInGameEntities(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "ACTORS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->animatedInGameEntitySelector, 1, 4);
}

static void AnimationEditor_printSprites(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "SPRITES", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->spriteSelector, 1, 4);
}

static void AnimationEditor_printActorAnimations(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "ACTOR'S ANIMATIONS", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_printOptions(this->animationsSelector, 1, 4);
}

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
extern const VBVec3D* _screenPosition;

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

static void AnimationEditor_createSpriteSelector(AnimationEditor this)
{
	if(this->spriteSelector)
	{
		__DELETE(this->spriteSelector);
	}

	this->spriteSelector = __NEW(OptionsSelector, (__SCREEN_WIDTH >> 3) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION >> 1, __FRAME_OPTION_MARK, kCount, NULL);

	VirtualList spriteIndexes = __NEW(VirtualList);

	int i = 0;
	while(_userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->inGameEntityDefinition.entityDefinition.spritesDefinitions[i])
	{
		VirtualList_pushBack(spriteIndexes, _userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->inGameEntityDefinition.entityDefinition.spritesDefinitions[i]);
		i++;
	}

	OptionsSelector_setOptions(this->spriteSelector, spriteIndexes);
	__DELETE(spriteIndexes);
}

static void AnimationEditor_createAnimationsSelector(AnimationEditor this)
{
	this->animationDescription = _userAnimatedInGameEntities[OptionsSelector_getSelectedOption(this->animatedInGameEntitySelector)].animatedInGameEntityDefinition->animationDescription;

	if(this->animationDescription)
	{
		if(this->animationsSelector)
		{
			__DELETE(this->animationsSelector);
		}

		this->animationsSelector = __NEW(OptionsSelector, 2, 16, __OPTION_MARK, kString, NULL);

		VirtualList animationsNames = __NEW(VirtualList);

		int i = 0;
		for(i = 0; this->animationDescription->animationFunctions[i]; i++)
		{
			VirtualList_pushBack(animationsNames, this->animationDescription->animationFunctions[i]->name);
		}

		OptionsSelector_setOptions(this->animationsSelector, animationsNames);
		__DELETE(animationsNames);
	}
	else
	{
		//TODO
	}
}

static void AnimationEditor_createAnimationEditionSelector(AnimationEditor this)
{
	if(this->animationEditionSelector)
	{
		__DELETE(this->animationEditionSelector);
	}

	this->animationEditionSelector = __NEW(OptionsSelector, 1, 4, __OPTION_MARK, kString, NULL);

	VirtualList optionsNames = __NEW(VirtualList);

	VirtualList_pushBack(optionsNames, "Number of frames:");
	VirtualList_pushBack(optionsNames, "Cycle delay:");
	VirtualList_pushBack(optionsNames, "Loop:");
	VirtualList_pushBack(optionsNames, "Frames:");

	OptionsSelector_setOptions(this->animationEditionSelector, optionsNames);
	__DELETE(optionsNames);

	this->mode = kEditAnimation;
}

static void AnimationEditor_createFrameEditionSelector(AnimationEditor this)
{
	if(this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
	}

	this->frameEditionSelector = __NEW(OptionsSelector, (__SCREEN_WIDTH >> 3) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION >> 1, __FRAME_OPTION_MARK, kInt, NULL);

	VirtualList framesIndexes = __NEW(VirtualList);

	int i = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION && i < this->animationFunction.numberOfFrames; i++)
	{
		VirtualList_pushBack(framesIndexes, &this->animationFunction.frames[i]);
	}

	OptionsSelector_setOptions(this->frameEditionSelector, framesIndexes);
	__DELETE(framesIndexes);
}

static void AnimationEditor_onAnimationComplete(AnimationEditor this, Object eventFirer __attribute__ ((unused)))
{
	if(!this->animationFunction.loop)
	{
		Printing_text(Printing_getInstance(), " Play     \x13 ", 36, 2, NULL);
	}
}


#endif
