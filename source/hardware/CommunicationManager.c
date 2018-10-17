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
	_communicationRegisters[__CCSR] |= __COM_DISABLE_INTERRUPT;
}

/**
 * Disable interrupts
 */
void CommunicationManager::disableInterrupts()
{
	_communicationRegisters[__CCR] |= __COM_DISABLE_INTERRUPT;
	_communicationRegisters[__CCSR] |= __COM_DISABLE_INTERRUPT;
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
		// system would had already closed the channel
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
		CommunicationManager::closeCommunicationsChannel(this);
		return true;
	}

	return false;
}

bool CommunicationManager::sendHandshake()
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingHandshake;
		CommunicationManager::startTransmissions(this, __COM_HANDSHAKE, true);
		return true;
	}

	return false;
}

bool CommunicationManager::isHandshakeIncoming()
{
	// Try to close the communication channel
	CommunicationManager::closeCommunicationsChannel(this);
	// If channel was unsuccessfully closed,
	// then there is a handshake taking place
	return CommunicationManager::isCommunicationsChannelOpen(this);
}

void CommunicationManager::startTransmissions(u8 payload, bool isHandShake)
{
	// Select clock
	_communicationRegisters[__CCR] = this->communicationMode;
	// Set transmission data
	_communicationRegisters[__CDTR] = payload;

	// Master must wait for slave to open the channel
	if(CommunicationManager::isMaster(this))
	{
		CommunicationManager::openCommunicationsChannel(this);

		while(!CommunicationManager::isCommunicationsChannelOpen(this));
	}

	// Enable interrupts just before starting communications
	CommunicationManager::enableInterrupts(this);

	// Set Start flag
	_communicationRegisters[__CCR] |= __COM_START;

	// Check if the Stat flag is raised
	if(!isHandShake)
	{
		while(!(_communicationRegisters[__CCR] & __COM_PENDING))
		{
			// Set Start flag
			_communicationRegisters[__CCR] |= __COM_START;
		}
	}

	// Open communications channel
	CommunicationManager::openCommunicationsChannel(this);
}

void CommunicationManager::stopTransmissions()
{
	_communicationRegisters[__CCR] &= (~__COM_START);
}

void CommunicationManager::openAuxChannel()
{
	_communicationRegisters[__CCSR] |= 0x02;
}

void CommunicationManager::closeAuxChannel()
{
	_communicationRegisters[__CCSR] &= (~0x02);
}

void CommunicationManager::openCommunicationsChannel()
{
	if(CommunicationManager::isMaster(this))
	{
		_communicationRegisters[__CCSR] |= 0x02;
	}
	else
	{
		_communicationRegisters[__CCSR] &= (~0x02);
	}
}

void CommunicationManager::closeCommunicationsChannel()
{
	_communicationRegisters[__CCSR] |= 0x02;
}

bool CommunicationManager::isAuxChannelOpen()
{
	return _communicationRegisters[__CCSR] & 0x01 ? true : false;
}

bool CommunicationManager::isCommunicationsChannelOpen()
{
	return 0 == (_communicationRegisters[__CCSR] & 0x01) ? true : false;
}

bool CommunicationManager::sendPayload(u8 payload)
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingPayload;
		CommunicationManager::startTransmissions(this, payload, false);
		return true;
	}

	return false;
}

bool CommunicationManager::receivePayload()
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusWaitingPayload;
		CommunicationManager::startTransmissions(this, __COM_CHECKSUM, false);
		return true;
	}

	return false;
}

/**
 * Communication's interrupt handler
 */
static void CommunicationManager::interruptHandler()
{
//	u8 ccr = _communicationRegisters[__CCR];
//	u8 ccsr = _communicationRegisters[__CCSR];
	bool pending = _communicationRegisters[__CCR] & __COM_PENDING;
	CommunicationManager::disableInterrupts(_communicationManager);
	CommunicationManager::stopTransmissions(_communicationManager);
/*
	if(pending)
	{
		PRINT_TIME(3, 26);
/*		PRINT_HEX(ccr & __COM_PENDING, 20, 8);
		PRINT_HEX(_communicationRegisters[__CCR] & __COM_PENDING, 20, 9);
			PRINT_TEXT("CCR: ", 5, 10);
			PRINT_HEX(ccr, 20, 10);
			PRINT_TEXT("CCSR: ", 5, 11);
			PRINT_HEX(ccsr, 20, 11);
*/
/*
		if(kCommunicationsStatusSendingHandshake != _communicationManager->status)
		{
			PRINT_TEXT("ERROR ", 5, 14);
			PRINT_TEXT("CCR: ", 5, 15);
			PRINT_HEX(ccr, 5, 16);
			PRINT_HEX(_communicationRegisters[__CCR], 5, 17);
			while(1);

			CommunicationManager::enableInterrupts(_communicationManager);

			return;
		}
	}
	else
	{
		PRINT_TIME(40, 26);
	}
*/
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

	CommunicationManager::closeCommunicationsChannel(this);

	switch(status)
	{
		case kCommunicationsStatusSendingHandshake:

			if(__COM_HANDSHAKE != _communicationRegisters[__CDRR])
			{
				CommunicationManager::sendHandshake(this);
				break;
			}

			this->connected = true;

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

void CommunicationManager::startDataTransmission(BYTE* data, int numberOfBytes, bool sendData)
{
	if(!this->connected || !data || 0 >= numberOfBytes || this->status != kCommunicationsStatusIdle)
	{
		return;
	}

	this->data = data;
		PRINT_TEXT("AA", 5, 24);

	while((int)this->data < (int)data + numberOfBytes)
	{
		PRINT_TEXT("BB", 5, 24);

		if(CommunicationManager::isMaster(this))
    	{
		PRINT_TEXT("CC", 5, 24);
			while(!CommunicationManager::isCommunicationsChannelOpen(this))
			{
		PRINT_TIME(5, 23);
		PRINT_TEXT("DD", 5, 24);
			//	CommunicationManager::printStatus(this, 5, 7);
			}
		}
		PRINT_TEXT("EE", 5, 24);

		if(sendData)
		{
		PRINT_TEXT("FF", 5, 24);
			CommunicationManager::sendPayload(this, *this->data);
		PRINT_TEXT("GG", 5, 24);
		}
		else
		{
		PRINT_TEXT("HH", 5, 24);
			CommunicationManager::receivePayload(this);
		PRINT_TEXT("II", 5, 24);
		}

		PRINT_TEXT("JJ", 5, 24);
		while(kCommunicationsStatusIdle != this->status)
		{
		PRINT_TIME(5, 23);
		PRINT_TEXT("KK", 5, 24);
		//	CommunicationManager::printStatus(this, 5, 7);
		}
		PRINT_TEXT("HH", 5, 24);
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
		PRINT_TEXT("II", 5, 24);
	this->status = kCommunicationsStatusIdle;
	this->data = NULL;

	//HardwareManager::enableInterrupts();
	CommunicationManager::closeCommunicationsChannel(this);
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
	PRINT_TEXT(CommunicationManager::isAuxChannelOpen(this) ? "Aux Open    " : "Aux Closed ", x + valueDisplacement, y++);
	PRINT_TEXT(CommunicationManager::isCommunicationsChannelOpen(this) ? "Comms Open    " : "Comms Closed ", x + valueDisplacement, y++);

	PRINT_TEXT("Transmission: ", x, y);
	PRINT_TEXT(_communicationRegisters[__CCR] & __COM_PENDING ? "Pending" : "Done  ", x + valueDisplacement, y++);
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