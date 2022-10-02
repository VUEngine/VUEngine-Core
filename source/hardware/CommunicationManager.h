/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COMMUNICATTION_MANAGER_H_
#define COMMUNICATTION_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Telegram.h>
#include <VirtualList.h>



//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

enum CommunicationsBroadcastStates
{
	kCommunicationsBroadcastNone = 0,
	kCommunicationsBroadcastSync,
	kCommunicationsBroadcastAsync
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class CommunicationManager : ListenerObject
{
	volatile int32 status;
	volatile BYTE* sentData;
	volatile BYTE* receivedData;
	volatile BYTE* syncSentByte;
	volatile BYTE* syncReceivedByte;
	volatile BYTE* asyncSentByte;
	volatile BYTE* asyncReceivedByte;
	volatile int32 numberOfBytesPendingTransmission;
	int32 numberOfBytesPreviouslySent;
	volatile uint32 broadcast;
	volatile bool connected;
	volatile uint8 communicationMode;

	uint32 timeout;

	/// @publicsection
	static CommunicationManager getInstance();
	static void interruptHandler();
	override bool handleMessage(Telegram telegram);
	void reset();
	void enableCommunications(EventListener eventLister, ListenerObject scope);
	bool cancelCommunications();
	void update();
	bool isConnected();
	bool isMaster();
	bool broadcastData(BYTE* data, int32 numberOfBytes);
	void broadcastDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope);
	bool sendData(BYTE* data, int32 numberOfBytes);
	bool receiveData(BYTE* data, int32 numberOfBytes);
	bool sendAndReceiveData(WORD message, BYTE* data, int32 numberOfBytes);
	bool sendDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope);
	bool receiveDataAsync(int32 numberOfBytes, EventListener eventLister, ListenerObject scope);
	bool sendAndReceiveDataAsync(WORD message, BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope);
	WORD getReceivedMessage();
	WORD getSentMessage();
	const BYTE* getReceivedData();
	const BYTE* getSentData();
	void startSyncCycle();
	void printStatus(int32 x, int32 y);
}


#endif
