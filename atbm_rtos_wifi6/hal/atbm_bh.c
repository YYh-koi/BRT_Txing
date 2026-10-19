/**************************************************************************************************************
 * altobeam RTOS
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#include "atbm_hal.h"
#include "../../include/svn_version.h"
#define DPLL_CLOCK 40

extern void frame_hexdump(char *prefix, atbm_uint8 *data, atbm_uint8 len);
extern int wsm_recovery(struct atbmwifi_common *hw_priv);
int atbm_hmac_receive_data(struct sbus_priv *self,unsigned int addr,atbm_void  *dst, int count);
int atbm_hmac_xmit_data(struct sbus_priv *self);
atbm_void atbm_process_rx_packet_v0(struct atbmwifi_common *hw_priv, struct atbm_buff *skb);
atbm_void atbm_process_rx_packet_v1(struct atbmwifi_common *hw_priv, struct atbm_buff *skb);
static void atbm_build_wsm_header_v1(struct atbmwifi_common *hw_priv, struct wsm_hdr_tx *wsm_tx, int *tx_len);

struct atbm_txrx_ops{
	int align_size;
	ATBM_BOOL hdr_rollback;
	void (*process_rx)(struct atbmwifi_common *hw_priv, struct atbm_buff *skb);
	void (*build_hdr)(struct atbmwifi_common *hw_priv, struct wsm_hdr_tx *wsm_tx, int *tx_len);;
};

static struct atbm_txrx_ops atbm_ops[ATBM_TXRX_OPS_MAX] = {
	{
		.hdr_rollback = ATBM_HDR_ROLLBACK,
		.build_hdr = atbm_build_wsm_header_v0,
		.process_rx = atbm_process_rx_packet_v0,
		.align_size = ATBM_PACKET_ALIGN_SIZE_V0,
	},
	{
		.hdr_rollback = ATBM_FALSE,
		.build_hdr = atbm_build_wsm_header_v1,
		.process_rx = atbm_process_rx_packet_v1,
		.align_size = ATBM_PACKET_ALIGN_SIZE_V1,
	}
};

static struct atbm_txrx_ops *current_ops;

#if ATBM_HIF_DEBUG
struct{
	int tx_skb_cnt;
	int tx_pkt_cnt;
	int rx_skb_cnt;
	int rx_pkt_cnt;
	atbm_uint32 last_print_time;
}hif_status;

static void add_tx_skb_cnt(){
	hif_status.tx_skb_cnt++;
}
static void add_tx_pkt_cnt(){
	hif_status.tx_pkt_cnt++;
}
static void add_rx_skb_cnt(){
	hif_status.rx_skb_cnt++;
}
static void add_rx_pkt_cnt(){
	hif_status.rx_pkt_cnt++;
}
static dump_hif_status(){
	if(atbm_TimeAfter(hif_status.last_print_time, 5*ATBM_HZ)){
		return;
	}
	wifi_printk(WIFI_ALWAYS, "tx %d %d rx %d %d\n", hif_status.tx_skb_cnt,
				hif_status.tx_pkt_cnt, hif_status.rx_skb_cnt, hif_status.rx_pkt_cnt);
	hif_status.tx_skb_cnt = 0;
	hif_status.tx_pkt_cnt = 0;
	hif_status.rx_skb_cnt = 0;
	hif_status.rx_pkt_cnt = 0;
	hif_status.last_print_time = atbm_GetOsTime();
}
#else
static void add_tx_skb_cnt(){}
static void add_tx_pkt_cnt(){}
static void add_rx_skb_cnt(){}
static void add_rx_pkt_cnt(){}
static void dump_hif_status(){}
#endif

static void atbm_process_decript_fifo(struct wsm_hdr_rx *hw_hdr, struct wsm_rx *rx)
{
	atbm_uint8 enc_type;

	if(atbm_unlikely(!(hw_hdr->enc_hdr.EncWriteBack & ENC_WB_PROC_SUCCESS))){
		if(hw_hdr->enc_hdr.EncWriteBack & (ENC_WB_MIC_VERIFY_ERR
						|ENC_WB_WEP_CRC_ERR|ENC_WB_BIP_MIC_FAIL)){
			rx->status = WSM_STATUS_MICFAILURE;
		}else{
			rx->status = WSM_STATUS_DECRYPTFAILURE;
		}
		wifi_printk(WIFI_DBG_ERROR, "addr:%p %p wb:%x\n", hw_hdr, rx, hw_hdr->enc_hdr.EncWriteBack);
	}
#ifdef CHKSUM_HW_SUPPORT
	else if(!(hw_hdr->enc_hdr.EncWriteBack & ENC_WB_CHECKSUM_SUCCESS)){
		rx->flags |= WSM_RX_STATUS_CHKSUM_ERROR;
	}
#endif
	enc_type = hw_hdr->enc_hdr.EncWriteBack >> ENC_WB_KEYTYPE_OFFSET;
	switch(enc_type){
		case WSM_KEY_TYPE_WEP_DEFAULT:
		case WSM_KEY_TYPE_WEP_PAIRWISE:
			rx->flags |= WSM_RX_STATUS_WEP;
			break;
		case WSM_KEY_TYPE_TKIP_GROUP:
		case WSM_KEY_TYPE_TKIP_PAIRWISE:
			rx->flags |= WSM_RX_STATUS_TKIP;
			break;
		case WSM_KEY_TYPE_AES_GROUP:
		case WSM_KEY_TYPE_AES_PAIRWISE:
			rx->flags |= WSM_RX_STATUS_AES;
			break;
		case WSM_KEY_TYPE_WAPI_GROUP:
		case WSM_KEY_TYPE_WAPI_PAIRWISE:
		case WSM_KEY_TYPE_IGTK_GROUP:
		case WSM_KEY_TYPE_NONE:
		case WSM_KEY_TYPE_UNKNOWN:
			break;
		default:
			wifi_printk(WIFI_DBG_ERROR, "1addr:%p %p wb:%x\n", hw_hdr, rx, hw_hdr->enc_hdr.EncWriteBack);
			ATBM_WARN_ON(1);
			break;
	}
}

int atbm_rx_bh_cb(struct atbmwifi_common *hw_priv,struct atbm_buff *skb, ATBM_BOOL checkseq)
{
	struct wsm_hdr *wsm;
	atbm_uint32 wsm_len = 0;
	int wsm_id;
	atbm_uint8 wsm_seq;

	wsm = (struct wsm_hdr *)ATBM_OS_SKB_DATA(skb);

	wsm_len = __atbm_le32_to_cpu(wsm->len);

	wsm_id	= __atbm_le32_to_cpu(wsm->id) & 0xFFF;

	ATBM_BUG_ON(wsm_len > 4096);
	atbm_skb_trim(skb,0);
	atbm_skb_put(skb,wsm_len);
	add_rx_skb_cnt();

	if (atbm_unlikely(wsm_id == 0x0800)) {
		wsm_handle_exception(hw_priv,
			 &ATBM_OS_SKB_DATA(skb)[sizeof(*wsm)],
			wsm_len - sizeof(*wsm));
		if(wsm_recovery(hw_priv)== RECOVERY_ERR){
			ATBM_BUG_ON(1);
		}
		goto __free;
	}
	if(checkseq){
		wsm_seq = (__atbm_le32_to_cpu(wsm->id) >> 13) & 7;	
		if (ATBM_WARN_ON(wsm_seq != hw_priv->wsm_rx_seq)) {	
			wifi_printk(WIFI_DBG_ERROR,"rx wsm_seq error %d %d \n",wsm_seq,hw_priv->wsm_rx_seq);
			if(wsm_recovery(hw_priv)== RECOVERY_ERR){
				ATBM_BUG_ON(1);
			}
			goto __free;
		}
		hw_priv->wsm_rx_seq = (wsm_seq + 1) & 7;
	}

	/* atbm_wsm_rx takes care on SKB livetime */
	if (ATBM_WARN_ON(wsm_handle_rx(hw_priv, wsm_id, wsm,&skb))){
		wifi_printk(WIFI_ALWAYS,"%s %d \n",__FUNCTION__,__LINE__);
		if(wsm_recovery(hw_priv)== RECOVERY_ERR){
			ATBM_BUG_ON(1);
		}
		goto __free;
	}

__free:
	//wifi_printk(WIFI_DBG_ERROR, "%s %d\n",__func__,__LINE__);
	if(skb != ATBM_NULL){
		atbm_dev_kfree_skb(skb);
	}

	return 0;
}
/*Tx Task Func*/
atbm_void atbm_tx_task(struct atbmwifi_common *hw_priv)
{
	int status =0;
	do {
		//if(hw_priv->bh_term)|| (hw_priv->bh_error))
		//{
		//	wifi_printk(WIFI_IF,"atbm_tx_task term(%d),err(%d)\n",atbm_atomic_read(&hw_priv->bh_term),hw_priv->bh_error);
		//	return;
		//}
		/*atbm transmit packet to device*/
		//hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
		atbm_hmac_xmit_data(hw_priv->sbus_priv);
		//hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	}while(status > 0);
}

atbm_void atbm_process_rx_packet_v0(struct atbmwifi_common *hw_priv, struct atbm_buff *skb){
#define RX_ALLOC_BUFF_OFFLOAD (  (56+16)/*RX_DESC_OVERHEAD*/+4/*FCS_LEN*/ -16 /*WSM_HI_RX_IND*/)
#if HI_RX_MUTIL_FRAME
	struct wsm_hdr *wsm;
	atbm_uint32 wsm_len, wsm_id, data_len;
	struct atbm_buff *skb_copy;
	ATBM_BOOL checkseq = ATBM_TRUE;
	ATBM_BOOL multirx = ATBM_FALSE;

	wsm = (struct wsm_hdr *)ATBM_OS_SKB_DATA(skb);
	wsm_len = __atbm_le32_to_cpu(wsm->len);
	wsm_id	= __atbm_le32_to_cpu(wsm->id) & 0xFFF;
	atbm_skb_trim(skb, wsm_len);
	//wifi_printk(WIFI_DBG_ERROR, "%s rxdata %x\n",__func__,wsm_id);
#if ATBM_USB_BUS
	if(wsm_id == WSM_MULTI_RECEIVE_INDICATION_ID){
		multirx = ATBM_TRUE; 
	}else if(wsm_id == WSM_SINGLE_CHANNEL_MULTI_RECEIVE_INDICATION_ID)
#endif
#if ATBM_SDIO_BUS
	if(wsm_id == WSM_MULTI_RECEIVE_INDICATION_ID || wsm_id == WSM_SINGLE_CHANNEL_MULTI_RECEIVE_INDICATION_ID)
#endif
	{
		atbm_uint8 wsm_seq;

		multirx = ATBM_TRUE;
		wsm_seq = (__atbm_le32_to_cpu(wsm->id) >> 13) & 7;		
		if (ATBM_WARN_ON(wsm_seq != hw_priv->wsm_rx_seq)) {		
			wifi_printk(WIFI_DBG_ERROR,"rx wsm_seq error %d %d \n",wsm_seq,hw_priv->wsm_rx_seq);
			if(wsm_recovery(hw_priv)== RECOVERY_ERR){
				ATBM_BUG_ON(1);
			}
			atbm_dev_kfree_skb(skb);
			/*atbm transmit packet to device*/
			atbm_hmac_receive_data(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
			return;
		}
		hw_priv->wsm_rx_seq = (wsm_seq + 1) & 7;
		checkseq = ATBM_FALSE;
	}

	if(multirx){
		struct wsm_multi_rx *  multi_rx = (struct wsm_multi_rx *)ATBM_OS_SKB_DATA(skb);			
		int RxFrameNum = multi_rx->RxFrameNum;
		data_len = wsm_len ;
		data_len -= sizeof(struct wsm_multi_rx);
		wsm = (struct wsm_hdr *)(multi_rx+1);
		wsm_len = __atbm_le32_to_cpu(wsm->len);
		wsm_id	= __atbm_le32_to_cpu(wsm->id) & 0xFFF;
		do {
			//wifi_printk(WIFI_DBG_ERROR, "wsm_len:%02x wsm_id:%02x\n", wsm_len, wsm_id);
			if(data_len < wsm_len){
				wifi_printk(WIFI_DBG_ERROR,"skb->len %x,wsm_len %x\n",ATBM_OS_SKB_LEN(skb),wsm_len);
				break;
			}
			ATBM_BUG_ON((wsm_id  & ~WSM_TX_LINK_ID(WSM_TX_LINK_ID_MAX)) !=  WSM_RECEIVE_INDICATION_ID);
#if ATBM_RX_REUSE_PACKET
			skb_copy = atbm_dev_reuse_skb(skb, (atbm_uint8*)wsm, wsm_len);
#else
			skb_copy = atbm_dev_alloc_skb(wsm_len + 16);
			/* In AP mode RXed SKB can be looped back as a broadcast.
			 * Here we reserve enough space for headers. */
			atbm_skb_reserve(skb_copy,  (8 - (((unsigned long)ATBM_OS_SKB_DATA(skb_copy))&7))/*ATBM_ALIGN 8*/);
			
			atbm_memmove(ATBM_OS_SKB_DATA(skb_copy), wsm, wsm_len);
			atbm_skb_put(skb_copy,wsm_len);
#endif
			atbm_rx_bh_cb(hw_priv,skb_copy,checkseq);
			data_len -= ATBM_ALIGN(wsm_len + RX_ALLOC_BUFF_OFFLOAD,4);
			RxFrameNum--;

#if (ATBM_RX_REUSE_PACKET == 0)
			if(RxFrameNum == 0){
				atbm_dev_kfree_skb(skb);
				/*atbm transmit packet to device*/
				atbm_hmac_receive_data(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
				return;
			}
#endif
			wsm = (struct wsm_hdr *)((atbm_uint8 *)wsm +ATBM_ALIGN(( wsm_len + RX_ALLOC_BUFF_OFFLOAD),4));
			wsm_len = __atbm_le32_to_cpu(wsm->len);
			wsm_id	= __atbm_le32_to_cpu(wsm->id) & 0xFFF;
#if ATBM_RX_REUSE_PACKET
			if(RxFrameNum == 1){
				atbm_skb_pull(skb, ATBM_OS_SKB_LEN(skb) - wsm_len);
				atbm_rx_bh_cb(hw_priv,skb,checkseq);
				/*atbm transmit packet to device*/
				atbm_hmac_receive_data(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
				return;
			}
#endif
		}while(1);
	}
	else
#endif
	{
		atbm_rx_bh_cb(hw_priv,skb,checkseq);
		/*atbm transmit packet to device*/
		atbm_hmac_receive_data(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
	}	
}

atbm_void atbm_process_rx_packet_v1(struct atbmwifi_common *hw_priv, struct atbm_buff *skb){
	struct wsm_hdr_rx *wsm;
	atbm_uint32 wsm_len;
	struct atbm_buff *skb_copy;

	wsm = (struct wsm_hdr_rx *)ATBM_OS_SKB_DATA(skb);
	wsm_len = __atbm_le32_to_cpu(wsm->total_len);
	do {
#if ATBM_RX_REUSE_PACKET
		if(WSM_GET_RX_ID(wsm->wsm.id) == 0x804){
			atbm_process_decript_fifo(wsm, (struct wsm_rx *)(wsm + 1));
		}
		/*If the last aggregated pkt, reuse the skb*/
		if((ATBM_OS_SKB_LEN(skb) <= (sizeof(struct wsm_hdr_rx) + wsm_len))
				|| (*((atbm_uint32*)(ATBM_OS_SKB_DATA(skb) + ATBM_ALIGN(wsm_len,4))) == 0)){
			atbm_skb_pull(skb, offsetof(struct wsm_hdr_rx, wsm));
			atbm_rx_bh_cb(hw_priv,skb,ATBM_TRUE);
			/*atbm transmit packet to device*/
			atbm_hmac_receive_data(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
			break;
		}
		skb_copy = atbm_dev_reuse_skb(skb, (atbm_uint8*)&wsm->wsm, wsm->wsm.len);
#else
		if(ATBM_OS_SKB_LEN(skb) < sizeof(struct wsm_hdr_rx) || wsm_len == 0){
			atbm_dev_kfree_skb(skb);
			/*atbm transmit packet to device*/
			atbm_hmac_receive_data(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
			break;
		}
		if(WSM_GET_RX_ID(wsm->wsm.id) == 0x804){
			atbm_process_decript_fifo(wsm, (struct wsm_rx *)(wsm + 1));
		}
		skb_copy = atbm_dev_alloc_skb(wsm->wsm.len + 16);
		/* In AP mode RXed SKB can be looped back as a broadcast.
		 * Here we reserve enough space for headers. */
		atbm_skb_reserve(skb_copy,  (8 - (((unsigned long)ATBM_OS_SKB_DATA(skb_copy))&7))/*ATBM_ALIGN 8*/);

		atbm_memmove(ATBM_OS_SKB_DATA(skb_copy), &wsm->wsm, wsm->wsm.len);
		atbm_skb_put(skb_copy, wsm->wsm.len);
#endif
		atbm_rx_bh_cb(hw_priv,skb_copy,ATBM_TRUE);
		atbm_skb_pull(skb, ATBM_ALIGN(wsm_len,4));
		wsm = (struct wsm_hdr_rx *)ATBM_OS_SKB_DATA(skb);
		wsm_len = __atbm_le32_to_cpu(wsm->total_len);
	}while(1);
}

/*Rx Task Func*/
atbm_void atbm_rx_task(struct atbmwifi_common *hw_priv)
{
	struct atbm_buff *skb ;

	while ((skb = atbm_skb_dequeue(&hw_priv->rx_frame_queue)) != ATBM_NULL) {
		if(hw_priv->bh_term|| hw_priv->bh_error)
			break;
		current_ops->process_rx(hw_priv, skb);
	}
}

void atbm_hmac_coml(struct atbmwifi_common *hw_priv){
	unsigned long flags=0;
    int ret = 0;
	int i;
	if(atbm_bh_is_term(hw_priv))
	{
		wifi_printk(WIFI_ALWAYS,"atbm_urb_coml Error\n");
		return;
	}
	atbm_spin_lock_irqsave(&hw_priv->tx_com_lock,&flags);
	while (!atbm_list_empty(&hw_priv->tx_urb_cmp)) {
	    struct sbus_desc *desc_cmp = atbm_list_first_entry(&hw_priv->tx_urb_cmp, struct sbus_desc, list);
		atbm_list_del(&desc_cmp->list);
		atbm_spin_unlock_irqrestore(&hw_priv->tx_com_lock,flags);
		struct sbus_priv *self = desc_cmp->obj;
		struct atbmwifi_common	*hw_priv	= self->core;

		for(i = 0; i < desc_cmp->frame_cnt; i++){
			if(desc_cmp->data[i] != ATBM_NULL){
				struct wsm_tx *tx = (struct wsm_tx *)desc_cmp->data[i];
				atbm_uint8 queue_id;
				struct atbmwifi_queue *queue=ATBM_NULL;

				if(!(tx->htTxParameters & atbm_cpu_to_le32(WSM_HT_TX_NEED_CONFIRM))){
					queue_id = atbmwifi_queue_get_queue_id(tx->packetID);
					queue = &hw_priv->tx_queue[queue_id];
					if(queue_id>=4){
						frame_hexdump("Error",(atbm_uint8*)tx,sizeof(*tx));
					}
					ATBM_BUG_ON(queue_id>=4);
					ret=atbmwifi_queue_remove(queue,tx->packetID);
					if(ret){
						wifi_printk(WIFI_DBG_ERROR,">>>>RET=%x\n",ret);
					}
				}
			}
		}
		ATBM_TX_DESC_PUT(self,self->drvobj,desc_cmp->desc_id);
		desc_cmp->link = 0;
		atbm_spin_lock_irqsave(&hw_priv->tx_com_lock,&flags);
	}
	atbm_spin_unlock_irqrestore(&hw_priv->tx_com_lock,flags);
	atbm_bh_schedule_tx(hw_priv);
}

atbm_void atbm_hmac_bh(atbm_void *arg, int wake_state)
{
	struct atbmwifi_common *hw_priv = (struct atbmwifi_common *)arg;
	int rx, tx,urb_compl;

	rx = atbm_atomic_xchg(&hw_priv->bh_rx, 0);
	tx = atbm_atomic_xchg(&hw_priv->bh_tx, 0);
#if (ATBM_TX_CMPL_TASK == 0)
	urb_compl = atbm_atomic_xchg(&hw_priv->urb_comp, 0);
#endif
	
	if ( hw_priv->bh_term || hw_priv->bh_error){
		wifi_printk(WIFI_DBG_ERROR,"%s BH thread break %d %d\n",__FUNCTION__,hw_priv->bh_term,hw_priv->bh_error);
		return;
	}
#if (ATBM_TX_CMPL_TASK == 0)
	if(urb_compl){
		//wifi_printk(WIFI_ALWAYS," urb_compl ----->.\n");
		atbm_hmac_coml(hw_priv);
	}
#endif
	if (rx){
		atbm_rx_task(hw_priv);	
	}
	if (tx){
		atbm_tx_task(hw_priv);
	}
#if (ATBM_TX_WRITE_TASK == 0)
	atbm_sdio_tx_queue_process(hw_priv,0,ATBM_TRUE);
#endif //#if (ATBM_TX_WRITE_TASK)
	dump_skb_status();
	dump_hif_status();
}

#if ATBM_TX_CMPL_TASK
atbm_void atbm_hmac_cmpl_bh(void *arg)
{
	struct atbmwifi_common *hw_priv = (struct atbmwifi_common *)arg;
	int urb_compl;

	urb_compl = atbm_atomic_xchg(&hw_priv->urb_comp, 0);

	if(urb_compl){
		//wifi_printk(WIFI_ALWAYS," urb_compl ----->.\n");
		atbm_hmac_coml(hw_priv);
	}
 
	wifi_printk(WIFI_BH,"atbm_hmac_tx_bh while--\n");
}
#endif

static atbm_void atbm_hmac_xmit_data_complete(ATBM_INF_DESC *atbm_desc)
{
	struct sbus_desc *desc_cmp=(struct sbus_desc*)(atbm_desc->context);
	struct sbus_priv *self = desc_cmp->obj;
	struct atbmwifi_common	*hw_priv = self->core;
	unsigned long flags;
	int ret = 0;
	int i;

	switch(atbm_desc->status){
		case 0:
			break;
		default:
			wifi_printk(WIFI_ALWAYS,"WARNING> status %d\n",atbm_desc->status);
			break;
	}

#if CONFIG_WIFI_BT_COMB
	for(i = 0; i < desc_cmp->frame_cnt; i++){
		if(atbm_unlikely(desc_cmp->data[i] == ATBM_NULL && hw_priv->wsm_cmd.cnf == 0)){
			hw_priv->wsm_cmd.ret = 0;
			hw_priv->wsm_cmd.done = 1;
			hw_priv->wsm_cmd.cmd = 0xFFFF;
			atbm_os_wakeup_event(&hw_priv->wsm_cmd_wq);
		}
	}
#endif

#if ATBM_DIRECT_TX
	for(i = 0; i < desc_cmp->frame_cnt; i++){
		if(desc_cmp->data[i] != ATBM_NULL){
			struct wsm_tx *tx = (struct wsm_tx *)desc_cmp->data[i];
			atbm_uint8 queue_id;
			struct atbmwifi_queue *queue=ATBM_NULL;
			if(!(tx->htTxParameters & atbm_cpu_to_le32(WSM_HT_TX_NEED_CONFIRM))){
				queue_id = atbmwifi_queue_get_queue_id(tx->packetID);
				queue = &hw_priv->tx_queue[queue_id];
				ATBM_BUG_ON(queue_id>=4);
				if(atbmwifi_queue_remove(queue,tx->packetID)){
					wifi_printk(WIFI_DBG_ERROR,">>>>error\n");
				}
			}
		}
	}
	ATBM_TX_DESC_PUT(self,self->drvobj,desc_cmp->desc_id);
	desc_cmp->link = 0;
	atbm_bh_schedule_tx(hw_priv);
#else
	/*Add tx urb to list tail*/
	atbm_spin_lock_irqsave(&hw_priv->tx_com_lock,&flags);
	atbm_list_add_tail(&desc_cmp->list,&hw_priv->tx_urb_cmp);
	atbm_spin_unlock_irqrestore(&hw_priv->tx_com_lock,flags);

	atbm_atomic_add_return(1, &hw_priv->urb_comp);
#if ATBM_TX_CMPL_TASK
	atbm_wakeupThreadInternal(hw_priv->cmpl_bh_thread);
#else
	atbm_wakeupThreadInternal(hw_priv->bh_thread);
#endif
#endif
	return ;
}

void atbm_hmac_free_err_cmd(struct sbus_priv *self)
{
	struct atbmwifi_common	*hw_priv = self->core;

	atbm_spin_lock_bh(&hw_priv->wsm_cmd.lock);
	hw_priv->wsm_cmd.ret = -1;
	hw_priv->wsm_cmd.done = 1;
	hw_priv->wsm_cmd.cmd = 0xFFFF;
	atbm_spin_unlock_bh(&hw_priv->wsm_cmd.lock);
	wifi_printk(WIFI_DBG_ERROR, "%s:release wsm_cmd.lock\n",__func__);
	atbm_os_wakeup_event(&hw_priv->wsm_cmd_wq);		
}


void atbm_hmac_free_err_data(struct sbus_priv *self,struct sbus_desc *tx_desc)
{
	struct atbmwifi_common	*hw_priv = self->core;
	struct wsm_tx *wsm = (struct wsm_tx *)tx_desc->data;	
	struct atbmwifi_queue *queue;
	atbm_uint8 queue_id;
	struct atbm_buff *skb;
	const struct atbmwifi_txpriv *txpriv;
	
	wifi_printk(WIFI_DBG_ERROR, "%s:release tx pakage\n",__func__);
	ATBM_BUG_ON(wsm == NULL);
	queue_id = atbmwifi_queue_get_queue_id(wsm->packetID);

	ATBM_BUG_ON(queue_id >= 4);
	queue = &hw_priv->tx_queue[queue_id];
	ATBM_BUG_ON(queue == NULL);

	if(!ATBM_WARN_ON(atbmwifi_queue_get_skb(queue, wsm->packetID, &skb, &txpriv))) {
		struct atbmwifi_ieee80211_tx_info *tx = ATBM_IEEE80211_SKB_TXCB(skb);
		tx->flags |= ATBM_IEEE80211_TX_STAT_ACK;
		atbmwifi_queue_remove(queue, wsm->packetID);
	}
}

static void atbm_build_wsm_header_v1(struct atbmwifi_common *hw_priv, struct wsm_hdr_tx *wsm_tx, int *tx_len){
	wsm_tx->u.common.mark = atbm_cpu_to_le16(0xe569);
	*tx_len = ATBM_ALIGN(*tx_len, 4);
#if ATBM_QUEUE_8023_FRAMES
	wsm_tx->enc_hdr.EncFlags &= ~atbm_cpu_to_le32(0xff << 24);	
#endif
	wsm_tx->enc_hdr.EncFlags |= atbm_cpu_to_le32(atbm_checksum_u8((atbm_uint8*)wsm_tx, 19) << 24);	
}

#if CONFIG_INF_AGGR_TX
static void atbm_init_tx_aggr_buf(struct sbus_desc *tx_desc, int size, int len){
	int i;
	
	for(i = 0; i < size; i++){
		tx_desc[i].pallocated_buf = atbm_kmalloc(len, GFP_KERNEL);
		ATBM_BUG_ON(!tx_desc[i].pallocated_buf);
		tx_desc[i].pallocated_buf_len = len;
	}
}

static void atbm_free_tx_aggr_buf(struct sbus_desc *tx_desc){
	if(!ATBM_WARN_ON(tx_desc[0].pallocated_buf == ATBM_NULL))
		atbm_kfree(tx_desc[0].pallocated_buf);
}

int atbm_hmac_free_tx_wsm(struct sbus_priv *self,struct sbus_desc *tx_desc)
{
	struct wsm_tx *wsm = NULL;

	wsm = tx_desc->data[0]; 
	if((wsm) && (!(wsm->htTxParameters&atbm_cpu_to_le32(WSM_HT_TX_NEED_CONFIRM)))){
		
		struct atbmwifi_queue *queue;
		atbm_uint8 queue_id;
		struct atbmwifi_common	*hw_priv = self->core;
		struct atbm_buff *skb;
		const struct atbmwifi_txpriv *txpriv;

		queue_id = atbmwifi_queue_get_queue_id(wsm->packetID);

		ATBM_BUG_ON(queue_id >= 4);

		queue = &hw_priv->tx_queue[queue_id];
		ATBM_BUG_ON(queue == NULL);

		if(!ATBM_WARN_ON(atbmwifi_queue_get_skb(queue, wsm->packetID, &skb, &txpriv))) {	
			struct atbmwifi_ieee80211_tx_info *tx = ATBM_IEEE80211_SKB_TXCB(skb);
			atbmwifi_queue_remove(queue, wsm->packetID);
		}

		tx_desc->data[0] = ATBM_NULL;
		return 1;
	}
	return 0;
}

int atbm_hmac_xmit_data(struct sbus_priv *self)
{
	unsigned int pipe;
	int status=0;
	int tx_burst=0;
	int vif_selected;
	struct wsm_hdr_tx *wsm;
	struct atbmwifi_common *hw_priv=self->core;
	atbm_uint8 *data =ATBM_NULL;
	int tx_len=0;
	int ret = 0;
	int desc_id =-1;	
	struct sbus_desc *tx_desc = ATBM_NULL;
	unsigned long flags;
	atbm_uint8 *txdmabuff = ATBM_NULL;
	atbm_uint8 *usb_aggr_buff = ATBM_NULL;

	//If Usb disconnecting, stop tx usb packet to host
	if(self->suspend){
		return -2;
	}

	wifi_printk(WIFI_IF, "atbm_hmac_xmit_data++\n");
	atbm_spin_lock_irqsave(&hw_priv->tx_com_lock, &flags);
	desc_id = ATBM_TX_DESC_GET(self,self->drvobj,TX_DESC_NUM);
	if(desc_id<0){
		wifi_printk(WIFI_DBG_MSG, "atbm_usb_xmit_data:desc_id<0\n");
		status=-4;
		goto error;
	}

	tx_desc = &self->tx_desc[desc_id];
	tx_desc->pallocated_buf_len = 0;
	tx_desc->frame_cnt = 0;

	if(tx_desc->pallocated_buf == NULL)
	{
		ATBM_TX_DESC_RESTORE(self,self->drvobj,desc_id);
		status=-7;
		goto error;
	}
	usb_aggr_buff = tx_desc->pallocated_buf;
	do{
		ret = wsm_get_tx(hw_priv, &data, &tx_len, &tx_burst,
					&vif_selected);
		if (ret <= 0) {
			  wifi_printk(WIFI_IF,"tx:atbm_usb_urb_put ATBM_NULL %d\n",desc_id);
			  break;
		}

		atbm_memcpy(usb_aggr_buff, data, tx_len);
		wsm = (struct wsm_hdr_tx *)usb_aggr_buff;
		tx_desc->data[tx_desc->frame_cnt] = data;

		current_ops->build_hdr(hw_priv, wsm, &tx_len);

		add_tx_skb_cnt();

		wsm->id &= atbm_cpu_to_le32(~WSM_TX_SEQ(WSM_TX_SEQ_MAX));
		wsm->id |= atbm_cpu_to_le32(WSM_TX_SEQ(hw_priv->wsm_tx_seq));
		self->tx_vif_selected = vif_selected;

		hw_priv->wsm_tx_seq++;
		
		tx_desc->pallocated_buf_len += ATBM_ALIGN(tx_len, current_ops->align_size);
		/*
		*if the data is cmd ,keep it last in the aggr buff
		*/
		wifi_printk(WIFI_IF, "wsm->len:%d:wsm->id %d seq %d desc_id %d\n",wsm->len, wsm->id,hw_priv->wsm_tx_seq,desc_id);
		if(wsm_txed(hw_priv, data)==0){
			struct wsm_tx *tx = (struct wsm_tx *)data;
			hw_priv->wsm_txframe_num++;
			if(tx->htTxParameters & atbm_cpu_to_le32(WSM_HT_TX_NEED_CONFIRM)){
				tx_desc->data[tx_desc->frame_cnt] = ATBM_NULL;
			}
		}
		else {
			tx_desc->data[tx_desc->frame_cnt] = ATBM_NULL;
		}

		tx_desc->frame_cnt++;
		if(tx_desc->frame_cnt >= INF_BUF_SG_NUM){
			break;
		}
		usb_aggr_buff += ATBM_ALIGN(tx_len, current_ops->align_size);
	}while(1);

	if(tx_desc->pallocated_buf_len == 0){
		ATBM_TX_DESC_RESTORE(self,self->drvobj,desc_id);
		goto error;
	}

	if(tx_desc->frame_cnt == 0){
		ATBM_WARN_ON(1);
	}

	add_tx_pkt_cnt();
	ATBM_TX_DESC_FILL(0,tx_desc->test_desc,tx_desc->pallocated_buf,tx_desc->pallocated_buf_len,
				self->drvobj,atbm_hmac_xmit_data_complete,tx_desc);
	tx_desc->link = 1;
	status = ATBM_TX_SUBMIT_DESC(tx_desc->test_desc, GFP_ATOMIC);

	if (status) {
		atbm_uint8 i = 0;
		int wsm_id;		
		struct wsm_tx *wsm_txd = NULL;				

		tx_desc->test_desc->status = 0;
		for(i = 0;i<tx_desc->frame_cnt;i++){			
			wsm_txd = (struct wsm_tx *)tx_desc->data[i];
			if(!wsm_txd){
				continue;
			}			
			wsm_id = atbm_le16_to_cpu(wsm_txd->hdr.id) & 0x3F;			
			wifi_printk(WIFI_DBG_ERROR, "%s:wsm_id(%x)\n",__func__,wsm_id);			
			hw_priv->wsm_tx_seq++;			
//			if(wsm_id == WSM_FIRMWARE_CHECK_ID){
//				continue;			
//			}			
			if(wsm_id == WSM_TRANSMIT_REQ_MSG_ID){
				if(!(wsm_txd->htTxParameters&atbm_cpu_to_le32(WSM_HT_TX_NEED_CONFIRM))){
					continue;
				}							
				atbm_hmac_free_err_data(self,tx_desc);			
			}else {				
				atbm_hmac_free_err_cmd(self);			
			}						
		}		
		ATBM_TX_DESC_RESTORE(self,self->drvobj,desc_id);			
		wifi_printk(WIFI_ALWAYS, "release all data finished\n");
		status = -1;
		goto error;
	}

	if(status==0){
		status = tx_burst;
	}
error:
	atbm_spin_unlock_irqrestore(&hw_priv->tx_com_lock, flags);
	//atbm_atomic_set(&self->tx_lock, 0);
	return status;
}
#else
int atbm_hmac_xmit_data(struct sbus_priv *self)
{
	unsigned int pipe;
	int status = 0;
	int tx_burst = 0;
	int vif_selected;
	struct wsm_hdr_tx *wsm;
	struct atbmwifi_common *hw_priv=self->core;
	atbm_void *txdata =ATBM_NULL;
	atbm_uint8 *data =ATBM_NULL;
	int tx_len=0;
	int ret = 0;
	int desc_id =-1;
	int i;
	struct sbus_desc *tx_desc = ATBM_NULL;
	unsigned long flags;
	//If Usb disconnecting, stop tx usb packet to host
	if(self->suspend){
		return -2;
	}
	wifi_printk(WIFI_IF, "atbm_usb_xmit_data++\n");
	atbm_spin_lock_irqsave(&hw_priv->tx_com_lock,&flags);
	desc_id = ATBM_TX_DESC_GET(self,self->drvobj,TX_DESC_NUM);
	if(desc_id<0){
		wifi_printk(WIFI_DBG_MSG, "atbm_usb_xmit_data:desc_id<0\n");
		status=-4;
		goto error;
	}

	for(i = 0; i < INF_BUF_SG_NUM; i++){
		ret = wsm_get_tx(hw_priv, &data, &tx_len, &tx_burst,
					&vif_selected);
		if (ret <= 0) {
			  wifi_printk(WIFI_IF,"tx:atbm_usb_urb_put ATBM_NULL %d\n",desc_id);
			  break;
		} else {
			tx_desc = &self->tx_desc[desc_id];
			wsm = (struct wsm_hdr_tx *)data;

			tx_desc->data[i] = data;
			current_ops->build_hdr(hw_priv, wsm, &tx_len);

			add_tx_skb_cnt();
			self->tx_vif_selected =vif_selected;
			wsm->id &= atbm_cpu_to_le32(~WSM_TX_SEQ(WSM_TX_SEQ_MAX));
			wsm->id |= atbm_cpu_to_le32(WSM_TX_SEQ(hw_priv->wsm_tx_seq));
			txdata =(atbm_void*)wsm;

			//wifi_printk(WIFI_ALWAYS, "wsm->id:wsm->id %d seq %d desc_id %d\n",wsm->id,hw_priv->wsm_tx_seq,desc_id);
			ATBM_TX_DESC_FILL(i, tx_desc->test_desc,txdata,tx_len,self->drvobj,atbm_hmac_xmit_data_complete,tx_desc);
			if(wsm_txed(hw_priv, data)==0){
	            struct wsm_tx *tx = (struct wsm_tx *)data;
				hw_priv->wsm_txframe_num++;
				//add by wp, 
				//this is to fix bug , because in sometime atbm_urb_coml is call later than txconfirm
				//when need txconfirm frame , txconfirm will free skb, but atbm_urb_coml need skb
	            if(tx->htTxParameters & atbm_cpu_to_le32(WSM_HT_TX_NEED_CONFIRM)){
	                tx_desc->data[i] = ATBM_NULL;
	            }
			}else {
				tx_desc->data[i] = ATBM_NULL;
				i += 1;
				break;
			}
		}
	}

	if(i > 0){
		tx_desc->frame_cnt = i;
		tx_desc->link =1;
		add_tx_pkt_cnt();
		status = ATBM_TX_SUBMIT_DESC(tx_desc->test_desc, GFP_ATOMIC);
		if (status) {
			status = -5;
			if (vif_selected != -1) {
				atbm_hmac_free_err_data(self,tx_desc);
			}else {
				atbm_hmac_free_err_cmd(self);
			}
			ATBM_TX_DESC_RESTORE(self,self->drvobj,desc_id);
			wifi_printk(WIFI_DBG_ANY," <ERROR>tx:atbm_usb_urb_put %d\n",desc_id);
			//msleep(1000);
			goto error;
		}
		self->tx_seqnum++;
		wifi_printk(WIFI_IF, "tx_seq %d\n",self->tx_seqnum);
		hw_priv->wsm_tx_seq++;
	}else{
		ATBM_TX_DESC_RESTORE(self,self->drvobj,desc_id);
		status = -3;
	}
error:
	atbm_spin_unlock_irqrestore(&hw_priv->tx_com_lock,flags);
	//atbm_atomic_set(&self->tx_lock, 0);
	return status;
}
#endif

atbm_void  atbm_hmac_receive_data_complete( ATBM_INF_DESC *atbm_desc)
{
	struct sbus_desc *rx_desc = (struct sbus_desc*)atbm_desc->context;
	struct sbus_priv *self = rx_desc->obj;
	struct atbm_buff *skb = rx_desc->test_skb;
	struct atbmwifi_common *hw_priv = self->core;
	int RecvLength = atbm_desc->actual_length;
	struct wsm_hdr *wsm;
	unsigned long flags;

	wifi_printk(WIFI_IF, "rxendd  Len %d desc_id %d\n",RecvLength,rx_desc->desc_id);

	if(!hw_priv)
		goto __free;

	rx_desc->link =0;

	switch(atbm_desc->status){
		case 0:
			break;
		default:
			wifi_printk(WIFI_DBG_ANY, "atbm_usb_rx_complete2 error status=%d len %d\n",atbm_desc->status,RecvLength);
			goto __free;
	}
#if USB_SUSPEND_SUPPORT
	if((self->drvobj->suspend_skb_len != 0)&&(self->drvobj->suspend_skb != ATBM_NULL))
	{
		if(atbm_skb_tailroom(self->drvobj->suspend_skb)<self->drvobj->suspend_skb_len+RecvLength)
		{
			struct atbm_buff * long_suspend_skb = ATBM_NULL;
			ATBM_BUG_ON(self->drvobj->suspend_skb_len+RecvLength>RX_BUFFER_SIZE);
			long_suspend_skb = atbm_dev_alloc_skb(RX_BUFFER_SIZE+64);
			ATBM_BUG_ON(!long_suspend_skb);
			atbm_skb_reserve(long_suspend_skb, 64);
			atbm_memcpy((atbm_uint8 *)ATBM_OS_SKB_DATA(long_suspend_skb),ATBM_OS_SKB_DATA(self->drvobj->suspend_skb),self->drvobj->suspend_skb_len);
			atbm_dev_kfree_skb(self->drvobj->suspend_skb);
			self->drvobj->suspend_skb = long_suspend_skb;
		}
		atbm_memcpy((atbm_uint8 *) ATBM_OS_SKB_DATA(self->drvobj->suspend_skb) + self->drvobj->suspend_skb_len,ATBM_OS_SKB_DATA(skb),RecvLength);
		RecvLength += self->drvobj->suspend_skb_len;
		atbm_memcpy(ATBM_OS_SKB_DATA(skb),(atbm_uint8 *)ATBM_OS_SKB_DATA(self->drvobj->suspend_skb),RecvLength);
		self->drvobj->suspend_skb_len = 0;					
	}
	
	wsm = (struct wsm_hdr *)ATBM_OS_SKB_DATA(skb);
	wifi_printk(WIFI_DBG_MSG,"atbm: atbm_usb_receive_data_complete()--len=%d, id=%d\n",wsm->len, wsm->id);
	if (wsm->len != RecvLength){
		if(((wsm->len  % 512)==0) && ((wsm->len+1) == RecvLength)){
			//this correct , lmac output len = (wsm len +1 ) ,inorder to let hmac usb callback
		}
		else {
			wifi_printk(WIFI_IF,"rx rebulid usbsuspend  id %d wsm->len %d,RecvLength %d\n",wsm->id,wsm->len,RecvLength);

			if(self->drvobj->suspend_skb_len== 0){ 
				if(wsm->len > RX_BUFFER_SIZE){
					wifi_printk(WIFI_DBG_ERROR," %s %d id %d wsm->len %d,RecvLength %d\n",__FUNCTION__,__LINE__,wsm->id,wsm->len,RecvLength);
					goto resubmit;
				}
				wifi_printk(WIFI_DBG_ERROR, "rx rebulid usbsuspend0	\n");
				/*
				* alloc 4K buff for suspend_skb is save
				*/
				ATBM_BUG_ON(self->drvobj->suspend_skb == ATBM_NULL);
				atbm_memcpy((atbm_uint8 *)ATBM_OS_SKB_DATA(self->drvobj->suspend_skb),ATBM_OS_SKB_DATA(skb),RecvLength);
				self->drvobj->suspend_skb_len = RecvLength;
				goto resubmit;
			}
		}
	}
	
	ATBM_WARN_ON_FUNC(self->drvobj->suspend_skb_len != 0);
#endif  //USB_SUSPEND_SUPPORT

	if (ATBM_WARN_ON(4 > RecvLength)){
		wsm = (struct wsm_hdr *)ATBM_OS_SKB_DATA(skb);
		wifi_printk(WIFI_DBG_ERROR,"%s %d id %d wsm->len %d,RecvLength %d\n",__FUNCTION__,__LINE__,wsm->id,wsm->len,RecvLength);
		//frame_hexdump("atbm_usb_receive_data_complete",(atbm_uint8 *)wsm,32);
		goto resubmit;
	}

	ATBM_BUG_ON(RecvLength > RX_BUFFER_SIZE);
	self->rx_seqnum++;

	add_rx_pkt_cnt();
	rx_desc->test_skb = ATBM_NULL;
	ATBM_OS_SKB_LEN(skb) = RecvLength;
	//atbm_spin_lock_irqsave(&self->lock, &flags);
#if ATBM_IMMD_RX
	{
		current_ops->process_rx(hw_priv, skb);
		ATBM_RX_DESC_PUT(self,self->drvobj,rx_desc->desc_id);
		return;
	}
#else
	
	ATBM_RX_DESC_PUT(self,self->drvobj,rx_desc->desc_id);
	atbm_skb_queue_tail(&hw_priv->rx_frame_queue, skb);

	atbm_atomic_set(&hw_priv->bh_rx, 1);
	wifi_printk(WIFI_BH,"atbm_bh_wakeup rxend\n");
	atbm_wakeupThreadInternal(hw_priv->bh_thread);

	wifi_printk(WIFI_DBG_MSG,"atbm: atbm_usb_receive_data_complete() 5.\n");

	if(!hw_priv->init_done){
		wifi_printk(WIFI_DBG_ERROR, "[BH] irq. init_done =0 drop\n");
		goto __free;
	}
	if (/* ATBM_WARN_ON */(hw_priv->bh_error))
		goto __free;

	wifi_printk(WIFI_DBG_MSG,"atbm: atbm_usb_receive_data_complete() 6.\n");
	return;
#endif
__free:
	if(self->drvobj->suspend_skb_len != 0){
		wifi_printk(WIFI_DBG_ERROR,"rx rebulid usbsuspend3	rx drop\n");
	}
	//atbm_usb_urb_put(self,self->drvobj->rx_urb_map,rx_urb->desc_id);
	wifi_printk(WIFI_DBG_ERROR, "[WARNING] atbm_hmac_receive_data drop\n");

	return;

resubmit:
	if(!hw_priv->init_done){
		wifi_printk(WIFI_DBG_ERROR, "[BH] irq. init_done =0 drop\n");
		goto __free;
	}
	if (/* ATBM_WARN_ON */(hw_priv->bh_error))
		goto __free;

	ATBM_RX_DESC_PUT(self,self->drvobj,rx_desc->desc_id);
	atbm_hmac_receive_data(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
	wifi_printk(WIFI_DBG_ANY, "atbm_hmac_receive_data resubmit\n");
	return;
}

int atbm_hmac_receive_data(struct sbus_priv *self,unsigned int addr,atbm_void  *dst, int count)
{
	unsigned int pipe;
	int status = 0;
	struct sbus_desc *rx_desc;
	int desc_id;
	struct atbmwifi_common *hw_priv=self->core;

	//If Usb disconnecting, stop rx usb packet from host
	if(self->suspend){
		return -2;
	}
	desc_id = ATBM_RX_DESC_GET(self, self->drvobj, RX_DESC_NUM);
	if(desc_id<0){
		status=-4;
		goto __err_rx;
	}
	rx_desc = &self->rx_desc[desc_id];

	ATBM_RX_DESC_FILL(rx_desc->test_desc,rx_desc->test_skb,count,self->drvobj,atbm_hmac_receive_data_complete,rx_desc);
	rx_desc->link = 1;
	status = ATBM_RX_SUBMIT_DESC(rx_desc->test_desc, 0);
	wifi_printk(WIFI_IF, "ATBM_RX_SUBMIT_DESC rx desc_id %d\n",desc_id);
	if (status) {
		status = -2;
		wifi_printk(WIFI_DBG_ERROR,"receive_data ATBM_RX_SUBMIT_DESC ++ ERR %d\n",status);
		goto __err_skb;
	}
__err_skb:
	if(status < 0){
		ATBM_RX_DESC_PUT(self,self->drvobj,desc_id);
	}
__err_rx:
	return status;
}

int atbm_register_bh(struct atbmwifi_common *hw_priv)
{
	atbm_thread_internal_t *bh_thread;
#if ATBM_TX_CMPL_TASK
	atbm_thread_internal_t *cmpl_bh_thread;
#endif
	int status =0;

	current_ops = &atbm_ops[hw_priv->chip_priv->hif_version];
	hw_priv->chip_priv->hdr_rollback = current_ops->hdr_rollback;
	atbm_atomic_set(&hw_priv->bh_rx, 0);
	atbm_atomic_set(&hw_priv->bh_tx, 0);
	atbm_os_init_waitevent(&hw_priv->wsm_cmd_wq);
	hw_priv->bh_term=0;
	hw_priv->bh_error=0;
	wifi_printk(WIFI_DBG_ERROR,"atbm_register_bh\n");
#if CONFIG_INF_AGGR_TX
	atbm_init_tx_aggr_buf(hw_priv->sbus_priv->tx_desc,TX_DESC_NUM, BUFF_ALLOC_LEN);
#endif

#if ATBM_SDIO_BUS
	atbm_sdio_queue_init(hw_priv);
#endif
#if ATBM_TX_CMPL_TASK
	/*Create RxData Task*/
	cmpl_bh_thread=atbm_createThreadInternal("atbm_bh_cmpl",
									(thread_callback)atbm_hmac_cmpl_bh,
									(atbm_void*)hw_priv,
									CMPL_BH_TASK_PRIO,
									10*ATBM_HZ);
	if (!cmpl_bh_thread){
		wifi_printk(WIFI_DBG_ERROR,"cmpl_bh_thread Failed\n");
		return -1;
	}
	hw_priv->cmpl_bh_thread = cmpl_bh_thread;
#endif

	/*Create RxData Task*/
	bh_thread=atbm_createThreadInternal("atbm_bh",
									(thread_callback)atbm_hmac_bh,
									(atbm_void*)hw_priv,
									BH_TASK_PRIO,
									100*ATBM_HZ);
	if (!bh_thread){
		wifi_printk(WIFI_DBG_ERROR,"bh_thread Failed\n");
		return -1;
	}
	hw_priv->bh_thread = bh_thread;

	return status;
}

atbm_void atbm_unregister_bh(struct atbmwifi_common *hw_priv)
{
	//kill createThread
	hw_priv->bh_term=1;
	atbm_stopThreadInternal(hw_priv->bh_thread);

	atbm_os_delete_waitevent(&hw_priv->wsm_cmd_wq);
	atbm_os_DeleteMutex(&hw_priv->wsm_cmd_mux);
#if ATBM_SDIO_BUS
	atbm_sdio_queue_exit(hw_priv);
#endif
#if CONFIG_INF_AGGR_TX
	atbm_free_tx_aggr_buf(hw_priv->sbus_priv->tx_desc);
#endif
#if ATBM_TX_CMPL_TASK
	atbm_stopThreadInternal(hw_priv->cmpl_bh_thread);
#endif
}


