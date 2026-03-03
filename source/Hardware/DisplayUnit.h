/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef DISPLAY_UNIT_H_
#define DISPLAY_UNIT_H_

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
/// @memberof DisplayUnit
typedef struct ColumnTableSpec
{
	/// Defines whether the spec's first half should be mirrored (true)
	/// or if a full 256 entry table is provided (false)
	bool mirror;

	/// Column table entries array
	uint8 columnTable[__COLUMN_TABLE_ENTRIES];

} ColumnTableSpec;

/// A ColumnTable spec that is stored in ROM
/// @memberof DisplayUnit
typedef const ColumnTableSpec ColumnTableROMSpec;

/// Brigtness control specification
/// @memberof DisplayUnit
typedef struct BrightnessRepeatSpec
{
	/// Defines whether the spec's first half should be mirrored (true)
	/// or if a full 96 entry table is provided (false)
	bool mirror;

	/// Brightness repeat values
	uint8 brightnessRepeat[__BRIGHTNESS_REPEAT_ENTRIES];

} BrightnessRepeatSpec;

/// A BrightnessRepeat spec that is stored in ROM
/// @memberof DisplayUnit
typedef const BrightnessRepeatSpec BrightnessRepeatROMSpec;

/// Color configuration struct
/// @memberof DisplayUnit
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
/// @memberof DisplayUnit
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
/// @memberof DisplayUnit
enum VIPMultiplexedInterrupts
{
	kVIPNoMultiplexedInterrupts 			= 1 << 0,
	kVIPOnlyVIPMultiplexedInterrupts 		= 1 << 2,
	kVIPOnlyNonVIPMultiplexedInterrupts 	= 1 << 3,
	kVIPAllMultiplexedInterrupts 			= 0x7FFFFFFF,
};

/// A method pointer for processing special effects after drawing operations are completed
typedef void (*PostProcessingEffect)(uint32 currentDrawingFrameBufferSet, Entity scope);

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class DisplayUnit
///
/// Inherits from ListenerObject
///
/// Manages the VIP.
singleton class DisplayUnit : ListenerObject
{
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

	/// Configure the VIP's palettes with the provided configuration.
	/// @param paletteConfig: Palettes configuration
	static void configurePalettes(PaletteConfig paletteConfig);

	/// Setup the column table with the provided spec.
	/// @param columnTableSpec: Specification for the configuration of the column table
	static void configureColumnTable(const ColumnTableSpec* columnTableSpec);

	/// Set the brightness registers with the provided configuration.
	/// @param brightness: Brightness configuration
	static void configureBrightness(Brightness brightness);

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

	/// Retrieve the current brightness from the hardware registers.
	/// @return Brightness configuration
	static Brightness getBrightness();

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
		uint8 backgroundColor, Brightness, const BrightnessRepeatSpec* brightnessRepeat,
		PaletteConfig paletteConfig, PostProcessingEffect* postProcessingEffects
	);

	/// Start memore refresh cycle
	static void startMemoryRefresh();

	/// Wait for the next FCLK signal.
	static void waitForFRAMESTART();

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

	/// Set the enabling of interrupts.
	/// @param allowInterrupts: If true, interrupts are enabled when drawing starts
	static void allowInterrupts(bool allowInterrupts);

	/// Retrieve inded of the frame buffers set using during the current game frame 
	static uint32 getCurrentDrawingFrameBufferSet();
}

#endif
