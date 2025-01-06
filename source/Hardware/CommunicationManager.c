/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with communicationManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __DEBUG_TOOL
#include <Debug.h>
#endif
#include <DebugConfig.h>
#include <Mem.h>
#include <MessageDispatcher.h>
#include <Printing.h>
#include <Profiler.h>
#include <Telegram.h>
#include <TimerManager.h>
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
	kCommunicationsStatusNone = 0,
	kCommunicationsStatusIdle,
	kCommunicationsStatusSendingHandshake,
	kCommunicationsStatusSendingPayload,
	kCommunicationsStatusWaitingPayload,
	kCommunicationsStatusSendingAndReceivingPayload,
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static volatile BYTE* _communicationRegisters =			(uint8*)0x02000000;

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

#define	__MESSAGE_SIZE					sizeof(WORD)

#define __REMOTE_READY_MESSAGE			0x43873AD1
#define __MASTER_FRMCYC_SET_MESSAGE		0x5DC289F4

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::interruptHandler()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeStartInterrupt, NULL);
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

static void CommunicationManager::reset()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	switch(communicationManager->status)
	{
		case kCommunicationsStatusNone:

			communicationManager->connected = false;
			communicationManager->communicationMode = __COM_AS_REMOTE;
			communicationManager->status = kCommunicationsStatusIdle;
			communicationManager->broadcast = kCommunicationsBroadcastNone;
			communicationManager->sentData = NULL;
			communicationManager->syncSentByte = NULL;
			communicationManager->asyncSentByte = NULL;
			communicationManager->receivedData = NULL;
			communicationManager->syncReceivedByte = NULL;
			communicationManager->asyncReceivedByte = NULL;
			communicationManager->numberOfBytesPreviouslySent = 0;

			CommunicationManager::endCommunications(communicationManager);
			break;

		case kCommunicationsStatusSendingHandshake:

			break;

		default:

			CommunicationManager::cancelCommunications(communicationManager);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::enableCommunications(EventListener eventLister, ListenerObject scope)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(communicationManager->connected || kCommunicationsStatusIdle != communicationManager->status)
	{
		return;
	}

	CommunicationManager::endCommunications(communicationManager);

	if(eventLister && !isDeleted(scope))
	{
		CommunicationManager::addEventListener(communicationManager, scope, eventLister, kEventCommunicationsConnected);
	}

#ifdef __RELEASE
	uint32 wait = 2000;
#else
	uint32 wait = 500;
#endif

	// Wait a little bit for channel to stabilize
	VUEngine::wait(VUEngine::getInstance(), wait);

	// If handshake is taking place
    if(CommunicationManager::isHandshakeIncoming(communicationManager))
    {
		// There is another system attached already managing
		// The channel
		communicationManager->communicationMode = __COM_AS_MASTER;

		// Send dummy payload to verify communication
		CommunicationManager::sendHandshake(communicationManager);
	}
	else
	{
		// I'm alone since a previously turned on
		// System would had already closed the channel
		communicationManager->communicationMode = __COM_AS_REMOTE;

		// Wait for incoming clock
		CommunicationManager::sendHandshake(communicationManager);
    }
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::disableCommunications()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	CommunicationManager::cancelCommunications(communicationManager);

	communicationManager->status = kCommunicationsStatusNone;

	CommunicationManager::reset(communicationManager);	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::cancelCommunications()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	CommunicationManager::endCommunications(communicationManager);

	if(NULL != communicationManager->sentData)
	{
		delete communicationManager->sentData;
	}

	if(NULL != communicationManager->receivedData)
	{
		delete communicationManager->receivedData;
	}

	communicationManager->sentData = NULL;
	communicationManager->syncSentByte = NULL;
	communicationManager->asyncSentByte = NULL;
	communicationManager->receivedData = NULL;
	communicationManager->syncReceivedByte = NULL;
	communicationManager->asyncReceivedByte = NULL;
	communicationManager->status = kCommunicationsStatusIdle;
	communicationManager->broadcast = kCommunicationsBroadcastNone;

	CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsConnected);
	CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsTransmissionCompleted);

	MessageDispatcher::discardAllDelayedMessagesForReceiver(ListenerObject::safeCast(communicationManager));

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::startSyncCycle()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(!communicationManager->connected)
	{
		return;
	}

	extern volatile uint16* _vipRegisters;

	_vipRegisters[__FRMCYC] = 0;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE);

	CommunicationManager::cancelCommunications(communicationManager);

	if(CommunicationManager::isMaster(communicationManager))
	{
		uint32 message = 0;
		do
		{
			CommunicationManager::receiveData((BYTE*)&message, sizeof(message));
		}
		while(__REMOTE_READY_MESSAGE != message);

		message = __MASTER_FRMCYC_SET_MESSAGE;
		while(!(_vipRegisters[__DPSTTS] & __FCLK));
		CommunicationManager::sendData((BYTE*)&message, sizeof(message));
	}
	else
	{
		uint32 message = __REMOTE_READY_MESSAGE;
		CommunicationManager::sendData((BYTE*)&message, sizeof(message));

		do
		{
			CommunicationManager::receiveData((BYTE*)&message, sizeof(message));
		}
		while(__MASTER_FRMCYC_SET_MESSAGE != message);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isConnected()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return communicationManager->connected;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isMaster()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return __COM_AS_MASTER == communicationManager->communicationMode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::broadcastData(BYTE* data, int32 numberOfBytes)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(CommunicationManager::isConnected(communicationManager))
	{
		return false;
	}

	switch (communicationManager->status)
	{
		case kCommunicationsStatusSendingHandshake:

			CommunicationManager::cancelCommunications(communicationManager);
			break;
	}

	if(kCommunicationsStatusIdle != communicationManager->status)
	{
		return false;
	}

	// Always start comms as master when broadcasting
	communicationManager->communicationMode = __COM_AS_MASTER;

	communicationManager->broadcast = kCommunicationsBroadcastSync;

	while(0 < numberOfBytes)
	{
		CommunicationManager::sendPayload(*data, false);

		while(CommunicationManager::isTransmitting(communicationManager));
		communicationManager->status = kCommunicationsStatusIdle;

		data++;

		numberOfBytes--;
	}

    
	communicationManager->broadcast = kCommunicationsBroadcastNone;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::broadcastDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(CommunicationManager::isConnected(communicationManager))
	{
		return;
	}

	switch (communicationManager->status)
	{
		case kCommunicationsStatusSendingHandshake:

			CommunicationManager::cancelCommunications(communicationManager);
			break;
	}

	if(kCommunicationsStatusIdle != communicationManager->status)
	{
		return;
	}

	// Always start comms as master when broadcasting
	communicationManager->communicationMode = __COM_AS_MASTER;

	communicationManager->broadcast = kCommunicationsBroadcastAsync;
	CommunicationManager::sendDataAsync(data, numberOfBytes, eventLister, scope);

	return;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::sendAndReceiveData(WORD message, BYTE* data, int32 numberOfBytes)
{
	return CommunicationManager::startBidirectionalDataTransmission(message, data, numberOfBytes);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::sendAndReceiveDataAsync
(
	WORD message, BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope
)
{
	return CommunicationManager::startBidirectionalDataTransmissionAsync(message, data, numberOfBytes, eventLister, scope);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static WORD CommunicationManager::getSentMessage()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return *(WORD*)communicationManager->sentData;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static WORD CommunicationManager::getReceivedMessage()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return *(WORD*)communicationManager->receivedData;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const BYTE* CommunicationManager::getSentData()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return (const BYTE*)communicationManager->sentData + __MESSAGE_SIZE;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const BYTE* CommunicationManager::getReceivedData()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return (const BYTE*)communicationManager->receivedData + __MESSAGE_SIZE;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void CommunicationManager::print(int32 x, int32 y)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	PRINT_TEXT(CommunicationManager::isConnected(communicationManager) ? "Connected   " : "Disconnected", x, y);

	char* helper = "";
	int32 valueDisplacement = 20;

	PRINT_TIME(x + valueDisplacement, y++);

	switch(communicationManager->status)
	{
		case kCommunicationsStatusIdle:
			helper = "Idle             ";
			break;
		case kCommunicationsStatusSendingHandshake:
			helper = "Sending handshake         ";
			break;
		case kCommunicationsStatusWaitingPayload:
			helper = "Waiting payload            ";
			break;
		case kCommunicationsStatusSendingPayload:
			helper = "Sending payload            ";
			break;
		case kCommunicationsStatusSendingAndReceivingPayload:
			helper = "Send / Recv payload        ";
			break;

		default:
			helper = "Error            ";
			break;
	}

	PRINT_TEXT("Status:", x, y);
	PRINT_INT(communicationManager->status, x + valueDisplacement * 2, y);
	PRINT_TEXT(helper, x + valueDisplacement, y++);

	PRINT_TEXT("Mode: ", x, y);
	PRINT_TEXT(__COM_AS_REMOTE == communicationManager->communicationMode ? "Remote" : "Master ", x + valueDisplacement, y++);

	PRINT_TEXT("Channel: ", x, y);
	PRINT_TEXT(CommunicationManager::isAuxChannelOpen(communicationManager) ? "Aux Open    " : "Aux Closed ", x + valueDisplacement, y++);
	PRINT_TEXT(CommunicationManager::isRemoteReady(communicationManager) ? "Comms Open    " : "Comms Closed ", x + valueDisplacement, y++);

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
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CommunicationManager::handleMessage(Telegram telegram)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	switch(Telegram::getMessage(telegram))
	{
		case kMessageCheckIfRemoteIsReady:

			if(CommunicationManager::isRemoteReady(communicationManager))
			{
				CommunicationManager::startClockSignal(communicationManager);
			}
			else
			{
				CommunicationManager::waitForRemote(communicationManager);
			}

			return true;
			break;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isTransmitting()
{
	return _communicationRegisters[__CCR] & __COM_PENDING ? true : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::managesChannel()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return !CommunicationManager::isMaster(communicationManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isHandshakeIncoming()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	// Try to close the communication channel
	CommunicationManager::setReady(false);
	// If channel was unsuccessfully closed,
	// Then there is a handshake taking place
	return CommunicationManager::isRemoteReady(communicationManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isRemoteReady()
{
	return 0 == (_communicationRegisters[__CCSR] & 0x01) ? true : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isCommunicationControlInterrupt()
{
	return 0 == (_communicationRegisters[__CCSR] & __COM_DISABLE_INTERRUPT);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isAuxChannelOpen()
{
	return _communicationRegisters[__CCSR] & 0x01 ? true : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::isFreeForTransmissions()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	return (
		(communicationManager->connected || communicationManager->broadcast) &&
		NULL == communicationManager->syncSentByte &&
		NULL == communicationManager->syncReceivedByte &&
		NULL == communicationManager->asyncSentByte &&
		NULL == communicationManager->asyncReceivedByte &&
		kCommunicationsStatusIdle == communicationManager->status
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::endCommunications()
{
	CommunicationManager::setReady(false);

	_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT;
	_communicationRegisters[__CDTR] = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::waitForRemote()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	MessageDispatcher::dispatchMessage
	(
		1, ListenerObject::safeCast(communicationManager), ListenerObject::safeCast(communicationManager), kMessageCheckIfRemoteIsReady, NULL
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::sendHandshake()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(kCommunicationsStatusIdle == communicationManager->status)
	{
		communicationManager->status = kCommunicationsStatusSendingHandshake;
		CommunicationManager::startTransmissions(__COM_HANDSHAKE, false);
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::startClockSignal()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	// Make sure to disable COMCNT interrupts
	// _communicationRegisters[__CCSR] |= __COM_DISABLE_INTERRUPT;

	switch(communicationManager->broadcast)
	{
		case kCommunicationsBroadcastSync:

			_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT | communicationManager->communicationMode | __COM_START;

			communicationManager->numberOfBytesPendingTransmission = 0;
			communicationManager->status = kCommunicationsStatusIdle;
			break;

		case kCommunicationsBroadcastAsync:

			_communicationRegisters[__CCR] = communicationManager->communicationMode | __COM_START;
			break;

		default:

			// Start communications
			_communicationRegisters[__CCR] = communicationManager->communicationMode | __COM_START;

			break;

	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::startTransmissions(uint8 payload, bool async)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	// Set transmission data
	_communicationRegisters[__CDTR] = payload;

	HardwareManager::enableInterrupts();

	// Master must wait for slave to open the channel
	if(CommunicationManager::isMaster(communicationManager))
	{
		CommunicationManager::setReady(true);

		if(kCommunicationsBroadcastNone != communicationManager->broadcast)
		{
			CommunicationManager::startClockSignal(communicationManager);
			return;
		}

		if(async)
		{
			if(CommunicationManager::isRemoteReady(communicationManager))
			{
				CommunicationManager::startClockSignal(communicationManager);
			}
			else
			{
				CommunicationManager::waitForRemote(communicationManager);
			}
		}
		else
		{
			while(!CommunicationManager::isRemoteReady(communicationManager));

			CommunicationManager::startClockSignal(communicationManager);
		}
	}
	else
	{
		// Set Start flag
		CommunicationManager::startClockSignal(communicationManager);

		// Open communications channel
		CommunicationManager::setReady(true);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::setReady(bool ready)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(ready)
	{
		if(CommunicationManager::isMaster(communicationManager))
		{
			if(communicationManager->broadcast)
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

static bool CommunicationManager::sendPayload(uint8 payload, bool async)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(kCommunicationsStatusIdle == communicationManager->status)
	{
		communicationManager->status = kCommunicationsStatusSendingPayload;
		CommunicationManager::startTransmissions(payload, async);
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::receivePayload(bool async)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(kCommunicationsStatusIdle == communicationManager->status)
	{
		communicationManager->status = kCommunicationsStatusWaitingPayload;
		CommunicationManager::startTransmissions(__COM_CHECKSUM, async);
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::sendAndReceivePayload(uint8 payload, bool async)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(kCommunicationsStatusIdle == communicationManager->status)
	{
		communicationManager->status = kCommunicationsStatusSendingAndReceivingPayload;
		CommunicationManager::startTransmissions(payload, async);
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::startDataTransmission(BYTE* data, int32 numberOfBytes, bool sendingData)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if((sendingData && NULL == data) || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(communicationManager))
	{
		return false;
	}

	if(NULL != communicationManager->sentData)
	{
		delete communicationManager->sentData;
	}

	if(NULL != communicationManager->receivedData)
	{
		delete communicationManager->receivedData;
	}

	communicationManager->sentData = NULL;
	communicationManager->receivedData = NULL;
	communicationManager->syncSentByte = NULL;
	communicationManager->syncReceivedByte = NULL;
	communicationManager->asyncSentByte = NULL;
	communicationManager->asyncReceivedByte = NULL;

	if(sendingData)
	{
		communicationManager->syncSentByte = data;
	}
	else
	{
		communicationManager->syncReceivedByte = data;
	}

	communicationManager->numberOfBytesPendingTransmission = numberOfBytes;

	while(0 < communicationManager->numberOfBytesPendingTransmission)
	{
		if(sendingData)
		{
			CommunicationManager::sendPayload(*communicationManager->syncSentByte, false);
		}
		else
		{
			CommunicationManager::receivePayload(false);
		}

		while(kCommunicationsStatusIdle != communicationManager->status);
	}

	communicationManager->status = kCommunicationsStatusIdle;
	communicationManager->syncSentByte = communicationManager->syncReceivedByte = NULL;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::sendDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(NULL == data || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(communicationManager))
	{
		return false;
	}

	if(eventLister && !isDeleted(scope))
	{
		CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsTransmissionCompleted);
		CommunicationManager::addEventListener(communicationManager, scope, eventLister, kEventCommunicationsTransmissionCompleted);
	}

	if(communicationManager->numberOfBytesPreviouslySent < numberOfBytes)
	{
		if(!isDeleted(communicationManager->sentData))
		{
			delete communicationManager->sentData;
			communicationManager->sentData = NULL;
		}
	}

	if(isDeleted(communicationManager->sentData))
	{
		// Allocate memory to hold both the message and the data
		communicationManager->sentData = (BYTE*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);
	}

	if(!isDeleted(communicationManager->receivedData))
	{
		delete communicationManager->receivedData;
		communicationManager->receivedData = NULL;
	}

	communicationManager->syncSentByte = communicationManager->syncReceivedByte = NULL;
	communicationManager->asyncSentByte = communicationManager->sentData;
	communicationManager->asyncReceivedByte = communicationManager->receivedData;
	communicationManager->numberOfBytesPreviouslySent = numberOfBytes;
	communicationManager->numberOfBytesPendingTransmission = numberOfBytes;

	// Copy the data
	Mem::copyBYTE((BYTE*)communicationManager->sentData, data, numberOfBytes);

	CommunicationManager::sendPayload(*communicationManager->asyncSentByte, true);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::sendData(BYTE* data, int32 numberOfBytes)
{
	return CommunicationManager::startDataTransmission(data, numberOfBytes, true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::receiveData(BYTE* data, int32 numberOfBytes)
{
	return CommunicationManager::startDataTransmission(data, numberOfBytes, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::startBidirectionalDataTransmission(WORD message, BYTE* data, int32 numberOfBytes)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if((NULL == data) || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(communicationManager))
	{
		return false;
	}

	if(communicationManager->numberOfBytesPreviouslySent < numberOfBytes)
	{
		if(!isDeleted(communicationManager->sentData))
		{
			delete communicationManager->sentData;
			communicationManager->sentData = NULL;
		}

		if(!isDeleted(communicationManager->receivedData))
		{
			delete communicationManager->receivedData;
			communicationManager->receivedData = NULL;
		}
	}

	if(isDeleted(communicationManager->sentData))
	{
		communicationManager->sentData = 
			(BYTE*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	if(isDeleted(communicationManager->receivedData))
	{
		communicationManager->receivedData = 
			(BYTE*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	communicationManager->asyncSentByte = communicationManager->asyncReceivedByte = NULL;
	communicationManager->syncSentByte = communicationManager->sentData;
	communicationManager->syncReceivedByte = communicationManager->receivedData;
	communicationManager->numberOfBytesPreviouslySent = numberOfBytes;	
	communicationManager->numberOfBytesPendingTransmission = numberOfBytes + __MESSAGE_SIZE;

	// Save the message
	*(WORD*)communicationManager->sentData = message;

	// Copy the data
	Mem::copyBYTE((BYTE*)communicationManager->sentData + __MESSAGE_SIZE, data, numberOfBytes);

	while(0 < communicationManager->numberOfBytesPendingTransmission)
	{
		CommunicationManager::sendAndReceivePayload(*communicationManager->syncSentByte, false);

		while(kCommunicationsStatusIdle != communicationManager->status)
		{
			HardwareManager::halt();
		}
	}

	communicationManager->status = kCommunicationsStatusIdle;
	communicationManager->syncSentByte = communicationManager->syncReceivedByte = NULL;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CommunicationManager::startBidirectionalDataTransmissionAsync
(
	WORD message, BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope
)
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	if(NULL == data || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(communicationManager))
	{
		return false;
	}

	if(NULL != eventLister && !isDeleted(scope))
	{
		CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsTransmissionCompleted);
		CommunicationManager::addEventListener(communicationManager, scope, eventLister, kEventCommunicationsTransmissionCompleted);
	}

	if(communicationManager->numberOfBytesPreviouslySent < numberOfBytes)
	{
		if(!isDeleted(communicationManager->sentData))
		{
			delete communicationManager->sentData;
			communicationManager->sentData = NULL;
		}

		if(!isDeleted(communicationManager->receivedData))
		{
			delete communicationManager->receivedData;
			communicationManager->receivedData = NULL;
		}
	}

	if(isDeleted(communicationManager->sentData))
	{
		// Allocate memory to hold both the message and the data
		communicationManager->sentData = 
			(BYTE*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	if(isDeleted(communicationManager->receivedData))
	{
		communicationManager->receivedData = 
			(BYTE*)((uint32)MemoryPool::allocate(numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	communicationManager->syncSentByte = communicationManager->syncReceivedByte = NULL;
	communicationManager->asyncSentByte = communicationManager->sentData;
	communicationManager->asyncReceivedByte = communicationManager->receivedData;
	communicationManager->numberOfBytesPreviouslySent = numberOfBytes;	
	communicationManager->numberOfBytesPendingTransmission = numberOfBytes + __MESSAGE_SIZE;

	// Save the message
	*(WORD*)communicationManager->sentData = message;

	// Copy the data
	Mem::copyBYTE((BYTE*)communicationManager->sentData + __MESSAGE_SIZE, data, numberOfBytes);
	CommunicationManager::sendAndReceivePayload(*communicationManager->asyncSentByte, true);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CommunicationManager::processInterrupt()
{
	CommunicationManager communicationManager = CommunicationManager::getInstance();

	int32 status = communicationManager->status;
	communicationManager->status = kCommunicationsStatusIdle;

	switch(status)
	{
		case kCommunicationsStatusSendingHandshake:

			if(__COM_HANDSHAKE != _communicationRegisters[__CDRR])
			{
				CommunicationManager::sendHandshake(communicationManager);
				break;
			}

			communicationManager->connected = true;
			CommunicationManager::fireEvent(communicationManager, kEventCommunicationsConnected);
			NM_ASSERT(!isDeleted(communicationManager), "CommunicationManager::processInterrupt: deleted communicationManager during kEventCommunicationsConnected");
			CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsConnected);

		default:

			communicationManager->connected = !communicationManager->broadcast;
			break;
	}

	switch(status)
	{
		case kCommunicationsStatusWaitingPayload:

			if(communicationManager->syncReceivedByte)
			{
				*communicationManager->syncReceivedByte = _communicationRegisters[__CDRR];
				communicationManager->syncReceivedByte++;
				--communicationManager->numberOfBytesPendingTransmission;
			}
			else if(communicationManager->asyncReceivedByte)
			{
				*communicationManager->asyncReceivedByte = _communicationRegisters[__CDRR];
				communicationManager->asyncReceivedByte++;

				if(0 < --communicationManager->numberOfBytesPendingTransmission)
				{
					CommunicationManager::receivePayload(false);
				}
				else
				{
					CommunicationManager::fireEvent(communicationManager, kEventCommunicationsTransmissionCompleted);

					NM_ASSERT
					(
						!isDeleted(communicationManager), 
						"CommunicationManager::processInterrupt: deleted communicationManager during kEventCommunicationsTransmissionCompleted"
					);

					CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsTransmissionCompleted);
					communicationManager->asyncReceivedByte = NULL;
#ifdef __ENABLE_PROFILER
					Profiler::lap(Profiler::getInstance(), kProfilerLapTypeCommunicationsInterruptProcess, PROCESS_NAME_COMMUNICATIONS);
#endif
				}
			}

			break;

		case kCommunicationsStatusSendingPayload:

			if(communicationManager->syncSentByte)
			{
				communicationManager->syncSentByte++;
				--communicationManager->numberOfBytesPendingTransmission;
			}
			else if(communicationManager->asyncSentByte)
			{
				communicationManager->asyncSentByte++;

				if(0 < --communicationManager->numberOfBytesPendingTransmission)
				{
					CommunicationManager::sendPayload(*communicationManager->asyncSentByte, true);
				}
				else
				{
					CommunicationManager::fireEvent(communicationManager, kEventCommunicationsTransmissionCompleted);
					
					NM_ASSERT
					(
						!isDeleted(communicationManager), 
						"CommunicationManager::processInterrupt: deleted communicationManager during kEventCommunicationsTransmissionCompleted"
					);

					CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsTransmissionCompleted);
					communicationManager->asyncSentByte = NULL;
					communicationManager->broadcast = kCommunicationsBroadcastNone;
#ifdef __ENABLE_PROFILER
					Profiler::lap(Profiler::getInstance(), kProfilerLapTypeCommunicationsInterruptProcess, PROCESS_NAME_COMMUNICATIONS);
#endif
				}
			}

			break;

		case kCommunicationsStatusSendingAndReceivingPayload:

			if(communicationManager->syncSentByte && communicationManager->syncReceivedByte)
			{
				*communicationManager->syncReceivedByte = _communicationRegisters[__CDRR];
				communicationManager->syncReceivedByte++;
				communicationManager->syncSentByte++;
				--communicationManager->numberOfBytesPendingTransmission;
			}
			else if(communicationManager->asyncSentByte && communicationManager->asyncReceivedByte)
			{
				*communicationManager->asyncReceivedByte = _communicationRegisters[__CDRR];
				communicationManager->asyncReceivedByte++;
				communicationManager->asyncSentByte++;

				if(0 < --communicationManager->numberOfBytesPendingTransmission)
				{
					CommunicationManager::sendAndReceivePayload(*communicationManager->asyncSentByte, false);
				}
				else
				{
					CommunicationManager::fireEvent(communicationManager, kEventCommunicationsTransmissionCompleted);
					
					NM_ASSERT
					(
						!isDeleted(communicationManager), 
						"CommunicationManager::processInterrupt: deleted communicationManager during kEventCommunicationsTransmissionCompleted"
					);

					CommunicationManager::removeEventListeners(communicationManager, NULL, kEventCommunicationsTransmissionCompleted);
					communicationManager->asyncSentByte = NULL;
					communicationManager->asyncReceivedByte = NULL;

#ifdef __ENABLE_PROFILER
					Profiler::lap(Profiler::getInstance(), kProfilerLapTypeCommunicationsInterruptProcess, PROCESS_NAME_COMMUNICATIONS);
#endif
				}
			}

			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->status = 	kCommunicationsStatusNone;

	CommunicationManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CommunicationManager::destructor()
{
	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
