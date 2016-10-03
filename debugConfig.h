#ifndef	DEBUG_CONFIG_H_
#undef	DEBUG_CONFIG_H_

#include <debugUtilities.h>

// avoid declaration warnings
#include <HardwareManager.h>
void HardwareManager_checkStackStatus(HardwareManager this);
void HardwareManager_printStackStatus(HardwareManager this, int x, int y, bool resumed);

#undef __PRINT_FRAMERATE
#undef __PROFILE_GAME
#undef __PROFILE_GAME_DETAILED
#undef __PROFILE_STREAMING
#undef __DIMM_FOR_PROFILING
#undef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
#undef __ALERT_VIP_OVERTIME
#undef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE

#undef __PRINT_MEMORY_POOL_STATUS
#undef __PRINT_DETAILED_MEMORY_POOL_STATUS

#undef __ALERT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VIP
#undef __ALERT_STACK_OVERFLOW

#endif
