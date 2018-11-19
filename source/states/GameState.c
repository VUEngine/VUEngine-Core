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
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void GameState::constructor()
{
	Base::constructor();

	this->stage = NULL;

	// clocks
	this->messagingClock = new Clock();
	this->updateClock = new Clock();
	this->physicsClock = new Clock();

	// construct the physical world and collision manager
	this->physicalWorld = new PhysicalWorld(this->physicsClock);
	this->collisionManager = new CollisionManager();

	this->cameraPosition.x = 0;
	this->cameraPosition.y = 0;
	this->cameraPosition.z = 0;
	this->previousUpdateTime = 0;
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
	if(this->stage)
	{
		// destroy the stage
		delete this->stage;

		this->stage = NULL;
	}

	// must delete these after deleting the stage
	delete this->physicalWorld;
	delete this->collisionManager;

	this->physicalWorld = NULL;
	this->collisionManager = NULL;

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
	Printing::resetWorldCoordinates(Printing::getInstance());

	GameState::pauseClocks(this);
	this->previousUpdateTime = Clock::getTime(this->updateClock);

	Clock::start(this->messagingClock);

	Game::enableHardwareInterrupts(Game::getInstance());
}

/**
 * Method called when by the StateMachine's update method
 *
 * @param owner		StateMachine's owner
 */
void GameState::execute(void* owner __attribute__ ((unused)))
{
	ASSERT(this->stage, "GameState::execute: null stage");

	if(!Clock::isPaused(this->updateClock))
	{
		// update the stage
		Container::update(this->stage, Clock::getTime(this->updateClock) - this->previousUpdateTime);

		this->previousUpdateTime = Clock::getTime(this->updateClock);
	}
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @param owner		StateMachine's owner
 */
void GameState::exit(void* owner __attribute__ ((unused)))
{
	Game::disableHardwareInterrupts(Game::getInstance());

	// make sure to free the memory
	if(this->stage)
	{
		// destroy the stage
		delete this->stage;
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
 * @param owner		StateMachine's owner
 */
void GameState::suspend(void* owner __attribute__ ((unused)))
{
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
 * @param owner		StateMachine's owner
 */
void GameState::resume(void* owner __attribute__ ((unused)))
{
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
 * @param userInput		User input
 */
void GameState::processUserInput(UserInput userInput __attribute__ ((unused)))
{}

/**
 * Method called when the Game's StateMachine receives a message to be processed
 *
 * @param owner			StateMachine's owner
 * @param telegram		Message wrapper
 * @return 				True if no further processing of the message is required
 */
bool GameState::processMessage(void* owner __attribute__ ((unused)), Telegram telegram)
{
	return Container::propagateMessage(this->stage, Container::onPropagatedMessage, Telegram::getMessage(telegram));
}

/**
 * Start pass a message to the Stage for it to forward to its children
 *
 * @param message	Message code
 * @return			The result of the propagation of the message
 */
int GameState::propagateMessage(int message)
{
	return Container::propagateMessage(this->stage, Container::onPropagatedMessage, message);
}

/**
 * Start a streaming cycle on the Stage
 */
bool GameState::stream()
{
	return  Stage::stream(this->stage);
}

/**
 * Start a transformation cycle on the Stage
 */
void GameState::transform()
{
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
 * @private
 */
void GameState::initialTransform()
{
	ASSERT(this->stage, "GameState::transform: null stage");

	extern Transformation neutralEnvironmentTransformation;

		Container::initialTransform(this->stage, &neutralEnvironmentTransformation, true);
}

/**
 * Start a cycle on the Stage that coordinates the entities with their sprites
 */
void GameState::synchronizeGraphics()
{
	ASSERT(this->stage, "GameState::synchronizeGraphics: null stage");

	// then transformation loaded entities
		Container::synchronizeGraphics(this->stage);
}

/**
 * Start a physics simulation cycle on the Stage
 */
void GameState::updatePhysics()
{
	PhysicalWorld::update(this->physicalWorld, this->physicsClock);
}

/**
 * Start a cycle for collision processing
 *
 * @return			The result of the collision processing
 */
u32 GameState::processCollisions()
{
	return CollisionManager::update(this->collisionManager, this->physicsClock);
}

/**
 * Load the Stage with the give definition
 *
 * @param stageDefinition				Stage's configuration
 * @param positionedEntitiesToIgnore	List of entities from the definition to not load
 * @param overrideCameraPosition		Flag to override or not the Camera's current position
 */
void GameState::loadStage(StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition)
{
	ASSERT(stageDefinition, "GameState::loadStage: null stageDefinition");

	// disable hardware interrupts
	Game::disableHardwareInterrupts(Game::getInstance());

	if(this->stage)
	{
		// destroy the stage
		delete this->stage;
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
 * @return			Stage
 */
Stage GameState::getStage()
{
	return this->stage;
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

	this->previousUpdateTime = Clock::getTime(this->updateClock);
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
 * Retrieve the PhysicalWorld
 *
 * @return			PhysicalWorld
 */
PhysicalWorld GameState::getPhysicalWorld()
{
	return this->physicalWorld;
}

/**
 * Retrieve the CollisionManager
 *
 * @return			CollisionManager
 */
CollisionManager GameState::getCollisionManager()
{
	return this->collisionManager;
}

