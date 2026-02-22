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

#include <Singleton.h>
#include <VirtualList.h>

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
#define __MODULATION_DATA					(uint8*)0x01000280
#define __MODULATION_DATA_ENTRIES			32
#define __SSTOP								*(uint8*)0x01000580
#define __SOUND_WRAPPER_STOP_SOUND 			0x20

// The following flags must use unused bits in their corresponding
// VSU registers to not clash with the hardware meanings
#define __SET_SxINT_FLAG					0x40
#define __SET_SxEV0_FLAG					0x80
#define __SET_SxEV1_FLAG					0x80
#define __SET_SxSWP_FLAG					0x08


#undef __CHAR_DARK_RED_BOX
#define __CHAR_DARK_RED_BOX					'\x0E'
#undef __CHAR_BRIGHT_RED_BOX
#define __CHAR_BRIGHT_RED_BOX				'\x10'


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
		VSUManager::freeableSoundSource
		(
			vsuManager, vsuSoundSourceConfigurationRequest->requesterId, vsuSoundSourceConfigurationRequest->type, 
			vsuSoundSourceConfigurationRequest->priority, vsuSoundSourceConfigurationRequest->skip
		);

	if(0 > vsuSoundSourceIndex)
	{
		if(vsuManager->allowQueueingSoundRequests && !vsuSoundSourceConfigurationRequest->skip)
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

static void VSUManager::stopSoundSourcesUsedBy(uint32 requesterId)
{
	VSUManager vsuManager = VSUManager::getInstance();

	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(requesterId == vsuManager->vsuSoundSourceConfigurations[i].requesterId)
		{
			vsuManager->vsuSoundSourceConfigurations[i].timeout = -1;
			vsuManager->vsuSoundSourceConfigurations[i].waveform = NULL;
			vsuManager->vsuSoundSourceConfigurations[i].SxINT = 0;
			vsuManager->vsuSoundSourceConfigurations[i].vsuSoundSource->SxINT = 0;
		}
	}	

	for(VirtualNode node = vsuManager->queuedVSUSoundSourceConfigurationRequests->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		VSUSoundSourceConfigurationRequest* queuedVSUSoundSourceConfigurationRequest = (VSUSoundSourceConfigurationRequest*)node->data;

		if(requesterId == queuedVSUSoundSourceConfigurationRequest->requesterId)
		{
			VirtualList::removeNode(vsuManager->queuedVSUSoundSourceConfigurationRequests, node);
			delete queuedVSUSoundSourceConfigurationRequest;
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

#ifndef __SHIPPING
static void VSUManager::printChannels(int32 x, int32 y)
{
	VSUManager vsuManager = VSUManager::getInstance();

	uint16 totalVolume = 0;

	for(uint16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		uint16 volume = vsuManager->vsuSoundSourceConfigurations[i].SxLRV;

		totalVolume += volume;

		uint16 leftVolume = (volume) >> 4;
		uint16 rightVolume = (volume & 0x0F);

		uint16 frequency = (vsuManager->vsuSoundSourceConfigurations[i].SxFQH << 4) | vsuManager->vsuSoundSourceConfigurations[i].SxFQL;

		uint16 leftValue = 0;
		uint16 rightValue = 0;

		leftValue = ((frequency * leftVolume) / __MAXIMUM_VOLUME) >> 4;
		rightValue = ((frequency * rightVolume) / __MAXIMUM_VOLUME) >> 4;

		char boxesArray[] = 
		{
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			'C', '0' + i + 1,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			'\0'
		};

		for(uint16 j = 0; j < leftValue && 15 > j; j++)
		{
			boxesArray[15 - j - 1] = __CHAR_BRIGHT_RED_BOX;
		}

		for(uint16 j = 0; j < rightValue && 15 > j; j++)
		{
			boxesArray[15 + 2 + j] = __CHAR_BRIGHT_RED_BOX;
		}

		PRINT_TEXT(boxesArray, x, y);

		y++;
	}
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
		this->waveforms[i].wave = __WAVE_ADDRESS(i);
		this->waveforms[i].data = NULL;
		this->waveforms[i].crc = 0;

		for(uint32 j = 0; j < 128; j++)
		{
			this->waveforms[i].wave[j] = 0;
		}
	}

	uint8* modulationData = __MODULATION_DATA;

	for(int16 i = 0; i <= __MODULATION_DATA_ENTRIES; i++)
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
		this->vsuSoundSourceConfigurations[i].requesterId = -1;
		this->vsuSoundSourceConfigurations[i].vsuSoundSource = &_vsuSoundSources[i];
		this->vsuSoundSourceConfigurations[i].waveform = NULL;
		this->vsuSoundSourceConfigurations[i].timeout = -1;
		this->vsuSoundSourceConfigurations[i].SxLRV = 0;
		this->vsuSoundSourceConfigurations[i].SxFQL = 0;
		this->vsuSoundSourceConfigurations[i].SxFQH = 0;
		this->vsuSoundSourceConfigurations[i].SxEV0 = 0;
		this->vsuSoundSourceConfigurations[i].SxEV1 = 0;
		this->vsuSoundSourceConfigurations[i].SxRAM = 0;
		this->vsuSoundSourceConfigurations[i].SxSWP = 0;
		this->vsuSoundSourceConfigurations[i].SxINT = 0;

		Waveform* waveform = VSUManager::findWaveform(this, NULL);

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
		this->vsuSoundSourceConfigurations[i].requesterId = -1;
		this->vsuSoundSourceConfigurations[i].waveform = NULL;
		this->vsuSoundSourceConfigurations[i].timeout = -1;
		this->vsuSoundSourceConfigurations[i].SxLRV = 0;
		this->vsuSoundSourceConfigurations[i].SxFQL = 0;
		this->vsuSoundSourceConfigurations[i].SxFQH = 0;
		this->vsuSoundSourceConfigurations[i].SxEV0 = 0;
		this->vsuSoundSourceConfigurations[i].SxEV1 = 0;
		this->vsuSoundSourceConfigurations[i].SxRAM = 0;
		this->vsuSoundSourceConfigurations[i].SxSWP = 0;
		this->vsuSoundSourceConfigurations[i].SxINT = 0;
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
		0 != (__SET_SxINT_FLAG & vsuSoundSourceConfigurationRequest->SxINT)
		|| 
		this->vsuSoundSourceConfigurations[i].SxINT != vsuSoundSourceConfigurationRequest->SxINT
		|| 
		(this->vsuSoundSourceConfigurations[i].requesterId != vsuSoundSourceConfigurationRequest->requesterId);

	bool setSxEV0 = 
		0 != (__SET_SxEV0_FLAG & vsuSoundSourceConfigurationRequest->SxEV1)
		||
		this->vsuSoundSourceConfigurations[i].SxEV0 != vsuSoundSourceConfigurationRequest->SxEV0
		||
		(this->vsuSoundSourceConfigurations[i].requesterId != vsuSoundSourceConfigurationRequest->requesterId);

	bool setSxEV1 = 
		0 != (__SET_SxEV1_FLAG & vsuSoundSourceConfigurationRequest->SxEV1)
		||
		this->vsuSoundSourceConfigurations[i].SxEV1 != vsuSoundSourceConfigurationRequest->SxEV1
		|| 
		(this->vsuSoundSourceConfigurations[i].requesterId != vsuSoundSourceConfigurationRequest->requesterId);

	bool setSxSWP = 
		0 != (__SET_SxSWP_FLAG & vsuSoundSourceConfigurationRequest->SxEV1)
		|| 
		this->vsuSoundSourceConfigurations[i].SxSWP != vsuSoundSourceConfigurationRequest->SxSWP
		||
		(this->vsuSoundSourceConfigurations[i].requesterId != vsuSoundSourceConfigurationRequest->requesterId);

	this->vsuSoundSourceConfigurations[i].requesterId = vsuSoundSourceConfigurationRequest->requesterId;
	this->vsuSoundSourceConfigurations[i].waveform = waveform;
	this->vsuSoundSourceConfigurations[i].timeout = this->ticks + vsuSoundSourceConfigurationRequest->timeout;
	this->vsuSoundSourceConfigurations[i].SxINT = vsuSoundSourceConfigurationRequest->SxINT;
	this->vsuSoundSourceConfigurations[i].SxLRV = vsuSoundSourceConfigurationRequest->SxLRV;
	this->vsuSoundSourceConfigurations[i].SxFQL = vsuSoundSourceConfigurationRequest->SxFQL;
	this->vsuSoundSourceConfigurations[i].SxFQH = vsuSoundSourceConfigurationRequest->SxFQH;
	this->vsuSoundSourceConfigurations[i].SxEV0 = vsuSoundSourceConfigurationRequest->SxEV0;
	this->vsuSoundSourceConfigurations[i].SxEV1 = vsuSoundSourceConfigurationRequest->SxEV1;
	this->vsuSoundSourceConfigurations[i].SxRAM = waveform->index;
	this->vsuSoundSourceConfigurations[i].SxSWP = vsuSoundSourceConfigurationRequest->SxSWP;
	this->vsuSoundSourceConfigurations[i].priority = vsuSoundSourceConfigurationRequest->priority;

	if(setSxINT)
	{
		vsuSoundSource->SxINT = vsuSoundSourceConfigurationRequest->SxINT;
	}

	if(setSxEV0)
	{
		vsuSoundSource->SxEV0 = vsuSoundSourceConfigurationRequest->SxEV0;
	}
	
	if(setSxEV1)
	{
		vsuSoundSource->SxEV1 = vsuSoundSourceConfigurationRequest->SxEV1;
	}

	vsuSoundSource->SxLRV = vsuSoundSourceConfigurationRequest->SxLRV;
	vsuSoundSource->SxFQL = vsuSoundSourceConfigurationRequest->SxFQL;
	vsuSoundSource->SxFQH = vsuSoundSourceConfigurationRequest->SxFQH;

	if(setSxSWP)
	{
		vsuSoundSource->SxSWP = vsuSoundSourceConfigurationRequest->SxSWP;
	}

	vsuSoundSource->SxRAM = waveform->index;

	if(NULL != vsuSoundSourceConfigurationRequest->SxMOD)
	{		
		uint8* modulationData = __MODULATION_DATA;

		for(int16 i = 0; i <= __MODULATION_DATA_ENTRIES; i++)
		{
			modulationData[i << 2] = vsuSoundSourceConfigurationRequest->SxMOD[i];
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 VSUManager::freeableSoundSource(uint32 requesterId, uint32 soundSourceType, uint8 priority, bool skip)
{
	// First try to find a sound source that has previously assigned to the same requesterId
	for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
	{
		if(0 == (soundSourceType & this->vsuSoundSourceConfigurations[i].type))
		{
			continue;
		}

		if(requesterId == this->vsuSoundSourceConfigurations[i].requesterId)
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

	int16 stolenSoundSourceIndex = -1;

	if(!skip)
	{
		// Now try to find a sound source whose timeout has just expired
		for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
		{
			if(0 == (soundSourceType & this->vsuSoundSourceConfigurations[i].type))
			{
				continue;
			}

			if(this->vsuSoundSourceConfigurations[i].priority > priority)
			{
				continue;
			}

			if
			(
				0 > stolenSoundSourceIndex 
				|| 
				this->vsuSoundSourceConfigurations[i].timeout < this->vsuSoundSourceConfigurations[stolenSoundSourceIndex].timeout
			)
			{
				stolenSoundSourceIndex = i;
			}
		}		
	}

	return stolenSoundSourceIndex;
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
			this->vsuSoundSourceConfigurations[i].requesterId = -1;
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
			VSUManager::freeableSoundSource
			(
				this, queuedVSUSoundSourceConfigurationRequest->requesterId, queuedVSUSoundSourceConfigurationRequest->type, 
				queuedVSUSoundSourceConfigurationRequest->priority, queuedVSUSoundSourceConfigurationRequest->skip
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
	if(NULL != waveform && NULL != waveFormData)
	{
		waveform->usageCount = 1;

		if(waveform->crc == waveFormData->crc) 
		{
			return;
		}

		waveform->data = waveFormData->data;
		waveform->crc = waveFormData->crc;

		// Disable interrupts to make the following as soon as possible
		HardwareManager::suspendInterrupts();

		// Must stop all sound sources before writing the waveforms
		__SSTOP = 0x01;

		// Set the wave data
		for(uint32 i = 0; i < 32; i++)
		{
			waveform->wave[(i << 2)] = (uint8)waveform->data[i];
		}

		for(int16 i = 0; i < __TOTAL_SOUND_SOURCES; i++)
		{
			if(0 < this->vsuSoundSourceConfigurations[i].timeout)
			{
				VSUSoundSource* vsuSoundSource = this->vsuSoundSourceConfigurations[i].vsuSoundSource;

				vsuSoundSource->SxINT = this->vsuSoundSourceConfigurations[i].SxINT | 0x80;
			}
		}

		// Turn back interrupts on
		HardwareManager::resumeInterrupts();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
