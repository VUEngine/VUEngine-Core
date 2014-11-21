/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


#ifdef __DEBUG_TOOLS

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Debug.h>
#include <Game.h>
#include <MemoryPool.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <MiscStructs.h>
#include <Globals.h>
#include <FrameRate.h>
#include <Clock.h>
#include <TextureManager.h>
#include <Level.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <SoundManager.h>
#include <StateMachine.h>
#include <Screen.h>
#include <Background.h>
#include <Image.h>
#include <VPUManager.h>
#include <Printing.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define Debug_ATTRIBUTES					\
											\
	/* super's attributes */				\
	Object_ATTRIBUTES;						\
											\
	/* pages */								\
	VirtualList pages;						\
											\
	/* sub pages */							\
	VirtualList subPages;					\
											\
	/* current page */						\
	VirtualNode currentPage;				\
											\
	/* current subb page */					\
	VirtualNode currentSubPage;				\
											\
	/* current layer */						\
	int currentLayer;						\
											\
	/* current bgmap */						\
	int currentBgmap;						\
											\
	/* current char segment */				\
	int charSeg;							\
											\
	/* window to look into bgmap memory */	\
	VBVec2D bgmapDisplacement;				\
											\
	/* update function pointer */			\
	void (*update)(void *);


// define the Debug
__CLASS_DEFINITION(Debug);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												  MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

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
static void Debug_showSpritesStatus(Debug this, int increment, int x, int y);
static void Debug_showPhysicsStatus(Debug this, int increment, int x, int y); 
static void Debug_showHardwareStatus(Debug this, int increment, int x, int y);

// sub pages
static void Debug_spritesShowStatus(Debug this, int increment, int x, int y);
static void Debug_textutesShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowStatus(Debug this, int increment, int x, int y);
static void Debug_charMemoryShowMemory(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowStatistics(Debug this, int increment, int x, int y);
static void Debug_physicStatusShowShapes(Debug this, int increment, int x, int y);

static void Debug_printClassSizes(int x, int y);
static void Debug_showCollisionShapes(Debug this);
static void Debug_showDebugBgmap(Debug this);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
__SINGLETON(Debug);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Debug_constructor(Debug this){

	ASSERT(this, "Debug::constructor: null this");

	__CONSTRUCT_BASE(Object);
	
	this->pages = __NEW(VirtualList);
	this->subPages = __NEW(VirtualList);
	this->currentPage = NULL;
	this->currentSubPage = NULL;
	
	this->currentLayer = __TOTAL_LAYERS;
	this->currentBgmap = 0;
	this->charSeg = 0;
	
	this->update = NULL;
	
	this->bgmapDisplacement.x = 0;
	this->bgmapDisplacement.y = 0;
	
	Debug_setupPages(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Debug_destructor(Debug this){
	
	ASSERT(this, "Debug::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup pages
static void Debug_setupPages(Debug this){
	
	VirtualList_pushBack(this->pages, &Debug_showGeneralStatus);
	VirtualList_pushBack(this->pages, &Debug_showMemoryStatus);
	VirtualList_pushBack(this->pages, &Debug_showSpritesStatus);
	VirtualList_pushBack(this->pages, &Debug_showTextureStatus);
	VirtualList_pushBack(this->pages, &Debug_showCharMemoryStatus);
	VirtualList_pushBack(this->pages, &Debug_showPhysicsStatus);
	VirtualList_pushBack(this->pages, &Debug_showHardwareStatus);

	this->currentPage = VirtualList_begin(this->pages);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_dimmGame(Debug this){
	
	VIP_REGS[GPLT0] = 0b01010100;	/* Set all eight palettes to: 11100100 */
	VIP_REGS[GPLT1] = 0b01010000;	/* (i.e. "Normal" dark to light progression.) */
	VIP_REGS[GPLT2] = 0b01010100;
	VIP_REGS[GPLT3] = __GPLT3VALUE;
	VIP_REGS[JPLT0] = __JPLT0VALUE;
	VIP_REGS[JPLT1] = __JPLT1VALUE;
	VIP_REGS[JPLT2] = __JPLT2VALUE;
	VIP_REGS[JPLT3] = __JPLT3VALUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_lightUpGame(Debug this){
	
	//VPUManager_setupPalettes(VPUManager_getInstance());
	VIP_REGS[GPLT0] = __GPLT0VALUE;
	VIP_REGS[GPLT1] = __GPLT1VALUE;
	VIP_REGS[GPLT2] = __GPLT2VALUE;
	VIP_REGS[GPLT3] = __GPLT3VALUE;
	VIP_REGS[JPLT0] = __JPLT0VALUE;
	VIP_REGS[JPLT1] = __JPLT1VALUE;
	VIP_REGS[JPLT2] = __JPLT2VALUE;
	VIP_REGS[JPLT3] = __JPLT3VALUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update
void Debug_update(Debug this){
	
	if(this->update) {
		
		((void (*)(Debug))this->update)(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show debug screens
void Debug_show(Debug this){
	
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());

	Debug_dimmGame(this);
	Debug_showPage(this, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// hide debug screens
void Debug_hide(Debug this){

	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
	SpriteManager_recoverLayers(SpriteManager_getInstance());
	Debug_lightUpGame(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show previous page
void Debug_showPreviousPage(Debug this){

	this->currentPage = VirtualNode_getPrevious(this->currentPage);
	
	if(NULL == this->currentPage){
		
		this->currentPage = VirtualList_end(this->pages);
	}

	Debug_showPage(this, -1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show next page
void Debug_showNextPage(Debug this){

	this->currentPage = VirtualNode_getNext(this->currentPage);
	
	if(NULL == this->currentPage){
		
		this->currentPage = VirtualList_begin(this->pages);
	}

	Debug_showPage(this, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show previous sub page
void Debug_showPreviousSubPage(Debug this){

	if(!this->currentSubPage) {

		return;
	}
	
	this->currentSubPage = VirtualNode_getPrevious(this->currentSubPage);
	
	if(NULL == this->currentSubPage){
		
		this->currentSubPage = VirtualList_end(this->subPages);
	}

	Debug_showSubPage(this, -1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show next sub page
void Debug_showNextSubPage(Debug this){
	
	if(!this->currentSubPage) {

		return;
	}
	
	this->currentSubPage = VirtualNode_getNext(this->currentSubPage);
	
	if(NULL == this->currentSubPage){
		
		this->currentSubPage = VirtualList_begin(this->subPages);
	}

	Debug_showSubPage(this, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show page 
static void Debug_showPage(Debug this, int increment) {
	
	if(this->currentPage && VirtualNode_getData(this->currentPage)) {
		
		this->update = NULL;
		VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		Printing_text("DEBUG SYSTEM", 17, 0);
		Printing_text("Use(left/right)", 33, 1);
		Debug_dimmGame(this);
		((void (*)(Debug, int, int, int))VirtualNode_getData(this->currentPage))(this, increment, 1, 3);		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show sub page 
static void Debug_showSubPage(Debug this, int increment) {
	
	if(this->currentSubPage && VirtualNode_getData(this->currentSubPage)) {
		
		this->update = NULL;
		VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
		Printing_text("DEBUG SYSTEM", 17, 0);
		Printing_text("Use(left/right)", 33, 1);
		Printing_text("Use(up/down)", 33, 2);
		((void (*)(Debug, int, int, int))VirtualNode_getData(this->currentSubPage))(this, increment, 1, 3);		
	}
}

#define DISPLACEMENT_STEP_X	512 - 384
#define DISPLACEMENT_STEP_Y	512 - 224

// displace view to the left
void Debug_diplaceLeft(Debug this){
	
	this->bgmapDisplacement.x = 0;
	Debug_showDebugBgmap(this);
}

// displace view to the right
void Debug_diplaceRight(Debug this){
	
	this->bgmapDisplacement.x = DISPLACEMENT_STEP_X;
	Debug_showDebugBgmap(this);
}

// displace view up
void Debug_diplaceUp(Debug this){

	this->bgmapDisplacement.y = 0;
	Debug_showDebugBgmap(this);
}

// displace view down
void Debug_diplaceDown(Debug this){
	
	this->bgmapDisplacement.y = DISPLACEMENT_STEP_Y;
	Debug_showDebugBgmap(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_removeSubPages(Debug this) {
	
	VirtualList_clear(this->subPages);
	this->currentSubPage = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showGeneralStatus(Debug this, int increment, int x, int y) {
	
	Debug_removeSubPages(this);
	Printing_text("GENERAL STATUS", 1, y++);
	Printing_text("General clock's time: ", 1, ++y);
	Clock_print(Game_getClock(Game_getInstance()), 23, y);
	Printing_text("In game clock's time: ", 1, ++y);
	Clock_print(Game_getInGameClock(Game_getInstance()), 23, y);
	
	FrameRate_print(FrameRate_getInstance(), 1, y + 3);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showMemoryStatus(Debug this, int increment, int x, int y) {
	
	Debug_removeSubPages(this);

	MemoryPool_printMemUsage(MemoryPool_getInstance(), x, y);
	Debug_printClassSizes(x + 21, y);
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showCharMemoryStatus(Debug this, int increment, int x, int y) {

	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_charMemoryShowStatus);
	this->currentSubPage = VirtualList_begin(this->subPages);
	
	this->charSeg = -1;

	Debug_showSubPage(this, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_charMemoryShowStatus(Debug this, int increment, int x, int y) {
	
	this->charSeg += increment;

	if(-1 > this->charSeg) {

		this->charSeg = __CHAR_SEGMENTS - 1;
	}
	
	if(-1 == this->charSeg) {

		SpriteManager_recoverLayers(SpriteManager_getInstance());
		CharSetManager_print(CharSetManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
	else if(__CHAR_SEGMENTS > this->charSeg) {
	
		Printing_text("CHAR MEMORY'S USAGE", x, y++);
		Printing_text("Char segment: ", x, ++y);
		Printing_int(this->charSeg + 1, x + 14, y);

		Debug_charMemoryShowMemory(this, increment, x, y);
		Debug_lightUpGame(this);
	}
	else {

		this->charSeg = -1;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		CharSetManager_print(CharSetManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_charMemoryShowMemory(Debug this, int increment, int x, int y) {

	SpriteManager_showLayer(SpriteManager_getInstance(), 0);

	int yOffset = y + 7;
	
	// each char segment has 512 slots
	BYTE CHAR_MEMORY_MP[ __CHAR_SEGMENT_TOTAL_CHARS]; 
	
	int i = 0;
	int j = 0;
	for (; i <  __CHAR_SEGMENT_TOTAL_CHARS; i+= 2, j++) {
		
		CHAR_MEMORY_MP[i] = (BYTE)(j & 0xFF);
		CHAR_MEMORY_MP[i + 1] = (BYTE)((j & 0xFF00) >> 8);
	}

	//put the map into memory calculating the number of char for each reference
	for(i = 0; i <  __CHAR_SEGMENT_TOTAL_CHARS / 64; i++){
	
		Mem_add((u8*)BGMap(__PRINTING_BGMAP) + (((yOffset << 6) + (i << 6)) << 1),
				(const u8*)CHAR_MEMORY_MP, 
				__SCREEN_WIDTH >> 3,
				(this->charSeg << 9) + i * (__SCREEN_WIDTH >> 3));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showTextureStatus(Debug this, int increment, int x, int y) {

	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_textutesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_textutesShowStatus);
	this->currentSubPage = VirtualList_begin(this->subPages);
	
	this->currentBgmap = -1;
	this->bgmapDisplacement.x = 0;
	this->bgmapDisplacement.y = 0;

	Debug_showSubPage(this, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showDebugBgmap(Debug this){

	if(VirtualNode_getData(this->currentPage) != &Debug_showTextureStatus ||
		0 > this->currentBgmap
	){

		return;
	}
	
	// write the head
	WORLD_HEAD(__TOTAL_LAYERS, WRLD_ON | this->currentBgmap);
	WORLD_MSET(__TOTAL_LAYERS, this->bgmapDisplacement.x, 0, this->bgmapDisplacement.y);
	WORLD_GSET(__TOTAL_LAYERS, 0, 0, 0);
	WORLD_SIZE(__TOTAL_LAYERS, __SCREEN_WIDTH, __SCREEN_HEIGHT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_textutesShowStatus(Debug this, int increment, int x, int y) {

	this->currentBgmap += increment;

	if(-1 > this->currentBgmap) {

		this->currentBgmap = __NUM_BGMAPS - 1;
	}
	
	if(-1 == this->currentBgmap) {

		SpriteManager_recoverLayers(SpriteManager_getInstance());
		TextureManager_print(TextureManager_getInstance(), x, y + 2);
		ParamTableManager_print(ParamTableManager_getInstance(), x + 24, y + 2);
		Debug_dimmGame(this);
	}
	else if(__NUM_BGMAPS > this->currentBgmap) {
	
		Printing_text("Bgmap: ", x, y);
		Printing_int (this->currentBgmap, x + 7, y);

		SpriteManager_showLayer(SpriteManager_getInstance(), 0);

		this->bgmapDisplacement.x = 0;
		this->bgmapDisplacement.y = 0;

		Debug_showDebugBgmap(this);
		Debug_lightUpGame(this);
	}
	else {
		
		this->currentBgmap = -1;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		TextureManager_print(TextureManager_getInstance(), x, y);
		ParamTableManager_print(ParamTableManager_getInstance(), x + 24, y);
		Debug_dimmGame(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showSpritesStatus(Debug this, int increment, int x, int y) {

	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	VirtualList_pushBack(this->subPages, &Debug_spritesShowStatus);
	this->currentSubPage = VirtualList_begin(this->subPages);
	
	this->currentLayer = __TOTAL_LAYERS + 1;

	Debug_showSubPage(this, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_spritesShowStatus(Debug this, int increment, int x, int y) {

	
	this->currentLayer -= increment;

	if(this->currentLayer > __TOTAL_LAYERS + 1) {

		this->currentLayer = SpriteManager_getFreeLayer(SpriteManager_getInstance()) + 1;
	}
	
	if(__TOTAL_LAYERS + 1 == this->currentLayer) {

		SpriteManager_recoverLayers(SpriteManager_getInstance());
		SpriteManager_print(SpriteManager_getInstance(), x, y + 1);
	}
	else if(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->currentLayer) {
	
		Printing_text("Layer: ", x, y);
		Printing_int (this->currentLayer, x + 7, y);
		SpriteManager_showLayer(SpriteManager_getInstance(), this->currentLayer);
		Debug_lightUpGame(this);
	}
	else {
		
		this->currentLayer = __TOTAL_LAYERS + 1;
		SpriteManager_recoverLayers(SpriteManager_getInstance());
		SpriteManager_print(SpriteManager_getInstance(), x, y);
		Debug_dimmGame(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showPhysicsStatus(Debug this, int increment, int x, int y){
	
	Debug_removeSubPages(this);

	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowStatistics);
	VirtualList_pushBack(this->subPages, &Debug_physicStatusShowShapes);
	this->currentSubPage = VirtualList_begin(this->subPages);
	
	Debug_showSubPage(this, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_physicStatusShowStatistics(Debug this, int increment, int x, int y){

	PhysicalWorld_print(PhysicalWorld_getInstance(), x, y);
	CollisionManager_print(CollisionManager_getInstance(), x, y + 6);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_physicStatusShowShapes(Debug this, int increment, int x, int y){

	Printing_text("COLLISION SHAPES", x, y++);
	this->update = (void (*)(void *))&Debug_showCollisionShapes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showCollisionShapes(Debug this){
	
	CollisionManager_drawShapes(CollisionManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Debug_showHardwareStatus(Debug this, int increment, int x, int y) {
	
	Debug_removeSubPages(this);

	HardwareManager_print(HardwareManager_getInstance(), 1, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Debug_printClassSizes(int x, int y){

	int columnIncrement = 20;

	Printing_text("CLASSES' MEMORY USAGE (B) ", x, y++);
	Printing_text("Name				Size", x, ++y);
	Printing_text("AnimatedSprite", x, ++y);
	Printing_int(AnimatedSprite_getObjectSize(), x + columnIncrement, y);
	Printing_text("Background", x, ++y);
	Printing_int(Background_getObjectSize(), x + columnIncrement, y);
	Printing_text("Character", x, ++y);
	Printing_int(Actor_getObjectSize(), x + columnIncrement, y);
	Printing_text("CharGroup", x, ++y);
	Printing_int(CharGroup_getObjectSize(), x + columnIncrement, y);
	Printing_text("Clock", x, ++y);
	Printing_int(Clock_getObjectSize(), x + columnIncrement, y);
	Printing_text("Entity", x, ++y);
	Printing_int(Entity_getObjectSize(), x + columnIncrement, y);
	Printing_text("Image", x, ++y);
	Printing_int(Image_getObjectSize(), x + columnIncrement, y);
	Printing_text("InGameEntity", x, ++y);
	Printing_int(InGameEntity_getObjectSize(), x + columnIncrement, y);
//	Printing_text("Rect", x, ++y);
//	Printing_int(Rect_getObjectSize(), x + columnIncrement, y);
	Printing_text("Shape", x, ++y);
	Printing_int(Shape_getObjectSize(), x + columnIncrement, y);
	Printing_text("Sprite", x, ++y);
	Printing_int(Sprite_getObjectSize(), x + columnIncrement, y);
	Printing_text("State", x, ++y);
	Printing_int(State_getObjectSize(), x + columnIncrement, y);
	Printing_text("StateMachine", x, ++y);
	Printing_int(StateMachine_getObjectSize(), x + columnIncrement, y);
	//vbjPrintText("Scroll", x, ++y);
	//vbjPrintInt(Scroll_getObjectSize(), x + columnIncrement, y);
	Printing_text("Telegram", x, ++y);
	Printing_int(Telegram_getObjectSize(), x + columnIncrement, y);;
	Printing_text("Texture", x, ++y);
	Printing_int(Texture_getObjectSize(), x + columnIncrement, y);
	Printing_text("VirtualList", x, ++y);
	Printing_int(VirtualList_getObjectSize(), x + columnIncrement, y);
	Printing_text("VirtualNode", x, ++y);
	Printing_int(VirtualNode_getObjectSize(), x + columnIncrement, y);
}

#endif 
