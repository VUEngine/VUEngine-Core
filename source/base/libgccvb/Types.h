/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TYPES_H_
#define TYPES_H_


//---------------------------------------------------------------------------------------------------------
//											 DECLARATIONS
//---------------------------------------------------------------------------------------------------------

// quick, easy types
typedef unsigned char 		uint8;
typedef unsigned short 		uint16;
typedef unsigned int 		uint32;
typedef unsigned long long	uint64;

typedef signed char 		int8;
typedef signed short 		int16;
typedef signed int	 		int32;
typedef signed long long 	int64;

typedef uint8		 		BYTE;
typedef uint16		 		HWORD;
typedef uint32		 		WORD;

// define of boolean type
typedef uint8				bool;
enum { false, true };

#endif
