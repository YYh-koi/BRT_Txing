#include "atbm_sysops.h"
#include "atbm_debug.h"
#include "timers.h"

atbm_void _atbm_timer_handler(TimerHandle_t xTimer)
{
	OS_TIMER *pTimer;
	//wifi_printk(WIFI_ALWAYS,"_atbm_timer_handler\n");

	pTimer = (OS_TIMER *)pvTimerGetTimerID(xTimer);
	if(pTimer->pCallback)
		pTimer->pCallback(pTimer->pCallRef);
}

int freertos_InitTimer(OS_TIMER *pTimer, TIMER_CALLBACK pCallback, atbm_void * CallRef)
{
	pTimer->pCallback = pCallback;
	pTimer->pCallRef = CallRef;
	pTimer->bTimerStart =0;
	/*Set timer initial value 100ms. change it in atbm_StartTimer*/
	/*
	pTimer->TimerHander = (atbm_uint32)xTimerCreate("atbm", 1000 / portTICK_PERIOD_MS,
					pdFALSE, (void * const)pTimer, _atbm_timer_handler);
	*/
	pTimer->TimerHander = (atbm_uint32)xTimerCreate("atbm", pdMS_TO_TICKS(100),
					pdFALSE, (void * const)pTimer, _atbm_timer_handler);//pdFALSE  pdTRUE
	//wifi_printk(WIFI_ALWAYS,"888888  freertos_InitTimer pTimer->TimerHander ret=%d pTimer addr=%x\n",pTimer->TimerHander,pTimer);
	return 0;
	/*
	if(xTimerStart(pTimer,pdMS_TO_TICKS(100))== pdPASS)
	{
		return 0;
	}
	else
	{
		wifi_printk(WIFI_ALWAYS,"xTimerStart -1\n");
		return -1;
	}
	*/
}
//Interval / portTICK_PERIOD_MS, 100 / portTICK_PERIOD_MS) == pdPASS) {

int freertos_StartTimer(OS_TIMER *pTimer, int Interval/*ms*/)
{
	//wifi_printk(WIFI_ALWAYS,"888888 freertos_StartTimer pTimer->bTimerStart =%d Interval=%d pTimer addr=%x\n",pTimer->bTimerStart,Interval,pTimer);
	if(!Interval)
	{
		Interval=1;

		wifi_printk(WIFI_ALWAYS,"Interval=0 %d\n",portTICK_PERIOD_MS);
	}
	else
	{
		 Interval = Interval / portTICK_PERIOD_MS;
	}
	if (pTimer == NULL) {
		return -1;
	}
	if (pTimer->bTimerStart == 1) {
		//return 0;
		//wifi_printk(WIFI_ALWAYS,"======\n");
	}
#if 1
/*xTimerChangePeriod will case timer start too */
	if (xTimerChangePeriod((TimerHandle_t)pTimer->TimerHander,Interval, 0) == pdPASS) {
		//if (xTimerStart((TimerHandle_t)pTimer->TimerHander,Interval) == pdPASS) {
		/*The command was successfully sent.*/
		pTimer->bTimerStart =1;
		return 0;
	} else {
		/*TODO: Take appropriate action here.*/
		return -1;
	}
#endif 

	
	
}
	
int freertos_CancelTimer(OS_TIMER *pTimer)
{
	int ret;
	if (pTimer == NULL) {
		wifi_printk(WIFI_ALWAYS,"null timer\n");
		return -1;
	}
	
	if (pTimer->bTimerStart == 1) {
		/*TODO:assert return value*/
		//wifi_printk(WIFI_ALWAYS,"------ \n");
		ret=xTimerStop((TimerHandle_t)pTimer->TimerHander, 100 / portTICK_PERIOD_MS);
		if (ret!=pdPASS) {
			wifi_printk(WIFI_ALWAYS,"++++++ ret=%d\n",ret);
		}
		pTimer->bTimerStart =0;
	}
	//wifi_printk(WIFI_ALWAYS,"888888 freertos_CancelTimer pTimer->bTimerStart =%d pTimeraddr=%x\n",pTimer->bTimerStart,pTimer);
	return 0;

}

int freertos_FreeTimer(OS_TIMER *pTimer)
{
	if (pTimer == NULL) {
		wifi_printk(WIFI_ALWAYS,"null timer\n");
		return -1;
	}
//wifi_printk(WIFI_ALWAYS,"888888 freertos_FreeTimer pTimer->bTimerStart ret=%d pTimeraddr=%x\n",pTimer->bTimerStart,pTimer);
	if(pTimer->bTimerStart !=0){
		pTimer->bTimerStart = 0;
		/*TODO:assert return value*/
		xTimerStop((TimerHandle_t)pTimer->TimerHander, 100 / portTICK_PERIOD_MS);
	}
	/*TODO:assert return value*/
	xTimerDelete((TimerHandle_t)pTimer->TimerHander, 100 / portTICK_PERIOD_MS);
	pTimer->TimerHander = (atbm_uint32)ATBM_NULL;
	
	return 0;
}

atbm_void freertos_SleepMs(atbm_uint32 uiMiliSecond)
{
	if (uiMiliSecond<10)
  		uiMiliSecond=10;
  	vTaskDelay(uiMiliSecond/portTICK_PERIOD_MS);
}

atbm_void freertos_mdelay(atbm_uint32 ms)
{
	vTaskDelay(ms / portTICK_PERIOD_MS);
}

unsigned int freertos_GetOsTime(void)
{
	return xTaskGetTickCount()*portTICK_PERIOD_MS;
}

unsigned int freertos_GetOsTimeMs(atbm_void)
{
	unsigned int ticks = (unsigned int)xTaskGetTickCount();
    return ticks * portTICK_PERIOD_MS;
}

ATBM_BOOL freertos_TimeAfter(unsigned int tickMs, atbm_uint32 intvMs)
{
	atbm_uint32 current_time = atbm_GetOsTimeMs();
//	return ((signed int)current_time - (signed int)tickMs < 0);
	return ((( (int)(tickMs + intvMs - current_time)) >= 0) && ( current_time >= tickMs));
}

atbm_void atbm_wifi_ticks_timer_init(void)
{


}

atbm_void atbm_wifi_ticks_timer_cancle(atbm_void)
{

}


atbm_uint32 atbm_TimerMsToTick(atbm_uint32 ms)
{
	return ms/portTICK_PERIOD_MS;
}

atbm_uint32 atbm_TimerTickToMs(atbm_uint32 tick)
{
	return (tick*portTICK_PERIOD_MS);
}

atbm_uint32 atbm_TimerTickGet(atbm_void)
{
	return xTaskGetTickCount();
}

atbm_uint8 atbm_TimerIsActive(OS_TIMER *pTimer)
{
	return (!!pTimer->bTimerStart);
}

atbm_uint32 atbm_TimerGetExpiry(OS_TIMER *pTimer)
{
/*	aos_timer_t *timer = (aos_timer_t *)(pTimer->TimerHander->os_timer);
	ktimer_t *ktimer = (ktimer_t *)(timer->hdl);
	if(atbm_TimerIsActive(pTimer)){
		return ktimer->match;
	}*/
	return 0;
}

