/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VIP_MANAGER_H_
#define VIP_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <SpatialObject.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define	__TIMEERR		0x8000
#define	__XPEND			0x4000
#define	__SBHIT			0x2000
#define	__FRAMESTART	0x0010
#define	__GAMESTART		0x0008
#define	__RFBEND		0x0004
#define	__LFBEND		0x0002
#define	__SCANERR		0x0001

#define	__LOCK			0x0400	// VPU SELECT __CTA
#define	__SYNCE			0x0200	// L,R_SYNC TO VPU
#define	__RE			0x0100	// MEMORY REFLASH CYCLE ON
#define	__FCLK			0x0080
#define	__SCANRDY		0x0040
#define	__DISP			0x0002	// DISPLAY ON
#define	__DPRST			0x0001	// RESET VPU COUNTER AND WAIT __FCLK

#define	__SBOUT			0x8000	// In FrameBuffer drawing included
#define	__SBCOUNT		0x1F00	// Current bloc being drawn
#define	__OVERTIME		0x0010	// Processing
#define	__XPBSYR		0x000C	// In the midst of drawing processing reset
#define	__XPBSY1		0x0008	// In the midst of FrameBuffer 1 picture editing
#define	__XPBSY0		0x0004	// In the midst of FrameBuffer 0 picture editing
#define	__XPEN			0x0002	// Start of drawing
#define	__XPRST			0x0001	// Forcing idling


/****** VIP Registers ******/
extern volatile uint16* _vipRegisters;

/****** VIP Register Mnemonics ******/
#define	__INTPND			0x00 // Interrupt Pending
#define	__INTENB			0x01 // Interrupt Enable
#define	__INTCLR			0x02 // Interrupt Clear

#define	__DPSTTS			0x10 // Display Status
#define	__DPCTRL			0x11 // Display Control
#define	__BRTA				0x12 // Brightness A
#define	__BRTB				0x13 // Brightness B
#define	__BRTC				0x14 // Brightness C
#define	__REST				0x15 // Brightness Idle

#define	__FRMCYC			0x17 // Frame Repeat
#define	__CTA				0x18 // Column Table Pointer

#define	__XPSTTS			0x20 // Drawing Status
#define	__XPCTRL			0x21 // Drawing Control
#define	__VER				0x22 // VIP Version

#define	__SPT0				0x24 // OBJ Group 0 Pointer
#define	__SPT1				0x25 // OBJ Group 1 Pointer
#define	__SPT2				0x26 // OBJ Group 2 Pointer
#define	__SPT3				0x27 // OBJ Group 3 Pointer

#define	__GPLT0				0x30 // BGMap Palette 0
#define	__GPLT1				0x31 // BGMap Palette 1
#define	__GPLT2				0x32 // BGMap Palette 2
#define	__GPLT3				0x33 // BGMap Palette 3

#define	__JPLT0				0x34 // OBJ Palette 0
#define	__JPLT1				0x35 // OBJ Palette 1
#define	__JPLT2				0x36 // OBJ Palette 2
#define	__JPLT3				0x37 // OBJ Palette 3

#define	__BACKGROUND_COLOR	0x38 // Background Color

// Display RAM
/*@null@*/
#define __LEFT_FRAME_BUFFER_0			0x00000000 // Left Frame Buffer 0
#define __LEFT_FRAME_BUFFER_1			0x00008000 // Left Frame Buffer 1
#define __RIGHT_FRAME_BUFFER_0			0x00010000 // Right Frame Buffer 0
#define __RIGHT_FRAME_BUFFER_1			0x00018000 // Right Frame Buffer 1

#define __CHAR_SEGMENT_0_BASE_ADDRESS	0x00006000 // Characters 0-511
#define __CHAR_SEGMENT_1_BASE_ADDRESS	0x0000E000 // Characters 512-1023
#define __CHAR_SEGMENT_2_BASE_ADDRESS	0x00016000 // Characters 1024-1535
#define __CHAR_SEGMENT_3_BASE_ADDRESS	0x0001E000 // Characters 1536-2047

#define __BGMAP_SPACE_BASE_ADDRESS		0x00020000 // Base address of BGMap Memory
#define __BGMAP_SEGMENT(b)				(__BGMAP_SPACE_BASE_ADDRESS + ((b) * 0x2000))	// Address of BGMap b (0 <= b <= 13)

#define __CHAR_SPACE_BASE_ADDRESS		0x00078000

#define __OBJECT_SPACE_BASE_ADDRESS		0x0003E000 // Base address of ListenerObject Attribute Memory

#define __WORLD_SPACE_BASE_ADDRESS		0x0003D800 // Base address of World Attribute Memory

typedef struct WorldAttributes
{
	uint16 head;
	int16 gx;
	int16 gp;
	int16 gy;
	uint16 mx;
	int16 mp;
	uint16 my;
	uint16 w;
	uint16 h;
	uint16 param;
	uint16 ovr;
	uint16 spacer[5];

} WorldAttributes;

typedef struct ObjectAttributes
{
	int16 jx;
	int16 head;
	int16 jy;
	int16 tile;

} ObjectAttributes;

// pointers to access the VRAM base address
extern WorldAttributes _worldAttributesCache[__TOTAL_LAYERS];
extern ObjectAttributes _objectAttributesCache[1024];

static WorldAttributes* const _worldAttributesBaseAddress		=	(WorldAttributes*)__WORLD_SPACE_BASE_ADDRESS;
static ObjectAttributes* const	_objectAttributesBaseAddress	=	(ObjectAttributes*)__OBJECT_SPACE_BASE_ADDRESS;					// Pointer to _objectAttributesBaseAddress

static uint16* const	_columnTableBaseAddressLeft			 		=	(uint16*)0x0003DC00; // base address of Column Table (Left Eye)
static uint16* const	_columnTableBaseAddressRight				=	(uint16*)0x0003DE00; // base address of Column Table (Right Eye)

// "vbSetWorld" header flags
// (OR these together to build a World Header)

#define	__WORLD_OFF				0x0000
#define	__WORLD_ON				0xC000
#define	__WORLD_LON				0x8000
#define	__WORLD_RON				0x4000
#define	__WORLD_OBJECT			0x3000
#define	__WORLD_AFFINE			0x2000
#define	__WORLD_HBIAS			0x1000
#define	__WORLD_BGMAP			0x0000

#define	__WORLD_1x1				0x0000
#define	__WORLD_1x2				0x0100
#define	__WORLD_1x4				0x0200
#define	__WORLD_1x8				0x0300
#define	__WORLD_2x1				0x0400
#define	__WORLD_2x2				0x0500
#define	__WORLD_2x4				0x0600
#define	__WORLD_4x1				0x0800
#define	__WORLD_4x2				0x0900
#define	__WORLD_8x1				0x0C00

#define	__WORLD_OVR				0x0080
#define	__WORLD_END				0x0040

// param table for affine and hbias render
extern uint32 _dram_dirty_start;
#define __PARAM_TABLE_END 		((uint32)&_dram_dirty_start)

#define	__COLOR_BLACK			0x00
#define	__COLOR_DARK_RED		0x01
#define	__COLOR_MEDIUM_RED		0x02
#define	__COLOR_BRIGHT_RED		0x03


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

typedef struct ColumnTableSpec
{
	// defines whether the spec's first half should be mirrored (true)
	// or if a full 256 entry table is provided (false)
	bool mirror;

	// column table spec
	BYTE columnTable[];

} ColumnTableSpec;

typedef const ColumnTableSpec ColumnTableROMSpec;

typedef struct BrightnessRepeatSpec
{
	// defines whether the spec's first half should be mirrored (true)
	// or if a full 96 entry table is provided (false)
	bool mirror;

	// brightness repeat values
	uint8 brightnessRepeat[];

} BrightnessRepeatSpec;

typedef const BrightnessRepeatSpec BrightnessRepeatROMSpec;

typedef struct ColorConfig
{
	// background color
	uint8 backgroundColor;

	// brightness config
	Brightness brightness;

	// brightness repeat values
	BrightnessRepeatSpec* brightnessRepeat;

} ColorConfig;

typedef struct PaletteConfig
{
	struct Bgmap
	{
		uint8 gplt0;
		uint8 gplt1;
		uint8 gplt2;
		uint8 gplt3;
	} bgmap;

	struct ListenerObject
	{
		uint8 jplt0;
		uint8 jplt1;
		uint8 jplt2;
		uint8 jplt3;
	} object;

} PaletteConfig;

typedef void (*PostProcessingEffect) (uint32, SpatialObject);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class VIPManager : ListenerObject
{
	VirtualList postProcessingEffects;
	uint32 totalMilliseconds;
	uint32 currentDrawingFrameBufferSet;
	uint16 multiplexedGAMESTARTCounter;
	uint16 multiplexedXPENDCounter;
	uint16 timeErrorCounter;
	uint16 scanErrorCounter;
	uint16 customInterrupts;
	uint16 currrentInterrupt;
	uint16 gameFrameDuration;
	uint8 frameCycle;
	bool processingGAMESTART;
	bool processingXPEND;
	bool logicEnded;
	bool drawingEnded;
	volatile bool frameStartedDuringXPEND;
	bool skipFrameBuffersProcessing;

	/// @publicsection
	static VIPManager getInstance();
	static void interruptHandler();
	void reset();
	void setSkipFrameBuffersProcessing(bool skipFrameBuffersProcessing);
	void enableCustomInterrupts(uint16 customInterrupts);
	void enableDrawing();
	void disableDrawing();
	void enableInterrupts(uint16 interruptCode);
	void disableInterrupts();
	void displayOn();
	void displayOff();
	void setupPalettes(PaletteConfig* paletteConfig);
	void upBrightness();
	void lowerBrightness();
	void displayHide();
	void clearScreen();
	void clearBgmapSegment(int32 segment, int32 size);
	void setFrameCycle(uint8 frameCycle);
	void setupColumnTable(ColumnTableSpec* columnTableSpec);
	void useInternalColumnTable(bool internal);
	void setupBrightness(Brightness* brightness);
	void setupBrightnessRepeat(BrightnessRepeatSpec* brightnessRepeat);
	void setBackgroundColor(uint8 color);
	void pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void removePostProcessingEffects();
	void registerCurrentDrawingFrameBufferSet();
	bool isRenderingPending();
	bool isDrawingAllowed();
	bool hasFrameStartedDuringXPEND();
	uint16 getGameFrameDuration();
	void wait(uint32 milliSeconds);
}


#endif
