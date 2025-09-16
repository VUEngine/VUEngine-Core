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
	kEventFramerateReady,
	kEventFramerateDipped,

	kEventContainerAllChildrenDeleted,
	kEventStageChildStreamedOut,
	kEventColliderDeleted,
	kEventColliderChanged,
	kEventComponentDestroyed,

	kEventGamePaused,
	kEventGameUnpaused,

	kEventNextStateSet,

	kEventSecondChanged,
	kEventMinuteChanged,

	kEventAnimationStarted,
	kEventAnimationCompleted,
	kEventTextureRewritten,
	kEventTextureSetFrame,
	kEventCharSetChangedFrame,
	kEventCharSetChangedOffset,
	kEventCharSetDeleted,
	kEventFontRewritten,

	kEventEffectFadeInComplete,
	kEventEffectFadeOutComplete,
	kEventEffectFadeStart,
	kEventEffectFadeStop,

	kEventCommunicationsConnected,
	kEventCommunicationsTransmissionCompleted,

	kEventActorDeleted,
	kEventActorCreated,
	kEventActorBounced,
	kEventActorCannotMove,

	kEventPlaySounds,

	kEventSoundFinished,
	kEventSoundReleased,

	kEventVIPManagerTimeError,
	kEventVIPManagerScanError,
	kEventVIPManagerFRAMESTART,
	kEventVIPManagerGAMESTART,
	kEventVIPManagerGAMESTARTDuringGAMESTART,
	kEventVIPManagerGAMESTARTDuringXPEND,
	kEventVIPManagerXPEND,
	kEventVIPManagerXPENDDuringXPEND,
	kEventVIPManagerXPENDDuringGAMESTART,

	kEventTimerManagerInterrupt,
	
	kEventKeypadManagerRaisedPowerFlag,

	kEventStateMachineWillCleanStack,
	kEventStateMachineCleanedStack,
	kEventStateMachineWillSwapState,
	kEventStateMachineSwapedState,
	kEventStateMachineWillPushState,
	kEventStateMachinePushedState,
	kEventStateMachineWillPopState,
	kEventStateMachinePoppedState,

	kEventVUEngineNextSecondStarted,

	// Do not remove me
	kEventPluginsFirst = 1000,
	kEventGameFirst = 10000,
	kEventEngineLast
};

#endif
