/**************************************************************************************************************
 * altobeam RTOS
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/

#include "atbm_hal.h"
#include "atbm_task.h"
#if QUEUE_LIST

static atbm_void atbm_workqueue_thread(atbm_void *arg, int wakeup_state)
{
	atbm_uint32 ret;
	atbm_uint32 irq_flag;
	struct atbmwifi_common *hw_priv = (struct atbmwifi_common *)arg;
		
	irq_flag = atbm_local_irq_save();
	while (!atbm_list_empty(&hw_priv->work_Link))
	{
		struct atbmwifi_work_struct *work = NULL;
		irq_flag = atbm_local_irq_save();
		work=atbm_list_first_entry(&hw_priv->work_Link,struct atbmwifi_work_struct,workLink);
		atbm_local_irq_restore(irq_flag);
		if(work->valid){
			if(work->fun){
				work->fun(work->data);
			}
			irq_flag = atbm_local_irq_save();
			atbm_list_del(&work->workLink);
			atbm_local_irq_restore(irq_flag);
		}else{
			break;
		}
	}

}

atbm_work  atbm_init_work(struct atbmwifi_common *hw_priv,atbm_work_fun fun, atbm_void *data)
{
	struct atbmwifi_work_struct *work = NULL;
	work = (struct atbmwifi_work_struct *)atbm_kmalloc(sizeof(struct atbmwifi_work_struct),GFP_KERNEL);	
	if(work){
		work->fun = (atbm_void (*)(atbm_void *))fun;
		work->data = data;
		work->valid=1;
	}
	return work;
}
atbm_void  atbm_cancel_work(struct atbmwifi_common *hw_priv,struct atbmwifi_work_struct *work)
{
	if(work->valid){
		work->valid=0;
		atbm_kfree(work);
	}
	work=ATBM_NULL;
}

atbm_uint32 atbm_queue_work(struct atbmwifi_common *hw_priv, struct atbmwifi_work_struct *work)
{
	atbm_uint32 irq_flg;
	irq_flg = atbm_local_irq_save();
	atbm_list_add_tail(&work->workLink, &hw_priv->work_Link);
	atbm_local_irq_restore(irq_flg);
	atbm_wakeupThreadInternal(hw_priv->work_queue_thread);
	return 0;
}
int atbm_create_workqueue(struct atbmwifi_common *hw_priv)
{
	atbm_uint32 ret;

	ATBM_INIT_LIST_HEAD(&hw_priv->work_Link);
	hw_priv->work_queue_thread=atbm_createThreadInternal("atbm_work",
											atbm_workqueue_thread,
											(atbm_void*)hw_priv,
											WORK_TASK_PRIO,
											10*ATBM_HZ);
	if (!hw_priv->work_queue_thread){
		wifi_printk(WIFI_IF,"work_queue_thread Failed\n");
		ret = WIFI_ERROR;
	}else{
		ret = WIFI_OK;
	}

	return ret;
}

int atbm_destory_task_work(struct atbmwifi_common *hw_priv)
{
	int ret = 0;
	//wifi_printk(WIFI_ALWAYS,"atbm_destory_task_work\n");
	hw_priv->bh_term=1;
	atbm_stopThreadInternal(hw_priv->work_queue_thread);
	return ret;
}
#else

int atbm_task_work(atbm_void *arg, int wakeup_state);
int atbm_create_workqueue(struct atbmwifi_common *hw_priv)
{
	int ret;
	atbm_memset(&hw_priv->work_queue_table[0],0,sizeof(struct atbmwifi_work_struct)*ATBM_WIFI_MAX_WORKQUEUE);
	hw_priv->work_map =0;
	//sema_init(&hw_priv->work_wq,0);
	hw_priv->work_queue_thread=atbm_createThreadInternal("atbm_work",
												atbm_task_work,
												(atbm_void*)hw_priv,
												WORK_TASK_PRIO,
												10*ATBM_HZ);
	if (!hw_priv->work_queue_thread){
		wifi_printk(WIFI_IF,"work_queue_thread Failed\n");
		ret = WIFI_ERROR;
	}else{
		ret = WIFI_OK;
	}

	return ret;
}
int atbm_destory_task_work(struct atbmwifi_common *hw_priv)
{
	int ret = 0;
	//wifi_printk(WIFI_ALWAYS,"atbm_destory_task_work\n");
	hw_priv->bh_term=1;
	atbm_stopThreadInternal(hw_priv->work_queue_thread);
	return ret;
}

int atbm_task_work(atbm_void *arg, int wakeup_state)
{

	struct atbmwifi_common *hw_priv = (struct atbmwifi_common *)arg;
	int id=0;
	struct atbmwifi_work_struct *work;
	atbm_work_fun fun;

	for(id=0;id<ATBM_WIFI_MAX_WORKQUEUE;id++){		
		work = &hw_priv->work_queue_table[id];
		fun = work->fun;
		if(atbm_test_bit(id,&hw_priv->work_map)){		
			atbm_clear_bit(id,&hw_priv->work_map);
			wifi_printk(WIFI_TASK,"work->fun-id %d-\n",id);
			fun(work->data);				
		}
		if(hw_priv->work_map ==0)
			break;
	}

	//atbm_stopThreadInternal(hw_priv->work_queue_thread);
	return 0;
}

atbm_void atbm_queue_work(struct atbmwifi_common *hw_priv,atbm_work workid)
{
	struct atbmwifi_work_struct *work;

	
	wifi_printk(WIFI_IF,"atbm_queue_work1++ workid %d\n",workid); 
	work = &hw_priv->work_queue_table[workid];
	if(work->valid ==0){
		wifi_printk(WIFI_ALWAYS,"atbm_queue_work error !\n");
		return;
	}
	if(atbm_atomic_read(&work->pending)==0){
		wifi_printk(WIFI_TASK,"atbm_queue_work _wakeup_\n");
		//atbm_atomic_set(&work->pending,1);
		atbm_set_bit(workid,&hw_priv->work_map);
		atbm_wakeupThreadInternal(hw_priv->work_queue_thread);
	}
}

atbm_work atbm_init_work(struct atbmwifi_common *hw_priv,atbm_work_fun fun, atbm_void *data)
{
	int id = -1;
	struct atbmwifi_work_struct *work;	
	atbm_uint32 irq_flg;
	irq_flg = atbm_local_irq_save();
	for(id=0;id<ATBM_WIFI_MAX_WORKQUEUE;id++){		
		work = &hw_priv->work_queue_table[id];
		if(work->valid ==0){	
			work->fun = fun;		
			work->data = data;
			work->index= id;
			work->valid= 1;	
			break;
		}
	}
	atbm_local_irq_restore(irq_flg);
	
	ATBM_ASSERT(id != -1);

	return id;
}

atbm_void  atbm_cancel_work(struct atbmwifi_common *hw_priv,atbm_work work_id)
{
	struct atbmwifi_work_struct *work;
	atbm_uint32 irq_flg;
	irq_flg = atbm_local_irq_save();

	work = &hw_priv->work_queue_table[work_id];

	atbm_clear_bit(work_id,&hw_priv->work_map);
	if(work->valid == 1){
		work->valid = 0;
		work->fun = ATBM_NULL;
		work->data = ATBM_NULL;
		work->index= 0;
	}

	atbm_local_irq_restore(irq_flg);
	return;
}

#endif

int atbm_hmac_xmit_data(struct sbus_priv *self);
atbm_void atbm_bh_schedule_tx(struct atbmwifi_common	*hw_priv)
{
#if (ATBM_TX_WRITE_TASK == 1)
	atbm_hmac_xmit_data(hw_priv->sbus_priv);
#else
	atbm_atomic_set(&hw_priv->bh_tx, 1);
	atbm_wakeupThreadInternal(hw_priv->bh_thread);
#endif
}
atbm_void atbm_bh_wakeup(struct atbmwifi_common *hw_priv)
{
	if ((hw_priv->bh_error)||atbm_bh_is_term(hw_priv)){
		wifi_printk(WIFI_IF,"atbm_wifi [BH] err drop\n");
		return;
	}
	/*wakeup TxThread to transmit.........*/
	
	atbm_bh_schedule_tx(hw_priv);
}

