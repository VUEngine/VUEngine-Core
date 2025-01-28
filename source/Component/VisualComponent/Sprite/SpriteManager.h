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
#include <Sprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class ObjectSpriteContainer;
class Printer;
class VirtualList;
class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TOTAL_OBJECT_SEGMENTS 	4

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum SpriteListTypes
{
	kSpriteListSpecial = 0,
	kSpriteListBgmap1,
	kSpriteListObject1,
	kSpriteListObject2,
	kSpriteListObject3,
	kSpriteListObject4,

	kSpriteListEnd
};

typedef struct SpriteRegistry
{
	VirtualList sprites;

	VirtualNode sortingNode;

} SpriteRegistry;

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
	SpriteRegistry spriteRegistry[kSpriteListEnd];

	/// List of object sprite containers
	ObjectSpriteContainer objectSpriteContainers[__TOTAL_OBJECT_SEGMENTS];

	/// Total pixels currently drawn
	int32 totalPixelsDrawn;
	
	/// Number of param table rows to write during each rendering cycle
	int16 maximumParamTableRowsToComputePerCall;

	/// Flag to distinguish between even and odd game frames
	bool evenFrame;

	/// Free WORLD layer during the last rendering cycle
	int16 bgmapIndex;

	/// Free OBJECT during the last rendering cycle
	int16 objectIndex;

	/// Previous free OBJECT during the last rendering cycle, used to avoid
	/// writing the whole address space
	int16 previousObjectIndex;

	/// Number of texture rows to write during each rendering cycle
	int8 texturesMaximumRowsToWrite;

	/// Flag to defer texturing writing over time
	bool deferTextureUpdating;

	/// Flag to defer param tables writing over time
	bool deferParamTableEffects;

	// Flag to forze a complete Z sorting of sprites
	bool completeSort;

	/// SPT index that for OBJECT memory management
	int8 spt;

	/// Cache for the VIP SPT registers
	uint16 vipSPTRegistersCache[__TOTAL_OBJECT_SEGMENTS];

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
	override Sprite instantiateComponent(Entity owner, const SpriteSpec* spriteSpec);

	/// Force the purging of deleted components.
	override void purgeComponents();

	/// Check if at least of the sprites that attach to the provided owner is visible.
	/// @param owner: Object to which the sprites attach to
	/// @return True if at least of the sprites that attach to the provided owner is visible
	override bool isAnyVisible(Entity owner);

	/// Configure the manager's state.
	/// @param texturesMaximumRowsToWrite: Number of texture rows to write during each rendering cycle
	/// @param maximumParamTableRowsToComputePerCall: Number of param table rows to write during each rendering cycle 
	/// @param size: Array with the number of OBJECTS for each container
	/// @param z: Array of Z coordinates for each container
	/// @param animationsClock: Clock for the animations
	void configure
	(
		uint8 texturesMaximumRowsToWrite, int32 maximumParamTableRowsToComputePerCall,
		const int16 size[__TOTAL_OBJECT_SEGMENTS], const int16 z[__TOTAL_OBJECT_SEGMENTS], Clock animationsClock
	);

	/// Set the clock that determines if the animations must be updated or not.
	/// @param clock: Clock for the animations
	void setAnimationsClock(Clock animationsClock);

	/// Configure the object sprite containers.
	/// @param size: Array with the number of OBJECTS for each container
	/// @param z: Array of Z coordinates for each container
	void setupObjectSpriteContainers(int16 size[__TOTAL_OBJECT_SEGMENTS], int16 z[__TOTAL_OBJECT_SEGMENTS]);

	/// Set the number of param table rows to write during each rendering cycle.
	/// @param maximumParamTableRowsToComputePerCall: Number of param table rows to write during each rendering cycle 
	void setMaximumParamTableRowsToComputePerCall(int32 maximumParamTableRowsToComputePerCall);

	/// Retrieve the number of param table rows to write during each rendering cycle.
	/// @return Number of param table rows to write during each rendering cycle 
	int32 getMaximumParamTableRowsToComputePerCall();

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

	/// Enable or disable the writing of param tables over time.
	/// @param deferAffineTransformations: If true, param tables are written overtime; otherwise
	/// they are written in a single pass
	void deferParamTableEffects(bool deferAffineTransformations);

	/// Force the rendering and drawing of all sprites.
	void prepareAll();

	/// Render sprites.
	void render();

	/// Force the rendering and drawing of all sprites (available only when __TOOLS is defined).
	void renderAndDraw();

	/// Force the writing of graphical data to DRAM space.
	void writeTextures();

	/// Hide all sprites except the provided one.
	/// @param spareSprite: Sprite to not hide
	/// @param hidePrinting: Flag to allow/prohibit the display of the printing sprite
	void hideAllSprites(Sprite spareSprite, bool hidePrinting);

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

	/// Retrieve the free WORLD layer during the last rendering cycle.
	/// @return Free WORLD layer during the last rendering cycle
	int8 getFreeLayer();

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

	/// Print object sprite containers statistics.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printObjectSpriteContainersStatus(int32 x, int32 y);
}

#endif
