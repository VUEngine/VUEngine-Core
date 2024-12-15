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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Clock.h>
#include <Object.h>
#include <Sprite.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class BgmapTextureManager;
class CharSetManager;
class ObjectSpriteContainer;
class ObjectTextureManager;
class ParamTableManager;
class Printing;
class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __TOTAL_OBJECT_SEGMENTS 	4


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class PrinSpriteManagerting
///
/// Inherits from ListenerObject
///
/// Manages all the sprite instances.
/// @ingroup graphics-2d-sprites
singleton class SpriteManager : Object
{
	/// @protectedsection

	/// Clock for the animations
	Clock animationsClock;

	/// Cache printing manager
	Printing printing;

	/// Cache param table manager
	ParamTableManager paramTableManager;

	/// Cache charset manager
	CharSetManager charSetManager;

	/// Cache BGMAP texture manager
	BgmapTextureManager bgmapTextureManager;

	/// Cache OBJECT texture manager
	ObjectTextureManager objectTextureManager;

	/// List of sprites to render
	VirtualList sprites;

	/// List of object sprite containers
	VirtualList objectSpriteContainers;

	/// List of sprites with special effects
	VirtualList specialSprites;

	/// List node used for for z sorting over time
	VirtualNode sortingSpriteNode;

	/// Total pixels currently drawn
	int32 totalPixelsDrawn;
	
	/// Number of param table rows to write during each rendering cycle
	int16 maximumParamTableRowsToComputePerCall;

	/// Flag to distinguish between even and odd game frames
	bool evenFrame;

	/// Free WORLD layer during the last rendering cycle
	int16 freeLayer;

	/// Number of texture rows to write during each rendering cycle
	int8 texturesMaximumRowsToWrite;

	/// Flag to defer texturing writing over time
	bool deferTextureUpdating;

	/// Flag to defer param tables writing over time
	bool deferParamTableEffects;

	// Flag to forze a complete Z sorting of sprites
	bool completeSort;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return SpriteManager singleton
	static SpriteManager getInstance();

	/// Reset the manager's state
	void reset();

	/// Set the clock that determines if the animations must be updated or not.
	/// @param clock: Clock for the animations
	void setAnimationsClock(Clock clock);
	
	/// Create a sprite with the provided spec.
	/// @param owner: Object to which the sprite will attach to
	/// @param spriteSpec: Spec to use to create the sprite
	/// @return Created sprite
	Sprite createSprite(SpatialObject owner, const SpriteSpec* spriteSpec);

	/// Destroy the provided sprite.
	/// @param sprite: Sprite to destroy
	void destroySprite(Sprite sprite);

	/// Register a sprite to be managed
	/// @param sprite: Sprite to be managed
	/// @return True if the sprite was successfully registered; false otherwise
	bool registerSprite(Sprite sprite);

	/// Unregister a sprite to be managed
	/// @param sprite: Sprite to no longer manage
	void unregisterSprite(Sprite sprite);

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

	/// Force the Z sorting of all sprites.
	void sortSprites();

	/// Force the rendering and drawing of all sprites.
	void prepareAll();

	/// Render sprites.
	void render();

	/// Force the rendering of all sprites.
	void forceRendering();

	/// Force the rendering and drawing of all sprites.
	void renderAndDraw();

	/// Copy DRAM cache data to real DRAM space.
	void writeDRAM();

	/// Force the writing of graphical data to DRAM space.
	void writeTextures();
	
	/// Show all sprites except the provided one.
	/// @param spareSprite: Sprite to not show
	/// @param showPrinting: Flag to allow/prohibit the display of the printing sprite
	void showSprites(Sprite spareSprite, bool showPrinting);

	/// Hide all sprites except the provided one.
	/// @param spareSprite: Sprite to not hide
	/// @param hidePrinting: Flag to allow/prohibit the display of the printing sprite
	void hideSprites(Sprite spareSprite, bool hidePrinting);

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
	
	/// Retrieve the object sprite container closer to the provided position.
	/// @param z: Z coordinate
	/// @return Object sprite container closer to the provided position
	ObjectSpriteContainer getObjectSpriteContainer(fixed_t z);

	/// Retrieve the object sprite container that manages the provided STP.
	/// @param spt: OBJECT space SPT
	/// @return Object sprite container that manages the provided STP
	ObjectSpriteContainer getObjectSpriteContainerBySPT(int32 spt);

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
