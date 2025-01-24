VUEngine
========

[VUEngine](https://github.com/VUEngine/VUEngine-Core) ("Virtual Utopia Engine", based on the Virtual Boy's code name, "Virtual Utopia Experience") is a high-level, object oriented game engine for the Nintendo Virtual Boy. It is written in [Virtual C](https://www.vuengine.dev/documentation/language/introduction/), a custom C-dialect that resembles some of C++’ syntax that is converted by our custom transpiler to plain C with macros.

The engine aims to facilitate the creation of games for the Virtual Boy without having to worry about nor master its underlying hardware unless you want to. Instead, it provides higher level abstractions that are relevant for general game development. Check the documentaiton [here](https://www.vuengine.dev/documentation/user-guide/introduction/).

Features:

- General:
	- Object Oriented
	- Composite and composition architecture
	- Decoupling through message sending and propagation, and event firing
	- State machines
	- Separation of concers through *Spec* recipes to instantiate
	game actors
	- Restricted singletons
	- Dinaymic memory allocation through custom memory pools
	- Runtime debugging tools
	- User data saving support
	- Assets preloading

- Stages:
	- 3D stages
	- Automatic streaming
	- Parenting

- Components:
	- Behaviors
	- Physics
	- Colliders
	- Sprites
	-Wireframes

- Particles:
	- Physically capable
	- Recyclable

- Rendering:
	- CHAR memory management
	- BGAMP memory management
	- OBJECT memory management
	- WORLD memory management
	- Direct frame buffer manipulation
	- Affine/H-Bias effects
	- Transparency
	- Event driven frame based animation

- Physics:
	- Basic accelerated/uniform movement
	- Gravity
	- Friction
	- Bouncing
	- Collision detection and notification

- Sound:
	- Fully flexible VSU-native soundtrack format
	- PCM playback

- Development tools:
	- Debug
		- Memory usage
		- Profiling data
		- Streaming
		- Hardware registers' usage
		- VIP inspector:
			- CHAR memory
			- BGMAP memory
			- OBJECT memory
			- WORLD memory
		- Colliders
	- Stage editor
	- Animations inspector
	- Sound test


LICENSE
-------

© Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>

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

- assets: engine default font, etc
- doc: Documentation
- config: various configuration files
- lib: precompiler, miscellaneous files needed for linking, etc.
- headers: various header files
- source: VUEngine source code.
- source/base/libgccvb: heavily modified libgccvb
- config: code generation templated


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
- Thunderstruck, for sharing with us the source of his MIDI and PCM music players and converters.

V810 is a trade mark of NEC. Virtual Boy is a trade mark of Nintendo.
Jorge Eremiev and Christian Radke are in no way affiliated with either of these parties.
