/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with vsuManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Printing.h>
#include <Profiler.h>
#include <VirtualList.h>
#include <WaveForms.h>

#include "VSUManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __WAVE_ADDRESS(n)					(uint8*)(0x01000000 + (n * 128))
#define __MODULATION_DATA					(uint8*)0x01000280;
#define __SSTOP								*(uint8*)0x01000580
#define __SOUND_WRAPPER_STOP_SOUND 			0x21
#define __MAXIMUM_VOLUME					0xF

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VSUSoundSource* const _vsuSoundSources = (VSUSoundSource*)0x01000400;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::applySoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	int16 vsuSoundSourceIndex = 
		VSUManager::findAvailableSoundSource
		(
			vsuSoundSourceConfiguration->requester, vsuSoundSourceConfiguration->type, !vsuSoundSourceConfiguration->skippable
		);

	if(0 > vsuSoundSourceIndex)
	{
		if(vsuManager->allowQueueingSoundRequests && !vsuSoundSourceConfiguration->skippable)
		{
			VSUManager::registerQueuedSoundSourceConfiguration(vsuSoundSourceConfiguration);
		}
	}
	else
	{
		Waveform* waveform = VSUManager::findWaveform(vsuSoundSourceConfiguration->SxRAM);

		VSUManager::configureSoundSource(vsuSoundSourceIndex, vsuSoundSourceConfiguration, waveform);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::applyPCMSampleToSoundSource(int8 sample)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::reset()
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	VSUManager::stopAllSounds();
	VSUManager::enableQueue();

	vsuManager->ticks = 0;
	vsuManager->haveUsedSoundSources = false;
	vsuManager->haveQueuedRequests = false;

	VirtualList::deleteData(vsuManager->queuedVSUSoundSourceConfigurations);

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		vsuManager->waveforms[i].index = i;
		vsuManager->waveforms[i].usageCount = 0;
		vsuManager->waveforms[i].wave = __WAVE_ADDRESS(i);
		vsuManager->waveforms[i].data = NULL;
		vsuManager->waveforms[i].overwrite = true;

		for(uint32 j = 0; j < 128; j++)
		{
			vsuManager->waveforms[i].wave[j] = 0;
		}
	}

	uint8* modulationData = __MODULATION_DATA;

	for(int16 i = 0; i <= 32 * 4; i++)
	{
		modulationData[i] = 0;
	}

	for(int16 i = 0; i < __TOTAL_NORMAL_CHANNELS; i++)
	{
		vsuManager->vsuSoundSourceConfigurations[i].type = kSoundSourceNormal;
	}

	for(int16 i = __TOTAL_NORMAL_CHANNELS; i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; i++)
	{
		vsuManager->vsuSoundSourceConfigurations[i].type = kSoundSourceModulation | kSoundSourceNormal;
	}

	for
	(
		int16 i = __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; 
		i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS + __TOTAL_NOISE_CHANNELS; 
		i++
	)
	{
		vsuManager->vsuSoundSourceConfigurations[i].type = kSoundSourceNoise;
	}

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		vsuManager->vsuSoundSourceConfigurations[i].requester = NULL;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource = &_vsuSoundSources[i];
		vsuManager->vsuSoundSourceConfigurations[i].waveform = NULL;
		vsuManager->vsuSoundSourceConfigurations[i].timeout = -1;
		vsuManager->vsuSoundSourceConfigurations[i].SxLRV = 0;
		vsuManager->vsuSoundSourceConfigurations[i].SxFQL = 0;
		vsuManager->vsuSoundSourceConfigurations[i].SxFQH = 0;
		vsuManager->vsuSoundSourceConfigurations[i].SxEV0 = kPlaybackPCM == vsuManager->playbackMode ? 0xFF : 0x00;
		vsuManager->vsuSoundSourceConfigurations[i].SxEV1 = 0;
		vsuManager->vsuSoundSourceConfigurations[i].SxRAM = kPlaybackPCM == vsuManager->playbackMode ? PCMWaveForm : NULL;
		vsuManager->vsuSoundSourceConfigurations[i].SxSWP = 0;
		vsuManager->vsuSoundSourceConfigurations[i].SxINT = kPlaybackPCM == vsuManager->playbackMode ? 0x9F : 0;

		Waveform* waveform = VSUManager::findWaveform(vsuManager->vsuSoundSourceConfigurations[i].SxRAM);

		vsuManager->vsuSoundSourceConfigurations[i].waveform = waveform;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxLRV = vsuManager->vsuSoundSourceConfigurations[i].SxLRV;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxFQL = vsuManager->vsuSoundSourceConfigurations[i].SxFQL;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxFQH = vsuManager->vsuSoundSourceConfigurations[i].SxFQH;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV0 = vsuManager->vsuSoundSourceConfigurations[i].SxEV0;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV1 = vsuManager->vsuSoundSourceConfigurations[i].SxEV1;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxRAM = NULL == waveform ? 0 : waveform->index;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxSWP = vsuManager->vsuSoundSourceConfigurations[i].SxSWP;
		vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxINT = vsuManager->vsuSoundSourceConfigurations[i].SxINT;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::setMode(uint32 playbackMode)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	vsuManager->playbackMode = playbackMode;

	VSUManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::update()
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	if(vsuManager->haveUsedSoundSources)
	{
		VSUManager::releaseSoundSources();
	}

	if(vsuManager->haveQueuedRequests)
	{
		VSUManager::dispatchQueuedSoundSourceConfigurations();
	}

	vsuManager->ticks += __I_TO_FIX7_9_EXT(1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::stopAllSounds()
{
	__SSTOP = 0x01;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::enableQueue()
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	vsuManager->allowQueueingSoundRequests = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::disableQueue()
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	vsuManager->allowQueueingSoundRequests = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::flushQueuedSounds()
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	VirtualList::deleteData(vsuManager->queuedVSUSoundSourceConfigurations);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void VSUManager::print(int32 x, int32 y)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

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

#ifndef __RELEASE
static void VSUManager::printWaveFormStatus(int32 x, int32 y)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::configureSoundSource
(
	int16 vsuSoundSourceIndex, const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration, Waveform* waveform
)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	int16 i = vsuSoundSourceIndex;
	VSUSoundSource* vsuSoundSource = vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource;

	vsuManager->haveUsedSoundSources = true;

	bool setSxINT = vsuManager->vsuSoundSourceConfigurations[i].SxINT != vsuSoundSourceConfiguration->SxINT;

	vsuManager->vsuSoundSourceConfigurations[i].requester = vsuSoundSourceConfiguration->requester;
	vsuManager->vsuSoundSourceConfigurations[i].waveform = waveform;
	vsuManager->vsuSoundSourceConfigurations[i].timeout = vsuManager->ticks + vsuSoundSourceConfiguration->timeout;
	vsuManager->vsuSoundSourceConfigurations[i].SxINT = vsuSoundSourceConfiguration->SxINT;
	vsuManager->vsuSoundSourceConfigurations[i].SxLRV = vsuSoundSourceConfiguration->SxLRV;
	vsuManager->vsuSoundSourceConfigurations[i].SxFQL = vsuSoundSourceConfiguration->SxFQL;
	vsuManager->vsuSoundSourceConfigurations[i].SxFQH = vsuSoundSourceConfiguration->SxFQH;
	vsuManager->vsuSoundSourceConfigurations[i].SxEV0 = vsuSoundSourceConfiguration->SxEV0;
	vsuManager->vsuSoundSourceConfigurations[i].SxEV1 = vsuSoundSourceConfiguration->SxEV1;
	vsuManager->vsuSoundSourceConfigurations[i].SxRAM = vsuSoundSourceConfiguration->SxRAM;
	vsuManager->vsuSoundSourceConfigurations[i].SxSWP = vsuSoundSourceConfiguration->SxSWP;
	vsuManager->vsuSoundSourceConfigurations[i].skippable = vsuSoundSourceConfiguration->skippable;

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 VSUManager::findAvailableSoundSource(Object requester, uint32 soundSourceType, bool force)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	// First try to find a sound source that has previously assigned to the same requester
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(0 == (soundSourceType & vsuManager->vsuSoundSourceConfigurations[i].type))
		{
			continue;
		}

		if(requester == vsuManager->vsuSoundSourceConfigurations[i].requester)
		{
		 	return i;
		}
	}

	// Now try to find a sound source whose timeout has just expired
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(0 == (soundSourceType & vsuManager->vsuSoundSourceConfigurations[i].type))
		{
			continue;
		}

		if(vsuManager->ticks >= vsuManager->vsuSoundSourceConfigurations[i].timeout)
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
			if(0 == (soundSourceType & vsuManager->vsuSoundSourceConfigurations[i].type))
			{
				continue;
			}

			if(!vsuManager->vsuSoundSourceConfigurations[i].skippable)
			{
				continue;
			}

			if
			(
				0 > soonestFreeSoundSource 
				|| 
				vsuManager->vsuSoundSourceConfigurations[i].timeout < vsuManager->vsuSoundSourceConfigurations[soonestFreeSoundSource].timeout
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

static void VSUManager::releaseSoundSources()
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	vsuManager->haveUsedSoundSources = false;

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		vsuManager->waveforms[i].usageCount = 0;
	}

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(0 > vsuManager->vsuSoundSourceConfigurations[i].timeout)
		{
			continue;
		}

		/// Don't change to >= since it prevents pop sounds when a new sound request
		/// arrives in during the same timer interrupt as vsuManager.
		if(vsuManager->ticks >= vsuManager->vsuSoundSourceConfigurations[i].timeout)
		{
			vsuManager->vsuSoundSourceConfigurations[i].timeout = -1;
			vsuManager->vsuSoundSourceConfigurations[i].SxINT |= __SOUND_WRAPPER_STOP_SOUND;
			vsuManager->vsuSoundSourceConfigurations[i].waveform = NULL;
			vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV1 |= 0x01;
		}
		else if(NULL != vsuManager->vsuSoundSourceConfigurations[i].waveform)
		{
			vsuManager->waveforms[vsuManager->vsuSoundSourceConfigurations[i].waveform->index].usageCount++;

			vsuManager->haveUsedSoundSources = true;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::dispatchQueuedSoundSourceConfigurations()
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	if(isDeleted(vsuManager->queuedVSUSoundSourceConfigurations) || NULL == vsuManager->queuedVSUSoundSourceConfigurations->head)
	{
		return;
	}

	for(VirtualNode node = vsuManager->queuedVSUSoundSourceConfigurations->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		VSUSoundSourceConfiguration* queuedVSUSoundSourceConfiguration = (VSUSoundSourceConfiguration*)node->data;

		int16 vsuSoundSourceIndex = 
			VSUManager::findAvailableSoundSource
			(
				queuedVSUSoundSourceConfiguration->requester, queuedVSUSoundSourceConfiguration->type, 
				!queuedVSUSoundSourceConfiguration->skippable
			);

		if(0 <= vsuSoundSourceIndex)
		{
			Waveform* waveform = VSUManager::findWaveform(queuedVSUSoundSourceConfiguration->SxRAM);

			VSUManager::configureSoundSource(vsuSoundSourceIndex, queuedVSUSoundSourceConfiguration, waveform);

			VirtualList::removeNode(vsuManager->queuedVSUSoundSourceConfigurations, node);

			delete queuedVSUSoundSourceConfiguration;
		}
	}

	vsuManager->haveQueuedRequests = NULL == vsuManager->queuedVSUSoundSourceConfigurations->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::registerQueuedSoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	if(NULL == vsuSoundSourceConfiguration || isDeleted(vsuManager->queuedVSUSoundSourceConfigurations))
	{
		return;
	}

	vsuManager->haveQueuedRequests = true;

	VSUSoundSourceConfiguration* queuedVSUSoundSourceConfiguration = new VSUSoundSourceConfiguration;
	*queuedVSUSoundSourceConfiguration = *vsuSoundSourceConfiguration;

	VirtualList::pushBack(vsuManager->queuedVSUSoundSourceConfigurations, queuedVSUSoundSourceConfiguration);

}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Waveform* VSUManager::findWaveform(const int8* waveFormData)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	if(NULL == waveFormData)
	{
		return NULL;
	}

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		if(waveFormData == vsuManager->waveforms[i].data)
		{
			return &vsuManager->waveforms[i];
		}
	}

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		if(NULL == vsuManager->waveforms[i].data || 0 == vsuManager->waveforms[i].usageCount)
		{
			VSUManager::setWaveform(&vsuManager->waveforms[i], waveFormData);

			return &vsuManager->waveforms[i];
		}
	}

	/// Fallback
	return &vsuManager->waveforms[0];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VSUManager::setWaveform(Waveform* waveform, const int8* data)
{
	VSUManager vsuManager = VSUManager::getInstance(NULL);

	if(NULL != waveform)// && waveform->overwrite)
	{
		waveform->usageCount = 1;
		waveform->data = (int8*)data;
		waveform->overwrite = false;

		// Disable interrupts to make the following as soon as possible
		HardwareManager::suspendInterrupts();

		// Must stop all sound sources before writing the waveforms
		for(int32 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
		{
			vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV1 |= 0x01;
		}

		// Set the wave data
		for(uint32 i = 0; i < 32; i++)
		{
			waveform->wave[(i << 2)] = (uint8)data[i];
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->queuedVSUSoundSourceConfigurations = new VirtualList();
	this->allowQueueingSoundRequests = true;
	this->targetPCMUpdates = 0;
	this->playbackMode = kPlaybackNative;
	this->haveUsedSoundSources = false;
	this->haveQueuedRequests = false;

	VSUManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VSUManager::destructor()
{
	if(!isDeleted(this->queuedVSUSoundSourceConfigurations))
	{
		VirtualList::deleteData(this->queuedVSUSoundSourceConfigurations);
		delete this->queuedVSUSoundSourceConfigurations;
		this->queuedVSUSoundSourceConfigurations = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
