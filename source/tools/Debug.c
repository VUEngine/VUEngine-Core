/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifdef __DEBUG_TOOLS


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
#include <Printing.h>
#include <MiscStructs.h>
#include <MBgmapSprite.h>
#include <BgmapAnimationCoordinator.h>
#include <ObjectAnimationCoordinator.h>
#include <KeyPadManager.h>

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
#include <Shape.h>
#include <Polygon.h>

#include <Container.h>
#include <Entity.h>
#include <InGameEntity.h>
#include <AnimatedInGameEntity.h>
#include <InanimatedInGameEntity.h>
#include <Actor.h>
#include <Image.h>
#include <ManagedEntity.h>
#include <MBackground.h>
#include <Particle.h>
#include <ParticleSystem.h>

#include <GameState.h>
#include <Stage.h>
#include <UI.h>
#include <Mem.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
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
        /* window to look into bgmap memory */															\
        VBVec2D bgmapDisplacement;																		\
        /* update function pointer */																	\
        void (*update)(void *);																			\

// define the Debug
__CLASS_DEFINITION(Debug, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
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
static void Debug_showCharMemoryStatus(Debug this, int increment, int x, int y);
static void Debug_showTextureStatus(Debug this, int increment, int x, int y);
static void Debug_showObjectStatus(Debug this, int increment, int x, int y);
static void Debug_showSpritesStatus(Debug this, int increment, int x, int y);
static void Debug_showPhysicsStatus(Debug this, int increment, int x, int y);
static void Debug_showHardwareStatus(Debug this, int increment, int x, int y);

// sub pages
static void Debug_spritesShowStatus(Debug this, int increment, int x, int y);
static void Debug_textutesShowStatus(Debug this, int increment, int x, int y);
static void Debug_objectsShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowMemory(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowStatistics(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowShapes(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFirstPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowSecondPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowThirdPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowFourthPage(Debug this, int increment, int x, int y);
static void Debug_memoryStatusShowUserDefinedClassesSizes(Debug this, int increment, int x, int y);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
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

	this->update = NULL;

	this->bgmapDisplacement.x = 0;
	this->bgmapDisplacement.y = 0;

	Debug_setupPages(this);
}

// class's destructor
void Debug_destructor(Debug this)
{
	ASSERT(this, "Debug::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// setup pages
static void Debug_setupPages(Debug this)
{
	VirtualList_pushBack(this->pages, &Debug_showGeneralStatus);
	VirtualList_pushBack(this->pages, &Debug_showMemoryStatus);
	VirtualList_pushBack(this->pages, &Debug_showSpritesStatus);
	VirtualList_pushBack(this->pages, &Debug_showTextureStatus);
	VirtualList_pushBack(this->pages, &Debug_showObjectStatus);
	VirtualList_pushBack(this->pages, &Debug_showCharMemoryStatus);
	VirtualList_pushBack(this->pages, &Debug_showPhysicsStatus);
	VirtualList_pushBack(this->pages, &Debug_showHardwareStatus);

	this->currentPage = this->pages->head;
}

static void Debug_dimmGame(Debug this __attribute__ ((unused)))
{
	_vipRegisters[__GPLT0] = 0x50;
	_vipRegisters[__GPLT1] = 0x50;
	_vipRegisters[__GPLT2] = 0x54;
	_vipRegisters[__GPLT3] = 0x54;
	_vipRegisters[__JPLT0] = 0x54;
	_vipRegisters[__JPLT1] = 0x54;
	_vipRegisters[__JPLT2] = 0x54;
	_vipRegisters[__JPLT3] = 0x54;

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
	CollisionManager_flushShapesDirectDrawData(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
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
    Printing_text(Printing_getInstance(), "  /  ", 16, 0, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getNodePosition(this->pages, this->currentPage) + 1, 17, 0, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getSize(this->pages), 19, 0, NULL);
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

#define DISPLACEMENT_STEP_X	512 - 384
#define DISPLACEMENT_STEP_Y	512 - 224

// displace view to the left
void Debug_diplaceLeft(Debug this)
{
	this->bgmapDisplacement.x = 0;
	Debug_showDebugBgmap(this);
}

// displace view to the right
void Debug_diplaceRight(Debug this)
{
	this->bgmapDisplacement.x = DISPLACEMENT_STEP_X;
	Debug_showDebugBgmap(this);
}

// displace view up
void Debug_diplaceUp(Debug this)
{
	this->bgmapDisplacement.y = 0;
	Debug_showDebugBgmap(this);
}

// displace view down
void Debug_diplaceDown(Debug this)
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
	Printing_text(Printing_getInstance(), "GENERAL STATUS", 1, y++, NULL);
	Printing_text(Printing_getInstance(), "General clock time: ", 1, ++y, NULL);
	Clock_print(Game_getClock(Game_getInstance()), 26, y, NULL);
	Printing_text(Printing_getInstance(), "Animations clock's time: ", 1, ++y, NULL);
	Clock_print(Game_getAnimationsClock(Game_getInstance()), 26, y, NULL);
	Printing_text(Printing_getInstance(), "Physics clock's time: ", 1, ++y, NULL);
	Clock_print(Game_getPhysicsClock(Game_getInstance()), 26, y, NULL);
	FrameRate_printLastCount(FrameRate_getInstance(), 1, y + 3);

	Printing_text(Printing_getInstance(), "STAGE STATUS", 20, y + 3, NULL);
	Printing_text(Printing_getInstance(), "Entities: ", 20, ++y + 3, NULL);
	Printing_int(Printing_getInstance(), Container_getChildCount(__SAFE_CAST(Container, GameState_getStage(this->gameState))), 34, y + 3, NULL);
	Printing_text(Printing_getInstance(), "UI Entities: ", 20, ++y + 3, NULL);

	UI ui = Stage_getUI(GameState_getStage(this->gameState));
	Printing_int(Printing_getInstance(), ui ? Container_getChildCount(__SAFE_CAST(Container, ui)) : 0, 34, y + 3, NULL);
}

static void Debug_showMemoryStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFirstPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSecondPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowThirdPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFourthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowUserDefinedClassesSizes);

	this->currentSubPage = this->subPages->head;

	Debug_showSubPage(this, 0);
}


static void Debug_memoryStatusShowFirstPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Clock_getObjectSize, "Clock"},
		{&Object_getObjectSize, "Object"},
		{&State_getObjectSize, "State"},
		{&StateMachine_getObjectSize, "StateMachine"},
		{&Telegram_getObjectSize, "Telegram"},
		{&VirtualList_getObjectSize, "VirtualList"},
		{&VirtualNode_getObjectSize, "VirtualNode"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VBJaEngine classes:");
}

static void Debug_memoryStatusShowSecondPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&CharSet_getObjectSize, "CharSet"},
		{&Texture_getObjectSize, "Texture"},
		{&Sprite_getObjectSize, "Sprite"},
		{&BgmapTexture_getObjectSize, "BgmapTexture"},
		{&BgmapSprite_getObjectSize, "BgmapSprite"},
		{&MBgmapSprite_getObjectSize, "MBgmapSprite"},
		{&BgmapAnimatedSprite_getObjectSize, "BgmapAnim. Sprite"},
		{&BgmapAnimationCoordinator_getObjectSize, "BgmapAnim. Coord."},
		{&ObjectTexture_getObjectSize, "ObjectTexture"},
		{&ObjectSprite_getObjectSize, "ObjectSprite"},
		{&ObjectSpriteContainer_getObjectSize, "ObjectSpriteCont."},
		{&ObjectAnimatedSprite_getObjectSize, "ObjectAnim. Sprite"},
		{&ObjectAnimationCoordinator_getObjectSize, "ObjectAnim.Coord."},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VBJaEngine classes:");
}

static void Debug_memoryStatusShowThirdPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&SpatialObject_getObjectSize, "SpatialObject"},
		{&Body_getObjectSize, "Body"},
		{&CollisionSolver_getObjectSize, "CollisionSolver"},
		{&Circle_getObjectSize, "Circle"},
		{&Cuboid_getObjectSize, "Cuboid"},
		{&Shape_getObjectSize, "Shape"},
		{&Polygon_getObjectSize, "Polygon"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VBJaEngine classes:");
}

static void Debug_memoryStatusShowFourthPage(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

//	typedef int (*int (*)(Object))(Object this);

	ClassSizeData classesSizeData[] =
	{
		{&Container_getObjectSize, "Container"},
		{&Entity_getObjectSize, "Entity"},
		{&Image_getObjectSize, "Image"},
		{&ManagedEntity_getObjectSize, "ManagedEntity"},
		{&MBackground_getObjectSize, "MBackground"},
		{&InGameEntity_getObjectSize, "InGameEntity"},
		{&InanimatedInGameEntity_getObjectSize, "Inanim. InGam. Ent."},
		{&AnimatedInGameEntity_getObjectSize, "Anim. InGameEntity"},
		{&Actor_getObjectSize, "Actor"},
		{&Particle_getObjectSize, "Particle"},
		{&ParticleSystem_getObjectSize, "ParticleSystem"},
		{&GameState_getObjectSize, "GameState"},
		{&GameState_getObjectSize, "Stage"},
	};

	Debug_printClassSizes(classesSizeData, sizeof(classesSizeData) / sizeof(ClassSizeData), x + 21, y, "VBJaEngine classes:");
}

static void Debug_memoryStatusShowUserDefinedClassesSizes(Debug this __attribute__ ((unused)), int increment __attribute__ ((unused)), int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	Debug_printClassSizes(_userClassesSizeData, 0, x + 21, y, "User defined classes:");
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

	if(-1 > this->charSegment)
	{
		this->charSegment = __CHAR_SEGMENTS - 1;
	}

	if(-1 == this->charSegment)
	{
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		CharSetManager_print(CharSetManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
	else if(__CHAR_SEGMENTS > this->charSegment)
	{
		Printing_text(Printing_getInstance(), "CHAR MEMORY'S USAGE", x, y++, NULL);
		Printing_text(Printing_getInstance(), "Segment: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->charSegment, x + 16, y, NULL);
		Printing_text(Printing_getInstance(), "Total CharSets: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), CharSetManager_getTotalCharSets(CharSetManager_getInstance(), this->charSegment), x + 16, y, NULL);
		Printing_text(Printing_getInstance(), "Used chars: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), CharSetManager_getTotalUsedChars(CharSetManager_getInstance(), this->charSegment), x + 16, y, NULL);
		Printing_text(Printing_getInstance(), "Free chars: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), CharSetManager_getTotalFreeChars(CharSetManager_getInstance(), this->charSegment), x + 16, y, NULL);

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

	int yOffset = y + 7;

	// each char segment has 512 slots
	BYTE CHAR_MEMORY_MP[ __CHAR_SEGMENT_TOTAL_CHARS];

	int i = 0;
	int j = 0;
	for(; i <  __CHAR_SEGMENT_TOTAL_CHARS; i+= 2, j++)
	{
		CHAR_MEMORY_MP[i] = (BYTE)(j & 0xFF);
		CHAR_MEMORY_MP[i + 1] = (BYTE)((j & 0xFF00) >> 8);
	}

	// put the map into memory calculating the number of char for each reference
	for(i = 0; i <  __CHAR_SEGMENT_TOTAL_CHARS / 48; i++)
	{
		Mem_add((u8*)__BGMAP_SEGMENT(BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance())) + (((yOffset << 6) + (i << 6)) << 1),
				(u8*)CHAR_MEMORY_MP,
				__SCREEN_WIDTH >> 3,
				(this->charSegment << 9) + i * (__SCREEN_WIDTH >> 3));
	}
}

static void Debug_showTextureStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int  __attribute__ ((unused))y)
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_textutesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_textutesShowStatus);
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

static void Debug_textutesShowStatus(Debug this, int increment, int x, int y)
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

		ParamTableManager_print(ParamTableManager_getInstance(), x, y + 8);
		Debug_dimmGame(this);
	}
	else if(BgmapTextureManager_getAvailableBgmapSegments(BgmapTextureManager_getInstance()) > this->bgmapSegment)
	{
		Printing_text(Printing_getInstance(), " \x1E\x1A\x1B\x1C\x1D\x1F\x1A\x1B\x1C\x1D ", 35, 0, NULL);
		Printing_text(Printing_getInstance(), "BGMAP TEXTURES' USAGE", x, y++, NULL);
		Printing_text(Printing_getInstance(), "Segment: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->bgmapSegment, x + 9, y, NULL);

		SpriteManager_showLayer(SpriteManager_getInstance(), 0);

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
		ParamTableManager_print(ParamTableManager_getInstance(), x, y + 8);
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

		this->bgmapDisplacement.x = 0;
		this->bgmapDisplacement.y = 0;

		Debug_showDebugBgmap(this);
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

		Printing_text(Printing_getInstance(), "SPRITES' USAGE", x, y++, NULL);
		Printing_text(Printing_getInstance(), "Layer: ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), this->currentLayer, x + 10, y, NULL);
		Printing_text(Printing_getInstance(), "Class: ", x, ++y, NULL);
		Printing_text(Printing_getInstance(), __GET_CLASS_NAME_UNSAFE(sprite), x + 10, y, NULL);
		Printing_text(Printing_getInstance(), "Position:                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL(Sprite, getPosition, sprite).x), x + 10, y, NULL);
		Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL(Sprite, getPosition, sprite).y), x + 20, y, NULL);
		Printing_float(Printing_getInstance(), FIX19_13TOF(__VIRTUAL_CALL(Sprite, getPosition, sprite).z + Sprite_getDisplacement(sprite).z), x + 30, y, NULL);
		Printing_text(Printing_getInstance(), "WORLD (x, y):                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldX(sprite), x + 15, y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldY(sprite), x + 25, y, NULL);
		Printing_text(Printing_getInstance(), "Size (w, h):                         ", x, ++y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldWidth(sprite), x + 15, y, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldHeight(sprite), x + 25, y, NULL);
		SpriteManager_showLayer(SpriteManager_getInstance(), this->currentLayer);
		Debug_lightUpGame(this);
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
	CollisionManager_drawShapes(GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance())))));
}

static void Debug_showHardwareStatus(Debug this, int increment __attribute__ ((unused)), int x __attribute__ ((unused)), int y)
{
	Debug_removeSubPages(this);

	HardwareManager_print(HardwareManager_getInstance(), 1, y);
}

static void Debug_printClassSizes(ClassSizeData* classesSizeData, int size, int x, int y, char* message)
{
	int columnIncrement = 20;

	Printing_text(Printing_getInstance(), "CLASSES MEMORY USAGE (B) ", x, y++, NULL);

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
