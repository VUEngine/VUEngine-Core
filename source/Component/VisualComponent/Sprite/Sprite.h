/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SPRITE_H_
#define SPRITE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Texture.h>
#include <VisualComponent.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Start address of WORLD space
#define __WORLD_SPACE_BASE_ADDRESS				0x0003D800	

#define __WORLD_OFF								0x0000
#define __WORLD_ON								0xC000
#define __WORLD_LON								0x8000
#define __WORLD_RON								0x4000
#define __WORLD_OBJECT							0x3000
#define __WORLD_AFFINE							0x2000
#define __WORLD_HBIAS							0x1000
#define __WORLD_BGMAP							0x0000

#define __WORLD_1x1								0x0000
#define __WORLD_1x2								0x0100
#define __WORLD_1x4								0x0200
#define __WORLD_1x8								0x0300
#define __WORLD_2x1								0x0400
#define __WORLD_2x2								0x0500
#define __WORLD_2x4								0x0600
#define __WORLD_4x1								0x0800
#define __WORLD_4x2								0x0900
#define __WORLD_8x1								0x0C00

#define __WORLD_OVR								0x0080
#define __WORLD_END								0x0040

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

/// A Sprite Spec
/// @memberof Sprite
typedef struct SpriteSpec
{
	/// VisualComponent spec
	VisualComponentSpec visualComponentSpec;

	/// Spec for the texture to display
	TextureSpec* textureSpec;

	/// Transparency mode
	/// (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	uint8 transparency;

	/// Displacement added to the sprite's position
	PixelVector displacement;

} SpriteSpec;

/// A Sprite spec that is stored in ROM
/// @memberof Sprite
typedef const SpriteSpec SpriteROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Pointers to access DRAM caches
extern WorldAttributes _worldAttributesCache[__TOTAL_LAYERS];
extern ObjectAttributes _objectAttributesCache[__TOTAL_OBJECTS];

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Sprite
///
/// Inherits from VisualComponent
///
/// Displays a Texture on the screen.
abstract class Sprite : VisualComponent
{
	/// @protectedsection


	/// Texture to display
	Texture texture;

	/// Position cache
	PixelVector position;

	/// Displacement added to the sprite's position
	PixelVector displacement;

	/// Rotation cache
	Rotation rotation;

	/// Scale cache
	PixelScale scale;

	/// Index of the block in DRAM that the sprite configures to
	/// display its texture
	int16 index;

	/// Head flags for DRAM entries
	uint16 head;

	/// Cache of the texture's half width
	int16 halfWidth;

	/// Cache of the texture's half height
	int16 halfHeight;


	/// Flag to check if rendered even if outside the screen
	bool checkIfWithinScreenSpace;

	/// Flag for special sprites
	bool hasTextures;

	/// @publicsection

	/// Class' constructor
	/// @param owner: Entity to which the sprite attaches to
	/// @param spriteSpec: Specification that determines how to configure the sprite
	void constructor(Entity owner, const SpriteSpec* spriteSpec);

	/// Called to release the component's resources.
	override void releaseResources();

	/// Retrieve the sprite's bounding box.
	/// @return Bounding box of the mesh
	override RightBox getRightBox();

	/// Create an animation controller for this sprite.
	override void createAnimationController();

	/// Force the change of frame according to each child class' implementation.
	/// @param actualFrame: The frame of the playing animation to skip to
	override void forceChangeOfFrame(int16 actualFrame);

	/// Compute the sprite's transformation.
	void transform();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @param updateAnimation: If false, animations are not updated
	/// @return The index that determines the region of DRAM that this sprite manages
	int16 render(int16 index, bool updateAnimation);

	/// Retrieve the sprite's texture.
	/// @return Texture displayed by the sprite
	Texture getTexture();

	/// Retrieve the index that determines the region of DRAM that this sprite configured
	/// @return The index that determines the region of DRAM that this sprite manages
	int16 getIndex();

	/// Retrieve the head flags for DRAM entries.
	/// @return Head flags for DRAM entries
	uint16 getHead();

	/// Retrieve the sprite's texture's half weight.
	/// @return Sprite's texture's half weight
	int32 getHalfWidth();

	/// Retrieve the sprite's texture's half height.
	/// @return Sprite's texture's half height
	int32 getHalfHeight();

	/// Retrieve the head flags written in the DRAM entries determined by index.
	/// @return Head flags written to DRAM entries
	uint32 getEffectiveHead();

	/// Retrieve the weight written in the DRAM entries determined by index.
	/// @return Weight written to DRAM entries
	uint16 getEffectiveWidth();

	/// Retrieve the height written in the DRAM entries determined by index.
	/// @return Height written to DRAM entries
	uint16 getEffectiveHeight();

	/// Retrieve the X coordinate written in the DRAM entries determined by index.
	/// @return X coordinate written to DRAM entries
	int16 getEffectiveX();

	/// Retrieve the Y coordinate written in the DRAM entries determined by index.
	/// @return Y coordinate written to DRAM entries
	int16 getEffectiveY();

	/// Retrieve the P value written in the DRAM entries determined by index.
	/// @return P value written to DRAM entries
	int16 getEffectiveP();

	/// Retrieve the MX coordinate written in the DRAM entries determined by index.
	/// @return MX coordinate written to DRAM entries
	int16 getEffectiveMX();

	/// Retrieve the MY coordinate written in the DRAM entries determined by index.
	/// @return MY coordinate written to DRAM entries
	int16 getEffectiveMY();

	/// Retrieve the MP value written in the DRAM entries determined by index.
	/// @return MP value written to DRAM entries
	int16 getEffectiveMP();

	/// Check if the sprite is visible.
	/// @return True if the sprite is visible; false otherwise
	bool isVisible();

	/// Check if the sprite is hidden.
	/// @return True if the sprite is hidden; false otherwise
	bool isHidden();

	/// Check if the sprite displays a texture in BGMAP mode.
	/// @return True if the sprite displays a texture in BGMAP mode; false otherwise
	bool isBgmap();

	/// Check if the sprite displays a texture in OBJECT mode.
	/// @return True if the sprite displays a texture in OBJECT mode; false otherwise
	bool isObject();

	/// Check if the sprite displays a texture in AFFINE mode.
	/// @return True if the sprite displays a texture in AFFINE mode; false otherwise
	bool isAffine();

	/// Check if the sprite displays a texture in HBIAS mode.
	/// @return True if the sprite displays a texture in HBIAS mode; false otherwise
	bool isHBias();

	/// Set the position cache.
	/// @param position: Position cache to save
	void setPosition(const PixelVector* position);

	/// Retrieve the position cache.
	const PixelVector* getPosition();

	/// Set the position displacement.
	/// @param displacement: Displacement added to the sprite's position
	void setDisplacement(const PixelVector* displacement);

	/// Retrieve the position displacement.
	/// @return Displacement added to the sprite's position
	const PixelVector* getDisplacement();

	/// Retrieve the cached position plus the position displacement.
	/// @return Cached position plus the position displacement
	PixelVector getDisplacedPosition();

	/// Add the color provided color data to a CHAR in the sprite's texture.
	/// @param texturePoint: Coordinate in texture's space of the CHAR to replace
	/// @param newChar: Color data array for the CHAR
	void addChar(const Point* texturePoint, const uint32* newChar);

	/// Replace a CHAR in the sprite's texture.
	/// @param texturePoint: Coordinate in texture's space of the CHAR to replace
	/// @param newChar: Color data array for the CHAR 
	/// __UINT32S_PER_CHARS(n) provides the offset within a a uint32 array of color data.
	void putChar(const Point* texturePoint, const uint32* newChar);

	/// Replace a pixel in the sprite's texture.
	/// @param texturePixel: Coordinate in texture's space of the CHAR to replace
	/// @param charSetPixel: Coordinate in CHAR space of the CHAR to replace
	/// @param newPixelColor: Color data array for the CHAR
	void putPixel(const Point* texturePixel, const Pixel* charSetPixel, uint8 newPixelColor);

	/// Invalidate the flags that determine if the sprite requires rendering.
	void invalidateRendering();

	/// Retrieve the basic class of this kind of sprite.
	/// @return ClassPointer the basic class
	virtual ClassPointer getBasicType() = 0;

	/// Load a texture.
	/// @param textureClass: Class of the texture to load
	/// @param listenForRewriting: If true, a listener is added for the texture's rewriting event
	virtual void loadTexture(ClassPointer textureClass, bool listenForRewriting);

	/// Release the sprite's texture(s)
	virtual void releaseTexture();

	/// Check if the sprite has special effects.
	/// @return True if the sprite has special effects
	virtual bool hasSpecialEffects();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	virtual int16 doRender(int16 index) = 0;

	/// Update the animation.
	virtual void updateAnimation();

	/// Process special effects.
	/// @param maximumParamTableRowsToComputePerCall: Used to defer param table computations 
	/// (-1 to compute the whole table)
	virtual void processEffects(int32 maximumParamTableRowsToComputePerCall);

	/// Set the current multiframe.
	/// @param frame: Current animation frame
	virtual void setMultiframe(uint16 frame);

	/// Forcefully show the sprite
	virtual void forceShow();

	/// Forcefully hide the sprite
	virtual void forceHide();

	/// Set the rotation cache.
	/// @param rotation: Rotation cache to save
	virtual void setRotation(const Rotation* rotation);

	/// Set the scale cache.
	/// @param scale: Scale cache to save
	virtual void setScale(const PixelScale* scale);

	/// Retrieve the sprite's total number of pixels actually displayed.
	/// @return Sprite's total number of pixels actually displayed
	virtual int32 getTotalPixels() = 0;

	/// Print the sprite's properties.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	virtual void print(int32 x, int32 y);
}

#endif
