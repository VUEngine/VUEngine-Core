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

#ifndef EVENTS_H_
#define EVENTS_H_


//---------------------------------------------------------------------------------------------------------
// 											 DEFINITIONS
//---------------------------------------------------------------------------------------------------------

enum Events
{
    // do not remove me
    kFirstEngineEvent = 0,

    // add events here
    kEventSpatialObjectDeleted,
    kEventContainerDeleted,
    kEventEntityLoaded,

    kEventSecondChanged,
    kEventMinuteChanged,
    kEventHourChanged,

    kEventAnimationCompleted,
    kEventTextureRewritten,
    kEventCharSetRewritten,
    kEventCharSetDeleted,

    kEventEffectFadeComplete,
    kEventEffectFadeStart,
    kEventEffectFadeStop,

    // do not remove me
    kLastEngineEvent
};


#endif
