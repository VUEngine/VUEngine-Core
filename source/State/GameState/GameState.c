/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BehaviorManager.h>
#include <BgmapTextureManager.h>
#include <BodyManager.h>
#include <Camera.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ColliderManager.h>
#include <FrameBufferManager.h>
#include <FrameRate.h>
#include <KeypadManager.h>
#include <MessageDispatcher.h>
#include <MutatorManager.h>
#include <ParamTableManager.h>
#include <Printer.h>
#include <Profiler.h>
#include <SpriteManager.h>
#include <StopwatchManager.h>
#include <Stage.h>
#include <Telegram.h>
#include <ToolState.h>
#include <VUEngine.h>
#include <VSUManager.h>
#include <WireframeManager.h>

#include "GameState.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->lockFrameRate = true;
	this->stage = NULL;
	this->uiContainer = NULL;

	this->logicsClock = new Clock();
	this->messagingClock = new Clock();
	this->animationsClock = new Clock();
	this->physicsClock = new Clock();

	this->stream = true;
	this->transform = true;
	this->processMutators = true;
	this->processBehaviors = true;
	this->updatePhysics = true;
	this->processCollisions = true;

	this->framerate = __TARGET_FPS;

	for(int16 i = 0; i < kComponentTypes; i++)
	{
		this->componentManagers[i] = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::destructor()
{
	MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), this->messagingClock);

	if(!isDeleted(this->messagingClock))
	{
		delete this->messagingClock;
		this->messagingClock = NULL;
	}

	if(!isDeleted(this->logicsClock))
	{
		delete this->logicsClock;
		this->logicsClock = NULL;
	}

	if(!isDeleted(this->animationsClock))
	{
		delete this->animationsClock;
		this->animationsClock = NULL;
	}

	if(!isDeleted(this->physicsClock))
	{
		delete this->physicsClock;
		this->physicsClock = NULL;
	}

	GameState::destroyContainers(this);
	GameState::destroyComponentManagers(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageRestoreFPS:

			VUEngine::setGameFrameRate(this->framerate);
			break;

		case kMessageEnableKeypad:

			KeypadManager::enable();
			break;

	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::start(void* owner)
{
	// Setup managers
	GameState::createManagers(this);
	GameState::enableManagers(this);

	GameState::pauseClocks(this);

	GameState::startClock(this, kGameStateMessagingClock);
	GameState::startClock(this, kGameStateAnimationsClock);

	// Call custom code implementation
	GameState::enter(this, owner);

	// Make sure that the rendering is up to date with any change made by the 
	// derive implementation of resume
	GameState::invalidateRendering(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::update(void* owner)
{
	GameState::render(this);
	
	GameState::focusCamera(this);

	GameState::applyTransformationsUI(this);

	GameState::updateStage(this);

	GameState::readUserInput(this);

	GameState::execute(this, owner);

	GameState::processBehaviors(this);

	GameState::processMutators(this);

	GameState::simulatePhysics(this);

	GameState::applyTransformations(this);

	GameState::processCollisions(this);

	GameState::dispatchDelayedMessages(this);

	GameState::updateSounds(this);

	GameState::stream(this);

#ifdef __DEBUGGING
	GameState::debugging(this);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::stop(void* owner)
{
	GameState::discardMessages(this, kMessageRestoreFPS); 

	this->stream = true;
	this->transform = true;
	this->updatePhysics = true;
	this->processMutators = true;
	this->processBehaviors = true;
	this->processCollisions = true;

	MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), this->messagingClock);

	GameState::destroyContainers(this);
	GameState::destroyComponentManagers(this);

	GameState::stopClocks(this);

	// Call custom code implementation
	GameState::exit(this, owner);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pause(void* owner)
{
	GameState::pauseClock(this, kGameStateMessagingClock);

#ifdef __TOOLS
	ToolState toolState = VUEngine::getActiveToolState();

	if(NULL == toolState)
#endif
	{
		// Force all streaming right now of any pending entity to make sure that their components are fully created
		// This must happen before the managers are disabled
		GameState::streamAll(this);
		
		if(!isDeleted(this->stage))
		{
			Stage::suspend(this->stage);
		}

		if(!isDeleted(this->uiContainer))
		{
			UIContainer::suspend(this->uiContainer);
		}

		GameState::disableManagers(this);

		// Call custom code implementation
		GameState::suspend(this, owner);
	}
#ifdef __TOOLS
	else
	{
		ToolState::configure(toolState, this, this->stage);		
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpause(void* owner)
{
	NM_ASSERT(this->stage, "GameState::resume: null stage");

#ifdef __TOOLS
	ToolState toolState = VUEngine::getActiveToolState();

	if(NULL == toolState)
#endif
	{
		// Setup managers
		GameState::createManagers(this);
		GameState::enableManagers(this);

		// Reset the engine state
		GameState::reset(this, NULL == Stage::getSpec(this->stage)->assets.sounds);

		// Configure the game state
		GameState::configure(this);

		// Resume the stage
		Stage::resume(this->stage);

		// Resume the UI		
		UIContainer::resume(this->uiContainer);

		// Move the camera to its previous position
		Camera::focus(Camera::getInstance());

		// Force all transformations to take place again
		GameState::applyTransformations(this);

		// Force all streaming right now
		GameState::streamAll(this);
		
		// Call custom code implementation
		GameState::resume(this, owner);

		// Make sure that the rendering is up to date with any change made by the 
		// derive implementation of resume
		GameState::invalidateRendering(this);
	}
#ifdef __TOOLS
	else
	{
		KeypadManager::disable();
		GameState::sendMessageToSelf(this, kMessageEnableKeypad, 500, 0);
	}
#endif

	GameState::unpauseClock(this, kGameStateMessagingClock);

	GameState::changeFramerate(this, this->framerate, -1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::processMessage(void* owner __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	return false;
	// Not sure if necessary, but this can cause problems if no unified messages list is used and can cause unintended performance issues	
	// Return Stage::propagateMessage
	// (
	//	this->stage, Container::onPropagatedMessage, Telegram::getMessage(telegram)) || UIContainer::propagateMessage(this->uiContainer, 
	//	Container::onPropagatedMessage, Telegram::getMessage(telegram)
	// );
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configureStage(StageSpec* stageSpec, VirtualList positionedActorsToIgnore)
{
	if(NULL == stageSpec)
	{
		extern StageROMSpec EmptyStageSpec;

		stageSpec = (StageSpec*)&EmptyStageSpec;
	}

	// Reset the engine state
	GameState::reset(this, NULL == stageSpec->assets.sounds);

	// Make sure no actor is set as focus for the camera
	Camera::setFocusActor(Camera::getInstance(), NULL);

	// Setup the stage
	GameState::createStage(this, stageSpec, positionedActorsToIgnore);

	// Load the UI
	GameState::configureUI(this, stageSpec);

	// Move the camera to its previous position
	Camera::focus(Camera::getInstance());

	// Transformation everything
	GameState::applyTransformations(this);

	// Slow down the frame rate briefly to give the CPU a chance to setup everything
	// without the VIP getting in its way
	GameState::changeFramerate(this, __TARGET_FPS >> 1, 100);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::purgeComponentManagers()
{
	for(int16 i = 0; i < kComponentTypes; i++)
	{
		if(isDeleted(this->componentManagers[i]))
		{
			continue;
		}

		ComponentManager::purgeComponents(this->componentManagers[i]);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

UIContainer GameState::getUIContainer()
{
	return this->uiContainer;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::startClocks()
{
	for(uint32 i = 0; i < kGameStateNoClock; i++)
	{
		GameState::startClock(this, i);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pauseClocks()
{
	for(uint32 i = 0; i < kGameStateNoClock; i++)
	{
		GameState::pauseClock(this, i);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpauseClocks()
{
	for(uint32 i = 0; i < kGameStateNoClock; i++)
	{
		GameState::unpauseClock(this, i);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::stopClocks()
{
	for(uint32 i = 0; i < kGameStateNoClock; i++)
	{
		GameState::stopClock(this, i);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::startClock(uint32 clockEnum)
{
	Clock clock = GameState::getClock(this, clockEnum);

	if(!isDeleted(clock))
	{
		Clock::start(clock);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pauseClock(uint32 clockEnum)
{
	Clock clock = GameState::getClock(this, clockEnum);

	if(!isDeleted(clock))
	{
		Clock::pause(clock, true);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpauseClock(uint32 clockEnum)
{
	Clock clock = GameState::getClock(this, clockEnum);

	if(!isDeleted(clock))
	{
		Clock::pause(clock, false);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::stopClock(uint32 clockEnum)
{
	Clock clock = GameState::getClock(this, clockEnum);

	if(!isDeleted(clock))
	{
		Clock::stop(clock);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::propagateMessage(int32 message)
{
	if(NULL == this->stage)
	{
		return false;
	}

	return 
		Stage::propagateMessage
		(
			this->stage, Container::onPropagatedMessage, message) || UIContainer::propagateMessage(this->uiContainer, 
			Container::onPropagatedMessage, message
		);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::propagateString(const char* string)
{
	return 
		Stage::propagateString
		(
			this->stage, Container::onPropagatedString, string) || Container::propagateString(this->uiContainer, 
			Container::onPropagatedString, string
		);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Actor GameState::getActorByName(const char* actorName)
{
	if(isDeleted(this->stage))
	{
		return NULL;
	}
	
	return Actor::safeCast(Stage::getChildByName(this->stage, actorName, false));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::showActorWithName(const char* actorName)
{
	if(isDeleted(this->stage))
	{
		return;
	}
	
	Actor actor = Actor::safeCast(Stage::getChildByName(this->stage, actorName, false));

	if(!isDeleted(actor))
	{
		Actor::show(actor);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::hideActorWithName(const char* actorName)
{
	if(isDeleted(this->stage))
	{
		return;
	}
	
	Actor actor = Actor::safeCast(Stage::getChildByName(this->stage, actorName, false));

	if(!isDeleted(actor))
	{
		Actor::hide(actor);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::changeFramerate(int16 targetFPS, int32 duration)
{
	GameState::discardMessages(this, kMessageRestoreFPS);
	
	VUEngine::setGameFrameRate(targetFPS);

	if(0 < duration)
	{
		GameState::sendMessageToSelf(this, kMessageRestoreFPS, duration + 1, 0);
	}
	else
	{
		this->framerate = targetFPS;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::streamAll()
{
	// Make sure that the focus actor is transformed before focusing the camera
	GameState::applyTransformations(this);

	// Move the camera to its initial position
	Camera::focus(Camera::getInstance());

	// Invalidate transformations
	Stage::invalidateTransformation(this->stage);

	// Transformation everything
	GameState::applyTransformations(this);

	// Stream in and out all relevant actors
	Stage::streamAll(this->stage);

	// Be sure that the manager's removed components are deleted
	GameState::purgeComponentManagers(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::lockFrameRate()
{
	return this->lockFrameRate;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::printClocks(int16 x, int16 y)
{
	Printer::text("Logics clock time: ", x, ++y, NULL);
	Clock::print(this->logicsClock, x + 26, y, NULL);
	Printer::text("Messagging clock time: ", x, ++y, NULL);
	Clock::print(this->messagingClock, x + 26, y, NULL);
	Printer::text("Animations clock's time: ", x, ++y, NULL);
	Clock::print(this->animationsClock, x + 26, y, NULL);
	Printer::text("Physics clock's time: ", x, ++y, NULL);
	Clock::print(this->physicsClock, x + 26, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::enter(void* owner __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::execute(void* owner __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::exit(void* owner __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::suspend(void* owner __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::resume(void* owner __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::processUserInput(const UserInput* userInput __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::isVersusMode()
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//—————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::reset(bool resetSounds)
{
	HardwareManager::reset();

	FrameRate::reset(FrameRate::getInstance());
	StopwatchManager::reset(StopwatchManager::getInstance());

	if(resetSounds)
	{
		SoundManager::reset(SoundManager::getInstance());
	}

#ifdef __ENABLE_PROFILER
	Profiler::reset();
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::destroyContainers()
{
	if(!isDeleted(this->stage))
	{
		delete this->stage;
		this->stage = NULL;
	}

	if(!isDeleted(this->uiContainer))
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::createManagers()
{
	for(int16 i = 0; i < kComponentTypes; i++)
	{
		if(NULL == this->componentManagers[i])
		{
			AllocatorPointer _componentManagerAllocators[kComponentTypes];
			_componentManagerAllocators[kMutatorComponent] = __TYPE(MutatorManager);
			_componentManagerAllocators[kBehaviorComponent] = __TYPE(BehaviorManager);
			_componentManagerAllocators[kPhysicsComponent] = __TYPE(BodyManager);
			_componentManagerAllocators[kColliderComponent] = __TYPE(ColliderManager);
			_componentManagerAllocators[kSpriteComponent] = __TYPE(SpriteManager);
			_componentManagerAllocators[kWireframeComponent] = __TYPE(WireframeManager);

			this->componentManagers[i] = ComponentManager::safeCast(_componentManagerAllocators[i]());
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::destroyComponentManagers()
{
	for(int16 i = 0; i < kComponentTypes; i++)
	{
		if(isDeleted(this->componentManagers[i]))
		{
			continue;
		}

		delete this->componentManagers[i];
		this->componentManagers[i] = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::enableManagers()
{
	for(int16 i = 0; i < kComponentTypes; i++)
	{
		if(NULL == this->componentManagers[i])
		{
			return;
		}

		ComponentManager::enable(this->componentManagers[i]);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::disableManagers()
{
	for(int16 i = 0; i < kComponentTypes; i++)
	{
		if(NULL == this->componentManagers[i])
		{
			return;
		}

		ComponentManager::disable(this->componentManagers[i]);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::createStage(StageSpec* stageSpec, VirtualList positionedActorsToIgnore)
{
	if(!isDeleted(this->stage))
	{
		delete this->stage;
		this->stage = NULL;
	}

	this->stage = ((Stage (*)(StageSpec*, GameState)) stageSpec->allocator)((StageSpec*)stageSpec, this);
	
	NM_ASSERT(!isDeleted(this->stage), "GameState::configureStage: null stage");

	GameState::configure(this);

	Stage::configure(this->stage, positionedActorsToIgnore);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configureUI(StageSpec* stageSpec)
{
	if(!isDeleted(this->uiContainer))
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	if(NULL != stageSpec->actors.UI.allocator)
	{
		this->uiContainer = 
			((UIContainer (*)(PositionedActor*)) stageSpec->actors.UI.allocator)(stageSpec->actors.UI.childrenSpecs);
	}
	else
	{
		this->uiContainer = new UIContainer(NULL);
	}

	NM_ASSERT(!isDeleted(this->uiContainer), "GameState::configureUI: null UIContainer");

	if(!isDeleted(this->uiContainer))
	{
		extern Transformation _neutralEnvironmentTransformation;
	
		UIContainer::transform(this->uiContainer, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(Camera::getInstance()));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::focusCamera()
{
	Camera::focus(Camera::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::render()
{
	SpriteManager::render(this->componentManagers[kSpriteComponent]);
	
	WireframeManager::render(this->componentManagers[kWireframeComponent]);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::applyTransformationsUI()
{
	if((!this->transform && __VALID_TRANSFORMATION == Camera::getTransformationFlags(Camera::getInstance())) || NULL == this->uiContainer)
	{
		return;
	}

	extern Transformation _neutralEnvironmentTransformation;

	UIContainer::transform(this->uiContainer, NULL, __INVALIDATE_TRANSFORMATION);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::updateStage()
{
	if(!Clock::isPaused(this->logicsClock))
	{
		Stage::update(this->stage);
		UIContainer::update(this->uiContainer);
	}

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_EXECUTE_STATE);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::readUserInput()
{
	if(!KeypadManager::isEnabled(KeypadManager::getInstance()))
	{
		return;
	}

	KeypadManager::readUserInput(KeypadManager::getInstance(), this->lockFrameRate);

	UserInput userInput = KeypadManager::getUserInput();

	if(0 != (userInput.dummyKey | userInput.pressedKey | userInput.holdKey | userInput.releasedKey))
	{
		GameState::processUserInput(this, &userInput);

#ifdef __ENABLE_PROFILER
		Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::processMutators()
{
	if(!this->processMutators || isDeleted(this->componentManagers[kMutatorComponent]))
	{
		return;
	}

	MutatorManager::update(this->componentManagers[kMutatorComponent]);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_MUTATORS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::processBehaviors()
{
	if(!this->processBehaviors || isDeleted(this->componentManagers[kBehaviorComponent]))
	{
		return;
	}

	if(Clock::isPaused(this->logicsClock))
	{
		return;
	}

	BehaviorManager::update(this->componentManagers[kBehaviorComponent]);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_BEHAVIORS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::simulatePhysics()
{
	if(!this->updatePhysics || isDeleted(this->componentManagers[kPhysicsComponent]))
	{
		return;
	}

	if(Clock::isPaused(this->physicsClock))
	{
		return;
	}

	BodyManager::update(this->componentManagers[kPhysicsComponent]);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_PHYSICS);
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::applyTransformations()
{
	if(!this->transform || NULL == this->stage)
	{
		return;
	}

	extern Transformation _neutralEnvironmentTransformation;

	Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(Camera::getInstance()));

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_TRANSFORMS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::processCollisions()
{
	if(!this->processCollisions || isDeleted(this->componentManagers[kColliderComponent]))
	{
		return;
	}

	if(Clock::isPaused(this->physicsClock))
	{
		return;
	}

	ColliderManager::update(this->componentManagers[kColliderComponent]);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_COLLISIONS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::dispatchDelayedMessages()
{
	MessageDispatcher::dispatchDelayedMessages(MessageDispatcher::getInstance());

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_MESSAGES);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::updateSounds()
{
	SoundManager::updateSounds(SoundManager::getInstance());

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_SOUND_PURGE);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::stream()
{
	if(!this->stream)
	{
		return;
	}

#ifndef __DEBUG
	while(Stage::stream(this->stage) && !VUEngine::hasGameFrameStarted());
#else
	while(Stage::stream(this->stage));
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::invalidateRendering()
{
	SpriteManager::invalidateRendering(this->componentManagers[kSpriteComponent]);
	WireframeManager::invalidateRendering(this->componentManagers[kWireframeComponent]);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configure()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	GameState::configureMessaging(this);
	GameState::configureTimer(this);
	GameState::configureGraphics(this);
	GameState::configurePhysics(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configureMessaging()
{
	MessageDispatcher::setClock(MessageDispatcher::getInstance(), this->messagingClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configureTimer()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	const StageSpec* stageSpec = Stage::getSpec(this->stage);

	TimerManager::configure
	(
		stageSpec->timer.resolution, stageSpec->timer.targetTimePerInterrupt, 
		stageSpec->timer.targetTimePerInterrupttUnits
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configureGraphics()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	const StageSpec* stageSpec = Stage::getSpec(this->stage);

	SpriteManager::configure
	(
		this->componentManagers[kSpriteComponent],
		stageSpec->rendering.texturesMaximumRowsToWrite,
		stageSpec->rendering.maximumAffineRowsToComputePerCall,
		stageSpec->rendering.objectSpritesContainersSize,
		stageSpec->rendering.objectSpritesContainersZPosition,
		this->animationsClock
	);

	VIPManager::configure
	(
		VIPManager::getInstance(), 
		stageSpec->rendering.colorConfig.backgroundColor,
		&stageSpec->rendering.colorConfig.brightness,
		stageSpec->rendering.colorConfig.brightnessRepeat,
		&stageSpec->rendering.paletteConfig,
		stageSpec->postProcessingEffects
	);

	BgmapTextureManager::configure
	(
		BgmapTextureManager::getInstance(), 
		ParamTableManager::configure
		(
			ParamTableManager::getInstance(), stageSpec->rendering.paramTableSegments
		)
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configurePhysics()
{
	if(isDeleted(this->stage))
	{
		return;
	}

	const StageSpec* stageSpec = Stage::getSpec(this->stage);

	BodyManager::setFrictionCoefficient(this->componentManagers[kPhysicsComponent], stageSpec->physics.frictionCoefficient);
	BodyManager::setGravity(this->componentManagers[kPhysicsComponent], stageSpec->physics.gravity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock GameState::getClock(uint32 clockEnum)
{
	switch(clockEnum)
	{
		case kGameStateAnimationsClock:
		{
			return this->animationsClock;
		}

		case kGameStateLogicsClock:
		{
			return this->logicsClock;
		}

		case kGameStateMessagingClock:
		{
			return this->messagingClock;
		}

		case kGameStatePhysicsClock:
		{
			return this->physicsClock;
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
#ifdef __DEBUGGING
void GameState::debugging()
{
	Printer::resetCoordinates();
	Printer::setWorldCoordinates(0, 0, -256, 0);

#ifdef __DEBUGGING_STACK_STATUS
	HardwareManager::printStackStatus(1, 1, false);
#endif	

#ifdef __DEBUGGING_MEMORY_POOL
#ifdef __DEBUGGING_MEMORY_POOL_DETAILED
	MemoryPool::printDetailedUsage(1, 1);
#else
	MemoryPool::printResumedUsage(1, 1);
#endif
#endif

#ifdef __DEBUGGING_CHAR_MEMORY
	CharSetManager::print(1, 1);
#endif

#ifdef __DEBUGGING_BGMAP_MEMORY
	BgmapTextureManager::print(1, 1);
	ParamTableManager::print(26, 1);
#endif

#ifdef __DEBUGGING_VIP
	VIPManager::print(1, 1);
#endif

#ifdef __DEBUGGING_VSU
	VSUManager::print(1, 1);
#endif

#ifdef __DEBUGGING_FRAME_BUFFERS
	FrameBufferManager::print(FrameBufferManager::getInstance(), 1, 1);
#endif

#ifdef __DEBUGGING_SPRITES
	SpriteManager::print(this->componentManagers[kSpriteComponent], 1, 1, true);
#endif
	
#ifdef __DEBUGGING_WIREFRAMES
	WireframeManager::print(this->componentManagers[kWireframeComponent], 1, 1);
#endif

#ifdef __DEBUGGING_PHYSICS
	BodyManager::print(this->componentManagers[kPhysicsComponent], 1, 1);
#endif
	
#ifdef __DEBUGGING_COLLISIONS
	ColliderManager::print(this->componentManagers[kColliderComponent], 1, 1);
#endif

#ifdef __DEBUGGING_STREAMING
	Stage::print(this->stage, 1, 1);
#endif
}
#endif	
#endif	
