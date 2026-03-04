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
#include <MutatorManager.h>
#include <BgmapSprite.h>
#include <BgmapTextureManager.h>
#include <Body.h>
#include <BodyManager.h>
#include <Camera.h>
#include <Communications.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <ColliderManager.h>
#include <DebugConfig.h>
#include <DebugState.h>
#include <FrameBuffers.h>
#include <Error.h>
#include <FrameRate.h>
#include <GameState.h>
#include <Hardware.h>
#include <Keypad.h>
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
#include <Rumble.h>
#include <SoundManager.h>
#include <SoundTrack.h>
#include <SpriteManager.h>
#include <SRAM.h>
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
#include <Timer.h>
#include <UIContainer.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <DisplayUnit.h>
#include <SoundUnit.h>
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
	typeofclass(GameState),
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
	typeofclass(CharSet),
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

const ClassPointer CommunicationsAuthorizedClasses[] =
{
	typeofclass(Rumble),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer FrameBuffersAuthorizedClasses[] =
{
	typeofclass(Hardware),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer KeypadAuthorizedClasses[] =
{
	typeofclass(GameState),
	typeofclass(Hardware),
	typeofclass(ToolState),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer MessageDispatcherAuthorizedClasses[] =
{
	typeofclass(GameState),
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
	typeofclass(GameState),
	typeofclass(MBgmapSprite),
	typeofclass(Printer),
	typeofclass(SpriteManager),
	NULL
};

const ClassPointer PrintingAuthorizedClasses[] =
{
	typeofclass(SpriteManager),
	NULL
};

const ClassPointer RumbleAuthorizedClasses[] =
{
	typeofclass(Hardware),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SoundManagerAuthorizedClasses[] =
{
	typeofclass(GameState),
#ifdef __TOOLS
	typeofclass(SoundTest),
#endif
	typeofclass(Timer),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SRAMAuthorizedClasses[] =
{
	typeofclass(Hardware),
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

const ClassPointer TimerAuthorizedClasses[] =
{
#ifdef __TOOLS
	typeofclass(SoundTest),
#endif
	typeofclass(Hardware),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer DisplayUnitAuthorizedClasses[] =
{
	typeofclass(Communications),
	typeofclass(GameState),
	typeofclass(Hardware),
	typeofclass(SpriteManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SoundUnitAuthorizedClasses[] =
{
	typeofclass(Hardware),
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
	return;
#ifndef __RELEASE
	AnimationCoordinatorFactory::secure(&AnimationCoordinatorFactoryAuthorizedClasses);
//	BgmapTextureManager::secure(&BgmapTextureManagerAuthorizedClasses);
	Camera::secure(&CameraAuthorizedClasses);
	CharSetManager::secure(&CharSetManagerAuthorizedClasses);
	ClockManager::secure(&ClockManagerAuthorizedClasses);
	Communications::secure(&CommunicationsAuthorizedClasses);
	FrameBuffers::secure(&FrameBuffersAuthorizedClasses);
//	Keypad::secure(&KeypadAuthorizedClasses);
	MessageDispatcher::secure(&MessageDispatcherAuthorizedClasses);
	ObjectTextureManager::secure(&ObjectTextureManagerAuthorizedClasses);
	ParamTableManager::secure(&ParamTableManagerAuthorizedClasses);
	Printer::secure(&PrintingAuthorizedClasses);
//	Rumble::secure(&RumbleAuthorizedClasses);
//	SRAM::secure(&SRAMAuthorizedClasses);
	StopwatchManager::secure(&StopwatchManagerAuthorizedClasses);
	Timer::secure(&TimerAuthorizedClasses);
	DisplayUnit::secure(&DisplayUnitAuthorizedClasses);	
	SoundUnit::secure(&SoundUnitAuthorizedClasses);
	VUEngine::secure(&VUEngineAuthorizedClasses);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
