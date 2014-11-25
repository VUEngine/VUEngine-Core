/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

#define __ANIMATION_EDITOR

#ifdef __ANIMATION_EDITOR

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#include <AnimationEditor.h>
#include <Game.h>
#include <Optics.h>
#include <Globals.h>
#include <SpriteManager.h>
#include <Level.h>
#include <Stage.h>
#include <Screen.h>
#include <string.h>
#include <OptionsSelector.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __USER_ACTOR_SHOW_ROW 	6

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define AnimationEditor_ATTRIBUTES						\
														\
	/* super's attributes */							\
	Object_ATTRIBUTES;									\
														\
	/* current in game level */							\
	Level level;										\
														\
	/* current animated sprite */						\
	AnimatedSprite animatedSprite;						\
														\
	/* current animation description */					\
	AnimationFunction animationFunction;				\
														\
	/* actors selector */								\
	OptionsSelector actorsSelector;						\
														\
	/* animations selector */							\
	OptionsSelector animationsSelector;					\
														\
	/* mode */											\
	int mode;											\


// define the AnimationEditor
__CLASS_DEFINITION(AnimationEditor);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												  MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __TRANSLATION_STEP	8
#define __SCREEN_X_TRANSLATION_STEP		__SCREEN_WIDTH / 4
#define __SCREEN_Y_TRANSLATION_STEP		__SCREEN_HEIGHT / 4
#define __SCREEN_Z_TRANSLATION_STEP		__SCREEN_HEIGHT / 4


enum Modes {
		kFirstMode = 0,
		kSelectActor,
		kSelectAnimation,
		kEditAnimation,
		kLastMode
};

extern UserActor _userActors[];

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

static void AnimationEditor_constructor(AnimationEditor this);

static void AnimationEditor_setupMode(AnimationEditor this);
static void AnimationEditor_printUserActors(AnimationEditor this);
static void AnimationEditor_printActorAnimations(AnimationEditor this);
static void AnimationEditor_printAnimationConfig(AnimationEditor this);

static void AnimationEditor_selectActor(AnimationEditor this, u16 pressedKey);
static void AnimationEditor_removePreviousAnimatedSprite(AnimationEditor this);
static void AnimationEditor_selectAnimation(AnimationEditor this, u16 pressedKey);
static void AnimationEditor_editAnimation(AnimationEditor this, u16 pressedKey);

static void AnimatorEditor_onAnimationComplete();

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
__SINGLETON(AnimationEditor);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void AnimationEditor_constructor(AnimationEditor this){

	ASSERT(this, "AnimationEditor::constructor: null this");

	__CONSTRUCT_BASE(Object);
	
	
	this->animatedSprite = NULL;
	this->level = NULL;
	this->actorsSelector = NULL;
	this->animationsSelector = NULL;
	this->mode = kFirstMode + 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void AnimationEditor_destructor(AnimationEditor this){
	
	ASSERT(this, "AnimationEditor::destructor: null this");

	if(this->actorsSelector) {
		
		__DELETE(this->actorsSelector);
	}

	if(this->animationsSelector) {
		
		__DELETE(this->animationsSelector);
	}

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update
void AnimationEditor_update(AnimationEditor this){

	if(this->animatedSprite) {

		AnimatedSprite_update(this->animatedSprite, Game_getClock(Game_getInstance()));
		
		// TODO 
		// fix me, I don't belong here
		// but otherwise other layers are shown
		SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer((Sprite)this->animatedSprite));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// start editor 
void AnimationEditor_start(AnimationEditor this){
	
	this->level = Game_getLevel(Game_getInstance());
	this->animatedSprite = NULL;
	
	this->animationsSelector = NULL;
	this->actorsSelector = __NEW(OptionsSelector, __ARGUMENTS(2, 16));
	
	VirtualList actorsNames = __NEW(VirtualList);
	
	int i = 0;
	for(; _userActors[i].actorDefinition; i++) {
	
		ASSERT(_userActors[i].name, "AnimationEditor::start: push null name");
		VirtualList_pushBack(actorsNames, _userActors[i].name);
	}
	
	ASSERT(actorsNames, "AnimationEditor::start: null actorsNames");
	ASSERT(VirtualList_getSize(actorsNames), "AnimationEditor::start: empty actorsNames");

	OptionsSelector_setOptions(this->actorsSelector, actorsNames);
	__DELETE(actorsNames);

	AnimationEditor_setupMode(this);
	SpriteManager_showLayer(SpriteManager_getInstance(), SpriteManager_getFreeLayer(SpriteManager_getInstance()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// hide editor screens
void AnimationEditor_stop(AnimationEditor this){

	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	
	AnimationEditor_removePreviousAnimatedSprite(this);
	
	if(this->actorsSelector) {
		
		__DELETE(this->actorsSelector);
		this->actorsSelector = NULL;
	}

	if(this->animationsSelector) {
		
		__DELETE(this->animationsSelector);
		this->animationsSelector = NULL;
	}
	
	SpriteManager_recoverLayers(SpriteManager_getInstance());
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print title
static void AnimationEditor_setupMode(AnimationEditor this) {
	
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	Printing_text("ANIMATION INSPECTOR", 14, 0);
	Printing_text("Accept (A)", 48 - 10, 0);
	Printing_text("Cancel (B)", 48 - 10, 1);

	switch(this->mode) {
	
		case kSelectActor:

			AnimationEditor_removePreviousAnimatedSprite(this);
			AnimationEditor_printUserActors(this);
			break;

		case kSelectAnimation:

			AnimatedSprite_pause(this->animatedSprite, true);
			AnimationEditor_printActorAnimations(this);
			SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer((Sprite)this->animatedSprite));
			break;
			
		case kEditAnimation:

			AnimatedSprite_playAnimationFunction(this->animatedSprite, &this->animationFunction);			AnimatedSprite_pause(this->animatedSprite, true);
			AnimationEditor_printAnimationConfig(this);
			SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer((Sprite)this->animatedSprite));
			AnimatedSprite_pause(this->animatedSprite, false);
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int AnimationEditor_handleMessage(AnimationEditor this, Telegram telegram){
	
	switch(Telegram_getMessage(telegram)){
	
		case kKeyPressed:	
			{
				u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

				if(pressedKey & K_B){
					
					this->mode--;
					
					if(kFirstMode >= this->mode) {
						
						this->mode = kFirstMode + 1;
					}
					else {
					
						AnimationEditor_setupMode(this);
					}				
					break;
				}
				
				switch(this->mode) {
				
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimationEditor_selectActor(AnimationEditor this, u16 pressedKey){
	
	int userActorsCount = 0;
	for(; _userActors[userActorsCount].actorDefinition; userActorsCount++);

	if(pressedKey & K_LU){
	
		OptionsSelector_selectPrevious(this->actorsSelector);
	}
	else if(pressedKey & K_LD){
		
		OptionsSelector_selectNext(this->actorsSelector);
	}
	else if(pressedKey & K_A) {
		
		VBVec3D position = Screen_getPosition(Screen_getInstance());
		
		position.x += ITOFIX19_13(__SCREEN_WIDTH >> 1);
		position.y += ITOFIX19_13(__SCREEN_HEIGHT >> 1);
		position.z += 0;
		
		AnimationEditor_removePreviousAnimatedSprite(this);
		
		this->animatedSprite = __NEW(AnimatedSprite, __ARGUMENTS((void*)this, (SpriteDefinition*)&_userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->inGameEntityDefinition.entityDefinition.spritesDefinitions[0]));	

		Sprite_setPosition((Sprite)this->animatedSprite, &position);
		SpriteManager_sortAllLayers(SpriteManager_getInstance());
		SpriteManager_render(SpriteManager_getInstance());

		SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer((Sprite)this->animatedSprite));

		AnimationDescription* animationDescription = _userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->animationDescription;

		if(animationDescription) {
			
			if(this->animationsSelector) {
				
				__DELETE(this->animationsSelector);
			}
			
			this->animationsSelector = __NEW(OptionsSelector, __ARGUMENTS(2, 16));
			
			VirtualList animationsNames = __NEW(VirtualList);
			
			int i = 0;
			for(i = 0; animationDescription->animationFunctions[i]; i++) {
			
				VirtualList_pushBack(animationsNames, animationDescription->animationFunctions[i]->name);
			}
			
			OptionsSelector_setOptions(this->animationsSelector, animationsNames);
			__DELETE(animationsNames);
		}
		else {
			
			//TODO
		}


		Level_transform(this->level);
		__VIRTUAL_CALL(void, Container, setLocalPosition, (Container)this->animatedSprite, __ARGUMENTS(position));
					
		// select the added entity
		this->mode = kSelectAnimation;
		AnimationEditor_setupMode(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimationEditor_removePreviousAnimatedSprite(AnimationEditor this){
	
	if(this->animatedSprite) {
		
		__DELETE(this->animatedSprite);
		this->animatedSprite = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimationEditor_selectAnimation(AnimationEditor this, u16 pressedKey){

	AnimationDescription* animationDescription = _userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->animationDescription;

	int actorAnimationsCount = 0;
	for(; animationDescription->animationFunctions[actorAnimationsCount]; actorAnimationsCount++);
	
	if(pressedKey & K_LU){
	
		OptionsSelector_selectPrevious(this->animationsSelector);
	}
	else if(pressedKey & K_LD){
		
		OptionsSelector_selectNext(this->animationsSelector);
	}
	else if(pressedKey & K_A) {
		
		AnimationDescription* animationDescription = _userActors[OptionsSelector_getSelectedOption(this->actorsSelector)].actorDefinition->animationDescription;

		AnimationFunction* animationFunction = animationDescription->animationFunctions[OptionsSelector_getSelectedOption(this->animationsSelector)];

		int i = 0;
		for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION; i++){
			
			this->animationFunction.frames[i] = animationFunction->frames[i];
		}
		
		strcpy(this->animationFunction.name, animationFunction->name);
		this->animationFunction.numberOfFrames = animationFunction->numberOfFrames;
		this->animationFunction.delay = animationFunction->delay;
		this->animationFunction.loop = animationFunction->loop;
		this->animationFunction.onAnimationComplete = &AnimatorEditor_onAnimationComplete;

		// select the added entity
		this->mode = kEditAnimation;
		AnimationEditor_setupMode(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimationEditor_editAnimation(AnimationEditor this, u16 pressedKey){
	
	if(pressedKey & K_A) {
		
		if(AnimatedSprite_isPlaying(this->animatedSprite)) {

			Printing_text("Play   (A)", 48 - 10, 2);
			AnimatedSprite_pause(this->animatedSprite, true);
		}
		else {
			
			Printing_text("Pause  (A)", 48 - 10, 2);
			AnimatedSprite_pause(this->animatedSprite, false);
		}
	}
	else if((pressedKey & K_RT)) {

		this->animationFunction.loop = !this->animationFunction.loop;
		AnimationEditor_printAnimationConfig(this);
	}
	else if(pressedKey & K_RL) {

		this->animationFunction.delay -= 1 * __FPS_ANIM_FACTOR;
		if(0 == this->animationFunction.delay) {
			
			this->animationFunction.delay = 0;
		}

		AnimationEditor_printAnimationConfig(this);
	}
	else if(pressedKey & K_RR) {

		this->animationFunction.delay += 1 * __FPS_ANIM_FACTOR;
		if(1000 <= this->animationFunction.delay) {
			
			this->animationFunction.delay = 1000;
		}
		
		AnimationEditor_printAnimationConfig(this);
		
		AnimatedSprite_playAnimationFunction(this->animatedSprite, &this->animationFunction);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimationEditor_printUserActors(AnimationEditor this){

	Printing_text("User's actors  ", 1, 2);
	Printing_text("                       ", 1, 3);
	OptionsSelector_showOptions(this->actorsSelector, 1, 4);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimationEditor_printActorAnimations(AnimationEditor this){

	Printing_text("Actor's animations ", 1, 2);
	Printing_text("                       ", 1, 3);
	OptionsSelector_showOptions(this->animationsSelector, 1, 4);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimationEditor_printAnimationConfig(AnimationEditor this){

	Printing_text("           ", 38, 0);
	
	Printing_text("Play   (A)", 48 - 10, 2);

	int x = 1;
	int y = 2;
	Printing_text("Animation: ", x, y);
	Printing_text(this->animationFunction.name, x + 12, y++);

	Printing_text("Frames:                    ", x, ++y);
	Printing_int(this->animationFunction.numberOfFrames, x + 12, y);

	Printing_text("Delay:                     ", x, ++y);
	Printing_int(this->animationFunction.delay, x + 12, y);
	Printing_text("(RL/RR)", 48 - 7, y);

	Printing_text("Loop:                      ", x, ++y);
	Printing_text(this->animationFunction.loop? "true": "false", x + 12, y);
	Printing_text("Toogle (RT)", 48 - 11, y);

	Printing_text("Frames:                    ", x, ++y);

	int i = 0;
	int j = 0;
	for(; i < __MAX_FRAMES_PER_ANIMATION_FUNCTION && i < this->animationFunction.numberOfFrames; i++){
		
		Printing_int(this->animationFunction.frames[i], x + 12 + j, y);
		j += Utilities_getDigitCount(this->animationFunction.frames[i]) + 1;
	}

//	void* onAnimationComplete;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AnimatorEditor_onAnimationComplete() {
	
//	Printing_text("Animation complete", 1, 12);
	Printing_text("Play   (A)", 48 - 10, 2);
}

#endif 