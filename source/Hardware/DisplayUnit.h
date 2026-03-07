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
class Sprite;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A method pointer for processing special effects after drawing operations are completed
typedef void (*PostProcessingEffect)(Entity scope);

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class DisplayUnit
///
/// Inherits from ListenerObject
///
/// Manages the platform's display unit.
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

	/// Enable display related interrupts that the engine doesn't use.
	/// @param customInterrupts: Interrupts to enable
	static void enableCustomInterrupts(uint16 customInterrupts);

	/// Set the multiplexed interrupts that are allowed
	/// @param enabledMultiplexedInterrupts: Multiplexed interrupts to allow
	static void enableMultiplexedInterrupts(uint32 enabledMultiplexedInterrupts);

	/// Configure the Display unit with the provided configuration.
	/// @param displayUnitConfig: Display configuration data
	static void configure(DisplayUnitConfig displayUnitConfig);

	/// Apply the provided color configuration.
	/// @param displayColorConfig: Color configuration data
	static void applyColorConfig(DisplayColorConfig displayColorConfig);

	/// Retrieve the current color configuration.
	/// @return Color configuration data
	static DisplayColorConfig getColorConfig();

	/// Up the brightness to the maximum.
	static void upBrightness();

	/// Lower the brightness to the minimum.
	static void lowerBrightness();

	/// Force showing the screen for showing an exception.
	static void showException();

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

	/// Print the status of the display hardware.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int16 x, int16 y);

	/// Reset the manager's state.
	static void reset();

	/// Erase the contents of graphics memory space.
	static void clearGraphicMemory();

	/// Start memore refresh cycle
	static void startMemoryRefresh();

	/// Wait for the next FCLK signal.
	static void waitForFrame();

	/// Start drawing operations.
	static void startDrawing();

	/// Resume drawing operations.
	static void resumeDrawing();

	/// Suspend drawing operations.
	static void suspendDrawing();

	/// Stop drawing operations.
	static void stopDrawing();

	/// Start displaying operations.
	static void startDisplaying();

	/// Stop displaying operations.
	static void stopDisplaying();

	/// Start rendering operations.
	static void startRendering();

	/// Stop rendering operations.
	static void stopRendering();

	/// Disable rendering operations.
	static void disableRendering();

	/// Commit graphics to memory.
	static void commitGraphics();

	/// Configure the number of available sprite slots per sprite lists
	/// @param availableSlots: Pointer to the array to fill
	/// @param nextSlotIndex: Pointer to the indexes of each slot for every sprite list 
	/// @param totalSpriteLists: Number of total sprite lists
	static void fillAvailableSlots(int16* availableSlots, const int16** nextSlotIndex, int16 totalSpriteLists);

	/// Retrieve the index of the list to which the sprite belongs.
	/// @param sprite: Sprite to check
	/// @return The index of the list in which a sprite must be registered
	static uint16 getSpriteListIndex(Sprite sprite);

	/// Set the enabling of interrupts.
	/// @param allowInterrupts: If true, interrupts are enabled when drawing starts
	static void allowInterrupts(bool allowInterrupts);

	/// Retrieve inded of the frame buffers set using during the current game frame 
	static uint32 getCurrentDrawingFrameBufferSet();
}

#endif
