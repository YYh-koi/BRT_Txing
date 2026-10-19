/**************************************************************************************************************
 * altobeam RTOS
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#ifndef ATBM_USB_H
#define ATBM_USB_H
#include "atbm_os_usb.h"
#include "atbm_os_atomic.h"
#include "atbm_os_spinlock.h"

#if CONFIG_INF_AGGR_TX
#define PER_PACKET_LEN 2048 //must pow 512
#define URB_AGGR_NUM 8
#define PER_BUFF_AGGR_NUM 16
#define TX_URB_NUM 4
#define RX_URB_NUM 4
#define BUFF_ALLOC_LEN (PER_BUFF_AGGR_NUM*PER_PACKET_LEN)
#define TX_BUFFER_SIZE PER_PACKET_LEN
//#define MAX_TX_PACKET_LEN (4*PER_PACKET_LEN)
#define MAX_TX_PACKET_LEN PER_PACKET_LEN
#define MAX_AGGR_PACKET_NUM 10
#define ATBM_PACKET_ALIGN_SIZE_V0 PER_PACKET_LEN
#else //CONFIG_USB_AGGR_URB_TX
#define TX_URB_NUM 4 //must be 4.can't change
#define RX_URB_NUM 4 //must <= 4
#define TX_BUFFER_SIZE 1680
#define ATBM_PACKET_ALIGN_SIZE_V0 0
#endif //CONFIG_USB_AGGR_URB_TX

#define ATBM_PACKET_ALIGN_SIZE_V1 4
#define ATBM_USB_EP0_MAX_SIZE 1024
#define ATBM_USB_EP1_MAX_RX_SIZE 512
#define ATBM_USB_EP2_MAX_TX_SIZE 512
#define ATBM_USB_VENQT_WRITE  0x40
#define ATBM_USB_VENQT_READ 0xc0

#define RX_BUFFER_SIZE (16*1024) //must biger than amsdu

#define TX_DESC_NUM TX_URB_NUM
#define RX_DESC_NUM RX_URB_NUM
#if ATBM_SUPPORT_SG
#define INF_BUF_SG_NUM 8
#else
#define INF_BUF_SG_NUM 1
#endif
#define ATBM_HDR_ROLLBACK ATBM_TRUE
#define DOWNLOAD_BLOCK_SIZE_V0 64
#define DOWNLOAD_BLOCK_SIZE_V1 1024
typedef atbm_urb_s ATBM_INF_DESC;

/*usb vendor define type, EP0, bRequest*/
enum {
	VENDOR_HW_READ=0,
	VENDOR_HW_WRITE=1,
	VENDOR_HW_RESVER=2,
	VENDOR_SW_CPU_JUMP=3,/*cpu jump to real lmac code,after fw download*/
	VENDOR_SW_READ=4,
	VENDOR_SW_WRITE=5,	
	VENDOR_HW_RESET =6,
	VENDOR_EP0_CMD=7,
};

struct dvobj_priv{
	struct atbm_usb_device *pusbdev;
	struct atbm_usb_interface *pusbintf;
#ifdef CONFIG_USB_AGGR_URB_TX
	char * tx_dma_addr_buffer;
	atbm_uint32 tx_dma_addr_buffer_len;
	struct sbus_urb *tx_save_urb;
	int tx_save_urb_data_len;
	char * NextAllocPost;
	char * NextFreePost;
	char * tx_dma_addr_buffer_end;
	ATBM_BOOL tx_dma_addr_buffer_full;
	int  free_dma_buffer_cnt;
	int  total_dma_buffer_cnt;
#endif  //CONFIG_USB_AGGR_URB_TX
	//unsigned long	 rx_urb_map[BITS_TO_LONGS(RX_URB_NUM)];
	//unsigned long	 tx_urb_map[BITS_TO_LONGS(TX_URB_NUM)];
	atbm_uint32 	rx_urb_map[1];  //current used 8 URBs, 1 byte
	atbm_uint32	 	tx_urb_map[1];
	int tx_test_seq_need; //just fot test
	int tx_test_hwseq_need;//just fot test
	atbm_urb_s *cmd_urb;
	//struct usb_anchor submitted;
	struct sbus_priv *self;
	struct atbm_net_device *netdev;
	atbm_uint8	usb_speed; // 1.1, 2.0 or 3.0
	atbm_uint8	nr_endpoint;
	int ep_in;
	int ep_in_size;
	int ep_out;
	int ep_out_size;
	int	ep_num[6]; //endpoint number
	struct atbm_buff *suspend_skb;
	unsigned long suspend_skb_len;
};

struct sbus_wtd {
	int 	wtd_init;
	struct task_struct		*wtd_thread;
//	atbm_os_wait_queue_head_t		wtd_evt_wq;
	atbm_atomic_t				wtd_term;
	atbm_atomic_t				wtd_run;
	atbm_atomic_t				wtd_probe;
};

enum atbm_bh_pm_state {
	 ATBMWIFI__BH_RESUMED = 0,
	 ATBMWIFI__BH_SUSPEND,
	 ATBMWIFI__BH_SUSPENDED,
	 ATBMWIFI__BH_RESUME,
};

#define ATBM_TX_DESC_GET(self,drvobj,num) atbm_usb_urb_get_irqen(self,drvobj->tx_urb_map,num)
#define ATBM_RX_DESC_GET(self,drvobj,num) atbm_usb_urb_get(self,drvobj->rx_urb_map,num)
#define ATBM_TX_DESC_PUT(self,drvobj,num) atbm_usb_urb_put(self,drvobj->tx_urb_map,num)
#define ATBM_RX_DESC_PUT(self,drvobj,num) atbm_usb_urb_put(self,drvobj->rx_urb_map,num)
#define ATBM_TX_DESC_PUT_IRQEN(self,drvobj,num) atbm_usb_urb_put_irqen(self,drvobj->tx_urb_map,num)
#define ATBM_TX_DESC_RESTORE(self,drvobj,num) ATBM_TX_DESC_PUT_IRQEN(self,drvobj,num)
#define ATBM_RX_DESC_RESTORE(self,drvobj,num) ATBM_RX_DESC_PUT(self,drvobj,num)

#define ATBM_USB_TX_URB_FILL(desc,buff,len,pusbdev,ep,complete,context)do { \
		atbm_uint32 pipe = atbm_usb_sndbulkpipe(pusbdev, ep); \
		atbm_usb_fill_bulk_urb(desc, pusbdev, pipe, buff, len, complete,context); \
	}while(0);

#define ATBM_USB_RX_URB_FILL(desc,buff,len,pusbdev,ep,complete,context)do { \
		atbm_uint32 pipe = atbm_usb_rcvbulkpipe(pusbdev, ep); \
		atbm_usb_fill_bulk_urb(desc, pusbdev, pipe, buff, len, complete,context); \
	}while(0);

#define ATBM_TX_DESC_FILL(id,desc,buff,len,drvobj,complete,context) ATBM_USB_TX_URB_FILL(desc,buff,len,drvobj->pusbdev,drvobj->ep_out,complete,context)
#define ATBM_RX_ALLOC_SKB_EARLY(skb,len) do{ \
				skb = atbm_dev_alloc_skb(len); \
			}while(0);
#define ATBM_RX_DESC_FILL(desc,skb,len,drvobj,complete,context) do { \
				skb = atbm_dev_alloc_skb(len); \
				ATBM_BUG_ON(!skb); \
				ATBM_USB_RX_URB_FILL(desc,ATBM_OS_SKB_DATA(skb),len,drvobj->pusbdev,drvobj->ep_in,complete,context); \
			}while(0);
			

#define ATBM_TX_SUBMIT_DESC(desc,x) atbm_usb_submit_urb(desc,x)
#define ATBM_RX_SUBMIT_DESC(desc,x) atbm_usb_submit_urb(desc,x)
#define SET_USB_EP_NUM(hw_priv) do{ \
		if(hw_priv->chip_priv->chip_cat < COEUS_CATEGORY) { \
			hw_priv->sbus_priv->drvobj->ep_in = 1; \
			hw_priv->sbus_priv->drvobj->ep_out = 2; \
		}else{ \
			hw_priv->sbus_priv->drvobj->ep_in = 5; \
			hw_priv->sbus_priv->drvobj->ep_out = 4; \
		} \
	}while(0);

extern struct sbus_ops atbm_usb_sbus_ops;
void atbm_build_wsm_header_v0(struct atbmwifi_common *hw_priv, struct wsm_hdr_tx *wsm_tx, int *tx_len);
int atbm_usb_pm(struct sbus_priv *self, ATBM_BOOL  auto_suspend);
int atbm_usb_pm_async(struct sbus_priv *self, ATBM_BOOL  auto_suspend);
atbm_void atbm_usb_urb_put(struct sbus_priv *self,atbm_uint32 *bitmap,int id);
int atbm_usb_urb_get(struct sbus_priv *self,atbm_uint32 *bitmap,int max_urb);
int atbm_usb_urb_get_irqen(struct sbus_priv *self,atbm_uint32 *bitmap,int max_urb);
atbm_void  atbm_usb_urb_put_irqen(struct sbus_priv *self,atbm_uint32 *bitmap,int id);
typedef int (*sbus_complete_handler)(atbm_urb_s *atbm_urb);
atbm_void atbm_usb_module_init(atbm_void);
int atbm_usbctrl_vendorreq_sync(struct sbus_priv *self, atbm_uint8 request,atbm_uint8 b_write,
					atbm_uint16 value, atbm_uint16 index, atbm_void *pdata,atbm_uint16 len);
int __atbm_usb_suspend(struct sbus_priv *self);
atbm_void atbm_usb_kill_all_rxurb(struct sbus_priv *self);
int __atbm_usb_resume(struct sbus_priv *self);
atbm_void atbm_urb_coml(struct atbmwifi_common *hw_priv);
atbm_void atbm_usb_module_exit(atbm_void);
static inline int atbm_sdio_tx_queue_process(struct atbmwifi_common *hw_priv, atbm_uint32 timeout, ATBM_BOOL wait){
}

#endif//ATBM_USB_H
