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
#include <VIPManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <MiscStructs.h>
#include <MBgmapSprite.h>
#include <BgmapAnimationCoordinator.h>
#include <ObjectAnimationCoordinator.h>
#include <KeyPadManager.h>
#include <SRAMManager.h>
#include <I18n.h>
#include <Camera.h>
#include <CameraEffectManager.h>
#include <CameraMovementManager.h>
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
#include <Ball.h>
#include <Box.h>
#include <InverseBox.h>
#include <Shape.h>
#include <Wireframe.h>
#include <Sphere.h>
#include <Polyhedron.h>

#include <Container.h>
#include <Entity.h>
#include <Entity.h>
#include <AnimatedEntity.h>
#include <AnimationCoordinatorFactory.h>
#include <Entity.h>
#include <Actor.h>
#include <ManagedEntity.h>
#include <ManagedEntity.h>
#include <Particle.h>
#include <ParticleBody.h>
#include <ParticleSystem.h>
#include <SolidParticle.h>


#include <GameState.h>
#include <Stage.h>
#include <UiContainer.h>
#include <Mem.h>

#include <DebugState.h>
#include <DebugState.h>
#include <GameState.h>
#include <StageEditorState.h>

#include <Debug.h>
#include <Debug.h>
#include <OptionsSelector.h>
#include <StageEditor.h>

#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define DISPLACEMENT_STEP_X				512 - 384
#define DISPLACEMENT_STEP_Y				512 - 224

#define __CHARS_PER_SEGMENT_TO_SHOW		512
#define __CHARS_PER_ROW_TO_SHOW			32


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Debug_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
		/**
		 * @var GameState	gameState
		 * @brief			current in game state
		 * @memberof		Debug
		 */																								\
		GameState gameState;																			\
		/**
		 * @var VirtualList	pages
		 * @brief			pages
		 * @memberof		Debug
		 */																								\
		VirtualList pages;																				\
		/**
		 * @var VirtualList	subPages
		 * @brief			sub pages
		 * @memberof		Debug
		 */																								\
		VirtualList subPages;																			\
		/**
		 * @var VirtualNode	currentPage
		 * @brief			current page
		 * @memberof		Debug
		 */																								\
		VirtualNode currentPage;																		\
		/**
		 * @var VirtualNode	currentSubPage
		 * @brief			current sub page
		 * @memberof		Debug
		 */																								\
		VirtualNode currentSubPage;																		\
		/**
		 * @var u8			currentLayer
		 * @brief			current layer
		 * @memberof		Debug
		 */																								\
		u8 currentLayer;																				\
		/**
		 * @var int			bgmapSegment
		 * @brief			current bgmap
		 * @memberof		Debug
		 */																								\
		int bgmapSegment;																				\
		/**
		 * @var int			objectSegment
		 * @brief			current obj segment
		 * @memberof		Debug
		 */																								\
		int objectSegment;																				\
		/**
		 * @var int			charSegment
		 * @brief			current char segment
		 * @memberof		Debug
		 */																								\
		int charSegment;																				\
		/**
		 * @var int			sramPage
		 * @brief			current page in sram inspector
		 * @memberof		Debug
		 */																								\
		int sramPage;																					\
		/**
		 * @var Vector2D		mapDisplacement
		 * @brief			window to look into bgmap memory
		 * @memberof		Debug
		 */																								\
		Vector2D mapDisplacement;																		\
		/**
		 * @var void 		(*update)(void	*)
		 * @brief			update function pointer
		 * @memberof		Debug
		 */																								\
		void (*update)(void *);																			\

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

#ifdef __DEBUG_TOOLS
extern ClassSizeData _userClassesSizeData[];
#endif

static void Debug_constructor(Debug this);
static void Debug_showCollisionShapes(Debug this);
static void Debug_showDebugBgmap(Debug this);
static void Debug_showBgmapSegment(Debug this);
static void Debug_setupPages(Debug this);
static void Debug_showPage(Debug this, int increment);
static void Debug_showSubPage(Debug this, int increment);
static void Debug_removeSubPages(Debug this);
static void Debug_dimmGame(Debug this);
static void Debug_lightUpGame(Debug this);

// pages
static void Debug_generalStatusPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusPage(Debug this, int increment, int x, int y);
static void Debug_gameProfilingPage(Debug this, int increment, int x, int y);
static void Debug_streamingPage(Debug this, int increment, int x, int y);
static void Debug_spritesPage(Debug this, int increment, int x, int y);
static void Debug_texturesPage(Debug this, int increment, int x, int y);
static void Debug_objectsPage(Debug this, int increment, int x, int y);
static void Debug_charMemoryPage(Debug this, int increment, int x, int y);
static void Debug_physicsPage(Debug this, int increment, int x, int y);
static void Debug_hardwareRegistersPage(Debug this, int increment, int x, int y);
static void Debug_sramPage(Debug this, int increment, int x, int y);

// sub pages
static void Debug_streamingShowStatus(Debug this, int increment, int x, int y);
static void Debug_spritesShowStatus(Debug this, int increment, int x, int y);
static void Debug_texturesShowStatus(Debug this, int increment, int x, int y);
static void Debug_objectsShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowMemory(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowStatistics(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowShapes(Debug this, int increment, int x, int y);

#ifdef __DEBUG_TOOLS
static void Debug_printClassSizes(Debug this __attribute__ ((unused)), ClassSizeData* classesSizeData, int count, int x, int y, char* message);

static void Debug_memoryStatusShowZeroPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFirstPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowSecondPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowThirdPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFourthPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFifthPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowSixthPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowSeventhPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowUserDefinedClassesSizes(Debug this, int increment, int x, int y);
#endif

static void Debug_showPreviousPage(Debug this);
static void Debug_showNextPage(Debug this);
static void Debug_showPreviousSubPage(Debug this);
static void Debug_showNextSubPage(Debug this);
static void Debug_displaceLeft(Debug this);
static void Debug_displaceRight(Debug this);
static void Debug_displaceUp(Debug this);
static void Debug_displaceDown(Debug this);

static void Debug_showSramPage(Debug this, int increment, int x, int y);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Debug_getInstance()
 * @memberof	Debug
 * @public
 *
 * @return		Debug instance
 */
__SINGLETON(Debug);

/**
 * Class constructor
 *
 * @memberof	Debug
 * @private
 *
 * @param this	Function scope
 */
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

	this->mapDisplacement.x = 0;
	this->mapDisplacement.y = 0;

	Debug_setupPages(this);
}

/**
 * Class destructor
 *
 * @memberof	Debug
 * @public
 *
 * @param this	Function scope
 */
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
	VirtualList_pushBack(this->pages, &Debug_generalStatusPage);
	VirtualList_pushBack(this->pages, &Debug_memoryStatusPage);
	VirtualList_pushBack(this->pages, &Debug_gameProfilingPage);
	VirtualList_pushBack(this->pages, &Debug_streamingPage);
	VirtualList_pushBack(this->pages, &Debug_spritesPage);
	VirtualList_pushBack(this->pages, &Debug_texturesPage);
	VirtualList_pushBack(this->pages, &Debug_objectsPage);
	VirtualList_pushBack(this->pages, &Debug_charMemoryPage);
	VirtualList_pushBack(this->pages, &Debug_physicsPage);
	VirtualList_pushBack(this->pages, &Debug_hardwareRegistersPage);
	VirtualList_pushBack(this->pages, &Debug_sramPage);

	this->currentPage = this->pages->head;
}

/**
 * Update
 *
 * @memberof	Debug
 * @public
 *
 * @param this	Function scope
 */
void Debug_update(Debug this)
{
	if(this->update)
	{
		((void (*)(Debug))this->update)(this);
	}
}

/**
 * Render
 *
 * @memberof	Debug
 * @public
 *
 * @param this	Function scope
 */
void Debug_render(Debug this)
{
	if(this->currentPage->data == &Debug_texturesPage && 0 <= this->bgmapSegment)
	{
		Debug_showBgmapSegment(this);
	}
}

/**
 * Show debugging screens
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 * @param gameState Current game state
 */
void Debug_show(Debug this, GameState gameState)
{
	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());

	this->gameState = gameState;

	Debug_dimmGame(this);
	Debug_showPage(this, 0);
}

/**
 * Hide debugging screens
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
void Debug_hide(Debug this)
{
	CollisionManager_hideShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
	VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());

	Debug_lightUpGame(this);
}

/**
 * Get the position in the pages list of the current page
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 *
 * @return 			Current page's node's position
 */
u8 Debug_getCurrentPageNumber(Debug this)
{
	return VirtualList_getNodePosition(this->pages, this->currentPage) + 1;
}

/**
 * Dimm game to make text easier to read
 *
 * @memberof		Debug
 * @private
 *
 * @param this		Function scope
 */
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

/**
 * Recover game's colors
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_lightUpGame(Debug this)
{
	Stage_setupPalettes(GameState_getStage(this->gameState));
}

/**
 * Process user input
 *
 * @memberof			Debug
 * @public
 *
 * @param this			Function scope
 * @param pressedKey	User input
 */
void Debug_processUserInput(Debug this __attribute__ ((unused)), u16 pressedKey)
{
	ASSERT(this, "Debug::processUserInput: null this");

	if(pressedKey & K_LL)
	{
		Debug_showPreviousPage(this);
	}
	else if(pressedKey & K_LR)
	{
		Debug_showNextPage(this);
	}
	else if(pressedKey & K_LU)
	{
		Debug_showPreviousSubPage(this);
	}
	else if(pressedKey & K_LD)
	{
		Debug_showNextSubPage(this);
	}
	else if(pressedKey & K_RL)
	{
		Debug_displaceLeft(this);
	}
	else if(pressedKey & K_RR)
	{
		Debug_displaceRight(this);
	}
	else if(pressedKey & K_RU)
	{
		Debug_displaceUp(this);
	}
	else if(pressedKey & K_RD)
	{
		Debug_displaceDown(this);
	}
}

/**
 * Show previous debugging page
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_showPreviousPage(Debug this)
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

/**
 * Show next debugging page
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_showNextPage(Debug this)
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

/**
 * Show previous debugging sub-page
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_showPreviousSubPage(Debug this)
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

/**
 * Show next debugging sub-page
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_showNextSubPage(Debug this)
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

/**
 * Print header
 *
 * @memberof		Debug
 * @private
 *
 * @param this		Function scope
 */
static void Debug_printHeader(Debug this)
{
	Printing_text(Printing_getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing_text(Printing_getInstance(), " DEBUG SYSTEM ", 1, 0, NULL);
	Printing_text(Printing_getInstance(), "   /   ", 16, 0, NULL);
	Printing_int(Printing_getInstance(), Debug_getCurrentPageNumber(this), Debug_getCurrentPageNumber(this) < 10 ? 18 : 17, 0, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->pages), 20, 0, NULL);
}

/**
 * Show current debugging page
 *
 * @memberof			Debug
 * @public
 *
 * @param this			Function scope
 * @param increment		Increment
 */
static void Debug_showPage(Debug this, int increment)
{
	if(this->currentPage && this->currentPage->data)
	{
		this->update = NULL;

		VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
		SpriteManager_recoverLayers(SpriteManager_getInstance());

		Debug_printHeader(this);
		Printing_text(Printing_getInstance(), " \x1E\x1C\x1D ", 42, 0, NULL);

		Debug_dimmGame(this);

		CollisionManager_hideShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));

		((void (*)(Debug, int, int, int))this->currentPage->data)(this, increment, 1, 2);
	}
}

/**
 * Show current debugging sub-page
 *
 * @memberof			Debug
 * @public
 *
 * @param this			Function scope
 * @param increment		Increment
 */
static void Debug_showSubPage(Debug this, int increment)
{
	if(this->currentSubPage && VirtualNode_getData(this->currentSubPage))
	{
		this->update = NULL;

		VIPManager_clearBgmapSegment(VIPManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);

		Debug_printHeader(this);
		Printing_text(Printing_getInstance(), " \x1E\x1A\x1B\x1C\x1D ", 40, 0, NULL);

		((void (*)(Debug, int, int, int))VirtualNode_getData(this->currentSubPage))(this, increment, 1, 2);
	}
}

/**
 * Displace window to the left
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_displaceLeft(Debug this)
{
	this->mapDisplacement.x = 0;
	Debug_showDebugBgmap(this);
}

/**
 * Displace window to the right
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_displaceRight(Debug this)
{
	this->mapDisplacement.x = DISPLACEMENT_STEP_X;
	Debug_showDebugBgmap(this);
}

/**
 * Displace window up
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_displaceUp(Debug this)
{
	this->mapDisplacement.y = 0;
	Debug_showDebugBgmap(this);
}

/**
 * Displace window down
 *
 * @memberof		Debug
 * @public
 *
 * @param this		Function scope
 */
static void Debug_displaceDown(Debug this)
{
	this->mapDisplacement.y = DISPLACEMENT_STEP_Y;
	Debug_showDebugBgmap(this);
}

/**
 * Remove sub-pages
 *
 * @memberof		Debug
 * @private
 *
 * @param this		Function scope
 */
static void Debug_removeSubPages(Debug this)
{
	VirtualList_clear(this->subPages);
	this->currentSubPage = NULL;
}

/**
 * Setup Game's general status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_generalStatusPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
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

/**
 * Setup Memory Pool's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_memoryStatusPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

#ifdef __DEBUG_TOOLS

	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowZeroPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFirstPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSecondPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowThirdPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFourthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFifthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSixthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSeventhPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowUserDefinedClassesSizes);

#endif

	this->currentSubPage = this->subPages->head;

	Debug_showSubPage(this, 0);
}

#ifdef __DEBUG_TOOLS

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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
		{&Camera_getObjectSize, 						"Camera"},
	};

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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
		{&ParamTableManager_getObjectSize, 				"ParamTableManager"},
		{&CameraEffectManager_getObjectSize, 			"CameraEff.Manager"},
		{&CameraMovementManager_getObjectSize, 			"CameraMov.Manager"},
		{&SpriteManager_getObjectSize, 					"SpriteManager"},
		{&SoundManager_getObjectSize, 					"SoundManager"},
		{&SRAMManager_getObjectSize, 					"SRAMManager"},
		{&TimerManager_getObjectSize, 					"TimerManager"},
		{&VIPManager_getObjectSize, 					"VIPManager"},
	};

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_memoryStatusShowFourthPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&PhysicalWorld_getObjectSize, 					"PhysicalWorld"},
		{&Body_getObjectSize, 							"Body"},
		{&Shape_getObjectSize, 							"Shape"},
		{&Ball_getObjectSize, 							"Ball"},
		{&Box_getObjectSize,		 					"Box"},
		{&InverseBox_getObjectSize,		 				"InverseBox"},
		{&Wireframe_getObjectSize, 						"Wireframe"},
		{&Polyhedron_getObjectSize, 					"Polyhedron"},
		{&Sphere_getObjectSize, 						"Sphere"},
	};

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_memoryStatusShowFifthPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Actor_getObjectSize,							"Actor"},
		{&AnimatedEntity_getObjectSize,					"AnimatedEntity"},
		{&Container_getObjectSize,						"Container"},
		{&Entity_getObjectSize,							"Entity"},
		{&EntityFactory_getObjectSize,					"EntityFactory"},
		{&GameState_getObjectSize,						"GameState"},
		{&GameState_getObjectSize,						"Stage"},
		{&ManagedEntity_getObjectSize,					"ManagedEntity"},
	};

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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
		{&UiContainer_getObjectSize,					"UiContainer"},
	};

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_memoryStatusShowSeventhPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&DebugState_getObjectSize,			"DebugSt."},
		{&DebugState_getObjectSize,						"DebugState"},
		{&GameState_getObjectSize,						"GameState"},
		{&StageEditorState_getObjectSize,				"StageEditorState"},
		{&Debug_getObjectSize,				"Debug"},
		{&Debug_getObjectSize,							"Debug"},
		{&OptionsSelector_getObjectSize,				"OptionsSelector"},
		{&StageEditor_getObjectSize,					"StageEditor"},
	};

	Debug_printClassSizes(this, classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VUEngine classes:");
}

/**
 * Show classes' memory footprint
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_memoryStatusShowUserDefinedClassesSizes(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);
	Debug_printClassSizes(this, _userClassesSizeData, 0, x + 21, y, "User defined classes:");
}

/**
 * Print classes' sizes
 *
 * @memberof					Debug
 * @private
 *
 * @param this					Function scope
 * @param classesSizeData		Array with a class names and their sizes
 * @param count					Number of entries to print
 * @param x						Camera's x coordinate
 * @param y						Camera's y coordinate
 * @param message				Message to add to the output
 */
static void Debug_printClassSizes(Debug this __attribute__ ((unused)), ClassSizeData* classesSizeData, int count, int x, int y, char* message)
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
	for(; classesSizeData[i].classSizeFunction && (0 == count || i < count); i++)
	{
		Printing_text(Printing_getInstance(), classesSizeData[i].name, x, ++y, NULL);
		Printing_int(Printing_getInstance(), ((int (*)(void))classesSizeData[i].classSizeFunction)(), x + columnIncrement, y, NULL);
	}
}
#endif

/**
 * Show Game's profiling
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_gameProfilingPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug_removeSubPages(this);

	Game_showLastGameFrameProfiling(Game_getInstance(), x, y);
}

/**
 * Setup streaming's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_streamingPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_streamingShowStatus);
	this->currentSubPage = this->subPages->head;

	Debug_showSubPage(this, 0);
}

/**
 * Show State's streaming status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_streamingShowStatus(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Stage_showStreamingProfiling(GameState_getStage(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), x, y);
}

/**
 * Setup CHAR memory's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_charMemoryPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	this->currentSubPage = this->subPages->head;

	this->charSegment = -1;

	Debug_showSubPage(this, 0);
}

/**
 * Show CHAR memory's state
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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

/**
 * Show CHAR memory segments's state
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_charMemoryShowMemory(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	SpriteManager_showLayer(SpriteManager_getInstance(), 0);

	int i = 0;
	int yOffset = y + 3;

	// print box
	Printing_text(Printing_getInstance(), "\x03\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x04", 1, yOffset-1, NULL);
	Printing_text(Printing_getInstance(), "\x05\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x06", 1, yOffset+16, NULL);

	for(i = 0; i < __CHARS_PER_SEGMENT_TO_SHOW / __CHARS_PER_ROW_TO_SHOW && i < __SCREEN_HEIGHT / 8; i++)
	{
		Printing_text(Printing_getInstance(), "\x07                                \x07", 1, yOffset+i, NULL);
	}

	const HWORD charMemoryMap[] =
	{
		0,	1,	2,	3,	4,	5,	6,	7,
		8,	9,	10,	11,	12,	13,	14,	15,
		16,	17,	18,	19,	20,	21,	22,	23,
		24,	25,	26,	27,	28,	29,	30,	31
	};

	// put the map into memory calculating the number of char for each reference
	for(i = 0; i <  __CHARS_PER_SEGMENT_TO_SHOW / __CHARS_PER_ROW_TO_SHOW; i++)
	{
		Mem_addHWORD
		(
			(HWORD*)__BGMAP_SEGMENT(BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance())) + (((yOffset + i) * (64)) + 2),
			(HWORD*)charMemoryMap,
			__CHARS_PER_ROW_TO_SHOW,
			this->charSegment * __CHARS_PER_SEGMENT_TO_SHOW + i * __CHARS_PER_ROW_TO_SHOW
		);
	}
}

/**
 * Setup BGMAP memory's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_texturesPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_texturesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_texturesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->bgmapSegment = -1;
	this->mapDisplacement.x = 0;
	this->mapDisplacement.y = 0;

	Debug_showSubPage(this, 0);
}

/**
 * Show BGMAP memory's status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 */
static void Debug_showDebugBgmap(Debug this)
{
	if(this->currentPage->data != &Debug_texturesPage ||
		0 > this->bgmapSegment
	)
	{
		return;
	}

	SpriteManager_showLayer(SpriteManager_getInstance(), 0);
	Debug_showBgmapSegment(this);
}

/**
 * Force display BGMAP memory's status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 */
static void Debug_showBgmapSegment(Debug this)
{
	// write the head
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].head = __WORLD_ON | this->bgmapSegment;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].mx = this->mapDisplacement.x;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].mp = 0;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].my = this->mapDisplacement.y;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].gx = 0;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].gp = 3;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].gy = 0;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].w = __SCREEN_WIDTH;
	_worldAttributesBaseAddress[__TOTAL_LAYERS - 1].h = __SCREEN_HEIGHT;
}

/**
 * Setup BGMAP segment's status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_texturesShowStatus(Debug this, int increment, int x, int y)
{
	this->bgmapSegment += increment;

	if(-1 > this->bgmapSegment)
	{
		this->bgmapSegment = BgmapTextureManager_getAvailableBgmapSegmentsForTextures(BgmapTextureManager_getInstance()) - 1;
	}

	if(-1 == this->bgmapSegment)
	{
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		BgmapTextureManager_print(BgmapTextureManager_getInstance(), x, y);

		ParamTableManager_print(ParamTableManager_getInstance(), x + 26, y);
		Debug_dimmGame(this);
	}
	else if(BgmapTextureManager_getAvailableBgmapSegmentsForTextures(BgmapTextureManager_getInstance()) > this->bgmapSegment)
	{
		Printing_text(Printing_getInstance(), " \x1E\x1A\x1B\x1C\x1D\x1F\x1A\x1B\x1C\x1D ", 35, 0, NULL);
		Printing_text(Printing_getInstance(), "BGMAP TEXTURES' USAGE", x, y++, NULL);
		Printing_text(Printing_getInstance(), "Segment: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->bgmapSegment, x + 9, y, NULL);

		this->mapDisplacement.x = 0;
		this->mapDisplacement.y = 0;

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

/**
 * Setup OBJECT memory's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_objectsPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_objectsShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_objectsShowStatus);
	this->currentSubPage = this->subPages->head;

	this->objectSegment = -1;

	Debug_showSubPage(this, 0);
}

/**
 * Show OBJECT memory's status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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

/**
 * Setup WORLD memory's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_spritesPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	this->currentSubPage = this->subPages->head;

	this->currentLayer = __TOTAL_LAYERS;

	Debug_showSubPage(this, 0);
}

/**
 * Show WORLD memory's status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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
		SpriteManager_print(SpriteManager_getInstance(), x, y, false);
	}
	else if(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->currentLayer)
	{
		Sprite sprite = SpriteManager_getSpriteAtLayer(SpriteManager_getInstance(), this->currentLayer);

		SpriteManager_showLayer(SpriteManager_getInstance(), this->currentLayer);

		Printing_text(Printing_getInstance(), "SPRITES' USAGE", x, y++, NULL);
		Sprite_print(sprite, x, y);
	}
	else
	{
		this->currentLayer = __TOTAL_LAYERS;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		SpriteManager_print(SpriteManager_getInstance(), x, y, false);
		Debug_dimmGame(this);
	}
}

/**
 * Setup physics' status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_physicsPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowStatistics);
	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowShapes);
	this->currentSubPage = this->subPages->head;

	Debug_showSubPage(this, 0);
}

/**
 * Show physics' status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_physicStatusShowStatistics(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	PhysicalWorld_print(GameState_getPhysicalWorld(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), x, y);
	CollisionManager_print(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))), x, y + 6);
}

/**
 * Setup physics sub-pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_physicStatusShowShapes(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	Printing_text(Printing_getInstance(), "COLLISION SHAPES", x, y++, NULL);
	this->update = (void (*)(void *))&Debug_showCollisionShapes;
}

/**
 * Show collision boxes
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 */
static void Debug_showCollisionShapes(Debug this __attribute__ ((unused)))
{
	CollisionManager_showShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
}

/**
 * Setup hardware register's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_hardwareRegistersPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug_removeSubPages(this);

	HardwareManager_print(HardwareManager_getInstance(), 1, y);
}

/**
 * Setup SRAM's status pages
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
static void Debug_sramPage(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_showSramPage);
	VirtualList_pushBack(this->subPages, &Debug_showSramPage);
	this->currentSubPage = this->subPages->head;

	this->sramPage = 0;

	Debug_showSubPage(this, 0);
}

/**
 * Show SRAM's status
 *
 * @memberof			Debug
 * @private
 *
 * @param this			Function scope
 * @param increment		Increment
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
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
	Printing_text(Printing_getInstance(), __CHAR_BRIGHT_RED_BOX, 46, y - 15 + (this->sramPage / (totalPages / 16)), NULL);
}
