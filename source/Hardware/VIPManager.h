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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Column table
#define __COLUMN_TABLE_ENTRIES				256
#define __BRIGHTNESS_REPEAT_ENTRIES 		96

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

	/// If false, no interrupts are enable
	bool allowInterrupts;

	/// If true, a VIP interrupt happened while in the midst of GAMESTART
	volatile bool processingGAMESTART;

	/// If true, a VIP interrupt happened while in the midst of XPEND
	volatile bool processingXPEND;

	/// If true, FRAMESTART happened during XPEND
	volatile bool FRAMESTARTDuringXPEND;

	/// Interrupt handler for timer's interrupts
	static void interruptHandler();

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

	/// Configure the VIP's palettes with the provided configuration.
	/// @param paletteConfig: Palettes configuration
	static void configurePalettes(const PaletteConfig* paletteConfig);

	/// Setup the column table with the provided spec.
	/// @param columnTableSpec: Specification for the configuration of the column table
	static void setupColumnTable(const ColumnTableSpec* columnTableSpec);

	/// Configure the brightness registers with the provided configuration.
	/// @param brightness: Brightness configuration
	static void configureBrightness(const Brightness* brightness);

	/// Configure the column table brightness repeat values.
	/// @param brightnessRepeat: Brightness configuration
	static void configureBrightnessRepeat(const BrightnessRepeatSpec* brightnessRepeat);

	/// Configure the background color.
	/// @param color: Color to apply to the background
	static void configureBackgroundColor(uint8 color);

	/// Up the brightness to the maximum.
	static void upBrightness();

	/// Lower the brightness to the minimum.
	static void lowerBrightness();

	/// Retrieve the registry of the interrupts being processed.
	/// @return The registry of the interrupts being processed
	static uint16 getCurrentInterrupt();

	/// Retrieve the time in milliseconds that each game frame lasts.
	/// @return Time in milliseconds that the game frame last according
	/// to the FRMCYC configuration
	static uint16 getGameFrameDuration();

	/// Set the FRMCYC value
	/// @param frameCycle: FRMCYC value
	static void setFrameCycle(uint8 frameCycle);

	/// Print the status of the VIP registers.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int16 x, int16 y);

	/// Reset the manager's state.
	void reset();

	/// Configure the manager's state.
	/// @param backgroundColor: Color to apply to the background
	/// @param brightness: Brightness configuration
	/// @param brightnessRepeat: Brightness configuration
	/// @param paletteConfig: Palettes configuration
	/// @param postProcessingEffects: Array of postprocessing effects
	/// Configure the brightness registers with the provided configuration.
	void configure
	(
		uint8 backgroundColor, const Brightness* brightness, const BrightnessRepeatSpec* brightnessRepeat,
		const PaletteConfig* paletteConfig, PostProcessingEffect* postProcessingEffects
	);

	/// Start memore refresh cycle
	void startMemoryRefresh();

	/// Wait for the next FCLK signal.
	void waitForFRAMESTART();

	/// Start VIP drawing operations.
	void startDrawing();

	/// Resume VIP drawing operations.
	void resumeDrawing();

	/// Suspend VIP drawing operations.
	void suspendDrawing();

	/// Stop VIP drawing operations.
	void stopDrawing();

	/// Start VIP displaying operations.
	void startDisplaying();

	/// Stop VIP displaying operations.
	void stopDisplaying();

	/// Set the enabling of interrupts.
	/// @param allowInterrupts: If true, interrupts are enabled when drawing starts
	void allowInterrupts(bool allowInterrupts);

	/// Retrieve inded of the frame buffers set using during the current game frame 
	uint32 getCurrentDrawingFrameBufferSet();
}

#endif
