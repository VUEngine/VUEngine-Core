/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Telegram_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* delay in ms */															\
	u32 delay;																	\
																				\
	/* the message itself. These are all enumerated in a file */				\
	int message;																\
																				\
	/* any additional information that may accompany the message */				\
	void* extraInfo;															\
																				\
	/* who sent this telegram */												\
	void* sender;																\
																				\
	/* who is to receive this telegram */										\
	void* receiver;

__CLASS_DEFINITION(Telegram, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void Telegram_constructor(Telegram this, u32 dispatchTime, void* sender, void* receiver, int message, void* extraInfo);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Telegram, u32 dispatchTime, void* sender, void* receiver, int message, void* extraInfo)
__CLASS_NEW_END(Telegram, dispatchTime, sender, receiver, message, extraInfo);

// class's constructor
static void Telegram_constructor(Telegram this, u32 delay, void* sender, void* receiver, int message, void* extraInfo)
{
	// construct base object
	__CONSTRUCT_BASE();

	// set the attributes
	this->delay = delay;

	this->sender = sender;

	this->receiver = receiver;

	this->message = message;

	this->extraInfo = extraInfo;
}

// class's destructor
void Telegram_destructor(Telegram this)
{
	ASSERT(this, "Telegram::destructor: null this");

	// free the memory
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

// retrieve delay time
u32 Telegram_getDelay(Telegram this)
{
	ASSERT(this, "Telegram::getDelay: null this");

	return this->delay;
}