/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef __DEBUG_TOOLS


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Debug.h>
#include <Game.h>
#include <Optics.h>
#include <FrameRate.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <CharSetManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <HardwareManager.h>
#include <SoundManager.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <RecyclableBgmapTextureManager.h>
#include <VIPManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Printing.h>
#include <MiscStructs.h>
#include <MBgmapSprite.h>
#include <BgmapAnimationCoordinator.h>
#include <ObjectAnimationCoordinator.h>
#include <KeyPadManager.h>
#include <SRAMManager.h>
#include <I18n.h>
#include <Screen.h>
#include <ScreenEffectManager.h>
#include <ScreenMovementManager.h>
#include <TimerManager.h>

#include <Clock.h>
#include <State.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <CharSet.h>
#include <Sprite.h>
#include <Texture.h>

#include <Body.h>
#include <CollisionSolver.h>
#include <Circle.h>
#include <Cuboid.h>
#include <InverseCuboid.h>
#include <Shape.h>
#include <Polyhedron.h>

#include <Container.h>
#include <Entity.h>
#include <CollisionsContainerEntity.h>
#include <InGameEntity.h>
#include <AnimatedInGameEntity.h>
#include <AnimationCoordinatorFactory.h>
#include <InanimatedInGameEntity.h>
#include <Actor.h>
#include <StaticImage.h>
#include <ManagedEntity.h>
#include <RecyclableImage.h>
#include <ManagedStaticImage.h>
#include <Particle.h>
#include <ParticleBody.h>
#include <ParticleSystem.h>
#include <SolidParticle.h>
#include <TriggerEntity.h>


#include <GameState.h>
#include <Stage.h>
#include <UiContainer.h>
#include <Mem.h>

#include <AnimationEditorState.h>
#include <DebugState.h>
#include <GameState.h>
#include <StageEditorState.h>

#include <AnimationEditor.h>
#include <Debug.h>
#include <OptionsSelector.h>
#include <StageEditor.h>

#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define DISPLACEMENT_STEP_X					512 - 384
#define DISPLACEMENT_STEP_Y					512 - 224

#define __CHARS_PER_SEGMENT_TO_SHOW		 512


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Debug_ATTRIBUTES																				\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* current in game state */																		\
		GameState gameState;																			\
		/* pages */																						\
		VirtualList pages;																				\
		/* sub pages */																					\
		VirtualList subPages;																			\
		/* current page */																				\
		VirtualNode currentPage;																		\
		/* current subb page */																			\
		VirtualNode currentSubPage;																		\
		/* current layer */																				\
		u8 currentLayer;																				\
		/* current bgmap */																				\
		int bgmapSegment;																				\
		/* current obj segment */																		\
		int objectSegment;																				\
		/* current char segment */																		\
		int charSegment;																				\
		/* current page in sram inspector */															\
		int sramPage;																					\
		/* window to look into bgmap memory */															\
		VBVec2D bgmapDisplacement;																		\
		/* update function pointer */																	\
		void (*update)(void *);																			\
		/* temporal array to hold char data */															\
		BYTE charMemoryMap[__CHARS_PER_SEGMENT_TO_SHOW];												\

/**
 * @class	Debug
 * @extends Object
 * @ingroup tools
 */
__CLASS_DEFINITION(Debug, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern ClassSizeData _userClassesSizeData[];

static void Debug_constructor(Debug this);
static void Debug_printClassSizes(ClassSizeData* classesSizeData, int size, int x, int y, char* message);
static void Debug_showCollisionShapes(Debug this);
static void Debug_showDebugBgmap(Debug this);
static void Debug_setupPages(Debug this);
static void Debug_showPage(Debug this, int increment);
static void Debug_showSubPage(Debug this, int increment);
static void Debug_removeSubPages(Debug this);
static void Debug_dimmGame(Debug this);
static void Debug_lightUpGame(Debug this);

// pages
static void Debug_showGeneralStatus(Debug this, int increment, int x, int y);
static void Debug_showMemoryStatus(Debug this, int increment, int x, int y);
static void Debug_showGameProfiling(Debug this, int increment, int x, int y);
static void Debug_showStreamingStatus(Debug this, int increment, int x, int y);
static void Debug_showSpritesStatus(Debug this, int increment, int x, int y);
static void Debug_showTextureStatus(Debug this, int increment, int x, int y);
static void Debug_showObjectStatus(Debug this, int increment, int x, int y);
static void Debug_showCharMemoryStatus(Debug this, int increment, int x, int y);
static void Debug_showPhysicsStatus(Debug this, int increment, int x, int y);
static void Debug_showHardwareStatus(Debug this, int increment, int x, int y);
static void Debug_showSramStatus(Debug this, int increment, int x, int y);

// sub pages
static void Debug_streamingShowStatus(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)));
static void Debug_RecyclableBgmapTextureManagerShowStatus(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)));
static void Debug_spritesShowStatus(Debug this, int increment, int x, int y);
static void Debug_texturesShowStatus(Debug this, int increment, int x, int y);
static void Debug_objectsShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowMemory(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowStatistics(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowShapes(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowZeroPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFirstPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowSecondPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowThirdPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFourthPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFifthPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowSixthPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowSeventhPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowUserDefinedClassesSizes(Debug this, int increment, int x, int y);
static void Debug_showSramPage(Debug this, int increment, int x, int y);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(Debug);

// class's constructor
static void __attribute__ ((noinline)) Debug_constructor(Debug this)
{
	ASSERT(this, "Debug::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->pages = __NEW(VirtualList);
	this->subPages = __NEW(VirtualList);
	this->currentPage = NULL;
	this->currentSubPage = NULL;

	this->gameState = NULL;

	this->currentLayer = __TOTAL_LAYERS - 1;
	this->bgmapSegment = 0;
	this->objectSegment = 0;
	this->charSegment = 0;
	this->sramPage = 0;

	this->update = NULL;

	this->bgmapDisplacement.x = 0;
	this->bgmapDisplacement.y = 0;

	Debug_setupPages(this);
}

// class's destructor
void Debug_destructor(Debug this)
{
	ASSERT(this, "Debug::destructor: null this");

	__DELETE(this->pages);
	__DELETE(this->subPages);

	// allow a new construct
	__SINGLETON_DESTROY;
}

// setup pages
static void Debug_setupPages(Debug this)
{
	VirtualList_pushBack(this->pages, &Debug_showGeneralStatus);
	VirtualList_pushBack(this->pages, &Debug_showMemoryStatus);
	VirtualList_pushBack(this->pages, &Debug_showGameProfiling);
	VirtualList_pushBack(this->pages, &Debug_showStreamingStatus);
	VirtualList_pushBack(this->pages, &Debug_showSpritesStatus);
	VirtualList_pushBack(this->pages, &Debug_showTextureStatus);
	VirtualList_pushBack(this->pages, &Debug_showObjectStatus);
	VirtualList_pushBack(this->pages, &Debug_showCharMemoryStatus);
	VirtualList_pushBack(this->pages, &Debug_showPhysicsStatus);
	VirtualList_pushBack(this->pages, &Debug_showHardwareStatus);
	VirtualList_pushBack(this->pages, &Debug_showSramStatus);

	this->currentPage = this->pages->head;
}

// get current pages
u8 Debug_getCurrentPageNumber(Debug this)
{
	return VirtualList_getNodePosition(this->pages, this->currentPage) + 1;
}

static void Debug_dimmGame(Debug this __attribute__ ((unused)))
{
	_vipRegisters[__GPLT0] = 0x50;
	_vipRegisters[__GPLT1] = 0x50;
	_vipRegisters[__GPLT2] = 0x50;
	_vipRegisters[__GPLT3] = 0x50;
	_vipRegisters[__JPLT0] = 0x50;
	_vipRegisters[__JPLT1] = 0x50;
	_vipRegisters[__JPLT2] = 0x50;
	_vipRegisters[__JPLT3] = 0x50;

	_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE4;
}

static void Debug_lightUpGame(Debug this)
{
	Stage_setupPalettes(GameState_getStage(this->gameState));
}

// update
void Debug_update(Debug this)
{
	if(this->update)
	{
		((void (*)(Debug))this->update)(this);
	}
}

void Debug_render(Debug this)
{
	if(this->currentPage->data == &Debug_showTextureStatus && 0 <= this->bgmapSegment)
	{
		Debug_showDebugBgmap(this);
	}
}

// show debug screens
void Debug_show(Debug this, GameState gameState)
{
	VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());

	this->gameState = gameState;

	Debug_dimmGame(this);
	Debug_showPage(this, 0);
}

// hide debug screens
void Debug_hide(Debug this)
{
	CollisionManager_hideShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
	VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());

	Debug_lightUpGame(this);
}

// show previous page
void Debug_showPreviousPage(Debug this)
{
	SpriteManager_recoverLayers(SpriteManager_getInstance());
	Debug_dimmGame(this);

	this->currentPage = VirtualNode_getPrevious(this->currentPage);

	if(NULL == this->currentPage)
	{
		this->currentPage = this->pages->tail;
	}

	Debug_showPage(this, -1);
}

// show next page
void Debug_showNextPage(Debug this)
{
	SpriteManager_recoverLayers(SpriteManager_getInstance());
	Debug_dimmGame(this);

	this->currentPage = this->currentPage->next;

	if(NULL == this->currentPage)
	{
		this->currentPage = this->pages->head;
	}

	Debug_showPage(this, 1);
}

// show previous sub page
void Debug_showPreviousSubPage(Debug this)
{
	if(!this->currentSubPage)
	{
		return;
	}

	this->currentSubPage = VirtualNode_getPrevious(this->currentSubPage);

	if(NULL == this->currentSubPage)
	{
		this->currentSubPage = this->subPages->tail;
	}

	Debug_showSubPage(this, -1);
}

// show next sub page
void Debug_showNextSubPage(Debug this)
{
	if(!this->currentSubPage)
	{
		return;
	}

	this->currentSubPage = this->currentSubPage->next;

	if(NULL == this->currentSubPage)
	{
		this->currentSubPage = this->subPages->head;
	}

	Debug_showSubPage(this, 1);
}

// print header
static void Debug_printHeader(Debug this)
{
	Printing_text(Printing_getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing_text(Printing_getInstance(), " DEBUG SYSTEM ", 1, 0, NULL);
	Printing_text(Printing_getInstance(), "   /   ", 16, 0, NULL);
	Printing_int(Printing_getInstance(), Debug_getCurrentPageNumber(this), Debug_getCurrentPageNumber(this) < 10 ? 18 : 17, 0, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->pages), 20, 0, NULL);
}

// show page
static void Debug_showPage(Debug this, int increment)
{
	if(this->currentPage && this->currentPage->data)
	{
		this->update = NULL;

		VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
		SpriteManager_recoverLayers(SpriteManager_getInstance());

		Debug_printHeader(this);
		Printing_text(Printing_getInstance(), " \x1E\x1C\x1D ", 42, 0, NULL);

		Debug_dimmGame(this);

		CollisionManager_hideShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));

		((void (*)(Debug, int, int, int))this->currentPage->data)(this, increment, 1, 2);
	}
}

// show sub page
static void Debug_showSubPage(Debug this, int increment)
{
	if(this->currentSubPage && VirtualNode_getData(this->currentSubPage))
	{
		this->update = NULL;

		VIPManager_clearBgmap(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);

		Debug_printHeader(this);
		Printing_text(Printing_getInstance(), " \x1E\x1A\x1B\x1C\x1D ", 40, 0, NULL);

		((void (*)(Debug, int, int, int))VirtualNode_getData(this->currentSubPage))(this, increment, 1, 2);
	}
}

// displace view to the left
void Debug_displaceLeft(Debug this)
{
	this->bgmapDisplacement.x = 0;
	Debug_showDebugBgmap(this);
}

// displace view to the right
void Debug_displaceRight(Debug this)
{
	this->bgmapDisplacement.x = DISPLACEMENT_STEP_X;
	Debug_showDebugBgmap(this);
}

// displace view up
void Debug_displaceUp(Debug this)
{
	this->bgmapDisplacement.y = 0;
	Debug_showDebugBgmap(this);
}

// displace view down
void Debug_displaceDown(Debug this)
{
	this->bgmapDisplacement.y = DISPLACEMENT_STEP_Y;
	Debug_showDebugBgmap(this);
}

static void Debug_removeSubPages(Debug this)
{
	VirtualList_clear(this->subPages);
	this->currentSubPage = NULL;
}

static void Debug_showGeneralStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug_removeSubPages(this);

	Printing_text(Printing_getInstance(), "GAME'S STATUS", 1, y++, NULL);
	Printing_text(Printing_getInstance(), "Current State:", 1, ++y, NULL);
	Printing_text(Printing_getInstance(), __GET_CLASS_NAME_UNSAFE(this->gameState), 19, y, NULL);
	Printing_text(Printing_getInstance(), "Auto Pause State:", 1, ++y, NULL);
	GameState autoPauseState = Game_getAutomaticPauseState(Game_getInstance());
	if(autoPauseState)
	{
		Printing_text(Printing_getInstance(), __GET_CLASS_NAME_UNSAFE(Game_getAutomaticPauseState(Game_getInstance())), 19, y, NULL);
	}
	else
	{
		Printing_text(Printing_getInstance(), "none", 19, y, NULL);
	}

	y += 3;

	Printing_text(Printing_getInstance(), "CLOCKS' STATUS", 1, y++, NULL);
	Printing_text(Printing_getInstance(), "General clock time: ", 1, ++y, NULL);
	Clock_print(Game_getClock(Game_getInstance()), 26, y, NULL);
	Printing_text(Printing_getInstance(), "In game clock's time: ", 1, ++y, NULL);
	Clock_print(GameState_getMessagingClock(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), 26, y, NULL);
	Printing_text(Printing_getInstance(), "Animations clock's time: ", 1, ++y, NULL);
	Clock_print(GameState_getUpdateClock(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), 26, y, NULL);
	Printing_text(Printing_getInstance(), "Physics clock's time: ", 1, ++y, NULL);
	Clock_print(GameState_getPhysicsClock(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), 26, y, NULL);
	y+=3;
}

static void Debug_showMemoryStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowZeroPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFirstPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSecondPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowThirdPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFourthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFifthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSixthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSeventhPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowUserDefinedClassesSizes);

	this->currentSubPage = this->subPages->head;

	Debug_showSubPage(this, 0);
}

static void Debug_memoryStatusShowZeroPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Game_getObjectSize, 							"Game"},
		{&DirectDraw_getObjectSize, 					"DirectDraw"},
		{&Error_getObjectSize, 							"Error"},
		{&FrameRate_getObjectSize, 						"FrameRate"},
		{&I18n_getObjectSize, 							"I18n"},
		{&MemoryPool_getObjectSize, 					"MemoryPool"},
		{&MessageDispatcher_getObjectSize, 				"MessageDispatcher"},
		{&Printing_getObjectSize, 						"Printing"},
		{&Screen_getObjectSize, 						"Screen"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

static void Debug_memoryStatusShowFirstPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&BgmapTextureManager_getObjectSize, 			"BgmapTextureManager"},
		{&CharSetManager_getObjectSize, 				"CharSetManager"},
		{&ClockManager_getObjectSize, 					"ClockManager"},
		{&CollisionManager_getObjectSize, 				"CollisionManager"},
		{&HardwareManager_getObjectSize, 				"HardwareManager"},
		{&KeypadManager_getObjectSize, 					"KeypadManager"},
		{&RecyclableBgmapTextureManager_getObjectSize, 			"RecyclableBgmapTextureManager"},
		{&ParamTableManager_getObjectSize, 				"ParamTableManager"},
		{&ScreenEffectManager_getObjectSize, 			"ScreenEff.Manager"},
		{&ScreenMovementManager_getObjectSize, 			"ScreenMov.Manager"},
		{&SpriteManager_getObjectSize, 					"SpriteManager"},
		{&SoundManager_getObjectSize, 					"SoundManager"},
		{&SRAMManager_getObjectSize, 					"SRAMManager"},
		{&TimerManager_getObjectSize, 					"TimerManager"},
		{&VIPManager_getObjectSize, 					"VIPManager"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

static void Debug_memoryStatusShowSecondPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Clock_getObjectSize, 							"Clock"},
		{&Object_getObjectSize, 						"Object"},
		{&State_getObjectSize, 							"State"},
		{&StateMachine_getObjectSize, 					"StateMachine"},
		{&Telegram_getObjectSize, 						"Telegram"},
		{&VirtualList_getObjectSize, 					"VirtualList"},
		{&VirtualNode_getObjectSize, 					"VirtualNode"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

static void Debug_memoryStatusShowThirdPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&AnimationController_getObjectSize, 			"AnimationController"},
		{&AnimationCoordinator_getObjectSize, 			"AnimationCoordinat."},
		{&AnimationCoordinatorFactory_getObjectSize,	"AnimationCoor.Fact."},
		{&BgmapAnimatedSprite_getObjectSize,			"BgmapAnim. Sprite"},
		{&BgmapAnimationCoordinator_getObjectSize,		"BgmapAnim. Coord."},
		{&BgmapSprite_getObjectSize, 					"BgmapSprite"},
		{&BgmapTexture_getObjectSize, 					"BgmapTexture"},
		{&CharSet_getObjectSize, 						"CharSet"},
		{&MBgmapSprite_getObjectSize, 					"MBgmapSprite"},
		{&ObjectAnimatedSprite_getObjectSize,			"ObjectAnim. Sprite"},
		{&ObjectAnimationCoordinator_getObjectSize,		"ObjectAnim.Coord."},
		{&ObjectSprite_getObjectSize,					"ObjectSprite"},
		{&ObjectSpriteContainer_getObjectSize,			"ObjectSpriteCont."},
		{&ObjectTexture_getObjectSize,					"ObjectTexture"},
		{&Texture_getObjectSize, 						"Texture"},
		{&Sprite_getObjectSize, 						"Sprite"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

static void Debug_memoryStatusShowFourthPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Body_getObjectSize, 							"Body"},
		{&CollisionSolver_getObjectSize, 				"CollisionSolver"},
		{&Circle_getObjectSize, 						"Circle"},
		{&Cuboid_getObjectSize,		 					"Cuboid"},
		{&InverseCuboid_getObjectSize,		 			"InverseCuboid"},
		{&PhysicalWorld_getObjectSize, 					"PhysicalWorld"},
		{&Polyhedron_getObjectSize, 						"Polyhedron"},
		{&Shape_getObjectSize, 							"Shape"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

static void Debug_memoryStatusShowFifthPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Actor_getObjectSize,							"Actor"},
		{&AnimatedInGameEntity_getObjectSize,			"Anim. InGameEntity"},
		{&Container_getObjectSize,						"Container"},
		{&CollisionsContainerEntity_getObjectSize,		"CollisionsCont.Ent."},
		{&Entity_getObjectSize,							"Entity"},
		{&EntityFactory_getObjectSize,					"EntityFactory"},
		{&GameState_getObjectSize,						"GameState"},
		{&GameState_getObjectSize,						"Stage"},
		{&StaticImage_getObjectSize,					"StaticImage"},
		{&InanimatedInGameEntity_getObjectSize,			"Inanim. InGam. Ent."},
		{&InGameEntity_getObjectSize,					"InGameEntity"},
		{&ManagedEntity_getObjectSize,					"ManagedEntity"},
		{&ManagedStaticImage_getObjectSize,				"ManagedStaticImage"},
		{&RecyclableImage_getObjectSize,				"RecyclableImage"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

static void Debug_memoryStatusShowSixthPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Particle_getObjectSize,						"Particle"},
		{&ParticleBody_getObjectSize,					"ParticleBody"},
		{&ParticleRemover_getObjectSize,				"ParticleRemover"},
		{&ParticleSystem_getObjectSize,					"ParticleSystem"},
		{&SolidParticle_getObjectSize,					"SolidParticle"},
		{&SpatialObject_getObjectSize,					"SpatialObject"},
		{&Stage_getObjectSize,							"Stage"},
		{&TriggerEntity_getObjectSize,					"TriggerEntity"},
		{&UiContainer_getObjectSize,					"UiContainer"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

static void Debug_memoryStatusShowSeventhPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&AnimationEditorState_getObjectSize,			"AnimationEditorSt."},
		{&DebugState_getObjectSize,						"DebugState"},
		{&GameState_getObjectSize,						"GameState"},
		{&StageEditorState_getObjectSize,				"StageEditorState"},
		{&AnimationEditor_getObjectSize,				"AnimationEditor"},
		{&Debug_getObjectSize,							"Debug"},
		{&OptionsSelector_getObjectSize,				"OptionsSelector"},
		{&StageEditor_getObjectSize,					"StageEditor"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}


static void Debug_memoryStatusShowUserDefinedClassesSizes(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	Debug_printClassSizes(_userClassesSizeData, 0, x + 21, y, "User defined classes:");
}

static void Debug_showGameProfiling(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug_removeSubPages(this);

	Game_showLastGameFrameProfiling(Game_getInstance(), x, y);
}

static void Debug_showStreamingStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_streamingShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_RecyclableBgmapTextureManagerShowStatus);
	this->currentSubPage = this->subPages->head;

	Debug_showSubPage(this, 0);
}

static void Debug_streamingShowStatus(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Stage_showStreamingProfiling(GameState_getStage(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), x, y);
}

static void Debug_RecyclableBgmapTextureManagerShowStatus(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Printing_text(Printing_getInstance(), "STREAMING'S STATUS", x, y++, NULL);
	RecyclableBgmapTextureManager_print(RecyclableBgmapTextureManager_getInstance(), x, ++y);
}

static void Debug_showCharMemoryStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	this->currentSubPage = this->subPages->head;

	this->charSegment = -1;

	Debug_showSubPage(this, 0);
}

static void Debug_charMemoryShowStatus(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	this->charSegment += increment;

	int charSegments = __CHAR_MEMORY_TOTAL_CHARS / __CHARS_PER_SEGMENT_TO_SHOW;

	if(-1 > this->charSegment)
	{
		this->charSegment = charSegments - 1;
	}

	if(-1 == this->charSegment)
	{
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		CharSetManager_print(CharSetManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
	else if(charSegments > this->charSegment)
	{
		Printing_text(Printing_getInstance(), "CHAR MEMORY'S USAGE", x, y++, NULL);
		Printing_text(Printing_getInstance(), "Chars:      -     ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW, x + 7, y, NULL);
		Printing_int(Printing_getInstance(), this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW + __CHARS_PER_SEGMENT_TO_SHOW - 1, x + 14, y, NULL);

		Debug_charMemoryShowMemory(this, increment, x, y);
		Debug_lightUpGame(this);
	}
	else
	{
		this->charSegment = -1;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		CharSetManager_print(CharSetManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
}

static void Debug_charMemoryShowMemory(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	SpriteManager_showLayer(SpriteManager_getInstance(), 0);

	int i = 0, j = 0;
	int yOffset = y + 3;

	// print box
	Printing_text(Printing_getInstance(), "\x03\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x04", 1, yOffset-1, NULL);
	Printing_text(Printing_getInstance(), "\x05\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x06", 1, yOffset+16, NULL);

	for(i = 0; i < __CHARS_PER_SEGMENT_TO_SHOW >> 5 && i < __SCREEN_HEIGHT / 8; i++)
	{
		Printing_text(Printing_getInstance(), "\x07                                \x07", 1, yOffset+i, NULL);
	}

	for(i = 0, j = this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW; i <  __CHARS_PER_SEGMENT_TO_SHOW; i+= 2, j++)
	{
		this->charMemoryMap[i] = (BYTE)(j & 0xFF);
		this->charMemoryMap[i + 1] = (BYTE)((j & 0xFF00) >> 8);
	}

	// put the map into memory calculating the number of char for each reference
	for(i = 0; i <  __CHARS_PER_SEGMENT_TO_SHOW >> 5; i++)
	{
		Mem_add
		(
			(u8*)__BGMAP_SEGMENT(BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance())) + (((yOffset << 6) + (i << 6) + 2) << 1),
			(u8*)this->charMemoryMap,
			32,
			(i << 5)
		);
	}
}

static void Debug_showTextureStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_texturesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_texturesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->bgmapSegment = -1;
	this->bgmapDisplacement.x = 0;
	this->bgmapDisplacement.y = 0;

	Debug_showSubPage(this, 0);
}

static void Debug_showDebugBgmap(Debug this)
{
	if(this->currentPage->data != &Debug_showTextureStatus ||
		0 > this->bgmapSegment
	)
	{
		return;
	}

	SpriteManager_showLayer(SpriteManager_getInstance(), 0);

	// write the head
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].head = __WORLD_ON | this->bgmapSegment;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].mx = this->bgmapDisplacement.x;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].mp = 0;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].my = this->bgmapDisplacement.y;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].gx = 0;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].gp = 3;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].gy = 0;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].w = __SCREEN_WIDTH;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].h = __SCREEN_HEIGHT;
}

static void Debug_texturesShowStatus(Debug this, int increment, int x, int y)
{
	this->bgmapSegment += increment;

	if(-1 > this->bgmapSegment)
	{
		this->bgmapSegment = BgmapTextureManager_getAvailableBgmapSegments(BgmapTextureManager_getInstance()) - 1;
	}

	if(-1 == this->bgmapSegment)
	{
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		BgmapTextureManager_print(BgmapTextureManager_getInstance(), x, y);

		ParamTableManager_print(ParamTableManager_getInstance(), x, y + 7);
		Debug_dimmGame(this);
	}
	else if(BgmapTextureManager_getAvailableBgmapSegments(BgmapTextureManager_getInstance()) > this->bgmapSegment)
	{
		Printing_text(Printing_getInstance(), " \x1E\x1A\x1B\x1C\x1D\x1F\x1A\x1B\x1C\x1D ", 35, 0, NULL);
		Printing_text(Printing_getInstance(), "BGMAP TEXTURES' USAGE", x, y++, NULL);
		Printing_text(Printing_getInstance(), "Segment: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->bgmapSegment, x + 9, y, NULL);

		this->bgmapDisplacement.x = 0;
		this->bgmapDisplacement.y = 0;

		Debug_showDebugBgmap(this);
		Debug_lightUpGame(this);
	}
	else
	{
		this->bgmapSegment = -1;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		BgmapTextureManager_print(BgmapTextureManager_getInstance(), x, y);
		ParamTableManager_print(ParamTableManager_getInstance(), x, y + 7);
		Debug_dimmGame(this);
	}
}

static void Debug_showObjectStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_objectsShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_objectsShowStatus);
	this->currentSubPage = this->subPages->head;

	this->objectSegment = -1;

	Debug_showSubPage(this, 0);
}

static void Debug_objectsShowStatus(Debug this, int increment, int x, int y)
{
	this->objectSegment += increment;

	if(-1 > this->objectSegment)
	{
		this->objectSegment = __TOTAL_OBJECT_SEGMENTS - 1;
	}

	if(-1 == this->objectSegment)
	{
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		ObjectSpriteContainerManager_print(ObjectSpriteContainerManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
	else if(__TOTAL_OBJECT_SEGMENTS > this->objectSegment)
	{
		Printing_text(Printing_getInstance(), "OBJECTS' USAGE", x, y++, NULL);

		ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainerManager_getObjectSpriteContainerBySegment(ObjectSpriteContainerManager_getInstance(), this->objectSegment);
		SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer(__SAFE_CAST(Sprite, objectSpriteContainer)));
		ObjectSpriteContainer_print(objectSpriteContainer, x, ++y);
		Debug_lightUpGame(this);
	}
	else
	{
		this->objectSegment = -1;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		ObjectSpriteContainerManager_print(ObjectSpriteContainerManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
}

static void Debug_showSpritesStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->currentLayer = __TOTAL_LAYERS;

	Debug_showSubPage(this, 0);
}

static void Debug_spritesShowStatus(Debug this, int increment, int x, int y)
{
	this->currentLayer -= increment;

	if(this->currentLayer > __TOTAL_LAYERS)
	{
		this->currentLayer = SpriteManager_getFreeLayer(SpriteManager_getInstance()) + 1;
	}

	if(__TOTAL_LAYERS == this->currentLayer)
	{
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		SpriteManager_print(SpriteManager_getInstance(), x, y);
	}
	else if(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->currentLayer)
	{
		Sprite sprite = SpriteManager_getSpriteAtLayer(SpriteManager_getInstance(), this->currentLayer);
		SpriteManager_showLayer(SpriteManager_getInstance(), this->currentLayer);

		Printing_text(Printing_getInstance(), "SPRITES' USAGE", x, y++, NULL);
		Printing_text(Printing_getInstance(), "Layer: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->currentLayer, x + 14, y, NULL);
		Printing_text(Printing_getInstance(), "Class: ", x, ++y, NULL);
		Printing_text(Printing_getInstance(), __GET_CLASS_NAME_UNSAFE(sprite), x + 14, y, NULL);
		Printing_text(Printing_getInstance(), "Head:                         ", x, ++y, NULL);
		Printing_hex(Printing_getInstance(), Sprite_getWorldHead(sprite), x + 14, y, 8, NULL);
		Printing_text(Printing_getInstance(), "Transparent:                         ", x, ++y, NULL);
		Printing_text(Printing_getInstance(), Sprite_isTransparent(sprite) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 14, y, NULL);
		Printing_text(Printing_getInstance(), "Position:                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL(Sprite, getPosition, sprite).x), x + 14, y, NULL);
		Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL(Sprite, getPosition, sprite).y), x + 24, y, NULL);
		Printing_float(Printing_getInstance(), FIX19_13TOF(__VIRTUAL_CALL(Sprite, getPosition, sprite).z + Sprite_getDisplacement(sprite).z), x + 34, y, NULL);
		Printing_text(Printing_getInstance(), "WORLD (x, y):                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldX(sprite), x + 14, y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldY(sprite), x + 24, y, NULL);
		Printing_text(Printing_getInstance(), "Size (w, h):                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldWidth(sprite), x + 14, y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldHeight(sprite), x + 24, y, NULL);

		if(Sprite_getTexture(sprite) && __GET_CAST(BgmapTexture, Sprite_getTexture(sprite)))
		{
			BgmapTexture bgmapTexture = __GET_CAST(BgmapTexture, Sprite_getTexture(sprite));

			Printing_text(Printing_getInstance(), "Texture (segment):                         ", x, ++y, NULL);
			Printing_int(Printing_getInstance(), BgmapTexture_getBgmapSegment(bgmapTexture), x + 24, y, NULL);
			Printing_text(Printing_getInstance(), "Texture (definition):                         ", x, ++y, NULL);
			Printing_hex(Printing_getInstance(), (int)Texture_getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture)), x + 24, y, 8, NULL);
			Printing_text(Printing_getInstance(), "Texture (written):                         ", x, ++y, NULL);
			Printing_text(Printing_getInstance(), Texture_isWritten(__SAFE_CAST(Texture, bgmapTexture)) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 24, y, NULL);
			Printing_text(Printing_getInstance(), "Texture (rows rem.):                         ", x, ++y, NULL);
			Printing_int(Printing_getInstance(), BgmapTexture_getRemainingRowsToBeWritten(bgmapTexture), x + 24, y, NULL);
		}
		//Debug_lightUpGame(this);
	}
	else
	{
		this->currentLayer = __TOTAL_LAYERS;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		SpriteManager_print(SpriteManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
}

static void Debug_showPhysicsStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowStatistics);
	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowShapes);
	this->currentSubPage = this->subPages->head;

	Debug_showSubPage(this, 0);
}

static void Debug_physicStatusShowStatistics(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	PhysicalWorld_print(GameState_getPhysicalWorld(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), x, y);
	CollisionManager_print(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), x, y + 6);
}

static void Debug_physicStatusShowShapes(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	Printing_text(Printing_getInstance(), "COLLISION SHAPES", x, y++, NULL);
	this->update = (void (*)(void *))&Debug_showCollisionShapes;
}

static void Debug_showCollisionShapes(Debug this __attribute__ ((unused)))
{
	CollisionManager_showShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
}

static void Debug_showHardwareStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug_removeSubPages(this);

	HardwareManager_print(HardwareManager_getInstance(), 1, y);
}

static void Debug_showSramStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_showSramPage);
	VirtualList_pushBack(this->subPages, &Debug_showSramPage);
	this->currentSubPage = this->subPages->head;

	this->sramPage = 0;

	Debug_showSubPage(this, 0);
}

static void Debug_showSramPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	u8 value;
	int i, j, totalPages;
	char word[9];

	totalPages = __TOTAL_SAVE_RAM >> 7;

	extern u32 _sram_bss_end;

	this->sramPage += increment;

	if(this->sramPage < 0)
	{
	   this->sramPage = totalPages - 1;
	}
	else if(this->sramPage >= totalPages)
	{
	   this->sramPage = 0;
	}

	// get sram base address
	u16* startAddress = (u16*)&_sram_bss_end;

	// print status header
	Printing_text(Printing_getInstance(), "SRAM STATUS", 1, y++, NULL);
	Printing_text(Printing_getInstance(), "Total (kb):", 1, ++y, NULL);
	Printing_int(Printing_getInstance(), __TOTAL_SAVE_RAM >> 10, 13, y, NULL);
	y+=2;

	// print inspector header
	Printing_text(Printing_getInstance(), "SRAM INSPECTOR", 1, ++y, NULL);
	Printing_text(Printing_getInstance(), "Page     /", 33, y, NULL);
	Printing_int(Printing_getInstance(), totalPages, 43, y, NULL);
	Printing_int(Printing_getInstance(), this->sramPage + 1, 38, y++, NULL);
	Printing_text(Printing_getInstance(), "Address     00 01 02 03 04 05 06 07 Word", 1, ++y, NULL);
	Printing_text(Printing_getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 1, ++y, NULL);

	// print values
	for(i = 0; i < 16; i++)
	{
		// print address
		Printing_text(Printing_getInstance(), "0x00000000: ", 1, ++y, NULL);
		Printing_hex(Printing_getInstance(), (int)startAddress + (this->sramPage << 7) + (i << 3), 3, y, 8, NULL);

		// values
		for(j = 0; j < 8; j++)
		{
			// read byte from sram
			value = startAddress[(this->sramPage << 7) + (i << 3) + j];

			// print byte
			Printing_hex(Printing_getInstance(), value, 13 + (j*3), y, 2, NULL);

			// add current character to line word
			// if outside of extended ascii range, print whitespace
			word[j] = (value >= 32) ? (char)value : (char)32;
			//word[j] = value ? (char)value : (char)32;
		}

		// add termination character to string
		word[8] = (char)0;

		// print word
		Printing_text(Printing_getInstance(), word, 37, y, NULL);

		// print scroll bar
		Printing_text(Printing_getInstance(), __CHAR_MEDIUM_RED_BOX, 46, y, NULL);
	}

	// mark scroll bar position
	Printing_text(Printing_getInstance(), __CHAR_BRIGHT_RED_BOX, 46, y - 15 + (this->sramPage / (totalPages >> 4)), NULL);
}

static void Debug_printClassSizes(ClassSizeData* classesSizeData, int size, int x, int y, char* message)
{
	int columnIncrement = 20;

	Printing_text(Printing_getInstance(), "CLASSES' MEMORY USAGE (B) ", x, y++, NULL);

	if(message)
	{
		Printing_text(Printing_getInstance(), message, x, ++y, NULL);
		y++;
	}

	Printing_text(Printing_getInstance(), "Name				Size", x, ++y, NULL);
	y++;

	int i = 0;
	for(; classesSizeData[i].classSizeFunction && (0 == size || i < size); i++)
	{
		Printing_text(Printing_getInstance(), classesSizeData[i].name, x, ++y, NULL);
		Printing_int(Printing_getInstance(), ((int (*)(void))classesSizeData[i].classSizeFunction)(), x + columnIncrement, y, NULL);
	}
}


#endif
