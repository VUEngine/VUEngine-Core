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
#include <Profiler.h>
#include <Singleton.h>
#include <VIPManager.h>
#include <VirtualList.h>
#include <WaveForms.h>

#include "VSUManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DEFINITIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const WaveformData PCMWaveform =
{
	/// Pointer to the waveform's data
	{
		63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 
	},

	/// Data's CRC
	0xFFFFFFFF,
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __WAVE_ADDRESS(n)					(uint8*)(0x01000000 + (n * 128))
#define __MODULATION_DATA					(uint8*)0x01000280;
#define __SSTOP								*(uint8*)0x01000580
#define __SOUND_WRAPPER_STOP_SOUND 			0x20

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VSUSoundSource* const _vsuSoundSources = (VSUSoundSource*)0x01000400;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::applySoundSourceConfiguration(const VSUSoundSourceConfigurationRequest* vsuSoundSourceConfigurationRequest)
{
	VSUManager vsuManager = VSUManager::getInstance();

	int16 vsuSoundSourceIndex = 
		VSUManager::findAvailableSoundSource
		(
			vsuManager, vsuSoundSourceConfigurationRequest->requester, vsuSoundSourceConfigurationRequest->type, 
			!vsuSoundSourceConfigurationRequest->skippable
		);

	if(0 > vsuSoundSourceIndex)
	{
		if(vsuManager->allowQueueingSoundRequests && !vsuSoundSourceConfigurationRequest->skippable)
		{
			VSUManager::registerQueuedSoundSourceConfigurationRequest(vsuManager, vsuSoundSourceConfigurationRequest);
		}
	}
	else
	{
		Waveform* waveform = VSUManager::findWaveform(vsuManager, vsuSoundSourceConfigurationRequest->SxRAM);

		VSUManager::configureSoundSource(vsuManager, vsuSoundSourceIndex, vsuSoundSourceConfigurationRequest, waveform);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::stopSoundSourcesUsedBy(Object requester)
{
	VSUManager vsuManager = VSUManager::getInstance();

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(requester == vsuManager->vsuSoundSourceConfigurations[i].requester)
		{
			vsuManager->vsuSoundSourceConfigurations[i].timeout = -1;
			vsuManager->vsuSoundSourceConfigurations[i].waveform = NULL;
			vsuManager->vsuSoundSourceConfigurations[i].SxINT = 0;
			vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxINT = 0;
		}
	}	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 VSUManager::getMode()
{
	return VSUManager::getInstance()->playbackMode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


#ifndef __SHIPPING
static void VSUManager::print(int32 x, int32 y)
{
	VSUManager vsuManager = VSUManager::getInstance();

	int32 xDisplacement = 15;
	int32 yDisplacement = y;

	int32 i = 0;

	// Reset all sounds and channels
	for(i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		int32 y = yDisplacement;

		VSUManager::printVSUSoundSourceConfiguration(&vsuManager->vsuSoundSourceConfigurations[i], x, y);
		
		x += xDisplacement;
		if(x > 47 - xDisplacement)
		{
			x = 1;
			yDisplacement += 15;
		}
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void VSUManager::printWaveFormStatus(int32 x, int32 y)
{
	VSUManager vsuManager = VSUManager::getInstance();

	for(uint32 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		PRINT_TEXT("           ", x, y + vsuManager->waveforms[i].index);
		PRINT_INT(vsuManager->waveforms[i].index, x, y + vsuManager->waveforms[i].index);
		PRINT_INT(vsuManager->waveforms[i].usageCount, x + 4, y + vsuManager->waveforms[i].index);
		PRINT_HEX((uint32)vsuManager->waveforms[i].data, x + 8, y + vsuManager->waveforms[i].index);
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void VSUManager::printVSUSoundSourceConfiguration
(
	const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration, int16 x, int y
)
{
	if(NULL == vsuSoundSourceConfiguration)
	{
		return;
	}

	PRINT_TEXT("TIMEO:         ", x, ++y);
	PRINT_INT(vsuSoundSourceConfiguration->timeout, x + 7, y);

	PRINT_TEXT("SXINT:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxINT, x + 7, y, 2);

	PRINT_TEXT("SXLRV:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxLRV, x + 7, y, 2);

	PRINT_TEXT("SXFQL:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxFQL, x + 7, y, 2);

	PRINT_TEXT("SXFQH:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxFQH, x + 7, y, 2);

	PRINT_TEXT("SXEV0:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxEV0, x + 7, y, 2);

	PRINT_TEXT("SXEV1:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxEV1, x + 7, y, 2);

	PRINT_TEXT("SXRAM:         ", x, ++y);
	PRINT_HEX_EXT(0x0000FFFF & (uint32)vsuSoundSourceConfiguration->SxRAM, x + 7, y, 2);

	PRINT_TEXT("SXSWP:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxSWP, x + 7, y, 2);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VSUManager::reset()
{
	VSUManager::stopAllSounds(this);
	VSUManager::enableQueue(this);

	this->ticks = 0;
	this->haveUsedSoundSources = false;
	this->haveQueuedRequests = false;

	VirtualList::deleteData(this->queuedVSUSoundSourceConfigurationRequests);

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		this->waveforms[i].index = i;
		this->waveforms[i].usageCount = 0;
		this->waveforms[i].overwrite = true;
		this->waveforms[i].wave = __WAVE_ADDRESS(i);
		this->waveforms[i].data = NULL;
		this->waveforms[i].crc = 0;

		for(uint32 j = 0; j < 128; j++)
		{
			this->waveforms[i].wave[j] = 0;
		}
	}

	uint8* modulationData = __MODULATION_DATA;

	for(int16 i = 0; i <= 32 * 4; i++)
	{
		modulationData[i] = 0;
	}

	for(int16 i = 0; i < __TOTAL_NORMAL_CHANNELS; i++)
	{
		this->vsuSoundSourceConfigurations[i].type = kSoundSourceNormal;
	}

	for(int16 i = __TOTAL_NORMAL_CHANNELS; i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; i++)
	{
		this->vsuSoundSourceConfigurations[i].type = kSoundSourceModulation | kSoundSourceNormal;
	}

	for
	(
		int16 i = __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; 
		i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS + __TOTAL_NOISE_CHANNELS; 
		i++
	)
	{
		this->vsuSoundSourceConfigurations[i].type = kSoundSourceNoise;
	}

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		this->vsuSoundSourceConfigurations[i].requester = NULL;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource = &_vsuSoundSources[i];
		this->vsuSoundSourceConfigurations[i].waveform = NULL;
		this->vsuSoundSourceConfigurations[i].timeout = -1;
		this->vsuSoundSourceConfigurations[i].SxLRV = 0;
		this->vsuSoundSourceConfigurations[i].SxFQL = 0;
		this->vsuSoundSourceConfigurations[i].SxFQH = 0;
		this->vsuSoundSourceConfigurations[i].SxEV0 = kPlaybackPCM == this->playbackMode ? 0xFF : 0x00;
		this->vsuSoundSourceConfigurations[i].SxEV1 = 0;
		this->vsuSoundSourceConfigurations[i].SxRAM = 0;
		this->vsuSoundSourceConfigurations[i].SxSWP = 0;
		this->vsuSoundSourceConfigurations[i].SxINT = kPlaybackPCM == this->playbackMode ? 0x9F : 0;

		Waveform* waveform = VSUManager::findWaveform(this, this->playbackMode ? &PCMWaveform : NULL);

		this->vsuSoundSourceConfigurations[i].waveform = waveform;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxLRV = this->vsuSoundSourceConfigurations[i].SxLRV;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxFQL = this->vsuSoundSourceConfigurations[i].SxFQL;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxFQH = this->vsuSoundSourceConfigurations[i].SxFQH;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV0 = this->vsuSoundSourceConfigurations[i].SxEV0;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV1 = this->vsuSoundSourceConfigurations[i].SxEV1;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxRAM = NULL == waveform ? 0 : waveform->index;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxSWP = this->vsuSoundSourceConfigurations[i].SxSWP;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxINT = this->vsuSoundSourceConfigurations[i].SxINT;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VSUManager::setMode(uint32 playbackMode)
{
	this->playbackMode = playbackMode;

	switch(playbackMode)
	{
		case kPlaybackPCM:
		{
			VIPManager::enableMultiplexedInterrupts(kVIPAllMultiplexedInterrupts);	
			break;
		}
		default:
		{
			VIPManager::enableMultiplexedInterrupts(kVIPNoMultiplexedInterrupts);
			break;
		}
	}

	VSUManager::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VSUManager::update()
{
	if(this->haveUsedSoundSources)
	{
		VSUManager::releaseSoundSources(this);
	}

	if(this->haveQueuedRequests)
	{
		VSUManager::dispatchQueuedSoundSourceConfigurations(this);
	}

	this->ticks += __I_TO_FIX7_9_EXT(1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VSUManager::stopAllSounds()
{
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		this->vsuSoundSourceConfigurations[i].requester = NULL;
		this->vsuSoundSourceConfigurations[i].waveform = NULL;
		this->vsuSoundSourceConfigurations[i].timeout = -1;
		this->vsuSoundSourceConfigurations[i].SxLRV = 0;
		this->vsuSoundSourceConfigurations[i].SxFQL = 0;
		this->vsuSoundSourceConfigurations[i].SxFQH = 0;
		this->vsuSoundSourceConfigurations[i].SxEV0 = kPlaybackPCM == this->playbackMode ? 0xFF : 0x00;
		this->vsuSoundSourceConfigurations[i].SxEV1 = 0;
		this->vsuSoundSourceConfigurations[i].SxRAM = 0;
		this->vsuSoundSourceConfigurations[i].SxSWP = 0;
		this->vsuSoundSourceConfigurations[i].SxINT = kPlaybackPCM == this->playbackMode ? 0x9F : 0;
	}

	__SSTOP = 0x01;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::enableQueue()
{
	this->allowQueueingSoundRequests = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::disableQueue()
{
	this->allowQueueingSoundRequests = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::flushQueuedSounds()
{
	VirtualList::deleteData(this->queuedVSUSoundSourceConfigurationRequests);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->queuedVSUSoundSourceConfigurationRequests = new VirtualList();
	this->allowQueueingSoundRequests = true;
	this->targetPCMUpdates = 0;
	this->playbackMode = kPlaybackNative;
	this->haveUsedSoundSources = false;
	this->haveQueuedRequests = false;

	VSUManager::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::destructor()
{
	if(!isDeleted(this->queuedVSUSoundSourceConfigurationRequests))
	{
		VirtualList::deleteData(this->queuedVSUSoundSourceConfigurationRequests);
		delete this->queuedVSUSoundSourceConfigurationRequests;
		this->queuedVSUSoundSourceConfigurationRequests = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::configureSoundSource
(
	int16 vsuSoundSourceIndex, const VSUSoundSourceConfigurationRequest* vsuSoundSourceConfigurationRequest, Waveform* waveform
)
{
	int16 i = vsuSoundSourceIndex;
	VSUSoundSource* vsuSoundSource = this->vsuSoundSourceConfigurations[i].vsuSoundSource;

	this->haveUsedSoundSources = true;

	bool setSxINT = 
		0 != (0x80 & vsuSoundSourceConfigurationRequest->SxINT)
		|| 
		(this->vsuSoundSourceConfigurations[i].requester != vsuSoundSourceConfigurationRequest->requester);

	this->vsuSoundSourceConfigurations[i].requester = vsuSoundSourceConfigurationRequest->requester;
	this->vsuSoundSourceConfigurations[i].waveform = waveform;
	this->vsuSoundSourceConfigurations[i].timeout = this->ticks + vsuSoundSourceConfigurationRequest->timeout;
	this->vsuSoundSourceConfigurations[i].SxINT = vsuSoundSourceConfigurationRequest->SxINT | 0x80;
	this->vsuSoundSourceConfigurations[i].SxLRV = vsuSoundSourceConfigurationRequest->SxLRV;
	this->vsuSoundSourceConfigurations[i].SxFQL = vsuSoundSourceConfigurationRequest->SxFQL;
	this->vsuSoundSourceConfigurations[i].SxFQH = vsuSoundSourceConfigurationRequest->SxFQH;
	this->vsuSoundSourceConfigurations[i].SxEV0 = vsuSoundSourceConfigurationRequest->SxEV0;
	this->vsuSoundSourceConfigurations[i].SxEV1 = vsuSoundSourceConfigurationRequest->SxEV1;
	this->vsuSoundSourceConfigurations[i].SxRAM = waveform->index;
	this->vsuSoundSourceConfigurations[i].SxSWP = vsuSoundSourceConfigurationRequest->SxSWP;
	this->vsuSoundSourceConfigurations[i].skippable = vsuSoundSourceConfigurationRequest->skippable;

	vsuSoundSource->SxLRV = vsuSoundSourceConfigurationRequest->SxLRV;
	vsuSoundSource->SxFQL = vsuSoundSourceConfigurationRequest->SxFQL;
	vsuSoundSource->SxFQH = vsuSoundSourceConfigurationRequest->SxFQH;
	vsuSoundSource->SxEV0 = vsuSoundSourceConfigurationRequest->SxEV0;
	vsuSoundSource->SxEV1 = vsuSoundSourceConfigurationRequest->SxEV1;
	vsuSoundSource->SxRAM = waveform->index;
	vsuSoundSource->SxSWP = vsuSoundSourceConfigurationRequest->SxSWP;

	/// If SxINT is set every time it can produce the pop sound because it 
	/// resets various of the VSU's internal counters.
	if(setSxINT)
	{
		vsuSoundSource->SxINT = vsuSoundSourceConfigurationRequest->SxINT | 0x80;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 VSUManager::findAvailableSoundSource(Object requester, uint32 soundSourceType, bool force)
{
	// First try to find a sound source that has previously assigned to the same requester
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(0 == (soundSourceType & this->vsuSoundSourceConfigurations[i].type))
		{
			continue;
		}

		if(requester == this->vsuSoundSourceConfigurations[i].requester)
		{
		 	return i;
		}
	}

	// Now try to find a sound source whose timeout has just expired
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(0 == (soundSourceType & this->vsuSoundSourceConfigurations[i].type))
		{
			continue;
		}

		if(this->ticks >= this->vsuSoundSourceConfigurations[i].timeout)
		{
			return i;
		}
	}

	if(force)
	{
		int16 soonestFreeSoundSource = -1;

		// Now try to find a sound source whose timeout has just expired
		for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
		{
			if(0 == (soundSourceType & this->vsuSoundSourceConfigurations[i].type))
			{
				continue;
			}

			if(!this->vsuSoundSourceConfigurations[i].skippable)
			{
				continue;
			}

			if
			(
				0 > soonestFreeSoundSource 
				|| 
				this->vsuSoundSourceConfigurations[i].timeout < this->vsuSoundSourceConfigurations[soonestFreeSoundSource].timeout
			)
			{
				soonestFreeSoundSource = i;
			}
		}

		return soonestFreeSoundSource;
	}

	return -1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::releaseSoundSources()
{
	this->haveUsedSoundSources = false;

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		this->waveforms[i].usageCount = 0;
	}

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(0 > this->vsuSoundSourceConfigurations[i].timeout)
		{
			continue;
		}

		/// Don't change to >= since it prevents pop sounds when a new sound request
		/// arrives in during the same timer interrupt as this.
		if(this->ticks > this->vsuSoundSourceConfigurations[i].timeout)
		{
			this->vsuSoundSourceConfigurations[i].requester = NULL;
			this->vsuSoundSourceConfigurations[i].timeout = -1;
			this->vsuSoundSourceConfigurations[i].waveform = NULL;
			this->vsuSoundSourceConfigurations[i].SxINT |= __SOUND_WRAPPER_STOP_SOUND;
			this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxINT |= __SOUND_WRAPPER_STOP_SOUND;
		}
		else if(NULL != this->vsuSoundSourceConfigurations[i].waveform)
		{
			this->waveforms[this->vsuSoundSourceConfigurations[i].waveform->index].usageCount++;

			this->haveUsedSoundSources = true;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::dispatchQueuedSoundSourceConfigurations()
{
	if(isDeleted(this->queuedVSUSoundSourceConfigurationRequests) || NULL == this->queuedVSUSoundSourceConfigurationRequests->head)
	{
		return;
	}

	for(VirtualNode node = this->queuedVSUSoundSourceConfigurationRequests->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		VSUSoundSourceConfigurationRequest* queuedVSUSoundSourceConfigurationRequest = (VSUSoundSourceConfigurationRequest*)node->data;

		int16 vsuSoundSourceIndex = 
			VSUManager::findAvailableSoundSource
			(
				this, queuedVSUSoundSourceConfigurationRequest->requester, queuedVSUSoundSourceConfigurationRequest->type, 
				!queuedVSUSoundSourceConfigurationRequest->skippable
			);

		if(0 <= vsuSoundSourceIndex)
		{
			Waveform* waveform = VSUManager::findWaveform(this, queuedVSUSoundSourceConfigurationRequest->SxRAM);

			VSUManager::configureSoundSource(this, vsuSoundSourceIndex, queuedVSUSoundSourceConfigurationRequest, waveform);

			VirtualList::removeNode(this->queuedVSUSoundSourceConfigurationRequests, node);

			delete queuedVSUSoundSourceConfigurationRequest;
		}
	}

	this->haveQueuedRequests = NULL == this->queuedVSUSoundSourceConfigurationRequests->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::registerQueuedSoundSourceConfigurationRequest(const VSUSoundSourceConfigurationRequest* vsuSoundSourceConfigurationRequest)
{
	if(NULL == vsuSoundSourceConfigurationRequest || isDeleted(this->queuedVSUSoundSourceConfigurationRequests))
	{
		return;
	}

	this->haveQueuedRequests = true;

	VSUSoundSourceConfigurationRequest* queuedVSUSoundSourceConfigurationRequest = new VSUSoundSourceConfigurationRequest;
	*queuedVSUSoundSourceConfigurationRequest = *vsuSoundSourceConfigurationRequest;

	VirtualList::pushBack(this->queuedVSUSoundSourceConfigurationRequests, queuedVSUSoundSourceConfigurationRequest);

}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Waveform* VSUManager::findWaveform(const WaveformData* waveFormData)
{
	if(NULL == waveFormData)
	{
		return NULL;
	}

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		if(NULL != this->waveforms[i].data && waveFormData->crc == this->waveforms[i].crc)
		{
			return &this->waveforms[i];
		}
	}

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		if(NULL == this->waveforms[i].data || 0 == this->waveforms[i].usageCount)
		{
			VSUManager::setWaveform(this, &this->waveforms[i], waveFormData);

			return &this->waveforms[i];
		}
	}

	/// Fallback
	return &this->waveforms[0];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::setWaveform(Waveform* waveform, const WaveformData* waveFormData)
{
	if(NULL != waveform)// && waveform->overwrite)
	{
		waveform->usageCount = 1;
		waveform->data = waveFormData->data;
		waveform->crc = waveFormData->crc;
		waveform->overwrite = false;

		// Disable interrupts to make the following as soon as possible
		HardwareManager::suspendInterrupts();

		// Must stop all sound sources before writing the waveforms
		VSUManager::stopAllSounds(this);

		// Set the wave data
		for(uint32 i = 0; i < 32; i++)
		{
			waveform->wave[(i << 2)] = (uint8)waveform->data[i];
		}

		// Turn back interrupts on
		HardwareManager::resumeInterrupts();
		/*
		// TODO
		const uint8 kModData[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 18, 17, 18, 19, 20, 21, -1, -2, -3, -4, -5,
		-6, -7, -8, -9, -16, -17, -18, -19, -20, -21, -22
		};

		uint8* moddata = __MODULATION_DATA;
		for(i = 0; i <= 0x7C; i++)
		{
			moddata[i << 2] = kModData[i];
		}
		*/
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
