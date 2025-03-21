/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Printer.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VSUManager.h>
#include <WaveForms.h>

#include "SoundTrack.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 _pcmTargetPlaybackRefreshRate = 4000;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundTrack::setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate)
{
	_pcmTargetPlaybackRefreshRate = pcmTargetPlaybackRefreshRate;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::start(bool wasPaused)
{
	if(!wasPaused)
	{
		SoundTrack::reset(this);
	}

	if(kTrackPCM == this->soundTrackSpec->trackType)
	{
		VSUManager::setMode(VSUManager::getInstance(), kPlaybackPCM);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::stop()
{
	if(kTrackPCM == this->soundTrackSpec->trackType)
	{
		VSUManager::setMode(VSUManager::getInstance(), kPlaybackNative);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::pause()
{
	VSUManager::stopSoundSourcesUsedBy(Object::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::unpause()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::suspend()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::resume()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::rewind()
{
	if(this->finished)
	{
		this->cursor = 0;
		this->cursorSxINT = 0;
		this->cursorSxLRV = 0;
		this->cursorSxFQ = 0;
		this->cursorSxEV0 = 0;
		this->cursorSxEV1 = 0;
		this->cursorSxRAM = 0;
		this->cursorSxSWP = 0;
		this->cursorSxMOD = 0;

		this->finished = false;
	}
	else
	{
		SoundTrack::reset(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundTrack::update
(
	uint32 elapsedMicroseconds, uint32 targetPCMUpdates, fix7_9_ext tickStep, fix7_9_ext targetTimerResolutionFactor, 
	fixed_t leftVolumeFactor, fixed_t rightVolumeFactor, int8 volumeReduction, uint8 volumenScalePower, uint16 frequencyDelta
)
{
	if(this->finished)
	{
		return true;
	}

	if(kTrackPCM == this->soundTrackSpec->trackType)
	{
		this->finished = SoundTrack::updatePCM(this, elapsedMicroseconds, targetPCMUpdates, volumeReduction);
	}
	else /*if(kTrackNative == this->soundTrackSpec->trackType)*/
	{
		this->finished = 
			SoundTrack::updateNative
			(
				this, tickStep, targetTimerResolutionFactor, leftVolumeFactor, rightVolumeFactor, volumeReduction, 
				volumenScalePower, frequencyDelta
			);
	}

	return this->finished;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 SoundTrack::getTicks()
{
	return this->ticks;
}


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 SoundTrack::getTotalElapsedTicks()
{
	return __FIX7_9_EXT_TO_I(this->totalElapsedTicks);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

float SoundTrack::getElapsedTicksPercentaje()
{
	if(0 == this->samples)
	{
		return 0;
	}

	return (float)this->cursor / this->samples;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 SoundTrack::getTotalPlaybackMilliseconds(uint16 targetTimerResolutionUS)
{
	uint32 totalPlaybackMilliseconds = 0;

	if(kTrackPCM == this->soundTrackSpec->trackType)
	{
		totalPlaybackMilliseconds = (this->samples * __MICROSECONDS_PER_MILLISECOND) / _pcmTargetPlaybackRefreshRate;
	}
	else if(kTrackNative == this->soundTrackSpec->trackType)
	{
		totalPlaybackMilliseconds = (uint32)((long)this->ticks * targetTimerResolutionUS / __MICROSECONDS_PER_MILLISECOND);
	}

	return totalPlaybackMilliseconds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::constructor(const SoundTrackSpec* soundTrackSpec)
{
	// Always explicitly call the base's constructor 
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::destructor()
{
	SoundTrack::stop(this);
	
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
	this->cursorSxMOD = 0;

	this->finished = false;
	this->elapsedTicks = 0;
	this->totalElapsedTicks = 0;
	this->nextElapsedTicksTarget = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::computeLength()
{
	if(NULL == this->soundTrackSpec || NULL == this->soundTrackSpec->trackKeyframes || NULL == this->soundTrackSpec->SxLRV)
	{
		return;
	}

	this->ticks = 0;
	this->samples = 0;

	int32 keyframe = 0;

	while(kSoundTrackEventEnd != this->soundTrackSpec->trackKeyframes[keyframe].events)
	{
		keyframe++;
		this->samples++;
		this->ticks += this->soundTrackSpec->trackKeyframes[keyframe].tick;
	}

	this->ticks += this->soundTrackSpec->trackKeyframes[keyframe].tick;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundTrack::updatePCM(uint32 elapsedMicroseconds, uint32 targetPCMUpdates, int8 volumeReduction)
{
	CACHE_ENABLE;

	// Elapsed time during PCM playback is based on the cursor, track's ticks and target Hz
	this->elapsedTicks += elapsedMicroseconds;

	this->cursor = this->elapsedTicks / targetPCMUpdates;

	VSUManager::applyPCMSampleToSoundSource(this->soundTrackSpec->SxLRV[this->cursor] - volumeReduction);

	CACHE_DISABLE;

	return this->cursor >= this->samples;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundTrack::updateNative
(
	fix7_9_ext tickStep, fix7_9_ext targetTimerResolutionFactor, fixed_t leftVolumeFactor, fixed_t rightVolumeFactor, 
	int8 volumeReduction, uint8 volumenScalePower, uint16 frequencyDelta
)
{
	this->elapsedTicks += tickStep;
	this->totalElapsedTicks += tickStep;

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

	if(0 != (kSoundTrackEventSxMOD & soundTrackKeyframe.events))
	{
		this->cursorSxMOD++;
	}

	uint8 volume = this->soundTrackSpec->SxLRV[this->cursorSxLRV];
	
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

	uint16 note = this->soundTrackSpec->SxFQ[this->cursorSxFQ] + frequencyDelta;

	VSUSoundSourceConfiguration vsuChannelConfiguration = 
	{
		// Requester object
		Object::safeCast(this),
		// VSU sound source to configure
		NULL,
		// VSU waveform to use
		NULL,
		// Time when the configuration elapses
		__FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(soundTrackKeyframe.tick), targetTimerResolutionFactor),
		// Sound source type
		0 != (kSoundTrackEventSweepMod & soundTrackKeyframe.events) ? 
			kSoundSourceModulation:
			0 != (kSoundTrackEventNoise & soundTrackKeyframe.events) ?
				kSoundSourceNoise:
				kSoundSourceNormal,
		// SxINT values
		this->soundTrackSpec->SxINT[this->cursorSxINT],
		// SxLRV values
		(leftVolume << 4) | rightVolume,
		// SxFQL values
		note & 0xFF,
		// SxFQH values
		note >> 8,
		// SxEV0 values
		this->soundTrackSpec->SxEV0[this->cursorSxEV0],
		// SxEV1 values
		this->soundTrackSpec->SxEV1[this->cursorSxEV1],
		// SxRAM pointer
		this->soundTrackSpec->SxRAM[this->cursorSxRAM],
		// SxSWP values
		this->soundTrackSpec->SxSWP[this->cursorSxSWP],
		// SxMOD pointer
		this->soundTrackSpec->SxMOD[this->cursorSxMOD],
		// Skip if no sound source available?
		this->soundTrackSpec->skippable
	};

	VSUManager::applySoundSourceConfiguration(&vsuChannelConfiguration);

	return ++this->cursor >= this->samples;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
