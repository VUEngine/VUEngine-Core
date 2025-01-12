/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VIP_MANAGER_H_
#define VIP_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Entity;

extern volatile uint16* _vipRegisters __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern uint32* _currentDrawingFrameBufferSet __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern uint32 _dramDirtyStart;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TIMEERR					0x8000
#define __XPEND						0x4000
#define __SBHIT						0x2000
#define __FRAMESTART				0x0010
#define __GAMESTART					0x0008
#define __RFBEND					0x0004
#define __LFBEND					0x0002
#define __SCANERR					0x0001

#define __LOCK						0x0400	// VPU SELECT __CTA
#define __SYNCE						0x0200	// L,R_SYNC TO VPU
#define __RE						0x0100	// MEMORY REFLASH CYCLE ON
#define __FCLK						0x0080
#define __SCANRDY					0x0040
#define __DISP						0x0002	// DISPLAY ON
#define __DPRST						0x0001	// RESET VPU COUNTER AND WAIT __FCLK
#define __DPBSY						0x003C	// In the midst of displaying

#define __SBOUT						0x8000					// In FrameBuffer drawing included
#define __SBCOUNT					0x1F00					// Current bloc being drawn
#define __OVERTIME					0x0010					// Processing
#define __XPBSY1					0x0008					// In the midst of FrameBuffer 1 picture editing
#define __XPBSY0					0x0004					// In the midst of FrameBuffer 0 picture editing
#define __XPBSY						(__XPBSY0 | __XPBSY1)  // In the midst of drawing
#define __XPEN						0x0002					// Start of drawing
#define __XPRST						0x0001					// Forcing idling

// VIP Register Mnemonics
#define __INTPND					0x00  // Interrupt Pending
#define __INTENB					0x01  // Interrupt Enable
#define __INTCLR					0x02  // Interrupt Clear

#define __DPSTTS					0x10  // Display Status
#define __DPCTRL					0x11  // Display Control
#define __BRTA						0x12  // Brightness A
#define __BRTB						0x13  // Brightness B
#define __BRTC						0x14  // Brightness C
#define __REST						0x15  // Brightness Idle

#define __FRMCYC					0x17  // Frame Repeat
#define __CTA						0x18  // Column Table Pointer

#define __XPSTTS					0x20  // Drawing Status
#define __XPCTRL					0x21  // Drawing Control
#define __VER						0x22  // VIP Version

#define __SPT0						0x24  // OBJ Group 0 Pointer
#define __SPT1						0x25  // OBJ Group 1 Pointer
#define __SPT2						0x26  // OBJ Group 2 Pointer
#define __SPT3						0x27  // OBJ Group 3 Pointer

#define __GPLT0						0x30  // BGMap Palette 0
#define __GPLT1						0x31  // BGMap Palette 1
#define __GPLT2						0x32  // BGMap Palette 2
#define __GPLT3						0x33  // BGMap Palette 3

#define __JPLT0						0x34  // OBJ Palette 0
#define __JPLT1						0x35  // OBJ Palette 1
#define __JPLT2						0x36  // OBJ Palette 2
#define __JPLT3						0x37  // OBJ Palette 3

#define __BACKGROUND_COLOR			0x38  // Background Color

// Video RAM
#define __LEFT_FRAME_BUFFER_0		0x00000000	// Left Frame Buffer 0
#define __LEFT_FRAME_BUFFER_1		0x00008000	// Left Frame Buffer 1
#define __RIGHT_FRAME_BUFFER_0		0x00010000	// Right Frame Buffer 0
#define __RIGHT_FRAME_BUFFER_1		0x00018000	// Right Frame Buffer 1

// Display RAM
#define __CHAR_SPACE_BASE_ADDRESS	0x00078000
#define __BGMAP_SPACE_BASE_ADDRESS	0x00020000	// Base address of BGMap Memory
#define __OBJECT_SPACE_BASE_ADDRESS 0x0003E000	// Base address of ListenerObject Attribute Memory
#define __WORLD_SPACE_BASE_ADDRESS	0x0003D800	// Base address of World Attribute Memory

#define __BGMAP_SEGMENT(b)																											\
	(__BGMAP_SPACE_BASE_ADDRESS + ((b) * 0x2000))  // Address of BGMap b (0 <= b <= 13)

#define __PARAM_TABLE_END			((uint32) & _dramDirtyStart)
#define __BGMAP_SEGMENT_SIZE		8192

#define __WORLD_OFF					0x0000
#define __WORLD_ON					0xC000
#define __WORLD_LON					0x8000
#define __WORLD_RON					0x4000
#define __WORLD_OBJECT				0x3000
#define __WORLD_AFFINE				0x2000
#define __WORLD_HBIAS				0x1000
#define __WORLD_BGMAP				0x0000

#define __WORLD_1x1					0x0000
#define __WORLD_1x2					0x0100
#define __WORLD_1x4					0x0200
#define __WORLD_1x8					0x0300
#define __WORLD_2x1					0x0400
#define __WORLD_2x2					0x0500
#define __WORLD_2x4					0x0600
#define __WORLD_4x1					0x0800
#define __WORLD_4x2					0x0900
#define __WORLD_8x1					0x0C00

#define __WORLD_OVR					0x0080
#define __WORLD_END					0x0040

#define __COLUMN_TABLE_ENTRIES		256
#define __BRIGHTNESS_REPEAT_ENTRIES 96

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Represents an entry in WORLD space in DRAM
/// @memberof VIPManager
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

/// Represents an entry in OBJECT space in DRAM
/// @memberof VIPManager
typedef struct ObjectAttributes
{
	int16 jx;
	int16 head;
	int16 jy;
	int16 tile;

} ObjectAttributes;

/// Column table specification
/// @memberof VIPManager
typedef struct ColumnTableSpec
{
	/// Defines whether the spec's first half should be mirrored (true)
	/// or if a full 256 entry table is provided (false)
	bool mirror;

	/// Column table entries array
	BYTE columnTable[__COLUMN_TABLE_ENTRIES];

} ColumnTableSpec;

/// A ColumnTable spec that is stored in ROM
/// @memberof VIPManager
typedef const ColumnTableSpec ColumnTableROMSpec;

/// Brigtness control specification
/// @memberof VIPManager
typedef struct BrightnessRepeatSpec
{
	/// Defines whether the spec's first half should be mirrored (true)
	/// or if a full 96 entry table is provided (false)
	bool mirror;

	/// Brightness repeat values
	uint8 brightnessRepeat[__BRIGHTNESS_REPEAT_ENTRIES];

} BrightnessRepeatSpec;

/// A BrightnessRepeat spec that is stored in ROM
/// @memberof VIPManager
typedef const BrightnessRepeatSpec BrightnessRepeatROMSpec;

/// Color configuration struct
/// @memberof VIPManager
typedef struct ColorConfig
{
	/// Background color
	uint8 backgroundColor;

	// Brightness config
	Brightness brightness;

	// Brightness repeat values
	BrightnessRepeatSpec* brightnessRepeat;

} ColorConfig;

/// Palette configuration struct
/// @memberof VIPManager
typedef struct PaletteConfig
{
	struct Bgmap
	{
		uint8 gplt0;
		uint8 gplt1;
		uint8 gplt2;
		uint8 gplt3;
	} bgmap;

	struct Object
	{
		uint8 jplt0;
		uint8 jplt1;
		uint8 jplt2;
		uint8 jplt3;
	} object;

} PaletteConfig;

/// Enums used to control VIP interrupts
/// @memberof VIPManager
enum VIPMultiplexedInterrupts
{
	kVIPNoMultiplexedInterrupts 			= 1 << 0,
	kVIPOnlyVIPMultiplexedInterrupts 		= 1 << 2,
	kVIPOnlyNonVIPMultiplexedInterrupts 	= 1 << 3,
	kVIPAllMultiplexedInterrupts 			= 0x7FFFFFFF,
};

/// Enums used to control the suspension and resuming of drawing
/// @member of VIPManager
enum VIPManagerStrategies
{
	kVIPManagerFavorStability = 0,
	kVIPManagerFavorPerformance,

	kVIPManagerStrategyLimiter
};

/// A method pointer for processing special effects after drawing operations are completed
typedef void (*PostProcessingEffect)(uint32 currentDrawingFrameBufferSet, Entity scope);

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Pointers to access DRAM caches
extern WorldAttributes _worldAttributesCache[__TOTAL_LAYERS];
extern ObjectAttributes _objectAttributesCache[__TOTAL_OBJECTS];

// Pointers to access the DRAM space
static WorldAttributes* const _worldAttributesBaseAddress = (WorldAttributes*)__WORLD_SPACE_BASE_ADDRESS;
static ObjectAttributes* const _objectAttributesBaseAddress =
	(ObjectAttributes*)__OBJECT_SPACE_BASE_ADDRESS;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class VIPManager
///
/// Inherits from ListenerObject
///
/// Manages the VIP.
singleton class VIPManager : ListenerObject
{
	/// @protectedsection

	/// Linked list of post processing effects to be applied after the VIP's
	/// drawing operations are completed
	VirtualList postProcessingEffects;

	/// Frame buffers set using during the current game frame
	uint32 currentDrawingFrameBufferSet;

	/// Enum that determines which multiplexed interrupts are allowed
	uint32 enabledMultiplexedInterrupts;

	/// Enum to determine if the manager waits for the VIP before suspending/resuming
	/// the VIP's drawing operations
	uint32 drawingStrategy;

	/// Allows VIP interrupts that the engine doesn't use
	uint16 customInterrupts;

	/// Register of the interrupts being processed
	uint16 currrentInterrupt;

	/// Time in milliseconds that the game frame last according to the
	/// FRMCYC configuration
	uint16 gameFrameDuration;

	/// If true, XPEN is raised
	bool isDrawingAllowed;

	/// If true, a VIP interrupt happened while in the midst of GAMESTART
	volatile bool processingGAMESTART;

	/// If true, a VIP interrupt happened while in the midst of XPEND
	volatile bool processingXPEND;

	/// If true, FRAMESTART happened during XPEND
	volatile bool FRAMESTARTDuringXPEND;

	/// Interrupt handler for timer's interrupts
	static void interruptHandler();

	/// @publicsection
	/// Register an object that will listen for events.
	/// @param listener: ListenerObject that listen for the event
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to listen for
	static void registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Remove a specific listener object from the listening to a give code with the provided callback.
	/// @param listener: ListenerObject to remove from the list of listeners
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to stop listen for
	static void unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Reset the manager's state.
	static void reset();

	/// Configure the manager's state.
	/// @param backgroundColor: Color to apply to the background
	/// @param brightness: Brightness configuration
	/// @param brightnessRepeat: Brightness configuration
	/// @param paletteConfig: Palettes configuration
	/// @param postProcessingEffects: Array of postprocessing effects
	/// Configure the brightness registers with the provided configuration.
	static void configure
	(
		uint8 backgroundColor, Brightness* brightness, BrightnessRepeatSpec* brightnessRepeat,
		PaletteConfig* paletteConfig, PostProcessingEffect* postProcessingEffects
	);

	/// Enable VIP interrupts that the engine doesn't use.
	/// @param customInterrupts: Interrupts to enable
	static void enableCustomInterrupts(uint16 customInterrupts);

	/// Set the multiplexed interrupts that are allowed
	/// @param enabledMultiplexedInterrupts: Multiplexed interrupts to allow
	static void enableMultiplexedInterrupts(uint32 enabledMultiplexedInterrupts);

	/// Set the drawing management drawingStrategy interrupts that are allowed
	/// @param drawingStrategy: Value to control the suspension and resuming of drawing
	static void setDrawingStrategy(uint32 drawingStrategy);

	/// Retrieve the drawing management drawingStrategy interrupts that are allowed
	/// @return Value that controls the suspension and resuming of drawing
	static uint32 getDrawingStrategy();

	/// Start VIP drawing operations.
	static void startDrawing();

	/// Resume VIP drawing operations.
	static void resumeDrawing();

	/// Suspend VIP drawing operations.
	static void suspendDrawing();

	/// Stop VIP drawing operations.
	static void stopDrawing();

	/// Start VIP displaying operations.
	static void startDisplaying();

	/// Stop VIP displaying operations.
	static void stopDisplaying();

	/// Set the FRMCYC value
	/// @param frameCycle: FRMCYC value
	static void setFrameCycle(uint8 frameCycle);

	/// Configure the VIP's palettes with the provided configuration.
	/// @param paletteConfig: Palettes configuration
	static void configurePalettes(PaletteConfig* paletteConfig);

	/// Setup the column table with the provided spec.
	/// @param columnTableSpec: Specification for the configuration of the column table
	static void setupColumnTable(ColumnTableSpec* columnTableSpec);

	/// Configure the brightness registers with the provided configuration.
	/// @param brightness: Brightness configuration
	static void configureBrightness(Brightness* brightness);

	/// Configure the column table brightness repeat values.
	/// @param brightnessRepeat: Brightness configuration
	static void configureBrightnessRepeat(BrightnessRepeatSpec* brightnessRepeat);

	/// Configure the background color.
	/// @param color: Color to apply to the background
	static void configureBackgroundColor(uint8 color);

	/// Up the brightness to the maximum.
	static void upBrightness();

	/// Lower the brightness to the minimum.
	static void lowerBrightness();

	/// Push a post processing effect at the start of the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
	/// @param entity: Post-processing effect function's scope
	static void pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);

	/// Push a post processing effect at the end of the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
	/// @param entity: Post-processing effect function's scope
	static void pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);

	/// Remove a post-processing effect from the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
	/// @param entity: Post-processing effect function's scope
	static void removePostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);

	/// Remove all a post-processing effects.
	static void removePostProcessingEffects();

	/// Retrieve the registry of the interrupts being processed.
	/// @return The registry of the interrupts being processed
	static uint16 getCurrentInterrupt();

	/// Retrieve the time in milliseconds that each game frame lasts.
	/// @return Time in milliseconds that the game frame last according
	/// to the FRMCYC configuration
	static uint16 getGameFrameDuration();
}

#endif
