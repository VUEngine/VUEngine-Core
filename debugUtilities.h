#ifndef	DEBUG_UTILITIES_H_
#define	DEBUG_UTILITIES_H_

#define	__PRINT_IN_GAME_TIME(x, y)	Printing_int(Printing_getInstance(), Game_getTime(Game_getInstance()), x, y, NULL);
#define	__PRINT_TIME(x, y)	Printing_int(Printing_getInstance(), TimerManager_getTotalMillisecondsElapsed(TimerManager_getInstance()), x, y, NULL);

#endif
