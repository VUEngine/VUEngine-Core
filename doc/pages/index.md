Introduction
------------

The VUEngine has been developed to fill the gap between the developer's game logic and the hardware management 
required to produce visual and audio results on the Nintendo Virtual Boy. The engine is a sprite based engine, 
which takes care of all the work necessary to display user defined sprite images on screen, allocating graphical 
and CPU's memory to hold the definitions of such images. 

@image html example.png "Optional Example Caption"


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


> This document is a work in progress and thus contains incomplete or partially outdated information.
> If you have any problems which this document is unable to help you resolve, please refer to the 
> (http://www.planetvb.com/modules/newbb/viewforum.php?forum=14&since=0 "VUEngine support forum").


License
-------

Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>

This software is licensed under the MIT License, which means you can basically you can do with it whatever 
you want as long as you include the original copyright and license notice in any copy of the software/source. 

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
	associated documentation files (the "Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
	the following conditions:
	
	The above copyright notice and this permission notice shall be included in all copies or substantial
	portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
	LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
	NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Some source code in this release that may not be covered by the license:

- libgccvb: compiled by a lot of other people.
