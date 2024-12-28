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

#include "Telegram.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Telegram::constructor(void* sender, void* receiver, int32 message, void* extraInfo)
{
	// construct base object
	Base::constructor();

	// set the attributes
	this->sender = sender;
	this->receiver = receiver;
	this->message = message;
	this->extraInfo = extraInfo;
}
//---------------------------------------------------------------------------------------------------------
void Telegram::destructor()
{
	this->sender = NULL;

	this->receiver = NULL;

	// free the memory

	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void* Telegram::getSender()
{
	return this->sender;
}
//---------------------------------------------------------------------------------------------------------
void* Telegram::getReceiver()
{
	return this->receiver;
}
//---------------------------------------------------------------------------------------------------------
int32 Telegram::getMessage()
{
	return this->message;
}
//---------------------------------------------------------------------------------------------------------
void* Telegram::getExtraInfo()
{
	return this->extraInfo;
}
//---------------------------------------------------------------------------------------------------------

