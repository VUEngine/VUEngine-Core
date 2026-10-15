/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// messages
enum MessagesTypes
{
	kMessageNone = 0,

	// General purpose messages
	kMessageHighFps,
	kMessageActorRemoved,

	// Keypad massages
	kMessageEnableKeypad,
	kMessageKeyPressed,
	kMessageKeyReleased,
	kMessageKeyHold,

	// Communication messages
	kMessageCheckIfRemoteIsReady,
	kMessageLastCommunicationMessage,

	// Framerate
	kMessageRestoreFPS,

	// Physics messages
	kMessageBodyStopped,
	kMessageBodyBounced,
	kMessageBodyStartedMoving,
	kMessageColliderShow,
	kMessageColliderHide,

	// Actor messages
	kMessageShow,
	kMessageHide,
	kMessageSetTransparency,
	kMessageAnimationCompleted,
	kMessageLoopingAnimationCompleted,

	// Don't place messages below this
	kMessageEngineLast,

	kMessagePlatformFirst 	= 1000,
	kMessagePluginsFirst 	= 2000,
	kMessageUserFirst 		= 3000,
	kMessageGameFirst 		= 4000
};

#endif
