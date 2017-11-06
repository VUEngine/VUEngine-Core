Graphics
========


2D
--

### Overview

The engine abstracts Virtual Boy's VIP's memory sections with the following classes:

- CHAR memory: CharSet
- BGMAP memory: BgmapTexture
- OBJECT memory: ObjectTexture
- WORLD memory: Sprite (BgmapSprite and ObjectSprite)

To allocate CHAR memory, the CharSetManager's getCharSet method must be called, which, depending on the CharSetDefintion's allocation type, will create a new CharSet or return an existing one.

To allocate BGMAP memory, the BgmapTextureManager's getTexture method must be called, which, depending on the CharSetDefintion's allocation type, will create a new Texture or return an existing one.

OBJECT memory is managed by four ObjectSpriteContainers.

Both CharSet and BgmapTextures are allocated using reference counting in order to reduce both VRAM and WRAM usage.


### CharSet

CharSets abstract the Virtual Boy's CHAR memory in order to optimize its usage. They are allocated and managed through reference counting by the `CharSetManager`. 

They must not be instantiated manually, but instead a request to the manager for it allocate a new `CharSet` if needed:

	CharSet charSet = CharSetManager_getCharSet(CharSetManager_getInstance(), charSetDefinition);

Whenever possible, the manager tries to return a previously allocated `CharSet` with the same CharSetDefinition as the one received on the request. That this happens depends on the allocation type used in the definition. For information about the different allocation types check the section about the animations.

Similarly, they should never be deleted manually, but must be released calling the manager:

	CharSetManager_releaseCharSet(CharSetManager_getInstance(), charSet);

When a released `CharSet`'s usage count is zero, the manager deletes it and starts defragmenting the CHAR memory.


### Texture

Textures are akin to Bitmap, PNG, JPEG, other formats used to hold graphical data. Roughly, they are the images to be displayed by the VIP either through the BGMAP memory or through the OBJECT memory.

#### BgmapTexture

This `Texture` type abstracts the Virtual Boy's BGMAP memory in order to optimize its usage. They are allocated and managed through reference counting by the `BgmapTextureManager`. 

They must not be instantiated manually, but instead a request to the manager for it allocate a new `BgmapTexture` if needed:

	BgmapTexture bgmapTexture = BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition));

Whenever possible, the manager tries to return a previously allocated `BgmapTexture` with the same TextureDefinition as the one received on the request. That this happens depends on the allocation type used in the CharSetDefinition referenced by the TextureDefinition. For information about the different `CharSet` allocation types check the section about the animations.

Similarly, they should never be deleted manually, but must be released calling the manager:

	BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), bgmapTexture);

When a released `BgmapTexture`'s usage count is zero, the manager deletes it, but since there is no BGMAP memory defragmentation mechanism within the engine, only WRAM is freed, BGMAP VRAM remains used until the current `GameState` exits.

#### BgmapTextureManager

The instance of this class manages the BGMAP memory. All textures must be retrieved by calling this class' methods instead of being instantiated or destroyed manually.

#### RecyclableBgmapTextureManager (MOVE to Texture documentation)

The instance of this class facilitates the re-usage of textures. It keeps a list of registered textures whose definitions can be replaced by new ones when released. Generally, the textures registered with this manager should be those that occupy big amounts of BGMAP memory, like those used to represent the levels' backgrounds or platforms.

#### ObjectTexture

This `Texture` type abstracts the Virtual Boy's OBJECT memory in order to optimize its usage. They are allocated individually by the object sprites. In contrast to the BGMAP textures, these are no managed at all, and are used exclusively by OBJECT based sprites.
 

### Sprite

This is the base class used to display textures. It abstract the WORLD layers used by the VIP to display BGMAPs or OBJECTs.

##### Displacement vector

Displacement vectors in SpriteDefinitions are used to manipulate the Sprite's coordinates relative to those of it's entity's parent <i>after</i> the projection from 3D to 2D space has taken place. It is to be used...
- when you need to do "internal sorting" for multi sprite entities, since the order of Sprites in the definition are not reliable
- when you want a `Sprite` to be always offset by a fixed number of pixels with respect to it's parent's 2D position (can be used for parallax offsets too)

#### BgmapSprite

This kind of `Sprite` uses a `BgmapTexture` and is displayed in one of the VIP's 32 WORLD layers. It requests a WORLD to the `SpriteManager` and relinquish it when deleted. The WORLD that it uses is constantly updated according to its z coordinate and that of the other sprites.

#### MBgmapSprite

This kind of `BgmapSprite` is used to display textures that expand across multiple BGMAP segments. Check the Virtual Boy's documentation for an explanation of the available configurations.

#### ObjectSprite

This kind of `Sprite` uses an `ObjectTexture` and is displayed in one of the VIP's 4 WORLD layers dedicated to OBJECTS. It registers itself with one of the 4 object sprite containers created by the engine and unregisters from it when deleted.

#### SpriteManager

The instance of this class manages the VIP's 32 WORLD layers. All sprites, besides those that are OBJECT based sprites, must register themselves with this manager. It handles the sorting of the WORLDs assigned to each registered `Sprite` based on their z coordinate.

#### ParamTableManager

The instance of this class manages the DRAM allocated by the engine for affine or h-bias transformation on those BGMAP based spritres that use those operations.


### Animation

Since there can be sprites with many frames of animations like the game's main character, and sprites with simple animations like enemies or background elements, there is the need to allocate textures in different ways to maximize the hardware's resources. Because of this, the engine provides multiple ways to allocate animations in memory:

##### __ANIMATED_SINGLE

When using this animation type, the engine allocates a new CharSet and Texture for each request, and each time a new frame must be show, the engines writes directly to CHAR memory.
For example, each one of the following AnimatedSprite has its own Texture and CharSet:

@image html graphics-animation-single-sample.png

The inspection of the CHAR memory reveals that the CHARs corresponding to the current frame of animation of each AnimatedSprite, have been loaded, even if they belong to the same CHAR definition:

@image html graphics-animation-single-char-memory-inspection.png

If all animated sprites are BgmapSprites, then the inspection of BGMAP memory will show that only one frame of animation is loaded for each AnimatedSprite:

@image html graphics-animation-single-bgmap-memory-inspection.png

If the animated sprites at both ends are ObjectSprites, then the inspection of OBJ memory will show the appropriate frame of animation for each AnimatedSprite:

@image html graphics-animation-single-object-memory-inspection.png

###### Usages: 
- This type of animation should be used for animated sprites with too many animation frames or whose graphics occupies too many CHARs.

###### Limitations:
- Textures that use CharSets with this type of allocation must not be preloaded, since the preloaded instance will be unusable.
- Char definition must not be optimized and each group of CHARs that form a frame of animation must preserve the order of the first frame's CHARs as specified by the BGMAP definition. Supposing that a Sprite has 3 animation frames, the Texture's size is 3x3 CHARS, and the BGMAP definition looks like:
	
		0 1 2
		3 4 5
		6 7 8

where each number signifies and index in CHAR memory; then if the 9 CHARs (0-8) that form the first frame of animation have the following appearance: 

@image html graphics-animation-single-char-memory-inspection-frame-0.png

then the second group of CHARs (9-17), that form the second frame of animation, must look like:

@image html graphics-animation-single-char-memory-inspection-frame-1.png

and finally, the third group of CHARs (9-26), that form the last frame of animation, must look like:

@image html graphics-animation-single-char-memory-inspection-frame-2.png

###### Downsides:
- Impacts performance.

##### __ANIMATED_SHARED

When using this animation type, the engine allocates a new CharSet once (and only a Texture if BgmapSprites are used); and for each new request with the same char definition, returns the same reference(s). Each time a new frame must be show, the engines writes directly to CHAR memory, and every Sprite that uses the same CharSet will display the change.
In the following example, the AnimatedSprite in the center uses an __ANIMATED_SINGLE `CharSet`, while the animated in game entities at both ends use the same __ANIMATED_SHARED `CharSet`:

@image html graphics-animation-shared-sample.png

The inspection of the CHAR memory reveals that the CHARs corresponding to the current frame of animation of the first and second animated sprites, have been loaded, being the first CHAR's sequence the one shared by the animated sprites at both ends of the screen:

@image html graphics-animation-shared-char-memory-inspection.png

If all animated sprites are BgmapSprites, then the inspection of BGMAP memory will show that only one BgmapTexture has been loaded for the animated sprites at both ends of the screen:

@image html graphics-animation-shared-bgmap-memory-inspection.png

If the animated sprites at both ends are ObjectSprites, then the inspection of OBJ memory will show the same frame of animation for both:

@image html graphics-animation-shared-object-memory-inspection.png

###### Usages:
- This type of animation should be used when there are many instances of the same AnimatedSprite definition, which has with too many animation frames or whose graphics occupies too many CHARs, and whose animations can be synchronized.

###### Limitations: 
- Playing an animation in one AnimatedSprite instance will affect the others, and playing animations in different instances will waste processor time, since only the last rendered AnimatedSprite's current animation frame will be shown.

###### Downsides:
- Impacts performance.

###### Remarks:
- If only one animated sprite instance use the CharSet, it behaves exactly as the __ANIMATED_SINGLE type.

##### __ANIMATED_SHARED_COORDINATED

This animation type works exactly the same as the __ANIMATED_SHARED, but for each CHAR definition, the engine spawns an AnimationCoordinator that ensures that only one AnimationController is playing an animation at any given time.

###### Usages:
- The same as __ANIMATED_SHARED_COORDINATED, but saves processor's time when multiple animated sprites must be synchronized.

##### __ANIMATED_MULTI

When using this animation type, the engine allocates a new CharSet once (an only Texture when using BgmapSprites); and for each new request, with the same char definition and allocation type, returns the same reference(s). Depending on the Sprite's type, when a new frame of animation must be shown, the engine either modifies the WORLD's mx and my values, or writes to OBJ memory. 
In the following example, the AnimatedSprite in the center uses an __ANIMATED_SHARED CharSet, while the animated sprites at both ends use the same __ANIMATED_MULTI `CharSet`:

@image html graphics-animation-multi-sample.png

The inspection of the CHAR memory reveals that all the CHARs used by all the animation frames have been loaded into CHAR memory for the animated sprites at both ends of the screen (the last CHARs correspond to the AnimatedSprite at the center, that uses an __ANIMATED_SINGLE CharSet):

@image html graphics-animation-multi-char-memory-inspection.png

If all the animated sprites are BgmapSprites, then the inspection of BGMAP memory will show that all frames of animation have been loaded for the animated sprites at both ends of the screen (the last frame corresponds to the AnimatedSprite in the center, that uses an __ANIMATED_SINGLE CharSet):

@image html graphics-animation-multi-bgmap-memory-inspection.png

If the animated sprites at both ends are ObjectSprites, then the inspection of OBJ memory will show the corresponding animation frame has been loaded for each of the animated sprites at each end of the screen:

@image html graphics-animation-multi-object-memory-inspection.png

###### Usages:
- This type of animation should be used for animated sprites with too many animation frames or whose graphics occupies too many CHARs, but when their instances must not be necessarily synchronized.

###### Limitations:
- When using a __WORLD_AFFINE BgmapSprite, each time that an animation frame have to be rendered, the affine table must be computed.

###### Downsides:
- Uses too much CHAR memory.

##### __NO_ANIMATED

Used for static images like backgrounds, logos, etc.


In order to play a specific animation, call the following method:

    AnimatedEntity_playAnimation(__SAFE_CAST(AnimatedEntity, this), "Blink");

Or directly:

    Sprite_play(__SAFE_CAST(SpriteSprite, this->sprite), animationDescription, "Blink");


3D
--

### Overview

The engine has a very primitive, and still highly inefficient, support for 3D wireframe graphics by writing directly into the Virtual Boy's display buffers.

It is meant to be used mainly for debugging purposes and for very light game features like simple graphical effects. In any case, it is not intended to be used to create a purely polygon based game.

### Polyhedron

This class is used to draw 3D bodies into the screen. It keeps a list of sorted vertexes.

### WireframeManager

Each `Polyhedron` must register itself against the unique instance of this class to be draw during the VIP's end of drawing interrupt. When a registered `Polyhedron` is deleted, it must inform this manager in order for it to be unregistered.
