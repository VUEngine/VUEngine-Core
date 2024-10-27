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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class Telegram;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class CommunicationManager
///
/// Inherits from ListenerObject
///
/// Manages communications on the EXT port.
/// @ingroup hardware
singleton class CommunicationManager : ListenerObject
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
	
	/// Method to retrieve the singleton instance
	/// @return CommunicationManager singleton
	static CommunicationManager getInstance();

	/// Interrupt handler for COM interrupts
	static void interruptHandler();

	/// Reset the manager's state.
	void reset();

	/// Enable communications on the EXT port.
	/// @param eventListener: Callback method for when a connection is stablished
	/// @param scope: Object on which to perform the callback
	void enableCommunications(EventListener eventLister, ListenerObject scope);

	/// Disable communication on the EXT port.
	void disableCommunications();

	/// Cancel all pending transmissions on the EXT port.
	bool cancelCommunications();

	/// Start the sync procedure according to the official documentation once a connection
	/// has been stablished.
	void startSyncCycle();

	/// Check if there is something attached to the EXT port.
	/// @return True if there is something attached to the EXT port; false otherwise
	bool isConnected();

	/// Check if the system is the master during the next cycle of communications over the EXT port.
	/// @return True if the system is the master; false otherwise
	bool isMaster();

	/// Send data synchronously over the EXT port if there is nothing detectable attached to it.
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	bool broadcastData(BYTE* data, int32 numberOfBytes);

	/// Send data asynchronously over the EXT port if there is nothing detectable attached to it.
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	/// @param eventListener: Callback method for when a connection is stablished
	/// @param scope: Object on which to perform the callback
	void broadcastDataAsync(BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope);

	/// Send and receive data synchronously over the EXT port if there is something detectable attached to it.
	/// @param message: Control message for the receiving partner
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	bool sendAndReceiveData(WORD message, BYTE* data, int32 numberOfBytes);

	/// Send and receive data asynchronously over the EXT port if there is something detectable attached to it.
	/// @param message: Control message for the receiving partner
	/// @param data: Data to broadcast
	/// @param numberOfBytes: Number of bytes to broadcast
	/// @param eventListener: Callback method for when a connection is stablished
	/// @param scope: Object on which to perform the callback
	bool sendAndReceiveDataAsync(WORD message, BYTE* data, int32 numberOfBytes, EventListener eventLister, ListenerObject scope);

	/// Retrieve the last sent message on the EXT port.
	WORD getSentMessage();

	/// Retrieve the last received message on the EXT port.
	WORD getReceivedMessage();

	/// Retrieve the last sent data on the EXT port.
	const BYTE* getSentData();

	/// Retrieve the last received data on the EXT port.
	const BYTE* getReceivedData();

	/// Print the manager's status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);
}


#endif
