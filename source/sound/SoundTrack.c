/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <MessageDispatcher.h>
#include <Printing.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VSUManager.h>
#include <VUEngine.h>

#include "SoundTrack.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SoundTrack::start(bool wasPaused)
{
	if(!wasPaused)
	{
		SoundTrack::reset(this);
	}
}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::stop()
{}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::pause()
{}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::unpause()
{}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::suspend()
{}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::resume()
{}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::rewind()
{
	SoundTrack::reset(this);
}
//---------------------------------------------------------------------------------------------------------
bool SoundTrack::update(fix7_9_ext tickStep, fix7_9_ext targetTimerResolutionFactor, fixed_t leftVolumeFactor, fixed_t rightVolumeFactor, int8 volumeReduction, uint8 volumenScalePower)
{
	if(this->finished)
	{
		return true;
	}

	this->elapsedTicks += tickStep;

	if(this->elapsedTicks < this->nextElapsedTicksTarget)
	{
		return false;
	}

	this->elapsedTicks -= this->nextElapsedTicksTarget;

	SoundTrackKeyframe soundTrackKeyframe = this->soundTrackSpec->trackKeyframes[this->cursor];

	this->nextElapsedTicksTarget = __I_TO_FIX7_9_EXT(soundTrackKeyframe.tick);

	if(0 != (kSoundTrackEventSxINT & soundTrackKeyframe.events))
	{
		this->cursorSxINT++;
	}

	if(0 != (kSoundTrackEventSxLRV & soundTrackKeyframe.events))
	{
		this->cursorSxLRV++;
	}

	if(0 != (kSoundTrackEventSxFQ & soundTrackKeyframe.events))
	{
		this->cursorSxFQ++;
	}

	if(0 != (kSoundTrackEventSxEV0 & soundTrackKeyframe.events))
	{
		this->cursorSxEV0++;
	}

	if(0 != (kSoundTrackEventSxEV1 & soundTrackKeyframe.events))
	{
		this->cursorSxEV1++;
	}

	if(0 != (kSoundTrackEventSxRAM & soundTrackKeyframe.events))
	{
		this->cursorSxRAM++;
	}

	if(0 != (kSoundTrackEventSxSWP & soundTrackKeyframe.events))
	{
		this->cursorSxSWP++;
	}

	bool noise = false;

	if(0 != (kSoundTrackEventSxTAP & soundTrackKeyframe.events))
	{
		noise = true;
	}

	uint8 volume = this->soundTrackSpec->SxINT[this->cursorSxLRV];
	
	int16 leftVolume = volume >> 4;
	int16 rightVolume = volume & 0xF;

	if(0 != volume)
	{
		fixed_t volumeHelper = __I_TO_FIXED(volume);

		if(0 <= leftVolumeFactor)
		{
			leftVolume = __FIXED_TO_I(__FIXED_MULT(volumeHelper, leftVolumeFactor));
		}

		if(0 <= rightVolumeFactor)
		{
			rightVolume = __FIXED_TO_I(__FIXED_MULT(volumeHelper, rightVolumeFactor));
		}		
	}

	leftVolume -= volumeReduction;
	rightVolume -= volumeReduction;

	if(leftVolume < 0)
	{
		leftVolume = 0;
	}
	else if(leftVolume > __MAXIMUM_VOLUME)
	{
		leftVolume = __MAXIMUM_VOLUME;
	}

	if(rightVolume < 0)
	{
		rightVolume = 0;
	}
	else if(rightVolume > __MAXIMUM_VOLUME)
	{
		rightVolume = __MAXIMUM_VOLUME;
	}

	leftVolume >>= volumenScalePower;
	rightVolume >>= volumenScalePower;

	VSUSoundSourceConfiguration vsuChannelConfiguration = 
	{
		NULL,
		__FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(soundTrackKeyframe.tick), targetTimerResolutionFactor),
		kSoundSourceNormal,
		this->soundTrackSpec->SxINT[this->cursorSxINT],
		(leftVolume << 4) | rightVolume,
		this->soundTrackSpec->SxFQ[this->cursorSxFQ] & 0xFF,
		this->soundTrackSpec->SxFQ[this->cursorSxFQ] >> 8,
		this->soundTrackSpec->SxEV0[this->cursorSxEV0],
		this->soundTrackSpec->SxEV1[this->cursorSxEV1],
		this->soundTrackSpec->SxRAM[this->cursorSxRAM],
		this->soundTrackSpec->SxSWP[this->cursorSxSWP],
		noise
	};

	VSUManager::applySoundSourceConfiguration(VSUManager::getInstance(), &vsuChannelConfiguration);

	this->finished = ++this->cursor > this->samples;

	return false;
}
//---------------------------------------------------------------------------------------------------------
uint32 SoundTrack::getTicks()
{
	return this->ticks;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SoundTrack::constructor(const SoundTrackSpec* soundTrackSpec)
{
	Base::constructor();

	this->soundTrackSpec = soundTrackSpec;
	this->samples = this->soundTrackSpec->samples;

	SoundTrack::reset(this);

	if(0 == this->samples)
	{
		SoundTrack::computeLength(this);
	}
	else
	{
		this->ticks = this->samples = this->soundTrackSpec->samples;
	}
}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::destructor()
{
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::reset()
{
	this->cursor = 0;
	this->cursorSxINT = 0;
	this->cursorSxLRV = 0;
	this->cursorSxFQ = 0;
	this->cursorSxEV0 = 0;
	this->cursorSxEV1 = 0;
	this->cursorSxRAM = 0;
	this->cursorSxSWP = 0;
	this->finished = false;

	this->finished = false;
	this->elapsedTicks = 0;
	this->nextElapsedTicksTarget = 0;
}
//---------------------------------------------------------------------------------------------------------
void SoundTrack::computeLength()
{
	if(NULL == this->soundTrackSpec || NULL == this->soundTrackSpec->trackKeyframes || NULL == this->soundTrackSpec->SxLRV)
	{
		return;
	}

	this->ticks = 0;
	this->samples = 0;

	int32 keyframe = 0;
	SoundTrackKeyframe soundTrackKeyframe = this->soundTrackSpec->trackKeyframes[keyframe];

	while(kSoundTrackEventEnd != soundTrackKeyframe.events)
	{
		this->samples++;
		this->ticks += soundTrackKeyframe.tick;

		soundTrackKeyframe = this->soundTrackSpec->trackKeyframes[keyframe++];
	}

	this->ticks += soundTrackKeyframe.tick;
}
//---------------------------------------------------------------------------------------------------------
