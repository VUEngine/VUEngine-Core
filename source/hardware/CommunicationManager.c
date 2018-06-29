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

enum CommunicationsStatus
{
	kCommunicationsStatusIdle = 1,
	kCommunicationsStatusSendingPayload,
	kCommunicationsStatusWaitingPayload,
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
#define	__COM_AS_REMOTE			(__COM_USE_EXTERNAL_CLOCK)
// start communication as master
#define	__COM_AS_MASTER			(0x00)


#define __COM_KEEP_ALIVE_TIMEOUT						100

#define __KEEP_ALIVE									0x44


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

	CommunicationManager::reset(this);

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
	_communicationRegisters[__CCR] |= __COM_DISABLE_INTERRUPT;
}

void CommunicationManager::openChannel()
{
	if(CommunicationManager::managesChannel(this))
	{
		_communicationRegisters[__CCSR] &= (~0x02);
	}
}

void CommunicationManager::closeChannel()
{
	if(CommunicationManager::managesChannel(this))
	{
		_communicationRegisters[__CCSR] |= 0x02;
	}
}

bool CommunicationManager::isChannelOpen()
{
	return _communicationRegisters[__CCSR] & 0x01 ? false : true;
}

bool CommunicationManager::managesChannel()
{
	return !CommunicationManager::isMaster(this);
}

bool CommunicationManager::isConnected()
{
	return this->connected;
}

bool CommunicationManager::isMaster()
{
	return __COM_AS_MASTER == this->communicationMode;
}

void CommunicationManager::reset()
{
	this->connected = false;
	this->status = kCommunicationsStatusIdle;
	this->numberOfBytesPendingTransmission = 0;
	this->dataPointer = NULL;
	this->communicationMode = __COM_AS_REMOTE;
	CommunicationManager::disableInterrupts(this);
}

void CommunicationManager::enableCommunications()
{
	CommunicationManager::disableInterrupts(this);

	// At reset, COMCNT's read values default to 1
    if(CommunicationManager::isChannelOpen(this))
    {
		// If it is 0, then there is another system attached
		// to the link cable and booted first
		this->communicationMode = __COM_AS_MASTER;

		// send dummy payload to verify communication
		CommunicationManager::sendPayload(this, 0xFF);
	}
	else
	{
		// If they are equal, then there is no system attached
		// to the link cable, so I'm booting first and
		// will control the COMCNT
		this->communicationMode = __COM_AS_REMOTE;

		// Wait for incoming clock
		CommunicationManager::receivePayload(this);
    }
}

bool CommunicationManager::cancelCommunications()
{
	if(kCommunicationsStatusIdle != this->status)
	{
		this->status = kCommunicationsStatusIdle;
		CommunicationManager::disableInterrupts(this);
		CommunicationManager::closeChannel(this);
		return true;
	}

	return false;
}

void CommunicationManager::startTransmissions()
{
	_communicationRegisters[__CCR] = this->communicationMode;
	CommunicationManager::enableInterrupts(this);
	_communicationRegisters[__CCR] |= __COM_START;

	// Open up channel (only if remote)
	CommunicationManager::openChannel(this);
}

bool CommunicationManager::sendPayload(u8 payload)
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingPayload;
		_communicationRegisters[__CDTR] = payload;
		CommunicationManager::startTransmissions(this);
		return true;
	}

	return false;
}

bool CommunicationManager::receivePayload()
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusWaitingPayload;
		CommunicationManager::startTransmissions(this);
		return true;
	}

	return false;
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
}

/**
 * Process interrupt method
 */
void CommunicationManager::processInterrupt()
{
	switch(this->status)
	{
		case kCommunicationsStatusIdle:

			// do nothing
			break;

		case kCommunicationsStatusWaitingPayload:

			if(this->dataPointer)
			{
				CommunicationManager::closeChannel(this);
				this->numberOfBytesPendingTransmission--;
				*this->dataPointer = _communicationRegisters[__CDRR];
				this->dataPointer++;
			}

			break;

		case kCommunicationsStatusSendingPayload:

			if(this->dataPointer)
			{
				CommunicationManager::closeChannel(this);
				this->numberOfBytesPendingTransmission--;
				this->dataPointer++;
			}
			break;
	}

	this->status = kCommunicationsStatusIdle;
	this->connected = true;
}

void CommunicationManager::update()
{
//	CommunicationManager::printStatus(this, 1, 5);
	int timeRemaining = (int)this->timeout - (int)TimerManager_getTotalMillisecondsElapsed(TimerManager_getInstance());

	switch(this->status)
	{
		case kCommunicationsStatusIdle:

			break;

		case kCommunicationsStatusWaitingPayload:
		case kCommunicationsStatusSendingPayload:

			break;
	}
}

void CommunicationManager::sendData(BYTE* data, int numberOfBytes)
{
	if(!data || 0 >= numberOfBytes || this->status != kCommunicationsStatusIdle)
	{
		return;
	}

	this->dataPointer = data;
	this->numberOfBytesPendingTransmission = numberOfBytes;

	volatile int* numberOfBytesPendingTransmission = &this->numberOfBytesPendingTransmission;
	volatile int* status = &this->status;
	while(0 < *numberOfBytesPendingTransmission)
	{
		{
			volatile int counter = 8000;
			for(; 0 < --counter;);
		}

		// Check for channel to be ready before proceeding
		while(!CommunicationManager::managesChannel(this) && !CommunicationManager::isChannelOpen(this));

		// Send data
		CommunicationManager::sendPayload(this, *this->dataPointer);

		// Wait for communications to end
		while(kCommunicationsStatusIdle != *status);
	}

	this->dataPointer = NULL;
	this->numberOfBytesPendingTransmission = 0;
}

void CommunicationManager::receiveData(BYTE* data, int numberOfBytes)
{
	if(!data || 0 >= numberOfBytes || this->status != kCommunicationsStatusIdle)
	{
		return;
	}

	this->dataPointer = data;
	this->numberOfBytesPendingTransmission = numberOfBytes;

	volatile int* numberOfBytesPendingTransmission = &this->numberOfBytesPendingTransmission;
	volatile int* status = &this->status;
	while(0 < *numberOfBytesPendingTransmission)
	{
		{
			volatile int counter = 8000;
			for(; 0 < --counter;);
		}

		// Check for channel to be ready before proceeding
		while(!CommunicationManager::managesChannel(this) && !CommunicationManager::isChannelOpen(this));

		// Receive data
		CommunicationManager::receivePayload(this);

		// Wait for communications to end
		while(kCommunicationsStatusIdle != *status);
	}

	this->dataPointer = NULL;
	this->numberOfBytesPendingTransmission = 0;
}

void CommunicationManager::printStatus(int x, int y)
{
	PRINT_TEXT(CommunicationManager::isConnected(this) ? "Connected   " : "Disconnected", x, y);

	char* helper = "";
	int valueDisplacement = 20;

	PRINT_TIME(x + valueDisplacement, y++);
	PRINT_TEXT("Time out:                  ", x, y);
	PRINT_INT(this->timeout, x + valueDisplacement, y++);

	switch(this->status)
	{
		case kCommunicationsStatusIdle:
			helper = "Idle     ";
			break;
		case kCommunicationsStatusWaitingPayload:
			helper = "Waiting  ";
			break;
		case kCommunicationsStatusSendingPayload:
			helper = "Sending  ";
			break;
		default:
			helper = "Error";
			break;
	}

	PRINT_TEXT("Status:", x, y);
	PRINT_TEXT(helper, x + valueDisplacement, y++);

	PRINT_TEXT("Mode: ", x, y);
	PRINT_TEXT(__COM_AS_REMOTE == this->communicationMode ? "Remote" : "Master ", x + valueDisplacement, y++);

	PRINT_TEXT(CommunicationManager::isChannelOpen(this) ? "Ready" : "Not ready ", x + valueDisplacement, y++);

	PRINT_TEXT("Bytes left:", x, y);
	PRINT_INT(this->numberOfBytesPendingTransmission, x + valueDisplacement, y++);

	PRINT_TEXT(_communicationRegisters[__CCR] & __COM_PENDING ? "Pending" : "Done  ", 1, y++);
	PRINT_TEXT("CCR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCR], x + valueDisplacement, y++);
	PRINT_TEXT("CCSR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCSR], x + valueDisplacement, y++);

	PRINT_TEXT("Write: ", x, y);
	PRINT_INT((_communicationRegisters[__CCSR] >> 1) & 0x01, x + valueDisplacement, y++);
	PRINT_TEXT("Read: ", x, y);
	PRINT_INT(_communicationRegisters[__CCSR] & 0x01, x + valueDisplacement, y++);

}