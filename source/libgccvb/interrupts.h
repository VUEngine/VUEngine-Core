#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"

extern u32 key_vector;
extern u32 tim_vector;
extern u32 cro_vector;
extern u32 com_vector;
extern u32 vpu_vector;

void setInterruptsVectors();

void key_hnd(void);
void tim_hnd(void);
void cro_hnd(void);
void com_hnd(void);
void vpu_hnd(void);



#endif //INTERRUPTS_H_
