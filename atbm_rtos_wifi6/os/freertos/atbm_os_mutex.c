
/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#include "atbm_sysops.h"
#include <FreeRTOS.h>
#include "semphr.h"
#include "atbm_debug.h"
int freertos_mutex_init(atbm_mutex *x)
{
    *x= xSemaphoreCreateMutex();
	ATBM_ASSERT(*x);
	return 0;
}

int freertos_mutex_lock(atbm_mutex *x, atbm_uint32 ulTimeout)
{
    int ret=0;
	atbm_uint32 ticks= ulTimeout / portTICK_PERIOD_MS;
	if (ulTimeout == 0) {
		ticks = portMAX_DELAY;
	}

	ret = xSemaphoreTake(*x, ticks);
	if (ret != pdTRUE) {
		return -1;
	}
	return 0;
}

int freertos_mutex_unlock(atbm_mutex *x)
{
    if (xSemaphoreGive(*x) != pdTRUE){
	        iot_printf("%s fail \n",__FUNCTION__);
		return -1;
	}
	return 0;
}



int freertos_DeleteMutex(atbm_mutex *x)
{
	vSemaphoreDelete(*x);
	*x = NULL;
	
	return 0;
}

int freertos_wait_event_timeout(atbm_mutex *x,atbm_uint32 ms)
{
    int ret=0;
	atbm_uint32 timeout_ticks = ms / portTICK_PERIOD_MS;

	if(!ms){
		timeout_ticks = portMAX_DELAY;
	}
						    //iot_printf("<<<<<wait event: %p %d\n",*pSem,timeout_ticks);
	ret = xSemaphoreTake(*x, timeout_ticks);
	if (ret == errQUEUE_EMPTY) {
		/* timed out */
		return 0;
	}
	ATBM_ASSERT(ret == pdTRUE);

	return 1;
}

int freertos_wait_event_timeout_ext(atbm_mutex *x,atbm_uint32 ms)
{
    int ret=0;
	atbm_uint32 timeout_ticks = ms / portTICK_PERIOD_MS;

	if(!ms){
		timeout_ticks = portMAX_DELAY;
	}
						    //iot_printf("<<<<<wait event: %p %d\n",*pSem,timeout_ticks);
	ret = xSemaphoreTake(*x, timeout_ticks);
	if (ret == errQUEUE_EMPTY) {
		/* timed out */
		return 1;
	}
	ATBM_ASSERT(ret == pdTRUE);

	return 0;
}


atbm_void freertos_os_init_waitevent(atbm_os_wait_queue_head_t* x)
{
    *x = xSemaphoreCreateBinary();
	    ATBM_ASSERT(*x!=NULL);
}

void freertos_os_wakeup_event(atbm_os_wait_queue_head_t *pSem)
{
	xSemaphoreGive(*pSem);
}

int atbm_os_wait_event_count_get(atbm_os_wait_queue_head_t *pSem)
{
	return uxSemaphoreGetCount(*pSem);
}

atbm_void freertos_os_delete_waitevent(atbm_os_wait_queue_head_t* x)
{
	if(*x != NULL)
		vSemaphoreDelete(*x);

	*x = NULL;
}
