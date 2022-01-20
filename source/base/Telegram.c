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

#include <Telegram.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param sender		Sender of the message
 * @param receiver		Intended receiver
 * @param message		Message code
 * @param extraInfo		Pointer to any attachment to the message
 */
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

/**
 * Class destructor
 */
void Telegram::destructor()
{
	this->sender = NULL;
	this->receiver = NULL;

	// free the memory
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Retrieve the sender
 *
 * @return				Sender of the message
 */
void* Telegram::getSender()
{
	return this->sender;
}

/**
 * Retrieve the receiver
 *
 * @return				Intended receiver
 */
void* Telegram::getReceiver()
{
	return this->receiver;
}

/**
 * Retrieve the message code
 *
 * @return				Message code
 */
int32 Telegram::getMessage()
{
	return this->message;
}

/**
 * Retrieve the message's attachment
 *
 * @return				Pointer to any attachment to the message
 */
void* Telegram::getExtraInfo()
{
	return this->extraInfo;
}

