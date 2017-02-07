Graphics
========


2D
--


### AnimatedSprite

#### Overview

The engine abstracts Virtual Boy's VIP's memory sections in the following classes:

•	CHAR memory: CharSet

•	BGMAP memory: BgmapTexture

•	OBJ memory: ObjectTexture

•	WORLD memory: Sprite (BgmapSprite and ObjectSprite)

To allocate CHAR memory, the CharSetManager's getCharSet method must be called, which, depending on the CharSetDefintion's allocation type, will create a new CharSet or return an existing one.

To allocate BGMAP memory, the BgmapTextureManager's getTexture method must be called, which, depending on the CharSetDefintion's allocation type, will create a new Texture or return an existing one.

Both CharSet and BgmapTextures are allocated using reference counting in order to reduce both VRAM and WRAM footprint.

Each object segment is allocated by an ObjectSpriteContainer.


#### Animation allocation types

Since there can be sprites with many frames of animations like the game's main character, and sprites with simple animations like enemies or background elements, there is the need to allocate textures in different ways to maximize the hardware's resources. Because of this, the engine provides multiple ways to allocate animations in memory:


##### __ANIMATED

When using this animation type, the engine allocates a new CharSet and Texture for each request, and each time a new frame must be show, the engines writes directly to CHAR memory.
For example, each one of the following AnimatedSprite has its own Texture and CharSet:

"@image html example.png"

The inspection of the CHAR memory reveals that the CHARs corresponding to the current frame of animation of each AnimatedSprite, have been loaded, even if they belong to the same CHAR definition:

[TODO] IMAGE

If all AnimatedSprites are BgmapSprites, then, the inspection of BGMAP memory will show that only one frame of animation is loaded for each AnimatedSprite:

[TODO] IMAGE

If the AnimatedSprites at both ends are ObjectSprites, then, the inspection of OBJ memory will show the appropriate frame of animation for each AnimatedSprite:

[TODO] IMAGE



##### __ANIMATED_SHARED

Both the Char and BGMap definitions are shared between Characters of the same type. All animation frames are allocated in both memories, so in order to display each animation frame, the world layer window is moved to the proper frame in case of BGMap mode being used, or the param table is rewritten in the case of affine mode. This is useful for example for an enemy Character which has few chars and few animation frames and when there are lot of the same Character on screen.


##### __ANIMATED_SHARED_COORDINATED

[TODO]


##### __NO_ANIMATED

Used with static images like backgrounds, logos, etc.


In order to play a specific animation, call the following method:

    Character_playAnimation((Character)this, "Blink");

Or directly:

    AnimatedSprite_play((AnimatedSprite)this->sprite, this->characterDefinition->animationDescription, animationName); 


### Sprite

#### Displacement vector

Displacement vectors in SpriteDefinitions are used to manipulate the Sprite's coordinates relative to those of it's entity's parent <i>after</i> the projection from 3D to 2D space has taken place. It is to be used...

- when you need to do "internal sorting" for multi sprite entities, since the order of Sprites in the Definition are not reliable
- when you want a Sprite to be always offset by a fixed number of pixels with respect to it's parent's 2D position (can be used for parallax offsets too)


#### Sprite

[TODO]


#### MSprite

[TODO]


#### SpriteManager

[TODO]


### ParamTable

[TODO]


#### ParamTableManager

[TODO]


### Texture

[TODO]


#### TextureManager

[TODO]


3D
--

[not yet implemented]
