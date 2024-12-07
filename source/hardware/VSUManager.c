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


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

VSUSoundSource* const _vsuSoundSources = (VSUSoundSource*)0x01000400;


//=========================================================================================================
// CLASS' PRIVATE STATIC METHODS
//=========================================================================================================

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
	int16 vsuSoundSourceIndex = VSUManager::findAvailableSoundSource(this);

	if(0 > vsuSoundSourceIndex)
	{
//		VSUManager::queueSoundSourceConfiguration(this, &vsuSoundSourceConfiguration);
	}
	else
	{
		VSUManager::configureSoundSource(this, vsuSoundSourceIndex, vsuSoundSourceConfiguration);
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::reset()
{
	this->ticks = 0;
	VirtualList::deleteData(this->queuedVSUSoundSourceConfigurations);

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		VSUSoundSource* vsuSoundSource = &_vsuSoundSources[i];

		this->vsuSoundSourceConfigurations[i].vsuSoundSource = vsuSoundSource;
		this->vsuSoundSourceConfigurations[i].timeout = 0;
		this->vsuSoundSourceConfigurations[i].SxINT = vsuSoundSource->SxINT = 0;
		this->vsuSoundSourceConfigurations[i].SxLRV = vsuSoundSource->SxLRV = 0;
		this->vsuSoundSourceConfigurations[i].SxFQL = vsuSoundSource->SxFQL = 0;
		this->vsuSoundSourceConfigurations[i].SxFQH = vsuSoundSource->SxFQH = 0;
		this->vsuSoundSourceConfigurations[i].SxEV0 = vsuSoundSource->SxEV0 = 0;
		this->vsuSoundSourceConfigurations[i].SxEV1 = vsuSoundSource->SxEV1 = 0;
		this->vsuSoundSourceConfigurations[i].SxRAM = vsuSoundSource->SxRAM = 0;
		this->vsuSoundSourceConfigurations[i].SxSWP = vsuSoundSource->SxSWP = 0;
	}

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

	VSUManager::stopAllSounds(this);
	VSUManager::unlock(this);
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::update()
{
	this->ticks += __I_TO_FIX7_9_EXT(1);
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::stopAllSounds()
{
	__SSTOP = 0x01;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::lock()
{
	this->lock = true;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::unlock()
{
	this->lock = false;
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
	this->lock = false;
	this->targetPCMUpdates = 0;

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
void VSUManager::configureSoundSource(int16 vsuSoundSourceIndex, const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	VSUSoundSource* vsuSoundSource = this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].vsuSoundSource;

	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].timeout = this->ticks + vsuSoundSourceConfiguration->timeout;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxINT = vsuSoundSourceConfiguration->SxINT;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxLRV = vsuSoundSourceConfiguration->SxLRV;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxFQL = vsuSoundSourceConfiguration->SxFQL;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxFQH = vsuSoundSourceConfiguration->SxFQH;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxEV0 = vsuSoundSourceConfiguration->SxEV0;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxEV1 = vsuSoundSourceConfiguration->SxEV1;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxRAM = vsuSoundSourceConfiguration->SxRAM;
	this->vsuSoundSourceConfigurations[vsuSoundSourceIndex].SxSWP = vsuSoundSourceConfiguration->SxSWP;

	vsuSoundSource->SxINT = vsuSoundSourceConfiguration->SxINT;
	vsuSoundSource->SxLRV = vsuSoundSourceConfiguration->SxLRV;
	vsuSoundSource->SxFQL = vsuSoundSourceConfiguration->SxFQL;
	vsuSoundSource->SxFQH = vsuSoundSourceConfiguration->SxFQH;
	vsuSoundSource->SxEV0 = vsuSoundSourceConfiguration->SxEV0;
	vsuSoundSource->SxEV1 = vsuSoundSourceConfiguration->SxEV1;
	vsuSoundSource->SxRAM = VSUManager::findWaveform(this, vsuSoundSourceConfiguration->SxRAM)->index;
	vsuSoundSource->SxSWP = vsuSoundSourceConfiguration->SxSWP;

//	VSUManager::printVSUSoundSourceConfiguration(&this->vsuSoundSourceConfigurations[vsuSoundSourceIndex], 1, 10);
//	VSUManager::printVSUSoundSource(vsuSoundSource, 20, 10);
}
//---------------------------------------------------------------------------------------------------------
int16 VSUManager::findAvailableSoundSource()
{
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(this->ticks >= this->vsuSoundSourceConfigurations[i].timeout)
		{
			return i;
		}
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::queueSoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration)
{
	if(NULL == vsuSoundSourceConfiguration || isDeleted(this->queuedVSUSoundSourceConfigurations))
	{
		return;
	}

	VSUSoundSourceConfiguration* queuedVSUSoundSourceConfiguration = new VSUSoundSourceConfiguration;
	*queuedVSUSoundSourceConfiguration = *vsuSoundSourceConfiguration;
	queuedVSUSoundSourceConfiguration->timeout = this->ticks + vsuSoundSourceConfiguration->timeout;

	VirtualList::pushBack(this->queuedVSUSoundSourceConfigurations, queuedVSUSoundSourceConfiguration);

}
//---------------------------------------------------------------------------------------------------------
Waveform* VSUManager::findWaveform(const int8* waveFormData)
{
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
void VSUManager::releaseWaveform(int8 waveFormIndex, const int8* waveFormData)
{
	if(0 <= waveFormIndex && waveFormIndex < __TOTAL_SOUND_SOURCES)
	{
		if(NULL == waveFormData || this->waveforms[waveFormIndex].data == waveFormData)
		{
			this->waveforms[waveFormIndex].usageCount -= 1;

			if(0 >= this->waveforms[waveFormIndex].usageCount)
			{
				this->waveforms[waveFormIndex].usageCount = 0;
			}
		}
		else
		{
#ifndef __RELEASE
			Printing::setDebugMode(Printing::getInstance());
			Printing::clear(Printing::getInstance());
			Printing::text(Printing::getInstance(), "Waveform index: ", 1, 12, NULL);
			Printing::int32(Printing::getInstance(), waveFormIndex, 18, 12, NULL);
			Printing::text(Printing::getInstance(), "Waveform data: ", 1, 13, NULL);
			Printing::hex(Printing::getInstance(), (int32)waveFormData, 18, 13, 8, NULL);
			Printing::text(Printing::getInstance(), "Waveform data[]: ", 1, 14, NULL);
			Printing::hex(Printing::getInstance(), (int32)this->waveforms[waveFormIndex].data, 18, 14, 8, NULL);
#endif
			NM_ASSERT(false, "VSUManager::releaseWaveform: mismatch between index and data");
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::suspendPlayingSounds()
{
	for(int32 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		_vsuSoundSources[i].SxINT |= __SOUND_WRAPPER_STOP_SOUND;
		_vsuSoundSources[i].SxLRV = 0x00;
	}
}
//---------------------------------------------------------------------------------------------------------
void VSUManager::resumePlayingSounds()
{
}
//---------------------------------------------------------------------------------------------------------