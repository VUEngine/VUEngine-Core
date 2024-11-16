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

#include <Camera.h>
#include <Clock.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <Printing.h>
#include <Stage.h>
#include <SpriteManager.h>
#include <Telegram.h>
#include <VIPManager.h>
#include <VUEngine.h>

#include "GameState.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S STATICS
//---------------------------------------------------------------------------------------------------------

static Camera _camera = NULL;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void GameState::constructor()
{
	_camera = Camera::getInstance();

	Base::constructor();

	this->stage = NULL;
	this->uiContainer = NULL;

	// clocks
	this->messagingClock = new Clock();
	this->updateClock = new Clock();
	this->physicsClock = new Clock();

	// construct the physical world and collision manager
	this->physicalWorld = NULL;
	this->collisionManager = NULL;

	this->stream = true;
	this->transform = true;
	this->updatePhysics = true;
	this->processCollisions = true;
}

/**
 * Class destructor
 */
void GameState::destructor()
{
	delete this->messagingClock;
	delete this->updateClock;
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
	if(!isDeleted(this->physicalWorld))
	{
		delete this->physicalWorld;
		this->physicalWorld = NULL;
	}

	if(!isDeleted(this->collisionManager))
	{
		delete this->collisionManager;
		this->collisionManager = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @param owner		StateMachine's owner
 */
void GameState::enter(void* owner __attribute__ ((unused)))
{
	VIPManager::removePostProcessingEffects(VIPManager::getInstance());
	Printing::resetCoordinates(Printing::getInstance());

	GameState::pauseClocks(this);

	Clock::start(this->messagingClock);
}

/**
 * Method called when by the StateMachine's update method
 *
 * @param owner		StateMachine's owner
 */
void GameState::execute(void* owner __attribute__ ((unused)))
{
	NM_ASSERT(this->stage, "GameState::execute: null stage");

	if(!Clock::isPaused(this->updateClock))
	{
		Stage::update(this->stage);
		UIContainer::update(this->uiContainer);
	}
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @param owner		StateMachine's owner
 */
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

	if(!isDeleted(this->physicalWorld))
	{
		delete this->physicalWorld;
		this->physicalWorld = NULL;
	}

	if(!isDeleted(this->collisionManager))
	{
		delete this->collisionManager;
		this->collisionManager = NULL;
	}

	this->stage = NULL;

	// stop my clocks
	GameState::stopClocks(this);
}

/**
 * Method called when the StateMachine enters another state without exiting this one
 *
 * @param owner		StateMachine's owner
 */
void GameState::suspend(void* owner __attribute__ ((unused)))
{
	Clock::pause(this->messagingClock, true);

#ifdef __TOOLS
	if(!VUEngine::isEnteringToolState(VUEngine::getInstance()))
#endif
	{
		// Make sure collision colliders are not drawn while suspended
		if(this->collisionManager)
		{
			CollisionManager::hideColliders(this->collisionManager);
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
	}
}

/**
 * Method called when the StateMachine returns to this state from another
 *
 * @param owner		StateMachine's owner
 */
void GameState::resume(void* owner __attribute__ ((unused)))
{
	NM_ASSERT(this->stage, "GameState::resume: null stage");

	HardwareManager::suspendInterrupts();

	if(!VUEngine::isExitingToolState(VUEngine::getInstance()))
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

/**
 * Process user input
 *
 * @param userInput		User input
 */
void GameState::processUserInput(const UserInput* userInput __attribute__ ((unused)))
{}

/**
 * Whether to call processUserInput even when there is no input at all during the current cycle
 *
 * @return 				False by default
 */
bool GameState::processUserInputRegardlessOfInput()
{
	return false;
}

/**
 * Method called when the Game's StateMachine receives a message to be processed
 *
 * @param owner			StateMachine's owner
 * @param telegram		Message wrapper
 * @return 				True if no further processing of the message is required
 */
bool GameState::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageRestoreFPS:

			VUEngine::setGameFrameRate(VUEngine::getInstance(), __MAXIMUM_FPS);
			break;
	}

	return false;
}

/**
 * Method called when the Game's StateMachine receives a message to be processed
 *
 * @param owner			StateMachine's owner
 * @param telegram		Message wrapper
 * @return 				True if no further processing of the message is required
 */
bool GameState::processMessage(void* owner __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	return false;
	// Not sure if necessary, but this can cause problems if no unified messages list is used and can cause unintended performance issues	
//	return Stage::propagateMessage(this->stage, Container::onPropagatedMessage, Telegram::getMessage(telegram)) || UIContainer::propagateMessage(this->uiContainer, Container::onPropagatedMessage, Telegram::getMessage(telegram));
}

/**
 * Start pass a message to the Stage for it to forward to its children
 *
 * @param message	Message code
 * @return			The result of the propagation of the message
 */
int32 GameState::propagateMessage(int32 message)
{	
	return Stage::propagateMessage(this->stage, Container::onPropagatedMessage, message) || UIContainer::propagateMessage(this->uiContainer, Container::onPropagatedMessage, message);
}

/**
 * Start pass a message to the Stage for it to forward to its children
 *
 * @param string	String
 * @return			The result of the propagation of the string
 */
int32 GameState::propagateString(const char* string)
{
	return Stage::propagateString(this->stage, Container::onPropagatedString, string) || Container::propagateString(this->uiContainer, Container::onPropagatedString, string);
}

/**
 * Start a streaming cycle on the Stage
 */
bool GameState::stream()
{
	if(!this->stream)
	{
		return false;
	}

	NM_ASSERT(this->stage, "GameState::stream: null stage");

	return Stage::stream(this->stage);
}

/**
 * Streaming everything on the Stage
 */
void GameState::doStreamAll(bool in, bool out)
{
	HardwareManager::suspendInterrupts();

	do
	{
		// Make sure that the focus entity is transformed before focusing the camera
		GameState::transform(this);

		// Move the camera to its initial position
		Camera::focus(Camera::getInstance());

		// invalidate transformations
		Stage::invalidateTransformation(this->stage);

		// Transformation everything
		GameState::transform(this);

		// Stream in and out all relevant entities
		bool streamingComplete = !Stage::streamAll(this->stage, in, out);

		// Make sure all graphics are ready
		VUEngine::prepareGraphics(VUEngine::getInstance());

		// Force collision purging
		if(!isDeleted(this->collisionManager))
		{
			CollisionManager::purgeDestroyedColliders(this->collisionManager);
		}

		if(streamingComplete)
		{
			break;
		}
	}
	while(true);

	HardwareManager::resumeInterrupts();
}

void GameState::streamAll()
{
	GameState::streamOutAll(this);
	GameState::streamInAll(this);
}

void GameState::streamInAll()
{
	GameState::doStreamAll(this, true, false);
}

/**
 * Streaming everything on the Stage
 */
void GameState::streamOutAll()
{
	GameState::doStreamAll(this, false, true);
}

/**
 * By default, every state is single player
 */
bool GameState::isVersusMode()
{
	return false;
}

/**
 * Start a transformation cycle on the Stage
 */
void GameState::transform()
{
	if(!this->transform)
	{
		return;
	}

	NM_ASSERT(this->stage, "GameState::transform: null stage");

	extern Transformation neutralEnvironmentTransformation;

	Stage::transform(this->stage, &neutralEnvironmentTransformation, Camera::getTransformationFlags(_camera));
}

/**
 * Start a transformation cycle on the Stage
 */
void GameState::transformUI()
{
	if(!this->transform)
	{
		return;
	}

	NM_ASSERT(this->uiContainer, "GameState::transform: null uiContainer");

	extern Transformation neutralEnvironmentTransformation;

	UIContainer::transform(this->uiContainer, NULL, __INVALIDATE_TRANSFORMATION);
}

/**
 * Start a physics simulation cycle on the Stage
 */
void GameState::updatePhysics()
{
	if(!this->updatePhysics || isDeleted(this->physicalWorld))
	{
		return;
	}

	if(Clock::isPaused(this->physicsClock))
	{
		return;
	}

	PhysicalWorld::update(this->physicalWorld);
}

/**
 * Start a cycle for collision processing
 *
 * @return			The result of the collision processing
 */
uint32 GameState::processCollisions()
{
	if(!this->processCollisions || isDeleted(this->collisionManager))
	{
		return false;
	}

	if(Clock::isPaused(this->physicsClock))
	{
		return false;
	}

	return CollisionManager::update(this->collisionManager);
}

/**
 * Load the Stage with the give spec
 *
 * @param stageSpec				Stage's configuration
 * @param positionedEntitiesToIgnore	List of entities from the spec to not load
 */
void GameState::loadStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore)
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
	GameState::setupStage(this, stageSpec, positionedEntitiesToIgnore);

	// load the UI
	GameState::setupUI(this, stageSpec);

	// move the camera to its previous position
	Camera::focus(Camera::getInstance());

	// transformation everything
	GameState::transform(this);

	// Transform everything definitively
	GameState::transform(this);

	// Make sure all graphics are ready
	VUEngine::prepareGraphics(VUEngine::getInstance());

	HardwareManager::resumeInterrupts();

	GameState::changeFrameRate(this, __TARGET_FPS >> 1, 100);
}

void GameState::setupStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore)
{
	if(!isDeleted(this->stage))
	{
		delete this->stage;
		this->stage = NULL;
	}

	this->stage = ((Stage (*)(StageSpec*)) stageSpec->allocator)((StageSpec*)stageSpec);
	
	NM_ASSERT(!isDeleted(this->stage), "GameState::loadStage: null stage");

	Stage::configure(this->stage, positionedEntitiesToIgnore);
}

// setup ui
void GameState::setupUI(StageSpec* stageSpec)
{
	if(!isDeleted(this->uiContainer))
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	if(NULL != stageSpec->entities.UI.allocator)
	{
		this->uiContainer = ((UIContainer (*)(PositionedEntity*)) stageSpec->entities.UI.allocator)(stageSpec->entities.UI.childrenSpecs);
	}
	else
	{
		this->uiContainer = new UIContainer(NULL);
	}

	NM_ASSERT(!isDeleted(this->uiContainer), "GameState::setupUI: null UIContainer");

	// setup ui if allocated and constructed
	if(!isDeleted(this->uiContainer))
	{
		extern Transformation neutralEnvironmentTransformation;
	
		// apply transformations
		UIContainer::transform(this->uiContainer, &neutralEnvironmentTransformation, Camera::getTransformationFlags(_camera));
	}
}

/**
 * Retrieve the Stage
 *
 * @return			Stage
 */
Stage GameState::getStage()
{
	return this->stage;
}

/**
 * Retrieve the UI container
 *
 * @return			UIContainer
 */
UIContainer GameState::getUIContainer()
{
	return this->uiContainer;
}

/**
 * Retrieve the Clock used for delayed messages
 *
 * @return			Clock
 */
Clock GameState::getMessagingClock()
{
	return this->messagingClock;
}

/**
 * Retrieve the Clock passed to the Stage's update method (used for animations)
 *
 * @return			Clock
 */
Clock GameState::getUpdateClock()
{
	return this->updateClock;
}

/**
 * Retrieve the Clock used for physic calculations
 *
 * @return			Clock
 */
Clock GameState::getPhysicsClock()
{
	return this->physicsClock;
}

/**
 * Start all clocks
 */
void GameState::startClocks()
{
	Clock::start(this->messagingClock);
	Clock::start(this->updateClock);
	Clock::start(this->physicsClock);
}

/**
 * Stop all clocks
 */
void GameState::stopClocks()
{
	Clock::stop(this->messagingClock);
	Clock::stop(this->updateClock);
	Clock::stop(this->physicsClock);
}

/**
 * Pause all clocks
 */
void GameState::pauseClocks()
{
	Clock::pause(this->messagingClock, true);
	Clock::pause(this->updateClock, true);
	Clock::pause(this->physicsClock, true);
}

/**
 * Resume all clocks
 */
void GameState::resumeClocks()
{
	Clock::pause(this->messagingClock, false);
	Clock::pause(this->updateClock, false);
	Clock::pause(this->physicsClock, false);
}

/**
 * Start the Clock used for delayed messages
 */
void GameState::startDispatchingDelayedMessages()
{
	Clock::start(this->messagingClock);
}

/**
 * Start the Clock passed to the Stage's update method (used for animations)
 */
void GameState::startAnimations()
{
	Clock::start(this->updateClock);
}

/**
 * Start the Clock used for physics simulations
 */
void GameState::startPhysics()
{
	Clock::start(this->physicsClock);
}

/**
 * Pause the Clock used for delayed messages
 *
 * @param pause		Pause flag
 */
void GameState::pauseMessagingClock(bool pause)
{
	Clock::pause(this->messagingClock, pause);
}

/**
 * Pause the Clock used for animations
 *
 * @param pause		Pause flag
 */
void GameState::pauseAnimations(bool pause)
{
	Clock::pause(this->updateClock, pause);
}

/**
 * Pause the Clock used for physics simulations
 *
 * @param pause		Pause flag
 */
void GameState::pausePhysics(bool pause)
{
	Clock::pause(this->physicsClock, pause);
}

/**
 * Pause the Clock used for messaging
 *
 * @param pause		Pause flag
 */
void GameState::pauseMessaging(bool pause)
{
	Clock::pause(this->messagingClock, pause);
}

/**
 * Retrieve the PhysicalWorld
 *
 * @return			PhysicalWorld
 */
PhysicalWorld GameState::getPhysicalWorld()
{
	if(NULL == this->physicalWorld)
	{
		this->physicalWorld = new PhysicalWorld();
	}

	return this->physicalWorld;
}

/**
 * Retrieve the CollisionManager
 *
 * @return			CollisionManager
 */
CollisionManager GameState::getCollisionManager()
{
	if(NULL == this->collisionManager)
	{
		this->collisionManager = new CollisionManager();
	}

	return this->collisionManager;
}

/**
 * Retrieve the update clock
 *
 * @return			CollisionManager
 */
Clock GameState::getClock()
{
	return this->updateClock;
}

/**
 * Find an entity in the stage
 *
 * @return			CollisionManager
 */
Entity GameState::getEntityByName(const char* entityName)
{
	if(isDeleted(this->stage))
	{
		return NULL;
	}
	
	return Entity::safeCast(Stage::getChildByName(this->stage, entityName, false));
}

/**
 * Hide an entity in the stage
 *
 * @param entityName
 */
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

/**
 * Hide an entity in the stage
 *
 * @param entityName
 */
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

void GameState::changeFrameRate(int16 targetFPS, int32 duration)
{
	VUEngine::setGameFrameRate(VUEngine::getInstance(), targetFPS);

	if(0 <= duration)
	{
		GameState::sendMessageToSelf(this, kMessageRestoreFPS, duration + 1, 0);
	}
}