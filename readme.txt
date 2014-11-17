VbJaEngine GPL source release				24 October 2008
==================================

This file contains the following sections:

LICENSE
GENERAL NOTES
COMPILING ON WIN32
COMPILING ON GNU/LINUX


LICENSE
=======

Copyright (C) 2008 Jorge Eremiev
jorgech3@gmail.com

See GPL-license.txt for the GNU GENERAL PUBLIC LICENSE

Some source code in this release that may not be covered by the GPL:

	· libgccvb: compiled by a lot of other people.

-----------------------------------------------------------------------------

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
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



GENERAL NOTES
=============

Description: 
	
	vbJaEngine is an attempt to provide other indie programmers the base 
	functionality to facilitate the creation of content for the Nintendo 
	Virtual Boy by taking care of most hardware management task to create 
	3D games with 2D graphics or "sprites".


Features:

	· General features:
		* Automatic frame rate control
		* Generic clocks based on hardware interrupts
		* Memory Pool to allocate memory dynamically
		* Easy to use printing functions to facilitate debug
		* Generic parenting system
		* Generic messaging system
		* Generic state Machines
		* Generic clocks based on hardware interrupts
	· Object Oriented support through the use of Metaprogramming (C MACROS):
		* Inheritance
		* Polymorphism
		* Encapsulation
	· Rendering:
		* Automatic char memory allocation
		* Automatic bgmap memory allocation
		* Automatic world layer assignment based on the objectï¿½s z position
		* Scaling/rotation effects
		* Automatic projection/parallax/scale calculation and rendering
		* Customizable perspective/deep effects on real time
		* Automatic memory allocation for Param tables (used in affine and h-bias modes).
	· Animation:
		* Multiple memory allocation schemas to improve efficiency
		* Frame based animation system with callback support
	· Sound:
		* Sound reproduction of one BGM and up to two FX sounds simultaneous.
	· Physics:
		* Accelerated/uniform movement
		* Gravity
		* Friction
		* Automatic Collision detection and notification.
	· Stages:
		* 3D stages
		* Automatic object streaming
	· Useful classes to speed up the content creation process:
		* Actors
		* Images
		* Backgrounds
		* Scrolling


A short summary of the file layout:

src/					vbJaEngine source code.
src/base/libgccvb/		heavily modified libgccvb
lib/					miscelaneous files needed for linking, etc.				
lib/src/				miscelaneous files needed for linking, etc.	sources, and other utilities sources.
utilities/				miscelaneous utitilities for padding, sound, etc.


For more documentation go to:

http://www.vr32.de/modules/dokuwiki/doku.php?id=vbjaengine_programmer_s_guide


COMPILING ON WIN32
==================

Requirements: 

	· Minimal Linux environment (i.e: CygWin)
	
	· GCCVB 4.4.2 for V810 


1. Create a CygWin environment variable called VBJAENGINE which must point to the vbJaEngine's folder.

2. Replace the crt0.o file created by gccvb with the one provided with this source code:

	$ cp -f /usr/local/v810/lib/crt0.o /usr/local/v810/lib/crt0.o.bak
	$ cp -f $VBJAENGINE/lib/crt0.o /usr/local/v810/lib/crt0.o
 
3. To compile the execute the following command inside the engine's folder:

	$ make

	This will produce the file libvbjae.a which must be linked agains the game's code.


COMPILING ON GNU/LINUX
==================

Requirements: 

	· GCCVB 4.4.2 for V810


1. Create an environment variable called VBJAENGINE which must point to the vbJaEngine's folder.

2. As root replace the crt0.o file created by gccvb with the one provided with this source code:

	# cp -f /usr/local/v810/lib/crt0.o /usr/local/v810/lib/crt0.o.bak
	# cp -f $VBJAENGINE/lib/crt0.o /usr/local/v810/lib/crt0.o
 
3. To compile the execute the following command inside the engine's folder:

	$ make

	This will produce the file libvbjae.a which must be linked agains the game's code.


ACKNOWLEDGMENTS
==================

	· David Tucker, for always being open to answer all my questions (even the silly ones).
	
	· DogP, for all the performace tips which really helped this project to be feasible, and
	  for providing the code necessary to make sound support possible in the engine. 
	
	· Christian Radke (KR155E), for helping me with the documentation, debbuging, and all the other little 
	  tasks that I can't remeber.
	  
	· RunnerPack, DanB, Dasi and all the other people in Planet Virtual Boy develpment forums 
	  who always are kind enough to share their knowledge. 

	· Libgccvb has been refactored to accomodate better to this engine, all it's code is credited to
	  it's creators and Jorge Eremiev doesn't take any credit on it.

==================  

V810 is a trade mark of NEC.
Virtual Boy is a trade mark of Nintendo.

Jorge Eremiev is in no way affiliated with either of these parties.
