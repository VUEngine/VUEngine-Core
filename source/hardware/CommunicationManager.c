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

	// communication messages
enum CommunicationManagerMessages
{
	kCommunicationsListenForHandshake = 1,
	kCommunicationsTryStartingHandshake,
	kCommunicationsWaitForKeepalive,
	kCommunicationsWaitToSendKeepalive,
	kCommunicationsReset,
};

enum CommunicationManagerStatus
{
	kStatusIdle = 1,
	kSendingPayload,
	kWaitingPayload,
};

enum HandshakeStatus
{
	kHandshakeIdle = 1,
	kSendingSyn,
	kWaitingSyn,
	kSendingAck,
	kWaitingAck,
	kSendingKeepAlive,
	kWaitingKeepAlive,
	kSendingMessage,
	kWaitingMessage,
};

enum NextHandshakeAction
{
	kCommunicationsNoAction = 1,
	kCommunicationsSendSyn,
	kCommunicationsWaitSyn,
	kCommunicationsSendAck,
	kCommunicationsWaitAck,
	kCommunicationsSendKeepAlive,
	kCommunicationsWaitKeepAlive,
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



#define __COM_TIME_TO_WAIT_FOR_RESPONSE		50
#define __COM_WAIT_TO_FINISH				-1
#define __HANDSHAKE_SIN						0x11
#define __HANDSHAKE_ACK						0x22
#define __HANDSHAKE_FIN						0x33
#define __KEEP_ALIVE						0x44


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
	// CommunicationManager::enableInterrupts(_communicationManager);
}

void CommunicationManager::resetCommunications()
{
	// reset
	this->communicationMode = __COM_AS_REMOTE;
	this->status = kStatusIdle;
	this->sequenceNumber = 1;
	this->handshake = kHandshakeIdle;
	this->nextHandshakeAction = kCommunicationsNoAction;
	CommunicationManager::waitForSelfMessage(this, kCommunicationsListenForHandshake, __COM_TIME_TO_WAIT_FOR_RESPONSE);
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
		this->handshake = kSendingKeepAlive;
		handshakePayload = this->sequenceNumber;
	}

	this->status = kStatusIdle;
	CommunicationManager::sendPayload(this, handshakePayload);

	return true;
}

bool CommunicationManager::waitHandshake(u8 handshakePayload)
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
	}

	this->status = kStatusIdle;
	CommunicationManager::receivePayload(this);

	return true;
}

/**
 * Process interrupt method
 */
static void CommunicationManager::processInterrupt()
{
	CommunicationManager this = _communicationManager;
//	CommunicationManager::printStatus(this, 25, 5);

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

				// give up and start communications again after a while
				switch(this->handshake)
				{
					case kWaitingSyn:

						if(__HANDSHAKE_SIN == package.payload)
						{
							this->nextHandshakeAction = kCommunicationsSendAck;
							CommunicationManager::waitForSelfMessage(this, kCommunicationsReset, __COM_TIME_TO_WAIT_FOR_RESPONSE);
						}
						break;

					case kWaitingAck:

						if(__HANDSHAKE_ACK == package.payload)
						{
							this->sequenceNumber++;
							this->nextHandshakeAction = kCommunicationsSendKeepAlive;
							CommunicationManager::waitForSelfMessage(this, kCommunicationsReset, __COM_TIME_TO_WAIT_FOR_RESPONSE);
						}

						break;

					case kWaitingKeepAlive:
/*
{

	static bool done = false;

	if(!done)
	{
						if(this->sequenceNumber == package.payload)
							PRINT_TEXT("Match SQN   ", 1, 20);
						else
							PRINT_TEXT("No match SQN", 1, 20);

							PRINT_INT(this->sequenceNumber, 1, 21);
							PRINT_INT(package.payload, 1, 22);
	}
	done = true;

}
*/

//						if(package.isValid && this->sequenceNumber == package.payload)
						if(package.isValid && abs(this->sequenceNumber - package.payload) < 10)
						{
												PRINT_TIME(10, 18);

							this->sequenceNumber++;
							CommunicationManager::waitForSelfMessage(this, kCommunicationsWaitToSendKeepalive, __COM_TIME_TO_WAIT_FOR_RESPONSE);
						}
						else
						{
						PRINT_TIME(21, 18);
							CommunicationManager::waitForSelfMessage(this, kCommunicationsReset, 0);
						}

						break;
				}
			}

			break;

		case kSendingPayload:
			{
				// give up and start communications again after a while
				CommunicationManager::waitForSelfMessage(this, kCommunicationsReset, __COM_TIME_TO_WAIT_FOR_RESPONSE);

				switch(this->handshake)
				{
					case kSendingSyn:

						this->nextHandshakeAction = kCommunicationsWaitAck;
						break;

					case kSendingAck:

						this->sequenceNumber++;
						this->nextHandshakeAction = kCommunicationsWaitKeepAlive;
						break;

					case kSendingKeepAlive:

						this->sequenceNumber++;
						CommunicationManager::waitForSelfMessage(this, kCommunicationsWaitForKeepalive, __COM_TIME_TO_WAIT_FOR_RESPONSE);

						PRINT_TIME(1, 18);
						break;
				}

				break;
			}
	}

//	CommunicationManager::printStatus(this, 25, 15);
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

void CommunicationManager::waitForSelfMessage(int messageToSelf, int minimumValue)
{
	this->messageToSelf = messageToSelf;
	this->waitCycles = minimumValue + Utilities::random(Utilities::randomSeed(), minimumValue);
}

void CommunicationManager::handleMessageToSelf()
{
	switch(this->messageToSelf)
	{
		case kCommunicationsListenForHandshake:

			this->nextHandshakeAction = kCommunicationsWaitSyn;

			// give up and try to send syn after a while
			CommunicationManager::waitForSelfMessage(this, kCommunicationsTryStartingHandshake, __COM_TIME_TO_WAIT_FOR_RESPONSE);
        	break;

		case kCommunicationsTryStartingHandshake:

			this->nextHandshakeAction = kCommunicationsSendSyn;

			// give up and start communications again after a while
			CommunicationManager::waitForSelfMessage(this, kCommunicationsReset, __COM_TIME_TO_WAIT_FOR_RESPONSE);
			break;

		case kCommunicationsWaitForKeepalive:

			this->nextHandshakeAction = kCommunicationsWaitKeepAlive;

						PRINT_TIME(31, 18);

			// give up and start communications again after a while
			CommunicationManager::waitForSelfMessage(this, kCommunicationsReset, __COM_TIME_TO_WAIT_FOR_RESPONSE);
			break;

		case kCommunicationsWaitToSendKeepalive:

			this->nextHandshakeAction = kCommunicationsSendKeepAlive;
						PRINT_TIME(41, 18);

			// give up and start communications again after a while
			CommunicationManager::waitForSelfMessage(this, kCommunicationsWaitForKeepalive, __COM_TIME_TO_WAIT_FOR_RESPONSE);
			break;

		case kCommunicationsReset:

			// reset
			CommunicationManager::resetCommunications(this);
			break;
	}
}

void CommunicationManager::checkNextAction()
{
	switch(this->nextHandshakeAction)
	{
		case kCommunicationsNoAction:
			break;

		case kCommunicationsWaitSyn:

			CommunicationManager::waitHandshake(this, __HANDSHAKE_SIN);
			break;

		case kCommunicationsWaitAck:

			CommunicationManager::waitHandshake(this, __HANDSHAKE_ACK);
			break;

		case kCommunicationsWaitKeepAlive:

			CommunicationManager::waitHandshake(this, __KEEP_ALIVE);
			break;

		case kCommunicationsSendSyn:

			// try as master
			CommunicationManager::sendHandshake(this, __HANDSHAKE_SIN);
			//this->nextHandshakeAction = kCommunicationsNoAction;
			break;

		case kCommunicationsSendAck:

			CommunicationManager::sendHandshake(this, __HANDSHAKE_ACK);
			//this->nextHandshakeAction = kCommunicationsNoAction;
			break;

		case kCommunicationsSendKeepAlive:

			CommunicationManager::sendHandshake(this, __KEEP_ALIVE);
			this->nextHandshakeAction = kCommunicationsNoAction;
			break;
	}
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
	else
	{
		CommunicationManager::checkNextAction(this);
	}

/*
	PRINT_INT(sizeof(UserInput), 10, 10);
	PRINT_INT(sizeof(UserInput) * 8, 10, 11);
	PRINT_INT(sizeof(UserInput) * 8 *50, 10, 18);
	*/
}

void CommunicationManager::printStatus(int x, int y)
{
	PRINT_TIME(x, y++);
	char* helper = "";
	int valueDisplacement = 13;

	PRINT_TEXT("Wait cycles:     ", x, y);
	PRINT_INT(this->waitCycles, x + valueDisplacement, y++);

	switch(this->messageToSelf)
	{
		case kCommunicationsListenForHandshake:
			helper = "Listen for handshake ";
			break;

		case kCommunicationsTryStartingHandshake:
			helper = "Try handshake        ";
			break;

		case kCommunicationsWaitForKeepalive:
			helper = "Wait keepalive       ";
			break;

		case kCommunicationsWaitToSendKeepalive:
			helper = "Wait send keepalive  ";
			break;

		case kCommunicationsReset:
			helper = "Reset                 ";
			break;

		default:
			helper = "Error in message";
			break;
	}

	PRINT_TEXT("Message:", x, y);
	PRINT_TEXT(helper, x + valueDisplacement, y++);

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
		default:
			helper = "Error in status";
			break;
	}

	PRINT_TEXT("Status:", x, y);
	PRINT_TEXT(helper, x + valueDisplacement, y++);

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
		default:
			helper = "Error in handshake";
			break;
	}

	PRINT_TEXT("Handshake:", x, y);
	PRINT_TEXT(helper, x + valueDisplacement, y++);

	switch(this->nextHandshakeAction)
	{
		case kCommunicationsNoAction:
			helper = "No action";
			break;

		case kCommunicationsWaitSyn:
			helper = "Wait SYN  ";
			break;

		case kCommunicationsWaitAck:
			helper = "Wait ACK  ";
			break;

		case kCommunicationsWaitKeepAlive:
			helper = "Wait KPA  ";
			break;

		case kCommunicationsSendSyn:
			helper = "Send SYN  ";
			break;

		case kCommunicationsSendAck:
			helper = "Send ACK  ";
			break;

		case kCommunicationsSendKeepAlive:

			helper = "Send KPA  ";
			break;
	}

	PRINT_TEXT("Next action:", x, y);
	PRINT_TEXT(helper, x + valueDisplacement, y++);

	PRINT_TEXT("Mode: ", x, y);
	PRINT_TEXT(__COM_AS_REMOTE == this->communicationMode ? "Remote" : "Master ", x + valueDisplacement, y++);
	PRINT_TEXT("KPA seq.:    ", x, y);
	PRINT_INT(this->sequenceNumber, x + valueDisplacement, y++);
	PRINT_TEXT(this->handshake == kSendingKeepAlive || this->handshake == kWaitingKeepAlive ? "Connected" : "           ", x, y++);
//	PRINT_TEXT(_communicationRegisters[__CCR] & __COM_PENDING ? "Pending" : "Done  ", 1, y++);
/*	PRINT_TEXT("CCR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCR], x + valueDisplacement, y++);
	PRINT_TEXT("CCSR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCSR], x + valueDisplacement, y);
	*/
}