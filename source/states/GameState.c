/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
	this->physicalWorld = NULL;
	this->collisionManager = NULL;

	this->cameraPosition.x = 0;
	this->cameraPosition.y = 0;
	this->cameraPosition.z = 0;
	this->previousUpdateTime = 0;

	this->stream = true;
	this->transform = true;
	this->synchronizeGraphics = true;
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
	if(this->stage)
	{
		// destroy the stage
		delete this->stage;

		this->stage = NULL;
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
	this->previousUpdateTime = Clock::getTime(this->updateClock);

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
	this->stream = true;
	this->transform = true;
	this->synchronizeGraphics = true;
	this->updatePhysics = true;
	this->processCollisions = true;

	// make sure to free the memory
	if(this->stage)
	{
		// destroy the stage
		delete this->stage;
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

	// Save the camera position for resume reconfiguration
	this->cameraPosition = Camera::getPosition(Camera::getInstance());

	// Make sure shapes are not drawn while suspended
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

	if(!Game::isExitingSpecialMode(Game::getInstance()))
	{
		// Set camera to its previous position
		Camera::setStageSize(Camera::getInstance(), Stage::getSize(this->stage));
		Camera::setPosition(Camera::getInstance(), this->cameraPosition);
		Camera::setCameraFrustum(Camera::getInstance(), Stage::getCameraFrustum(this->stage));

		// Reset the engine state
		Game::reset(Game::getInstance());

		// Update the stage
		Container::resume(this->stage);

		// Move the camera to its previous position
		Camera::focus(Camera::getInstance(), false);

		// Force all transformations to take place again
		GameState::initialTransform(this);

		// Force all streaming right now
		Stage::streamAll(this->stage);

		// Force char memory defragmentation
		CharSetManager::defragment(CharSetManager::getInstance());

		// Set up visual representation
		GameState::synchronizeGraphics(this);

		// Make sure everything is properly rendered
		SpriteManager::prepareAll(SpriteManager::getInstance());
	}

	// Restore timer
	Stage::setupTimer(this->stage);

	// load post processing effects
	Stage::loadPostProcessingEffects(this->stage);

	// unpause clock
	Clock::pause(this->messagingClock, false);
}

/**
 * Process user input
 *
 * @param userInput		User input
 */
void GameState::processUserInput(UserInput userInput __attribute__ ((unused)))
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
void GameState::streamAll()
{
	do
	{
		// Move the camera to its initial position
		Camera::focus(Camera::getInstance(), false);

		// Transformation everything so anything outside the camera
		// can be streamed out
		GameState::transform(this);

		// Froce graphics to get ready
		GameState::synchronizeGraphics(this);
	}
	while(Stage::stream(this->stage));

	// Stream in and out all relevant entities
	Stage::streamAll(this->stage);

	// Transformation everything definitively
	GameState::transform(this);

	// Froce graphics to get ready
	GameState::synchronizeGraphics(this);

	// Make sure all sprites are ready
	SpriteManager::prepareAll(SpriteManager::getInstance());
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

	u8 invalidateTransformationFlag = (_cameraDisplacement->x | _cameraDisplacement->y | _cameraDisplacement->z) ? __INVALIDATE_PROJECTION : 0;
	invalidateTransformationFlag |= _cameraDisplacement->z ? __INVALIDATE_SCALE : 0;

	// then transformation loaded entities
	Container::transform(this->stage, &neutralEnvironmentTransformation, invalidateTransformationFlag);
}

/**
 * By default, every state is single player
 */
bool GameState::isVersusMode()
{
	return false;
}


/**
 * Call the initial transformation on the Stage to setup its children
 *
 * @private
 */
void GameState::initialTransform()
{
	ASSERT(this->stage, "GameState::initialTransform: null stage");

	extern Transformation neutralEnvironmentTransformation;

	Container::initialTransform(this->stage, &neutralEnvironmentTransformation, true);
}

/**
 * Start a cycle on the Stage that coordinates the entities with their sprites
 */
void GameState::synchronizeGraphics()
{
	if(!this->synchronizeGraphics)
	{
		return;
	}

	NM_ASSERT(this->stage, "GameState::synchronizeGraphics: null stage");

	// then transformation loaded entities
	Container::synchronizeGraphics(this->stage);
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

	PhysicalWorld::update(this->physicalWorld, this->physicsClock);
}

/**
 * Start a cycle for collision processing
 *
 * @return			The result of the collision processing
 */
u32 GameState::processCollisions()
{
	if(!this->processCollisions || isDeleted(this->collisionManager))
	{
		return false;
	}

	return CollisionManager::update(this->collisionManager, this->physicsClock);
}

/**
 * Load the Stage with the give spec
 *
 * @param stageSpec				Stage's configuration
 * @param positionedEntitiesToIgnore	List of entities from the spec to not load
 * @param overrideCameraPosition		Flag to override or not the Camera's current position
 */
void GameState::loadStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition)
{
	NM_ASSERT(stageSpec, "GameState::loadStage: null stageSpec");

	if(this->stage)
	{
		// destroy the stage
		delete this->stage;
	}

	// Reset the engine state
	Game::reset(Game::getInstance());

	// make sure no entity is set as focus for the camera
	Camera::setFocusGameEntity(Camera::getInstance(), NULL);

	// construct the stage
	this->stage = ((Stage (*)(StageSpec*)) stageSpec->allocator)((StageSpec*)stageSpec);
	ASSERT(this->stage, "GameState::loadStage: null stage");

	// load world entities
	Stage::load(this->stage, positionedEntitiesToIgnore, overrideCameraPosition);

	// move the camera to its previous position
	Camera::focus(Camera::getInstance(), false);

	// transformation everything
	GameState::initialTransform(this);

	// set up visual representation
	GameState::synchronizeGraphics(this);

	// Make sure everything is properly rendered
	SpriteManager::prepareAll(SpriteManager::getInstance());

	// load post processing effects
	Stage::loadPostProcessingEffects(this->stage);

	// Transformation everything definitively
	GameState::transform(this);
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
	if(NULL == this->physicalWorld)
	{
		this->physicalWorld = new PhysicalWorld(this->physicsClock);
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