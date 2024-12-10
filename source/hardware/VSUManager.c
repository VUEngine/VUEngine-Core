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
static int16 VSUManager::getSoundSourceType(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	uint32 soundSourceType = kSoundSourceNormal;

	if(0 != vsuSoundSourceConfiguration->SxSWP)
	{
		soundSourceType = kSoundSourceModulation;
	}
	else if(0 != vsuSoundSourceConfiguration->noise)
	{
		soundSourceType = kSoundSourceNoise;
	}

	return soundSourceType;
}
//---------------------------------------------------------------------------------------------------------
static void VSUManager::printVSUSoundSource(const VSUSoundSource* vsuSoundSource, int16 x, int y)
{
	if(NULL == vsuSoundSource)
	{
		return;
	}

	PRINT_TEXT("Source  :     ", x, ++y);
	PRINT_HEX((uint32)vsuSoundSource, x + 9, y);

	PRINT_TEXT("SxINT  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxINT, x + 9, y, 2);

	PRINT_TEXT("SxLRV  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxLRV, x + 9, y, 2);

	PRINT_TEXT("SxFQL  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxFQL, x + 9, y, 2);

	PRINT_TEXT("SxFQH  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxFQH, x + 9, y, 2);

	PRINT_TEXT("SxEV0  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxEV0, x + 9, y, 2);

	PRINT_TEXT("SxEV1  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxEV1, x + 9, y, 2);

	PRINT_TEXT("SxRAM  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxRAM, x + 9, y, 2);

	PRINT_TEXT("SxSWP  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSource->SxSWP, x + 9, y, 2);
}
//---------------------------------------------------------------------------------------------------------
static void VSUManager::printVSUSoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration, int16 x, int y)
{
	if(NULL == vsuSoundSourceConfiguration)
	{
		return;
	}

	PRINT_TEXT("Timeout  :     ", x, ++y);
	PRINT_INT(vsuSoundSourceConfiguration->timeout, x + 9, y);

	PRINT_TEXT("SxINT  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxINT, x + 9, y, 2);

	PRINT_TEXT("SxLRV  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxLRV, x + 9, y, 2);

	PRINT_TEXT("SxFQL  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxFQL, x + 9, y, 2);

	PRINT_TEXT("SxFQH  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxFQH, x + 9, y, 2);

	PRINT_TEXT("SxEV0  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxEV0, x + 9, y, 2);

	PRINT_TEXT("SxEV1  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxEV1, x + 9, y, 2);

	PRINT_TEXT("SxRAM  :     ", x, ++y);
	PRINT_HEX_EXT((uint32)vsuSoundSourceConfiguration->SxRAM, x + 9, y, 2);

	PRINT_TEXT("SxSWP  :     ", x, ++y);
	PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxSWP, x + 9, y, 2);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void VSUManager::applySoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	int16 vsuSoundSourceIndex = VSUManager::findAvailableSoundSource(this, VSUManager::getSoundSourceType(vsuSoundSourceConfiguration));

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
		this->vsuSoundSourceConfigurations[i].type = kSoundSourceModulation;
	}

	for(int16 i = __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS + __TOTAL_NOISE_CHANNELS; i++)
	{
		this->vsuSoundSourceConfigurations[i].type = kSoundSourceNoise;
	}

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		this->vsuSoundSourceConfigurations[i].vsuSoundSource = &_vsuSoundSources[i];
		this->vsuSoundSourceConfigurations[i].timeout = -1;
		this->vsuSoundSourceConfigurations[i].SxLRV = 0;
		this->vsuSoundSourceConfigurations[i].SxFQL = 0;
		this->vsuSoundSourceConfigurations[i].SxFQH = 0;
		this->vsuSoundSourceConfigurations[i].SxEV0 = kPlaybackPCM == this->playbackMode ? 0xFF : 0x00;
		this->vsuSoundSourceConfigurations[i].SxEV1 = 0;
		this->vsuSoundSourceConfigurations[i].SxRAM = kPlaybackPCM == this->playbackMode ? PCMWaveForm : NULL;
		this->vsuSoundSourceConfigurations[i].SxSWP = 0;
		this->vsuSoundSourceConfigurations[i].noise = false;
		this->vsuSoundSourceConfigurations[i].SxINT = kPlaybackPCM == this->playbackMode ? 0x9F : 0;

		Waveform* waveform = VSUManager::findWaveform(this, this->vsuSoundSourceConfigurations[i].SxRAM);

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
	int32 xDisplacement = 8;
	int32 yDisplacement = y;

	int32 i = 0;

	// Reset all sounds and channels
	for(i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		VSUSoundSourceConfiguration* vsuSoundSourceConfiguration = &this->vsuSoundSourceConfigurations[i];

		int32 y = yDisplacement;

		PRINT_TEXT("SOURCE ", x, y);
		PRINT_INT(i, x + xDisplacement, y);

		PRINT_TEXT("Type   : ", x, ++y);

		char* channelType = "?";
		switch(vsuSoundSourceConfiguration->type)
		{
			case kSoundSourceNormal:

				channelType = "Normal";
				break;

			case kSoundSourceModulation:

				channelType = "Modulation";
				break;

			case kSoundSourceNoise:

				channelType = "Noise";
				break;
		}
		PRINT_TEXT(channelType, x + xDisplacement, y);


		PRINT_TEXT("Timeout  :     ", x, ++y);
		PRINT_INT(vsuSoundSourceConfiguration->timeout, x + 9, y);

		PRINT_TEXT("SxINT  :     ", x, ++y);
		PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxINT, x + 9, y, 2);

		PRINT_TEXT("SxLRV  :     ", x, ++y);
		PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxLRV, x + 9, y, 2);

		PRINT_TEXT("SxFQL  :     ", x, ++y);
		PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxFQL, x + 9, y, 2);

		PRINT_TEXT("SxFQH  :     ", x, ++y);
		PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxFQH, x + 9, y, 2);

		PRINT_TEXT("SxEV0  :     ", x, ++y);
		PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxEV0, x + 9, y, 2);

		PRINT_TEXT("SxEV1  :     ", x, ++y);
		PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxEV1, x + 9, y, 2);

		PRINT_TEXT("SxRAM  :     ", x, ++y);
		PRINT_HEX_EXT((uint32)vsuSoundSourceConfiguration->SxRAM, x + 9, y, 2);

		PRINT_TEXT("SxSWP  :     ", x, ++y);
		PRINT_HEX_EXT(vsuSoundSourceConfiguration->SxSWP, x + 9, y, 2);
		
		x += 16;

		if(x > 33)
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
	this->allowQueueingSoundRequests = false;
	this->targetPCMUpdates = 0;
	this->playbackMode = kPlaybackNative;
	this->allowQueueingSoundRequests = true;
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

	this->vsuSoundSourceConfigurations[i].timeout = this->ticks + vsuSoundSourceConfiguration->timeout;
	this->vsuSoundSourceConfigurations[i].SxLRV = vsuSoundSourceConfiguration->SxLRV;
	this->vsuSoundSourceConfigurations[i].SxFQL = vsuSoundSourceConfiguration->SxFQL;
	this->vsuSoundSourceConfigurations[i].SxFQH = vsuSoundSourceConfiguration->SxFQH;
	this->vsuSoundSourceConfigurations[i].SxEV0 = vsuSoundSourceConfiguration->SxEV0;
	this->vsuSoundSourceConfigurations[i].SxEV1 = vsuSoundSourceConfiguration->SxEV1;
	this->vsuSoundSourceConfigurations[i].SxRAM = vsuSoundSourceConfiguration->SxRAM;
	this->vsuSoundSourceConfigurations[i].SxSWP = vsuSoundSourceConfiguration->SxSWP;
	this->vsuSoundSourceConfigurations[i].SxINT = vsuSoundSourceConfiguration->SxINT;

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

//	VSUManager::printVSUSoundSourceConfiguration(&this->vsuSoundSourceConfigurations[vsuSoundSourceIndex], 1 + 20 * vsuSoundSourceIndex, 10);
//	VSUManager::printVSUSoundSource(vsuSoundSource, 20, 10);
}
//---------------------------------------------------------------------------------------------------------
int16 VSUManager::findAvailableSoundSource(uint32 soundSourceType)
{
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(soundSourceType != this->vsuSoundSourceConfigurations[i].type)
		{
			continue;
		}
		
		if(this->ticks >= this->vsuSoundSourceConfigurations[i].timeout)
		{
			return i;
		}
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::releaseSoundSources()
{
	this->haveUsedSoundSources = false;

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
			this->vsuSoundSourceConfigurations[i].timeout = -1;

			this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxEV1 |= 0x01;
			
			VSUManager::releaseWaveform(this, this->vsuSoundSourceConfigurations[i].vsuSoundSource->SxRAM, this->vsuSoundSourceConfigurations[i].SxRAM);
		}
		else
		{
			this->haveUsedSoundSources = true;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::releaseWaveform(int8 waveFormIndex, const int8* waveFormData)
{
	if(0 <= waveFormIndex && waveFormIndex < __TOTAL_WAVEFORMS)
	{
		if(NULL == waveFormData || this->waveforms[waveFormIndex].data == waveFormData)
		{
			this->waveforms[waveFormIndex].usageCount -= 1;

			if(0 >= this->waveforms[waveFormIndex].usageCount)
			{
				this->waveforms[waveFormIndex].usageCount = 0;
			}
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

		int16 vsuSoundSourceIndex = VSUManager::findAvailableSoundSource(this, VSUManager::getSoundSourceType(queuedVSUSoundSourceConfiguration));

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
		if(this->waveforms[i].data == waveFormData)
		{
			this->waveforms[i].usageCount++;

			return &this->waveforms[i];
		}
	}

	for(int16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		if(0 == this->waveforms[i].usageCount)
		{
			this->waveforms[i].usageCount = 1;

			this->waveforms[i].data = waveFormData;

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