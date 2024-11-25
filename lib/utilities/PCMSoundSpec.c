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

#include <SoundManager.h>
#include <WaveForms.h>


//=========================================================================================================
// DECLARATIONS
//=========================================================================================================

extern const uint8 PCMSoundTrack[];


//=========================================================================================================
// MACROS
//=========================================================================================================

#define PCMSoundTrack PCMSoundTrack
#define PCMSoundTrackLength 	215674


//=========================================================================================================
// DEFINITIONS
//=========================================================================================================

SoundChannelConfigurationROM PCMSoundChannelConfiguration =
{
	/// kMIDI, kPCM
	kPCM,

	/// SxINT
	0x00,

	/// Volume SxLRV
	0x00,

	/// SxRAM (this is overrode by the SoundManager)
	0x00,

	/// SxEV0
	0xF0,

	/// SxEV1
	0x00,

	/// SxFQH
	0x00,

	/// SxFQL
	0x00,

	/// Ch. 5 only
	0x00,

	/// Waveform data pointer
	PCMWaveForm,

	/// kChannelNormal, kChannelModulation, kChannelNoise
	kChannelNormal,

	/// Volume
	__SOUND_LR
};

SoundChannelROM PCMSoundChannel =
{
	/// Configuration
	(SoundChannelConfiguration*) &PCMSoundChannelConfiguration,

	//// Total number of samples
	PCMSoundTrackLength,

	/// Sound track
	{
		PCMSoundTrack
	}
};


SoundChannelROM* const PCMSoundChannels[] =
{
	&PCMSoundChannel,
	NULL
};

SoundROMSpec PCMSoundSpec =
{
	/// Name
	"PCM Sound Name",

	/// Play in loop
	false,

	/// Target timer resolution in us
	0,

	/// Tracks
	(SoundChannel**)PCMSoundChannels
};