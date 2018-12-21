In-game object types
====================


Container
---------

A generic container for parenting purposes. Eases translation, rotation, scaling etc. of several entites at once.


Entity
------

A `Container` with a list of Sprites. It is the base class for any visual object.


EntityFactory
-------------

Allows the deferring of the instantiation and initialization of entities to be added to the stage.


AnimatedEntity
--------------

An entity with animated sprites.


Actor
-----

`AnimatedEntity` with a physical body and a collision solver. It coordinates the collisions and the way the body must react to them. This class is designed to handle movable entities.


CollisionsContainerEntity
-------------------------

Just a handy `Entity` for non-visual collisions.


ManagedEntity
-------------

`Container` that manages the sprites of all its children. It helps to reduce the performance overhead of translating lots of sprites, and helps to workaround rounding problems when projecting 3D coordinates into 2D caused by the low resolution of the Virtual Boy's displays.


RecyclableImage (MOVE TO Texture documentation)
---------------

Used to display the level's backgrounds. These allow for the textures used as backgrounds to be recycled, allowing better usage of CHAR and BGMAP.

Its purpose is to hold stage's background images and to allow for texture recycling since the engine does not support BGMAP memory defragmentation.

Texture allocation must be planned ahead with texture preloading to work with multiple segments. Not all the textures that are used by all the MBackgrounds that compose a level should be preloaded.


ManagedEntity
-------------

Like the `ManagedEntity`, it manages the sprites of all its children, but it is meant to hold entities that don't need to do anything in their update methods.


TriggerEntity
-------------

A handy `Entity` for collisions detection against non-physical entities.

