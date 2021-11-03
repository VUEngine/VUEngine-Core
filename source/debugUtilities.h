/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef	DEBUG_UTILITIES_H_
#define	DEBUG_UTILITIES_H_

#define	PRINT_IN_GAME_TIME(x, y)	Printing_int32(Printing_getInstance(), Game_getTime(Game_getInstance()), x, y, NULL);
#define	PRINT_TIME(x, y)	Printing_int32(Printing_getInstance(), TimerManager_getTotalMillisecondsElapsed(TimerManager_getInstance()), x, y, NULL);

#endif
