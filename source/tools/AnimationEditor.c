/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __USER_ACTOR_SHOW_ROW 	6
#define __OPTION_MARK	"\x0B"
#define __FRAME_OPTION_MARK	"*"

#define __TRANSLATION_STEP	8
#define __SCREEN_X_TRANSLATION_STEP		__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP		__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP		__SCREEN_HEIGHT / 4

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define AnimationEditor_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* current in game gameState */												\
	GameState gameState;														\
																				\
	/* current animated sprite */												\
	AnimatedSprite animatedSprite;												\
																				\
	/* current animation description */											\
	AnimationDescription* animationDescription;									\
																				\
	/* current animation function */											\
	AnimationFunction animationFunction;										\
																				\
	/* actors selector */														\
	OptionsSelector actorsSelector;												\
																				\
	/* animations selector */													\
	OptionsSelector animationsSelector;											\
																				\
	/* animation edition selector */											\
	OptionsSelector animationEditionSelector;									\
																				\
	/* frame edition selector */												\
	OptionsSelector frameEditionSelector;										\
																				\
	/* mode */																	\
	int mode;																	\

// define the AnimationEditor
__CLASS_DEFINITION(AnimationEditor);

enum Modes
{
	kFirstMode = 0,
	kSelectActor,
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

extern UserActor _userActors[];

static void AnimationEditor_constructor(AnimationEditor this);
static void AnimationEditor_setupMode(AnimationEditor this);
static void AnimationEditor_printUserActors(AnimationEditor this);
static void AnimationEditor_printActorAnimations(AnimationEditor this);
static void AnimationEditor_printAnimationConfig(AnimationEditor this);
static void AnimationEditor_selectActor(AnimationEditor this, u16 pressedKey);
static void AnimationEditor_removePreviousAnimatedSprite(AnimationEditor this);
static void AnimationEditor_selectAnimation(AnimationEditor this, u16 pressedKey);
static void AnimationEditor_editAnimation(AnimationEditor this, u16 pressedKey);
static void AnimationEditor_loadAnimationFunction(AnimationEditor this);
static void AnimationEditor_createAnimatedSprite(AnimationEditor this);
static void AnimationEditor_createAnimationsSelector(AnimationEditor this);
static void AnimationEditor_createAnimationEditionSelector(AnimationEditor this);
static void AnimationEditor_createFrameEditionSelector(AnimationEditor this);
static void AnimationEditor_onAnimationComplete(AnimationEditor this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(AnimationEditor);

// class's constructor
static void AnimationEditor_constructor(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->animatedSprite = NULL;
	this->gameState = NULL;
	this->actorsSelector = NULL;
	this->animationsSelector = NULL;
	this->animationEditionSelector = NULL;
	this->frameEditionSelector = NULL;

	this->mode = kFirstMode + 1;
}

// class's destructor
void AnimationEditor_destructor(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::destructor: null this");

	if (this->actorsSelector)
	{
		__DELETE(this->actorsSelector);
	}

	if (this->animationsSelector)
	{
		__DELETE(this->animationsSelector);
	}

	if (this->animationEditionSelector)
	{
		__DELETE(this->animationEditionSelector);
	}

	if (this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
	}

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// update
void AnimationEditor_update(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::update: null this");

	if (this->gameState && this->animatedSprite)
	{
		AnimatedSprite_update(this->animatedSprite, Game_getClock(Game_getInstance()));

		// TODO
		// fix me, I don't belong here
		// but otherwise other layers are shown
		SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer((Sprite)this->animatedSprite));
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

	this->actorsSelector = __NEW(OptionsSelector, __ARGUMENTS(2, 16, __OPTION_MARK, kString));

	VirtualList actorsNames = __NEW(VirtualList);

	int i = 0;
	for (; _userActors[i].actorDefinition; i++)
	{
		ASSERT(_userActors[i].name, "AnimationEditor::start: push null name");
		VirtualList_pushBack(actorsNames, _userActors[i].name);
	}

	ASSERT(actorsNames, "AnimationEditor::start: null actorsNames");
	ASSERT(VirtualList_getSize(actorsNames), "AnimationEditor::start: empty actorsNames");

	OptionsSelector_setOptions(this->actorsSelector, actorsNames);
	__DELETE(actorsNames);

	this->mode = kFirstMode + 1;
	AnimationEditor_setupMode(this);
	SpriteManager_showLayer(SpriteManager_getInstance(), SpriteManager_getFreeLayer(SpriteManager_getInstance()));
}

// hide editor screens
void AnimationEditor_stop(AnimationEditor this)
{
	ASSERT(this, "AnimationEditor::stop: null this");

	VPUManager_clearBgmap(VPUManager_getInstance(), TextureManager_getPrintingBgmapSegment(TextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);

	AnimationEditor_removePreviousAnimatedSprite(this);

	if (this->actorsSelector)
	{
		__DELETE(this->actorsSelector);
		this->actorsSelector = NULL;
	}

	if (this->animationsSelector)
	{
		__DELETE(this->animationsSelector);
		this->animationsSelector = NULL;
	}

	if (this->animationEditionSelector)
	{
		__DELETE(this->animationEditionSelector);
		this->animationEditionSelector = NULL;
	}

	if (this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
		this->frameEditionSelector = NULL;
	}

	SpriteManager_recoverLayers(SpriteManager_getInstance());
}


// print title
static void AnimationEditor_setupMode(AnimationEditor this)
{
	VPUManager_clearBgmap(VPUManager_getInstance(), TextureManager_getPrintingBgmapSegment(TextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	Printing_text(Printing_getInstance(), "\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07", 0, 0, NULL);
	Printing_text(Printing_getInstance(), " ANIMATION INSPECTOR ", 1, 0, NULL);
	Printing_text(Printing_getInstance(), " Accept \x13  ", 38, 1, NULL);
	Printing_text(Printing_getInstance(), " Cancel \x14  ", 38, 2, NULL);

	switch (this->mode)
	{
		case kSelectActor:

			AnimationEditor_printUserActors(this);
			break;

		case kSelectAnimation:

			AnimationEditor_createAnimatedSprite(this);
			AnimationEditor_createAnimationsSelector(this);
			AnimatedSprite_pause(this->animatedSprite, true);
			AnimationEditor_printActorAnimations(this);
			break;

		case kEditAnimation:

			AnimationEditor_loadAnimationFunction(this);
			AnimationEditor_createAnimationEditionSelector(this);
			AnimationEditor_createFrameEditionSelector(this);
			AnimatedSprite_playAnimationFunction(this->animatedSprite, &this->animationFunction);
			AnimatedSprite_pause(this->animatedSprite, true);
			AnimatedSprite_pause(this->animatedSprite, false);
			AnimationEditor_printAnimationConfig(this);
			break;
	}
}

// process a telegram
bool AnimationEditor_handleMessage(AnimationEditor this, Telegram telegram)
{
	ASSERT(this, "AnimationEditor::handleMessage: null this");

	if (!this->gameState)
	{
		return false;
	}

	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:

			{
				u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

				if (pressedKey & K_B)
				{
					this->mode--;

					if (kFirstMode >= this->mode)
					{
						this->mode = kFirstMode + 1;
					}
					else
					{
						AnimationEditor_setupMode(this);
					}
					break;
				}

				switch (this->mode)
				{
					case kSelectActor:

						AnimationEditor_selectActor(this, pressedKey);
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

static void AnimationEditor_selectActor(AnimationEditor this, u16 pressedKey)
{
	int userActorsCount = 0;
	for (; _userActors[userActorsCount].actorDefinition; userActorsCount++);

	if (pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->actorsSelector);
	}
	else if (pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->actorsSelector);
	}
	else if (pressedKey & K_A)
	{
		// select the added entity
		this->mode = kSelectAnimation;
		AnimationEditor_setupMode(this);
	}
}

static void AnimationEditor_removePreviousAnimatedSprite(AnimationEditor this)
{
	if (this->animatedSprite)
	{
		__DELETE(this->animatedSprite);
		this->animatedSprite = NULL;
	}
}

static void AnimationEditor_selectAnimation(AnimationEditor this, u16 pressedKey)
{
	this->animationDescription = _userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->animationDescription;

	int actorAnimationsCount = 0;
	for (; this->animationDescription->animationFunctions[actorAnimationsCount]; actorAnimationsCount++);

	if (pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->animationsSelector);
	}
	else if (pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->animationsSelector);
	}
	else if (pressedKey & K_A)
	{
		this->mode = kEditAnimation;
		AnimationEditor_setupMode(this);
	}
}

static void AnimationEditor_editAnimation(AnimationEditor this, u16 pressedKey)
{
	if (pressedKey & K_A)
	{
		if (AnimatedSprite_isPlaying(this->animatedSprite))
		{
			AnimatedSprite_pause(this->animatedSprite, true);

		}
		else
		{
			AnimatedSprite_pause(this->animatedSprite, false);
		}
	}
	else if ((pressedKey & K_LU))
	{
		OptionsSelector_selectPrevious(this->animationEditionSelector);
	}
	else if ((pressedKey & K_LD))
	{
		OptionsSelector_selectNext(this->animationEditionSelector);
	}
	else if ((pressedKey & K_LL))
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch (selectedProperty)
		{
			case kNumberOfFrames:

				if (0 >= --this->animationFunction.numberOfFrames)
				{
					this->animationFunction.numberOfFrames = 1;
				}

				AnimationEditor_createFrameEditionSelector(this);
				break;

			case kDelay:

				this->animationFunction.delay -= 1 * __FPS_ANIM_FACTOR;

				if (0 > this->animationFunction.delay)
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
	else if (pressedKey & K_LR)
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch (selectedProperty)
		{
			case kNumberOfFrames:

				if (__MAX_FRAMES_PER_ANIMATION_FUNCTION < ++this->animationFunction.numberOfFrames)
				{
					this->animationFunction.numberOfFrames = __MAX_FRAMES_PER_ANIMATION_FUNCTION;
				}

				AnimationEditor_createFrameEditionSelector(this);

				break;

			case kDelay:

				this->animationFunction.delay += 1 * __FPS_ANIM_FACTOR;

				if (1000 < this->animationFunction.delay)
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
	else if (pressedKey & K_RU)
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch (selectedProperty)
		{
			case kFrames:

				OptionsSelector_selectPrevious(this->frameEditionSelector);
				break;
		}
	}
	else if (pressedKey & K_RD)
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		switch (selectedProperty)
		{
			case kFrames:

				OptionsSelector_selectNext(this->frameEditionSelector);
				break;
		}
	}
	else if (pressedKey & K_RL)
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		int selectedFrame = OptionsSelector_getSelectedOption(this->frameEditionSelector);

		switch (selectedProperty)
		{
			case kFrames:

				if (0 > --this->animationFunction.frames[selectedFrame])
				{
					this->animationFunction.frames[selectedFrame] = 0;
				}

				break;
		}
	}
	else if (pressedKey & K_RR)
	{
		int selectedProperty = OptionsSelector_getSelectedOption(this->animationEditionSelector);
		int selectedFrame = OptionsSelector_getSelectedOption(this->frameEditionSelector);

		switch (selectedProperty)
		{
			case kFrames:

				if (this->animationDescription->numberOfFrames < ++this->animationFunction.frames[selectedFrame])
				{
					this->animationFunction.frames[selectedFrame] = this->animationDescription->numberOfFrames;
				}

				break;
		}
	}

	AnimationEditor_printAnimationConfig(this);
}


static void AnimationEditor_printUserActors(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "User's actors  ", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_showOptions(this->actorsSelector, 1, 4);
}

static void AnimationEditor_printActorAnimations(AnimationEditor this)
{
	Printing_text(Printing_getInstance(), "Actor's animations ", 1, 2, NULL);
	Printing_text(Printing_getInstance(), "                       ", 1, 3, NULL);
	OptionsSelector_showOptions(this->animationsSelector, 1, 4);
}

static void AnimationEditor_printAnimationConfig(AnimationEditor this)
{
	int x = 1;
	int y = 2;

	Printing_text(Printing_getInstance(), "Animation: ", x, y, NULL);
	Printing_text(Printing_getInstance(), this->animationFunction.name, x + 12, y++, NULL);
	OptionsSelector_showOptions(this->animationEditionSelector, x, ++y);

	Printing_int(Printing_getInstance(), this->animationFunction.numberOfFrames, x + 20, y++, NULL);
	Printing_int(Printing_getInstance(), this->animationFunction.delay, x + 20, y++, NULL);
	Printing_text(Printing_getInstance(), this->animationFunction.loop ? "true" : "false", x + 20, y++, NULL);

	OptionsSelector_showOptions(this->frameEditionSelector, x, ++y + 1);

	Printing_text(Printing_getInstance(), " Cancel   \x14 ", 36, 1, NULL);
	if (!AnimatedSprite_isPlaying(this->animatedSprite))
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

	switch (selectedProperty)
	{
		case kFrames:

			Printing_text(Printing_getInstance(), " Select \x1F\x1A\x1B", 36, 6, NULL);
			Printing_text(Printing_getInstance(), " Modify \x1F\x1C\x1D", 36, 7, NULL);
			break;
	}
}

static void AnimationEditor_loadAnimationFunction(AnimationEditor this)
{
	this->animationDescription = _userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->animationDescription;

	AnimationFunction* animationFunction = this->animationDescription->animationFunctions[OptionsSelector_getSelectedOption(this->animationsSelector)];

	int i = 0;
	for (; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION; i++)
	{
		this->animationFunction.frames[i] = animationFunction->frames[i];
	}

	strcpy(this->animationFunction.name, animationFunction->name);
	this->animationFunction.numberOfFrames = animationFunction->numberOfFrames;
	this->animationFunction.delay = animationFunction->delay;
	this->animationFunction.loop = animationFunction->loop;
	this->animationFunction.onAnimationComplete = &AnimationEditor_onAnimationComplete;
}

static void AnimationEditor_createAnimatedSprite(AnimationEditor this)
{
	AnimationEditor_removePreviousAnimatedSprite(this);

	VBVec3D position = Screen_getPosition(Screen_getInstance());

	position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
	position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
	position.z += 0;

	this->animatedSprite = __NEW(AnimatedSprite, __ARGUMENTS((SpriteDefinition*)&_userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->inGameEntityDefinition.entityDefinition.spritesDefinitions[0]), (void*)this);

	Sprite_setPosition((Sprite)this->animatedSprite, position);
	SpriteManager_render(SpriteManager_getInstance());

	SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer((Sprite)this->animatedSprite));
}

static void AnimationEditor_createAnimationsSelector(AnimationEditor this)
{
	this->animationDescription = _userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->animationDescription;

	if (this->animationDescription)
	{
		if (this->animationsSelector)
		{
			__DELETE(this->animationsSelector);
		}

		this->animationsSelector = __NEW(OptionsSelector, __ARGUMENTS(2, 16, __OPTION_MARK, kString));

		VirtualList animationsNames = __NEW(VirtualList);

		int i = 0;
		for (i = 0; this->animationDescription->animationFunctions[i]; i++)
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
	if (this->animationEditionSelector)
	{
		__DELETE(this->animationEditionSelector);
	}

	this->animationEditionSelector = __NEW(OptionsSelector, __ARGUMENTS(1, 4, __OPTION_MARK, kString));

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
	if (this->frameEditionSelector)
	{
		__DELETE(this->frameEditionSelector);
	}

	this->frameEditionSelector = __NEW(OptionsSelector, __ARGUMENTS((__SCREEN_WIDTH >> 3) / 3, __MAX_FRAMES_PER_ANIMATION_FUNCTION / 2, __FRAME_OPTION_MARK, kInt));

	VirtualList framesIndexes = __NEW(VirtualList);

	int i = 0;
	for (; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION && i < this->animationFunction.numberOfFrames; i++)
	{
		VirtualList_pushBack(framesIndexes, &this->animationFunction.frames[i]);
	}

	OptionsSelector_setOptions(this->frameEditionSelector, framesIndexes);
	__DELETE(framesIndexes);
}

static void AnimationEditor_onAnimationComplete(AnimationEditor this)
{
	if (!this->animationFunction.loop)
	{
		Printing_text(Printing_getInstance(), " Play     \x13 ", 36, 2, NULL);
	}
}


#endif