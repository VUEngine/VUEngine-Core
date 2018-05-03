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

#include <CommunicationManager.h>
#include <TimerManager.h>
#include <Game.h>
#include <MessageDispatcher.h>
#include <Utilities.h>
#ifdef __DEBUG_TOOLS
#include <Debug.h>
#endif
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------

//volatile u16* _communicationRegisters __INITIALIZED_DATA_SECTION_ATTRIBUTE = (u16*)_hardwareRegisters;
static u8* volatile _communicationRegisters =			(u8*)0x02000000;


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

enum CommunicationManagerStatus
{
	kStatusIdle = 0,
	kWaitingPayload,
	kSendingPayload,
};

enum HandshakeStatus
{
	kHandshakeIdle = 0,
	kWaitingSyn,
	kWaitingAck,
	kWaitingKeepAlive,
	kSendingSyn,
	kSendingAck,
	kSendingKeepAlive,
};


//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define	__CCR		0x00	// Communication Control Register	(0x0200 0000)
#define	__CCSR		0x04	// COMCNT Control Register			(0x0200 0004)
#define	__CDTR		0x08	// Transmitted Data Register		(0x0200 0008)
#define	__CDRR		0x0C	// Received Data Register			(0x0200 000C)

// communicating penging flag
#define	__COM_PENDING					0x02
// start communication
#define	__COM_START						0x04
// use external clock (remote)
#define	__COM_USE_EXTERNAL_CLOCK		0x10
// disable interrupt
#define	__COM_DISABLE_INTERRUPT			0x80

// start communication as remote
#define	__COM_AS_REMOTE			(__COM_USE_EXTERNAL_CLOCK | __COM_START)
// start communication as master
#define	__COM_AS_MASTER			(__COM_START)



#define __COM_TIME_TO_WAIT_FOR_SYN		500
#define __HANDSHAKE_TRIES				10000
#define __COM_WAIT_TO_FINISH			-1
#define __HANDSHAKE_SIN					0x11
#define __HANDSHAKE_ACK					0x22
#define __HANDSHAKE_FIN					0x33
#define __KEEP_ALIVE					0x44
#define __WAIT_PAYLOAD_TRIES			100


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CommunicationManager_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		u8 communicationMode;																			\
		u8 sequenceNumber;																				\
		int handshake;																					\
		int status;																						\
		int waitPayloadTries;																			\

/**
 * @class	CommunicationManager
 * @extends Object
 * @ingroup hardware
 */
__CLASS_DEFINITION(CommunicationManager, Object);


static CommunicationManager _communicationManager;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void CommunicationManager_constructor(CommunicationManager this);
static void CommunicationManager_processInterrupt(CommunicationManager this);
static void CommunicationManager_enableInterrupts(CommunicationManager this __attribute__ ((unused)));
static void CommunicationManager_disableInterrupts(CommunicationManager this __attribute__ ((unused)));
static void CommunicationManager_resetCommunications(CommunicationManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CommunicationManager_getInstance()
 * @memberof	CommunicationManager
 * @public
 *
 * @return		CommunicationManager instance
 */
__SINGLETON(CommunicationManager);

/**
 * Class constructor
 *
 * @memberof	CommunicationManager
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) CommunicationManager_constructor(CommunicationManager this)
{
	ASSERT(this, "CommunicationManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	CommunicationManager_resetCommunications(this);

	_communicationManager = this;
}

/**
 * Class destructor
 *
 * @memberof	CommunicationManager
 * @public
 *
 * @param this	Function scope
 */
void CommunicationManager_destructor(CommunicationManager this)
{
	ASSERT(this, "CommunicationManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Enable interrupts
 *
 * @memberof	CommunicationManager
 * @public
 *
 * @param this	Function scope
 */
static void CommunicationManager_enableInterrupts(CommunicationManager this __attribute__ ((unused)))
{
	ASSERT(this, "CommunicationManager::enableInterrupts: null this");

	_communicationRegisters[__CCR] &= ~__COM_DISABLE_INTERRUPT;
}

/**
 * Disable interrupts
 *
 * @memberof	CommunicationManager
 * @public
 *
 * @param this	Function scope
 */
static void CommunicationManager_disableInterrupts(CommunicationManager this __attribute__ ((unused)))
{
	ASSERT(this, "CommunicationManager::disableInterrupts: null this");

	// disable interrupts to allow sends without affecting unready receiver
	_communicationRegisters[__CCSR] = 0xFF;
	_communicationRegisters[__CCR] |= __COM_DISABLE_INTERRUPT;
}

/**
 * Enable communications
 *
 * @memberof	CommunicationManager
 * @public
 *
 * @param this	Function scope
 */
void CommunicationManager_enableCommunications(CommunicationManager this __attribute__ ((unused)))
{
	ASSERT(this, "CommunicationManager::enableCommunications: null this");


}

/**
 * Disable communications
 *
 * @memberof	CommunicationManager
 * @public
 *
 * @param this	Function scope
 */
void CommunicationManager_disableCommunications(CommunicationManager this __attribute__ ((unused)))
{
	ASSERT(this, "CommunicationManager::disableCommunications: null this");

}

/**
 * Communication's interrupt handler
 *
 * @memberof		CommunicationManager
 * @public
 */
void CommunicationManager_interruptHandler(void)
{
	// disable interrupts
	CommunicationManager_disableInterrupts(_communicationManager);

	// handle the interrupt
	CommunicationManager_processInterrupt(_communicationManager);

	// enable interrupts
//	CommunicationManager_enableInterrupts(_communicationManager);
}

static void CommunicationManager_resetCommunications(CommunicationManager this)
{
	// reset
	this->communicationMode = __COM_AS_REMOTE;
	this->handshake = kHandshakeIdle;
	this->status = kStatusIdle;
	this->sequenceNumber = 1;
	this->waitPayloadTries = 0;
	CommunicationManager_disableInterrupts(this);

	MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kCommunicationsSendSyn);
	MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kCommunicationsWaitSyn);
}

static bool CommunicationManager_sendHandshake(CommunicationManager this, u8 handshakePayload)
{
	if(__HANDSHAKE_SIN == handshakePayload)
	{
		this->communicationMode = __COM_AS_MASTER;
		this->handshake = kSendingSyn;
	}
	else if(__HANDSHAKE_ACK == handshakePayload)
	{
		this->handshake = kSendingAck;
	}
	else if(__KEEP_ALIVE == handshakePayload)
	{
		this->handshake = kSendingKeepAlive;
		this->sequenceNumber++;
	}

	this->status = kStatusIdle;

	CommunicationManager_sendPayload(this, handshakePayload);

	this->waitPayloadTries = __WAIT_PAYLOAD_TRIES + Utilities_random(Utilities_randomSeed(), __WAIT_PAYLOAD_TRIES);

	return true;
}

static bool CommunicationManager_waitHandshake(CommunicationManager this, u8 handshakePayload)
{
	if(__HANDSHAKE_SIN == handshakePayload)
	{
		this->communicationMode = __COM_AS_REMOTE;
		this->handshake = kWaitingSyn;
	}
	else if(__HANDSHAKE_ACK == handshakePayload)
	{
		this->handshake = kWaitingAck;
	}
	else if(__KEEP_ALIVE == handshakePayload)
	{
		this->handshake = kWaitingKeepAlive;
		this->sequenceNumber++;
	}

	this->status = kStatusIdle;

	CommunicationManager_receivePayload(this);

	return true;
}

/**
 * Process interrupt method
 *
 * @memberof		CommunicationManager
 * @public
 */
inline static void CommunicationManager_processInterrupt(CommunicationManager this)
{

	PRINT_TIME(31, 3);
	PRINT_INT(this->waitPayloadTries, 31, 8);
	PRINT_TEXT("Status: ", 31, 9);
	PRINT_INT(this->status, 31 + 12, 9);
	PRINT_TEXT("Handshake: ", 31, 10);
	PRINT_INT(this->handshake, 31 + 12, 10);
	PRINT_TEXT("Connected: ", 31, 11);
	PRINT_INT(this->handshake == kSendingKeepAlive || this->handshake == kWaitingKeepAlive, 31 + 12, 11);
	PRINT_TEXT("Mode: ", 31, 12);
	PRINT_TEXT(__COM_AS_REMOTE == this->communicationMode ? "Remote" : "Master ", 31 + 12, 12);

	switch(this->status)
	{
		case kStatusIdle:

			// do nothing
			break;

		case kWaitingPayload:
			{
				Package package = (Package){_communicationRegisters[__CDRR], !(_communicationRegisters[__CCR] & __COM_PENDING)};

				if(!package.isValid)
				{
					CommunicationManager_resetCommunications(this);
					return;
				}

				switch(this->handshake)
				{
					case kWaitingSyn:

						if(__HANDSHAKE_SIN == package.payload)
						{
							//MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kCommunicationsSendSyn);

							CommunicationManager_sendHandshake(this, __HANDSHAKE_ACK);
						}
						else if(0 < --this->waitPayloadTries)
						{
							CommunicationManager_waitHandshake(this, __HANDSHAKE_SIN);
						}
						else
						{
							//CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN);
		//					CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN);
							CommunicationManager_resetCommunications(this);
						}

						break;

					case kWaitingAck:

						if(__HANDSHAKE_ACK == package.payload)
						{
							CommunicationManager_sendHandshake(this, __KEEP_ALIVE);
						}
						else if(0 < --this->waitPayloadTries)
						{
							CommunicationManager_waitHandshake(this, __HANDSHAKE_ACK);
						}
						else
						{
//							CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN);
							CommunicationManager_resetCommunications(this);
						}

						break;

					case kWaitingKeepAlive:

						if(__KEEP_ALIVE == package.payload)
						//if(package.isValid && this->sequenceNumber == package.payload)
						{
							CommunicationManager_sendHandshake(this, __KEEP_ALIVE);
						}
						else if(0 < --this->waitPayloadTries)
						{
							CommunicationManager_waitHandshake(this, __KEEP_ALIVE);
						}
						else
						{
//							CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN);
							CommunicationManager_resetCommunications(this);
						}

						break;
				}
			}

			break;

		case kSendingPayload:
			{
				Package package = (Package){_communicationRegisters[__CDTR], !(_communicationRegisters[__CCR] & __COM_PENDING)};

				if(!package.isValid)
				{
					CommunicationManager_resetCommunications(this);
					return;
				}

				switch(this->handshake)
				{
					case kSendingSyn:

						if(__HANDSHAKE_SIN == package.payload)
						{
//							MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kCommunicationsWaitSyn);

							CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN);
							//CommunicationManager_waitHandshake(this, __HANDSHAKE_ACK);
						}

						PRINT_TEXT("hola", 20, 14);

						break;

					case kSendingAck:

						if(__HANDSHAKE_ACK == package.payload)
						{
							CommunicationManager_waitHandshake(this, __KEEP_ALIVE);
						}

						break;

					case kSendingKeepAlive:

						if(__KEEP_ALIVE == package.payload)
						{
							CommunicationManager_waitHandshake(this, __KEEP_ALIVE);
						}

						break;
				}

				break;
			}
	}
}

bool CommunicationManager_handleMessage(CommunicationManager this, Telegram telegram)
{
/*
	switch(Telegram_getMessage(telegram))
	{
		case kCommunicationsSendSyn:

			// try as master
			this->communicationMode = __COM_AS_MASTER;

			if(CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN))
			{
				MessageDispatcher_dispatchMessage(__COM_TIME_TO_WAIT_FOR_SYN + Utilities_random(Utilities_randomSeed(), __COM_TIME_TO_WAIT_FOR_SYN), __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kCommunicationsWaitSyn, NULL);
			}

			break;

		case kCommunicationsWaitSyn:

			// reset
			CommunicationManager_resetCommunications(this);
			break;
	}
*/
	return false;
}


bool CommunicationManager_sendPayload(CommunicationManager this, u8 payload)
{
	if(kStatusIdle == this->status)
	{
		this->status = kSendingPayload;
		_communicationRegisters[__CDTR] = payload;
		_communicationRegisters[__CCR] = this->communicationMode;

		return true;
	}

	return false;
}

bool CommunicationManager_receivePayload(CommunicationManager this)
{
	if(kStatusIdle == this->status)
	{
		this->status = kWaitingPayload;
		_communicationRegisters[__CCR] = this->communicationMode;

		return true;
	}

	return false;
}

Package CommunicationManager_getPackage(CommunicationManager this)
{
	if(kStatusIdle == this->status)
	{
		return (Package){_communicationRegisters[__CDRR], !(_communicationRegisters[__CCR] & __COM_PENDING)};
	}

	return (Package){0, false};
}

void CommunicationManager_update(CommunicationManager this)
{
	PRINT_INT(this->waitPayloadTries, 1, 8);
	PRINT_TEXT("Status: ", 1, 9);
	PRINT_INT(this->status, 1 + 12, 9);
	PRINT_TEXT("Handshake: ", 1, 10);
	PRINT_INT(this->handshake, 1 + 12, 10);
	PRINT_TEXT("Connected: ", 1, 11);
	PRINT_INT(this->handshake == kSendingKeepAlive || this->handshake == kWaitingKeepAlive, 1 + 12, 11);
	PRINT_TEXT("Mode: ", 1, 12);
	PRINT_TEXT(__COM_AS_REMOTE == this->communicationMode ? "Remote" : "Master ", 1 + 12, 12);
	PRINT_TEXT(_communicationRegisters[__CCR] & __COM_PENDING ? "Pending" : "Done  ", 1, 13);
	PRINT_HEX(_communicationRegisters[__CCR], 1 + 12, 13);
//	_communicationRegisters[__CCSR] = 0xFF;
	_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT;

	if(_communicationRegisters[__CCR] & __COM_PENDING)
	{
		PRINT_TIME(10, 4);
		return;
	}

		PRINT_TIME(1, 4);
		return;
	switch(this->status)
	{
		case kStatusIdle:

			switch(this->handshake)
			{
				case kHandshakeIdle:
					{
						PRINT_TIME(1, 3);
						this->waitPayloadTries = __WAIT_PAYLOAD_TRIES + Utilities_random(Utilities_randomSeed(), __WAIT_PAYLOAD_TRIES);

						CommunicationManager_waitHandshake(this, __HANDSHAKE_SIN);
/*
						if(CommunicationManager_waitHandshake(this, __HANDSHAKE_SIN))
						{
							MessageDispatcher_dispatchMessage(__COM_TIME_TO_WAIT_FOR_SYN + Utilities_random(Utilities_randomSeed(), __COM_TIME_TO_WAIT_FOR_SYN), __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kCommunicationsSendSyn, NULL);
						}
						*/
					}


					break;
			}

			break;

		case kWaitingPayload:

			switch(this->handshake)
			{
				case kWaitingSyn:

					if(0 < --this->waitPayloadTries)
					{
						CommunicationManager_waitHandshake(this, __HANDSHAKE_SIN);
					}
					else
					{
						CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN);
					}

					break;
/*
				case kWaitingAck:

					if(0 < --this->waitPayloadTries)
					{
						CommunicationManager_sendHandshake(this, __HANDSHAKE_SIN);
					}

					break;
					*/
			}

		default:

			PRINT_TIME(1, 4);
			break;
	}

/*
	PRINT_INT(sizeof(UserInput), 10, 10);
	PRINT_INT(sizeof(UserInput) * 8, 10, 11);
	PRINT_INT(sizeof(UserInput) * 8 *50, 10, 12);
	*/

}
