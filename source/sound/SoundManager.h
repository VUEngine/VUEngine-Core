/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <MiscStructs.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define SoundManager_METHODS								\
		Object_METHODS										\


// declare the virtual methods which are redefined
#define SoundManager_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)									\


__CLASS(SoundManager);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
SoundManager SoundManager_getInstance();

// class's destructor
void SoundManager_destructor(SoundManager this);

// load wave form data in the VB memory
void SoundManager_setWaveForm(SoundManager this);

// load a bgm
void SoundManager_loadBGM(SoundManager this, u16 (*bgm)[]);

// play background song loaded
void SoundManager_playBGM(SoundManager this);

// play sound
void SoundManager_playFxSounds(SoundManager this);

// load a fx sound to be played
// it is not guaranted that the sound has been loaded
int SoundManager_loadFxSound(SoundManager this, u16* fxSound, VBVec3D  position);

// returns true if the sound is being played
int SoundManager_playingSound(SoundManager this, u16* fxSound);

// stop sound
void SoundManager_stopSound(SoundManager this, BYTE *sound);

// continue BGM play 
void SoundManager_continueBGM(SoundManager this,BYTE *sound);

// stop all playing sounds 
void SoundManager_stopAllSound(SoundManager this);
	

#endif /*SOUND_H_*/
