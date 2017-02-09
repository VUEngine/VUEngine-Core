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

#ifdef __ANIMATION_EDITOR


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimationEditorState.h>
#include <AnimationEditor.h>
#include <Game.h>
#include <MessageDispatcher.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimationEditorState_destructor(AnimationEditorState this);
static void AnimationEditorState_constructor(AnimationEditorState this);
static void AnimationEditorState_enter(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static void AnimationEditorState_execute(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static void AnimationEditorState_exit(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static bool AnimationEditorState_processMessage(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)), Telegram telegram);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define AnimationEditorState_ATTRIBUTES																	\
		/* inherits */																					\
		GameState_ATTRIBUTES																			\

/**
 * @class	AnimationEditorState
 * @extends GameState
 */
__CLASS_DEFINITION(AnimationEditorState, GameState);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(AnimationEditorState);

// class's constructor
static void __attribute__ ((noinline)) AnimationEditorState_constructor(AnimationEditorState this)
{
	__CONSTRUCT_BASE(GameState);
}

// class's destructor
static void AnimationEditorState_destructor(AnimationEditorState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void AnimationEditorState_enter(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	GameState_pauseClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
	AnimationEditor_start(AnimationEditor_getInstance(), __SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

// state's execute
static void AnimationEditorState_execute(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	AnimationEditor_update(AnimationEditor_getInstance());
}

// state's exit
static void AnimationEditorState_exit(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	AnimationEditor_stop(AnimationEditor_getInstance());
	GameState_resumeClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

// state's on message
static bool AnimationEditorState_processMessage(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)), Telegram telegram)
{
	// process message
	switch(Telegram_getMessage(telegram))
	{
		case kKeyPressed:
			{
				MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, AnimationEditor_getInstance()), kKeyPressed, ((u16*)Telegram_getExtraInfo(telegram)));
			}
			break;
	}

	return true;
}


#endif
