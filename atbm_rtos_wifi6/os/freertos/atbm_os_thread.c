#include "atbm_os_thread.h"
#include "atbm_sysops.h"

#define testwifiTASK_STACK_SIZE             (1024*8)

pAtbm_thread_t freertos_createThread(char *name,atbm_void (*task)(atbm_void *p_arg),atbm_void *p_arg,int prio)
{
	int ret = 0;
	TaskHandle_t pid;
	char task_name[10];
	if(WORK_TASK_PRIO==prio)
	{
		strcpy(task_name,"WORK_TASK");
		printf ("++%s =%d WORK_TASK_PRIO\n", task_name,prio);
	}
	else if(BH_TASK_PRIO==prio)
	{	
	
		strcpy(task_name,"BH_TASK");
		printf ("++%s =%dBH_TASK_PRIO\n", task_name,prio);
	}
	else if(ELOOP_TASK_PRIO==prio)
	{
		strcpy(task_name,"ELOOP_TASK");
		printf ("++%s =%dELOOP_TASK_PRIO\n", task_name,prio);
	}
	else if(HIF_TASK_PRIO==prio)
	{
		strcpy(task_name,"HIF_TASK");
		printf ("++%s =%dHIF_TASK_PRIO\n", task_name,prio);
	}
	else if(TX_BH_TASK_PRIO==prio)
	{
		strcpy(task_name,"TX_BH_TASK_");
		printf ("++%s =%dTX_BH_TASK_PRIO\n", task_name,prio);
	}else{
		strcpy(task_name,name);
		printf ("F:%s ++%s =%d \n", __func__, task_name,prio);
	}

	
	prio += tskIDLE_PRIORITY;
	ret = xTaskCreate(
				task,
				task_name,
				testwifiTASK_STACK_SIZE,
				p_arg,
				prio,
				&pid);
	if(ret != pdPASS){
		return NULL;
	}
	return pid;
}

atbm_void freertos_stopThread(pAtbm_thread_t thread_id)
{
	vTaskDelete(thread_id);
	thread_id = 0;
}

atbm_void freertos_ThreadStopEvent(pAtbm_thread_t thread_id)
{
	thread_id = 0;
	THREAD_RETURN(0);
}

atbm_uint32 atbm_getCurThreadId(void)
{
	return (atbm_uint32)xTaskGetCurrentTaskHandle();
}
