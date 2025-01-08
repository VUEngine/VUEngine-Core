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

	kEventEntityDeleted,
	kEventContainerDeleted,
	kEventContainerAllChildrenDeleted,
	kEventActorLoaded,
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
	kEventCharSetChangedOffset,
	kEventCharSetDeleted,
	kEventFontRewritten,

	kEventEffectFadeComplete,
	kEventEffectFadeStart,
	kEventEffectFadeStop,

	kEventCommunicationsConnected,
	kEventCommunicationsTransmissionCompleted,

	kEventStatefulActorBounced,
	kEventStatefulActorCannotMove,

	kEventSoundFinished,
	kEventSoundReleased,

	kEventVIPManagerInterrupt,
	kEventVIPManagerTimeError,
	kEventVIPManagerScanError,
	kEventVIPManagerFRAMESTART,
	kEventVIPManagerGAMESTART,
	kEventVIPManagerGAMESTARTDuringGAMESTART,
	kEventVIPManagerGAMESTARTDuringXPEND,
	kEventVIPManagerXPEND,
	kEventVIPManagerXPENDDuringXPEND,
	kEventVIPManagerXPENDDuringGAMESTART,

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
	kEventEngineLast
};

#endif
