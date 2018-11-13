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
#include <Mem.h>
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
static volatile BYTE* _communicationRegisters =			(u8*)0x02000000;


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
	this->data = NULL;
	this->syncData = NULL;
	this->asyncData = NULL;
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
		CommunicationManager::setReady(this, false);
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
	CommunicationManager::setReady(this, false);
	// If channel was unsuccessfully closed,
	// then there is a handshake taking place
	return CommunicationManager::isRemoteReady(this);
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
		CommunicationManager::setReady(this, true);

		while(!CommunicationManager::isRemoteReady(this));
	}

	// Enable interrupts just before starting communications
	CommunicationManager::enableInterrupts(this);

	// Set Start flag
	_communicationRegisters[__CCR] |= __COM_START;

PRINT_TIME(10, 10);
PRINT_TEXT("11", 10, 11);

	// Check if the Stat flag is raised
	if(!isHandShake)
	{
		while(!(_communicationRegisters[__CCR] & __COM_PENDING))
		{
		        			PRINT_TIME(30, 10);
        			PRINT_TEXT("BB", 10, 11);

			// Set Start flag
			_communicationRegisters[__CCR] |= __COM_START;
		}

	}

	// Open communications channel
	CommunicationManager::setReady(this, true);

PRINT_TIME(10, 10);
PRINT_TEXT("22", 10, 11);
}

void CommunicationManager::stopTransmissions()
{
	_communicationRegisters[__CCR] &= (~__COM_START);
}

void CommunicationManager::setReady(bool ready)
{
	if(ready)
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
	else
	{
		_communicationRegisters[__CCSR] |= 0x02;
	}
}

bool CommunicationManager::isAuxChannelOpen()
{
	return _communicationRegisters[__CCSR] & 0x01 ? true : false;
}

bool CommunicationManager::isRemoteReady()
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

 static bool inInterrupt = false;
static void CommunicationManager::interruptHandler()
{
	if(inInterrupt)
	{
		PRINT_TEXT("ERROR", 30, 16);
	}
	inInterrupt = true;

	// Don't disable interrupts right away, otherwise it messes up with the lecture
	// of the transmitted data and with the sending of new data

	CommunicationManager::setReady(_communicationManager, false);

	CommunicationManager::processInterrupt(_communicationManager);
	inInterrupt = false;
}

/**
 * Process interrupt method
 */
void CommunicationManager::processInterrupt()
{
	int status = this->status;
	this->status = kCommunicationsStatusIdle;

	switch(status)
	{
		case kCommunicationsStatusSendingHandshake:

			if(__COM_HANDSHAKE != _communicationRegisters[__CDRR])
			{
				CommunicationManager::sendHandshake(this);
				break;
			}

			CommunicationManager::disableInterrupts(_communicationManager);

		default:

			this->connected = true;
			break;
	}

	switch(status)
	{
		case kCommunicationsStatusWaitingPayload:

			if(this->syncData)
			{
				*this->syncData = _communicationRegisters[__CDRR];
				CommunicationManager::disableInterrupts(_communicationManager);
				this->syncData++;
				this->numberOfBytesPendingTransmission--;
			}
			else if(this->asyncData)
			{
				*this->asyncData = _communicationRegisters[__CDRR];
				this->asyncData++;

				if(0 < --this->numberOfBytesPendingTransmission)
				{
					int breaker = 8000;
					if(CommunicationManager::isMaster(this))
					{
						while(0 > --breaker && CommunicationManager::isRemoteReady(this));
					}

					CommunicationManager::receivePayload(this);
				}
				else
				{
					CommunicationManager::disableInterrupts(_communicationManager);
					Object::fireEvent(Object::safeCast(this), kEventCommunicationsCompleted);
					Object::removeAllEventListeners(Object::safeCast(this), kEventCommunicationsCompleted);
					delete this->asyncData;
					this->data = this->asyncData = NULL;
				}
			}

			break;

		case kCommunicationsStatusSendingPayload:

			if(this->syncData)
			{
				CommunicationManager::disableInterrupts(_communicationManager);
				this->syncData++;
				this->numberOfBytesPendingTransmission--;
			}
			else if(this->asyncData)
			{
				this->asyncData++;

				if(0 < --this->numberOfBytesPendingTransmission)
				{
					int breaker = 8000;
					if(CommunicationManager::isMaster(this))
					{
						while(0 > --breaker && CommunicationManager::isRemoteReady(this));
					}

					CommunicationManager::sendPayload(this, *this->asyncData);
				}
				else
				{
					CommunicationManager::disableInterrupts(_communicationManager);
					Object::fireEvent(Object::safeCast(this), kEventCommunicationsCompleted);
					Object::removeAllEventListeners(Object::safeCast(this), kEventCommunicationsCompleted);
					delete this->asyncData;
					this->data = this->asyncData = NULL;
				}
			}

			break;
	}
}

bool CommunicationManager::startDataTransmission(volatile BYTE* data, volatile int numberOfBytes, volatile bool sendData)
{
	if(!this->connected || this->asyncData || !data || 0 >= numberOfBytes || this->status != kCommunicationsStatusIdle)
	{
		return false;
	}

	this->asyncData = NULL;
	this->data = this->syncData = data;
	this->numberOfBytesPendingTransmission = numberOfBytes;

	while(0 < this->numberOfBytesPendingTransmission)
	{
		if(CommunicationManager::isMaster(this))
		{
			volatile int i = 0; for(; i++ < 1*8000;);
		}
		if(sendData)
		{
			CommunicationManager::sendPayload(this, *this->syncData);
		}
		else
		{
			CommunicationManager::receivePayload(this);
		}

//		volatile bool didChannelClosed = !CommunicationManager::isRemoteReady(this);

		while(kCommunicationsStatusIdle != this->status);

PRINT_TIME(10, 10);
PRINT_TEXT("33", 10, 11);

		/*
		{
			if(!didChannelClosed)
			{
				didChannelClosed = !CommunicationManager::isRemoteReady(this);
			}
		}*/

		if(!CommunicationManager::isMaster(this))
		{
			volatile int i = 0; for(; i++ < 2*8000;);
		}

		//for(; !didChannelClosed; didChannelClosed = !CommunicationManager::isRemoteReady(this));

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
PRINT_TIME(10, 10);
PRINT_TEXT("44", 10, 11);
	}

	this->status = kCommunicationsStatusIdle;
	this->data = this->syncData = NULL;

	//HardwareManager::enableInterrupts();
	CommunicationManager::setReady(this, false);

PRINT_TIME(10, 10);
PRINT_TEXT("55", 10, 11);

	return true;
}

bool CommunicationManager::sendData(BYTE* data, int numberOfBytes)
{
	return CommunicationManager::startDataTransmission(this, data, numberOfBytes, true);
}

bool CommunicationManager::receiveData(BYTE* data, int numberOfBytes)
{
	return CommunicationManager::startDataTransmission(this, data, numberOfBytes, false);
}

bool CommunicationManager::startDataTransmissionAsync(BYTE* data, int numberOfBytes, bool sendData, EventListener eventLister, Object scope)
{
	if(!this->connected || this->syncData || this->asyncData || (sendData && !data) || 0 >= numberOfBytes || this->status != kCommunicationsStatusIdle || Object::hasActiveEventListeners(Object::safeCast(this)))
	{
		return false;
	}

	Object::addEventListener(this, scope, eventLister, kEventCommunicationsCompleted);
	this->syncData = NULL;
	this->data = this->asyncData = MemoryPool::allocate(MemoryPool::getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD;
	this->numberOfBytesPendingTransmission = numberOfBytes;

	if(sendData)
	{
		Mem::copyBYTE((BYTE*)this->asyncData, data, numberOfBytes);
		CommunicationManager::sendPayload(this, *this->asyncData);
	}
	else
	{
		CommunicationManager::receivePayload(this);
	}

	return true;
}

bool CommunicationManager::sendDataAsync(BYTE* data, int numberOfBytes, EventListener eventLister, Object scope)
{
	return CommunicationManager::startDataTransmissionAsync(this, data, numberOfBytes, true, eventLister, scope);
}

bool CommunicationManager::receiveDataAsync(int numberOfBytes, EventListener eventLister, Object scope)
{
	return CommunicationManager::startDataTransmissionAsync(this, NULL, numberOfBytes, false, eventLister, scope);
}

const BYTE* CommunicationManager::getData()
{
	return (const BYTE*)this->data;
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
	PRINT_TEXT(CommunicationManager::isRemoteReady(this) ? "Comms Open    " : "Comms Closed ", x + valueDisplacement, y++);

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