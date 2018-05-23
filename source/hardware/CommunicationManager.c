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
	kSendingPayload,
	kWaitingPayload,
};

enum HandshakeStatus
{
	kHandshakeIdle = 0,
	kSendingSyn,
	kWaitingSyn,
	kSendingAck,
	kWaitingAck,
	kSendingKeepAlive,
	kWaitingKeepAlive,
	kSendingMessage,
	kWaitingMessage,
	kWaitingToSendKeepAlive,
	kWaitingToWaitKeepAlive
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



#define __COM_TIME_TO_WAIT_FOR_RESPONSE		10
#define __HANDSHAKE_TRIES					10000
#define __COM_WAIT_TO_FINISH				-1
#define __HANDSHAKE_SIN						0x11
#define __HANDSHAKE_ACK						0x22
#define __HANDSHAKE_FIN						0x33
#define __KEEP_ALIVE						0x44
#define __WAIT_RESPONSE_TRIES				100


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

static CommunicationManager _communicationManager;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CommunicationManager::getInstance()
 * @memberof	CommunicationManager
 * @public
 * @return		CommunicationManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void CommunicationManager::constructor()
{
	Base::constructor();

	CommunicationManager::resetCommunications(this);

	_communicationManager = this;
}

/**
 * Class destructor
 */
void CommunicationManager::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Enable interrupts
 */
void CommunicationManager::enableInterrupts()
{
	_communicationRegisters[__CCR] &= ~__COM_DISABLE_INTERRUPT;
}

/**
 * Disable interrupts
 */
void CommunicationManager::disableInterrupts()
{
	// disable interrupts to allow sends without affecting unready receiver
//	_communicationRegisters[__CCSR] = 0xFF;
	_communicationRegisters[__CCR] |= __COM_DISABLE_INTERRUPT;
}

/**
 * Enable communications
 */
void CommunicationManager::enableCommunications()
{

}

/**
 * Disable communications
 */
void CommunicationManager::disableCommunications()
{
}

/**
 * Communication's interrupt handler
 */
static void CommunicationManager::interruptHandler()
{
	// disable interrupts
	CommunicationManager::disableInterrupts(_communicationManager);

	// handle the interrupt
	CommunicationManager::processInterrupt(_communicationManager);

	// enable interrupts
//	CommunicationManager::enableInterrupts(_communicationManager);
}

void CommunicationManager::resetCommunications()
{
	// reset
	this->communicationMode = __COM_AS_REMOTE;
	this->status = kStatusIdle;
	this->sequenceNumber = 1;
	this->handshake = kHandshakeIdle;
	this->messageToSelf = kNoMessage;
	this->waitCycles = __COM_TIME_TO_WAIT_FOR_RESPONSE + Utilities::random(Utilities::randomSeed(), __COM_TIME_TO_WAIT_FOR_RESPONSE);
	CommunicationManager::disableInterrupts(this);
}

bool CommunicationManager::sendHandshake(u8 handshakePayload)
{
	//CommunicationManager::disableInterrupts(this);

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
		this->handshake = kWaitingToSendKeepAlive;
		this->sequenceNumber++;
	}

	this->status = kStatusIdle;
	CommunicationManager::sendPayload(this, handshakePayload);
	this->messageToSelf = kCommunicationsReset;

	this->waitCycles = __COM_TIME_TO_WAIT_FOR_RESPONSE + Utilities::random(Utilities::randomSeed(), __COM_TIME_TO_WAIT_FOR_RESPONSE);
	return true;
}

bool CommunicationManager::waitHandshake(u8 handshakePayload, bool sendMessage)
{
	this->messageToSelf = sendMessage ? kCommunicationsReset : this->messageToSelf;

	//CommunicationManager::disableInterrupts(this);

	if(__HANDSHAKE_SIN == handshakePayload)
	{
		this->communicationMode = __COM_AS_REMOTE;
		this->handshake = kWaitingSyn;
		this->messageToSelf = sendMessage ? kCommunicationsSendSyn : this->messageToSelf;
	}
	else if(__HANDSHAKE_ACK == handshakePayload)
	{
		this->handshake = kWaitingAck;
	}
	else if(__KEEP_ALIVE == handshakePayload)
	{
		this->handshake = kWaitingToWaitKeepAlive;
		this->sequenceNumber++;
	}

	this->status = kStatusIdle;
	CommunicationManager::receivePayload(this);

	if(sendMessage)
	{
		this->waitCycles = __COM_TIME_TO_WAIT_FOR_RESPONSE + Utilities::random(Utilities::randomSeed(), __COM_TIME_TO_WAIT_FOR_RESPONSE);
	}

	return true;
}

/**
 * Process interrupt method
 */
static void CommunicationManager::processInterrupt()
{
//	CommunicationManager::printStatus(this, 25, 5);
	CommunicationManager this = _communicationManager;

	switch(this->status)
	{
		case kStatusIdle:

			// do nothing
			break;

		case kWaitingPayload:
			{
				Package package = (Package){_communicationRegisters[__CDRR], !(_communicationRegisters[__CCR] & __COM_PENDING)};

/*				if(!package.isValid)
				{
					PRINT_TEXT("!package.isValid", 20, 24);
					PRINT_HEX(__HANDSHAKE_SIN, 30, 23);
					PRINT_HEX(package.payload, 30, 24);
					CommunicationManager::enableInterrupts(_communicationManager);
					return;
				}
*/

				switch(this->handshake)
				{
					case kWaitingSyn:

						if(__HANDSHAKE_SIN == package.payload)
						{
							CommunicationManager::sendHandshake(this, __HANDSHAKE_ACK);
						}
						else
						{
	//PRINT_TIME(1, 3);
							CommunicationManager::waitHandshake(this, __HANDSHAKE_SIN, false);
						}

						break;

					case kWaitingAck:

						if(__HANDSHAKE_ACK == package.payload)
						{
							CommunicationManager::sendHandshake(this, __KEEP_ALIVE);
						}
						else
						{
	//PRINT_TIME(8, 3);
							CommunicationManager::waitHandshake(this, __HANDSHAKE_ACK, false);
						}

						break;

					case kWaitingKeepAlive:

						if(__KEEP_ALIVE == package.payload)
						//if(package.isValid && this->sequenceNumber == package.payload)
						{
							CommunicationManager::sendHandshake(this, __KEEP_ALIVE);
						}
						else
						{
	//PRINT_TIME(15, 3);
							CommunicationManager::waitHandshake(this, __KEEP_ALIVE, false);
						}

						break;
				}
			}

			break;

		case kSendingPayload:
			{
				Package package = (Package){_communicationRegisters[__CDTR], !(_communicationRegisters[__CCR] & __COM_PENDING)};
/*
				if(!package.isValid)
				{
					CommunicationManager::resetCommunications(this);
					return;
				}
*/
				switch(this->handshake)
				{
					case kSendingSyn:

//	PRINT_TIME(22, 3);
						// wait for ack
						CommunicationManager::waitHandshake(this, __HANDSHAKE_ACK, true);
						break;

					case kSendingAck:

//	PRINT_TIME(29, 3);
						// wait for keep alive
						CommunicationManager::waitHandshake(this, __KEEP_ALIVE, true);
						break;

					case kSendingKeepAlive:

//	PRINT_TIME(36, 3);
						// wait for keep alive
						CommunicationManager::waitHandshake(this, __KEEP_ALIVE, true);
						break;
				}

				break;
			}
	}

//	CommunicationManager::printStatus(this, 25, 15);
}

void CommunicationManager::handleMessageToSelf()
{
	switch(this->messageToSelf)
	{
		case kCommunicationsSendSyn:

			// try as master
			CommunicationManager::sendHandshake(this, __HANDSHAKE_SIN);
			PRINT_TIME(20, 13);
			PRINT_TEXT("M", 20, 14);

			break;

		case kCommunicationsReset:

			// reset
			CommunicationManager::resetCommunications(this);
			PRINT_TIME(20, 15);
			PRINT_TEXT("R", 20, 14);
			break;

		case kNoMessage:

			switch(this->status)
        	{
        		case kStatusIdle:

        			switch(this->handshake)
        			{
        				case kHandshakeIdle:

        					CommunicationManager::waitHandshake(this, __HANDSHAKE_SIN, true);
        					break;
        			}

        			break;
        	}
	}
}

bool CommunicationManager::sendPayload(u8 payload)
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

bool CommunicationManager::receivePayload()
{
	if(kStatusIdle == this->status)
	{
		this->status = kWaitingPayload;
		_communicationRegisters[__CCR] = this->communicationMode;

		return true;
	}

	return false;
}

Package CommunicationManager::getPackage()
{
	if(kStatusIdle == this->status)
	{
		return (Package){_communicationRegisters[__CDRR], !(_communicationRegisters[__CCR] & __COM_PENDING)};
	}

	return (Package){0, false};
}

void CommunicationManager::update()
{
	CommunicationManager::printStatus(this, 1, 5);
//	_communicationRegisters[__CCSR] = 0xFF;
//	_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT;

	if(0 >= --this->waitCycles)
	{
		CommunicationManager::handleMessageToSelf(this);
	}

/*
	PRINT_INT(sizeof(UserInput), 10, 10);
	PRINT_INT(sizeof(UserInput) * 8, 10, 11);
	PRINT_INT(sizeof(UserInput) * 8 *50, 10, 12);
	*/
}

void CommunicationManager::printStatus(int x, int y)
{
	PRINT_TIME(x, y++);
	PRINT_TEXT("Status:", x, y);
	char* helper = "";

	switch(this->status)
	{
		case kStatusIdle:
			helper = "Idle   ";
			break;
		case kWaitingPayload:
			helper = "Waiting";
			break;
		case kSendingPayload:
			helper = "Sending";
			break;
	}

	PRINT_TEXT(helper, x + 12, y++);
	PRINT_TEXT("Handshake:", 1, y);

	switch(this->handshake)
	{
		case kHandshakeIdle:
			helper = "Idle    ";
			break;
		case kSendingSyn:
		case kWaitingSyn:
			helper = "Syn     ";
			break;
		case kSendingAck:
		case kWaitingAck:
			helper = "Ack      ";
			break;
		case kSendingKeepAlive:
		case kWaitingKeepAlive:
			helper = "KeepAlive";
			break;
	}

	PRINT_TEXT(helper, x + 12, y++);

	PRINT_TEXT(this->handshake == kSendingKeepAlive || this->handshake == kWaitingKeepAlive ? "Connected" : "           ", x, y++);
	PRINT_TEXT("Mode: ", x, y);
	PRINT_TEXT(__COM_AS_REMOTE == this->communicationMode ? "Remote" : "Master ", x + 12, y++);
//	PRINT_TEXT(_communicationRegisters[__CCR] & __COM_PENDING ? "Pending" : "Done  ", 1, y++);
/*	PRINT_TEXT("CCR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCR], x + 12, y++);
	PRINT_TEXT("CCSR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCSR], x + 12, y);
	*/
}