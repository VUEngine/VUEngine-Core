/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <AnimationCoordinatorFactory.h>
#include <AnimationInspectorState.h>
#include <BehaviorManager.h>
#include <BgmapSprite.h>
#include <BgmapTextureManager.h>
#include <Body.h>
#include <BodyManager.h>
#include <Camera.h>
#include <CommunicationManager.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <ColliderManager.h>
#include <DebugConfig.h>
#include <DebugState.h>
#include <FrameBufferManager.h>
#include <Error.h>
#include <FrameRate.h>
#include <GameState.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <MBgmapSprite.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <ObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <ObjectTextureManager.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <Printer.h>
#include <Profiler.h>
#include <RumbleManager.h>
#include <SoundManager.h>
#include <SoundTrack.h>
#include <SpriteManager.h>
#include <SRAMManager.h>
#include <Stage.h>
#include <StageEditor.h>
#include <StageEditorState.h>
#include <State.h>
#include <StateMachine.h>
#include <Stopwatch.h>
#include <StopwatchManager.h>
#include <SoundTest.h>
#include <Telegram.h>
#include <ToolState.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VSUManager.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "Singleton.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __RELEASE

const ClassPointer AnimationCoordinatorFactoryAuthorizedClasses[] =
{
	typeofclass(Sprite),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer BgmapTextureManagerAuthorizedClasses[] =
{
	typeofclass(Printer),
	typeofclass(SpriteManager),
	typeofclass(Stage),
	typeofclass(Texture),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer CameraAuthorizedClasses[] =
{
	typeofclass(GameState),
	typeofclass(Stage),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer CharSetManagerAuthorizedClasses[] =
{
	typeofclass(Printer),
	typeofclass(Stage),
	typeofclass(SpriteManager),
	NULL
};

const ClassPointer ClockManagerAuthorizedClasses[] =
{
	typeofclass(Clock),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer CommunicationManagerAuthorizedClasses[] =
{
	typeofclass(RumbleManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer FrameBufferManagerAuthorizedClasses[] =
{
	typeofclass(HardwareManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer KeypadManagerAuthorizedClasses[] =
{
	typeofclass(HardwareManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer MessageDispatcherAuthorizedClasses[] =
{
	typeofclass(GameState),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer ObjectTextureManagerAuthorizedClasses[] =
{
	typeofclass(Texture),
	NULL
};

const ClassPointer ParamTableManagerAuthorizedClasses[] =
{
	typeofclass(BgmapSprite),
	typeofclass(MBgmapSprite),
	typeofclass(Printer),
	typeofclass(SpriteManager),
	typeofclass(Stage),
	NULL
};

const ClassPointer PrintingAuthorizedClasses[] =
{
	typeofclass(SpriteManager),
	NULL
};

const ClassPointer RumbleManagerAuthorizedClasses[] =
{
	typeofclass(HardwareManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SoundManagerAuthorizedClasses[] =
{
	typeofclass(GameState),
#ifdef __TOOLS
	typeofclass(SoundTest),
#endif
	typeofclass(TimerManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SRAMManagerAuthorizedClasses[] =
{
	typeofclass(HardwareManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer StopwatchManagerAuthorizedClasses[] =
{
	typeofclass(GameState),
	typeofclass(Stopwatch),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer TimerManagerAuthorizedClasses[] =
{
#ifdef __TOOLS
	typeofclass(SoundTest),
#endif
	typeofclass(HardwareManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer VIPManagerAuthorizedClasses[] =
{
	typeofclass(HardwareManager),
	typeofclass(Stage),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer VSUManagerAuthorizedClasses[] =
{
	typeofclass(HardwareManager),
	typeofclass(SoundManager),
	typeofclass(SoundTrack),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer VUEngineAuthorizedClasses[] =
{
	NULL
};

#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Singleton::secure()
{
#ifndef __RELEASE
	AnimationCoordinatorFactory::secure(&AnimationCoordinatorFactoryAuthorizedClasses);
	BgmapTextureManager::secure(&BgmapTextureManagerAuthorizedClasses);
	Camera::secure(&CameraAuthorizedClasses);
	CharSetManager::secure(&CharSetManagerAuthorizedClasses);
	ClockManager::secure(&ClockManagerAuthorizedClasses);
	CommunicationManager::secure(&CommunicationManagerAuthorizedClasses);
	FrameBufferManager::secure(&FrameBufferManagerAuthorizedClasses);
	KeypadManager::secure(&KeypadManagerAuthorizedClasses);
	MessageDispatcher::secure(&MessageDispatcherAuthorizedClasses);
	ObjectTextureManager::secure(&ObjectTextureManagerAuthorizedClasses);
	ParamTableManager::secure(&ParamTableManagerAuthorizedClasses);
	Printer::secure(&PrintingAuthorizedClasses);
	RumbleManager::secure(&RumbleManagerAuthorizedClasses);
	SoundManager::secure(&SoundManagerAuthorizedClasses);
	SRAMManager::secure(&SRAMManagerAuthorizedClasses);
	StopwatchManager::secure(&StopwatchManagerAuthorizedClasses);
	TimerManager::secure(&TimerManagerAuthorizedClasses);
	VIPManager::secure(&VIPManagerAuthorizedClasses);	
	VSUManager::secure(&VSUManagerAuthorizedClasses);
	VUEngine::secure(&VUEngineAuthorizedClasses);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
