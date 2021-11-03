/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <Mem.h>
#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// redefine memcpy
__attribute__ ((unused)) static void* memcpy(void *destination, const void *source, size_t numberOfBytes)
{
	BYTE* finalSource = (BYTE*)source + numberOfBytes / sizeof(uint32) + __MODULO(numberOfBytes, sizeof(uint32));

	asm("				\n\t"      \
		"jr end%=		\n\t"      \
		"loop%=:		\n\t"      \
		"ld.w 0[%1],r10	\n\t"      \
		"st.w r10,0[%0] \n\t"      \
		"add 4,%0		\n\t"      \
		"add 4,%1		\n\t"      \
		"end%=:			\n\t"      \
		"cmp %1,%2		\n\t"      \
		"bgt loop%=		\n\t"
		: // No Output
		: "r" (destination), "r" (source), "r" (finalSource)
		: "r10" // regs used
	);

	return destination;
}

// Produces graphical glitches if inlined
// Not too critical since it is not used a lot
static void Mem::clear(BYTE* destination, uint32 numberOfBYTES)
{
	uint32 i;

	for(i = 0; i < numberOfBYTES; i++)
	{
		*destination++ = 0;
	}
}

// TODO: inlining this causes trouble with ANIMATED_MULTI animations
static void Mem::addHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset)
{
	const HWORD* finalSource = source + numberOfHWORDS;

    asm("					\n\t"      \
		"jr end%=			\n\t"      \
		"loop%=:			\n\t"      \
		"ld.h 0[%1],r10		\n\t"      \
		"add %3,r10			\n\t"      \
		"st.h r10,0[%0]		\n\t"      \
		"add 2,%0			\n\t"      \
		"add 2,%1			\n\t"      \
		"end%=:				\n\t"      \
		"cmp %1,%2			\n\t"      \
		"bgt loop%=			\n\t"      \
    : // No Output
    : "r" (destination), "r" (source), "r" (finalSource), "r" (offset)
	: "r10" // regs used
    );
}

