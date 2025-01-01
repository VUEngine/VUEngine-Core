/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <stdio.h>

#include <HardwareManager.h>

#include "Mem.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// It doesn't need to be inlined since it is not used in performance critical places
/// and it produces graphical glitches when inlined.
static void Mem::clear(BYTE* destination, uint32 numberOfBYTES)
{
	uint32 i;

	for(i = 0; i < numberOfBYTES; i++)
	{
		*destination++ = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// GLOBAL METHODS REDEFINITIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Redefinition of memcpy
__attribute__ ((unused)) static void* memcpy(void *destination, const void *source, size_t numberOfBytes)
{
	BYTE* finalSource = (BYTE*)source + numberOfBytes / sizeof(uint32) + __MODULO(numberOfBytes, sizeof(uint32));

	asm(
		"jr		end%=		\n\t"      \
		"loop%=:			\n\t"      \
		"ld.w	0[%1], r10	\n\t"      \
		"st.w	r10, 0[%0] 	\n\t"      \
		"add	4, %0		\n\t"      \
		"add	4, %1		\n\t"      \
		"end%=:				\n\t"      \
		"cmp	%1, %2		\n\t"      \
		"bgt	loop%=		\n\t"
		: // No Output
		: "r" (destination), "r" (source), "r" (finalSource)
		: "r10" // regs used
	);

	return destination;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

