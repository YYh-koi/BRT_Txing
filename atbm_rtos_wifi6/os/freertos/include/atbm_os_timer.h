#ifndef ATBM_OS_TIMER_H
#define ATBM_OS_TIMER_H

#include "atbm_sysops.h"
#define ATBM_HZ 1000  //fixed 
#define TIMER_1ST_PARAM

typedef atbm_void (*TIMER_CALLBACK)(atbm_void * CallRef);
typedef atbm_void* timerHandle;


struct OS_TIMER_S
{
    atbm_uint32         TimerId;
    TIMER_CALLBACK  	pCallback;
    atbm_void *         pCallRef;
	atbm_uint32			TimerHander;
	int 				bTimerStart;

};

typedef struct OS_TIMER_S OS_TIMER;

int freertos_InitTimer(OS_TIMER *pTimer, TIMER_CALLBACK pCallback, atbm_void * CallRef);
int freertos_StartTimer(OS_TIMER *pTimer, int Interval/*ms*/);
int freertos_CancelTimer(OS_TIMER *pTimer);
int freertos_FreeTimer(OS_TIMER *pTimer);
atbm_void freertos_SleepMs(atbm_uint32 uiMiliSecond);
unsigned int freertos_GetOsTime(void);

unsigned int freertos_GetOsTimeMs(atbm_void);
ATBM_BOOL freertos_TimeAfter(unsigned int tickMs, atbm_uint32 intvMs);
atbm_void freertos_mdelay(atbm_uint32 uiMiliSecond);

atbm_void atbm_wifi_ticks_timer_init(void);
atbm_void atbm_wifi_ticks_timer_cancle(atbm_void);

atbm_uint32 atbm_TimerMsToTick(atbm_uint32 ms);
atbm_uint32 atbm_TimerTickToMs(atbm_uint32 tick);
atbm_uint32 atbm_TimerTickGet(atbm_void);
atbm_uint8 atbm_TimerIsActive(OS_TIMER *pTimer);
atbm_uint32 atbm_TimerGetExpiry(OS_TIMER *pTimer);

#define atbm_InitTimer freertos_InitTimer
#define atbm_StartTimer freertos_StartTimer 
#define atbm_CancelTimer freertos_CancelTimer
#define atbm_FreeTimer freertos_FreeTimer
#define atbm_SleepMs freertos_SleepMs
#define atbm_GetOsTime freertos_GetOsTime

#define atbm_GetOsTimeMs freertos_GetOsTimeMs
#define atbm_TimeAfter freertos_TimeAfter
#define atbm_mdelay		freertos_mdelay

//////////////////////////////////////////////////////////////////////////////////////////////////////
	  
#endif /* ATBM_OS_TIMER_H */
