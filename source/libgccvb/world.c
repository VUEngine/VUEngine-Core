/*---------------------------------INCLUDES--------------------------------*/
#include "world.h"

/*---------------------------------FUNCTIONS-------------------------------*/
void vbActivateWorldShow(){
	//all world from 30 through 1 depends on world 31 be active
	WORLD_HEAD(31, WRLD_ON);
}
void vbDesactivateWorldShow(){
	int i=31;
	//al world from 30 through 1 depends on world 31 be active
	for(;i--;){
		WORLD_HEAD(i, WRLD_OFF);
		WORLD_SIZE(i, 0,0);
	}
	WORLD_HEAD(31, WRLD_OFF);
}