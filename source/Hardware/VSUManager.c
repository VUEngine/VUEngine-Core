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

#include <Printing.h>
#include <Profiler.h>
#include <VirtualList.h>
#include <WaveForms.h>

#include "VSUManager.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __WAVE_ADDRESS(n)					(uint8*)(0x01000000 + (n * 128))
#define __MODULATION_DATA					(uint8*)0x01000280;
#define __SSTOP								*(uint8*)0x01000580
#define __SOUND_WRAPPER_STOP_SOUND 			0x21
#define __MAXIMUM_VOLUME					0xF


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

VSUSoundSource* const _vsuSoundSources = (VSUSoundSource*)0x01000400;


//=========================================================================================================
// CLASS' PRIVATE STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void VSUManager::printVSUSoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration, int16 x, int y)
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
	PRINT_HEX_EXT(0x000FFFFF & (uint16)vsuSoundSourceConfiguration->SxRAM, x + 7, y, 2);

	PRINT_TEXT("SXSWP:         ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxSWP, x + 7, y, 2);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void VSUManager::applySoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	int16 vsuSoundSourceIndex = VSUManager::findAvailableSoundSource(this, vsuSoundSourceConfiguration->requester, vsuSoundSourceConfiguration->type, !vsuSoundSourceConfiguration->skippable);

	if(0 > vsuSoundSourceIndex)
	{
		if(this->allowQueueingSoundRequests && !vsuSoundSourceConfiguration->skippable)
		{
			VSUManager::registerQueuedSoundSourceConfiguration(this, vsuSoundSourceConfiguration);
		}
	}
	else
	{
		Waveform* waveform = VSUManager::findWaveform(this, vsuSoundSourceConfiguration->SxRAM);

		VSUManager::configureSoundSource(this, vsuSoundSourceIndex, vsuSoundSourceConfiguration, waveform);
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::applyPCMSampleToSoundSource(int8 sample)
{
	int16 vsuSoundSourceIndex = 0;
	
	while(true)
	{
		if(__MAXIMUM_VOLUME <= sample)
		{
			_vsuSoundSources[vsuSoundSourceIndex].SxLRV = 0xFF;
			sample -= __MAXIMUM_VOLUME;
		}
		else
		{
			_vsuSoundSources[vsuSoundSourceIndex].SxLRV = ((sample << 4) | sample);
			break;
		}

		vsuSoundSourceIndex++;
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::reset()
{
	VSUManager::stopAllSounds(this);
	VSUManager::enableQueue(this);

	this->ticks = 0;
	this->haveUsedSoundSources = false;
	this->haveQueuedRequests = false;

	VirtualList::deleteData(this->queuedVSUSoundSourceConfigurations);

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		this->waveforms[i].index = i;
		this->waveforms[i].usageCount = 0;
		this->waveforms[i].wave = __WAVE_ADDRESS(i);
		this->waveforms[i].data = NULL;
		this->waveforms[i].overwrite = true;

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

	for(int16 i = __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS + __TOTAL_NOISE_CHANNELS; i++)
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
		this->vsuSoundSourceConfigurations[i].SxRAM = kPlaybackPCM == this->playbackMode ? PCMWaveForm : NULL;
		this->vsuSoundSourceConfigurations[i].SxSWP = 0;
		this->vsuSoundSourceConfigurations[i].SxINT = kPlaybackPCM == this->playbackMode ? 0x9F : 0;

		Waveform* waveform = VSUManager::findWaveform(this, this->vsuSoundSourceConfigurations[i].SxRAM);

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
//---------------------------------------------------------------------------------------------------------
void VSUManager::setMode(uint32 playbackMode)
{
	this->playbackMode = playbackMode;

	VSUManager::reset(this);
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::update()
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
//---------------------------------------------------------------------------------------------------------
void VSUManager::stopAllSounds()
{
	__SSTOP = 0x01;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::enableQueue()
{
	this->allowQueueingSoundRequests = true;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::disableQueue()
{
	this->allowQueueingSoundRequests = false;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::flushQueuedSounds()
{
	VirtualList::deleteData(this->queuedVSUSoundSourceConfigurations);
}
//---------------------------------------------------------------------------------------------------------
#ifndef __SHIPPING
void VSUManager::print(int32 x, int32 y)
{
	int32 xDisplacement = 15;
	int32 yDisplacement = y;

	int32 i = 0;

	// Reset all sounds and channels
	for(i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		int32 y = yDisplacement;

		VSUManager::printVSUSoundSourceConfiguration(&this->vsuSoundSourceConfigurations[i], x, y);
		
		x += xDisplacement;
		if(x > 47 - xDisplacement)
		{
			x = 1;
			yDisplacement += 15;
		}
	}
}
#endif
//---------------------------------------------------------------------------------------------------------
#ifndef __RELEASE
void VSUManager::printWaveFormStatus(int32 x, int32 y)
{
	for(uint32 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		PRINT_TEXT("           ", x, y + this->waveforms[i].index);
		PRINT_INT(this->waveforms[i].index, x, y + this->waveforms[i].index);
		PRINT_INT(this->waveforms[i].usageCount, x + 4, y + this->waveforms[i].index);
		PRINT_HEX((uint32)this->waveforms[i].data, x + 8, y + this->waveforms[i].index);
	}
}
#endif
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void VSUManager::constructor()
{
	Base::constructor();

	this->queuedVSUSoundSourceConfigurations = new VirtualList();
	this->allowQueueingSoundRequests = true;
	this->targetPCMUpdates = 0;
	this->playbackMode = kPlaybackNative;
	this->haveUsedSoundSources = false;
	this->haveQueuedRequests = false;

	VSUManager::reset(this);
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::destructor()
{
	if(!isDeleted(this->queuedVSUSoundSourceConfigurations))
	{
		VirtualList::deleteData(this->queuedVSUSoundSourceConfigurations);
		delete this->queuedVSUSoundSourceConfigurations;
		this->queuedVSUSoundSourceConfigurations = NULL;
	}

	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::configureSoundSource(int16 vsuSoundSourceIndex, const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration, Waveform* waveform)
{
	int16 i = vsuSoundSourceIndex;
	VSUSoundSource* vsuSoundSource = this->vsuSoundSourceConfigurations[i].vsuSoundSource;

	this->haveUsedSoundSources = true;

	bool setSxINT = this->vsuSoundSourceConfigurations[i].SxINT != vsuSoundSourceConfiguration->SxINT;

	this->vsuSoundSourceConfigurations[i].requester = vsuSoundSourceConfiguration->requester;
	this->vsuSoundSourceConfigurations[i].waveform = waveform;
	this->vsuSoundSourceConfigurations[i].timeout = this->ticks + vsuSoundSourceConfiguration->timeout;
	this->vsuSoundSourceConfigurations[i].SxINT = vsuSoundSourceConfiguration->SxINT;
	this->vsuSoundSourceConfigurations[i].SxLRV = vsuSoundSourceConfiguration->SxLRV;
	this->vsuSoundSourceConfigurations[i].SxFQL = vsuSoundSourceConfiguration->SxFQL;
	this->vsuSoundSourceConfigurations[i].SxFQH = vsuSoundSourceConfiguration->SxFQH;
	this->vsuSoundSourceConfigurations[i].SxEV0 = vsuSoundSourceConfiguration->SxEV0;
	this->vsuSoundSourceConfigurations[i].SxEV1 = vsuSoundSourceConfiguration->SxEV1;
	this->vsuSoundSourceConfigurations[i].SxRAM = vsuSoundSourceConfiguration->SxRAM;
	this->vsuSoundSourceConfigurations[i].SxSWP = vsuSoundSourceConfiguration->SxSWP;
	this->vsuSoundSourceConfigurations[i].skippable = vsuSoundSourceConfiguration->skippable;

	vsuSoundSource->SxLRV = vsuSoundSourceConfiguration->SxLRV;
	vsuSoundSource->SxFQL = vsuSoundSourceConfiguration->SxFQL;
	vsuSoundSource->SxFQH = vsuSoundSourceConfiguration->SxFQH;
	vsuSoundSource->SxEV0 = vsuSoundSourceConfiguration->SxEV0;
	vsuSoundSource->SxEV1 = vsuSoundSourceConfiguration->SxEV1;
	vsuSoundSource->SxRAM = waveform->index;
	vsuSoundSource->SxSWP = vsuSoundSourceConfiguration->SxSWP;

	/// If SxINT is set every time it can produce the pop sound because it 
	/// resets various of the VSU's internal counters.
	if(setSxINT)
	{
		vsuSoundSource->SxINT = vsuSoundSourceConfiguration->SxINT;
	}
}
//---------------------------------------------------------------------------------------------------------
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

			if(0 > soonestFreeSoundSource || this->vsuSoundSourceConfigurations[i].timeout < this->vsuSoundSourceConfigurations[soonestFreeSoundSource].timeout)
			{
				soonestFreeSoundSource = i;
			}
		}

		return soonestFreeSoundSource;
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
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
		if(this->ticks >= this->vsuSoundSourceConfigurations[i].timeout)
		{
			this->vsuSoundSourceConfigurations[i].timeout = -1;
			this->vsuSoundSourceConfigurations[i].SxINT |= __SOUND_WRAPPER_STOP_SOUND;
			this->vsuSoundSourceConfigurations[i].waveform = NULL;
			this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxINT |= __SOUND_WRAPPER_STOP_SOUND;
			this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV1 |= 0x01;
		}
		else if(NULL != this->vsuSoundSourceConfigurations[i].waveform)
		{
			this->waveforms[this->vsuSoundSourceConfigurations[i].waveform->index].usageCount++;

			this->haveUsedSoundSources = true;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::dispatchQueuedSoundSourceConfigurations()
{
	if(isDeleted(this->queuedVSUSoundSourceConfigurations) || NULL == this->queuedVSUSoundSourceConfigurations->head)
	{
		return;
	}

	for(VirtualNode node = this->queuedVSUSoundSourceConfigurations->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		VSUSoundSourceConfiguration* queuedVSUSoundSourceConfiguration = (VSUSoundSourceConfiguration*)node->data;

		int16 vsuSoundSourceIndex = VSUManager::findAvailableSoundSource(this, queuedVSUSoundSourceConfiguration->requester, queuedVSUSoundSourceConfiguration->type, !queuedVSUSoundSourceConfiguration->skippable);

		if(0 <= vsuSoundSourceIndex)
		{
			Waveform* waveform = VSUManager::findWaveform(this, queuedVSUSoundSourceConfiguration->SxRAM);

			VSUManager::configureSoundSource(this, vsuSoundSourceIndex, queuedVSUSoundSourceConfiguration, waveform);

			VirtualList::removeNode(this->queuedVSUSoundSourceConfigurations, node);

			delete queuedVSUSoundSourceConfiguration;
		}
	}

	this->haveQueuedRequests = NULL == this->queuedVSUSoundSourceConfigurations->head;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::registerQueuedSoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	if(NULL == vsuSoundSourceConfiguration || isDeleted(this->queuedVSUSoundSourceConfigurations))
	{
		return;
	}

	this->haveQueuedRequests = true;

	VSUSoundSourceConfiguration* queuedVSUSoundSourceConfiguration = new VSUSoundSourceConfiguration;
	*queuedVSUSoundSourceConfiguration = *vsuSoundSourceConfiguration;

	VirtualList::pushBack(this->queuedVSUSoundSourceConfigurations, queuedVSUSoundSourceConfiguration);

}
//---------------------------------------------------------------------------------------------------------
Waveform* VSUManager::findWaveform(const int8* waveFormData)
{
	if(NULL == waveFormData)
	{
		return NULL;
	}

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		if(waveFormData == this->waveforms[i].data)
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
//---------------------------------------------------------------------------------------------------------
void VSUManager::setWaveform(Waveform* waveform, const int8* data)
{
	if(NULL != waveform)// && waveform->overwrite)
	{
		waveform->usageCount = 1;
		waveform->data = (int8*)data;
		waveform->overwrite = false;

		// Disable interrupts to make the following as soon as possible
		HardwareManager::suspendInterrupts();

		// Must stop all soundSpec before writing the waveforms
		VSUManager::suspendPlayingSounds(this);

		for(uint32 i = 0; i < 32; i++)
		{
			waveform->wave[(i << 2)] = (uint8)data[i];
		}

		// Resume playing sounds
		VSUManager::resumePlayingSounds(this);

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
//---------------------------------------------------------------------------------------------------------
void VSUManager::suspendPlayingSounds()
{
	for(int32 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV0 &= 0xF8;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV1 |= 0x01;
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::resumePlayingSounds()
{
}
//---------------------------------------------------------------------------------------------------------