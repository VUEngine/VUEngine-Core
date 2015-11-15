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
#include <VPUManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Printing.h>
#include <MiscStructs.h>
#include <MBgmapSprite.h>
#include <BgmapAnimationCoordinator.h>
#include <ObjectAnimationCoordinator.h>

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

#include <ScrollBackground.h>
#include <GameState.h>
#include <Stage.h>
#include <UI.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Debug_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* current in game state */													\
	GameState gameState;														\
																				\
	/* pages */																	\
	VirtualList pages;															\
																				\
	/* sub pages */																\
	VirtualList subPages;														\
																				\
	/* current page */															\
	VirtualNode currentPage;													\
																				\
	/* current subb page */														\
	VirtualNode currentSubPage;													\
																				\
	/* current layer */															\
	u8 currentLayer;															\
																				\
	/* current bgmap */															\
	int bgmapSegment;															\
																				\
	/* current obj segment */													\
	int objectSegment;															\
																				\
	/* current char segment */													\
	int charSegment;															\
																				\
	/* window to look into bgmap memory */										\
	VBVec2D bgmapDisplacement;													\
																				\
	/* update function pointer */												\
	void (*update)(void *);														\

// define the Debug
__CLASS_DEFINITION(Debug, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern ClassSizeData _userClassesSizeData[];

// class's constructor
static void Debug_constructor(Debug this);

// setup pages
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

static void Debug_printClassSizes(ClassSizeData* classesSizeData, int size, int x, int y, char* message);
static void Debug_showCollisionShapes(Debug this);
static void Debug_showDebugBgmap(Debug this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(Debug);

// class's constructor
static void Debug_constructor(Debug this)
{
	ASSERT(this, "Debug::constructor: null this");

	__CONSTRUCT_BASE();

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

	this->currentPage = VirtualList_begin(this->pages);
}

static void Debug_dimmGame(Debug this)
{
	VIP_REGS[GPLT0] = 0b01010100;
	VIP_REGS[GPLT1] = 0b01010000;
	VIP_REGS[GPLT2] = 0b01010100;
	VIP_REGS[GPLT3] = 0xE0;
	VIP_REGS[JPLT0] = 0b01010100;
	VIP_REGS[JPLT1] = 0b01010100;
	VIP_REGS[JPLT2] = 0b01010100;
	VIP_REGS[JPLT3] = 0b01010100;
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
	VPUManager_clearBgmap(VPUManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());

	this->gameState = gameState;

	Debug_dimmGame(this);
	Debug_showPage(this, 0);
}

// hide debug screens
void Debug_hide(Debug this)
{
	CollisionManager_flushShapesDirectDrawData(CollisionManager_getInstance());
	VPUManager_clearBgmap(VPUManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());
	Debug_lightUpGame(this);
}

// show previous page
void Debug_showPreviousPage(Debug this)
{
	this->currentPage = VirtualNode_getPrevious(this->currentPage);

	if(NULL == this->currentPage)
	{
		this->currentPage = VirtualList_end(this->pages);
	}

	Debug_showPage(this, -1);
}

// show next page
void Debug_showNextPage(Debug this)
{
	this->currentPage = VirtualNode_getNext(this->currentPage);

	if(NULL == this->currentPage)
	{
		this->currentPage = VirtualList_begin(this->pages);
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
		this->currentSubPage = VirtualList_end(this->subPages);
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

	this->currentSubPage = VirtualNode_getNext(this->currentSubPage);

	if(NULL == this->currentSubPage)
	{
		this->currentSubPage = VirtualList_begin(this->subPages);
	}

	Debug_showSubPage(this, 1);
}

// print header
static void Debug_printHeader(Debug this)
{
	Printing_text(Printing_getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
    Printing_text(Printing_getInstance(), " DEBUG SYSTEM ", 1, 0, NULL);
    Printing_text(Printing_getInstance(), "  /  ", 16, 0, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getNodePosition(this->pages, VirtualNode_getData(this->currentPage)) + 1, 17, 0, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getSize(this->pages), 19, 0, NULL);
}

// show page
static void Debug_showPage(Debug this, int increment)
{
	if(this->currentPage && VirtualNode_getData(this->currentPage))
	{
		this->update = NULL;

		VPUManager_clearBgmap(VPUManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);
		SpriteManager_recoverLayers(SpriteManager_getInstance());

        Debug_printHeader(this);
		Printing_text(Printing_getInstance(), " \x1E\x1C\x1D ", 42, 0, NULL);

		Debug_dimmGame(this);

		((void (*)(Debug, int, int, int))VirtualNode_getData(this->currentPage))(this, increment, 1, 2);
	}
}

// show sub page
static void Debug_showSubPage(Debug this, int increment)
{
	if(this->currentSubPage && VirtualNode_getData(this->currentSubPage))
	{
		this->update = NULL;

		VPUManager_clearBgmap(VPUManager_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()), __PRINTABLE_BGMAP_AREA);

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

static void Debug_showGeneralStatus(Debug this, int increment, int x, int y)
{
	Debug_removeSubPages(this);
	Printing_text(Printing_getInstance(), "GENERAL STATUS", 1, y++, NULL);
	Printing_text(Printing_getInstance(), "General clock time: ", 1, ++y, NULL);
	Clock_print(Game_getClock(Game_getInstance()), 21, y, NULL);
	Printing_text(Printing_getInstance(), "In game clock time: ", 1, ++y, NULL);
	Clock_print(Game_getInGameClock(Game_getInstance()), 21, y, NULL);
	FrameRate_printLastRecord(FrameRate_getInstance(), 1, y + 3);

	Printing_text(Printing_getInstance(), "STAGE STATUS", 20, y + 3, NULL);
	Printing_text(Printing_getInstance(), "Entities: ", 20, ++y + 3, NULL);
	Printing_int(Printing_getInstance(), Container_getChildCount(__GET_CAST(Container, GameState_getStage(this->gameState))), 34, y + 3, NULL);
	Printing_text(Printing_getInstance(), "UI Entities: ", 20, ++y + 3, NULL);

	UI ui = Stage_getUI(GameState_getStage(this->gameState));
	Printing_int(Printing_getInstance(), ui ? Container_getChildCount(__GET_CAST(Container, ui)) : 0, 34, y + 3, NULL);
}

static void Debug_showMemoryStatus(Debug this, int increment, int x, int y)
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFirstPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowSecondPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowThirdPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowFourthPage);
	VirtualList_pushBack(this->subPages, &Debug_memoryStatusShowUserDefinedClassesSizes);

	this->currentSubPage = VirtualList_begin(this->subPages);

	Debug_showSubPage(this, 0);
}


static void Debug_memoryStatusShowFirstPage(Debug this, int increment, int x, int y)
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

static void Debug_memoryStatusShowSecondPage(Debug this, int increment, int x, int y)
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

static void Debug_memoryStatusShowThirdPage(Debug this, int increment, int x, int y)
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

static void Debug_memoryStatusShowFourthPage(Debug this, int increment, int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	ClassSizeData classesSizeData[] =
	{
		{&Container_getObjectSize, "Container"},
		{&Entity_getObjectSize, "Entity"},
		{&Image_getObjectSize, "Image"},
		{&ManagedEntity_getObjectSize, "ManagedEntity"},
		{&MBackground_getObjectSize, "MBackground"},
		{&InGameEntity_getObjectSize, "InGameEntity"},
		{&ScrollBackground_getObjectSize, "ScrollBackg."},
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

static void Debug_memoryStatusShowUserDefinedClassesSizes(Debug this, int increment, int x, int y)
{
	MemoryPool_printDetailedUsage(MemoryPool_getInstance(), x, y);

	Debug_printClassSizes(_userClassesSizeData, 0, x + 21, y, "User defined classes:");
}

static void Debug_showCharMemoryStatus(Debug this, int increment, int x, int y)
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	this->currentSubPage = VirtualList_begin(this->subPages);

	this->charSegment = -1;

	Debug_showSubPage(this, 0);
}

static void Debug_charMemoryShowStatus(Debug this, int increment, int x, int y)
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

static void Debug_charMemoryShowMemory(Debug this, int increment, int x, int y)
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

	//put the map into memory calculating the number of char for each reference
	for(i = 0; i <  __CHAR_SEGMENT_TOTAL_CHARS / 48; i++)
	{
		Mem_add((u8*)BGMap(BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance())) + (((yOffset << 6) + (i << 6)) << 1),
				(const u8*)CHAR_MEMORY_MP,
				__SCREEN_WIDTH >> 3,
				(this->charSegment << 9) + i * (__SCREEN_WIDTH >> 3));
	}
}

static void Debug_showTextureStatus(Debug this, int increment, int x, int y)
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_textutesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_textutesShowStatus);
	this->currentSubPage = VirtualList_begin(this->subPages);

	this->bgmapSegment = -1;
	this->bgmapDisplacement.x = 0;
	this->bgmapDisplacement.y = 0;

	Debug_showSubPage(this, 0);
}

static void Debug_showDebugBgmap(Debug this)
{
	if(VirtualNode_getData(this->currentPage) != &Debug_showTextureStatus ||
		0 > this->bgmapSegment
	)
	{
		return;
	}

	// write the head
	
	WA[__TOTAL_LAYERS - 1].head = WRLD_ON | this->bgmapSegment;
	WA[__TOTAL_LAYERS - 1].mx = this->bgmapDisplacement.x;
	WA[__TOTAL_LAYERS - 1].mp = 0;
	WA[__TOTAL_LAYERS - 1].my = this->bgmapDisplacement.y;
	WA[__TOTAL_LAYERS - 1].gx = 0;
	WA[__TOTAL_LAYERS - 1].gp = 3;
	WA[__TOTAL_LAYERS - 1].gy = 0;
	WA[__TOTAL_LAYERS - 1].w = __SCREEN_WIDTH;
	WA[__TOTAL_LAYERS - 1].h = __SCREEN_HEIGHT;
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

static void Debug_showObjectStatus(Debug this, int increment, int x, int y)
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_objectsShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_objectsShowStatus);
	this->currentSubPage = VirtualList_begin(this->subPages);

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
		SpriteManager_showLayer(SpriteManager_getInstance(), Sprite_getWorldLayer(__GET_CAST(Sprite, objectSpriteContainer)));

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

static void Debug_showSpritesStatus(Debug this, int increment, int x, int y)
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	this->currentSubPage = VirtualList_begin(this->subPages);

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
		Printing_text(Printing_getInstance(), __GET_CLASS_NAME(sprite), x + 10, y, NULL);
		Printing_text(Printing_getInstance(), "Position:", x, ++y, NULL);
		Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL_UNSAFE(const VBVec3D*, Sprite, getPosition, sprite)->x), x + 10, y, NULL);
		Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL_UNSAFE(const VBVec3D*, Sprite, getPosition, sprite)->y), x + 14, y, NULL);
		Printing_int(Printing_getInstance(), FIX19_13TOI(__VIRTUAL_CALL_UNSAFE(const VBVec3D*, Sprite, getPosition, sprite)->z), x + 18, y, NULL);
		SpriteManager_showLayer(SpriteManager_getInstance(), this->currentLayer);
	}
	else
	{
		this->currentLayer = __TOTAL_LAYERS;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		SpriteManager_print(SpriteManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
}

static void Debug_showPhysicsStatus(Debug this, int increment, int x, int y)
{
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowStatistics);
	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowShapes);
	this->currentSubPage = VirtualList_begin(this->subPages);

	Debug_showSubPage(this, 0);
}

static void Debug_physicStatusShowStatistics(Debug this, int increment, int x, int y)
{
	PhysicalWorld_print(PhysicalWorld_getInstance(), x, y);
	CollisionManager_print(CollisionManager_getInstance(), x, y + 6);
}

static void Debug_physicStatusShowShapes(Debug this, int increment, int x, int y)
{
	Printing_text(Printing_getInstance(), "COLLISION SHAPES", x, y++, NULL);
	this->update = (void (*)(void *))&Debug_showCollisionShapes;
}

static void Debug_showCollisionShapes(Debug this)
{
	CollisionManager_drawShapes(CollisionManager_getInstance());
}

static void Debug_showHardwareStatus(Debug this, int increment, int x, int y)
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
