/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef EVENTS_H_
#define EVENTS_H_


//---------------------------------------------------------------------------------------------------------
//											 DEFINITIONS
//---------------------------------------------------------------------------------------------------------

enum Events
{
	// do not remove me
	kEventEngineFirst = 0,

	// add events here
	kEventTornFrame,
	kEventFrameRateDipped,

	kEventSpatialObjectDeleted,
	kEventContainerDeleted,
	kEventContainerAllChildrenDeleted,
	kEventEntityLoaded,
	kEventStageChildStreamedOut,
	kEventColliderDeleted,
	kEventColliderChanged,

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

	kEventActorBounced,
	kEventActorCannotMove,

	kEventSoundFinished,
	kEventSoundReleased,

	kEventVIPManagerInterrupt,
	kEventVIPManagerTimeError,
	kEventVIPManagerScanError,
	kEventVIPManagerGAMESTARTDuringGAMESTART,
	kEventVIPManagerXPENDDuringXPEND,
	kEventVIPManagerGAMESTARTDuringXPEND,
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

	// do not remove me
	kEventEngineLast
};


#endif
