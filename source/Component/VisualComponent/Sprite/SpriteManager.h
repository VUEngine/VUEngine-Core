/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SPRITE_MANAGER_H_
#define SPRITE_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Clock.h>
#include <ComponentManager.h>
#include <DisplayUnit.h>
#include <Sprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Printer;
class VirtualList;
class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TOTAL_BGMAP_SPRITE_LISTS			1
#define __TOTAL_OBJECT_SPRITE_LISTS			1
#define __TOTAL_SPRITE_LISTS				(__TOTAL_BGMAP_SPRITE_LISTS + __TOTAL_OBJECT_SPRITE_LISTS)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Data for sprite registering
/// @memberof SpriteManager
typedef struct SpriteRegistry
{
	/// List of sprites in this registry
	VirtualList sprites;

	/// Next node in the list to sort
	VirtualNode sortingNode;

	/// Total available hardware entries per sprite type
	int16 availableSlots;

	/// Index of the last slot used
	int16 nextSlotIndex;

} SpriteRegistry;

/// Rendering configuration data
/// @memberof SpriteManager
typedef struct RenderingConfig
{
	/// Maximum number of texture's rows to write each time the texture writing is active
	int32 texturesMaximumRowsToWrite;

	/// Maximum number of rows to compute on each call to the affine functions
	int32 specialEffectsRowsPerFrame;

	/// Per platform configuration data
	DisplayUnitConfig displayUnitConfig;

	/// Struct defining the optical settings for the stage
	PixelOptical pixelOptical;

} RenderingConfig;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class SpriteManager
///
/// Inherits from ComponentManager
///
/// Manages all the sprite instances.
class SpriteManager : ComponentManager
{
	/// @protectedsection

	/// Clock for the animations
	Clock animationsClock;

	/// List of all created sprites
	SpriteRegistry spriteRegistry[__TOTAL_SPRITE_LISTS];

	/// List of special sprites
	VirtualList specialSprites;

	/// Total pixels currently drawn
	int32 totalPixelsDrawn;
	
	/// Number of special effects rows to write during each rendering cycle
	int16 specialEffectsRowsPerFrame;

	/// Flag to distinguish between even and odd game frames
	bool evenFrame;

	/// Number of texture rows to write during each rendering cycle
	int8 texturesMaximumRowsToWrite;

	/// Flag to defer texturing writing over time
	bool deferTextureUpdating;

	/// Flag to defer special effect writing over time
	bool deferSpecialEffectsProcessing;

	// Flag to forze a complete Z sorting of sprites
	bool completeSort;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Process an event that the instance is listen for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// Retrieve the compoment type that the manager manages.
	/// @return Component type
	override uint32 getType();

	/// Enable the manager.
	override void enable();

	/// Disable the manager.
	override void disable();

	/// Create a sprite with the provided spec.
	/// @param owner: Object to which the sprite will attach to
	/// @param spriteSpec: Spec to use to create the sprite
	/// @return Created sprite
	override Sprite create(Entity owner, const SpriteSpec* spriteSpec);

	/// Force the purging of deleted components.
	override void purgeComponents();

	/// Configure the manager's state.
	/// @param renderingConfig: Rendering configuration data
	/// @param animationsClock: Clock for the animations
	void configure(RenderingConfig renderingConfig, Clock animationsClock);

	/// Set the clock that determines if the animations must be updated or not.
	/// @param clock: Clock for the animations
	void setAnimationsClock(Clock animationsClock);

	/// Set the number of special effects rows to write during each rendering cycle.
	/// @param specialEffectsRowsPerFrame: Number of special effects rows to write during each rendering cycle 
	void setSpecialEffectsRowsPerFrame(int32 specialEffectsRowsPerFrame);

	/// Retrieve the number of special effects rows to write during each rendering cycle.
	/// @return Number of special effects rows to write during each rendering cycle 
	int32 getSpecialEffectsRowsPerFrame();

	/// Set the number of texture rows to write during each rendering cycle.
	/// @param texturesMaximumRowsToWrite: Number of texture rows to write during each rendering cycle
	void setTexturesMaximumRowsToWrite(uint8 texturesMaximumRowsToWrite);

	/// Get the number of texture rows to write during each rendering cycle.
	/// @return Number of texture rows to write during each rendering cycle
	int8 getTexturesMaximumRowsToWrite();

	/// Enable or disable the texture writing over time.
	/// @param deferTextureUpdating: If true, textures are written overtime; otherwise
	/// they are written in a single pass
	void deferTextureUpdating(bool deferTextureUpdating);

	/// Enable or disable the writing of special effectss over time.
	/// @param deferSpecialEffectsProcessing: If true, special effectss are written overtime; otherwise
	/// they are written in a single pass
	void deferSpecialEffectsProcessing(bool deferSpecialEffectsProcessing);

	/// Render sprites.
	void render();

	/// Force the rendering and drawing of all sprites (available only when __TOOLS is defined).
	void renderAndDraw();

	/// Force the writing of graphical data to DRAM space.
	void writeTextures();

	/// Invalidate the rendering status of all sprites so they re-render again in the next cycle.
	void invalidateRendering();

	/// Show all sprites except the provided one (available only when __TOOLS is defined).
	/// @param spareSprite: Sprite to not show
	/// @param showPrinting: Flag to allow/prohibit the display of the printing sprite
	void showAllSprites(Sprite spareSprite, bool showPrinting);

	/// Hide all sprites except the provided one (available only when __TOOLS is defined).
	/// @param spareSprite: Sprite to not hide
	/// @param hidePrinting: Flag to allow/prohibit the display of the printing sprite
	void hideAllSprites(Sprite spareSprite, bool hidePrinting);

	/// Compute the total pixels drawn.
	void computeTotalPixelsDrawn();

	/// Retrieve the total number of registerd sprites.
	/// @return Total number of registerd sprites
	int32 getNumberOfSprites();

	/// Retrieve the sprite at the provided position in the list of sprites.
	/// @param index: Index of the node in the list of sprites
	/// @return Sprite at the provided position in the list of sprites
	Sprite getSpriteAtIndex(int16 index);

	/// Print sprites statistics.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param resumed: If true it only prints the most important statistics
	void print(int32 x, int32 y, bool resumed);
}

#endif
