/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#include "atbm_hal.h"
#include "../../include/svn_version.h"
#include "atbm_sdio_hwio.h"

static atbm_os_msgq atbm_sdio_tx_msgQ;
static ATBM_INF_DESC *atbm_urb_tx_msqQbuf[TX_DESC_NUM+1];
static atbm_os_msgq atbm_sdio_rx_msgQ ;
static ATBM_INF_DESC *atbm_urb_rx_msqQbuf[RX_DESC_NUM+1];

extern atbm_void atbm_core_release(struct atbmwifi_common *hw_priv);
extern int atbm_hmac_receive_data(struct sbus_priv *self,unsigned int addr,atbm_void  *dst, int count);
extern int atbm_hmac_xmit_data(struct sbus_priv *self);
extern int atbm_reg_read_16(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_uint16 *val);
int atbm_sdio_set_block_size(struct sbus_priv *self, int type, atbm_uint32 size);

extern struct atbmwifi_common g_hw_prv;
static int atbm_wifi_inif = 0;

void atbm_wifi_set_init_flag(int value)
{
	atbm_wifi_inif = value;
}

int atbm_wifi_get_init_flag(void)
{
	return atbm_wifi_inif;
}

#define DPLL_CLOCK 24
struct build_info{
	int ver;
	int dpll;
	char driver_info[64];
};
const char DRIVER_INFO[]={"[===SDIO-ATHENAB=="};
static int driver_build_info(atbm_void)
{
	struct build_info build;
	build.ver=SVN_VERSION;
	build.dpll=DPLL_CLOCK;
	atbm_memcpy(build.driver_info,(atbm_void*)DRIVER_INFO,sizeof(DRIVER_INFO));
	wifi_printk(WIFI_DBG_ANY,"SVN_VER=%d,DPLL_CLOCK=%d,BUILD_TIME=%s\n",build.ver,build.dpll,build.driver_info);
	wifi_printk(WIFI_DBG_ANY,"BRT SVN_VER=%s\n", BRT_SVN_VERSION);
	return 0;
}

int atbm_sdio_suspend(struct atbmwifi_common *hw_priv)
{
	return 0;
}
int atbm_sdio_resume(struct atbmwifi_common *hw_priv)
{
	return 0;
}

static int atbm_sdio_memcpy_fromio(struct sbus_priv *self,
				     unsigned int addr,
				     void *dst, int count)
{
	int i=0,ret = 0;
	if(atbm_atomic_add_return(0, &self->rx_lock)){
		wifi_printk(WIFI_ALWAYS,"%s %d \n",__func__,__LINE__);
		return -1;
	}

	for(i=0;i<RX_DESC_NUM;i++){
	 	ret = atbm_hmac_receive_data(self,addr,dst,count);
		if(ret < 0){
			wifi_printk(WIFI_ALWAYS,"%s %d atbm_hmac_receive_data err\n",__func__,__LINE__);
		}
	}
	return 0;
}
static int atbm_sdio_memcpy_toio(struct sbus_priv *self,
				   unsigned int addr,
				   const void *src, int count)
{
	return __atbm_sdio_memcpy_toio(self->func, addr, (void *)src, count);
}

static int atbm_sdio_read_sync(struct sbus_priv *self,
				     unsigned int addr,
				     void *dst, int count)
{
	return __atbm_sdio_memcpy_fromio(self->func, dst, addr, count);
}

static void atbm_sdio_lock(struct sbus_priv *self)
{
	atbm_sdio_claim_host(self->func);
}
static void atbm_sdio_unlock(struct sbus_priv *self)
{
	atbm_sdio_release_host(self->func);
}
static int atbm_sdio_abort(struct sbus_priv *self)
{
	wifi_printk(WIFI_ALWAYS,"%s %d \n",__func__,__LINE__);
	return 0;
}

static void atbm_sdio_irq_handler(struct atbm_sdio_func *func)
{
	struct sbus_priv *self = atbm_sdio_get_drvdata(func);

	ATBM_BUG_ON(!self);

	if (self->irq_handler)
		self->irq_handler(self->irq_priv);

}

static int atbm_sdio_irq_subscribe(struct sbus_priv *self,
				     sbus_irq_handler handler,
				     void *priv)
{
	int ret;
	unsigned long flags;

	if (!handler)
		return -ATBM_EINVAL;

	atbm_spin_lock_irqsave(&self->lock, &flags);
	self->irq_priv = priv;
	self->irq_handler = handler;
	atbm_spin_unlock_irqrestore(&self->lock, flags);
	atbm_sdio_claim_host(self->func);
	ret = atbm_sdio_claim_irq(self->func, atbm_sdio_irq_handler);
	if (ret)
		wifi_printk(WIFI_IF,"Failed to claim sdio Irq :%d\n",ret);
	atbm_sdio_release_host(self->func);
	return ret;
}

static void atbm_sdio_irq_en(struct sbus_priv *self,atbm_uint8 en)
{

}
static int atbm_sdio_irq_unsubscribe(struct sbus_priv *self)
{
	int ret = 0;
	unsigned long flags;
	//const struct resource *irq = self->pdata->irq;

	ATBM_WARN_ON_FUNC(!self->irq_handler);
	if (!self->irq_handler)
		return 0;

	atbm_sdio_claim_host(self->func);
	ret = atbm_sdio_release_irq(self->func);
	atbm_sdio_release_host(self->func);
	atbm_spin_lock_irqsave(&self->lock, &flags);
	self->irq_priv = ATBM_NULL;
	self->irq_handler = ATBM_NULL;
	atbm_spin_unlock_irqrestore(&self->lock, flags);

	return ret;
}

static int atbm_sdio_reset(struct sbus_priv *self)
{
	int ret;
	int regdata;
	int func_num;

	return 0;
	wifi_printk(WIFI_IF,"atbm_sdio_reset++\n");
	atbm_sdio_claim_host(self->func);

	/**********************/
	wifi_printk(WIFI_IF,"SDIO_RESET++\n");
	/* SDIO Simplified Specification V2.0, 4.4 Reset for SDIO */
	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_ABORT, &ret);
	if (ret)
		regdata = 0x08;
	else
		regdata |= 0x08;
	atbm_sdio_f0_writeb(self->func, regdata, ATBM_SDIO_CCCR_ABORT, &ret);
	if (ATBM_WARN_ON(ret))
		goto set_func0_err;
	atbm_mdelay(1500);
	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_ABORT, &ret);
	wifi_printk(WIFI_IF,"SDIO_RESET-- 0x%x\n",regdata);

	/**********************/
	wifi_printk(WIFI_IF,"ATBM_SDIO_SPEED_EHS++\n");
	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_SPEED, &ret);
	if (ATBM_WARN_ON(ret))
		goto set_func0_err;

	regdata |= ATBM_SDIO_SPEED_EHS;
	atbm_sdio_f0_writeb(self->func, regdata, ATBM_SDIO_CCCR_SPEED, &ret);
	if (ATBM_WARN_ON(ret))
		goto set_func0_err;

	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_SPEED, &ret);
	wifi_printk(WIFI_IF,"ATBM_SDIO_SPEED_EHS -- 0x%x:0x%x\n",regdata,ATBM_SDIO_SPEED_EHS);

	/**********************/
	wifi_printk(WIFI_IF,"ATBM_SDIO_BUS_WIDTH_4BIT++\n");
	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_IF, &ret);
	if (ATBM_WARN_ON(ret))
		goto set_func0_err;

	//regdata |= ATBM_SDIO_BUS_WIDTH_4BIT;
	regdata = 0xff;
	atbm_sdio_f0_writeb(self->func, regdata, ATBM_SDIO_CCCR_IF, &ret);
	if (ATBM_WARN_ON(ret))
		goto set_func0_err;
	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_IF, &ret);
	wifi_printk(WIFI_IF,"ATBM_SDIO_BUS_WIDTH_4BIT -- 0x%x:0x%x\n",regdata,ATBM_SDIO_BUS_WIDTH_4BIT);
	/**********************/
	wifi_printk(WIFI_IF,"SDIO_BUS_ENABLE_FUNC++\n");
	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_IOEx, &ret);
	if (ATBM_WARN_ON(ret))
		goto set_func0_err;
	regdata |= ATBM_BIT(func_num);
	wifi_printk(WIFI_IF,"SDIO_BUS_ENABLE_FUNC regdata %x\n",regdata);
	atbm_sdio_f0_writeb(self->func, regdata, ATBM_SDIO_CCCR_IOEx, &ret);
	if (ATBM_WARN_ON(ret))
		goto set_func0_err;
	regdata = atbm_sdio_f0_readb(self->func, ATBM_SDIO_CCCR_IOEx, &ret);
	wifi_printk(WIFI_IF,"SDIO_BUS_ENABLE_FUNC -- 0x%x\n",regdata);
	/**********************/
set_func0_err:
	atbm_sdio_set_block_size(self,0,ATBM_SDIO_BLOCK_SIZE);
	/* Restore the WLAN function number */
	atbm_sdio_release_host(self->func);
	return 0;
}
static atbm_uint32 atbm_sdio_align_size(struct sbus_priv *self, atbm_uint32 size)
{
	atbm_uint32 aligned = atbm_sdio_alignsize(self->func, size);
	return aligned;
}
int atbm_sdio_set_block_size(struct sbus_priv *self, int type, atbm_uint32 size)
{
	if(type && self->cmd_func){
		return atbm_sdio_set_blocksize(self->cmd_func, size);
	}else{
		return atbm_sdio_set_blocksize(self->func, size);
	}
}
static int atbm_sdio_pm(struct sbus_priv *self, ATBM_BOOL  suspend)
{
	int ret = 0;
	return ret;
}

static struct dvobj_priv *sdio_dvobj_init()
{
	struct dvobj_priv *pdvobjpriv=ATBM_NULL;
	
	pdvobjpriv = (struct dvobj_priv *)atbm_kzalloc(sizeof(*pdvobjpriv),GFP_KERNEL);
	if (!pdvobjpriv){
		wifi_printk(WIFI_IF, "Can't allocate USB dvobj.");
		goto exit;
	}
	pdvobjpriv->tx_desc_num_put = TX_DESC_NUM;
	pdvobjpriv->rx_desc_num_put = RX_DESC_NUM;
exit:
	return pdvobjpriv;
}

atbm_void atbm_sdio_desc_free(struct sbus_priv *self,struct sbus_desc *pDesc,int max_num)
{
	int i=0;
#if CONFIG_INF_AGGR_TX
	if(self->tx_dma_addr_buffer){
		atbm_kfree(self->tx_dma_addr_buffer);
		self->tx_dma_addr_buffer = ATBM_NULL;
	}
#endif //CONFIG_INF_AGGR_TX
	for(i=0;i<max_num;i++){
		atbm_kfree(pDesc[i].test_desc);
		pDesc[i].test_desc = ATBM_NULL;
		if(pDesc[i].test_skb){
			atbm_dev_kfree_skb(pDesc[i].test_skb);
			pDesc[i].test_skb = ATBM_NULL;
		}
	}
}

int atbm_sdio_desc_malloc(struct sbus_priv *self,struct sbus_desc *pDesc,int max_num,int len)
{
	int i=0;
	for(i=0;i<max_num;i++){
		pDesc[i].test_desc = (ATBM_INF_DESC*)atbm_kzalloc(sizeof(ATBM_INF_DESC),0);
		if (!pDesc[i].test_desc){
			wifi_printk(WIFI_DBG_ERROR, "Can't allocate test_urb.");
			goto __free_urb;
		}
		pDesc[i].test_skb = ATBM_NULL;
		pDesc[i].desc_id = i;
		pDesc[i].obj =self;
		
		pDesc[i].link =0;
	}
	return 0;
__free_urb:
	for( ;i>=0;--i){
		atbm_kfree(pDesc[i].test_desc);
	}

	return -ATBM_ENOMEM;
}

/* Probe Function to be called by SDIO stack when device is discovered */
int atbm_sdio_probe(struct atbm_sdio_func *func,
			      const struct atbm_sdio_device_id *id)
{
	struct sbus_priv *self;
	int ret;

	wifi_printk(WIFI_IF,"Probe called\n");

	if(func->num != ATBM_SDIO_CARE_NUM)
		return 0;	
	//atbm_atomic_set(&g_wtd.wtd_probe, 0);
#ifdef LINUX_OS
	func->card->quirks|=MMC_QUIRK_LENIENT_FN0;
	func->card->quirks |= MMC_QUIRK_BLKSZ_FOR_BYTE_MODE;
#endif

	self = (struct sbus_priv *)atbm_kzalloc(sizeof(*self), GFP_KERNEL);
	if (!self) {
		wifi_printk(WIFI_DBG_ERROR, "Can't allocate SDIO sbus_priv.");
		return -1;
	}
	atbm_spin_lock_init(&self->lock);
	self->func = func;
	self->drvobj = sdio_dvobj_init();
	atbm_sdio_set_drvdata(func, self);
	atbm_sdio_claim_host(func);
	ret=atbm_sdio_enable_func(func);
	if(ret){
		atbm_sdio_disable_func(func);
		atbm_sdio_release_host(func);
		atbm_kfree(self);
		return -1;
	}
	atbm_sdio_release_host(func);
	ret = atbm_sdio_desc_malloc(self,self->rx_desc,RX_DESC_NUM,RX_BUFFER_SIZE);
	if (ret != 0){
		wifi_printk(WIFI_DBG_ERROR, "Can't allocate rx_desc.");
		return ret;
	}
	/*3---alloc tx_urb*/
	ret = atbm_sdio_desc_malloc(self,self->tx_desc,TX_DESC_NUM,TX_BUFFER_SIZE);
	if (ret != 0){
		wifi_printk(WIFI_DBG_ERROR, "Can't allocate tx_desc.");
		return ret;
	}
	ret=Atbmwifi_halEntry(self);
	if (ret) {
		atbm_kfree(self);
		atbm_sdio_claim_host(func);
		atbm_sdio_disable_func(func);
		atbm_sdio_release_host(func);
		atbm_sdio_set_drvdata(func, NULL);
		//atbm_atomic_set(&g_wtd.wtd_probe, -1);
		atbm_wifi_set_init_flag(0);
	}
	else {
		//atbm_atomic_set(&g_wtd.wtd_probe, 1);
		wifi_printk(WIFI_IF,"[atbm_wtd]:set wtd_probe = 1\n");
		atbm_wifi_set_init_flag(1);
	}
	return ret;
}

void atbm_build_wsm_header_v0(struct atbmwifi_common *hw_priv, struct wsm_hdr_tx *wsm_tx, int *tx_len){
	atbm_uint16 wsm_len_u16[2];
	atbm_uint16 wsm_len_sum;
	wsm_len_u16[0] = wsm_tx->u.common.total_len & 0xff;
	wsm_len_u16[1] = (wsm_tx->u.common.total_len >> 8) & 0xff;
	wsm_len_sum = wsm_len_u16[0] + wsm_len_u16[1];
	if (wsm_len_sum & ATBM_BIT(8)){
	  wsm_tx->u.common.mark = atbm_cpu_to_le16(((wsm_len_sum + 1) & 0xff) << 8);
	}else{
	  wsm_tx->u.common.mark = atbm_cpu_to_le16((wsm_len_sum & 0xff) << 8);
	}
}

int atbm_sdio_tx_submit(ATBM_INF_DESC *desc)
{
	atbm_uint32 status;

	status = atbm_os_MsgQ_Send(&atbm_sdio_tx_msgQ, &desc,sizeof(ATBM_INF_DESC*),ATBM_TX_WAIT_FOREVER);
	if (0 != status){
		wifi_printk(WIFI_ALWAYS, "atbm_sdio_tx_msgQ send failed 0x%x\n", status);
	}

#if PROCESS_TXRX_IN_ONE_THREAD
	extern struct atbmwifi_common g_hw_prv;
#if PROCESS_RX_IN_IRQ_THREAD
	atbm_sdio_wakeup_irq_thread(g_hw_prv.sbus_priv->func);
#else
	atbm_wakeupThreadInternal(g_hw_prv.atbm_sdio_rx_thread);
#endif
#endif
	return status;
}

int atbm_sdio_rx_submit(ATBM_INF_DESC *desc)
{
	atbm_uint32 status;

	status = atbm_os_MsgQ_Send(&atbm_sdio_rx_msgQ, &desc,sizeof(ATBM_INF_DESC*),ATBM_TX_WAIT_FOREVER);
	if (0 != status){
		wifi_printk(WIFI_ALWAYS, "atbm_sdio_rx_msgQ send failed 0x%x\n", status);
	}
	
	return status;
}

ATBM_BOOL atbm_sdio_wait_enough_space(struct atbmwifi_common	*hw_priv, atbm_uint32 n_needs)
{
#define MAX_LOOP_POLL_CNT  (2*3000)
	atbm_uint32 hw_xmited = 0;
	ATBM_BOOL enough = ATBM_FALSE;
	int ret = 0;
	int loop = 0;
	atbm_uint32 print = 0;

	enough = hw_priv->hw_bufs_free >= n_needs ? ATBM_TRUE : ATBM_FALSE;
	while(enough == ATBM_FALSE){
		if(atbm_bh_is_term(hw_priv)){
			wifi_printk(WIFI_DBG_ERROR, "%s:bh term\n",__func__);
			return ATBM_FALSE;
		}

		hw_priv->hw_bufs_free = atbm_sdio_get_tx_free_buffers(hw_priv);
		if(ret){
			enough = ATBM_FALSE;
			break;
		}
		
		enough = hw_priv->hw_bufs_free >= n_needs ? ATBM_TRUE : ATBM_FALSE;
		atbm_spin_lock_bh(&hw_priv->tx_com_lock);
		hw_priv->hw_buf_enough = enough;
		atbm_spin_unlock_bh(&hw_priv->tx_com_lock);

		if(enough == ATBM_FALSE){
			loop ++;
			if(loop>=MAX_LOOP_POLL_CNT)
				break;
			if((loop >= 3)&&(print == 0)){			
				wifi_printk(WIFI_DBG_MSG, "%s:n_xmits(%d),hw_xmited(%d),need(%d)\n",__func__,
					hw_priv->n_xmits,hw_xmited,n_needs);
				print = 1;
			}
			atbm_os_wait_event_timeout(&hw_priv->wake_up_wq, 5);
		}
	}

	return enough;
}

ATBM_BOOL atbm_sdio_have_enough_space(struct atbmwifi_common	*hw_priv, atbm_uint32 n_needs)
{
	ATBM_BOOL enough = ATBM_FALSE;
	int ret = 0;

	enough = hw_priv->hw_bufs_free >= n_needs ? ATBM_TRUE : ATBM_FALSE;
	if(enough == ATBM_FALSE){
		hw_priv->hw_bufs_free = atbm_sdio_get_tx_free_buffers(hw_priv);
		enough = hw_priv->hw_bufs_free >= n_needs ? ATBM_TRUE : ATBM_FALSE;
	}
	return enough;
}

#if SUPPORT_LIGHT_SLEEP
static int atbm_device_wakeup(struct atbmwifi_common *hw_priv)
{
	atbm_uint16 ctrl_reg;
	int ret=0;
	int loop = 1;

	/* To force the device to be always-on, the host sets WLAN_UP to 1 */
	ret = atbm_reg_write_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID,
			ATBM_HIFREG_CONT_WUP_BIT);
	if (ATBM_WARN_ON(ret))
		return ret;

	while(1){
		ret = atbm_bh_read_ctrl_reg(hw_priv, &ctrl_reg);
		if (ATBM_WARN_ON(ret)){
			wifi_printk(WIFI_ALWAYS,"[BH] wake up err\n");
			goto __loop_continue;
		}
		/* If the device returns WLAN_RDY as 1, the device is active and will
		 * remain active. */
		if (ctrl_reg & ATBM_HIFREG_CONT_RDY_BIT) {
			wifi_printk(WIFI_BH,"[BH] Device awake.<%d>\n",loop);
			ret= 1;
			goto __wup_exit;
		}
__loop_continue:
		//wait 1S,else fail
		if(loop++ > 1000){
			ret = atbm_bh_read_ctrl_reg(hw_priv, &ctrl_reg);
			wifi_printk(WIFI_ALWAYS,"[BH] Device wakeup Fail<%d> %x\n",loop,ctrl_reg);
			ret= -1;
			goto __wup_exit;
		}
		atbm_mdelay(2);
	}
	wifi_printk(WIFI_BH,"[BH]  wakeup err\n");
__wup_exit:
#if PROCESS_RX_IN_IRQ_THREAD
	//#error "Add code to wake up sdio irq thread here"
	atbm_sdio_wakeup_irq_thread(hw_priv->sbus_priv->func);
#else
	if(ctrl_reg & ATBM_HIFREG_CONT_NEXT_LEN_MASK){
		atbm_wakeupThreadInternal(&hw_priv->atbm_sdio_rx_thread);
	}
#endif
	return ret;
}

#if LIGHT_SLEEP_DELAY
static void atbm_light_sleep_delay_expire(atbm_void *data1,atbm_void *data2){
	struct atbmwifi_common	*hw_priv = data1;
	atbm_os_mutexLock(&hw_priv->sleep_mutex, 0);
	if((hw_priv->device_can_sleep == ATBM_FALSE) && (hw_priv->ps_flag == 0)){
		wifi_printk(WIFI_DBG_MSG, "tx Enter light sleep!!\n");
		ATBM_WARN_ON(atbm_reg_write_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID, 0));
		hw_priv->device_can_sleep = ATBM_TRUE;
	}
	atbm_os_mutexUnLock(&hw_priv->sleep_mutex);
}
#endif

void _atbm_light_sleep_try_to_wakeup(struct atbmwifi_common *hw_priv, int reason){
	atbm_os_mutexLock(&hw_priv->sleep_mutex, 0);
	if(hw_priv->device_can_sleep){
		ATBM_WARN_ON(atbm_device_wakeup(hw_priv) < 0);
		hw_priv->device_can_sleep = ATBM_FALSE;
	}
	hw_priv->ps_flag |= ATBM_BIT(reason);
	atbm_os_mutexUnLock(&hw_priv->sleep_mutex);
}

void _atbm_light_sleep_try_to_keepalive(struct atbmwifi_common *hw_priv, int reason){
	atbm_os_mutexLock(&hw_priv->sleep_mutex, 0);
	hw_priv->ps_flag |= ATBM_BIT(reason);
	atbm_os_mutexUnLock(&hw_priv->sleep_mutex);
}

void _atbm_light_sleep_try_to_sleep(struct atbmwifi_common *hw_priv, int reason){
	atbm_os_mutexLock(&hw_priv->sleep_mutex, 0);
	hw_priv->ps_flag &= ~ATBM_BIT(reason);
	if((hw_priv->device_can_sleep == ATBM_FALSE) && (hw_priv->ps_flag == 0)){
#if LIGHT_SLEEP_DELAY
		hw_priv->last_active_time = atbm_GetOsTime();
		atbmwifi_eloop_cancel_timeout(atbm_light_sleep_delay_expire, hw_priv, ATBM_NULL);
		atbmwifi_eloop_register_timeout(0, LIGHT_SLEEP_DELAY, atbm_light_sleep_delay_expire, hw_priv, ATBM_NULL);
#else
		wifi_printk(WIFI_DBG_MSG, "tx Enter light sleep!!\n");
		ATBM_WARN_ON(atbm_reg_write_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID, 0));
		hw_priv->device_can_sleep = ATBM_TRUE;
#endif	
	}
	atbm_os_mutexUnLock(&hw_priv->sleep_mutex);	
}
#endif

static int atbm_sdio_process_tx(struct atbmwifi_common *hw_priv, ATBM_INF_DESC *pDesc){
	struct sbus_desc *tx_desc = (struct sbus_desc *)pDesc->context;
	int tx_count, tx_sg, frame_cnt;
	int status = 0;

#if (ATBM_SUPPORT_SG || CONFIG_INF_AGGR_TX)
	tx_count = tx_desc->frame_cnt;
#else
	tx_count = 1;
#endif
	tx_sg = 0;
	frame_cnt = tx_desc->frame_cnt;	
	do{
		hw_priv->n_xmits += tx_count;
		hw_priv->hw_bufs_free -= tx_count;
		hw_priv->buf_id_tx_offset = tx_count;

#if ATBM_SUPPORT_SG
		if(tx_count > 1){
			if (ATBM_WARN_ON(atbm_data_write_sgs(hw_priv,(const void **)pDesc->buf, pDesc->len))) { 			
				wifi_printk(WIFI_DBG_ERROR, "%s: xmit data err\n",__func__);
				status = -1;
				break;
			}
		}else
#endif
		if (ATBM_WARN_ON(atbm_data_write(hw_priv,pDesc->buf[tx_sg], pDesc->len[tx_sg]))) {		
			wifi_printk(WIFI_DBG_ERROR, "%s: xmit data err\n",__func__);
			status = -1;
			break;
		}
		frame_cnt -= tx_count;
		tx_sg++;
	}while(frame_cnt > 0);
	pDesc->status = status;
	/*Tx Urb complete*/
	pDesc->complete(pDesc);
	return status;
}

/*
 *1. No desc return 0
 *2. Have desc need wait 0x10000
 *3. Have desc write bytes return total len + 0x10000
 *4. No desc write bytes return total len
*/
int atbm_sdio_tx_queue_process(struct atbmwifi_common *hw_priv, atbm_uint32 timeout, ATBM_BOOL wait){
	int status;
	static ATBM_INF_DESC *pDesc = ATBM_NULL;
	struct sbus_desc *tx_desc;
	ATBM_BOOL enough;
	int total_len = 0;

	if(!pDesc){
		status = atbm_os_MsgQ_Recv(&atbm_sdio_tx_msgQ,&pDesc,sizeof(ATBM_INF_DESC *),timeout);
		if(status != 0){
			pDesc = ATBM_NULL;
			return total_len;
		}
	}
	atbm_light_sleep_try_to_wakeup(hw_priv, ATBM_SDIO_WAKEUP_REASON_TX_PROGRESS);
TxAction:
	tx_desc = (struct sbus_desc *)pDesc->context;
	enough = atbm_sdio_have_enough_space(hw_priv,tx_desc->frame_cnt);
	if(enough == ATBM_FALSE){
		goto TxWait;
	}
TxContinue:
	total_len += 1;
	status = atbm_sdio_process_tx(hw_priv, pDesc);
	if(status){
		goto TxErr;
	}
	status = atbm_os_MsgQ_Recv(&atbm_sdio_tx_msgQ,&pDesc,sizeof(ATBM_INF_DESC *),timeout);
	if(status != 0){
		goto TxFinished;
	}
	goto TxAction;
TxWait:
	if(wait == ATBM_FALSE){
		total_len += 0x10000;
		return total_len;
	}
	if((enough == ATBM_FALSE)&&(atbm_sdio_wait_enough_space(hw_priv,tx_desc->frame_cnt) == ATBM_FALSE)){
		wifi_printk(WIFI_DBG_ERROR, "%s: wait space timeout\n",__func__);
		goto TxErr;
	}
	goto TxContinue;
TxErr:
	pDesc->status = -1;
	/*Tx Urb complete*/
	pDesc->complete(pDesc);
TxFinished:
	pDesc = ATBM_NULL;
	atbm_light_sleep_try_to_sleep(hw_priv, ATBM_SDIO_WAKEUP_REASON_TX_PROGRESS);	
	return total_len;
}

#if ATBM_TX_WRITE_TASK
int atbm_sdio_tx_queue_task(void *arg, int wakeup_state)
{
	struct atbmwifi_common *hw_priv = (struct atbmwifi_common *)arg;

	while(!hw_priv->bh_term){
		atbm_sdio_tx_queue_process(hw_priv, ATBM_TX_WAIT_FOREVER, ATBM_TRUE);
	}
	return 0;
}
#endif

static int atbm_sdio_process_rx(struct atbmwifi_common *hw_priv, int read_len, ATBM_INF_DESC *pDesc){
	atbm_uint32 read_len_lsb = 0;
	atbm_uint32 read_len_msb = 0;
	atbm_uint32 alloc_len = 0;
	int status = 0;
	struct sbus_desc *rx_desc;

	/* Add SIZE of PIGGYBACK reg (CONTROL Reg)
	 * to the NEXT Message length + 2 Bytes for SKB */
	alloc_len = read_len;
	if (alloc_len % ATBM_SDIO_BLOCK_SIZE ) {
		alloc_len -= (alloc_len % ATBM_SDIO_BLOCK_SIZE );
		alloc_len += ATBM_SDIO_BLOCK_SIZE;
	}

	/* Check if not exceeding CW1200 capabilities */
	if (ATBM_WARN_ON(alloc_len > EFFECTIVE_BUF_SIZE)) {	
		wifi_printk(WIFI_ALWAYS,"Read aligned len: %d\n",
			alloc_len);
	}

	rx_desc = (struct sbus_desc *)pDesc->context;
	rx_desc->test_skb = atbm_dev_alloc_skb(alloc_len);
	if (ATBM_WARN_ON(atbm_data_read(hw_priv,ATBM_OS_SKB_DATA(rx_desc->test_skb), alloc_len))) {	
		wifi_printk(WIFI_ALWAYS, "%s: rx data err\n",__func__);
		status = -1;
	}
	//pDesc->actual_length = ((struct wsm_hdr *)(pDesc->buf[0]))->len;
	pDesc->actual_length = alloc_len;
	pDesc->status = status;

	read_len = atbm_sdio_get_rx_len(hw_priv, __atbm_le16_to_cpu(
			((atbm_uint16*)ATBM_OS_SKB_DATA(rx_desc->test_skb))[alloc_len / 2 - 1]));
	pDesc->complete(pDesc);
	return read_len;
}

/*
 *1. No desc return 0x10000
 *2. Have desc no read len return 0 
 *3. Have desc read bytes return total len
 *4. Have desc read bytes -> no desc 0x10000 + total_len
*/
int atbm_sdio_rx_queue_process(struct atbmwifi_common *hw_priv, int wakeup_state, atbm_uint32 timeout){
	atbm_uint16 rx_len = 0;
	int total_len = 0;
	static ATBM_INF_DESC *pDesc = ATBM_NULL;
	int status;

	if(!pDesc){
		status = atbm_os_MsgQ_Recv(&atbm_sdio_rx_msgQ,&pDesc,sizeof(ATBM_INF_DESC*), timeout);
		if(status != 0){
			pDesc = ATBM_NULL;
			total_len += 0x10000;
			goto RxExit;
		}
	}

	rx_len = atbm_sdio_get_rx_len(hw_priv, rx_len);
	if(rx_len > 0){
		if(wakeup_state == 0){
			wifi_printk(WIFI_ALWAYS, "BH miss %x\n", rx_len);
		}
		atbm_light_sleep_try_to_keepalive(hw_priv, ATBM_SDIO_WAKEUP_REASON_RX_PROGRESS);
	}else{
		goto RxExit;
	}
RxAction:
	total_len += 1;
	rx_len = atbm_sdio_process_rx(hw_priv, rx_len, pDesc);
	if(rx_len > 0){
		status = atbm_os_MsgQ_Recv(&atbm_sdio_rx_msgQ,&pDesc,sizeof(ATBM_INF_DESC*), timeout);
		if(status != 0){
			pDesc = ATBM_NULL; //Not sure if packet remains
			total_len += 0x10000;
			goto RxExit;
		}
		goto RxAction;
	}
	pDesc = ATBM_NULL;
	atbm_light_sleep_try_to_sleep(hw_priv, ATBM_SDIO_WAKEUP_REASON_RX_PROGRESS);
RxExit:
	return total_len;
}

int atbm_sdio_rx_queue_task(void *arg, int wakeup_state)
{
	struct atbmwifi_common *hw_priv = (struct atbmwifi_common *)arg;
#if PROCESS_TXRX_IN_ONE_THREAD
	int tx_len, rx_len;

	do{
		tx_len = atbm_sdio_tx_queue_process(hw_priv, 0, ATBM_FALSE);
		rx_len = atbm_sdio_rx_queue_process(hw_priv, 1, 0);
		if(rx_len || tx_len)
			continue;
	}while(atbm_atomic_xchg(&hw_priv->atbm_sdio_rx_thread->count, 0) != 0);
#else
	atbm_sdio_rx_queue_process(hw_priv, wakeup_state, ATBM_TX_WAIT_FOREVER);
#endif
	atbm_sdio_host_enable_irq(1); //Enable irq at first rxframe?
	return 0;
}

void atbm_sdio_queue_init(struct atbmwifi_common *hw_priv)
{
	atbm_uint32 status;

	wifi_printk(WIFI_ALWAYS, "atbm_urb_queue_init ==>\n");

	atbm_os_init_waitevent(&hw_priv->wake_up_wq);
	hw_priv->hw_buf_enough = ATBM_TRUE;
#if SUPPORT_LIGHT_SLEEP
	atbm_os_mutexLockInit(&hw_priv->sleep_mutex);
#endif

	// Init TX MessageQ
	status=atbm_os_MsgQ_Create(&atbm_sdio_tx_msgQ,(atbm_uint32 *)&atbm_urb_tx_msqQbuf[0],sizeof(atbm_uint32),TX_DESC_NUM);
	if(status != 0){
		wifi_printk(WIFI_ALWAYS, "atbm_sdio_tx_msgQ create failed 0x%x\n", status);
		return;
	}

	// Init RX MessageQ
	status = atbm_os_MsgQ_Create(&atbm_sdio_rx_msgQ,(atbm_uint32 *)&atbm_urb_rx_msqQbuf,sizeof(atbm_uint32*),RX_DESC_NUM);
	if(status != 0){
		wifi_printk(WIFI_ALWAYS, "atbm_sdio_rx_msgQ create failed 0x%x\n", status);
		return;
	}
#if ATBM_TX_WRITE_TASK && (PROCESS_TXRX_IN_ONE_THREAD == 0)
	//Init TX Task
	hw_priv->atbm_sdio_tx_thread = atbm_createThreadInternal("Atbm_TX",
															atbm_sdio_tx_queue_task,
															hw_priv,
															TX_BH_TASK_PRIO,
															0);
	if (!hw_priv->atbm_sdio_tx_thread){
		wifi_printk(WIFI_DBG_ERROR,"bh_thread Failed\n");
		return;
	}
#endif

#if (PROCESS_RX_IN_IRQ_THREAD == 0)
	//Init RX Task
	hw_priv->atbm_sdio_rx_thread=atbm_createThreadInternal("Atbm_RX",
															atbm_sdio_rx_queue_task,
															hw_priv,
															RX_BH_TASK_PRIO,
															100*ATBM_HZ);
	if (!hw_priv->atbm_sdio_rx_thread){
		wifi_printk(WIFI_DBG_ERROR,"bh_thread Failed\n");
		return;
	}
#endif
	//Urb Init Ready
	wifi_printk(WIFI_ALWAYS, "atbm_urb_queue_init <==\n");
	return ;
}

void atbm_sdio_queue_exit(struct atbmwifi_common *hw_priv)
{
	atbm_uint32 status;
	
	wifi_printk(WIFI_ALWAYS, "atbm_urb_queue_exit ==>\n");

#if ATBM_TX_WRITE_TASK && (PROCESS_TXRX_IN_ONE_THREAD == 0)
	//Delete TX Task
	status = atbm_stopThreadInternal(hw_priv->atbm_sdio_tx_thread);
	if(status != 0){
		wifi_printk(WIFI_ALWAYS, "atbm_urb_tx_thread delete failed 0x%x\n", status);
	}
#endif

#if (PROCESS_RX_IN_IRQ_THREAD == 0)
	//Delete RX Task
	status = atbm_stopThreadInternal(hw_priv->atbm_sdio_rx_thread);
	if(status != 0){
		wifi_printk(WIFI_ALWAYS, "atbm_urb_rx_thread delete failed 0x%x\n", status);
	}
#endif

	//Delete TX MessageQ
	status = atbm_os_MsgQ_Delete(&atbm_sdio_tx_msgQ);
	if(status != 0){
		wifi_printk(WIFI_ALWAYS, "atbm_sdio_tx_msgQ delete failed 0x%x\n", status);
	}

	//Delete RX MessageQ
	status = atbm_os_MsgQ_Delete(&atbm_sdio_rx_msgQ);
	if(status != 0){
		wifi_printk(WIFI_ALWAYS, "atbm_sdio_rx_msgQ delete failed 0x%x\n", status);
	}

	atbm_os_delete_waitevent(&hw_priv->wake_up_wq);
#if SUPPORT_LIGHT_SLEEP
	atbm_os_DeleteMutex(&hw_priv->sleep_mutex);
#endif

	wifi_printk(WIFI_ALWAYS, "atbm_urb_queue_exit <==\n");
	return;
}

#if PROCESS_RX_IN_IRQ_THREAD
void atbm_irq_handler(struct atbmwifi_common *hw_priv)
{
	int rx_len;
#if PROCESS_TXRX_IN_ONE_THREAD
	int tx_len;
#endif

	/* To force the device to be always-on, the host sets WLAN_UP to 1 */
	if(!hw_priv->init_done){
		wifi_printk(WIFI_ALWAYS,"[BH] irq. init_done =0 drop\n");
		return;
	}
	if (/* WARN_ON */(hw_priv->bh_error))
		return;

#if PROCESS_TXRX_IN_ONE_THREAD
	do{
		tx_len = atbm_sdio_tx_queue_process(hw_priv, 0, ATBM_FALSE);
		rx_len = atbm_sdio_rx_queue_process(hw_priv, 1, 0);
	}while(rx_len || tx_len);
#else
	atbm_spin_lock_bh(&hw_priv->tx_com_lock);
	if(hw_priv->hw_buf_enough == ATBM_FALSE){
		atbm_os_wakeup_event(&hw_priv->wake_up_wq);
	}
	atbm_spin_unlock_bh(&hw_priv->tx_com_lock);

	atbm_sdio_rx_queue_process(hw_priv, 1, ATBM_TX_WAIT_FOREVER);
#endif
	atbm_sdio_host_enable_irq(1); //Enable irq at first rxframe?
}
#else
void atbm_irq_handler(struct atbmwifi_common *hw_priv)
{

	/* To force the device to be always-on, the host sets WLAN_UP to 1 */
	if(!hw_priv->init_done){
		wifi_printk(WIFI_ALWAYS,"[BH] irq. init_done =0 drop\n");
		return;
	}
	if (/* WARN_ON */(hw_priv->bh_error))
		return;

	atbm_spin_lock_bh(&hw_priv->tx_com_lock);
	if(hw_priv->hw_buf_enough == ATBM_FALSE){
		atbm_os_wakeup_event(&hw_priv->wake_up_wq);
	}
	atbm_spin_unlock_bh(&hw_priv->tx_com_lock);
	atbm_wakeupThreadInternal(hw_priv->atbm_sdio_rx_thread);
}
#endif

void atbm_sdio_disconnect(struct atbm_sdio_func *func)
{
	struct sbus_priv *self = atbm_sdio_get_drvdata(func);
	wifi_printk(WIFI_IF,"atbm_sdio_disconnect");
	if (self) {
		//atbm_atomic_set(&g_wtd.wtd_probe, 0);
		if (self->core) {
			atbm_core_release(self->core);
			self->core->sbus_ops->irq_unsubscribe(self->core->sbus_priv);
			//self->core = ATBM_NULL;
		}
		atbm_sdio_claim_host(func);

		if (self->core) {
			self->core = ATBM_NULL;
		}
		atbm_sdio_disable_func(func);
		atbm_sdio_release_host(func);
		atbm_sdio_set_drvdata(func, ATBM_NULL);
		atbm_sdio_desc_free(self,self->rx_desc,RX_DESC_NUM);
		atbm_sdio_desc_free(self,self->tx_desc,TX_DESC_NUM);
		atbm_kfree(self);
		atbm_wifi_set_init_flag(0);
	}
}

struct sbus_ops atbm_sdio_sbus_ops;
static int  atbm_sdio_init(atbm_void)
{
	atbm_sdio_sbus_ops.sbus_memcpy_fromio	= atbm_sdio_memcpy_fromio;
	atbm_sdio_sbus_ops.sbus_memcpy_toio	= atbm_sdio_memcpy_toio;
	atbm_sdio_sbus_ops.sbus_read_sync		= atbm_sdio_read_sync;
	atbm_sdio_sbus_ops.sbus_write_sync	= atbm_sdio_memcpy_toio;
	atbm_sdio_sbus_ops.lock				= atbm_sdio_lock;
	atbm_sdio_sbus_ops.unlock				= atbm_sdio_unlock;
	atbm_sdio_sbus_ops.reset				= atbm_sdio_reset;
	atbm_sdio_sbus_ops.align_size			= atbm_sdio_align_size;
	atbm_sdio_sbus_ops.power_mgmt			= atbm_sdio_pm;
	atbm_sdio_sbus_ops.set_block_size		= atbm_sdio_set_block_size;
	atbm_sdio_sbus_ops.irq_unsubscribe	= atbm_sdio_irq_unsubscribe;
	atbm_sdio_sbus_ops.irq_subscribe	= atbm_sdio_irq_subscribe;
	atbm_sdio_sbus_ops.abort	= atbm_sdio_abort;
	atbm_sdio_sbus_ops.sdio_irq_en = atbm_sdio_irq_en;

	wifi_printk(WIFI_IF, "atbm_sdio_init\n");
	driver_build_info();

	return atbm_sdio_register_init();
}
static atbm_void  atbm_sdio_exit(atbm_void)
{
	atbm_sdio_register_deinit();
	wifi_printk(WIFI_IF,"atbm_usb_exit:atbm_sdio_exit\n");
}
atbm_void atbm_sdio_module_init(atbm_void)
{
	wifi_printk(WIFI_ALWAYS, "atbm_sdio_module_init\n");
	atbm_init_firmware();
	wifi_printk(WIFI_ALWAYS,"[Wifi] Enter %s \n", __func__);
	atbm_sdio_init();
}
atbm_void atbm_sdio_module_exit(atbm_void)
{
	wifi_printk(WIFI_IF,"atbm_sdio_module_exit\n");
	atbm_sdio_exit();
	atbm_release_firmware();
	return ;
}

