#ifndef TYPES_H_
#define TYPES_H_


//---------------------------------------------------------------------------------------------------------
// 											 DECLARATIONS
//---------------------------------------------------------------------------------------------------------

typedef enum { false, true } bool;

//quick, easy types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef signed long long s64;

typedef unsigned char BYTE;
typedef unsigned short HWORD;
typedef unsigned long WORD;

// for fixed point maths
#define f8 s8
#define f16 s16
#define f32 s32


#endif