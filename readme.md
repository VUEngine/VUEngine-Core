VBJaEngine
==========

VBJaEngine is an attempt to provide other indie developers the base functionality to facilitate the 
creation of content for the Nintendo Virtual Boy by taking care of most hardware management task to create
3D games with 2D graphics or "sprites".

Features:

- General features:
    - Automatic frame rate control
    - Generic clocks based on hardware interrupts
    - Memory Pool to allocate memory dynamically
    - Generic state machines
    - Generic parenting system
    - Generic messaging system
    - Generic event listening/firing system
    - Easy to use printing functions to facilitate debug
- Debugging
    - Memory usage
    - Hardware registers' usage
    - Real time tools to check:
        - Char memory status
        - BGMap memory status
        - World layer status
        - Collision boxes
- Object Oriented support through the use of Metaprogramming (C MACROS):
    - Simple inheritance
    - Polymorphism
    - Encapsulation
    - Friend classes support
    - Runtime type checking
- Rendering:
    - Automatic char memory allocation
    - Real time char memory defragmentation
    - Automatic BGMap memory allocation
    - Automatic OBJ memory allocation
    - Automatic world layer assignment based on the objects' z position
    - Preloading textures
    - Automatic char memory defragmentation
    - Scaling/rotation effects
    - Automatic projection/parallax/scale calculations and rendering
    - Customizable perspective/deep effects on real time
    - Automatic memory allocation for param tables (used in affine and h-bias modes)
- Animation:
    - Multiple memory allocation schemas to improve efficiency
    - Frame based animation system with callback support
- Sound:
    - Sound reproduction of one BGM and up to two FX sounds simultaneous.
- Physics:
    - Accelerated/uniform movement
    - Gravity
    - Friction
    - Bouncing
    - Automatic collision detection and notification
- Particles:
    - Physically based
- Stages:
    - 3D stages
    - Level streaming
- Useful classes to speed up the content creation process:
    - Container: for transformation propagation (translation/rotation/scaling)
    - Entity: a container with a list of sprites (a "visual object")
    - InGameEntity: interactive entities in the levels (collision detection)
    - AnimatedInGameEntity: an entity with animated sprites.
    - InAnimatedInGameEntity: static in game entity with physical properties (friction, elasticity, etc.)
    - Actor: animated in game entity which coordinates a physical body with a collision shape
    - Image: handy entity to display non interactive images
    - ScrollBackground: infinite loop image (currently only supports looping over the x axe)


LICENSE
-------

Copyright (C) 2008 Jorge Eremiev <jorgech3@gmail.com>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

See See GPL-license.txt for the full GNU GENERAL PUBLIC LICENSE.

Some source code in this release that may not be covered by the GPL:

- libgccvb: compiled by a lot of other people.


GENERAL NOTES
-------------

A short summary of the file layout:

- source: VBJaEngine source code.
- source/base/libgccvb: heavily modified libgccvb
- lib: miscellaneous files needed for linking, etc.
- lib/src: miscellaneous files needed for linking, etc.    sources, and other utilities sources.
- utilities: miscellaneous utilities for padding, sound, etc.

Full documentation can be found in doc/html/index.html


COMPILING ON WIN32
------------------

The VBJaEngine comes bundled with VBDE (http://vbde.planetvb.com). 
Compile by starting the VBDE build system. 
This will produce the file libvbjae.a which must be linked against the game's code.


ACKNOWLEDGMENTS
---------------

- David Tucker, for always being open to answer all my questions (even the silly ones).

- DogP, for all the performance tips which really helped this project to be feasible, and
  for providing the code necessary to make sound support possible in the engine.

- Christian Radke (KR155E):
    * Co-development
    * Engine's features and structure advisor
    * Debugging and testing
    * Documentation

- RunnerPack, DanB, Dasi and all the other people in Planet Virtual Boy development forums
  who always are kind enough to share their knowledge.

- ElmerPCFX, for giving access to a much better compiler through his GCC 4.7 patches, and for all
  the tips, suggestions and knowledge about best practices and bad practices (of which the engine was
  previously plagued with).

- Libgccvb has been refactored to accommodate better to this engine, all its code is credited to
  its creators, Jorge Eremiev doesn't take any credit on it.

---

V810 is a trade mark of NEC.
Virtual Boy is a trade mark of Nintendo.

Jorge Eremiev is in no way affiliated with either of these parties.
