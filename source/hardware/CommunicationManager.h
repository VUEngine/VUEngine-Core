/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#ifndef COMMUNICATTION_MANAGER_H_
#define COMMUNICATTION_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
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
singleton class CommunicationManager : Object
{
	volatile int status;
	volatile BYTE* sentData;
	volatile BYTE* receivedData;
	volatile BYTE* syncSentByte;
	volatile BYTE* syncReceivedByte;
	volatile BYTE* asyncSentByte;
	volatile BYTE* asyncReceivedByte;
	volatile int numberOfBytesPendingTransmission;
	int numberOfBytesPreviouslySent;
	volatile u32 broadcast;
	volatile bool connected;
	volatile u8 communicationMode;

	u32 timeout;

	/// @publicsection
	static CommunicationManager getInstance();
	static void interruptHandler();
	override bool handleMessage(Telegram telegram);
	void reset();
	void enableCommunications(EventListener eventLister, Object scope);
	bool cancelCommunications();
	void update();
	bool isConnected();
	bool isMaster();
	bool broadcastData(BYTE* data, int numberOfBytes);
	void broadcastDataAsync(BYTE* data, int numberOfBytes, EventListener eventLister, Object scope);
	bool sendData(BYTE* data, int numberOfBytes);
	bool receiveData(BYTE* data, int numberOfBytes);
	bool sendAndReceiveData(WORD message, BYTE* data, int numberOfBytes);
	bool sendDataAsync(BYTE* data, int numberOfBytes, EventListener eventLister, Object scope);
	bool receiveDataAsync(int numberOfBytes, EventListener eventLister, Object scope);
	bool sendAndReceiveDataAsync(WORD message, BYTE* data, int numberOfBytes, EventListener eventLister, Object scope);
	WORD getReceivedMessage();
	WORD getSentMessage();
	const BYTE* getReceivedData();
	const BYTE* getSentData();
	void startSyncCycle();
	void printStatus(int x, int y);
}


#endif
