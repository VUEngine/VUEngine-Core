/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MIDI_H_
#define MIDI_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#define PAU 0x00
#define HOLD 0x01
#define STOP 0x02
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
//beginning of VB audible range
#define DS2 0x27
#define E_2 0x98
#define F_2 0x102
#define FS2 0x167
#define G_2 0x1C6
#define GS2 0x21F
#define A_2 0x274
#define AS2 0x2C3
#define B_2 0x30F
#define C_3 0x356
#define CS3 0x399
#define D_3 0x3D8
#define DS3 0x414
#define E_3 0x44C
#define F_3 0x481
#define FS3 0x4B3
#define G_3 0x4E3
#define GS3 0x510
#define A_3 0x53A
#define AS3 0x562
#define B_3 0x587
#define C_4 0x5AB
#define CS4 0x5CC
#define D_4 0x5EC
#define DS4 0x60B
#define E_4 0x626
#define F_4 0x640
#define FS4 0x659
#define G_4 0x672
#define GS4 0x688
#define A_4 0x69D
#define AS4 0x6B1
#define B_4 0x6C4
#define C_5 0x6D5
#define CS5 0x6E6
#define D_5 0x6F6
#define DS5 0x705
#define E_5 0x713
#define F_5 0x720
#define FS5 0x72D
#define G_5 0x739
#define GS5 0x744
#define A_5 0x74E
#define AS5 0x758
#define B_5 0x762
#define C_6 0x76B
#define CS6 0x773
#define D_6 0x77B
#define DS6 0x782
#define E_6 0x78A
#define F_6 0x790
#define FS6 0x796
#define G_6 0x79C
#define GS6 0x7A2
#define A_6 0x7A8
#define AS6 0x7AC
#define B_6 0x7B1
#define C_7 0x7B5
#define CS7 0x7BA
#define D_7 0x7BD
#define DS7 0x7C1
#define E_7 0x7C5
#define F_7 0x7C8
#define FS7 0x7CB
#define G_7 0x7CE
#define GS7 0x7D1
#define A_7 0x7D4
#define AS7 0x7D6
#define B_7 0x7D8
#define C_8 0x7DB
#define CS8 0x7DD
#define D_8 0x7DF
//end of VB audible range
//(Higher sounds are audible, but will not produce a pure, specific note)
#define DS8 0x7E1
#define E_8 0x7E2
#define F_8 0x7E4
#define FS8 0x7E6
#define G_8 0x7E7
#define GS8 0x7E8
#define A_8 0x7EA
#define AS8 0x7EB
#define B_8 0x7EC
#define C_9 0x7ED
#define CS9 0x00 //8869,84
#define D_9 0x00 //9397,27
#define DS9 0x00 //9956,06
#define E_9 0x00 //10548,08
#define F_9 0x00 //11175,30
#define FS9 0x00 //11839,82
#define G_9 0x00 //12543,85
#define GS9 0x00 //13289,75
#define A_9 0x00 //14080,00
#define AS9 0x00 //14917,24
#define B_9 0x00 //15804,27
#define C_10 0x00 //16744,04
#define CS10 0x00 //17739,69
#define D_10 0x00 //18794,55
#define DS10 0x00 //19912,13
#define E_10 0x00 //21096,16
#define F_10 0x00 //22350,61
#define FS10 0x00 //23679,64
#define G_10 0x00 //25087,71

// Special sound notes
#define ENDSOUND  0xFFFF  // Ends the sound.
#define LOOPSOUND 0xFFFE  // Repeats the sound from the beginning.
#define MINIMUM_AUDIBLE_NOTE	DS2
#define MAXIMUM_AUDIBLE_NOTE	D_8


#endif