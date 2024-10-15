/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <AnimationCoordinatorFactory.h>
#include <AnimationInspectorState.h>
#include <BgmapTextureManager.h>
#include <Camera.h>
#include <CommunicationManager.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <DebugConfig.h>
#include <DebugState.h>
#include <DirectDraw.h>
#include <CollisionManager.h>
#include <FrameRate.h>
#include <GameState.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <PhysicalWorld.h>
#include <Profiler.h>
#include <RumbleManager.h>
#include <SoundManager.h>
#include <SpriteManager.h>
#include <SRAMManager.h>
#include <Stage.h>
#include <StageEditor.h>
#include <StageEditorState.h>
#include <State.h>
#include <StateMachine.h>
#include <StopwatchManager.h>
#include <SoundTestState.h>
#include <Telegram.h>
#include <ToolState.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "VUEngine.h"


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

VUEngine _vuEngine __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
uint32 _dispatchCycle = 0;
#endif

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static void VUEngine::init()
{
	_vuEngine = VUEngine::getInstance();
}

// class's constructor
static bool VUEngine::isConstructed()
{
	return 0 < _singletonConstructed;
}

// class's constructor
void VUEngine::constructor()
{
	// check memory map before anything else
	HardwareManager::checkMemoryMap();

	// construct base object
	Base::constructor();

	// make sure the memory pool is initialized now
	MemoryPool::getInstance();

	// force construction now
	this->clockManager = ClockManager::getInstance();

	// construct the general clock
	this->clock = new Clock();

	// construct the game's state machine
	this->stateMachine = new StateMachine(this);

	this->nextGameCycleStarted = false;
	this->currentGameCycleEnded = false;
	this->isPaused = false;
	this->isToolStateTransition = false;

	// make sure all managers are initialized now
	this->saveDataManager = NULL;
	this->camera = NULL;
	this->keypadManager = NULL;
	this->vipManager = NULL;
	this->timerManager = NULL;
	this->communicationManager = NULL;
	this->frameRate = NULL;
	this->soundManager = NULL;

#ifdef __TOOLS
	DebugState::getInstance();
	StageEditorState::getInstance();
	AnimationInspectorState::getInstance();
	SoundTestState::getInstance();
#endif

	// to make debugging easier
	this->lastProcessName = PROCESS_NAME_START_UP;

	// setup engine parameters
	VUEngine::initialize(this);
}

// class's destructor
void VUEngine::destructor()
{
	// destroy the clocks
	Clock::destructor(this->clock);

	delete this->stateMachine;

	Base::destructor();
}

// setup engine parameters
void VUEngine::initialize()
{
	// make sure all managers are initialized now
	this->camera = Camera::getInstance();
	this->keypadManager = KeypadManager::getInstance();
	this->vipManager = VIPManager::getInstance();
	this->timerManager = TimerManager::getInstance();
	this->communicationManager = CommunicationManager::getInstance();
	this->frameRate = FrameRate::getInstance();
	this->soundManager = SoundManager::getInstance();
	this->wireframeManager = WireframeManager::getInstance();
	this->spriteManager = SpriteManager::getInstance();

	SpriteManager::reset(this->spriteManager);
	DirectDraw::reset(DirectDraw::getInstance());
	SRAMManager::reset(SRAMManager::getInstance());

	// setup vectorInterrupts
	HardwareManager::setInterruptVectors(HardwareManager::getInstance());

	// clear sprite memory
	HardwareManager::clearScreen(HardwareManager::getInstance());

	// make sure timer interrupts are enable
	HardwareManager::setupTimer(HardwareManager::getInstance(), __TIMER_100US, 10, kMS);

	// Reset sounds
	SoundManager::reset(this->soundManager);

	// Reset Rumble Pak
	RumbleManager::reset(RumbleManager::getInstance());

	// start the game's general clock
	Clock::start(this->clock);

	// Enable interrupts
	HardwareManager::enableInterrupts();

	// Enable communications
#ifdef __ENABLE_COMMUNICATIONS
#ifdef __RELEASE
	CommunicationManager::enableCommunications(this->communicationManager, NULL, NULL, 2000);
#else
	CommunicationManager::enableCommunications(this->communicationManager, NULL, NULL, 500);
#endif
#else
#ifdef __RELEASE
	VUEngine::wait(VUEngine::getInstance(), 4000);
#endif
#endif
}

void VUEngine::debug()
{
#ifdef __SHOW_WIREFRAME_MANAGER_STATUS
	WireframeManager::print(this->wireframeManager, 1, 1);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_END_FRAME;
#endif

#ifdef __SHOW_SOUND_STATUS
	SoundManager::print(this->soundManager);
#endif
}

// set game's initial state
void VUEngine::start(GameState currentGameState)
{
	ASSERT(currentGameState, "VUEngine::start: currentGameState is NULL");

	// Initialize VPU and turn off the brightness
	HardwareManager::lowerBrightness(HardwareManager::getInstance());

	if(NULL == StateMachine::getCurrentState(this->stateMachine))
	{
		VUEngine::run(this, currentGameState);
	}
	else
	{
		ASSERT(false, "VUEngine::start: already started");
	}
}

void VUEngine::setState(GameState gameState)
{
#ifdef __TOOLS
	this->isToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::cleaniningStatesStack, kEventStateMachineWillCleanStack);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachineCleanedStack);
	StateMachine::transitionTo(this->stateMachine, NULL != gameState ? State::safeCast(gameState) : NULL, kStateMachineCleanStack);
}

void VUEngine::changeState(GameState gameState)
{
#ifdef __TOOLS
	this->isToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::swappingState, kEventStateMachineWillSwapState);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachineSwapedState);
	StateMachine::transitionTo(this->stateMachine, State::safeCast(gameState), kStateMachineSwapState);
}

void VUEngine::addState(GameState gameState)
{
#ifdef __TOOLS
	this->isToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::pushingState, kEventStateMachineWillPushState);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachinePushedState);
	StateMachine::transitionTo(this->stateMachine, State::safeCast(gameState), kStateMachinePushState);
}

void VUEngine::removeState(GameState gameState)
{
#ifdef __TOOLS
	this->isToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::poppingState, kEventStateMachineWillPopState);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachinePoppedState);
	StateMachine::transitionTo(this->stateMachine, State::safeCast(gameState), kStateMachinePopState);
}

bool VUEngine::cleaniningStatesStack(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::turnDisplayOff(HardwareManager::getInstance());
	HardwareManager::stopDrawing(HardwareManager::getInstance());

	// Clean the game's stack
	// pop states until the stack is empty
	VirtualList stateMachineStack = StateMachine::getStateStack(this->stateMachine);

	// Cancel all messages
	VirtualNode node = VirtualList::begin(stateMachineStack);

	for(; NULL != node; node = VirtualNode::getNext(node))
	{
		GameState gameState = GameState::safeCast(VirtualNode::getData(node));

		MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(gameState));
		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}

	return false;
}

bool VUEngine::pushingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

#ifdef __TOOLS
	this->isToolStateTransition = NULL != __GET_CAST(ToolState, StateMachine::getNextState(this->stateMachine));
#endif

	HardwareManager::turnDisplayOff(HardwareManager::getInstance());
	HardwareManager::stopDrawing(HardwareManager::getInstance());

	return false;
}

bool VUEngine::swappingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::turnDisplayOff(HardwareManager::getInstance());
	HardwareManager::stopDrawing(HardwareManager::getInstance());

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(currentGameState));
		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}

	return false;
}

bool VUEngine::poppingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::turnDisplayOff(HardwareManager::getInstance());
	HardwareManager::stopDrawing(HardwareManager::getInstance());

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(currentGameState));
		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}

#ifdef __TOOLS
	this->isToolStateTransition = NULL != __GET_CAST(ToolState, currentGameState);
#endif

	return false;
}

bool VUEngine::changedState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

	StateMachine::removeAllEventListeners(this->stateMachine);

	// Reset flags
	this->currentGameCycleEnded = true;
	this->nextGameCycleStarted = true;

	// Save current state
	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(GameState::isVersusMode(currentGameState))
	{
		CommunicationManager::startSyncCycle(this->communicationManager);
	}

	// Make sure everything is properly rendered
	VUEngine::prepareGraphics(this);

	HardwareManager::startDrawing(HardwareManager::getInstance());
	HardwareManager::turnDisplayOn(HardwareManager::getInstance());

	// Fire event
	VUEngine::fireEvent(this, kEventNextStateSet);

	StopwatchManager::reset(StopwatchManager::getInstance());
	FrameRate::reset(this->frameRate);

	return false;
}

// erase engine's current status
void VUEngine::reset(bool resetSounds)
{
#ifdef __ENABLE_PROFILER
	Profiler::reset(Profiler::getInstance());
#endif

	HardwareManager::disableInterrupts();

	// Disable timer

	// disable rendering
	HardwareManager::lowerBrightness(HardwareManager::getInstance());
	HardwareManager::clearScreen(HardwareManager::getInstance());
	VIPManager::removePostProcessingEffects(this->vipManager);

	// reset managers
	WireframeManager::reset(this->wireframeManager);

	if(resetSounds)
	{
		SoundManager::reset(this->soundManager);
	}

	TimerManager::resetMilliseconds(this->timerManager);
	KeypadManager::reset(this->keypadManager);
	CommunicationManager::reset(this->communicationManager);
	StopwatchManager::reset(StopwatchManager::getInstance());
	FrameRate::reset(this->frameRate);

	// the order of reset for the graphics managers must not be changed!
	VIPManager::reset(this->vipManager);
	SpriteManager::reset(this->spriteManager);
	DirectDraw::reset(DirectDraw::getInstance());
	AnimationCoordinatorFactory::reset(AnimationCoordinatorFactory::getInstance());

	HardwareManager::enableInterrupts();
}

#ifdef __TOOLS
void VUEngine::toggleTool(ToolState toolState)
{
	if(VUEngine::isInState(this, GameState::safeCast(toolState)))
	{
		VUEngine::removeState(this, GameState::safeCast(toolState));
	}
	else
	{
		if(VUEngine::isInToolState(this))
		{
			VUEngine::removeState(this, GameState::safeCast(toolState));
		}

		VUEngine::addState(this, GameState::safeCast(toolState));
	}
}

bool VUEngine::checkIfToggleTool(const UserInput* userInput)
{
	ToolState engineToolStates[] =
	{
#ifdef __TOOLS
		ToolState::safeCast(DebugState::getInstance()),
		ToolState::safeCast(StageEditorState::getInstance()),
		ToolState::safeCast(AnimationInspectorState::getInstance()),
		ToolState::safeCast(SoundTestState::getInstance()),
#endif
		NULL
	};

	int32 i = 0;

	for(; engineToolStates[i]; i++)
	{
		// check code to access special feature
		if(ToolState::isKeyCombination(engineToolStates[i], userInput))
		{
			VUEngine::toggleTool(this, engineToolStates[i]);
			return true;
		}
	}

	extern const ToolState _userToolStates[];

	i = 0;

	for(; _userToolStates[i]; i++)
	{
		// check code to access special feature
		if(ToolState::isKeyCombination(_userToolStates[i], userInput))
		{
			VUEngine::toggleTool(this, _userToolStates[i]);
			return true;
		}
	}

	return false;
}

#endif


// process input data according to the actual game status
void VUEngine::processUserInput(GameState currentGameState)
{
	if(!KeypadManager::isEnabled(this->keypadManager))
	{
#ifdef __ENABLE_PROFILER
		Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_INPUT;
#endif

	// poll the user's input
	UserInput userInput = KeypadManager::captureUserInput(this->keypadManager);
	
#ifdef __TOOLS
	if(VUEngine::checkIfToggleTool(this, &userInput))
	{
		return;
	}
#endif

	if(0 != (userInput.pressedKey + userInput.holdKey + userInput.releasedKey) || GameState::processUserInputRegardlessOfInput(currentGameState))
	{
		GameState::processUserInput(currentGameState, &userInput);
	}

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
}

void VUEngine::dispatchDelayedMessages()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_MESSAGES;
#endif

#ifndef __ENABLE_PROFILER
#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	if(_dispatchCycle++ & 1)
#endif
#endif
	{

		MessageDispatcher::dispatchDelayedMessages(MessageDispatcher::getInstance());

#ifdef __ENABLE_PROFILER
		Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_MESSAGES);
#endif

	}
}

// update game's logic subsystem
GameState VUEngine::updateLogic(GameState currentGameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_LOGIC;
#endif

	// update the game's logic
	currentGameState = GameState::safeCast(StateMachine::update(this->stateMachine));

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_LOGIC);
#endif

	return currentGameState;
}

	// Update sound related logic
void VUEngine::updateSound()
{
	SoundManager::update(this->soundManager);

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_SOUND_SETUP;
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_SOUND_SETUP);
#endif
}

// update game's physics subsystem
void VUEngine::updatePhysics(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_PHYSICS;
#endif

	// simulate physics
	GameState::updatePhysics(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_PHYSICS);
#endif
}

void VUEngine::focusCamera()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_CAMERA;
#endif

#ifdef __TOOLS
	if(!VUEngine::isInToolState(this))
	{
#endif
		// position the camera
		Camera::focus(this->camera, true);
#ifdef __TOOLS
	}
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_CAMERA);
#endif
}

void VUEngine::updateTransformations(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_TRANSFORMS;
#endif

	// apply world transformations
	GameState::transform(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_TRANSFORMS);
#endif
}

void VUEngine::updateCollisions(GameState gameState)
{
	// process the collisions after the transformations have taken place
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_COLLISIONS;
#endif

	// process collisions
	GameState::processCollisions(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_COLLISIONS);
#endif
}

bool VUEngine::stream(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STREAMING;
#endif

#ifndef __ENABLE_PROFILER
	bool result = GameState::stream(gameState);
#else
	bool result = false;

	// While we wait for the next game start
	while(!this->nextGameCycleStarted)
	{
		// Stream the heck out of the pending entities
		result = GameState::stream(gameState);
	}

	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif

	return result;
}

void VUEngine::updateFrameRate()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		return;
	}
#endif

	FrameRate::update(this->frameRate);
}

void VUEngine::nextGameCycleStarted(uint16 gameFrameDuration)
{
	this->nextGameCycleStarted = true;

	// focus the camera once collisions are resolved
	VUEngine::focusCamera(this);

	UIContainer uiContainer = VUEngine::getUIContainer(this);

	if(!isDeleted(uiContainer))
	{
		UIContainer::prepareToRender(uiContainer);
	}

	ClockManager::update(this->clockManager, gameFrameDuration);

	FrameRate::gameFrameStarted(this->frameRate, this->currentGameCycleEnded);
}

void VUEngine::nextFrameStarted(uint16 gameFrameDuration)
{
	static uint16 totalTime = 0;

	totalTime += gameFrameDuration;

	TimerManager::nextFrameStarted(this->timerManager, gameFrameDuration * __MICROSECONDS_PER_MILLISECOND);

	if(__MILLISECONDS_PER_SECOND <= totalTime)
	{
		if(NULL != this->events)
		{
			VUEngine::fireEvent(this, kEventVUEngineNextSecondStarted);
		}
#ifdef __SHOW_TIMER_MANAGER_STATUS
		TimerManager::printStatus(this->timerManager, 1, 0);
		TimerManager::nextSecondStarted(this->timerManager);
#endif

		totalTime = 0;

#ifdef __SHOW_STREAMING_PROFILING

		if(!VUEngine::isInToolState(this))
		{
			Printing::resetCoordinates(Printing::getInstance());
			Stage::showStreamingProfiling(VUEngine::getStage(this), 1, 1);
		}
#endif

#ifdef __DEBUG
#ifdef __PRINT_DEBUG_ALERT
		Printing::text(Printing::getInstance(), "DEBUG MODE", 0, (__SCREEN_HEIGHT_IN_CHARS) - 1, NULL);
#endif
#endif

#ifdef __SHOW_CHAR_MEMORY_STATUS
		CharSetManager::print(CharSetManager::getInstance(), 1, 5);
#endif

#ifdef __SHOW_BGMAP_MEMORY_STATUS
		BgmapTextureManager::print(BgmapTextureManager::getInstance(), 1, 5);
		ParamTableManager::print(ParamTableManager::getInstance(), 1 + 27, 5);
#endif

#ifdef __SHOW_MEMORY_POOL_STATUS
		if(!VUEngine::isInToolState(this))
		{
#ifdef __SHOW_DETAILED_MEMORY_POOL_STATUS
			MemoryPool::printDetailedUsage(MemoryPool::getInstance(), 30, 1);
#else
			MemoryPool::printResumedUsage(MemoryPool::getInstance(), 35, 1);
#endif
		}
#endif

#ifdef __SHOW_STACK_OVERFLOW_ALERT
		if(!VUEngine::isInToolState(this))
		{
			Printing::resetCoordinates(Printing::getInstance());
			HardwareManager::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 25, 0, false);
		}
#endif
	}

#ifdef __TOOLS
	if(VUEngine::isInSoundTest(this))
	{
		SoundManager::printPlaybackTime(this->soundManager);
	}
#endif
}

bool VUEngine::hasCurrentFrameEnded()
{
	// raise flag to allow the next frame to start
	return this->currentGameCycleEnded;
}

void VUEngine::run(GameState currentGameState)
{
	// Set state
	VUEngine::setState(this, currentGameState);

	while(NULL != currentGameState)
	{
#ifndef __RELEASE
		VUEngine::debug(this);
#endif

		this->nextGameCycleStarted = false;
		this->currentGameCycleEnded = false;

		VUEngine::updateFrameRate(this);

#ifdef __ENABLE_PROFILER
		HardwareManager::disableInterrupts();
		Profiler::start(Profiler::getInstance());
#endif

		// Generate random seed
		_gameRandomSeed = Math::randomSeed();

		// process user's input
		VUEngine::processUserInput(this, currentGameState);

		// simulate physics
		VUEngine::updatePhysics(this, currentGameState);

		// apply transformations
		VUEngine::updateTransformations(this, currentGameState);

		// process collisions
		VUEngine::updateCollisions(this, currentGameState);

		// dispatch delayed messages
		VUEngine::dispatchDelayedMessages(this);

		currentGameState = VUEngine::updateLogic(this, currentGameState);

#ifdef __ENABLE_PROFILER
		// Update sound related logic if the streaming did nothing
		VUEngine::updateSound(this);

		HardwareManager::enableInterrupts();

		// Stream entities
		VUEngine::stream(this, currentGameState);
#else
		// Give priority to the stream
		if(!VUEngine::stream(this, currentGameState))
		{
			// Update sound related logic if the streaming did nothing
			VUEngine::updateSound(this);
		}

#ifndef __UNLOCK_FPS
		// While we wait for the next game start
		while(!this->nextGameCycleStarted)
		{
			// Stream the heck out of the pending entities
			VUEngine::stream(this, currentGameState);
		}
#endif
#endif

		this->currentGameCycleEnded = true;
	}
}

#ifdef __REGISTER_LAST_PROCESS_NAME
void VUEngine::setLastProcessName(char* processName)
{
	this->lastProcessName = processName;
}
#endif

// process a telegram
bool VUEngine::handleMessage(Telegram telegram)
{
	ASSERT(this->stateMachine, "VUEngine::handleMessage: NULL stateMachine");

	return StateMachine::handleMessage(this->stateMachine, telegram);
}

// retrieve time
uint32 VUEngine::getElapsedMilliseconds()
{
	return Clock::getMilliseconds(this->clock);
}

// retrieve clock
Clock VUEngine::getClock()
{
	return this->clock;
}

void VUEngine::resetClock()
{
	Clock::reset(this->clock);
}

// retrieve in game clock
Clock VUEngine::getMessagingClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getMessagingClock(GameState::safeCast(state));
}

// retrieve animations' clock
Clock VUEngine::getUpdateClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getUpdateClock(GameState::safeCast(state));
}

// retrieve in physics' clock
Clock VUEngine::getPhysicsClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicsClock(GameState::safeCast(state));
}

uint16 VUEngine::getGameFrameDuration()
{
	return VIPManager::getGameFrameDuration(this->vipManager);
}

// retrieve last process' name
char* VUEngine::getLastProcessName()
{
	return this->lastProcessName;
}

void VUEngine::setGameFrameRate(uint16 gameFrameRate)
{
	if(__MAXIMUM_FPS < gameFrameRate)
	{
		gameFrameRate = __MAXIMUM_FPS;
	}

	FrameRate::setTarget(this->frameRate, gameFrameRate);
	VIPManager::setFrameCycle(this->vipManager, __MAXIMUM_FPS / gameFrameRate - 1);
}

#ifdef __TOOLS
bool VUEngine::isInState(GameState gameState)
{
	return StateMachine::getCurrentState(this->stateMachine) == State::safeCast(gameState);
}

bool VUEngine::isInDebugMode()
{
	return VUEngine::isInState(this, GameState::safeCast(DebugState::getInstance()));
}

bool VUEngine::isInStageEditor()
{
	return VUEngine::isInState(this, GameState::safeCast(StageEditorState::getInstance()));
}

bool VUEngine::isInAnimationInspector()
{
	return VUEngine::isInState(this, GameState::safeCast(AnimationInspectorState::getInstance()));
}

bool VUEngine::isInSoundTest()
{
	return VUEngine::isInState(this, GameState::safeCast(SoundTestState::getInstance()));
}
#endif


// whether if a special mode is active
bool VUEngine::isInToolState()
{
	int32 isInToolState = false;

#ifdef __TOOLS
	isInToolState |= VUEngine::isInDebugMode(this);
	isInToolState |= VUEngine::isInStageEditor(this);
	isInToolState |= VUEngine::isInAnimationInspector(this);
	isInToolState |= VUEngine::isInSoundTest(this);
#endif

	return isInToolState;
}

// whether if a special mode is being started
bool VUEngine::isEnteringToolState()
{
	return this->isToolStateTransition;
}

// whether if a special mode is being started
bool VUEngine::isExitingToolState()
{
	return this->isToolStateTransition;
}

// retrieve state machine, use with caution!!!
StateMachine VUEngine::getStateMachine()
{
	return this->stateMachine;
}

// retrieve the current level's stage
Stage VUEngine::getStage()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		return GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getStage(GameState::safeCast(state));
}

// retrieve the current level's stage
UIContainer VUEngine::getUIContainer()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		return GameState::getUIContainer(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getUIContainer(GameState::safeCast(state));
}

// retrieve current state
GameState VUEngine::getCurrentState()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

PhysicalWorld VUEngine::getPhysicalWorld()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
}

CollisionManager VUEngine::getCollisionManager()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
}

// pause
void VUEngine::pause(GameState pauseState)
{
	ASSERT(pauseState, "VUEngine::pause: null pauseState");

	if(pauseState)
	{
		VUEngine::addState(this, pauseState);
		this->isPaused = true;
		// VUEngine::fireEvent(this, kEventGamePaused);
	}
}

// resume game
void VUEngine::unpause(GameState pauseState)
{
	ASSERT(pauseState, "VUEngine::unpause: null pauseState");

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	ASSERT(pauseState == currentGameState, "VUEngine::unpause: pauseState sent is not the current one");

	if(NULL != pauseState && currentGameState == pauseState)
	{
		VUEngine::removeState(this, pauseState);
		this->isPaused = false;
		// VUEngine::fireEvent(this, kEventGameUnpaused);
	}
}

// is game currently paused?
bool VUEngine::isPaused()
{
	return this->isPaused;
}

void VUEngine::disableKeypad()
{
	KeypadManager::disable(this->keypadManager);
}

void VUEngine::enableKeypad()
{
	KeypadManager::enable(this->keypadManager);
}

void VUEngine::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushFrontPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void VUEngine::pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushBackPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void VUEngine::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::removePostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void VUEngine::wait(uint32 milliSeconds)
{
	TimerManager::wait(this->timerManager, milliSeconds);
}

void VUEngine::prepareGraphics()
{
	SpriteManager::prepareAll(this->spriteManager);
}

/**
 * Register the current save data manager. Use NULL if none.
 *
 * @param saveDataManager
 */
void VUEngine::setSaveDataManager(ListenerObject saveDataManager)
{
	this->saveDataManager = saveDataManager;
}

ListenerObject VUEngine::getSaveDataManager()
{
	return this->saveDataManager;
}

#ifdef __ENABLE_PROFILER
void VUEngine::startProfiling()
{
	Profiler::initialize(Profiler::getInstance());
}
#endif

int32 __GAME_ENTRY_POINT(void);
int32 main(void)
{
	return __GAME_ENTRY_POINT();
}
