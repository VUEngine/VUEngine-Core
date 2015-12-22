/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef	STDARG_H_
#define	STDARG_H_


//---------------------------------------------------------------------------------------------------------
// 											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

#ifndef VA_LIST_
#define VA_LIST_
typedef char *va_list;
#endif

#define va_start(list, start) ((void)((list) = (sizeof(start) < 4 										\
    ? (char *)((int *)&(start)+1) 																		\
    : (char *)(&(start)+1))))
#define va_end(list) ((void)0)
#define va_arg(list, mode) *(mode *)(&(list = (char*)(((int)list + 7)&~3U))[-4])


#endif