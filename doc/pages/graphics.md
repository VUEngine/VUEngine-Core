Graphics
========


2D
--


### AnimatedSprite


#### Animation allocation types

Since there can be Characters with many frames of animations like the game's hero on one hand, and Characters with simple animations like enemies on the other hand, there is a need to allocate each kind in different ways. There are three ways to allocate animations in memory:


##### __ANIMATED

Each Character has its dedicated Char and BGMap memory. On each animation frame change, the char definition is rewritten with the next. Since this kind of texture is the most expensive one performance-wise, it is recommended to use it only for either Characters with lots of animations, or when the number of chars for all the frames of animation is too large to be be allocated as `__ANIMATED_SHARED` (that'd be more than 511 chars, 1 is always reserved by the engine for each CharSet).


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
