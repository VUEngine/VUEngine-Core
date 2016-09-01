In-game object types
====================


Container
---------

A generic container for parenting purposes. Eases translation, rotation, scaling etc. of several entites at once.


Entity
------

A `Container` with a list of Sprites. It is the base class for any visual object.


InGameEntity
------------

Interactive entities in the levels, in general, they have a collision cuboid which Actors' cuboids test against for collisions.


AnimatedInGameEntity
--------------------

An entity with animated sprites.

### Actor

AnimatedInGameEntity with a physical body. It coordinates the collisions and the way the body must react to them. This class is designed to handle movable entities.


InanimatedInGameEntity
----------------------

InGameEntity with physical properties (friction, elasticity, etc.). In general, it is not supposed to move.


Image
-----

Just a handy entity for non-interactive images.


Backgrounds
-----------

There are two ways to create repeating backgrounds.


### ScrollBackground

Simple class to create infinitely loop images out of two Textures, either different ones or two times the same one. It supports looping over the X axis only. It is easier to use than the `MBackground`, but is quite imperformant and should therefore be avoided if possible.


### MBackground

An infinitely looping background that is composed of a variable number of up to 8 BGMaps.

This makes use of MSprite, which can be quite complex to use. Texture allocation must be planned ahead with texture preloading to work with multiple segments.
