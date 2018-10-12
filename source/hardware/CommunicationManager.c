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
	kCommunicationsStatusSendingHandshake,
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
#define	__COM_AS_REMOTE			(__COM_DISABLE_INTERRUPT | __COM_USE_EXTERNAL_CLOCK)
// start communication as master
#define	__COM_AS_MASTER			(__COM_DISABLE_INTERRUPT |  0x00)


#define __COM_HANDSHAKE					0x34
#define __COM_KEEP_ALIVE_TIMEOUT		100
#define __COM_CHECKSUM					0x44


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
	_communicationRegisters[__CCSR] |= __COM_DISABLE_INTERRUPT;
}

void CommunicationManager::openChannel()
{
	_communicationRegisters[__CCSR] |= 0x02;
}

void CommunicationManager::closeChannel()
{
	_communicationRegisters[__CCSR] &= (~0x02);
}

bool CommunicationManager::isHandshakeIncoming()
{
	// If channel was successfully opened,
	// then there is no handshake taking place
	return !CommunicationManager::isChannelOpen(this);
}

bool CommunicationManager::isChannelOpen()
{
	return _communicationRegisters[__CCSR] & 0x01 ? true : false;
}

bool CommunicationManager::isTransmitting()
{
	return _communicationRegisters[__CCR] & __COM_PENDING ? true : false;
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
	CommunicationManager::disableInterrupts(this);
	this->connected = false;
	this->status = kCommunicationsStatusIdle;
	this->communicationMode = __COM_AS_REMOTE;
}

void CommunicationManager::enableCommunications()
{
	CommunicationManager::disableInterrupts(this);

	// Wait a little bit for channel to stabilize
	TimerManager::wait(TimerManager::getInstance(), 2000);

	// If handshake is taking place
    if(CommunicationManager::isHandshakeIncoming(this))
    {
		// There is another system attached already managing
		// the channel
		this->communicationMode = __COM_AS_MASTER;

		// send dummy payload to verify communication
		CommunicationManager::sendHandshake(this);
	}
	else
	{
		// I'm alone since a previously turned on
		// system had to already closed the channel
		this->communicationMode = __COM_AS_REMOTE;

		// Wait for incoming clock
		CommunicationManager::sendHandshake(this);
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

void CommunicationManager::startTransmissions(bool keepTrying)
{
	_communicationRegisters[__CCR] = this->communicationMode;
	CommunicationManager::enableInterrupts(this);
	_communicationRegisters[__CCR] |= __COM_START;
	while(keepTrying && !CommunicationManager::isTransmitting(this));
}

bool CommunicationManager::sendPayload(u8 payload)
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingPayload;
		_communicationRegisters[__CDTR] = payload;
		CommunicationManager::startTransmissions(this, true);
		CommunicationManager::openChannel(this);

		return true;
	}

	return false;
}

bool CommunicationManager::receivePayload()
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusWaitingPayload;
		_communicationRegisters[__CDTR] = __COM_CHECKSUM;
		CommunicationManager::startTransmissions(this, true);
		CommunicationManager::openChannel(this);
		return true;
	}

	return false;
}

bool CommunicationManager::sendHandshake()
{
	if(kCommunicationsStatusIdle == this->status)
	{
		CommunicationManager::closeChannel(this);
		this->status = kCommunicationsStatusSendingHandshake;
		_communicationRegisters[__CDTR] = __COM_HANDSHAKE;
		CommunicationManager::startTransmissions(this, false);
		return true;
	}

	return false;
}

/**
 * Communication's interrupt handler
 */
static void CommunicationManager::interruptHandler()
{
	CommunicationManager::disableInterrupts(_communicationManager);

	CommunicationManager::processInterrupt(_communicationManager);
}

/**
 * Process interrupt method
 */
void CommunicationManager::processInterrupt()
{
	int status = this->status;
	this->status = kCommunicationsStatusIdle;

	bool switchMode = false;

	switch(status)
	{
		case kCommunicationsStatusSendingHandshake:

			if(__COM_HANDSHAKE != _communicationRegisters[__CDRR])
			{
				CommunicationManager::sendHandshake(this);
				break;
			}

		default:

			this->connected = true;
			break;
	}

	switch(status)
	{
		case kCommunicationsStatusWaitingPayload:

			if(this->data)
			{
				*this->data = _communicationRegisters[__CDRR];
				this->data++;
			}

			switchMode = true;

			break;

		case kCommunicationsStatusSendingPayload:

			if(this->data)
			{
				this->data++;
			}

			switchMode = true;
			break;
	}
/*
	if(switchMode)
	{
		if(CommunicationManager::isMaster(this))
    	{
    		this->communicationMode = __COM_AS_REMOTE;
    	}
    	else
    	{
    		this->communicationMode = __COM_AS_MASTER;
    	}
	}*/
}

void CommunicationManager::processInterrupt1()
{
	switch(this->status)
	{
		case kCommunicationsStatusSendingHandshake:

			if(__COM_HANDSHAKE != _communicationRegisters[__CDRR])
			{
				this->connected = false;
				this->status = kCommunicationsStatusIdle;
				CommunicationManager::sendHandshake(this);
			}
			else
			{
				this->connected = true;
			}

			break;

		case kCommunicationsStatusWaitingPayload:

			this->status = kCommunicationsStatusIdle;
			CommunicationManager::closeChannel(this);

			if(this->data)
			{
//				while(_communicationRegisters[__CCR] & __COM_PENDING);
				*this->data = _communicationRegisters[__CDRR];
				this->data++;
			}

			break;

		case kCommunicationsStatusSendingPayload:

			this->status = kCommunicationsStatusIdle;
			CommunicationManager::closeChannel(this);

			if(this->data)
			{
//				while(_communicationRegisters[__CCR] & __COM_PENDING);
				this->data++;
			}

			break;
	}
}

void CommunicationManager::startDataTransmission(BYTE* data, int numberOfBytes, bool sendData)
{
	if(!this->connected || !data || 0 >= numberOfBytes || this->status != kCommunicationsStatusIdle)
	{
		return;
	}

	this->data = data;

	//HardwareManager::disableInterrupts();
	while((int)this->data < (int)data + numberOfBytes)
	{
		if(CommunicationManager::isMaster(this))
    	{
			CommunicationManager::openChannel(this);
			while(!CommunicationManager::isChannelOpen(this));
		}

		if(sendData)
		{
			CommunicationManager::sendPayload(this, *this->data);
		}
		else
		{
			CommunicationManager::receivePayload(this);
		}

		// Wait for transmissions to complete

//		volatile int* status = &this->status;
	while(CommunicationManager::isTransmitting(this))
	{
//		CommunicationManager::printStatus(this, 5, 7);
	}

	CommunicationManager::closeChannel(this);

///		while(kCommunicationsStatusIdle != this->status);
/*
		if(CommunicationManager::isMaster(this))
		{
			this->communicationMode = __COM_AS_REMOTE;
		}
		else
		{
			this->communicationMode = __COM_AS_MASTER;
		}
*/
	}

	this->status = kCommunicationsStatusIdle;
	this->data = NULL;

	//HardwareManager::enableInterrupts();
	CommunicationManager::closeChannel(this);
}

void CommunicationManager::sendData(BYTE* data, int numberOfBytes)
{
	CommunicationManager::startDataTransmission(this, data, numberOfBytes, true);
}

void CommunicationManager::receiveData(BYTE* data, int numberOfBytes)
{
	CommunicationManager::startDataTransmission(this, data, numberOfBytes, false);
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
			helper = "Idle             ";
			break;
		case kCommunicationsStatusSendingHandshake:
			helper = "Sending handshake";
			break;
		case kCommunicationsStatusWaitingPayload:
			helper = "Waiting payload  ";
			break;
		case kCommunicationsStatusSendingPayload:
			helper = "Sending payload  ";
			break;
		default:
			helper = "Error            ";
			break;
	}

	PRINT_TEXT("Status:", x, y);
	PRINT_INT(this->status, x + valueDisplacement * 2, y);
	PRINT_TEXT(helper, x + valueDisplacement, y++);

	PRINT_TEXT("Mode: ", x, y);
	PRINT_TEXT(__COM_AS_REMOTE == this->communicationMode ? "Remote" : "Master ", x + valueDisplacement, y++);

	PRINT_TEXT("Channel: ", x, y);
	PRINT_TEXT(CommunicationManager::isChannelOpen(this) ? "Open    " : "Closed ", x + valueDisplacement, y++);

	PRINT_TEXT(_communicationRegisters[__CCR] & __COM_PENDING ? "Pending" : "Done  ", 1, y++);
	PRINT_TEXT("CCR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCR], x + valueDisplacement, y++);
	PRINT_TEXT("CCSR: ", x, y);
	PRINT_HEX(_communicationRegisters[__CCSR], x + valueDisplacement, y++);

	PRINT_TEXT("Sig: ", x, y);
	PRINT_INT((_communicationRegisters[__CCSR] >> 3) & 0x01, x + valueDisplacement, y++);
	PRINT_TEXT("Smp: ", x, y);
	PRINT_INT((_communicationRegisters[__CCSR] >> 2) & 0x01, x + valueDisplacement, y++);

	PRINT_TEXT("Write: ", x, y);
	PRINT_INT((_communicationRegisters[__CCSR] >> 1) & 0x01, x + valueDisplacement, y++);
	PRINT_TEXT("Read: ", x, y);
	PRINT_INT(_communicationRegisters[__CCSR] & 0x01, x + valueDisplacement, y++);

}