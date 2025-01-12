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
#include <BodyManager.h>
#include <Camera.h>
#include <CommunicationManager.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <ColliderManager.h>
#include <DebugConfig.h>
#include <DebugState.h>
#include <DirectDraw.h>
#include <ColliderManager.h>
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
#include <Printing.h>
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

const ClassPointer AnimationCoordinatorFactoryAuthClasses[] =
{
	typeofclass(Sprite),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer BehaviorManagerAuthClasses[] =
{
	typeofclass(VUEngine),
	NULL
};

const ClassPointer BgmapTextureManagerAuthClasses[] =
{
	typeofclass(SpriteManager),
	typeofclass(Stage),
	typeofclass(Texture),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer CameraAuthClasses[] =
{
	typeofclass(GameState),
	typeofclass(Stage),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer CharSetManagerAuthClasses[] =
{
	typeofclass(Printing),
	typeofclass(Stage),
	typeofclass(SpriteManager),
	NULL
};

const ClassPointer ClockManagerAuthClasses[] =
{
	typeofclass(Clock),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer CommunicationManagerAuthClasses[] =
{
	typeofclass(RumbleManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer DirectDrawAuthClasses[] =
{
	typeofclass(VUEngine),
	typeofclass(WireframeManager),
	NULL
};

const ClassPointer KeypadManagerAuthClasses[] =
{
	typeofclass(VUEngine),
	NULL
};

const ClassPointer MessageDispatcherAuthClasses[] =
{
	typeofclass(VUEngine),
	NULL
};

const ClassPointer ObjectTextureManagerAuthClasses[] =
{
	typeofclass(Texture),
	NULL
};

const ClassPointer ParamTableManagerAuthClasses[] =
{
	typeofclass(BgmapSprite),
	typeofclass(MBgmapSprite),
	typeofclass(SpriteManager),
	typeofclass(Stage),
	NULL
};

const ClassPointer RumbleManagerAuthClasses[] =
{
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SoundManagerAuthClasses[] =
{
	typeofclass(TimerManager),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SpriteManagerAuthClasses[] =
{
	typeofclass(ComponentManager),
	typeofclass(Stage),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer SRAMManagerAuthClasses[] =
{
	typeofclass(VUEngine),
	NULL
};

const ClassPointer StopwatchManagerAuthClasses[] =
{
	typeofclass(Stopwatch),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer TimerManagerAuthClasses[] =
{
	typeofclass(SoundTest),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer VIPManagerAuthClasses[] =
{
	typeofclass(Stage),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer VSUManagerAuthClasses[] =
{
	typeofclass(SoundManager),
	typeofclass(SoundTrack),
	typeofclass(VUEngine),
	NULL
};

const ClassPointer WireframeManagerAuthClasses[] =
{
	typeofclass(ComponentManager),
	typeofclass(VUEngine),
	NULL
};

#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Singleton::secure()
{
#ifndef __RELEASE
	AnimationCoordinatorFactory::secure(&AnimationCoordinatorFactoryAuthClasses);
	BehaviorManager::secure(&BehaviorManagerAuthClasses);
	BgmapTextureManager::secure(&BgmapTextureManagerAuthClasses);
	Camera::secure(&CameraAuthClasses);
	CharSetManager::secure(&CharSetManagerAuthClasses);
	ClockManager::secure(&ClockManagerAuthClasses);
//	CommunicationManager::secure(&CommunicationManagerAuthClasses);
	DirectDraw::secure(&DirectDrawAuthClasses);
	KeypadManager::secure(&KeypadManagerAuthClasses);
	MessageDispatcher::secure(&MessageDispatcherAuthClasses);
	ObjectTextureManager::secure(&ObjectTextureManagerAuthClasses);
	ParamTableManager::secure(&ParamTableManagerAuthClasses);
	RumbleManager::secure(&RumbleManagerAuthClasses);
	SoundManager::secure(&SoundManagerAuthClasses);
	SpriteManager::secure(&SpriteManagerAuthClasses);
	SRAMManager::secure(&SRAMManagerAuthClasses);
	StopwatchManager::secure(&StopwatchManagerAuthClasses);
	TimerManager::secure(&TimerManagerAuthClasses);
	VIPManager::secure(&VIPManagerAuthClasses);	
	VSUManager::secure(&VSUManagerAuthClasses);
	WireframeManager::secure(&WireframeManagerAuthClasses);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
