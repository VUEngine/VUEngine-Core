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

#include <ToolState.h>
#include <AnimationInspector.h>
#include <VUEngine.h>
#include <MessageDispatcher.h>
#include <Telegram.h>
#include <KeypadManager.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ToolState::getInstance()
 * @memberof	ToolState
 * @public
 * @return		ToolState instance
 */


/**
 * Class constructor
 *
 * @private
 */
void ToolState::constructor()
{
	Base::constructor();

	this->tool = NULL;
}

/**
 * Class destructor
 *
 * @private
 */
void ToolState::destructor()
{
	this->tool = NULL;

	// destroy base
	Base::destructor();
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @param owner		StateMachine's owner
 */
void ToolState::enter(void* owner __attribute__ ((unused)))
{
	Base::enter(this, owner);
	GameState::pauseClocks(GameState::safeCast(StateMachine::getPreviousState(VUEngine::getStateMachine(VUEngine::getInstance()))));

	if(!isDeleted(this->tool))
	{
		Tool::setGameState(this->tool, GameState::safeCast(StateMachine::getPreviousState(VUEngine::getStateMachine(VUEngine::getInstance()))));
		Tool::show(this->tool);
	}
}

bool ToolState::stream()
{
	return false;
}

/**
 * Method called when by the StateMachine's update method
 *
 * @param owner		StateMachine's owner
 */
void ToolState::execute(void* owner __attribute__ ((unused)))
{
	if(!isDeleted(this->tool))
	{
		Tool::update(this->tool);
	}
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @param owner		StateMachine's owner
 */
void ToolState::exit(void* owner __attribute__ ((unused)))
{
	if(!isDeleted(this->tool))
	{
		Tool::hide(this->tool);
	}

	GameState::resumeClocks(GameState::safeCast(StateMachine::getPreviousState(VUEngine::getStateMachine(VUEngine::getInstance()))));
	Base::exit(this, owner);
}

/**
 * Process user input
 *
 * @param userInput		User input
 */
void ToolState::processUserInput(UserInput userInput)
{
	if(!isDeleted(this->tool))
	{
		Tool::processUserInput(this->tool, userInput.releasedKey);
	}
}

/**
 * Transform
 *
 */
void ToolState::transform()
{
}

/**
 * Sync graphics
 *
 */
void ToolState::synchronizeGraphics()
{
}

