/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COMMUNICATTION_MANAGER_H_
#define COMMUNICATTION_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Telegram;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class CommunicationManager
///
/// Inherits from ListenerObject
///
/// Manages communications on the EXT port.
singleton! class CommunicationManager : ListenerObject
{
	/// @protectedsection

	/// Status of the communications
	volatile int32 status;

	/// Data sent over the EXT port
	volatile BYTE* sentData;

	/// Data received over the EXT port
	volatile BYTE* receivedData;

	/// Last byte sent synchronously over the EXT port
	volatile BYTE* syncSentByte;

	/// Last byte received synchronously over the EXT port
	volatile BYTE* syncReceivedByte;

	/// Last byte sent asynchronously over the EXT port
	volatile BYTE* asyncSentByte;

	/// Last byte received asynchronously over the EXT port
	volatile BYTE* asyncReceivedByte;

	/// Number of bytes pending transmission over the EXT port
	volatile int32 numberOfBytesPendingTransmission;

	/// Number of bytes already transmitted over the EXT port
	int32 numberOfBytesPreviouslySent;

	/// Status of broadcast communications
	volatile uint32 broadcast;

	/// Flag that indicates if there is something connected to the EXT port
	volatile bool connected;

	/// Keeps track of the role as master or slave that the system holds in data transmissions
	volatile uint8 communicationMode;

	/// @publicsection

	/// Interrupt handler for COM interrupts
	static void interruptHandler();

	/// Register an object that will listen for events.
	/// @param listener: ListenerObject that listen for the event
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to listen for
	static void registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Remove a specific listener object from the listening to a give code with the provided callback.
	/// @param listener: ListenerObject to remove from the list of listeners
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to stop listen for
	static void unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Reset the manager's state.
	static void reset();

	/// Enable communications on the EXT port.
	/// @param eventListener: Callback method for when a connection is stablished
	/// @param scope: Object on which to perform the callback
	static void enableCommunications(EventListener eventListener, ListenerObject scope);

	/// Disable communication on the EXT port.
	static void disableCommunications();

	/// Cancel all pending transmissions on the EXT port.
	static bool cancelCommunications();

	/// Start the sync procedure according to the official documentation once a connection
	/// has been stablished.
	static void startSyncCycle();

	/// Check if there is something attached to the EXT port.
	/// @return True if there is something attached to the EXT port; false otherwise
	static bool isConnected();

	/// Check if the system is the master during the next cycle of communications over the EXT port.
	/// @return True if the system is the master; false otherwise
	static bool isMaster();

	/// Send data synchronously over the EXT port if there is nothing detectable attached to it.
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	static bool broadcastData(BYTE* data, int32 numberOfBytes);

	/// Send data asynchronously over the EXT port if there is nothing detectable attached to it.
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	/// @param eventListener: Callback method for when a connection is stablished
	/// @param scope: Object on which to perform the callback
	static void broadcastDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventListener, ListenerObject scope);

	/// Send and receive data synchronously over the EXT port if there is something detectable attached to it.
	/// @param message: Control message for the receiving partner
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	static bool sendAndReceiveData(WORD message, BYTE* data, int32 numberOfBytes);

	/// Send and receive data asynchronously over the EXT port if there is something detectable attached to it.
	/// @param message: Control message for the receiving partner
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	/// @param eventListener: Callback method for when a connection is stablished
	/// @param scope: Object on which to perform the callback
	static bool sendAndReceiveDataAsync(WORD message, BYTE* data, int32 numberOfBytes, EventListener eventListener, ListenerObject scope);

	/// Retrieve the last sent message on the EXT port.
	static WORD getSentMessage();

	/// Retrieve the last received message on the EXT port.
	static WORD getReceivedMessage();

	/// Retrieve the last sent data on the EXT port.
	static const BYTE* getSentData();

	/// Retrieve the last received data on the EXT port.
	static const BYTE* getReceivedData();

	/// Print the manager's status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);
}

#endif
