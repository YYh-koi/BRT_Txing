
/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/

#include "atbm_sysops.h"
#include "atbm_timer.h"

int freertos_os_MsgQ_Create(atbm_os_msgq *pmsgQ, atbm_uint32 *pstack, atbm_uint32 item_size, atbm_uint32 item_num){

	if ((*pmsgQ = xQueueCreate(item_num, item_size)) == NULL)
        return -1;

    return 0;
}

int freertos_os_MsgQ_Delete(atbm_os_msgq *pmsgQ)
{
    int result = 0;

	  if(uxQueueMessagesWaiting((const QueueHandle_t)pmsgQ))
	  {
	  	  result = -1;
	  }
    vQueueDelete(*pmsgQ);
    return result;
}

static atbm_uint32 freertos_ms_to_systime(atbm_uint32 ms)
{
	return ms / portTICK_PERIOD_MS;
}

int freertos_os_MsgQ_Recv(atbm_os_msgq *pmsgQ, atbm_void *pbuf, atbm_uint32 val, int timeout_ms)
{
	if(timeout_ms == ATBM_TX_WAIT_FOREVER) {
		timeout_ms = portMAX_DELAY;
	} else {
		timeout_ms = freertos_ms_to_systime(timeout_ms);
	}

    if ( xQueueReceive( *pmsgQ, pbuf, timeout_ms ) != pdPASS )
    {
        return -1;
    }

    return 0;
}

int freertos_os_MsgQ_Send(atbm_os_msgq *pmsgQ, atbm_void *pbuf, atbm_uint32 val, int timeout_ms)
{
	if(timeout_ms == ATBM_TX_WAIT_FOREVER) {
	      timeout_ms = portMAX_DELAY;
	} else {
	      timeout_ms = freertos_ms_to_systime(timeout_ms);
	}

    if ( xQueueSendToBack( *pmsgQ, pbuf, timeout_ms) != pdPASS )
    {
        return -1;
    }

    return 0;

}

////

