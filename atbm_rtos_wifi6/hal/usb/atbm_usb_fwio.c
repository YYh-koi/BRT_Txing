/**************************************************************************************************************
 * altobeam RTOS
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/

 
#include "atbm_hal.h"  

extern int atbm_direct_read_reg_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val);
atbm_void atbm_release_firmware(atbm_void)    
{
	wifi_printk(WIFI_ALWAYS,"atbm_release_firmware\n");
}
int atbm_init_firmware(atbm_void)   
{
	wifi_printk(WIFI_ALWAYS,"atbm_init_firmware\n");
	//atbm_memset(&fw_altobeam,0,sizeof(struct firmware_altobeam));
	return 0; 
}


static int atbm_load_firmware_generic(struct atbmwifi_common *priv, const atbm_uint8 *data,atbm_uint32 size,atbm_uint32 addr)
{
	int ret=0;
	atbm_uint32 put = 0;
	atbm_uint8 *buf = ATBM_NULL;

	buf = atbm_kmalloc(priv->chip_priv->download_bs*2,GFP_KERNEL);
	if (!buf) {
		wifi_printk(WIFI_DBG_ERROR,
			"%s: can't allocate bootloader buffer.\n", __FUNCTION__);
		ret = -ATBM_ENOMEM;
		goto error;
	}

	//if(priv->sbus_ops->bootloader_debug_config)
	//	priv->sbus_ops->bootloader_debug_config(priv->sbus_priv,0);

	/*  downloading loop */
	wifi_printk(WIFI_ALWAYS,"%s: addr %x: len %x\n",__FUNCTION__,addr,size);
	for (put = 0; put < size ;put += priv->chip_priv->download_bs) {
		atbm_uint32 tx_size;

		/* calculate the block size */
		tx_size  = atbm_min((size - put),(atbm_uint32)priv->chip_priv->download_bs);

		atbm_memcpy(buf, &data[put], tx_size);

		/* send the block to sram */
		ret = atbm_fw_write(priv,put+addr,buf, tx_size);
		if (ret < 0) {
			wifi_printk(WIFI_DBG_ERROR,
				"%s: can't write block at line %d.\n",
				__FUNCTION__, __LINE__);
			goto error;
		}
	} /* End of bootloader download loop */
error:
	atbm_kfree(buf);
	return ret;
}

#if USE_COMPRESSED_FIRMWARE
static int decopress_callback(CBP *cbp, atbm_uint8 *addr, int offset, int size){
	return atbm_load_firmware_generic(cbp->priv,addr,size,cbp->addr+offset);
}

static int atbm_start_load_firmware(struct atbmwifi_common *priv)
{
	CBP cbp;

	cbp.addr = DOWNLOAD_ITCM_ADDR;
	cbp.priv = priv;
	cbp.cb = decopress_callback;

	fcs_decompress(&fw_code, &cbp);

	cbp.addr = DOWNLOAD_DTCM_ADDR;
	fcs_decompress(&fw_data, &cbp);
}

#else
static int atbm_start_load_firmware(struct atbmwifi_common *priv)
{
	int ret;
	struct firmware_altobeam *fw_altobeam = &priv->chip_priv->fw_altobeam;
	struct firmware_headr* hdr = fw_altobeam->hdr;

	wifi_printk(WIFI_DBG_ERROR,"%s: START DOWNLOAD ICCM=========\n", __FUNCTION__);
	ret = atbm_load_firmware_generic(priv,fw_altobeam->fw_iccm,hdr->iccm_len,DOWNLOAD_ITCM_ADDR|priv->chip_priv->mem_access_bit);
	if(ret<0)
		goto error;

	wifi_printk(WIFI_DBG_ERROR,"%s: START DOWNLOAD DCCM=========\n", __FUNCTION__);
	ret = atbm_load_firmware_generic(priv,fw_altobeam->fw_dccm,hdr->dccm_len,DOWNLOAD_DTCM_ADDR|priv->chip_priv->mem_access_bit);
	if(ret<0)
		goto error;

	if(hdr->sram_len){
		wifi_printk(WIFI_DBG_ERROR,"START DOWNLOAD BLE SRAM=========\n");
		ret = atbm_load_firmware_generic(priv, fw_altobeam->fw_sram, hdr->sram_len, hdr->sram_addr);
		if(ret<0)
			goto error;
	}
	wifi_printk(WIFI_DBG_ERROR, "%s: FIRMWARE DOWNLOAD SUCCESS\n",__FUNCTION__);
error:
	return ret;
}
#endif

int atbm_load_firmware(struct atbmwifi_common *hw_priv)
{
	int ret;

	wifi_printk(WIFI_ALWAYS,"atbm_before_load_firmware++\n");
	ret = atbm_before_load_firmware(hw_priv);
	if(ret <0)
		goto out;
	wifi_printk(WIFI_ALWAYS,"atbm_start_load_firmware++\n");
	ret = atbm_start_load_firmware(hw_priv);
	if(ret <0)
		goto out;
	wifi_printk(WIFI_ALWAYS,"atbm_after_load_firmware++\n");
	ret = atbm_after_load_firmware(hw_priv);
	if(ret <0){
		goto out;
	}
	ret =0;
out:
	return ret;
}
