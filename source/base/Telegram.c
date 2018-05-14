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

#include <Telegram.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Telegram_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
		/**
		 * @var int		message
		 * @brief		The message itself. These are all enumerated in a file.
		 * @memberof	Telegram
		 */																								\
		int message;																					\
		/**
		 * @var void*	extraInfo
		 * @brief		Any additional information that may accompany the message
		 * @memberof	Telegram
		 */																								\
		void* extraInfo;																				\
		/**
		 * @var void*	sender
		 * @brief		Who sent this telegram
		 * @memberof	Telegram
		 */																								\
		void* sender;																					\
		/**
		 * @var void*	receiver
		 * @brief		Who is to receive this telegram
		 * @memberof	Telegram
		 */																								\
		void* receiver;																					\

/**
 * @class 	Telegram
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(Telegram, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Telegram, void* sender, void* receiver, int message, void* extraInfo)
__CLASS_NEW_END(Telegram, sender, receiver, message, extraInfo);

/**
 * Class constructor
 *
 * @memberof			Telegram
 * @private
 *
 * @param this			Function scope
 * @param sender		Sender of the message
 * @param receiver		Intended receiver
 * @param message		Message code
 * @param extraInfo		Pointer to any attachment to the message
 */
void Telegram_constructor(Telegram this, void* sender, void* receiver, int message, void* extraInfo)
{
	ASSERT(this, "Telegram::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// set the attributes
	this->sender = sender;
	this->receiver = receiver;
	this->message = message;
	this->extraInfo = extraInfo;
}

/**
 * Class destructor
 *
 * @memberof			Telegram
 * @private
 *
 * @param this			Function scope
 */
void Telegram_destructor(Telegram this)
{
	ASSERT(this, "Telegram::destructor: null this");

	this->sender = NULL;

	this->receiver = NULL;

	// free the memory
	// must always be called at the end of the destructor
	Base_destructor();
}

/**
 * Retrieve the sender
 *
 * @memberof			Telegram
 * @private
 *
 * @param this			Function scope
 *
 * @return				Sender of the message
 */
void* Telegram_getSender(Telegram this)
{
	ASSERT(this, "Telegram::getSender: null this");

	return this->sender;
}

/**
 * Retrieve the receiver
 *
 * @memberof			Telegram
 * @private
 *
 * @param this			Function scope
 *
 * @return				Intended receiver
 */
void* Telegram_getReceiver(Telegram this)
{
	ASSERT(this, "Telegram::getReceiver: null this");

	return this->receiver;
}

/**
 * Retrieve the message code
 *
 * @memberof			Telegram
 * @private
 *
 * @param this			Function scope
 *
 * @return				Message code
 */
int Telegram_getMessage(Telegram this)
{
	ASSERT(this, "Telegram::getMessage: null this");

	return this->message;
}

/**
 * Retrieve the message's attachment
 *
 * @memberof			Telegram
 * @private
 *
 * @param this			Function scope
 *
 * @return				Pointer to any attachment to the message
 */
void* Telegram_getExtraInfo(Telegram this)
{
	ASSERT(this, "Telegram::getExtraInfo: null this");

	return this->extraInfo;
}

