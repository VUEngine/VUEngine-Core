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

#include <SoundUnit.h>

#include "SoundTrack.h"

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
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::stop()
{
	SoundUnit::stopSoundSourcesUsedBy(this->id);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::pause()
{
	SoundUnit::stopSoundSourcesUsedBy(this->id);
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
	SoundTrack::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fix7_9_ext SoundTrack::loop()
{
	return SoundTrack::fastForward(this, this->soundTrackSpec->loopPointCursor);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundTrack::update
(
	fix7_9_ext tickStep, fix7_9_ext targetTimerResolutionFactor, uint8 maximumVolume, uint8 leftVolumeReduction, 
	uint8 rightVolumeReduction, uint8 volumeReduction, uint16 frequencyDelta
)
{
	if(this->finished)
	{
		return true;
	}

	this->finished = false;
	
	this->elapsedTicks += tickStep;

	if(this->elapsedTicks < this->nextElapsedTicksTarget)
	{
		return this->finished;
	}

	this->elapsedTicks -= this->nextElapsedTicksTarget;

	if(this->cursor >= this->samples)
	{
		this->finished = true;
		return this->finished;
	}

	SoundTrackKeyframe soundTrackKeyframe = this->soundTrackSpec->trackKeyframes[this->cursor];

	this->nextElapsedTicksTarget = __I_TO_FIX7_9_EXT(soundTrackKeyframe.tick);

	SoundTrack::updateCursors(this);

	SoundTrack::sendSoundRequest
	(
		this, targetTimerResolutionFactor, maximumVolume, leftVolumeReduction, 
		rightVolumeReduction, volumeReduction, frequencyDelta
	);

	this->cursor++;

	return this->finished;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 SoundTrack::getTicks()
{
	return this->ticks;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fix7_9_ext SoundTrack::getElapsedTicks()
{
	return this->elapsedTicks;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

float SoundTrack::getElapsedTicksPercentage()
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
	return (uint32)((long)this->ticks * targetTimerResolutionUS / __MICROSECONDS_PER_MILLISECOND);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void SoundTrack::print(int16 x, int16 y)
{
	if(this->finished)
	{
		Printer::text("Done", x, y, NULL);
	}
	else
	{
		Printer::text("Play", x, y, NULL);	
	}

	Printer::text("     ", x, ++y, NULL);
	Printer::int32(this->cursor, x, y, NULL);
	Printer::text("     ", x, ++y, NULL);
	Printer::int32(this->nextElapsedTicksTarget, x, y, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::reset()
{
	this->cursor = 0;
	this->finished = false;
	this->elapsedTicks = 0;
	this->nextElapsedTicksTarget = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::updateCursors()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::sendSoundRequest
(
	fix7_9_ext targetTimerResolutionFactor __attribute__((unused)), uint8 maximumVolume __attribute__((unused)),
	uint8 leftVolumeReduction __attribute__((unused)), uint8 rightVolumeReduction __attribute__((unused)),
	uint8 volumeReduction __attribute__((unused)), uint16 frequencyDelta __attribute__((unused))
)
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::constructor(const SoundTrackSpec* soundTrackSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	static uint32 id = 0;
	this->id = id++;

	this->soundTrackSpec = soundTrackSpec;
	this->samples = 0;
	this->ticks = 0;

	SoundTrack::reset(this);
	SoundTrack::computeLength(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::destructor()
{
	SoundTrack::stop(this);
	
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTrack::computeLength()
{
	if(NULL == this->soundTrackSpec || NULL == this->soundTrackSpec->trackKeyframes )
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

fix7_9_ext SoundTrack::fastForward(uint32 cursor)
{
	SoundTrack::reset(this);

	fix7_9_ext elapsedTicks = 0;

	for(; this->cursor < cursor;)
	{
		SoundTrackKeyframe soundTrackKeyframe = this->soundTrackSpec->trackKeyframes[this->cursor];

		SoundTrack::updateCursors(this);

		this->cursor++;
	
		elapsedTicks += __I_TO_FIX7_9_EXT(soundTrackKeyframe.tick);
	}
	
	return elapsedTicks;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
