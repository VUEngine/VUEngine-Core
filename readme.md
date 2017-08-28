VUEngine
========

The VUEngine ("Virtual Utopia Engine", based on the Virtual Boy's code name, "Virtual Utopia Experience") 
is an attempt to provide other indie developers the base functionality to facilitate the creation of 
content for the Nintendo Virtual Boy by taking care of most hardware management tasks to create 3D games 
with 2D graphics.

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
	- User data saving support
	- Program's memory layout management
		- Use DRAM as WRAM
		- Use SRAM as WRAM
		- Variables' in-program-section allocation control
- Object Oriented support through the use of Metaprogramming (C MACROS):
	- Simple inheritance
	- Polymorphism
	- Encapsulation
	- Friend classes support
	- Runtime type checking
- Rendering:
	- Automatic CHAR memory allocation
	- Real time CHAR memory defragmentation
	- Automatic BGMAP memory allocation
	- Automatic OBJECT memory allocation
	- Automatic WORLD layer assignment based on the objects' z position
	- Texture preloading & recycling
	- Scaling/rotation effects
	- Transparency
	- Automatic projection/parallax/scale calculations and rendering
	- Customizable perspective/deep effects in real time
	- Automatic memory allocation for param tables (used in affine and h-bias modes)
- Animation:
	- Multiple memory allocation schemas to improve efficiency
	- Frame based animation system with callback support
- Sound:
	- Sound reproduction of one BGM and up to two FX sounds simultaneous.
- Physics:
	- Basic accelerated/uniform movement
	- Gravity
	- Friction
	- Bouncing
	- Automatic collision detection and notification
- Particles:
	- Physically based particles
	- Recyclable particles
- Stages:
	- 3D stages
	- Level streaming
- Debugging / Development:
	- Memory usage
	- Profiling data
	- Streaming status
	- Hardware registers' usage
	- Real time tools to check:
		- CHAR memory status
		- BGMAP memory status
		- WORLD layer status
	- Collision boxes
	- Real time stage editor
	- Real time animation inspector
- Useful classes to speed up the content creation process:
	- Container: for transformation propagation (translation/rotation/scaling)
	- Entity: a container with a list of sprites (a "visual object")
	- AnimatedEntity: an entity with animated sprites.
	- Actor: animated in game entity which coordinates a physical body with a collision shape


LICENSE
-------

Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>

This software is licensed under the MIT License, which means you can basically do with it whatever you 
want as long as you include the original copyright and license notice in any copy of the software/source. 

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


GENERAL NOTES
-------------

A short summary of the file layout:

- source: VUEngine source code.
- source/base/libgccvb: heavily modified libgccvb
- lib: miscellaneous files needed for linking, etc.
- lib/src: miscellaneous files needed for linking, etc. sources, and other utilities sources.
- utilities: miscellaneous utilities for padding, sound, etc.

Full documentation can be found in doc/DevelopmentManual.htm


COMPILING ON WIN32
------------------

The VUEngine comes bundled with VBDE <http://vbde.planetvb.com>, compile by starting the VBDE build system. 
This will produce the file libvuengine.a which must be linked against the game's code.


ACKNOWLEDGMENTS
---------------

- David Tucker, for always being open to answer all my questions (even the silly ones).
- DogP, for all the performance tips which really helped this project to be feasible, and
for providing the code necessary to make sound support possible in the engine.
- RunnerPack, DanB, Dasi and all the other people in Planet Virtual Boy development forums
who always are kind enough to share their knowledge.
- ElmerPCFX, for giving access to a much better compiler through his GCC 4.7 patches, and for all
the tips, suggestions and knowledge about best practices and bad practices (of which the engine was
previously plagued with).
- Libgccvb has been refactored to accommodate better to this engine, all its code is credited to
its creators, we do not take any credit on it.


V810 is a trade mark of NEC. Virtual Boy is a trade mark of Nintendo.
Jorge Eremiev and Christian Radke are in no way affiliated with either of these parties.
