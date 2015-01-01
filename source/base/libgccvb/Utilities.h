#ifndef UTILITIES_H_
#define UTILITIES_H_


//---------------------------------------------------------------------------------------------------------
// 											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

long Utilities_randomSeed();
int Utilities_random(long seed, int randnums);
char* Utilities_itoa(u32 num, u8 base, u8 digits);
WORD Utilities_rotateBits(WORD invalue, int places, int direction);
int Utilities_equalSign(int a, int b);
int Utilities_getDigitCount(int value);
int Utilities_intLength(int value);


#endif