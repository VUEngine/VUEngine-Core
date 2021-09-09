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

