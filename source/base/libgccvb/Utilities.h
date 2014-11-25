#ifndef UTILITIES_H_
#define UTILITIES_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											DECLARATIONS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// generate random seed
long Utilities_randomSeed();

// retrieve random number
int Utilities_random(long seed, int randnums);

// converte int to char
char* Utilities_itoa(u32 num, u8 base, u8 digits);

// rotate bits
WORD Utilities_rotateBits(WORD invalue, int places, int direction);

// return true if 2 numbers have equal sign
int Utilities_equalSign(int a, int b);

// retrieve an int's total digits
int Utilities_getDigitCount(int value);

#endif