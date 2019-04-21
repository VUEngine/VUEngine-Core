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

#include <string.h>
#include <Game.h>
#include <SRAMManager.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <MiscStructs.h>
#include <FrameRate.h>
#include <Clock.h>
#include <BgmapTextureManager.h>
#include <WireframeManager.h>
#include <GameState.h>
#include <Utilities.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <AnimationCoordinatorFactory.h>
#include <StateMachine.h>
#include <Camera.h>
#include <CameraMovementManager.h>
#include <KeypadManager.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VIPManager.h>
#include <I18n.h>
#include <debugConfig.h>

#ifdef __DEBUG_TOOLS
#include <DebugState.h>
#endif
#ifdef __STAGE_EDITOR
#include <StageEditorState.h>
#endif
#ifdef __ANIMATION_INSPECTOR
#include <AnimationInspectorState.h>
#endif


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#ifdef __PROFILE_GAME
#ifndef __REGISTER_LAST_PROCESS_NAME
#define __REGISTER_LAST_PROCESS_NAME
#endif
#endif

enum StateOperations
{
	kSwapState = 0,
	kCleanAndSwapState,
	kPushState,
	kPopState
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#ifdef __PROFILE_GAME

bool _updateProfiling = false;

static char* _processNameDuringFRAMESTART = NULL;
static char* _processNameDuringXPEND = NULL;

static s16 _gameFrameRealDuration = 0;
static s16 _gameFrameDurationAverage = 0;
static s16 _gameFrameEffectiveDuration = 0;
static s16 _gameFrameEffectiveDurationAverage = 0;
static s16 _gameFrameHighestEffectiveDuration = 0;
s16 _tornGameFrameCount = 0;

static s16 _waitForFrameStartTotalTime = 0;
s16 _renderingTotalTime = 0;
static s16 _synchronizeGraphicsTotalTime = 0;
static s16 _updateLogicTotalTime = 0;
static s16 _updatePhysicsTotalTime = 0;
static s16 _updateTransformationsTotalTime = 0;
static s16 _streamingTotalTime = 0;
static s16 _processUserInputTotalTime = 0;
static s16 _dispatchDelayedMessageTotalTime = 0;

s16 _renderingHighestTime = 0;
static s16 _synchronizeGraphicsHighestTime = 0;
static s16 _updateLogicHighestTime = 0;
static s16 _streamingHighestTime = 0;
static s16 _updatePhysicsHighestTime = 0;
static s16 _updateTransformationsHighestTime = 0;
static s16 _processUserInputHighestTime = 0;
static s16 _dispatchDelayedMessageHighestTime = 0;
static s16 _processingCollisionsTotalTime = 0;
static s16 _processingCollisionsHighestTime = 0;

s16 _renderingProcessTimeHelper = 0;
s16 _renderingProcessTime = 0;
static s16 _synchronizeGraphicsProcessTime = 0;
static s16 _updateLogicProcessTime = 0;
static s16 _streamingProcessTime = 0;
static s16 _updatePhysicsProcessTime = 0;
static s16 _updateTransformationsProcessTime = 0;
static s16 _processUserInputProcessTime = 0;
static s16 _dispatchDelayedMessageProcessTime = 0;
static s16 _processingCollisionsProcessTime = 0;

static s16 _previousGameFrameRealDuration = 0;
static s16 _previousGameFrameDurationAverage = 0;
static s16 _previousGameFrameEffectiveDuration = 0;
static s16 _previousGameFrameEffectiveDurationAverage = 0;
static s16 _previousGameFrameHighestEffectiveDuration = 0;
static s16 _previousTornGameFrameCount = 0;

static s16 _previousWaitForFrameStartTotalTime = 0;
static s16 _previousRenderingTotalTime = 0;
static s16 _previousUpdateVisualsTotalTime = 0;
static s16 _previousUpdateLogicTotalTime = 0;
static s16 _previousUpdatePhysicsTotalTime = 0;
static s16 _previousUpdateTransformationsTotalTime = 0;
static s16 _previousStreamingTotalTime = 0;
static s16 _previousHandleInputTotalTime = 0;
static s16 _previousDispatchDelayedMessageTotalTime = 0;

static s16 _previousRenderingHighestTime = 0;
static s16 _previousUpdateVisualsHighestTime = 0;
static s16 _previousUpdateLogicHighestTime = 0;
static s16 _previousStreamingHighestTime = 0;
static s16 _previousUpdatePhysicsHighestTime = 0;
static s16 _previousUpdateTransformationsHighestTime = 0;
static s16 _previousHandleInputHighestTime = 0;
static s16 _previousDispatchDelayedMessageHighestTime = 0;
static s16 _previousProcessCollisionsTotalTime = 0;
static s16 _previousProcessCollisionsHighestTime = 0;

static s16 _previousGameFrameTotalTime = 0;

#endif


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static void Game::init()
{
	Game::getInstance();
}

// class's constructor
static bool Game::isConstructed()
{
	return 0 < _singletonConstructed;
}

// class's constructor
void Game::constructor()
{
	// check memory map before anything else
	HardwareManager::checkMemoryMap();

	// construct base object
	Base::constructor();

	// make sure the memory pool is initialized now
	MemoryPool::getInstance();

	this->gameFrameTotalTime = 0;
	this->randomSeed = 0;

	// force construction now
	this->clockManager = ClockManager::getInstance();

	// construct the general clock
	this->clock = new Clock();

	// construct the game's state machine
	this->stateMachine = new StateMachine(this);

	this->currentState = NULL;
	this->autoPauseState = NULL;
	this->saveDataManager = NULL;
	this->nextState = NULL;
	this->currentFrameEnded = false;
	this->isPaused = false;

	// make sure all managers are initialized now
	this->camera = Camera::getInstance();
	this->keypadManager = KeypadManager::getInstance();
	this->vipManager = VIPManager::getInstance();
	this->timerManager = TimerManager::getInstance();
	this->communicationManager = CommunicationManager::getInstance();

	SoundManager::getInstance();
	CharSetManager::getInstance();
	BgmapTextureManager::getInstance();
	FrameRate::getInstance();
	HardwareManager::getInstance();
	SpriteManager::getInstance();
	DirectDraw::getInstance();
	I18n::getInstance();
	ParamTableManager::getInstance();

	Utilities::setClock(this->clock);
	Utilities::setKeypadManager(this->keypadManager);

#ifdef __DEBUG_TOOLS
	DebugState::getInstance();
#endif
#ifdef __STAGE_EDITOR
	StageEditorState::getInstance();
#endif
#ifdef __ANIMATION_INSPECTOR
	AnimationInspectorState::getInstance();
#endif

	// to make debugging easier
	this->lastProcessName = "start up";

	this->nextStateOperation = kSwapState;

	// setup engine parameters
	Game::initialize(this);
}

// class's destructor
void Game::destructor()
{
	// destroy the clocks
	Clock::destructor(this->clock);

	delete this->stateMachine;

	Base::destructor();
}

// setup engine parameters
void Game::initialize()
{
	// setup vectorInterrupts
	HardwareManager::setInterruptVectors(HardwareManager::getInstance());

	// Reset sounds
	SoundManager::reset(SoundManager::getInstance());

	// clear sprite memory
	HardwareManager::clearScreen(HardwareManager::getInstance());

	// make sure timer interrupts are enable
	HardwareManager::initializeTimer(HardwareManager::getInstance());

	// start the game's general clock
	Clock::start(this->clock);
}

// set game's initial state
void Game::start(GameState state)
{
	ASSERT(state, "Game::start: initial state is NULL");

	// initialize SRAM
	SRAMManager::getInstance();

	// initialize VPU and turn off the brightness
	HardwareManager::lowerBrightness(HardwareManager::getInstance());

	if(!StateMachine::getCurrentState(this->stateMachine))
	{
		// set state
		Game::setNextState(this, state);

		while(true)
		{
			while(!this->currentFrameEnded)
			{
				HardwareManager::halt();
			}

			this->currentFrameEnded = false;

#ifdef __PROFILE_GAME
			u32 elapsedTime = TimerManager::getMillisecondsElapsed(this->timerManager);

			if(_updateProfiling)
			{
				static u32 cycleCount = 0;
				static u32 totalGameFrameDuration = 0;
				totalGameFrameDuration += elapsedTime;

				_gameFrameEffectiveDuration = elapsedTime;
				_gameFrameEffectiveDurationAverage = totalGameFrameDuration / ++cycleCount;
				_gameFrameHighestEffectiveDuration = (s16)elapsedTime > _gameFrameHighestEffectiveDuration ? (s16)elapsedTime : _gameFrameHighestEffectiveDuration;
			}
#endif

#ifdef __PRINT_PROFILING_INFO
			Game::checkFrameRate(this);
#endif

#ifdef __PRINT_WIREFRAME_MANAGER_STATUS
			WireframeManager::print(WireframeManager::getInstance(), 1, 1);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "start frame";
#endif

#ifdef __PROFILE_GAME
			_waitForFrameStartTotalTime += TimerManager::getMillisecondsElapsed(this->timerManager) - elapsedTime;
#endif

			// execute game frame
			Game::run(this);

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "end frame";
#endif

			// increase the fps counter
			FrameRate::increaseFps(FrameRate::getInstance());

#ifdef __PROFILE_GAME

			if(_updateProfiling)
			{
				static u32 cycleCount = 0;
				s32 gameFrameDuration = _waitForFrameStartTotalTime +
									_renderingProcessTime +
									_synchronizeGraphicsProcessTime +
									_updateLogicProcessTime +
									_streamingProcessTime +
									_updatePhysicsProcessTime +
									_updateTransformationsProcessTime +
									_processUserInputProcessTime +
									_dispatchDelayedMessageProcessTime +
									_processingCollisionsProcessTime;

				static s32 totalGameFrameRealDuration = 0;
				totalGameFrameRealDuration += gameFrameDuration;

				_gameFrameRealDuration = gameFrameDuration > _gameFrameRealDuration ? gameFrameDuration : _gameFrameRealDuration;
				_gameFrameDurationAverage = totalGameFrameRealDuration / ++cycleCount;

				if(cycleCount >= (0x000000001 < (sizeof(u32) * 7)))
				{
					totalGameFrameRealDuration = 0;
					cycleCount = 0;
				}

				// skip the rest of the cycle if already late
				if(this->currentFrameEnded)
				{
					_tornGameFrameCount++;
				}
			}
#endif

#ifdef __SHOW_GAME_PROFILE_DURING_TORN_FRAMES
			// skip the rest of the cycle if already late
			if(_processNameDuringFRAMESTART && strcmp(_processNameDuringFRAMESTART, "end frame"))
			{
				PRINT_TIME(20, 0);
				Game::showCurrentGameFrameProfiling(this, 1, 0);
			}
#endif

#ifdef __SHOW_ALERT_FOR_TORN_FRAMES
		if(this->currentFrameEnded)
		{
			static int counter = 0;
			PRINT_TEXT("Torn Frames:", 0, 26);
			PRINT_INT(counter++, 13, 26);
		}
#endif

#ifdef __PROFILE_GAME
			_updateProfiling = !Game::isInSpecialMode(this);
#endif
		}
	}
	else
	{
		ASSERT(false, "Game::start: already started");
	}
}

// set game's state
void Game::changeState(GameState state)
{
	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kSwapState;
}

// set game's state after cleaning the stack
void Game::cleanAndChangeState(GameState state)
{
	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kCleanAndSwapState;
}

// add a state to the game's state machine's stack
void Game::addState(GameState state)
{
	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kPushState;
}

// set game's state
void Game::setNextState(GameState state)
{
	ASSERT(state, "Game::setState: setting NULL state");

	// prevent the VIPManager to modify the DRAM during the next state's setup
	VIPManager::allowDRAMAccess(this->vipManager, false);

	switch(this->nextStateOperation)
	{
		case kCleanAndSwapState:

			// clean the game's stack
			// pop states until the stack is empty
			while(StateMachine::getStackSize(this->stateMachine) > 0)
			{
				State stateMachineCurrentState = StateMachine::getCurrentState(this->stateMachine);
				if(stateMachineCurrentState)
				{
					// discard delayed messages from the current state
					MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(stateMachineCurrentState));
					MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
				}

				StateMachine::popState(this->stateMachine);
			}

			// setup new state
			StateMachine::pushState(this->stateMachine, (State)state);
			break;

		case kSwapState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "swapping state";
#endif

			if(this->currentState)
			{
				// discard delayed messages from the current state
				MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine))));
				MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
			}

			// setup new state
			StateMachine::swapState(this->stateMachine, (State)state);
			break;

		case kPushState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "pushing state";
#endif
			// setup new state
			StateMachine::pushState(this->stateMachine, (State)state);
			break;

		case kPopState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "popping state";
#endif

			if(this->currentState)
			{
				// discard delayed messages from the current state
				MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine))));
				MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
			}

			// setup new state
			StateMachine::popState(this->stateMachine);
			break;
	}

	// no next state now
	this->nextState = NULL;

	// save current state
	this->currentState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	// allow the VIPManager to modify the DRAM
	VIPManager::allowDRAMAccess(this->vipManager, true);

	// fire event
	Object::fireEvent(this, kEventNextStateSet);
}

// disable interrupts
void Game::disableHardwareInterrupts()
{
	HardwareManager::disableInterrupts();
}

// enable interrupts
void Game::enableHardwareInterrupts()
{
	HardwareManager::enableInterrupts();
}

// erase engine's current status
void Game::reset()
{
#ifdef	__MEMORY_POOL_CLEAN_UP
	MemoryPool::cleanUp(MemoryPool::getInstance());
#endif

	// Disable timer
	TimerManager::enable(this->timerManager, false);

	// disable rendering
	HardwareManager::lowerBrightness(HardwareManager::getInstance());
	HardwareManager::displayOff(HardwareManager::getInstance());
	HardwareManager::disableRendering(HardwareManager::getInstance());
	HardwareManager::clearScreen(HardwareManager::getInstance());
	HardwareManager::setupColumnTable(HardwareManager::getInstance(), NULL);
	VIPManager::removePostProcessingEffects(this->vipManager);

	// reset managers
	WireframeManager::reset(WireframeManager::getInstance());
	SoundManager::reset(SoundManager::getInstance());
	TimerManager::resetMilliseconds(this->timerManager);
	KeypadManager::reset(this->keypadManager);

	// the order of reset for the graphics managers must not be changed!
	SpriteManager::reset(SpriteManager::getInstance());
	BgmapTextureManager::reset(BgmapTextureManager::getInstance());
	CharSetManager::reset(CharSetManager::getInstance());
	ParamTableManager::reset(ParamTableManager::getInstance());
	AnimationCoordinatorFactory::reset(AnimationCoordinatorFactory::getInstance());
	Printing::reset(Printing::getInstance());

	// reset profiling
	Game::resetProfiling(this);

	// Enable timer
	TimerManager::enable(this->timerManager, true);
}

void Game::enableRendering(bool isVersusMode)
{
	if(isVersusMode)
	{
		CommunicationManager::startSyncCycle(this->communicationManager);
	}

	HardwareManager::displayOn(HardwareManager::getInstance());
	HardwareManager::enableRendering(HardwareManager::getInstance());
	HardwareManager::enableInterrupts();
}

// process input data according to the actual game status
u32 Game::processUserInput()
{
	if(!KeypadManager::isEnabled(this->keypadManager))
	{
		return false;
	}

#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "processing input";
#endif

	// poll the user's input
	KeypadManager::captureUserInput(this->keypadManager);
	UserInput userInput = KeypadManager::getUserInput(this->keypadManager);

#ifdef __DEBUG_TOOLS

	// check code to access special feature
	if((userInput.previousKey & K_LT) && (userInput.previousKey & K_RT) && (userInput.pressedKey & K_RL))
	{
		if(Game::isInDebugMode(this))
		{
			this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
			StateMachine::popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if(Game::isInSpecialMode(this))
			{
				this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
				StateMachine::popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = GameState::safeCast(DebugState::getInstance());
			StateMachine::pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		return true;
	}
#endif

#ifdef __STAGE_EDITOR

	// check code to access special feature
	if((userInput.previousKey & K_LT) && (userInput.previousKey & K_RT) && (userInput.pressedKey & K_RD))
	{
		if(Game::isInStageEditor(this))
		{
			this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
			StateMachine::popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if(Game::isInSpecialMode(this))
			{
				this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
				StateMachine::popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = GameState::safeCast(StageEditorState::getInstance());
			StateMachine::pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		return true;
	}
#endif

#ifdef __ANIMATION_INSPECTOR

	// check code to access special feature
	if((userInput.previousKey & K_LT) && (userInput.previousKey & K_RT) && (userInput.pressedKey & K_RR))
	{		if(Game::isInAnimationInspector(this))
		{
			this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
			StateMachine::popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if(Game::isInSpecialMode(this))
			{
				this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
				StateMachine::popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = GameState::safeCast(AnimationInspectorState::getInstance());
			StateMachine::pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		return true;
	}
#endif

#ifdef __DEBUG_TOOLS
	if(!Game::isInSpecialMode(this) && ((userInput.pressedKey & K_LT) || (userInput.pressedKey & K_RT)))
	{
		return true;
	}
#endif

#ifdef __STAGE_EDITOR
	if(!Game::isInSpecialMode(this) && ((userInput.pressedKey & K_LT) || (userInput.pressedKey & K_RT)))
	{
		return true;
	}
#endif

#ifdef __ANIMATION_INSPECTOR
	if(!Game::isInSpecialMode(this) && ((userInput.pressedKey & K_LT) || (userInput.pressedKey & K_RT)))
	{
		return true;
	}
#endif

	if(GameState::processUserInputRegardlessOfInput(Game::getCurrentState(this)) || (userInput.pressedKey | userInput.releasedKey | userInput.holdKey))
	{
		GameState::processUserInput(Game::getCurrentState(this), userInput);
	}

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_processUserInputProcessTime =  -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_processUserInputTotalTime += _processUserInputProcessTime;
		_processUserInputHighestTime = _processUserInputHighestTime < _processUserInputProcessTime ? _processUserInputProcessTime : _processUserInputHighestTime;
	}
#endif

	return userInput.pressedKey | userInput.releasedKey;
}

u32 Game::dispatchDelayedMessages()
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "processing messages";
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	static u32 dispatchCycle = 0;

	if(dispatchCycle++ & 1)
	{
#endif

#ifdef __PROFILE_GAME
		u32 dispatchedMessages = MessageDispatcher::dispatchDelayedMessages(MessageDispatcher::getInstance());

		if(_updateProfiling)
		{
			_dispatchDelayedMessageProcessTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
			_dispatchDelayedMessageTotalTime += _dispatchDelayedMessageProcessTime;
			_dispatchDelayedMessageHighestTime = _dispatchDelayedMessageHighestTime < _dispatchDelayedMessageProcessTime ? _dispatchDelayedMessageProcessTime : _dispatchDelayedMessageHighestTime;
		}

		return dispatchedMessages;
#else
		return MessageDispatcher::dispatchDelayedMessages(MessageDispatcher::getInstance());
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	}

	return false;
#endif
}

// update game's logic subsystem
void Game::updateLogic()
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __DEBUG_TOOLS
	if(!Game::isInSpecialMode(this))
	{
#endif
#ifdef __STAGE_EDITOR
	if(!Game::isInSpecialMode(this))
	{
#endif
#ifdef __ANIMATION_INSPECTOR
	if(!Game::isInSpecialMode(this))
	{
#endif
	// it is the update cycle
	ASSERT(this->stateMachine, "Game::update: no state machine");
#ifdef __DEBUG_TOOLS
	}
#endif
#ifdef __STAGE_EDITOR
	}
#endif
#ifdef __ANIMATION_INSPECTOR
	}
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "updating logic";
#endif

	// update the game's logic
	StateMachine::update(this->stateMachine);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_updateLogicProcessTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_updateLogicTotalTime += _updateLogicProcessTime;
		_updateLogicHighestTime = _updateLogicHighestTime < _updateLogicProcessTime ? _updateLogicProcessTime : _updateLogicHighestTime;
	}
#endif
}

// update game's rendering subsystem
void Game::synchronizeGraphics()
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "synchronizing graphics";
#endif

	// prevent the VIPManager to modify the DRAM
	// during the synchronization of the entities'
	// positions with their sprites
	VIPManager::allowDRAMAccess(this->vipManager, false);

	// apply transformations to graphics
	GameState::synchronizeGraphics(this->currentState);

	if(VIPManager::isRenderingPending(this->vipManager))
	{
#ifdef __REGISTER_LAST_PROCESS_NAME
		this->lastProcessName = "rendering sprites";
#endif

		SpriteManager::render(SpriteManager::getInstance());
	}

	// allow the VIPManager to modify the DRAM
	VIPManager::allowDRAMAccess(this->vipManager, true);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_synchronizeGraphicsProcessTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_synchronizeGraphicsTotalTime += _synchronizeGraphicsProcessTime;
		_synchronizeGraphicsHighestTime = _synchronizeGraphicsHighestTime < _synchronizeGraphicsProcessTime ? _synchronizeGraphicsProcessTime : _synchronizeGraphicsHighestTime;
	}

#endif
}

// update game's physics subsystem
void Game::updatePhysics()
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "updating physics";
#endif

	// simulate physics
	GameState::updatePhysics(this->currentState);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_updatePhysicsProcessTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_updatePhysicsTotalTime += _updatePhysicsProcessTime;
		_updatePhysicsHighestTime = _updatePhysicsHighestTime < _updatePhysicsProcessTime ? _updatePhysicsProcessTime : _updatePhysicsHighestTime;
	}
#endif
}

void Game::updateTransformations()
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "focusing camera";
#endif
	// position the camera
	Camera::focus(this->camera, true);

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "updating transforms";
#endif

	// apply world transformations
	GameState::transform(this->currentState);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_updateTransformationsProcessTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_updateTransformationsTotalTime += _updateTransformationsProcessTime;
		_updateTransformationsHighestTime = _updateTransformationsHighestTime < _updateTransformationsProcessTime ? _updateTransformationsProcessTime : _updateTransformationsHighestTime;
	}
#endif
}

u32 Game::updateCollisions()
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

	// process the collisions after the transformations have taken place
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "processing collisions";
#endif

	// process collisions
#ifdef __PROFILE_GAME
	u32 processedCollisions = GameState::processCollisions(this->currentState);

	if(_updateProfiling)
	{
		_processingCollisionsProcessTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_processingCollisionsTotalTime += _processingCollisionsProcessTime;
		_processingCollisionsHighestTime = _processingCollisionsHighestTime < _processingCollisionsProcessTime ? _processingCollisionsProcessTime : _processingCollisionsHighestTime;
	}

	return processedCollisions;
#else
	return GameState::processCollisions(this->currentState);
#endif
}

bool Game::stream()
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager::getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "streaming level";
#endif

#ifdef __PROFILE_GAME
	u32 streamProcessed = GameState::stream(this->currentState);

	if(_updateProfiling)
	{
		_streamingProcessTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_streamingTotalTime += _streamingProcessTime;
		_streamingHighestTime = _streamingHighestTime < _streamingProcessTime ? _streamingProcessTime : _streamingHighestTime;
	}

	return streamProcessed;
#else
	return GameState::stream(this->currentState);
#endif
}

void Game::checkForNewState()
{
	if(this->nextState)
	{
#ifdef __REGISTER_LAST_PROCESS_NAME
		this->lastProcessName = "checking for new state";
#endif
		Game::setNextState(this, this->nextState);

#undef __DIMM_FOR_PROFILING
#ifdef __DIMM_FOR_PROFILING

		_vipRegisters[__GPLT0] = 0x50;
		_vipRegisters[__GPLT1] = 0x50;
		_vipRegisters[__GPLT2] = 0x54;
		_vipRegisters[__GPLT3] = 0x54;
		_vipRegisters[__JPLT0] = 0x54;
		_vipRegisters[__JPLT1] = 0x54;
		_vipRegisters[__JPLT2] = 0x54;
		_vipRegisters[__JPLT3] = 0x54;

		_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE4;
#endif
	}
}

void Game::increaseGameFrameDuration(u32 gameFrameDuration)
{
	this->gameFrameTotalTime += gameFrameDuration;
}

void Game::checkFrameRate()
{
	if(Game::isInSpecialMode(this))
	{
		return;
	}

	// this method "doesn't" exist
	TimerManager::enable(this->timerManager, false);

	if(this->gameFrameTotalTime >= __MILLISECONDS_IN_SECOND)
	{
#ifdef __SHOW_GAME_PROFILING
		if(_updateProfiling)
		{
			Game::showProfiling(this, 1, 0);
			Game::resetProfiling(this);
			FrameRate::print(FrameRate::getInstance(), 29, 0);
		}
#else
#ifdef __PROFILE_GAME
		if(_updateProfiling)
		{
			Game::resetProfiling(this);
		}
#endif
#ifdef __SHOW_STREAMING_PROFILING

		if(!Game::isInSpecialMode(this))
		{
			Printing::resetWorldCoordinates(Printing::getInstance());
			Stage::showStreamingProfiling(Game::getStage(this), 1, 1);
		}
#endif

#endif
		this->gameFrameTotalTime = 0;

#ifdef __DEBUG
#ifdef __PRINT_DEBUG_ALERT
		Printing::text(Printing::getInstance(), "DEBUG MODE", 0, (__SCREEN_HEIGHT_IN_CHARS) - 1, NULL);
#endif
#endif

#ifdef __PRINT_FRAMERATE
		if(!Game::isInSpecialMode(this))
		{
			FrameRate::print(FrameRate::getInstance(), 21, 26);
		}
#endif

#ifdef __PRINT_MEMORY_POOL_STATUS
		if(!Game::isInSpecialMode(this))
		{
			Printing::resetWorldCoordinates(Printing::getInstance());

#ifdef __PRINT_DETAILED_MEMORY_POOL_STATUS
			MemoryPool::printDetailedUsage(MemoryPool::getInstance(), 30, 1);
#else
			MemoryPool::printResumedUsage(MemoryPool::getInstance(), 35, 1);
#endif
		}
#endif

#ifdef __ALERT_STACK_OVERFLOW
		if(!Game::isInSpecialMode(this))
		{
			Printing::resetWorldCoordinates(Printing::getInstance());
			HardwareManager::printStackStatus(HardwareManager::getInstance(), (__SCREEN_WIDTH_IN_CHARS) - 10, 0, true);
		}
#endif
		//reset frame rate counters
		FrameRate::reset(FrameRate::getInstance());

#ifdef __PROFILE_GAME
		Game::resetCurrentFrameProfiling(this, TimerManager::getMillisecondsElapsed(this->timerManager));
#endif
	}

	// enable timer
	TimerManager::enable(this->timerManager, true);
}

void Game::currentFrameEnded()
{
	// raise flag to allow the next frame to start
	this->currentFrameEnded = true;
}

#define SKIP_FRAME(x)		if(this->currentFrameEnded && ! flag) {flag = true; PRINT_TEXT(x, 20, 27); }

void Game::run()
{
	bool flag = false;
	// reset timer
	TimerManager::resetMilliseconds(this->timerManager);

	// Generate random seed
	this->randomSeed = Utilities::randomSeed();

	// sync entities with their sprites
	Game::synchronizeGraphics(this);

SKIP_FRAME("sync")

	// process user's input
	bool skipNonCriticalProcesses = Game::processUserInput(this);

SKIP_FRAME("inpu")

	// simulate physics
	Game::updatePhysics(this);

SKIP_FRAME("phys")

	// apply transformations
	Game::updateTransformations(this);

SKIP_FRAME("trans")

	// process collisions
	skipNonCriticalProcesses |= Game::updateCollisions(this);

SKIP_FRAME("coll ")

	// dispatch delayed messages
	skipNonCriticalProcesses |= Game::dispatchDelayedMessages(this);

SKIP_FRAME("mess ")

	// update game's logic
	Game::updateLogic(this);

SKIP_FRAME("log ")

#ifndef __DISABLE_STREAMING
	// skip streaming if the game frame has been too busy
	if(!skipNonCriticalProcesses)
	{
		// stream
		Game::stream(this);
	}
#endif

SKIP_FRAME("   ")

	// this is the moment to check if the game's state
	// needs to be changed
	Game::checkForNewState(this);
}

#ifdef __REGISTER_LAST_PROCESS_NAME
void Game::setLastProcessName(char* processName)
{	this->lastProcessName = processName;
}
#endif

// process a telegram
bool Game::handleMessage(Telegram telegram)
{
	ASSERT(this->stateMachine, "Game::handleMessage: NULL stateMachine");

	return StateMachine::handleMessage(this->stateMachine, telegram);
}

// retrieve time
u32 Game::getTime()
{
	return Clock::getTime(this->clock);
}

// retrieve clock
Clock Game::getClock()
{
	return this->clock;
}

// retrieve in game clock
Clock Game::getMessagingClock()
{
	return GameState::getMessagingClock(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine)));
}

// retrieve animations' clock
Clock Game::getUpdateClock()
{
	return GameState::getUpdateClock(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine)));
}

// retrieve in physics' clock
Clock Game::getPhysicsClock()
{
	return GameState::getPhysicsClock(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine)));
}

// retrieve last process' name
char* Game::getLastProcessName()
{
	return this->lastProcessName;
}

#ifdef __DEBUG_TOOLS
bool Game::isInDebugMode()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)DebugState::getInstance();
}
#endif

#ifdef __STAGE_EDITOR
bool Game::isInStageEditor()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)StageEditorState::getInstance();
}
#endif

#ifdef __ANIMATION_INSPECTOR
bool Game::isInAnimationInspector()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)AnimationInspectorState::getInstance();
}
#endif

// whether if a special mode is active
bool Game::isInSpecialMode()
{
	int isInSpecialMode = false;

#ifdef __DEBUG_TOOLS
	isInSpecialMode |= Game::isInDebugMode(this);
#endif
#ifdef __STAGE_EDITOR
	isInSpecialMode |= Game::isInStageEditor(this);
#endif
#ifdef __ANIMATION_INSPECTOR
	isInSpecialMode |= Game::isInAnimationInspector(this);
#endif

	return isInSpecialMode;
}

// whether if a special mode is being started
bool Game::isEnteringSpecialMode()
{
	int isEnteringSpecialMode = false;
#ifdef __DEBUG_TOOLS
	isEnteringSpecialMode |= GameState::safeCast(DebugState::getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isEnteringSpecialMode |= GameState::safeCast(StageEditorState::getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_INSPECTOR
	isEnteringSpecialMode |= GameState::safeCast(AnimationInspectorState::getInstance()) == this->nextState;
#endif

	return isEnteringSpecialMode;
}

// whether if a special mode is being started
bool Game::isExitingSpecialMode()
{
	int isEnteringSpecialMode = false;
#ifdef __DEBUG_TOOLS
	isEnteringSpecialMode |= GameState::safeCast(DebugState::getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isEnteringSpecialMode |= GameState::safeCast(StageEditorState::getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_INSPECTOR
	isEnteringSpecialMode |= GameState::safeCast(AnimationInspectorState::getInstance()) == this->nextState;
#endif

	return isEnteringSpecialMode;
}

// retrieve state machine, use with caution!!!
StateMachine Game::getStateMachine()
{
	return this->stateMachine;
}

// retrieve the current level's stage
Stage Game::getStage()
{
	if(Game::isInSpecialMode(this))
	{
		return GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}

	return GameState::getStage(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine)));
}

// retrieve current state
GameState Game::getCurrentState()
{
	return GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
}

PhysicalWorld Game::getPhysicalWorld()
{
	if(Game::isInSpecialMode(this))
	{
		return GameState::getPhysicalWorld(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}

	return GameState::getPhysicalWorld(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine)));
}

CollisionManager Game::getCollisionManager()
{
	if(Game::isInSpecialMode(this))
	{
		return GameState::getCollisionManager(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}

	return GameState::getCollisionManager(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine)));
}

// pause
void Game::pause(GameState pauseState)
{
	ASSERT(pauseState, "Game::pause: null pauseState");

	if(pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPushState;
		this->isPaused = true;
		Object::fireEvent(this, kEventGamePaused);
	}
}

// resume game
void Game::unpause(GameState pauseState)
{
	ASSERT(pauseState, "Game::unpause: null pauseState");
	ASSERT(pauseState == this->currentState, "Game::unpause: pauseState sent is not the current one");

	if(pauseState && this->currentState == pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPopState;
		this->isPaused = false;
		Object::fireEvent(this, kEventGameUnpaused);
	}
}

// is game currently paused?
bool Game::isPaused()
{
	return this->isPaused;
}

void Game::disableKeypad()
{
	KeypadManager::disable(this->keypadManager);
}

void Game::enableKeypad()
{
	KeypadManager::enable(this->keypadManager);
}

void Game::pushFrontProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushFrontPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game::pushBackProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushBackPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::removePostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game::wait(u32 milliSeconds)
{
	ASSERT(this, "Game::wait: this null");

	TimerManager::wait(this->timerManager, milliSeconds);
}

#ifdef __PROFILE_GAME
void Game::saveProcessNameDuringFRAMESTART()
{
	ASSERT(this, "Game::saveProcessNameDuringFRAMESTART: this null");

	_processNameDuringFRAMESTART = this->lastProcessName;
}

void Game::saveProcessNameDuringXPEND()
{
	ASSERT(this, "Game::saveProcessNameDuringXPEND: this null");

	_processNameDuringXPEND = this->lastProcessName;
}
#endif

#ifdef __SHOW_GAME_PROFILING
void Game::showProfiling(int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	ASSERT(this, "Game::showProfiling: this null");

	_updateProfiling = false;

	int xDisplacement = 32;

	Printing printing = Printing::getInstance();

	Printing::resetWorldCoordinates(printing);

	Printing::text(printing, "PROFILING", x, y++, NULL);

	Printing::text(printing, "Last game frame's info        (ms)", x, ++y, NULL);
	Printing::text(printing, "Milliseconds elapsed:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, TimerManager::getMillisecondsElapsed(this->timerManager), x + xDisplacement, y++, NULL);

	Printing::text(printing, "Real duration:", x, ++y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _gameFrameRealDuration, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Average real duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _gameFrameDurationAverage, x + xDisplacement, y++, NULL);

	Printing::text(printing, "Effective duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _gameFrameEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Average effective duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _gameFrameEffectiveDurationAverage, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Highest effective duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _gameFrameHighestEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Torn frames count:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _tornGameFrameCount, x + xDisplacement, y++, NULL);
	Printing::text(printing, "                                                ", x, y, NULL);
	if(_processNameDuringFRAMESTART)
	{
		Printing::text(printing, "Process during FRAMESTART:", x, y, NULL);
		Printing::text(printing, _processNameDuringFRAMESTART, x + xDisplacement - 5, y++, NULL);
	}
	Printing::text(printing, "                                                ", x, y, NULL);
	if(_processNameDuringXPEND)
	{
		Printing::text(printing, "Process during XPEND:", x, y, NULL);
		Printing::text(printing, _processNameDuringXPEND, x + xDisplacement - 5, y++, NULL);
	}

	int xDisplacement2 = 7;

#ifdef __SHOW_GAME_DETAILED_PROFILING

	Printing::text(printing, "Processes' duration (ms/sec)", x, ++y, NULL);

	Printing::text(printing, "                              total highest", x, ++y, NULL);

	int processNumber = 1;
	Printing::text(printing, "  Rendering:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _renderingTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _renderingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Updating visuals:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _synchronizeGraphicsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _synchronizeGraphicsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Handling input:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _processUserInputTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _processUserInputHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Updating physics:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _updatePhysicsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _updatePhysicsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Transforming:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _updateTransformationsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _updateTransformationsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Processing collisions:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _processingCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _processingCollisionsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Updating logic:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _updateLogicTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _updateLogicHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Processing messages:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _dispatchDelayedMessageTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _dispatchDelayedMessageHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "  Streaming:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _streamingTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _streamingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

#endif

	Printing::text(printing, "Last second processing (ms)", x, ++y, NULL);
	Printing::text(printing, "Real processing time:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _waitForFrameStartTotalTime + _renderingTotalTime + _synchronizeGraphicsTotalTime + _processUserInputTotalTime + _dispatchDelayedMessageTotalTime + _updateLogicTotalTime + _streamingTotalTime + _updatePhysicsTotalTime + _updateTransformationsTotalTime + _processingCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _renderingHighestTime + _synchronizeGraphicsHighestTime + _updateLogicHighestTime + _streamingHighestTime + _updatePhysicsHighestTime + _updateTransformationsHighestTime + _processUserInputHighestTime + _dispatchDelayedMessageHighestTime + _processingCollisionsHighestTime, x + xDisplacement + xDisplacement2, y, NULL);

	Printing::text(printing, "Effective processing time:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, this->gameFrameTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _gameFrameHighestEffectiveDuration, x + xDisplacement + xDisplacement2, y, NULL);
}
#endif

#ifdef __PROFILE_GAME
void Game::showCurrentGameFrameProfiling(int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	ASSERT(this, "Game::showCurrentGameFrameProfiling: this null");

	_updateProfiling = false;

	Printing printing = Printing::getInstance();

	int xDisplacement = 32;

	Printing::resetWorldCoordinates(printing);

	Printing::text(printing, "PROFILING", x, y++, NULL);

	Printing::text(printing, "Current game frame's info        (ms)", x, ++y, NULL);
	Printing::text(printing, "Milliseconds elapsed:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, TimerManager::getMillisecondsElapsed(this->timerManager), x + xDisplacement, y++, NULL);

	Printing::text(printing, "Process during FRAMESTART:", x, y, NULL);

	if(_processNameDuringFRAMESTART)
	{
		Printing::text(printing, _processNameDuringFRAMESTART, x + xDisplacement - 5, y++, NULL);
	}
	else
	{
		Printing::text(printing, "                      ", x + xDisplacement, y++, NULL);
	}

	Printing::text(printing, "                                                ", x, y, NULL);

	Printing::text(printing, "Process during XPEND:", x, y, NULL);

	if(_processNameDuringXPEND)
	{
		Printing::text(printing, _processNameDuringXPEND, x + xDisplacement - 5, y++, NULL);
	}
	else
	{
		Printing::text(printing, "                      ", x + xDisplacement, y++, NULL);
	}

	Printing::text(printing, "Processes' duration (ms/sec)", x, ++y, NULL);

	int processNumber = 1;
	Printing::text(printing, "  Rendering:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _renderingProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Updating visuals:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _synchronizeGraphicsProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Handling input:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _processUserInputProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Updating physics:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _updatePhysicsProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Transforming:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _updateTransformationsProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Processing collisions:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _processingCollisionsProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Updating logic:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _updateLogicProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Processing messages:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _dispatchDelayedMessageProcessTime, x + xDisplacement, y, NULL);

	Printing::text(printing, "  Streaming:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, processNumber, x, y, NULL);
	Printing::int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing::int(printing, _streamingProcessTime, x + xDisplacement, y++, NULL);

	Printing::text(printing, "Last second processing (ms)", x, ++y, NULL);
	Printing::text(printing, "Real processing time:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _renderingProcessTime +
							_synchronizeGraphicsProcessTime +
							_processUserInputProcessTime +
							_updatePhysicsProcessTime +
							_updateTransformationsProcessTime +
							_processingCollisionsProcessTime +
							_updateLogicProcessTime +
							_dispatchDelayedMessageProcessTime +
							_streamingProcessTime,
							x + xDisplacement, y, NULL);


#ifdef __DIMM_FOR_PROFILING
		_vipRegisters[__GPLT0] = 0x50;
		_vipRegisters[__GPLT1] = 0x50;
		_vipRegisters[__GPLT2] = 0x54;
		_vipRegisters[__GPLT3] = 0x54;
		_vipRegisters[__JPLT0] = 0x54;
		_vipRegisters[__JPLT1] = 0x54;
		_vipRegisters[__JPLT2] = 0x54;
		_vipRegisters[__JPLT3] = 0x54;

		_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE4;
#endif
}
#endif

void Game::showLastGameFrameProfiling(int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	ASSERT(this, "Game::showLastGameFrameProfiling: this null");

#ifdef __PROFILE_GAME

	int xDisplacement = 32;

	Printing printing = Printing::getInstance();

	Printing::text(printing, "PROFILING", x, y++, NULL);

	Printing::text(printing, "Last game frame's info (ms)", x, ++y, NULL);
	Printing::text(printing, "Milliseconds elapsed:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, TimerManager::getMillisecondsElapsed(this->timerManager), x + xDisplacement, y++, NULL);

	Printing::text(printing, "Real duration:", x, ++y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousGameFrameRealDuration, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Average real duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousGameFrameDurationAverage, x + xDisplacement, y++, NULL);

	Printing::text(printing, "Effective duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousGameFrameEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Average effective duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousGameFrameEffectiveDurationAverage, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Highest effective duration:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousGameFrameHighestEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing::text(printing, "Torn frames count:", x, y, NULL);
	Printing::text(printing, "   ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousTornGameFrameCount, x + xDisplacement, y++, NULL);

	Printing::text(printing, "Processes' duration (ms in sec)", x, ++y, NULL);

	int xDisplacement2 = 7;

	Printing::text(printing, "                              total highest", x, ++y, NULL);

	Printing::text(printing, "Rendering:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousRenderingTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousRenderingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Updating visuals:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdateVisualsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdateVisualsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Handling input:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousHandleInputTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousHandleInputHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Updating physics:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdatePhysicsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdatePhysicsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Transforming:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdateTransformationsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdateTransformationsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Processing collisions:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousProcessCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousProcessCollisionsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Updating logic:     ", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdateLogicTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousUpdateLogicHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Processing messages:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousDispatchDelayedMessageTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousDispatchDelayedMessageHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Streaming:", x, y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousStreamingTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousStreamingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing::text(printing, "Last second processing (ms)", x, ++y, NULL);
	Printing::text(printing, "Real processing time:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousWaitForFrameStartTotalTime + _previousRenderingTotalTime + _previousUpdateVisualsTotalTime + _previousHandleInputTotalTime + _previousDispatchDelayedMessageTotalTime + _previousUpdateLogicTotalTime + _previousStreamingTotalTime + _previousUpdatePhysicsTotalTime + _previousUpdateTransformationsTotalTime + _previousProcessCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousRenderingHighestTime + _previousUpdateVisualsHighestTime + _previousHandleInputHighestTime + _previousDispatchDelayedMessageHighestTime + _previousUpdateLogicHighestTime + _previousStreamingHighestTime + _previousUpdatePhysicsHighestTime + _previousUpdateTransformationsHighestTime + _previousProcessCollisionsHighestTime, x + xDisplacement + xDisplacement2, y, NULL);

	Printing::text(printing, "Effective processing time:", x, ++y, NULL);
	Printing::text(printing, "          ", x + xDisplacement, y, NULL);
	Printing::int(printing, _previousGameFrameTotalTime, x + xDisplacement, y, NULL);
	Printing::int(printing, _previousGameFrameHighestEffectiveDuration, x + xDisplacement + xDisplacement2, y, NULL);
#endif
}

void Game::resetCurrentFrameProfiling(s32 gameFrameDuration __attribute__ ((unused)))
{
	ASSERT(this, "Game::showProfiling: this null");

#ifdef __PROFILE_GAME
/*
	if(_gameFrameHighestEffectiveDuration < gameFrameDuration)
	{
	//	_renderingHighestTime = _renderingProcessTime;
		_dispatchDelayedMessageHighestTime = _dispatchDelayedMessageProcessTime;
		_processUserInputHighestTime = _processUserInputProcessTime;
		_updateLogicHighestTime = _updateLogicProcessTime;
		_synchronizeGraphicsHighestTime = _synchronizeGraphicsProcessTime;
		_updatePhysicsHighestTime = _updatePhysicsProcessTime;
		_updateTransformationsHighestTime = _updateTransformationsProcessTime;
		_processingCollisionsHighestTime = _processingCollisionsProcessTime;
		_streamingHighestTime = _streamingProcessTime;
	}
*/
	_renderingProcessTimeHelper = 0;
	_renderingProcessTime = 0;
	_synchronizeGraphicsProcessTime = 0;
	_updateLogicProcessTime = 0;
	_streamingProcessTime = 0;
	_updatePhysicsProcessTime = 0;
	_updateTransformationsProcessTime = 0;
	_processUserInputProcessTime = 0;
	_dispatchDelayedMessageProcessTime = 0;
	_processingCollisionsProcessTime = 0;

#endif
}

void Game::resetProfiling()
{
	ASSERT(this, "Game::resetProfiling: this null");

#ifdef __PROFILE_GAME

	_previousGameFrameTotalTime = this->gameFrameTotalTime;
	_previousGameFrameHighestEffectiveDuration = _gameFrameHighestEffectiveDuration;

	_previousGameFrameRealDuration = _gameFrameRealDuration;
	_previousGameFrameDurationAverage = _gameFrameDurationAverage;
	_previousGameFrameEffectiveDuration = _gameFrameEffectiveDuration;
	_previousGameFrameEffectiveDurationAverage = _gameFrameEffectiveDurationAverage;
	_previousTornGameFrameCount = _tornGameFrameCount;

	_previousWaitForFrameStartTotalTime = _waitForFrameStartTotalTime;
	_previousRenderingTotalTime = _renderingTotalTime;
	_previousUpdateVisualsTotalTime = _synchronizeGraphicsTotalTime;
	_previousUpdateLogicTotalTime = _updateLogicTotalTime;
	_previousUpdatePhysicsTotalTime = _updatePhysicsTotalTime;
	_previousUpdateTransformationsTotalTime = _updateTransformationsTotalTime;
	_previousStreamingTotalTime = _streamingTotalTime;
	_previousHandleInputTotalTime = _processUserInputTotalTime;
	_previousDispatchDelayedMessageTotalTime = _dispatchDelayedMessageTotalTime;
	_previousProcessCollisionsTotalTime = _processingCollisionsTotalTime;

	_previousRenderingHighestTime = _renderingHighestTime;
	_previousUpdateVisualsHighestTime = _synchronizeGraphicsHighestTime;
	_previousUpdateLogicHighestTime = _updateLogicHighestTime;
	_previousUpdatePhysicsHighestTime = _updatePhysicsHighestTime;
	_previousUpdateTransformationsHighestTime = _updateTransformationsHighestTime;
	_previousStreamingHighestTime = _streamingHighestTime;
	_previousHandleInputHighestTime = _processUserInputHighestTime;
	_previousDispatchDelayedMessageHighestTime = _dispatchDelayedMessageHighestTime;
	_previousProcessCollisionsHighestTime = _processingCollisionsHighestTime;

	_gameFrameRealDuration = 0;
	_gameFrameDurationAverage = 0;
	_gameFrameEffectiveDuration = 0;
	_gameFrameEffectiveDurationAverage = 0;
	_tornGameFrameCount = 0;

	_waitForFrameStartTotalTime = 0;
	_renderingTotalTime = 0;
	_synchronizeGraphicsTotalTime = 0;
	_updateLogicTotalTime = 0;
	_updatePhysicsTotalTime = 0;
	_updateTransformationsTotalTime = 0;
	_streamingTotalTime = 0;
	_processUserInputTotalTime = 0;
	_dispatchDelayedMessageTotalTime = 0;
	_processingCollisionsTotalTime = 0;

	_renderingHighestTime = 0;
	_synchronizeGraphicsHighestTime = 0;
	_updateLogicHighestTime = 0;
	_updatePhysicsHighestTime = 0;
	_updateTransformationsHighestTime = 0;
	_streamingHighestTime = 0;
	_processUserInputHighestTime = 0;
	_dispatchDelayedMessageHighestTime = 0;
	_processingCollisionsHighestTime = 0;

	_renderingProcessTime = 0;

	_gameFrameHighestEffectiveDuration = 0;

	_processNameDuringFRAMESTART = NULL;
	_processNameDuringXPEND = NULL;
#endif
}

/**
 * Register the current auto pause state. Use NULL if no state.
 *
 * @param autoPauseState
 */
void Game::registerAutoPauseState(GameState autoPauseState)
{
	this->autoPauseState = autoPauseState;
}

GameState Game::getAutoPauseState()
{
	return this->autoPauseState;
}

/**
 * Register the current save data manager. Use NULL if none.
 *
 * @param saveDataManager
 */
void Game::registerSaveDataManager(Object saveDataManager)
{
	this->saveDataManager = saveDataManager;
}

Object Game::getSaveDataManager()
{
	return this->saveDataManager;
}

long Game::getRandomSeed()
{
	return this->randomSeed;
}

