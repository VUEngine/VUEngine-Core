/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CommunicationManager.h>
#include <TimerManager.h>
#include <Mem.h>
#include <Utilities.h>
#include <VIPManager.h>
#include <MessageDispatcher.h>
#include <VUEngine.h>
#include <Profiler.h>
#ifdef __DEBUG_TOOLS
#include <Debug.h>
#endif
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------

//volatile uint16* _communicationRegisters __INITIALIZED_DATA_SECTION_ATTRIBUTE = (uint16*)_hardwareRegisters;
static volatile BYTE* _communicationRegisters =			(uint8*)0x02000000;


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

enum CommunicationsStatus
{
	kCommunicationsStatusNone = 0,
	kCommunicationsStatusIdle,
	kCommunicationsStatusSendingHandshake,
	kCommunicationsStatusSendingPayload,
	kCommunicationsStatusWaitingPayload,
	kCommunicationsStatusSendingAndReceivingPayload,
};


//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

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
#define __COM_KEEP_ALIVE_TIMEOUT		100
#define __COM_CHECKSUM					0x44

#define	__MESSAGE_SIZE					sizeof(WORD)

#define __REMOTE_READY_MESSAGE			0x43873AD1
#define __MASTER_FRMCYC_SET_MESSAGE		0x5DC289F4


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

	this->status = 	kCommunicationsStatusNone;

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
	switch(this->status)
	{
		case kCommunicationsStatusNone:

			this->connected = false;
			this->communicationMode = __COM_AS_REMOTE;
			this->status = kCommunicationsStatusIdle;
			this->broadcast = kCommunicationsBroadcastNone;
			this->sentData = NULL;
			this->syncSentByte = NULL;
			this->asyncSentByte = NULL;
			this->receivedData = NULL;
			this->syncReceivedByte = NULL;
			this->asyncReceivedByte = NULL;
			this->numberOfBytesPreviouslySent = 0;

			CommunicationManager::endCommunications(this);
			break;

		case kCommunicationsStatusSendingHandshake:

			break;

		default:

			CommunicationManager::cancelCommunications(this);
			break;
	}
}

void CommunicationManager::enableCommunications(EventListener eventLister, ListenerObject scope)
{
	if(this->connected || kCommunicationsStatusIdle != this->status)
	{
		return;
	}

	CommunicationManager::endCommunications(this);

	if(eventLister && !isDeleted(scope))
	{
		CommunicationManager::addEventListener(this, scope, eventLister, kEventCommunicationsConnected);
	}

	// Wait a little bit for channel to stabilize
#ifdef __RELEASE
	TimerManager::wait(TimerManager::getInstance(), 2000);
#else
	TimerManager::wait(TimerManager::getInstance(), 500);
#endif

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
	_communicationRegisters[__CDTR] = 0;
}

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

	CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsConnected);
	CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);

	MessageDispatcher::discardAllDelayedMessagesForReceiver(MessageDispatcher::getInstance(), ListenerObject::safeCast(this));

	return true;
}

bool CommunicationManager::sendHandshake()
{
	if(kCommunicationsStatusIdle == this->status)
	{
		this->status = kCommunicationsStatusSendingHandshake;
		CommunicationManager::startTransmissions(this, __COM_HANDSHAKE, false);
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

void CommunicationManager::startClockSignal()
{
	// Make sure to disable COMCNT interrupts
	// _communicationRegisters[__CCSR] |= __COM_DISABLE_INTERRUPT;

	switch(this->broadcast)
	{
		case kCommunicationsBroadcastSync:

			_communicationRegisters[__CCR] = __COM_DISABLE_INTERRUPT | this->communicationMode | __COM_START;

			this->numberOfBytesPendingTransmission = 0;
			this->status = kCommunicationsStatusIdle;
			break;

		case kCommunicationsBroadcastAsync:

			_communicationRegisters[__CCR] = this->communicationMode | __COM_START;
			break;

		default:

			// Start communications
			_communicationRegisters[__CCR] = this->communicationMode | __COM_START;

			break;

	}
}

void CommunicationManager::waitForRemote()
{
	MessageDispatcher::dispatchMessage(1, ListenerObject::safeCast(this), ListenerObject::safeCast(this), kMessageCheckIfRemoteIsReady, NULL);
}

bool CommunicationManager::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageCheckIfRemoteIsReady:

			if(CommunicationManager::isRemoteReady(this))
			{
				CommunicationManager::startClockSignal(this);
			}
			else
			{
				CommunicationManager::waitForRemote(this);
			}

			return true;
			break;
	}

	return false;
}

void CommunicationManager::startTransmissions(uint8 payload, bool async)
{
	// Set transmission data
	_communicationRegisters[__CDTR] = payload;

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

bool CommunicationManager::isAuxChannelOpen()
{
	return _communicationRegisters[__CCSR] & 0x01 ? true : false;
}

bool CommunicationManager::isRemoteReady()
{
	return 0 == (_communicationRegisters[__CCSR] & 0x01) ? true : false;
}

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

bool CommunicationManager::isCommunicationControlInterrupt()
{
	return 0 == (_communicationRegisters[__CCSR] & __COM_DISABLE_INTERRUPT);
}

/**
 * Communication's interrupt handler
 */
static void CommunicationManager::interruptHandler()
{
#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeStartInterrupt, NULL);
#endif

	if(kCommunicationsBroadcastNone == _communicationManager->broadcast && CommunicationManager::isMaster(_communicationManager))
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
	int32 status = this->status;
	this->status = kCommunicationsStatusIdle;

	switch(status)
	{
		case kCommunicationsStatusSendingHandshake:

			if(__COM_HANDSHAKE != _communicationRegisters[__CDRR])
			{
				CommunicationManager::sendHandshake(this);
				break;
			}

			this->connected = true;
			CommunicationManager::fireEvent(this, kEventCommunicationsConnected);
			NM_ASSERT(!isDeleted(this), "CommunicationManager::processInterrupt: deleted this during kEventCommunicationsConnected");
			CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsConnected);

		default:

			this->connected = !this->broadcast;
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
					CommunicationManager::receivePayload(this, false);
				}
				else
				{
					CommunicationManager::fireEvent(this, kEventCommunicationsTransmissionCompleted);
					NM_ASSERT(!isDeleted(this), "CommunicationManager::processInterrupt: deleted this during kEventCommunicationsTransmissionCompleted");
					CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
					this->asyncReceivedByte = NULL;
#ifdef __ENABLE_PROFILER
					Profiler::lap(Profiler::getInstance(), kProfilerLapTypeCommunicationsInterruptProcess, PROCESS_NAME_COMMUNICATIONS);
#endif
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
					CommunicationManager::sendPayload(this, *this->asyncSentByte, true);
				}
				else
				{
					CommunicationManager::fireEvent(this, kEventCommunicationsTransmissionCompleted);
					NM_ASSERT(!isDeleted(this), "CommunicationManager::processInterrupt: deleted this during kEventCommunicationsTransmissionCompleted");
					CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
					this->asyncSentByte = NULL;
					this->broadcast = kCommunicationsBroadcastNone;
#ifdef __ENABLE_PROFILER
					Profiler::lap(Profiler::getInstance(), kProfilerLapTypeCommunicationsInterruptProcess, PROCESS_NAME_COMMUNICATIONS);
#endif
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
					CommunicationManager::sendAndReceivePayload(this, *this->asyncSentByte, false);
				}
				else
				{
					CommunicationManager::fireEvent(this, kEventCommunicationsTransmissionCompleted);
					NM_ASSERT(!isDeleted(this), "CommunicationManager::processInterrupt: deleted this during kEventCommunicationsTransmissionCompleted");
					CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
					this->asyncSentByte = NULL;
					this->asyncReceivedByte = NULL;

#ifdef __ENABLE_PROFILER
					Profiler::lap(Profiler::getInstance(), kProfilerLapTypeCommunicationsInterruptProcess, PROCESS_NAME_COMMUNICATIONS);
#endif
				}
			}

			break;
	}
}

bool CommunicationManager::isFreeForTransmissions()
{
	return (
		(this->connected || this->broadcast) &&
		NULL == this->syncSentByte &&
		NULL == this->syncReceivedByte &&
		NULL == this->asyncSentByte &&
		NULL == this->asyncReceivedByte &&
		kCommunicationsStatusIdle == this->status
	);
}

bool CommunicationManager::startDataTransmission(BYTE* data, int32 numberOfBytes, bool sendingData)
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


bool CommunicationManager::broadcastData(BYTE* data, int32 numberOfBytes)
{
	if(CommunicationManager::isConnected(this))
	{
		return false;
	}

	switch (this->status)
	{
		case kCommunicationsStatusSendingHandshake:

			CommunicationManager::cancelCommunications(this);
			break;
	}

	if(kCommunicationsStatusIdle != this->status)
	{
		return false;
	}

	// Always start comms as master when broadcasting
	this->communicationMode = __COM_AS_MASTER;

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

void CommunicationManager::broadcastDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope)
{
	if(CommunicationManager::isConnected(this))
	{
		return;
	}

	switch (this->status)
	{
		case kCommunicationsStatusSendingHandshake:

			CommunicationManager::cancelCommunications(this);
			break;
	}

	if(kCommunicationsStatusIdle != this->status)
	{
		return;
	}

	// Always start comms as master when broadcasting
	this->communicationMode = __COM_AS_MASTER;

	this->broadcast = kCommunicationsBroadcastAsync;
	CommunicationManager::sendDataAsync(this, data, numberOfBytes, eventLister, scope);

	return;
}

bool CommunicationManager::sendDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope)
{
	if(NULL == data || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	if(eventLister && !isDeleted(scope))
	{
		CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
		CommunicationManager::addEventListener(this, scope, eventLister, kEventCommunicationsTransmissionCompleted);
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
		this->sentData = (BYTE*)((uint32)MemoryPool::allocate(MemoryPool::getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD);
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
	Mem::copyBYTE((BYTE*)this->sentData, data, numberOfBytes);

	CommunicationManager::sendPayload(this, *this->asyncSentByte, true);

	return true;
}

bool CommunicationManager::sendData(BYTE* data, int32 numberOfBytes)
{
	return CommunicationManager::startDataTransmission(this, data, numberOfBytes, true);
}

bool CommunicationManager::receiveData(BYTE* data, int32 numberOfBytes)
{
	return CommunicationManager::startDataTransmission(this, data, numberOfBytes, false);
}

bool CommunicationManager::startBidirectionalDataTransmission(WORD message, BYTE* data, int32 numberOfBytes)
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
		this->sentData = (BYTE*)((uint32)MemoryPool::allocate(MemoryPool::getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	if(isDeleted(this->receivedData))
	{
		this->receivedData = (BYTE*)((uint32)MemoryPool::allocate(MemoryPool::getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	this->asyncSentByte = this->asyncReceivedByte = NULL;
	this->syncSentByte = this->sentData;
	this->syncReceivedByte = this->receivedData;
	this->numberOfBytesPreviouslySent = numberOfBytes;	
	this->numberOfBytesPendingTransmission = numberOfBytes + __MESSAGE_SIZE;

	// Save the message
	*(WORD*)this->sentData = message;

	// Copy the data
	Mem::copyBYTE((BYTE*)this->sentData + __MESSAGE_SIZE, data, numberOfBytes);

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

bool CommunicationManager::startBidirectionalDataTransmissionAsync(WORD message, BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope)
{
	if(NULL == data || 0 >= numberOfBytes || !CommunicationManager::isFreeForTransmissions(this))
	{
		return false;
	}

	if(NULL != eventLister && !isDeleted(scope))
	{
		CommunicationManager::removeEventListeners(this, NULL, kEventCommunicationsTransmissionCompleted);
		CommunicationManager::addEventListener(this, scope, eventLister, kEventCommunicationsTransmissionCompleted);
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
		this->sentData = (BYTE*)((uint32)MemoryPool::allocate(MemoryPool::getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	if(isDeleted(this->receivedData))
	{
		this->receivedData = (BYTE*)((uint32)MemoryPool::allocate(MemoryPool::getInstance(), numberOfBytes + __DYNAMIC_STRUCT_PAD + __MESSAGE_SIZE) + __DYNAMIC_STRUCT_PAD);
	}

	this->syncSentByte = this->syncReceivedByte = NULL;
	this->asyncSentByte = this->sentData;
	this->asyncReceivedByte = this->receivedData;
	this->numberOfBytesPreviouslySent = numberOfBytes;	
	this->numberOfBytesPendingTransmission = numberOfBytes + __MESSAGE_SIZE;

	// Save the message
	*(WORD*)this->sentData = message;

	// Copy the data
	Mem::copyBYTE((BYTE*)this->sentData + __MESSAGE_SIZE, data, numberOfBytes);
	CommunicationManager::sendAndReceivePayload(this, *this->asyncSentByte, true);

	return true;
}

bool CommunicationManager::sendAndReceiveData(WORD message, BYTE* data, int32 numberOfBytes)
{
	return CommunicationManager::startBidirectionalDataTransmission(this, message, data, numberOfBytes);
}

bool CommunicationManager::sendAndReceiveDataAsync(WORD message, BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope)
{
	return CommunicationManager::startBidirectionalDataTransmissionAsync(this, message, data, numberOfBytes, eventLister, scope);
}

WORD CommunicationManager::getReceivedMessage()
{
	return *(WORD*)this->receivedData;
}

WORD CommunicationManager::getSentMessage()
{
	return *(WORD*)this->sentData;
}

const BYTE* CommunicationManager::getReceivedData()
{
	return (const BYTE*)this->receivedData + __MESSAGE_SIZE;
}

const BYTE* CommunicationManager::getSentData()
{
	return (const BYTE*)this->sentData + __MESSAGE_SIZE;
}

void CommunicationManager::startSyncCycle()
{
	if(!this->connected)
	{
		return;
	}

	extern volatile uint16* _vipRegisters;

	_vipRegisters[__FRMCYC] = 0;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE);

	CommunicationManager::cancelCommunications(this);

	if(CommunicationManager::isMaster(this))
	{
		uint32 message = 0;
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
		uint32 message = __REMOTE_READY_MESSAGE;
		CommunicationManager::sendData(this, (BYTE*)&message, sizeof(message));

		do
		{
			CommunicationManager::receiveData(this, (BYTE*)&message, sizeof(message));
		}
		while(__MASTER_FRMCYC_SET_MESSAGE != message);
	}
}

void CommunicationManager::printStatus(int32 x, int32 y)
{
	PRINT_TEXT(CommunicationManager::isConnected(this) ? "Connected   " : "Disconnected", x, y);

	char* helper = "";
	int32 valueDisplacement = 20;

	PRINT_TIME(x + valueDisplacement, y++);
	PRINT_TEXT("Time out:                  ", x, y);
	PRINT_INT(this->timeout, x + valueDisplacement, y++);

	switch(this->status)
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
