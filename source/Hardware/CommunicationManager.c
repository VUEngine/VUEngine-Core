/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __DEBUG_TOOL
#include <Debug.h>
#endif
#include <DebugConfig.h>
#include <Mem.h>
#include <Printer.h>
#include <Profiler.h>
#include <Telegram.h>
#include <Singleton.h>
#include <VIPManager.h>
#include <VUEngine.h>

#include "CommunicationManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum CommunicationsBroadcastStates
{
	kCommunicationsBroadcastNone = 0,
	kCommunicationsBroadcastSync,
	kCommunicationsBroadcastAsync
};

enum CommunicationsStatus
{
	kCommunicationsStatusReset = 0,
	kCommunicationsStatusIdle,
	kCommunicationsStatusSendingHandshake,
	kCommunicationsStatusSendingPayload,
	kCommunicationsStatusWaitingPayload,
	kCommunicationsStatusSendingAndReceivingPayload,
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

volatile uint8* _communicationRegisters =			(uint8*)0x02000000;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define	__CCR							0x00	// Communication Control Register	(0x0200 0000)
#define	__CCSR							0x04	// COMCNT Control Register			(0x0200 0004)
#define	__CDTR							0x08	// Transmitted Data Register		(0x0200 0008)
#define	__CDRR							0x0C	// Received Data Register			(0x0200 000C)

// communicating penging flag
#define	__COM_PENDING					0x02
// start communication
#define	__COM_START						0x04
// use external clock (remote)
#define	__COM_USE_EXTERNAL_CLOCK		0x10
// disable interrupt
#define	__COM_DISABLE_INTERRUPT			0x80

// start communication as remote
#define	__COM_AS_REMOTE					(__COM_USE_EXTERNAL_CLOCK)
// start communication as master
#define	__COM_AS_MASTER					(0x00)

#define __COM_HANDSHAKE					0x34
#define __COM_CHECKSUM					0x44

#define	__MESSAGE_SIZE					sizeof(uint32)

#define __REMOTE_READY_MESSAGE			0x43873AD1
#define __MASTER_FRMCYC_SET_MESSAGE		0x5DC289F4

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::interruptHandler()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeStartInterrupt, NULL);
#endif

	if(kCommunicationsBroadcastNone == communicationManager->broadcast && CommunicationManager::isMaster(communicationManager))
	{
		communicationManager->communicationMode = __COM_AS_REMOTE;
	}
	else
	{
		communicationManager->communicationMode = __COM_AS_MASTER;
	}

	CommunicationManager::endCommunications(communicationManager);

	CommunicationManager::processInterrupt(communicationManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageCheckIfRemoteIsReady:
		{
			if(CommunicationManager::isRemoteReady(this))
			{
				CommunicationManager::startClockSignal(this);
			}
			else
			{
				CommunicationManager::waitForRemote(this);
			}

			return true;
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::reset()
{
	CommunicationManager::cancelCommunications(this);

	this->status = kCommunicationsStatusReset;
	this->broadcast = kCommunicationsBroadcastNone;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::enableCommunications(ListenerObject scope)
{
	if(this->connected || kCommunicationsStatusReset != this->status)
	{
		return;
	}

	CommunicationManager::endCommunications(this);

	if(!isDeleted(scope))
	{
		CommunicationManager::addEventListener(this, scope, kEventCommunicationsConnected);
	}

	// If handshake is taking place
	if(CommunicationManager::isHandshakeIncoming(this))
	{
		// There is another system attached already managing the channel
		this->communicationMode = __COM_AS_MASTER;

		// Send dummy payload to verify communication
		CommunicationManager::sendHandshake(this);
	}
	else
	{
		// I'm alone since a previously turned on
		// System would had already closed the channel
		this->communicationMode = __COM_AS_REMOTE;

		// Wait for incoming clock
		CommunicationManager::sendHandshake(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::disableCommunications()
{
	CommunicationManager::reset(this);	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::cancelCommunications()
{
	CommunicationManager::endCommunications(this);

	if(NULL != this->sentData)
	{
		delete this->sentData;
	}

	if(NULL != this->receivedData)
	{
		delete this->receivedData;
	}

	this->sentData = NULL;
	this->syncSentByte = NULL;
	this->asyncSentByte = NULL;
	this->receivedData = NULL;
	this->syncReceivedByte = NULL;
	this->asyncReceivedByte = NULL;
	this->status = kCommunicationsStatusIdle;
	this->broadcast = kCommunicationsBroadcastNone;
	this->numberOfBytesPendingTransmission = 0;
	this->numberOfBytesPreviouslySent = 0;

	CommunicationManager::removeEventListeners(this, NULL, kEventEngineFirst);
	CommunicationManager::discardAllMessages(this);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::startSyncCycle()
{
	if(!this->connected)
	{
		return;
	}

	CommunicationManager::cancelCommunications(this);

	VIPManager::startMemoryRefresh(VIPManager::getInstance());

	CommunicationManager::correctDataDrift(this);

	if(CommunicationManager::isMaster(this))
	{
		uint32 message = 0;
		do
		{
			CommunicationManager::receiveData(this, (uint8*)&message, sizeof(message));
		}
		while(__REMOTE_READY_MESSAGE != message);

		message = __MASTER_FRMCYC_SET_MESSAGE;

		VIPManager::waitForFRAMESTART(VIPManager::getInstance());

		CommunicationManager::sendData(this, (uint8*)&message, sizeof(message));
	}
	else
	{
		uint32 message = __REMOTE_READY_MESSAGE;
		CommunicationManager::sendData(this, (uint8*)&message, sizeof(message));

		do
		{
			CommunicationManager::receiveData(this, (uint8*)&message, sizeof(message));
		}
		while(__MASTER_FRMCYC_SET_MESSAGE != message);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isConnected()
{
	return this->connected;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isMaster()
{
	return __COM_AS_MASTER == this->communicationMode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure bool CommunicationManager::broadcastData(uint8* data, int32 numberOfBytes)
{
	if(!CommunicationManager::getReadyToBroadcast(this))
	{
		return false;
	}

	this->broadcast = kCommunicationsBroadcastSync;

	while(0 < numberOfBytes)
	{
		CommunicationManager::sendPayload(this, *data, false);

		while(CommunicationManager::isTransmitting(this));
		this->status = kCommunicationsStatusIdle;

		data++;

		numberOfBytes--;
	}
	
	this->broadcast = kCommunicationsBroadcastNone;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure bool CommunicationManager::broadcastDataAsync(uint8* data, int32 numberOfBytes, ListenerObject scope)
{
	if(!CommunicationManager::getReadyToBroadcast(this))
	{
		return false;
	}

	this->broadcast = kCommunicationsBroadcastAsync;

	CommunicationManager::sendDataAsync(this, data, numberOfBytes, scope);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::sendAndReceiveData(uint32 message, uint8* data, int32 numberOfBytes)
{
	if(kCommunicationsStatusReset == this->status)
	{
		CommunicationManager::startSyncCycle(CommunicationManager::getInstance());
	}

	return CommunicationManager::startBidirectionalDataTransmission(this, message, data, numberOfBytes);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::sendAndReceiveDataAsync
(
	uint32 message, uint8* data, int32 numberOfBytes, ListenerObject scope
)
{
	if(kCommunicationsStatusReset == this->status)
	{
		CommunicationManager::startSyncCycle(CommunicationManager::getInstance());
	}

	return CommunicationManager::startBidirectionalDataTransmissionAsync(this, message, data, numberOfBytes, scope);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 CommunicationManager::getSentMessage()
{
	return *(uint32*)this->sentData;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 CommunicationManager::getReceivedMessage()
{
	return *(uint32*)this->receivedData;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const uint8* CommunicationManager::getSentData()
{
	return (const uint8*)this->sentData + __MESSAGE_SIZE;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const uint8* CommunicationManager::getReceivedData()
{
	return (const uint8*)this->receivedData + __MESSAGE_SIZE;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void CommunicationManager::print(int32 x, int32 y)
{
	PRINT_TEXT(CommunicationManager::isConnected(this) ? "Connected   " : "Disconnected", x, y);

	char* helper = "";
	int32 valueDisplacement = 20;

	PRINT_TIME(x + valueDisplacement, y++);

	switch(this->status)
	{
		case kCommunicationsStatusIdle:
		{
			helper = "Idle             ";
			break;
		}

		case kCommunicationsStatusSendingHandshake:
		{
			helper = "Sending handshake         ";
			break;
		}

		case kCommunicationsStatusWaitingPayload:
		{
			helper = "Waiting payload            ";
			break;
		}

		case kCommunicationsStatusSendingPayload:
		{
			helper = "Sending payload            ";
			break;
		}

		case kCommunicationsStatusSendingAndReceivingPayload:
		{
			helper = "Send / Recv payload        ";
			break;
		}

		default:
		{
			helper = "Error            ";
			break;
		}
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
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->connected = false;
	this->communicationMode = __COM_AS_REMOTE;
	this->status = kCommunicationsStatusReset;
	this->broadcast = kCommunicationsBroadcastNone;
	this->sentData = NULL;
	this->syncSentByte = NULL;
	this->asyncSentByte = NULL;
	this->receivedData = NULL;
	this->syncReceivedByte = NULL;
	this->asyncReceivedByte = NULL;
	this->numberOfBytesPreviouslySent = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isTransmitting()
{
	return _communicationRegisters[__CCR] & __COM_PENDING ? true : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::managesChannel()
{
	return !CommunicationManager::isMaster(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isHandshakeIncoming()
{
	// Try to close the communication channel
	CommunicationManager::setReady(this, false);

	// If channel was unsuccessfully closed,
	// Then there is a handshake taking place
	return CommunicationManager::isRemoteReady(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isRemoteReady()
{
	return 0 == (_communicationRegisters[__CCSR] & 0x01) ? true : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isCommunicationControlInterrupt()
{
	return 0 == (_communicationRegisters[__CCSR] & __COM_DISABLE_INTERRUPT);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isAuxChannelOpen()
{
	return _communicationRegisters[__CCSR] & 0x01 ? true : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::isFreeForTransmissions()
{
	return (
		(this->connected || this->broadcast) &&
		NULL == this->syncSentByte &&
		NULL == this->syncReceivedByte &&
		NULL == this->asyncSentByte &&
		NULL == this->asyncReceivedByte &&
		kCommunicationsStatusIdle >= this->status
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::endCommunications()
{
	CommunicationManager::setReady(this, false);

	_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT;
	_communicationRegisters[__CDTR] = 0;
	_communicationRegisters[__CDRR] = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::waitForRemote()
{
	CommunicationManager::sendMessageToSelf(this, kMessageCheckIfRemoteIsReady, 1, 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::sendHandshake()
{
	this->status = kCommunicationsStatusSendingHandshake;
	CommunicationManager::startTransmissions(this, __COM_HANDSHAKE, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::startClockSignal()
{
	// Make sure to disable COMCNT interrupts
	// _communicationRegisters[__CCSR] |= __COM_DISABLE_INTERRUPT;

	switch(this->broadcast)
	{
		case kCommunicationsBroadcastSync:
		{
			_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT | this->communicationMode | __COM_START;

			this->numberOfBytesPendingTransmission = 0;
			this->status = kCommunicationsStatusIdle;
			break;
		}

		default:
		{
			// Start communications
			_communicationRegisters[__CCR] = this->communicationMode | __COM_START;
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::correctDataDrift()
{
	CommunicationManager::cancelCommunications(this);

	// The received message and data can be shifted due to the chance of this method being called out of sync
	// So, we create composite value to circle shift the result and compare with it
	uint64 target = (((uint64)(uint32)CommunicationManager::getClass()) << 32) | (*(uint32*)CommunicationManager::getClass());

	do
	{
		// Data transmission can fail if there was already a request to send data.
		if
		(
			!CommunicationManager::startBidirectionalDataTransmission
			(
				this, (uint32)CommunicationManager::getClass(), 
				(uint8*)CommunicationManager::getClass(), sizeof((uint32)CommunicationManager::getClass())
			)
		)
		{
			// In this case, simply cancel all communications and try again. This supposes
			// that there are no other calls that could cause a race condition.
			CommunicationManager::cancelCommunications(this);
			continue;
		}

		// Create the composite result to circle shift the results
		uint64 result = (((uint64)CommunicationManager::getReceivedMessage(this)) << 32) | (*(uint32*)CommunicationManager::getReceivedData(this));

		if(target == result)
		{
			return;
		}

		// Shifts be of 4 bits each
		int16 shifts = 64;
		
		while(0 < shifts)
		{
			// Circle shift the result fo the left
			result = ((result & 0xFF00000000000000) >> 60) | (result << 4);

			// If the shifted result is equal to the target
			if(result == target)
			{
				// Performe a dummy transmission to shift the send data by a byte
				CommunicationManager::startBidirectionalDataTransmission(this, (uint32)result, (uint8*)&result, 1);

				break;
			}
			
			shifts -= 4;
		}
	}
	while(true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::getReadyToBroadcast()
{
	if(CommunicationManager::isConnected(this))
	{
		return false;
	}

	switch (this->status)
	{
		case kCommunicationsStatusSendingHandshake:
		{
			CommunicationManager::cancelCommunications(this);			
		}
		// Intended fallthrough
		case kCommunicationsStatusIdle:
		case kCommunicationsStatusReset:
		{
			this->status = kCommunicationsStatusIdle;
			break;
		}

		default:
		{
			return false;
		}
	}

	// Always start comms as master when broadcasting
	this->communicationMode = __COM_AS_MASTER;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::startTransmissions(uint8 payload, bool async)
{
	// Set transmission data
	_communicationRegisters[__CDTR] = payload;

	HardwareManager::enableInterrupts();

	// Master must wait for slave to open the channel
	if(CommunicationManager::isMaster(this))
	{
		CommunicationManager::setReady(this, true);

		if(kCommunicationsBroadcastNone != this->broadcast)
		{
			CommunicationManager::startClockSignal(this);
			return;
		}

		if(async)
		{
			if(CommunicationManager::isRemoteReady(this))
			{
				CommunicationManager::startClockSignal(this);
			}
			else
			{
				CommunicationManager::waitForRemote(this);
			}
		}
		else
		{
			while(!CommunicationManager::isRemoteReady(this));

			CommunicationManager::startClockSignal(this);
		}
	}
	else
	{
		// Set Start flag
		CommunicationManager::startClockSignal(this);

		// Open communications channel
		CommunicationManager::setReady(this, true);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::setReady(bool ready)
{
	if(ready)
	{
		if(CommunicationManager::isMaster(this))
		{
			if(this->broadcast)
			{
				_communicationRegisters[__CCSR] &= (~0x02);
			}
			else
			{
				_communicationRegisters[__CCSR] |= 0x02;
			}
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::sendPayload(uint8 payload, bool async)
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingPayload;
		CommunicationManager::startTransmissions(this, payload, async);
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::receivePayload(bool async)
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusWaitingPayload;
		CommunicationManager::startTransmissions(this, __COM_CHECKSUM, async);
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::sendAndReceivePayload(uint8 payload, bool async)
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingAndReceivingPayload;
		CommunicationManager::startTransmissions(this, payload, async);
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::startDataTransmission(uint8* data, int32 numberOfBytes, bool sendingData)
{
	if((sendingData && NULL == data) || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	if(NULL != this->sentData)
	{
		delete this->sentData;
	}

	if(NULL != this->receivedData)
	{
		delete this->receivedData;
	}

	this->sentData = NULL;
	this->receivedData = NULL;
	this->syncSentByte = NULL;
	this->syncReceivedByte = NULL;
	this->asyncSentByte = NULL;
	this->asyncReceivedByte = NULL;

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
			CommunicationManager::sendPayload(this, *this->syncSentByte, false);
		}
		else
		{
			CommunicationManager::receivePayload(this, false);
		}

		while(kCommunicationsStatusIdle != this->status);
	}

	this->status = kCommunicationsStatusIdle;
	this->syncSentByte = this->syncReceivedByte = NULL;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::sendDataAsync(uint8* data, int32 numberOfBytes, ListenerObject scope)
{
	if(NULL == data || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	if(!isDeleted(scope))
	{
		CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
		CommunicationManager::addEventListener(this, scope, kEventCommunicationsTransmissionCompleted);
	}

	if(this->numberOfBytesPreviouslySent < numberOfBytes)
	{
		if(!isDeleted(this->sentData))
		{
			delete this->sentData;
			this->sentData = NULL;
		}
	}

	if(isDeleted(this->sentData))
	{
		// Allocate memory to hold both the message and the data
		this->sentData = (uint8*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);
	}

	if(!isDeleted(this->receivedData))
	{
		delete this->receivedData;
		this->receivedData = NULL;
	}

	this->syncSentByte = this->syncReceivedByte = NULL;
	this->asyncSentByte = this->sentData;
	this->asyncReceivedByte = this->receivedData;
	this->numberOfBytesPreviouslySent = numberOfBytes;
	this->numberOfBytesPendingTransmission = numberOfBytes;

	// Copy the data
	Mem::copyBYTE((uint8*)this->sentData, data, numberOfBytes);

	CommunicationManager::sendPayload(this, *this->asyncSentByte, true);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::sendData(uint8* data, int32 numberOfBytes)
{
	return CommunicationManager::startDataTransmission(this, data, numberOfBytes, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::receiveData(uint8* data, int32 numberOfBytes)
{
	return CommunicationManager::startDataTransmission(this, data, numberOfBytes, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::startBidirectionalDataTransmission(uint32 message, uint8* data, int32 numberOfBytes)
{
	if((NULL == data) || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	if(this->numberOfBytesPreviouslySent < numberOfBytes)
	{
		if(!isDeleted(this->sentData))
		{
			delete this->sentData;
			this->sentData = NULL;
		}

		if(!isDeleted(this->receivedData))
		{
			delete this->receivedData;
			this->receivedData = NULL;
		}
	}

	if(isDeleted(this->sentData))
	{
		this->sentData = 
			(uint8*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	if(isDeleted(this->receivedData))
	{
		this->receivedData = 
			(uint8*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	this->asyncSentByte = this->asyncReceivedByte = NULL;
	this->syncSentByte = this->sentData;
	this->syncReceivedByte = this->receivedData;
	this->numberOfBytesPreviouslySent = numberOfBytes;	
	this->numberOfBytesPendingTransmission = numberOfBytes + __MESSAGE_SIZE;

	// Save the message
	*(uint32*)this->sentData = message;

	// Copy the data
	Mem::copyBYTE((uint8*)this->sentData + __MESSAGE_SIZE, data, numberOfBytes);

	while(0 < this->numberOfBytesPendingTransmission)
	{
		CommunicationManager::sendAndReceivePayload(this, *this->syncSentByte, false);

		while(kCommunicationsStatusIdle != this->status)
		{
			HardwareManager::halt();
		}
	}

	this->status = kCommunicationsStatusIdle;
	this->syncSentByte = this->syncReceivedByte = NULL;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::startBidirectionalDataTransmissionAsync
(
	uint32 message, uint8* data, int32 numberOfBytes, ListenerObject scope
)
{
	if(NULL == data || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	if(!isDeleted(scope))
	{
		CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
		CommunicationManager::addEventListener(this, scope, kEventCommunicationsTransmissionCompleted);
	}

	if(this->numberOfBytesPreviouslySent < numberOfBytes)
	{
		if(!isDeleted(this->sentData))
		{
			delete this->sentData;
			this->sentData = NULL;
		}

		if(!isDeleted(this->receivedData))
		{
			delete this->receivedData;
			this->receivedData = NULL;
		}
	}

	if(isDeleted(this->sentData))
	{
		// Allocate memory to hold both the message and the data
		this->sentData = 
			(uint8*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	if(isDeleted(this->receivedData))
	{
		this->receivedData = 
			(uint8*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	this->syncSentByte = this->syncReceivedByte = NULL;
	this->asyncSentByte = this->sentData;
	this->asyncReceivedByte = this->receivedData;
	this->numberOfBytesPreviouslySent = numberOfBytes;	
	this->numberOfBytesPendingTransmission = numberOfBytes + __MESSAGE_SIZE;

	// Save the message
	*(uint32*)this->sentData = message;

	// Copy the data
	Mem::copyBYTE((uint8*)this->sentData + __MESSAGE_SIZE, data, numberOfBytes);
	CommunicationManager::sendAndReceivePayload(this, *this->asyncSentByte, true);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::processInterrupt()
{
	int32 status = this->status;
	this->status = kCommunicationsStatusIdle;

	switch(status)
	{
		case kCommunicationsStatusSendingHandshake:
		{
			if(__COM_HANDSHAKE != _communicationRegisters[__CDRR])
			{
				CommunicationManager::sendHandshake(this);
				break;
			}

			this->connected = true;
			CommunicationManager::fireEvent(this, kEventCommunicationsConnected);
			NM_ASSERT(!isDeleted(this), "CommunicationManager::processInterrupt: deleted this during kEventCommunicationsConnected");
			CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsConnected);
		}

		default:
		{
			this->connected = !this->broadcast;
			break;
		}
	}

	switch(status)
	{
		case kCommunicationsStatusWaitingPayload:
		{
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
					CommunicationManager::receivePayload(this, false);
				}
				else
				{
					CommunicationManager::fireEvent(this, kEventCommunicationsTransmissionCompleted);

					NM_ASSERT
					(
						!isDeleted(this), 
						"CommunicationManager::processInterrupt: deleted this during kEventCommunicationsTransmissionCompleted"
					);

					CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
				}
			}

			break;
		}

		case kCommunicationsStatusSendingPayload:
		{
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
					CommunicationManager::sendPayload(this, *this->asyncSentByte, true);
				}
				else
				{
					CommunicationManager::fireEvent(this, kEventCommunicationsTransmissionCompleted);
					
					NM_ASSERT
					(
						!isDeleted(this), 
						"CommunicationManager::processInterrupt: deleted this during kEventCommunicationsTransmissionCompleted"
					);

					CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
					this->asyncSentByte = NULL;
					this->broadcast = kCommunicationsBroadcastNone;
				}
			}

			break;
		}

		case kCommunicationsStatusSendingAndReceivingPayload:
		{
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
					CommunicationManager::sendAndReceivePayload(this, *this->asyncSentByte, false);
				}
				else
				{
					CommunicationManager::fireEvent(this, kEventCommunicationsTransmissionCompleted);
					
					NM_ASSERT
					(
						!isDeleted(this), 
						"CommunicationManager::processInterrupt: deleted this during kEventCommunicationsTransmissionCompleted"
					);

					CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
					this->asyncSentByte = NULL;
					this->asyncReceivedByte = NULL;
				}
			}

			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
