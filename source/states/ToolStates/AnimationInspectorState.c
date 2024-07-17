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

#include <AnimationInspector.h>

#include "AnimationInspectorState.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationInspectorState::getInstance()
 * @memberof	AnimationInspectorState
 * @public
 * @return		AnimationInspectorState instance
 */


/**
 * Class constructor
 *
 * @private
 */
void AnimationInspectorState::constructor()
{
	Base::constructor();

	this->tool = Tool::safeCast(AnimationInspector::getInstance());
}

/**
 * Class destructor
 *
 * @private
 */
void AnimationInspectorState::destructor()
{
	// destroy base
	Base::destructor();
}

/**
 * Check if key combinations invokes me
 *
 * @public
 * @param userInput		UserInput
 *
 * @return bool
 */
bool AnimationInspectorState::isKeyCombination(const UserInput* userInput)
{
	return ((userInput->holdKey & K_A) && (userInput->holdKey & K_B) && (userInput->releasedKey & K_RR));
}

#endif