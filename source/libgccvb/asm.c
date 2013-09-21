/*---------------------------------INCLUDES--------------------------------*/
#include "asm.h"


/*---------------------------------FUNCTIONS-------------------------------*/

void setIntLevel(u8 level) {
	asm(" \n\
		stsr	sr5,r5 \n\
		movhi	0xFFF1,r0,r6 \n\
		movea	0xFFFF,r6,r6 \n\
		and		r5,r6 \n\
		mov		%0,r5 \n\
		andi	0x000F,r5,r5 \n\
		shl		0x10,r5 \n\
		or		r6,r5 \n\
		ldsr	r5,sr5 \n\
		"	
	: // Output 
	: "r" (level) // Input 
	: "r5", "r6" // Clobber 
	);
	
}


inline int geIntLevel() {
	int level;

	asm(" \n\
		stsr	sr5,r5 \n\
		shr		0x10,r5 \n\
		andi	0x000F,r5,r5 \n\
		mov		r5,%0 \n\
	"
	: "=r" (level) // Output 
	: // Input 
	: "r5" // Clobber 
	);
	
	return level;
}

inline int getPSW() {
	int psw;
	asm(" \n\
		stsr	psw,%0  \n\
		"	
	: "=r" (psw) // Output 
	);
	return psw;
}

inline int getStackPointer() {
	int sp;
	asm(" \
		mov		sp,%0  \
		"	
	: "=r" (sp) // Output 
	);
	return sp;
}

