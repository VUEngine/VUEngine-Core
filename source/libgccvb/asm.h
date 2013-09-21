#ifndef ASM_H_
#define ASM_H_

/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"
/*---------------------------------CONSTANTS-------------------------------*/
#define INT_ENABLE		asm("EI")
#define INT_DISABLE		asm("DI")

/*---------------------------------FUNCTIONS-------------------------------*/
u32 jump_addr(void *addr);
void setIntLevel(u8 level);
inline int geIntLevel();
inline int getPSW() ;
inline int getStackPointer();
#endif
