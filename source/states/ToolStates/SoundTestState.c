/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SoundTestState.h>
#include <SoundTest.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			SoundTestState::getInstance()
 * @memberof	SoundTestState
 * @public
 * @return		SoundTestState instance
 */


/**
 * Class constructor
 *
 * @private
 */
void SoundTestState::constructor()
{
	Base::constructor();

	this->tool = Tool::safeCast(SoundTest::getInstance());
}

/**
 * Class destructor
 *
 * @private
 */
void SoundTestState::destructor()
{
	// destroy base
	Base::destructor();
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @param owner		StateMachine's owner
 */
void SoundTestState::enter(void* owner __attribute__ ((unused)))
{
	Base::enter(this, owner);

	this->stream = false;
	this->transform = false;
	this->synchronizeGraphics = false;
	this->updatePhysics = false;
	this->processCollisions = false;
}

/**
 * Check if key combinations invokes me
 *
 * @public
 * @param userInput		UserInput
 *
 * @return bool
 */
bool SoundTestState::isKeyCombination(UserInput userInput)
{
	return ((userInput.holdKey & K_LT) && (userInput.holdKey & K_RT) && (userInput.releasedKey & K_RD));
}

#endif
