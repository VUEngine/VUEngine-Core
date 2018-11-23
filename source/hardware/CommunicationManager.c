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
#include <VIPManager.h>
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
	kCommunicationsStatusSendingAndReceivingPayload,
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
	CommunicationManager::endCommunications(this);
	this->connected = false;
	this->sentData = NULL;
	this->syncSentByte = NULL;
	this->asyncSentByte = NULL;
	this->receivedData = NULL;
	this->syncReceivedByte = NULL;
	this->asyncReceivedByte = NULL;
	this->status = kCommunicationsStatusIdle;
	this->communicationMode = __COM_AS_REMOTE;
}

void CommunicationManager::enableCommunications()
{
	CommunicationManager::endCommunications(this);

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

void CommunicationManager::endCommunications()
{
	CommunicationManager::setReady(this, false);

	_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT;
}

bool CommunicationManager::cancelCommunications()
{
	if(kCommunicationsStatusIdle != this->status)
	{
		this->status = kCommunicationsStatusIdle;
		CommunicationManager::endCommunications(this);
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
		CommunicationManager::startTransmissions(this, __COM_HANDSHAKE);
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

void CommunicationManager::startTransmissions(u8 payload)
{
	// Master must wait for slave to open the channel
	if(CommunicationManager::isMaster(this))
	{
		CommunicationManager::setReady(this, true);

		while(!CommunicationManager::isRemoteReady(this));

		// Set transmission data
		_communicationRegisters[__CDTR] = payload;

		// Set Start flag
		_communicationRegisters[__CCR] = __COM_START;

	}
	else
	{
		// Set transmission data
		_communicationRegisters[__CDTR] = payload;

		// Set Start flag
		_communicationRegisters[__CCR] = __COM_USE_EXTERNAL_CLOCK | __COM_START;

		// Open communications channel
		CommunicationManager::setReady(this, true);
	}
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
		CommunicationManager::startTransmissions(this, payload);
		return true;
	}

	return false;
}

bool CommunicationManager::receivePayload()
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusWaitingPayload;
		CommunicationManager::startTransmissions(this, __COM_CHECKSUM);
		return true;
	}

	return false;
}

bool CommunicationManager::sendAndReceivePayload(u8 payload)
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingAndReceivingPayload;
		CommunicationManager::startTransmissions(this, payload);
		return true;
	}

	return false;
}

/**
 * Communication's interrupt handler
 */
static void CommunicationManager::interruptHandler()
{
	if(CommunicationManager::isMaster(_communicationManager))
	{
		_communicationManager->communicationMode = __COM_AS_REMOTE;
	}
	else
	{
		_communicationManager->communicationMode = __COM_AS_MASTER;
	}

	// End communications
	CommunicationManager::endCommunications(_communicationManager);

	// Process interrupt
	CommunicationManager::processInterrupt(_communicationManager);
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

		default:

			this->connected = true;
			break;
	}

	switch(status)
	{
		case kCommunicationsStatusWaitingPayload:

			if(this->syncReceivedByte)
			{
				*this->syncReceivedByte = _communicationRegisters[__CDRR];
				this->syncReceivedByte++;
				--this->numberOfBytesPendingTransmission;
			}
			else if(this->asyncReceivedByte)
			{
				*this->asyncReceivedByte = _communicationRegisters[__CDRR];
				this->asyncReceivedByte++;

				if(0 < --this->numberOfBytesPendingTransmission)
				{
					CommunicationManager::receivePayload(this);
				}
				else
				{
					Object::fireEvent(Object::safeCast(this), kEventCommunicationsCompleted);
					Object::removeAllEventListeners(Object::safeCast(this), kEventCommunicationsCompleted);
					delete this->receivedData;
					this->receivedData = this->asyncReceivedByte = NULL;
				}
			}

			break;

		case kCommunicationsStatusSendingPayload:

			if(this->syncSentByte)
			{
				this->syncSentByte++;
				--this->numberOfBytesPendingTransmission;
			}
			else if(this->asyncSentByte)
			{
				this->asyncSentByte++;

				if(0 < --this->numberOfBytesPendingTransmission)
				{
					CommunicationManager::sendPayload(this, *this->asyncSentByte);
				}
				else
				{
					Object::fireEvent(Object::safeCast(this), kEventCommunicationsCompleted);
					Object::removeAllEventListeners(Object::safeCast(this), kEventCommunicationsCompleted);
					delete this->sentData;
					this->sentData = this->asyncSentByte = NULL;
				}
			}

			break;

		case kCommunicationsStatusSendingAndReceivingPayload:

			if(this->syncSentByte && this->syncReceivedByte)
			{
				*this->syncReceivedByte = _communicationRegisters[__CDRR];
				this->syncReceivedByte++;
				this->syncSentByte++;
				--this->numberOfBytesPendingTransmission;
			}
			else if(this->asyncSentByte && this->asyncReceivedByte)
			{
				*this->asyncReceivedByte = _communicationRegisters[__CDRR];
				this->asyncReceivedByte++;
				this->asyncSentByte++;

				if(0 < --this->numberOfBytesPendingTransmission)
				{
					CommunicationManager::sendAndReceivePayload(this, *this->asyncSentByte);
				}
				else
				{
					Object::fireEvent(Object::safeCast(this), kEventCommunicationsCompleted);
					Object::removeAllEventListeners(Object::safeCast(this), kEventCommunicationsCompleted);
					delete this->sentData;
					delete this->receivedData;
					this->sentData = this->receivedData = this->asyncSentByte = this->asyncReceivedByte = NULL;
				}
			}

			break;
	}
}

bool CommunicationManager::isFreeForTransmissions()
{
	return
		!this->connected ||
		this->syncSentByte ||
		this->syncReceivedByte ||
		this->asyncSentByte ||
		this->asyncReceivedByte ||
		this->status != kCommunicationsStatusIdle ||
		Object::hasActiveEventListeners(Object::safeCast(this));
}

bool CommunicationManager::startDataTransmission(BYTE* data, int numberOfBytes, bool sendingData)
{
	if((sendingData && !data) || 0 >= numberOfBytes || CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	this->sentData = this->receivedData = NULL;
	this->syncSentByte = this->syncReceivedByte = NULL;
	this->asyncSentByte = this->asyncReceivedByte = NULL;

	if(sendingData)
	{
		this->syncSentByte = data;
	}
	else
	{
		this->syncReceivedByte = data;
	}

	this->numberOfBytesPendingTransmission = numberOfBytes;

	while(0 < this->numberOfBytesPendingTransmission)
	{
		if(sendingData)
		{
			CommunicationManager::sendPayload(this, *this->syncSentByte);
		}
		else
		{
			CommunicationManager::receivePayload(this);
		}

		while(kCommunicationsStatusIdle != this->status);
	}

	this->status = kCommunicationsStatusIdle;
	this->syncSentByte = this->syncReceivedByte = NULL;

	CommunicationManager::setReady(this, false);

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

bool CommunicationManager::startBidirectionalDataTransmission(BYTE* sentData, BYTE* receivedData, int numberOfBytes)
{
	if((!sentData || !receivedData) || 0 >= numberOfBytes || CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	this->sentData = this->receivedData = NULL;
	this->syncSentByte = this->syncReceivedByte = NULL;
	this->asyncSentByte = this->asyncReceivedByte = NULL;

	this->syncSentByte = sentData;
	this->syncReceivedByte = receivedData;

	this->numberOfBytesPendingTransmission = numberOfBytes;

	while(0 < this->numberOfBytesPendingTransmission)
	{
		CommunicationManager::sendAndReceivePayload(this, *this->syncSentByte);

		while(kCommunicationsStatusIdle != this->status);
	}

	this->status = kCommunicationsStatusIdle;
	this->syncSentByte = this->syncReceivedByte = NULL;

	CommunicationManager::setReady(this, false);

	return true;
}

bool CommunicationManager::sendAndReceiveData(BYTE* sentData, BYTE* receivedData, int numberOfBytes)
{
	return CommunicationManager::startBidirectionalDataTransmission(this, sentData, receivedData, numberOfBytes);
}

bool CommunicationManager::startDataTransmissionAsync(BYTE* data, int numberOfBytes, bool sendingData, EventListener eventLister, Object scope)
{
	if((sendingData && !data) || 0 >= numberOfBytes || CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	this->sentData = this->receivedData = NULL;
	this->syncSentByte = this->syncReceivedByte = NULL;
	this->asyncSentByte = this->asyncReceivedByte = NULL;

	if(eventLister && !isDeleted(scope))
	{
		Object::addEventListener(this, scope, eventLister, kEventCommunicationsCompleted);
	}

	this->numberOfBytesPendingTransmission = numberOfBytes;

	if(sendingData)
	{
		this->sentData = this->asyncSentByte = (BYTE*)((u32)MemoryPool_allocate(MemoryPool_getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);
		Mem::copyBYTE((BYTE*)this->asyncSentByte, data, numberOfBytes);
		CommunicationManager::sendPayload(this, *this->asyncSentByte);
	}
	else
	{
		this->receivedData = this->asyncReceivedByte = (BYTE*)((u32)MemoryPool_allocate(MemoryPool_getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);
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

bool CommunicationManager::startBidirectionalDataTransmissionAsync(BYTE* data, int numberOfBytes, EventListener eventLister, Object scope)
{
	if(!data || 0 >= numberOfBytes || CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	this->sentData = this->receivedData = NULL;
	this->syncSentByte = this->syncReceivedByte = NULL;
	this->asyncSentByte = this->asyncReceivedByte = NULL;

	if(eventLister && !isDeleted(scope))
	{
		Object::addEventListener(this, scope, eventLister, kEventCommunicationsCompleted);
	}

	this->sentData = this->asyncSentByte = (BYTE*)((u32)MemoryPool_allocate(MemoryPool_getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);
	this->receivedData = this->asyncReceivedByte = (BYTE*)((u32)MemoryPool_allocate(MemoryPool_getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);
	this->numberOfBytesPendingTransmission = numberOfBytes;

	Mem::copyBYTE((BYTE*)this->asyncSentByte, data, numberOfBytes);
	CommunicationManager::sendAndReceivePayload(this, *this->asyncSentByte);

	return true;
}

bool CommunicationManager::sendAndReceiveDataAsync(BYTE* data, int numberOfBytes, EventListener eventLister, Object scope)
{
	return CommunicationManager::startBidirectionalDataTransmissionAsync(this, data, numberOfBytes, eventLister, scope);
}

const BYTE* CommunicationManager::getData()
{
	return (const BYTE*)this->receivedData;
}

void CommunicationManager::startSyncCycle()
{
	extern volatile u16* _vipRegisters;

	_vipRegisters[__FRMCYC] = 0;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE);

#define __REMOTE_READY_MESSAGE			0x43873AD1
#define __MASTER_FRMCYC_SET_MESSAGE		0x5DC289F4

	if(CommunicationManager::isMaster(this))
	{
		u32 message = 0;
		do
		{
			CommunicationManager::receiveData(this, (BYTE*)&message, sizeof(message));
		}
		while(__REMOTE_READY_MESSAGE != message);

		message = __MASTER_FRMCYC_SET_MESSAGE;
		while(!(_vipRegisters[__DPSTTS] & __FCLK));
		CommunicationManager::sendData(this, (BYTE*)&message, sizeof(message));
	}
	else
	{
		u32 message = __REMOTE_READY_MESSAGE;
		CommunicationManager::sendData(this, (BYTE*)&message, sizeof(message));

		do
		{
			CommunicationManager::receiveData(this, (BYTE*)&message, sizeof(message));
		}
		while(__MASTER_FRMCYC_SET_MESSAGE != message);
	}
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