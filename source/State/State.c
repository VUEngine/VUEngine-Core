/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include "State.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void State::constructor()
{
	Base::constructor();
}
//---------------------------------------------------------------------------------------------------------
void State::destructor()
{
	// free processor's memory

	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void State::enter(void* owner __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void State::execute(void* owner __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void State::exit(void* owner __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void State::suspend(void* owner __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void State::resume(void* owner __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
bool State::processMessage(void* owner __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------
