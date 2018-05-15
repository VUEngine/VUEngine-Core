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

#include <GameState.h>
#include <Game.h>
#include <Camera.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	GameState
 * @extends State
 * @ingroup states
 */
implements GameState : State;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void GameState::initialTransform(GameState this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::constructor(GameState this)
{
	ASSERT(this, "GameState::constructor: null this");

	Base::constructor();

	this->stage = NULL;

	// clocks
	this->messagingClock = __NEW(Clock);
	this->updateClock = __NEW(Clock);
	this->physicsClock = __NEW(Clock);

	// construct the physical world and collision manager
	this->physicalWorld = __NEW(PhysicalWorld, this->physicsClock);
	this->collisionManager = __NEW(CollisionManager);

	this->cameraPosition.x = 0;
	this->cameraPosition.y = 0;
	this->cameraPosition.z = 0;
	this->previousUpdateTime = 0;
}

/**
 * Class destructor
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::destructor(GameState this)
{
	ASSERT(this, "GameState::destructor: null this");

	__DELETE(this->messagingClock);
	__DELETE(this->updateClock);
	__DELETE(this->physicsClock);

	// destroy the stage
	if(this->stage)
	{
		// destroy the stage
		__DELETE(this->stage);

		this->stage = NULL;
	}

	// must delete these after deleting the stage
	__DELETE(this->physicalWorld);
	__DELETE(this->collisionManager);

	this->physicalWorld = NULL;
	this->collisionManager = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void GameState::enter(GameState this, void* owner __attribute__ ((unused)))
{
	ASSERT(this, "GameState::enter: null this");

	VIPManager::removePostProcessingEffects(VIPManager::getInstance());
	Printing::resetWorldCoordinates(Printing::getInstance());

	GameState::pauseClocks(this);

	Clock::start(this->messagingClock);

	Game::enableHardwareInterrupts(Game::getInstance());
}

/**
 * Method called when by the StateMachine's update method
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void GameState::execute(GameState this, void* owner __attribute__ ((unused)))
{
	ASSERT(this, "GameState::execute: null this");
	ASSERT(this->stage, "GameState::execute: null stage");

	if(!Clock::isPaused(this->messagingClock))
	{
		// update the stage
		 Container::update(this->stage, Clock::getTime(this->updateClock) - this->previousUpdateTime);

		this->previousUpdateTime = Clock::getTime(this->updateClock);
	}
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void GameState::exit(GameState this, void* owner __attribute__ ((unused)))
{
	ASSERT(this, "GameState::exit: null this");

	Game::disableHardwareInterrupts(Game::getInstance());

	// make sure to free the memory
	if(this->stage)
	{
		// destroy the stage
		__DELETE(this->stage);
	}

	this->stage = NULL;

	// stop my clocks
	GameState::stopClocks(this);

	// make sure that all my bodies and colliders get deleted
	PhysicalWorld::reset(this->physicalWorld);
	CollisionManager::reset(this->collisionManager);
}

/**
 * Method called when the StateMachine enters another state without exiting this one
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void GameState::suspend(GameState this, void* owner __attribute__ ((unused)))
{
	ASSERT(this, "GameState::suspend: null this");

	Game::disableHardwareInterrupts(Game::getInstance());

	Clock::pause(this->messagingClock, true);

#ifdef __DEBUG_TOOLS
	if(!Game::isEnteringSpecialMode(Game::getInstance()))
	{
#endif
#ifdef __STAGE_EDITOR
	if(!Game::isEnteringSpecialMode(Game::getInstance()))
	{
#endif
#ifdef __ANIMATION_INSPECTOR
	if(!Game::isEnteringSpecialMode(Game::getInstance()))
	{
#endif

	// save the camera position for resume reconfiguration
	this->cameraPosition = Camera::getPosition(Camera::getInstance());

	// make sure shapes are not drawn while suspended
	CollisionManager::hideShapes(this->collisionManager);

	if(this->stage)
	{
		 Container::suspend(this->stage);
	}

#ifdef __DEBUG_TOOLS
	}
#endif
#ifdef __STAGE_EDITOR
	}
#endif
#ifdef __ANIMATION_INSPECTOR
	}
#endif
}

/**
 * Method called when the StateMachine returns to this state from another
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void GameState::resume(GameState this, void* owner __attribute__ ((unused)))
{
	ASSERT(this, "GameState::resume: null this");
	NM_ASSERT(this->stage, "GameState::resume: null stage");

#ifdef __DEBUG_TOOLS
	if(!Game::isExitingSpecialMode(Game::getInstance()))
	{
#endif
#ifdef __STAGE_EDITOR
	if(!Game::isExitingSpecialMode(Game::getInstance()))
	{
#endif
#ifdef __ANIMATION_INSPECTOR
	if(!Game::isExitingSpecialMode(Game::getInstance()))
	{
#endif

	// set camera to its previous position
	Camera::setStageSize(Camera::getInstance(), Stage::getSize(this->stage));
	Camera::setPosition(Camera::getInstance(), this->cameraPosition);
	Camera::setCameraFrustum(Camera::getInstance(), Stage::getCameraFrustum(this->stage));

	Game::reset(Game::getInstance());

	// must make sure that all textures are completely written
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);

	// update the stage
	 Container::resume(this->stage);

	// move the camera to its previous position
	Camera::focus(Camera::getInstance(), false);

	// force all transformations to take place again
	GameState::initialTransform(this);

	// force all streaming right now
	 Stage::streamAll(this->stage);

	// force char memory defragmentation
	CharSetManager::defragment(CharSetManager::getInstance());

	// set up visual representation
	GameState::synchronizeGraphics(this);

	// make sure all textures are written right now
	SpriteManager::writeTextures(SpriteManager::getInstance());

	// sort all sprites' layers
	SpriteManager::sortLayers(SpriteManager::getInstance());

	// render sprites as soon as possible
	SpriteManager::render(SpriteManager::getInstance());

	// sort all sprites' layers again
	// don't remove me, some custom sprites depend on others
	// to have been setup up before
	SpriteManager::sortLayers(SpriteManager::getInstance());

	// defer rendering again
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);
#ifdef __DEBUG_TOOLS
	}
#endif
#ifdef __STAGE_EDITOR
	}
#endif
#ifdef __ANIMATION_INSPECTOR
	}
#endif

	// load post processing effects
	Stage::loadPostProcessingEffects(this->stage);

	// unpause clock
	Clock::pause(this->messagingClock, false);

	Game::enableHardwareInterrupts(Game::getInstance());
}

/**
 * Process user input
 *
 * @memberof			GameState
 * @public
 *
 * @param this			Function scope
 * @param userInput		User input
 */
void GameState::processUserInput(GameState this __attribute__ ((unused)), UserInput userInput __attribute__ ((unused)))
{
	ASSERT(this, "GameState::processUserInput: null this");
}

/**
 * Method called when the Game's StateMachine receives a message to be processed
 *
 * @memberof			GameState
 * @public
 *
 * @param this			Function scope
 * @param owner			StateMachine's owner
 * @param telegram		Message wrapper
 *
 * @return 				True if no further processing of the message is required
 */
bool GameState::processMessage(GameState this, void* owner __attribute__ ((unused)), Telegram telegram)
{
	ASSERT(this, "GameState::handleMessage: null this");

	return Container::propagateMessage(__SAFE_CAST(Container, this->stage), Container_onPropagatedMessage, Telegram::getMessage(telegram));
}

/**
 * Start pass a message to the Stage for it to forward to its children
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param message	Message code
 *
 * @return			The result of the propagation of the message
 */
int GameState::propagateMessage(GameState this, int message)
{
	return Container::propagateMessage(__SAFE_CAST(Container, this->stage), Container_onPropagatedMessage, message);
}

/**
 * Start a streaming cycle on the Stage
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
bool GameState::stream(GameState this)
{
	ASSERT(this, "GameState::stream: null this");

	return  Stage::stream(this->stage);
}

/**
 * Start a transformation cycle on the Stage
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::transform(GameState this)
{
	ASSERT(this, "GameState::transform: null this");
	ASSERT(this->stage, "GameState::transform: null stage");

	extern Transformation neutralEnvironmentTransformation;

	u8 invalidateTransformationFlag = (_cameraDisplacement->x | _cameraDisplacement->y | _cameraDisplacement->z) ? __INVALIDATE_PROJECTION : 0;
	invalidateTransformationFlag |= _cameraDisplacement->z ? __INVALIDATE_SCALE : 0;

	// then transformation loaded entities
	 Container::transform(this->stage, &neutralEnvironmentTransformation, invalidateTransformationFlag);
}

/**
 * Call the initial transformation on the Stage to setup its children
 *
 * @memberof		GameState
 * @private
 *
 * @param this		Function scope
 */
static void GameState::initialTransform(GameState this)
{
	ASSERT(this, "GameState::initialTransform: null this");
	ASSERT(this->stage, "GameState::transform: null stage");

	extern Transformation neutralEnvironmentTransformation;

	 Container::initialTransform(this->stage, &neutralEnvironmentTransformation, true);
}


/**
 * Start a cycle on the Stage that coordinates the entities with their sprites
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::synchronizeGraphics(GameState this)
{
	ASSERT(this, "GameState::synchronizeGraphics: null this");
	ASSERT(this->stage, "GameState::synchronizeGraphics: null stage");

	// then transformation loaded entities
	 Container::synchronizeGraphics(this->stage);
}

/**
 * Start a physics simulation cycle on the Stage
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::updatePhysics(GameState this)
{
	ASSERT(this, "GameState::updatePhysics: null this");

	PhysicalWorld::update(this->physicalWorld, this->physicsClock);
}

/**
 * Start a cycle for collision processing
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 *
 * @return			The result of the collision processing
 */
u32 GameState::processCollisions(GameState this)
{
	ASSERT(this, "GameState::processCollisions: null this");

	return CollisionManager::update(this->collisionManager, this->physicsClock);
}

/**
 * Load the Stage with the give definition
 *
 * @memberof							GameState
 * @public
 *
 * @param this							Function scope
 * @param stageDefinition				Stage's configuration
 * @param positionedEntitiesToIgnore	List of entities from the definition to not load
 * @param overrideCameraPosition		Flag to override or not the Camera's current position
 */
void GameState::loadStage(GameState this, StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition)
{
	ASSERT(this, "GameState::loadStage: null this");
	ASSERT(stageDefinition, "GameState::loadStage: null stageDefinition");

	// disable hardware interrupts
	Game::disableHardwareInterrupts(Game::getInstance());

	if(this->stage)
	{
		// destroy the stage
		__DELETE(this->stage);
	}

	PhysicalWorld::reset(this->physicalWorld);
	CollisionManager::reset(this->collisionManager);

	// reset the engine state
	Game::reset(Game::getInstance());

	// make sure no entity is set as focus for the camera
	Camera::setFocusGameEntity(Camera::getInstance(), NULL);

	// must make sure that all textures are completely written
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), false);

	// construct the stage
	this->stage = ((Stage (*)(StageDefinition*)) stageDefinition->allocator)((StageDefinition*)stageDefinition);
	ASSERT(this->stage, "GameState::loadStage: null stage");

	// load world entities
	Stage::load(this->stage, positionedEntitiesToIgnore, overrideCameraPosition);

	// move the camera to its previous position
	Camera::focus(Camera::getInstance(), false);

	// transformation everything
	GameState::initialTransform(this);

	// set up visual representation
	GameState::synchronizeGraphics(this);

	// make sure all textures are written right now
	SpriteManager::writeTextures(SpriteManager::getInstance());

	// sort all sprites' layers
	SpriteManager::sortLayers(SpriteManager::getInstance());

	// render sprites as soon as possible
	SpriteManager::render(SpriteManager::getInstance());

	// sort all sprites' layers again
	// don't remove me, some custom sprites depend on others
	// to have been setup up before
	SpriteManager::sortLayers(SpriteManager::getInstance());

	// defer rendering again
	SpriteManager::deferParamTableEffects(SpriteManager::getInstance(), true);

	// load post processing effects
	Stage::loadPostProcessingEffects(this->stage);

	Game::enableHardwareInterrupts(Game::getInstance());
}

/**
 * Retrieve the Stage
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 *
 * @return			Stage
 */
Stage GameState::getStage(GameState this)
{
	ASSERT(this, "GameState::getStage: null this");

	return this->stage;
}

/**
 * Retrieve the Clock used for delayed messages
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 *
 * @return			Clock
 */
Clock GameState::getMessagingClock(GameState this)
{
	ASSERT(this, "GameState::getMessagingClock: null this");

	return this->messagingClock;
}

/**
 * Retrieve the Clock passed to the Stage's update method (used for animations)
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 *
 * @return			Clock
 */
Clock GameState::getUpdateClock(GameState this)
{
	ASSERT(this, "GameState::getUpdateClock: null this");

	return this->updateClock;
}

/**
 * Retrieve the Clock used for physic calculations
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 *
 * @return			Clock
 */
Clock GameState::getPhysicsClock(GameState this)
{
	ASSERT(this, "GameState::getPhysicsClock: null this");

	return this->physicsClock;
}

/**
 * Start all clocks
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::startClocks(GameState this)
{
	ASSERT(this, "GameState::startClocks: null this");

	Clock::start(this->messagingClock);
	Clock::start(this->updateClock);
	Clock::start(this->physicsClock);

	this->previousUpdateTime = Clock::getTime(this->messagingClock);
}

/**
 * Stop all clocks
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::stopClocks(GameState this)
{
	ASSERT(this, "GameState::stopClocks: null this");

	Clock::stop(this->messagingClock);
	Clock::stop(this->updateClock);
	Clock::stop(this->physicsClock);
}

/**
 * Pause all clocks
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::pauseClocks(GameState this)
{
	ASSERT(this, "GameState::pauseClocks: null this");

	Clock::pause(this->messagingClock, true);
	Clock::pause(this->updateClock, true);
	Clock::pause(this->physicsClock, true);
}

/**
 * Resume all clocks
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::resumeClocks(GameState this)
{
	ASSERT(this, "GameState::resumeClocks: null this");

	Clock::pause(this->messagingClock, false);
	Clock::pause(this->updateClock, false);
	Clock::pause(this->physicsClock, false);
}

/**
 * Start the Clock used for delayed messages
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::startDispatchingDelayedMessages(GameState this)
{
	ASSERT(this, "GameState::startInGameClock: null this");

	Clock::start(this->messagingClock);
}

/**
 * Start the Clock passed to the Stage's update method (used for animations)
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::startAnimations(GameState this)
{
	ASSERT(this, "GameState::startAnimations: null this");

	Clock::start(this->updateClock);
}

/**
 * Start the Clock used for physics simulations
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 */
void GameState::startPhysics(GameState this)
{
	ASSERT(this, "GameState::startPhysics: null this");

	Clock::start(this->physicsClock);
}

/**
 * Pause the Clock used for delayed messages
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param pause		Pause flag
 */
void GameState::pauseMessagingClock(GameState this, bool pause)
{
	ASSERT(this, "GameState::pauseInGameClock: null this");

	Clock::pause(this->messagingClock, pause);
}

/**
 * Pause the Clock used for animations
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param pause		Pause flag
 */
void GameState::pauseAnimations(GameState this, bool pause)
{
	ASSERT(this, "GameState::pauseAnimations: null this");

	Clock::pause(this->updateClock, pause);
}

/**
 * Pause the Clock used for physics simulations
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 * @param pause		Pause flag
 */
void GameState::pausePhysics(GameState this, bool pause)
{
	ASSERT(this, "GameState::pausePhysics: null this");

	Clock::pause(this->physicsClock, pause);
}

/**
 * Retrieve the PhysicalWorld
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 *
 * @return			PhysicalWorld
 */
PhysicalWorld GameState::getPhysicalWorld(GameState this)
{
	ASSERT(this, "GameState::getPhysicalWorld: null this");

	return this->physicalWorld;
}

/**
 * Retrieve the CollisionManager
 *
 * @memberof		GameState
 * @public
 *
 * @param this		Function scope
 *
 * @return			CollisionManager
 */
CollisionManager GameState::getCollisionManager(GameState this)
{
	ASSERT(this, "GameState::getCollisionManager: null this");

	return this->collisionManager;
}

