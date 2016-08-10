/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Telegram.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Telegram_ATTRIBUTES																				\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* the message itself. These are all enumerated in a file */									\
        int message;																					\
        /* any additional information that may accompany the message */									\
        void* extraInfo;																				\
        /* who sent this telegram */																	\
        void* sender;																					\
        /* who is to receive this telegram */															\
        void* receiver;																					\

__CLASS_DEFINITION(Telegram, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Telegram_constructor(Telegram this, void* sender, void* receiver, int message, void* extraInfo);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Telegram, void* sender, void* receiver, int message, void* extraInfo)
__CLASS_NEW_END(Telegram, sender, receiver, message, extraInfo);

// class's constructor
static void Telegram_constructor(Telegram this, void* sender, void* receiver, int message, void* extraInfo)
{
	// construct base object
	__CONSTRUCT_BASE(Object);

	// set the attributes
	this->sender = sender;
	this->receiver = receiver;
	this->message = message;
	this->extraInfo = extraInfo;
}

// class's destructor
void Telegram_destructor(Telegram this)
{
	ASSERT(this, "Telegram::destructor: null this");

	this->sender = NULL;

	this->receiver = NULL;

	// free the memory
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// retrieve sender
void* Telegram_getSender(Telegram this)
{
	ASSERT(this, "Telegram::getSender: null this");

	return this->sender;
}

// retrieve receiver
void* Telegram_getReceiver(Telegram this)
{
	ASSERT(this, "Telegram::getReceiver: null this");

	return this->receiver;
}

// retrieve message
int Telegram_getMessage(Telegram this)
{
	ASSERT(this, "Telegram::getMessage: null this");

	return this->message;
}

// retrieve extra info
void* Telegram_getExtraInfo(Telegram this)
{
	ASSERT(this, "Telegram::getExtraInfo: null this");

	return this->extraInfo;
}

