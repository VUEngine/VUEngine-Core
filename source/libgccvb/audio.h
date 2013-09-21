#ifndef AUDIO_H_
#define AUDIO_H_

#include "types.h"

typedef struct SOUNDREG{
	//this table is for the most part untested, but looks to be accurate
	//                 |      D7      ||      D6      ||      D5      ||      D4      ||      D3      ||      D2      ||      D1      ||      D0      |
	u8 SxINT; //       [----Enable----][--XXXXXXXXXX--][-Interval/??--][--------------------------------Interval Data---------------------------------]
	u8 spacer1[3];
	u8 SxLRV; //       [---------------------------L Level----------------------------][---------------------------R Level----------------------------]
	u8 spacer2[3];
	u8 SxFQL; //       [------------------------------------------------------Frequency Low Byte------------------------------------------------------]
	u8 spacer3[3];
	u8 SxFQH; //       [--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Frequency High Byte-------------]
	u8 spacer4[3];
	u8 SxEV0; //       [---------------------Initial Envelope Value-------------------][------U/D-----][-----------------Envelope Step----------------]
	u8 spacer5[3];
		 //Ch. 1-4 [--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	         //Ch. 5   [--XXXXXXXXXX--][------E/D-----][----?/Short---][--Mod./Sweep--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	u8 SxEV1; //Ch. 6  [--XXXXXXXXXX--][----------------------E/D---------------------][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	u8 spacer6[3];
	//Ch. 1-5 only (I believe address is only 3 bits, but may be 4, needs testing)
	u8 SxRAM; //       [--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Waveform RAM Address------------]
	u8 spacer7[3];
	//Ch. 5 only
	u8 S5SWP; //       [------CLK-----][-------------Sweep/Modulation Time------------][------U/D-----][----------------Number of Shifts--------------]
	u8 spacer8[35];
} SOUNDREG;

//reserved for world's background
static u8* const WAVEDATA1 =		(u8*)0x01000000;
static u8* const WAVEDATA2 =		(u8*)0x01000080;
//reserved for world's background
static u8* const WAVEDATA3 =		(u8*)0x01000100;
static u8* const WAVEDATA4 =		(u8*)0x01000180;
static u8* const WAVEDATA5 =		(u8*)0x01000200;
static u8* const MODDATA =			(u8*)0x01000280;
static SOUNDREG* const SND_REGS =	(SOUNDREG*)0x01000400; //(SOUNDREG*)0x010003C0;
#define SSTOP				*(u8*)0x01000580

/***** Sound Register Mnemonics *****/
/*
#define	WAVE1	0x00	// Voluntary wave channel #1
#define	WAVE2	0x01	// Voluntary wave channel #2
#define	WAVE3	0x02	// Voluntary wave channel #3
#define	WAVE4	0x03	// Voluntary wave channel #4
#define	SWEEP	0x04	// Sweep/modulation channel
#define	NOISE	0x05	// Pseudorandom noise channel
*/

#define	BGM0	0x00	// Voluntary bgm channel #1
#define	BGM1	0x01	// Voluntary bgm channel #2
#define	SFX0	0x02	// Voluntary sound special effect channel #3
#define	SFX1	0x03	// Voluntary sound special effectchannel #4
#define	SWEEP	0x04	// Sweep/modulation (or sound special effect) channel
#define	NOISE	0x05	// Pseudorandom noise channel

/*
#define	BGM		0x00	// Background music channel #1
#define	WAVE2	0x01	// Voluntary wave channel #2
#define	WAVE3	0x02	// Voluntary wave channel #3
#define	WAVE4	0x03	// Voluntary wave channel #4
#define	SWEEP	0x04	// Sweep/modulation channel
#define	NOISE	0x05	// Pseudorandom noise channel
*/
/* DogP's code
 */
//musical notes to VB register values (may not be quite correct, based on frequencies from gameboy)
#define NONE 0x00
#define C_10 0x00
#define CS10 0x00
#define D_10 0x00
#define DS10 0x00
#define E_10 0x00
#define F_10 0x00
#define FS10 0x00
#define G_10 0x00
#define C_0 0x00
#define CS0 0x00
#define D_0 0x00
#define DS0 0x00
#define E_0 0x00
#define F_0 0x00
#define FS0 0x00
#define G_0 0x00
#define GS0 0x00
#define A_0 0x00
#define AS0 0x00
#define B_0 0x00
#define C_1 0x00
#define CS1 0x00
#define D_1 0x00
#define DS1 0x00
#define E_1 0x00
#define F_1 0x00
#define FS1 0x00
#define G_1 0x00
#define GS1 0x00
#define A_1 0x00
#define AS1 0x00
#define B_1 0x00
#define C_2 0x00
#define CS2 0x00
#define D_2 0x00
#define DS2 0x00
#define E_2 0x2C
#define F_2 0x9C
//beginning of VB audible range
#define FS2 0x106
#define G_2 0x16B
#define GS2 0x1C9
#define A_2 0x223
#define AS2 0x277
#define B_2 0x2C6
#define C_3 0x312
#define CS3 0x356
#define D_3 0x39B
#define DS3 0x3DA
#define E_3 0x416
#define F_3 0x44E
#define FS3 0x483
#define G_3 0x4B5
#define GS3 0x4E5
#define A_3 0x511
#define AS3 0x53B
#define B_3 0x563
#define C_4 0x589
#define CS4 0x5AC
#define D_4 0x5CE
#define DS4 0x5ED
#define E_4 0x60A
#define F_4 0x627
#define FS4 0x642
#define G_4 0x65B
#define GS4 0x672
#define A_4 0x689
#define AS4 0x69E
#define B_4 0x6B2
#define C_5 0x6C4
#define CS5 0x6D6
#define D_5 0x6E7
#define DS5 0x6F7
#define E_5 0x706
#define F_5 0x714
#define FS5 0x721
#define G_5 0x72D
#define GS5 0x739
#define A_5 0x744
#define AS5 0x74F
#define B_5 0x759
#define C_6 0x762
#define CS6 0x76B
#define D_6 0x773
#define DS6 0x77B
#define E_6 0x783
#define F_6 0x78A
#define FS6 0x790
#define G_6 0x797
#define GS6 0x79D
#define A_6 0x7A2
#define AS6 0x7A7
#define B_6 0x7AC
#define C_7 0x7B1
#define CS7 0x7B6
#define D_7 0x7BA
#define DS7 0x7BE
#define E_7 0x7C1
#define F_7 0x7C4
#define FS7 0x7C8
#define G_7 0x7CB
#define GS7 0x7CE
#define A_7 0x7D1
#define AS7 0x7D4
#define B_7 0x7D6
#define C_8 0x7D9
#define CS8 0x7DB
#define D_8 0x7DD
#define DS8 0x7DF
//end of VB audible range
#define E_8 0x00
#define F_8 0x00
#define FS8 0x00
#define G_8 0x00
#define GS8 0x00
#define A_8 0x00
#define AS8 0x00
#define B_8 0x00
#define C_9 0x00
#define CS9 0x00
#define D_9 0x00
#define DS9 0x00
#define E_9 0x00
#define F_9 0x00
#define FS9 0x00
#define G_9 0x00
#define GS9 0x00
#define A_9 0x00
#define AS9 0x00
#define B_9 0x00

#endif
