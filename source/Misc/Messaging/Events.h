/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef EVENTS_H_
#define EVENTS_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum Events
{
	// Do not remove me
	kEventEngineFirst = 0,

	// Add events here

	// Game
	kEventGamePaused,
	kEventGameUnpaused,
	kEventGameChangedState,

	// Framerate
	kEventFramerateReady,
	kEventFramerateDipped,

	// Time
	kEventSecondChanged,
	kEventMinuteChanged,
	kEventNextSecondStarted,

	// DisplayUnit
	kEventDisplayUnitTimeError,
	kEventDisplayUnitScanError,
	kEventDisplayUnitFrameStart,
	kEventDisplayUnitGameStart,
	kEventDisplayUnitGameStartDuringVBlank,
	kEventDisplayUnitVBlank,
	kEventDisplayUnitVBlankDuringGameStart,

	// Timer
	kEventTimerInterrupt,
	
	// Keypad
	kEventKeypadRaisedPowerFlag,

	// Communications
	kEventCommunicationsConnected,
	kEventCommunicationsTransmissionCompleted,

	// State machine
	kEventStateMachineWillCleanStack,
	kEventStateMachineCleanedStack,
	kEventStateMachineWillSwapState,
	kEventStateMachineSwapedState,
	kEventStateMachineWillPushState,
	kEventStateMachinePushedState,
	kEventStateMachineWillPopState,
	kEventStateMachinePoppedState,

	// Stage
	kEventLowStreamingRate,

	// Actors
	kEventActorDeleted,
	kEventActorCreated,

	// Components
	kEventComponentDestroyed,

	// Colliders
	kEventColliderDeleted,
	kEventColliderChanged,

	// Sounds
	kEventSoundFinished,
	kEventSoundReleased,

	// Animations
	kEventAnimationStarted,
	kEventAnimationCompleted,

	// Textures
	kEventTextureRewritten,
	kEventTextureSetFrame,

	// Tiles
	kEventTileSetChangedFrame,
	kEventTileSetChangedOffset,
	kEventTileSetDeleted,

	// Fonts
	kEventFontRewritten,

	// Fade effects
	kEventEffectFadeInComplete,
	kEventEffectFadeOutComplete,
	kEventEffectFadeStart,
	kEventEffectFadeStop,

	// Do not remove me
	kEventPluginsFirst = 1000,
	kEventGameFirst = 10000,
	kEventEngineLast
};

#endif
