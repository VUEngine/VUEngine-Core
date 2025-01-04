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

#include <Camera.h>
#include <Clock.h>
#include <ColliderManager.h>
#include <BodyManager.h>
#include <Printing.h>
#include <Stage.h>
#include <SpriteManager.h>
#include <Telegram.h>
#include <VIPManager.h>
#include <VUEngine.h>

#include "GameState.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Camera _camera = NULL;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::constructor()
{
	_camera = Camera::getInstance();

	// Always explicitly call the base's constructor 
	Base::constructor();

	this->stage = NULL;
	this->uiContainer = NULL;

	this->logicsClock = new Clock();
	this->messagingClock = new Clock();
	this->animationsClock = new Clock();
	this->physicsClock = new Clock();

	// construct the physical world and collision manager
	this->bodyManager = NULL;
	this->colliderManager = NULL;

	this->stream = true;
	this->transform = true;
	this->updatePhysics = true;
	this->processCollisions = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::destructor()
{
	delete this->logicsClock;
	delete this->messagingClock;
	delete this->animationsClock;
	delete this->physicsClock;

	// destroy the stage
	if(!isDeleted(this->stage))
	{
		// destroy the stage
		delete this->stage;

		this->stage = NULL;
	}

	if(!isDeleted(this->uiContainer))
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	// must delete these after deleting the stage
	if(!isDeleted(this->bodyManager))
	{
		delete this->bodyManager;
		this->bodyManager = NULL;
	}

	if(!isDeleted(this->colliderManager))
	{
		delete this->colliderManager;
		this->colliderManager = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageRestoreFPS:

			VUEngine::setGameFrameRate(VUEngine::getInstance(), __TARGET_FPS);
			break;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::enter(void* owner __attribute__ ((unused)))
{
	VIPManager::removePostProcessingEffects(VIPManager::getInstance());
	Printing::resetCoordinates(Printing::getInstance());

	GameState::pauseClocks(this);
	SpriteManager::setAnimationsClock(SpriteManager::getInstance(), this->animationsClock);

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

	if(!isDeleted(this->bodyManager))
	{
		delete this->bodyManager;
		this->bodyManager = NULL;
	}

	if(!isDeleted(this->colliderManager))
	{
		delete this->colliderManager;
		this->colliderManager = NULL;
	}

	this->stage = NULL;

	// stop my clocks
	GameState::stopClocks(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::suspend(void* owner __attribute__ ((unused)))
{
	Clock::pause(this->messagingClock, true);

#ifdef __TOOLS
	if(!VUEngine::isInToolStateTransition(VUEngine::getInstance()))
#endif
	{
		// Make sure collision colliders are not drawn while suspended
		if(this->colliderManager)
		{
			ColliderManager::hideColliders(this->colliderManager);
		}

		if(!isDeleted(this->stage))
		{
			Stage::suspend(this->stage);
		}

		if(!isDeleted(this->uiContainer))
		{
			UIContainer::suspend(this->uiContainer);
		}

		// Make sure that all graphical resources are released.
		SpriteManager::reset(SpriteManager::getInstance());
		SpriteManager::setAnimationsClock(SpriteManager::getInstance(), this->animationsClock);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::resume(void* owner __attribute__ ((unused)))
{
	NM_ASSERT(this->stage, "GameState::resume: null stage");

	HardwareManager::suspendInterrupts();

#ifdef __TOOLS
	if(!VUEngine::isInToolStateTransition(VUEngine::getInstance()))
#endif
	{
		// Reset the engine state
		VUEngine::reset(VUEngine::getInstance(), NULL == Stage::getSpec(this->stage)->assets.sounds);

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

	// unpause clock
	Clock::pause(this->messagingClock, false);

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameState::processMessage(void* owner __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	return false;
	// Not sure if necessary, but this can cause problems if no unified messages list is used and can cause unintended performance issues	
	// return Stage::propagateMessage
	// (
	//	this->stage, Container::onPropagatedMessage, Telegram::getMessage(telegram)) || UIContainer::propagateMessage(this->uiContainer, 
	//	Container::onPropagatedMessage, Telegram::getMessage(telegram)
	// );
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configureStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore)
{
	if(NULL == stageSpec)
	{
		extern StageROMSpec EmptyStageSpec;

		stageSpec = (StageSpec*)&EmptyStageSpec;
	}

	// Reset the engine state
	VUEngine::reset(VUEngine::getInstance(), NULL == stageSpec->assets.sounds);

	HardwareManager::suspendInterrupts();

	// make sure no entity is set as focus for the camera
	Camera::setFocusEntity(Camera::getInstance(), NULL);

	// setup the stage
	GameState::createStage(this, stageSpec, positionedEntitiesToIgnore);

	// load the UI
	GameState::configureUI(this, stageSpec);

	// move the camera to its previous position
	Camera::focus(Camera::getInstance());

	// transformation everything
	GameState::transform(this);

	// Transform everything definitively
	GameState::transform(this);

	// Make sure all graphics are ready
	VUEngine::prepareGraphics(VUEngine::getInstance());

	HardwareManager::resumeInterrupts();

	GameState::changeFramerate(this, __TARGET_FPS >> 1, 100);
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

BodyManager GameState::getBodyManager()
{
	if(NULL == this->bodyManager)
	{
		this->bodyManager = new BodyManager();
	}

	return this->bodyManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ColliderManager GameState::getColliderManager()
{
	if(NULL == this->colliderManager)
	{
		this->colliderManager = new ColliderManager();
	}

	return this->colliderManager;
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
	if(!this->transform)
	{
		return;
	}

	NM_ASSERT(this->stage, "GameState::transform: null stage");

	extern Transformation _neutralEnvironmentTransformation;

	Stage::transform(this->stage, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(_camera));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::transformUI()
{
	if(!this->transform && __VALID_TRANSFORMATION == Camera::getTransformationFlags(Camera::getInstance()))
	{
		return;
	}

	NM_ASSERT(this->uiContainer, "GameState::transform: null uiContainer");

	extern Transformation _neutralEnvironmentTransformation;

	UIContainer::transform(this->uiContainer, NULL, __INVALIDATE_TRANSFORMATION);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::simulatePhysics()
{
	if(!this->updatePhysics || isDeleted(this->bodyManager))
	{
		return;
	}

	if(Clock::isPaused(this->physicsClock))
	{
		return;
	}

	BodyManager::update(this->bodyManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::processCollisions()
{
	if(!this->processCollisions || isDeleted(this->colliderManager))
	{
		return;
	}

	if(Clock::isPaused(this->physicsClock))
	{
		return;
	}

	ColliderManager::update(this->colliderManager);
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

Entity GameState::getEntityByName(const char* entityName)
{
	if(isDeleted(this->stage))
	{
		return NULL;
	}
	
	return Entity::safeCast(Stage::getChildByName(this->stage, entityName, false));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::showEntityWithName(const char* entityName)
{
	if(isDeleted(this->stage))
	{
		return;
	}
	
	Entity entity = Entity::safeCast(Stage::getChildByName(this->stage, entityName, false));

	if(!isDeleted(entity))
	{
		Entity::show(entity);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::hideEntityWithName(const char* entityName)
{
	if(isDeleted(this->stage))
	{
		return;
	}
	
	Entity entity = Entity::safeCast(Stage::getChildByName(this->stage, entityName, false));

	if(!isDeleted(entity))
	{
		Entity::hide(entity);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::changeFramerate(int16 targetFPS, int32 duration)
{
	GameState::discardMessages(this, kMessageRestoreFPS);
	
	VUEngine::setGameFrameRate(VUEngine::getInstance(), targetFPS);

	if(0 < duration)
	{
		GameState::sendMessageToSelf(this, kMessageRestoreFPS, duration + 1, 0);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::createStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore)
{
	if(!isDeleted(this->stage))
	{
		delete this->stage;
		this->stage = NULL;
	}

	this->stage = ((Stage (*)(StageSpec*)) stageSpec->allocator)((StageSpec*)stageSpec);
	
	NM_ASSERT(!isDeleted(this->stage), "GameState::configureStage: null stage");

	Stage::configure(this->stage, positionedEntitiesToIgnore);

	SpriteManager::setAnimationsClock(SpriteManager::getInstance(), this->animationsClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::configureUI(StageSpec* stageSpec)
{
	if(!isDeleted(this->uiContainer))
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	if(NULL != stageSpec->entities.UI.allocator)
	{
		this->uiContainer = 
			((UIContainer (*)(PositionedEntity*)) stageSpec->entities.UI.allocator)(stageSpec->entities.UI.childrenSpecs);
	}
	else
	{
		this->uiContainer = new UIContainer(NULL);
	}

	NM_ASSERT(!isDeleted(this->uiContainer), "GameState::configureUI: null UIContainer");

	// setup ui if allocated and constructed
	if(!isDeleted(this->uiContainer))
	{
		extern Transformation _neutralEnvironmentTransformation;
	
		// apply transformations
		UIContainer::transform(this->uiContainer, &_neutralEnvironmentTransformation, Camera::getTransformationFlags(_camera));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameState::streamAll()
{
	HardwareManager::suspendInterrupts();

	// Make sure that the focus entity is transformed before focusing the camera
	GameState::transform(this);

	// Move the camera to its initial position
	Camera::focus(Camera::getInstance());

	// invalidate transformations
	Stage::invalidateTransformation(this->stage);

	// Transformation everything
	GameState::transform(this);

	// Stream in and out all relevant entities
	Stage::streamAll(this->stage);

	// Force collision purging
	if(!isDeleted(this->colliderManager))
	{
		ColliderManager::purgeDestroyedColliders(this->colliderManager);
	}

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

