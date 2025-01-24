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
#include <BodyManager.h>
#include <Camera.h>
#include <Clock.h>
#include <ColliderManager.h>
#include <FrameRate.h>
#include <MessageDispatcher.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <StopwatchManager.h>
#include <Stage.h>
#include <Telegram.h>
#include <VUEngine.h>
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

	this->stage = NULL;
	this->uiContainer = NULL;

	this->logicsClock = new Clock();
	this->messagingClock = new Clock();
	this->animationsClock = new Clock();
	this->physicsClock = new Clock();

	this->stream = true;
	this->transform = true;
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
	GameState::destroyManagers(this);

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
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::enter(void* owner __attribute__ ((unused)))
{
	Printing::resetCoordinates();
	GameState::pauseClocks(this);
	Clock::start(this->messagingClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::execute(void* owner __attribute__ ((unused)))
{
	NM_ASSERT(this->stage, "GameState::execute: null stage");

	if(!Clock::isPaused(this->logicsClock))
	{
		Stage::update(this->stage);
		UIContainer::update(this->uiContainer);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::exit(void* owner __attribute__ ((unused)))
{
	GameState::discardMessages(this, kMessageRestoreFPS); 

	this->stream = true;
	this->transform = true;
	this->updatePhysics = true;
	this->processCollisions = true;

	MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), this->messagingClock);

	GameState::destroyContainers(this);
	GameState::destroyManagers(this);

	// Stop my clocks
	GameState::stopClocks(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::suspend(void* owner __attribute__ ((unused)))
{
	Clock::pause(this->messagingClock, true);

#ifdef __TOOLS
	if(!VUEngine::isInToolStateTransition())
#endif
	{
		// Force all streaming right now of any pending entity
		// to make sure that their components are fully created
		// This must happen before the managers are disabled
		GameState::streamAll(this);
		
		// Make sure collision colliders are not drawn while suspended
		if(!isDeleted(this->componentManagers[kColliderComponent]))
		{
			ColliderManager::hideColliders(this->componentManagers[kColliderComponent]);
		}

		if(!isDeleted(this->stage))
		{
			Stage::suspend(this->stage);
		}

		if(!isDeleted(this->uiContainer))
		{
			UIContainer::suspend(this->uiContainer);
		}

		// Disable the managers
		GameState::disableManagers(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::resume(void* owner __attribute__ ((unused)))
{
	NM_ASSERT(this->stage, "GameState::resume: null stage");

#ifdef __TOOLS
	if(!VUEngine::isInToolStateTransition())
#endif
	{
		// Reset the engine state
		GameState::reset(this, NULL == Stage::getSpec(this->stage)->assets.sounds);

		// Resume the stage
		Stage::resume(this->stage);

		// Resume the UI		
		UIContainer::resume(this->uiContainer);

		// Move the camera to its previous position
		Camera::focus(Camera::getInstance());

		// Force all transformations to take place again
		GameState::transform(this);

		// Force all streaming right now
		GameState::streamAll(this);
	}

	// Unpause clock
	Clock::pause(this->messagingClock, false);

	// Restore the frame rate
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
	GameState::transform(this);

	// Slow down the frame rate briefly to give the CPU a chance to setup everything
	// without the VIP getting in its way
	GameState::changeFramerate(this, __TARGET_FPS >> 1, 100);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ComponentManager GameState::getComponentManager(uint32 componentType)
{
	if(kComponentTypes <= componentType)
	{
		NM_ASSERT(false, "GameState::getComponentManager: invalid type");
		return NULL;
	}
	
	return this->componentManagers[componentType];	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

UIContainer GameState::getUIContainer()
{
	return this->uiContainer;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Stage GameState::getStage()
{
	return this->stage;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock GameState::getLogicsClock()
{
	return this->logicsClock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock GameState::getMessagingClock()
{
	return this->messagingClock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock GameState::getAnimationsClock()
{
	return this->animationsClock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock GameState::getPhysicsClock()
{
	return this->physicsClock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::startClocks()
{
	Clock::start(this->logicsClock);
	Clock::start(this->messagingClock);
	Clock::start(this->animationsClock);
	Clock::start(this->physicsClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pauseClocks()
{
	Clock::pause(this->logicsClock, true);
	Clock::pause(this->messagingClock, true);
	Clock::pause(this->animationsClock, true);
	Clock::pause(this->physicsClock, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpauseClocks()
{
	Clock::pause(this->logicsClock, false);
	Clock::pause(this->messagingClock, false);
	Clock::pause(this->animationsClock, false);
	Clock::pause(this->physicsClock, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::stopClocks()
{
	Clock::stop(this->logicsClock);
	Clock::stop(this->messagingClock);
	Clock::stop(this->animationsClock);
	Clock::stop(this->physicsClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::startLogics()
{
	Clock::start(this->logicsClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pauseLogics()
{
	Clock::pause(this->logicsClock, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpauseLogics()
{
	Clock::pause(this->logicsClock, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::startMessaging()
{
	Clock::start(this->messagingClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pauseMessaging()
{
	Clock::pause(this->messagingClock, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpauseMessaging()
{
	Clock::pause(this->messagingClock, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::startAnimations()
{
	Clock::start(this->logicsClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pauseAnimations()
{
	Clock::pause(this->animationsClock, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpauseAnimations()
{
	Clock::pause(this->animationsClock, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::startPhysics()
{
	Clock::start(this->physicsClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::pausePhysics()
{
	Clock::pause(this->physicsClock, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::unpausePhysics()
{
	Clock::pause(this->physicsClock, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::transform()
{
	if(!this->transform || NULL == this->stage)
	{
		return;
	}

	extern Transformation _neutralEnvironmentTransformation;

	Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(Camera::getInstance()));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::transformUI()
{
	if((!this->transform && __VALID_TRANSFORMATION == Camera::getTransformationFlags(Camera::getInstance())) || NULL == this->uiContainer)
	{
		return;
	}

	extern Transformation _neutralEnvironmentTransformation;

	UIContainer::transform(this->uiContainer, NULL, __INVALIDATE_TRANSFORMATION);
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
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::propagateMessage(int32 message)
{	
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

bool GameState::stream()
{
	if(!this->stream)
	{
		return false;
	}

	NM_ASSERT(this->stage, "GameState::stream: null stage");

	return Stage::stream(this->stage);
}

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

	GameState::createManagers(this);
	GameState::enableManagers(this);
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

void GameState::destroyManagers()
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

	// Setup ui if allocated and constructed
	if(!isDeleted(this->uiContainer))
	{
		extern Transformation _neutralEnvironmentTransformation;
	
		// Apply transformations
		UIContainer::transform(this->uiContainer, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(Camera::getInstance()));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::streamAll()
{
	// Make sure that the focus actor is transformed before focusing the camera
	GameState::transform(this);

	// Move the camera to its initial position
	Camera::focus(Camera::getInstance());

	// Invalidate transformations
	Stage::invalidateTransformation(this->stage);

	// Transformation everything
	GameState::transform(this);

	// Stream in and out all relevant actors
	Stage::streamAll(this->stage);

	// Force collision purging
	if(!isDeleted(this->componentManagers[kColliderComponent]))
	{
		ColliderManager::purgeDestroyedColliders(this->componentManagers[kColliderComponent]);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
