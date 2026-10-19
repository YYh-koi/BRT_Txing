/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#ifndef  ATBMWIFI__SDIO_H_INCLUDED
#define  ATBMWIFI__SDIO_H_INCLUDED
 /* DPLL initial values */
#define DPLL_INIT_VAL_9000		(0x00000191)
#define DPLL_INIT_VAL_CW1200		(0x0EC4F121)
#define ALTOBEAM_WIFI_HDR_FLAG  (0x34353677)

#define DOWNLOAD_ITCM_ADDR		(0x00060000)

#if CONFIG_WIFI_BT_COMB
#define DOWNLOAD_BLE_SRAM_ADDR		(0x09018000)
#define BLE_SRAM_CODE_SIZE			0x10000
#else//#ifdef CONFIG_WIFI_BT_COMB
#define DOWNLOAD_BLE_SRAM_ADDR		(0x0900A000)
#define BLE_SRAM_CODE_SIZE			0x16000
#endif  //#ifdef CONFIG_WIFI_BT_COMB

#define DOWNLOAD_ITCM_SIZE		(160*1024)
#define DOWNLOAD_DTCM_ADDR		(0x00800000)
#define DOWNLOAD_DTCM_SIZE		(48*1024)
#define DOWNLOAD_BLOCK_SIZE		(256)
#define ATBM_ALIGN_SIZE  		64
#define SYS_BASE_ADDR_SILICON		(0)
#define PAC_BASE_ADDRESS_SILICON	(SYS_BASE_ADDR_SILICON + 0x09000000)
#define PAC_SHARED_MEMORY_SILICON	(PAC_BASE_ADDRESS_SILICON)
#define CW12000_APB(addr)		(PAC_SHARED_MEMORY_SILICON + (addr)) 
 /* ***************************************************************
 *Device register definitions
 *************************************************************** */
 /* WBF - SPI Register Addresses */
#define ATBM_HIFREG_ADDR_ID_BASE		(0x0000)
	 /* 16/32 bits */
#define ATBM_HIFREG_CONFIG_REG_ID		(0x0000)
	 /* 16/32 bits */
#define ATBM_HIFREG_CONTROL_REG_ID		(0x0001)
	 /* 16 bits, Q mode W/R */
#define ATBM_HIFREG_IN_OUT_QUEUE_REG_ID	(0x0002)
	 /* 32 bits, AHB bus R/W */
#define ATBM_HIFREG_AHB_DPORT_REG_ID	(0x0003)
	 /* 16/32 bits */
#define ATBM_HIFREG_SRAM_BASE_ADDR_REG_ID   (0x0004)
	 /* 32 bits, APB bus R/W */
#define ATBM_HIFREG_SRAM_DPORT_REG_ID	(0x0005)
	 /* 32 bits, t_settle/general */
#define ATBM_HIFREG_TSET_GEN_R_W_REG_ID	(0x0006)
	 /* 16 bits, Q mode read, no length */
#define ATBM_HIFREG_FRAME_OUT_REG_ID	(0x0007)
	 /*32 bits 15:0 avilable buffer size; 31:16 avilable channel num*/
#define ATBM_6X65S_SDIO_FN1_TX_TOT_LEN	(0x0008)
	 /*32 bits 15:0 rx length*/
#define ATBM_6X65S_SDIO_FN1_RX_TOT_LEN	(0x0009)
	 /*32 bits 15:0 avilable buffer size; 31:16 avilable channel num*/
#define ATBM_6X65S_SDIO_FN2_TX_TOT_LEN	(0x000a)
	 /*32 bits 15:0 rx length*/
#define ATBM_6X65S_SDIO_FN2_RX_TOT_LEN	(0x000b)
	 /*32 bits 15:0 avilable buffer size; 31:16 avilable channel num*/
#define ATBM_6X65S_SDIO_FN3_TX_TOT_LEN	(0x000c)
	 /*32 bits 15:0 rx length*/
#define ATBM_6X65S_SDIO_FN3_RX_TOT_LEN	(0x000d)
	 /*32 bits base address of mem for SDIO fn4 access*/
#define ATBM_6X65S_SDIO_FN4_MEM_BASE_0		(0x000e)
	 /*32 bits base address of mem for SDIO fn4 access*/
#define ATBM_6X65S_SDIO_FN4_MEM_BASE_1		(0x000f)
	 /*32 bits base address of mem for SDIO fn5 access*/
#define ATBM_6X65S_SDIO_FN5_MEM_BASE_2		(0x0010)
	 /*32 bits base address of mem for SDIO fn5 access*/
#define ATBM_6X65S_SDIO_FN5_MEM_BASE_3		(0x0011)
	 /*32 bit reg used for reset chip*/
#define ATBM_6X65S_SDIO_FN1_RESET_REG       (0x0014)

#define ATBM_HIFREG_ADDR_ID_MAX		(ATBM_6X65S_SDIO_FN1_RESET_REG)

	 /* WBF - Control register bit set */
	 /* next o/p length, bit 11 to 0 */
#define ATBM_HIFREG_CONT_NEXT_LEN_MASK	(0xCFFF)
#define ATBM_HIFREG_CONT_NEXT_LEN_LSB_MASK	(0x0FFF)
#define ATBM_HIFREG_CONT_NEXT_LEN_MSB_MASK	(0xC000)
#define ATBM_HIFREG_CONT_WUP_BIT		(ATBM_BIT(12))
#define ATBM_HIFREG_CONT_RDY_BIT		(ATBM_BIT(13))
#define ATBM_HIFREG_CONT_IRQ_ENABLE		(ATBM_BIT(14))
#define ATBM_HIFREG_CONT_RDY_ENABLE		(ATBM_BIT(15))
#define ATBM_HIFREG_CONT_IRQ_RDY_ENABLE	(ATBM_BIT(14)|ATBM_BIT(15)) 
#define ATBM_HIFREG_PS_SYNC_SDIO_FLAG	(ATBM_BIT(23))
#define ATBM_HIFREG_PS_SYNC_SDIO_CLEAN	(ATBM_BIT(24))
/* SPI Config register bit set */
#define ATBM_HIFREG_CONFIG_FRAME_BIT	(ATBM_BIT(2))
#define ATBM_HIFREG_CONFIG_WORD_MODE_BITS	(ATBM_BIT(3)|ATBM_BIT(4))
#define ATBM_HIFREG_CONFIG_WORD_MODE_1	(ATBM_BIT(3))
#define ATBM_HIFREG_CONFIG_WORD_MODE_2	(ATBM_BIT(4))
#define ATBM_HIFREG_CONFIG_ERROR_0_BIT	(ATBM_BIT(5))
#define ATBM_HIFREG_CONFIG_ERROR_1_BIT	(ATBM_BIT(6))
#define ATBM_HIFREG_CONFIG_ERROR_2_BIT	(ATBM_BIT(7))
/* TBD: Sure??? */
#define ATBM_HIFREG_CONFIG_CSN_FRAME_BIT	(ATBM_BIT(7))
#define ATBM_HIFREG_CONFIG_ERROR_3_BIT	(ATBM_BIT(8))
#define ATBM_HIFREG_CONFIG_ERROR_4_BIT	(ATBM_BIT(9))
/* QueueM */
#define ATBM_HIFREG_CONFIG_ACCESS_MODE_BIT	(ATBM_BIT(10))
/* AHB bus */
#define ATBM_HIFREG_CONFIG_AHB_PFETCH_BIT	(ATBM_BIT(11))
#define ATBM_HIFREG_CONFIG_CPU_CLK_DIS_BIT	(ATBM_BIT(12))
/* APB bus */
#define ATBM_HIFREG_CONFIG_PFETCH_BIT	(ATBM_BIT(13))
/* cpu reset */
#define ATBM_HIFREG_CONFIG_CPU_RESET_BIT	(ATBM_BIT(14))
#define ATBM_HIFREG_CONFIG_CLEAR_INT_BIT	(ATBM_BIT(15))
/* For CW1200 the IRQ Enable and Ready Bits are in CONFIG register */
#define ATBM_HIFREG_CONF_IRQ_RDY_ENABLE	(ATBM_BIT(16)|ATBM_BIT(17))
#define ATBM_HIFREG_CONF_RDY_IRQ_ENABLE	(ATBM_BIT(17))
#define ATBM_HIFREG_CONF_DATA_IRQ_ENABLE	(ATBM_BIT(16))	 
#define ATBM_HIFREG_CONFIG_CPU_RESET_BIT_2	(ATBM_BIT(22))

#define ATBM_SDIO_FN1_RESET_WIFI_CPU             (ATBM_BIT(4))
#define ATBM_SDIO_FN1_RESET_MMU_SYNC             (ATBM_BIT(11))
#define ATBM_SDIO_FN1_RESET_WIFI_CHANN           (ATBM_BIT(5))
#define ATBM_SDIO_FN1_RESET_WIFI_MAC_PHY         (ATBM_BIT(6))
#define ATBM_SDIO_FN1_RESET_CIPHER               (ATBM_BIT(7))
#define ATBM_SDIO_FN1_RESET_WIIF_CMD_CHANN       (ATBM_BIT(10))
#define ATBM_SDIO_CHAN_NUM_CONFIG                (0xa900010)
#define ATBM_HW_RESET_WIFI_RECOERY \
			(ATBM_SDIO_FN1_RESET_WIFI_CHANN \
			|ATBM_SDIO_FN1_RESET_WIFI_MAC_PHY \
			|ATBM_SDIO_FN1_RESET_CIPHER \
            |ATBM_SDIO_FN1_RESET_WIIF_CMD_CHANN)


#define WRITE_32K_ADDR_MSK	(0xfffff000)
#define WRITE_32K_ADDR	(0x16101000)

extern struct sbus_ops atbm_sdio_sbus_ops;
#define ATBM_SDIO_CCCR_IENx		0x04
#define ATBM_SDIO_CCCR_IOEx  0x02
#define ATBM_SDIO_CCCR_IORx  0x03
#define ATBM_SDIO_CCCR_ABORT		0x06	/* function abort/card reset */
#define ATBM_SDIO_CCCR_IF		0x07	/* bus interface controls */
#define ATBM_SDIO_CCCR_SPEED		0x13

#define  ATBM_SDIO_SPEED_SHS		0x01	/* Supports High-Speed mode */

#define  ATBM_SDIO_SPEED_BSS_SHIFT	1
#define  ATBM_SDIO_SPEED_SDR25	(1<<ATBM_SDIO_SPEED_BSS_SHIFT)
#define  ATBM_SDIO_SPEED_EHS		ATBM_SDIO_SPEED_SDR25	/* Enable High-Speed */
#define  ATBM_SDIO_BUS_WIDTH_4BIT	0x02
#define ATBM_SDIO_FBR_BLKSIZE   0x10

#define ATBM_SDIO_CCCR_POWER		0x12

#define  SDIO_POWER_SMPC	0x01	/* Supports Master Power Control */
#define  SDIO_POWER_EMPC	0x02	/* Enable Master Power Control */


#define ATBM_SDIO_CCCR_CAPS 0x08

#define  SDIO_CCCR_CAP_SDC	0x01	/* can do CMD52 while data transfer */
#define  SDIO_CCCR_CAP_SMB	0x02	/* can do multi-block xfers (CMD53) */
#define  SDIO_CCCR_CAP_SRW	0x04	/* supports read-wait protocol */
#define  SDIO_CCCR_CAP_SBS	0x08	/* supports suspend/resume */
#define  SDIO_CCCR_CAP_S4MI	0x10	/* interrupt during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_E4MI	0x20	/* enable ints during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_LSC	0x40	/* low speed card */
#define  SDIO_CCCR_CAP_4BLS	0x80	/* 4 bit low speed card */

	
#define ATBM_SDIO_CCCR_CCCR  0x00
#define  SDIO_CCCR_REV_1_00	0	/* CCCR/FBR Version 1.00 */
#define  SDIO_CCCR_REV_1_10	1	/* CCCR/FBR Version 1.10 */
#define  SDIO_CCCR_REV_1_20	2	/* CCCR/FBR Version 1.20 */
	
#define  SDIO_SDIO_REV_1_00	0	/* SDIO Spec Version 1.00 */
#define  SDIO_SDIO_REV_1_10	1	/* SDIO Spec Version 1.10 */
#define  SDIO_SDIO_REV_1_20	2	/* SDIO Spec Version 1.20 */
#define  SDIO_SDIO_REV_2_00	3	/* SDIO Spec Version 2.00 */
#define ATBM_SDIO_CARE_NUM  (1)

#define ATBM_SDIO_BLOCK_SIZE 256
#define DOWNLOAD_BLOCK_SIZE_WR	(0x2000 - 4)
#if CONFIG_INF_AGGR_TX
#define PER_PACKET_LEN ATBM_SDIO_BLOCK_SIZE
#define TX_DESC_NUM 4
#define RX_DESC_NUM 4
#define ATBM_PACKET_ALIGN_SIZE_V0 PER_PACKET_LEN
#define INF_BUF_SG_NUM 8
#define TX_BUFFER_SIZE 2048
#define BUFF_ALLOC_LEN (TX_BUFFER_SIZE*INF_BUF_SG_NUM)
#else //CONFIG_INF_AGGR_TX
/*must be 2 ^ N*/
#define RX_DESC_NUM 8
/*must be 2 ^ N*/
#define TX_DESC_NUM 8
#define TX_BUFFER_SIZE (1680)
#define ATBM_PACKET_ALIGN_SIZE_V0 0
#endif //CONFIG_INF_AGGR_TX


#define ATBM_PACKET_ALIGN_SIZE_V1 4

/* an SPI message cannot be bigger than (2"12-1)*2 bytes
 * "*2" to cvt to bytes */
#define PIGGYBACK_CTRL_REG	(2)
#define EFFECTIVE_BUF_SIZE	(16*1024)
#define RX_BUFFER_SIZE (16*1024) //must biger than amsdu
#define WAKE_UP_HOST_REQ_FLAG ATBM_BIT(15)

#define INF_BUF_SG_NUM 8

#define ATBM_HDR_ROLLBACK ATBM_FALSE
#define SET_USB_EP_NUM(hw_priv)
#define DOWNLOAD_BLOCK_SIZE_V0 508
#define DOWNLOAD_BLOCK_SIZE_V1 ATBM_SDIO_BLOCK_SIZE //must equal


#define ATBM_SDIO_WAKEUP_REASON_RX_PROGRESS 0
#define ATBM_SDIO_WAKEUP_REASON_TX_PROGRESS 1
#define ATBM_SDIO_WAKEUP_REASON_ETF_TEST	2

typedef void (*sdio_complete_t)(void *);

struct sdio_inf_desc{
	int len[INF_BUF_SG_NUM];
	void *buf[INF_BUF_SG_NUM];
	void *data;
	void *context;
	int actual_length;
	int status;
	sdio_complete_t complete;
};

typedef struct sdio_inf_desc ATBM_INF_DESC;
struct dvobj_priv{
	atbm_uint32 tx_desc_num_get;
	atbm_uint32 tx_desc_num_put;
	atbm_uint32 rx_desc_num_get;
	atbm_uint32 rx_desc_num_put;
	int suspend_skb_len;
};

static inline int atbm_sdio_desc_get(atbm_spinlock_t	*lock, atbm_uint32 *desc_get, int desc_put, int total){
	int id = 0;

	atbm_spin_lock(lock);
	if(*desc_get == desc_put){
		atbm_spin_unlock(lock);
		return -1;
	}
	id = *desc_get%total;
	*desc_get = *desc_get + 1;
	atbm_spin_unlock(lock);
	return id;
}

static inline void atbm_sdio_desc_put(atbm_spinlock_t	*lock, atbm_uint32 *desc_put){
	atbm_spin_lock(lock);
	*desc_put = *desc_put + 1;
	atbm_spin_unlock(lock);
}

static inline void atbm_sdio_desc_restore(atbm_spinlock_t	*lock, atbm_uint32 *desc_get){
	atbm_spin_lock(lock);
	*desc_get = *desc_get - 1;
	atbm_spin_unlock(lock);
}


#define ATBM_SDIO_DESC_FILL(id,desc,buff_t,len_t,complete_t,context_t) do { \
			desc->buf[id] = buff_t; \
			desc->len[id] = len_t; \
			desc->context = context_t; \
			desc->complete = (sdio_complete_t)complete_t; \
		}while(0);


#define ATBM_RX_DESC_GET(self,drvobj,num) atbm_sdio_desc_get(&self->lock,&drvobj->rx_desc_num_get,drvobj->rx_desc_num_put,num)
#define ATBM_RX_DESC_PUT(self,drvobj,num) atbm_sdio_desc_put(&self->lock,&drvobj->rx_desc_num_put)
#define ATBM_RX_DESC_RESTORE(self,drvobj,num) atbm_sdio_desc_restore(&self->lock,&drvobj->rx_desc_num_get)
#define ATBM_TX_DESC_GET(self,drvobj,num) atbm_sdio_desc_get(&self->lock,&drvobj->tx_desc_num_get,drvobj->tx_desc_num_put,num)
#define ATBM_TX_DESC_PUT(self,drvobj,num) atbm_sdio_desc_put(&self->lock,&drvobj->tx_desc_num_put)
#define ATBM_TX_DESC_RESTORE(self,drvobj,num) atbm_sdio_desc_restore(&self->lock,&drvobj->tx_desc_num_get)

#define ATBM_RX_ALLOC_SKB_EARLY(skb,len)
#define ATBM_RX_DESC_FILL(desc,skb,len,drvobj,complete,context) ATBM_SDIO_DESC_FILL(0,desc,ATBM_NULL,len,complete,context)
#define ATBM_TX_DESC_FILL(id,desc,buff,len,drvobj,complete,context) ATBM_SDIO_DESC_FILL(id,desc,buff,len,complete,context)

#define ATBM_TX_SUBMIT_DESC(desc,x) atbm_sdio_tx_submit(desc)
#define ATBM_RX_SUBMIT_DESC(desc,x) atbm_sdio_rx_submit(desc)

void atbm_build_wsm_header_v0(struct atbmwifi_common *hw_priv, struct wsm_hdr_tx *wsm_tx, int *tx_len);


int __atbm_irq_enable(struct atbmwifi_common *priv, int enable);
atbm_void atbm_sdio_module_init(atbm_void);
int atbm_sdio_suspend(struct atbmwifi_common *hw_priv);
int atbm_sdio_resume(struct atbmwifi_common *hw_priv);
atbm_void atbm_sdio_module_exit(atbm_void);

void atbm_irq_handler(struct atbmwifi_common *hw_priv);
int atbm_device_wakeup(struct atbmwifi_common *hw_priv);
int atbm_sdio_tx_submit(ATBM_INF_DESC *desc);
int atbm_sdio_rx_submit(ATBM_INF_DESC *desc);
void atbm_sdio_queue_init(struct atbmwifi_common *hw_priv);
void atbm_sdio_queue_exit(struct atbmwifi_common *hw_priv);
int atbm_sdio_tx_queue_process(struct atbmwifi_common *hw_priv, atbm_uint32 timeout, ATBM_BOOL wait);


#if SUPPORT_LIGHT_SLEEP
static inline void atbm_light_sleep_try_to_wakeup(struct atbmwifi_common *hw_priv, int reason){
	_atbm_light_sleep_try_to_wakeup(hw_priv, reason);
}
static inline void atbm_light_sleep_try_to_keepalive(struct atbmwifi_common *hw_priv, int reason){
	_atbm_light_sleep_try_to_keepalive(hw_priv, reason);
}
static inline void atbm_light_sleep_try_to_sleep(struct atbmwifi_common *hw_priv, int reason){
	_atbm_light_sleep_try_to_sleep(hw_priv, reason);
}
#else
static inline void atbm_light_sleep_try_to_wakeup(struct atbmwifi_common *hw_priv, int reason){
}
static inline void atbm_light_sleep_try_to_keepalive(struct atbmwifi_common *hw_priv, int reason){
}
static inline void atbm_light_sleep_try_to_sleep(struct atbmwifi_common *hw_priv, int reason){
}
#endif

#endif /*  ATBMWIFI__HWIO_SDIO_H_INCLUDED */

