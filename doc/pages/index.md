Introduction
------------

The VBJaEngine has been developed to fill the gap between the developer's game logic and the hardware management required to produce visual and audio results on the Nintendo Virtual Boy. The engine is a sprite based engine, which takes care of all the work necessary to display user defined sprite images on screen, allocating graphical and CPU's memory to hold the definitions of such images. 


Features
--------

The engine provides the following features for the programmer to take advantage of:

- Automatic Char memory allocation
- Automatic BGMap memory allocation
- Automatic OBJ memory allocation
- Automatic world layer assignment based on the object's Z position
- Automatic frame rate control
- Memory Pool to allocate memory dynamically
- Automatic memory allocation for Param tables (used in Affine and H-Bias modes)
- Easy to use printing functions to facilitate debug
- Sound reproduction of one BGM and up to two FX sounds simultaneously
- A generic container
- Messaging system
- Generic State Machine
- Automatic collision detection and notification
- Particle system
- Object Oriented support through the use of Metaprogramming (C MACROS):
    - Inheritance
    - Polymorphism
    - Encapsulation
- Useful classes to speed up the content creation process:
    - Image
    - Background
    - Character
    - Scroll
- Automatic loading/unloading of objects in/outside the screen
- 3D stages
- Simple physics simulation:
    - Accelerated/uniform movement.
    - Gravity
    - Friction
    - Scaling/rotation effects.
- Post-processing effects
- Clocking system
- Automatic projection/parallax/scale calculation and rendering
- Customizable perspective/deep effects on real time
- Frame based animation system with callback support
- Generic main game algorithm (game loop)

TODO

- Polygons
- Tile based collision detection
- BGMAP defragmentation
- More collision shapes: spheres, complex polyhedrons, etc.