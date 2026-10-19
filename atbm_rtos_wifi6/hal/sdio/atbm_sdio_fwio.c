/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#include "atbm_hal.h"
#include "atbm_sdio.h"
#include "atbm_sdio_hwio.h"
#include "atbm_sdio_fwio.h"
#include "firmware_bt.h"

#define ALTOBEAM_WIFI_HDR_FLAG  		(0x34353677)
#define BT_DOWNLOAD_ITCM_ADDR				0x02010000
#define BT_DOWNLOAD_ITCM_LEN				0x80000
#define BT_DOWNLOAD_DTCM_ADDR				0x02800000

struct bt_firmware_headr {
	atbm_uint32 flags; /*0x34353677*/
	atbm_uint32 version;
	atbm_uint32 iccm_len;
	atbm_uint32 dccm_len;
	atbm_uint32 reserve[3];
	atbm_uint16 reserve2;
	atbm_uint16 checksum;
};


extern int atbm_reg_read_32(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_uint32 *val);
extern int atbm_reg_write_32(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_uint32 val);
extern int atbm_reg_read_16(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_uint16 *val);
extern int atbm_reg_write_16(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_uint16 val);
extern int atbm_ahb_read_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val);
extern int atbm_ahb_write_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 val);

static int atbm_reg_write_bit(struct atbmwifi_common *hw, atbm_uint32 addr, atbm_uint8 endBit,
    atbm_uint8 startBit, atbm_uint32 data)
{
    atbm_uint32	uiRegValue = 0;
    atbm_uint32 regmask = 0;
    int ret = 0;
    ret = atbm_ahb_read_32(hw, addr, &uiRegValue);
    if (ret < 0) {
        goto rw_end;
    }
    regmask = ~((1 << startBit) - 1);
    regmask &= ((1 << endBit) - 1) | (1 << endBit);
    uiRegValue &= ~regmask;
    uiRegValue |= (data << startBit) & regmask;
    ret = atbm_ahb_write_32(hw, addr, uiRegValue);
    if (ret < 0) {
        goto rw_end;
    }

rw_end:
    return ret;
}

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

int atbm_before_load_firmware(struct atbmwifi_common *hw_priv)
{
	int ret=0;
	int i;
	atbm_uint32 val32;
	atbm_uint16 val16;
	//int major_revision;

	atbm_uint32 config_reg;
	
	ATBM_BUG_ON(!hw_priv);

	/* Read CONFIG Register Value - We will read 32 bits */
	ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: can't read config register.\n", __FUNCTION__);
		goto out;
	}
	/* Set wakeup bit in device */
	ret = atbm_reg_read_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID, &val16);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: set_wakeup: can't read " \
			"control register.\n", __FUNCTION__);
		goto out;
	}

	ret = atbm_reg_write_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID,
		val16 | ATBM_HIFREG_CONT_WUP_BIT);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: set_wakeup: can't write " \
			"control register.\n", __FUNCTION__);
		goto out;
	}
#if TEST_DCXO_DPLL_CONFIG
	/*start config dcxo */
	ret=atbm_config_dcxo(hw_priv,dcxo_value,PROJ_TYPE,DCXO_TYPE,DPLL_CLOCK);
	if (ret<0){
		wifi_printk(WIFI_IF, "atbm_config_dcxo error.\n");
	}
	/*start config dpll */
	ret = atbm_config_dpll(hw_priv,dpll_value,PROJ_TYPE,DPLL_CLOCK);
	if (ret<0){
		wifi_printk(WIFI_IF, "atbm_config_dpll error.\n");
	 }
	/*The fifth step store dpll value to smu*/
	atbm_set_config_to_smu(hw_priv,DPLL_CLOCK);
	/*start shut down system*/
	ret =atbm_system_done(hw_priv);
	if (ret<0){
		wifi_printk(WIFI_IF, "atbm_system_done error.\n");
	}
	wifi_printk(WIFI_IF, "atbm_wait_wlan_rdy  Wait for wakeup .\n");
	/* Set wakeup bit in device */
	ret = atbm_reg_read_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID, &val16);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: set_wakeup: can't read " \
			"control register.\n", __FUNCTION__);
		goto out;
	}

	ret = atbm_reg_write_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID,
		val16 | ATBM_HIFREG_CONT_WUP_BIT);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: set_wakeup: can't write " \
			"control register.\n", __FUNCTION__);
		goto out;
	}
#endif
	wifi_printk(WIFI_DBG_MSG, "atbm_wait_wlan_rdy  Wait for wakeup .\n");
	/* Set wakeup bit in device */
	ret = atbm_reg_read_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID, &val16);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: set_wakeup: can't read " \
			"control register.\n", __FUNCTION__);
		goto out;
	}

	ret = atbm_reg_write_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID,
		val16 | ATBM_HIFREG_CONT_WUP_BIT);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: set_wakeup: can't write " \
			"control register.\n", __FUNCTION__);
		goto out;
	}
	/* Wait for wakeup */
	for (i = 0 ; i < 3000 ; i += 1 + i / 2) {
		ret = atbm_reg_read_16(hw_priv,
			ATBM_HIFREG_CONTROL_REG_ID, &val16);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: wait_for_wakeup: can't read " \
				"control register.\n", __FUNCTION__);
			goto out;
		}

		if (val16 & ATBM_HIFREG_CONT_RDY_BIT) {
			wifi_printk(WIFI_IF,
				"WLAN device is ready.\n");
			break;
		}
		atbm_mdelay(i);
	}

	if ((val16 & ATBM_HIFREG_CONT_RDY_BIT) == 0) {
		wifi_printk(WIFI_IF,
			"%s: wait_for_wakeup: device is not responding.\n",
			__FUNCTION__);
		ret = -ATBM_ETIMEDOUT;
		goto out;
	}
	atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &config_reg);
	if(config_reg & ATBM_HIFREG_PS_SYNC_SDIO_FLAG)
	{
		config_reg |= ATBM_HIFREG_PS_SYNC_SDIO_CLEAN;
		atbm_reg_write_32(hw_priv,ATBM_HIFREG_CONFIG_REG_ID,config_reg);
	}

	/*
	*reset chip
	*/
	if(hw_priv->chip_ver){
		int read_time = 0;

		/* dpll */
		ret = atbm_ahb_read_32(hw_priv, 0x1610102c,&val32);
		if(ret) {
			wifi_printk(WIFI_ALWAYS,"%s:read 0x1610102c err\n",__func__);
			goto out;
		}

		val32 |= ATBM_BIT(3);
		ret = atbm_ahb_write_32(hw_priv, 0x1610102c,val32);
		if(ret) {
			wifi_printk(WIFI_ALWAYS, "%s:read 0x1610102c err\n",__func__);
			goto out;
		}

		/* read efuse */
		do{
			ret = atbm_ahb_read_32(hw_priv, 0x16101028, &val32);
			if ((val32 & 0xf) == 5 || read_time++ > 200) {
				break;
			} else {
				wifi_printk(WIFI_ALWAYS, "%s:read 0x16101028 %x\n",__func__, val32);
			}
		} while (1);


		ret = atbm_ahb_read_32(hw_priv, 0x16101004,&val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS,"%s:read 0x16101004,err\n",__func__);
			goto out;
		}

		wifi_printk(WIFI_ALWAYS,"%s:read 0x16101004,%x\n",__func__, val32);
		val32 |= ATBM_BIT(29); 
		ret = atbm_ahb_write_32(hw_priv, 0x16101004,val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS,"%s:write 0x16101004,err\n",__func__);
			goto out;
		}

		ret = atbm_ahb_read_32(hw_priv, 0x1610029c,&val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS,"%s:read 0x16100074,err\n",__func__);
			goto out;
		}

		wifi_printk(WIFI_ALWAYS,"%s:read 0x16100074,%x\n",__func__, val32);
		val32 |= ATBM_BIT(0);
		ret = atbm_ahb_write_32(hw_priv, 0x1610029c,val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS,"%s:write 0x16100074,err\n",__func__);
			goto out;
		}

		/* for asic reset */
		ret = atbm_ahb_read_32(hw_priv, 0x16100244,&val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS,"%s:read 0x16100244,err\n",__func__);
			goto out;
		}
		val32 &= ~0xff;
		val32 |= 0xf7;
		ret = atbm_ahb_write_32(hw_priv, 0x16100244, val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS,"%s:write 0x161000244 err\n",__func__);
			goto out;
		}

		val32 = ATBM_SDIO_FN1_RESET_WIFI_CPU;
		ret = atbm_reg_write_32(hw_priv, ATBM_6X65S_SDIO_FN1_RESET_REG, val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS, "%s:change to sdio mode err\n", __func__);
			goto out;
		}

		val32 = ATBM_HW_RESET_WIFI_RECOERY | ATBM_SDIO_FN1_RESET_WIFI_CPU | ATBM_SDIO_FN1_RESET_MMU_SYNC;
		ret = atbm_reg_write_32(hw_priv, ATBM_6X65S_SDIO_FN1_RESET_REG, val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS, "%s:change to sdio mode err\n", __func__);
			goto out;
		}

		val32 = 0;
		atbm_ahb_write_32(hw_priv,  0x16100244, val32);
		val32 = ATBM_SDIO_FN1_RESET_WIFI_CPU;
		ret = atbm_ahb_write_32(hw_priv, ATBM_6X65S_SDIO_FN1_RESET_REG, val32);
		if (ret) {
			wifi_printk(WIFI_ALWAYS, "%s:change to sdio mode err\n", __func__);
			goto out;
		}

		val32 = 0xffffffff;
		atbm_ahb_write_32(hw_priv, 0x16100008, 0xffffffff);
		ret = atbm_ahb_write_32(hw_priv, 0xa900000, 0);
		if (ret) {
			wifi_printk(WIFI_ALWAYS, "%s:change to sdio mode err\n",__func__);
			goto out;
		}
		/* end of asic reset */

		ret = atbm_ahb_read_32(hw_priv, 0xa900000, &val32);
		if (ret < 0) {
			wifi_printk(WIFI_ALWAYS,"%s:read 0xa900000 err\n",__func__);
			goto out;
		}
		wifi_printk(WIFI_ALWAYS, "%s:read 0xa900000 %x\n",__func__, val32);

		wifi_printk(WIFI_ALWAYS, "%s:0xa900000 [%x]\n",__func__,val32);
		if (val32 != 0) {
			wifi_printk(WIFI_ALWAYS,"%s:chip mode err(%d)\n",__func__,val32);
			goto out;
		}
	}else

	{
		/* set cpu reset ,cpu will stop */
		/* Checking for access mode */
		ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: enable_irq: can't read " \
				"config register.\n", __FUNCTION__);
			goto out;
		}
		val32 |= ATBM_HIFREG_CONFIG_CPU_RESET_BIT|ATBM_HIFREG_CONFIG_ACCESS_MODE_BIT;
		ret = atbm_reg_write_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: enable_irq: can't write " \
				"config register.\n", __FUNCTION__);
			goto out;
		}
		
		ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: enable_irq: can't read " \
				"config register.\n", __FUNCTION__);
			goto out;
		}
		
		ATBM_WARN_ON_FUNC(!(val32 & ATBM_HIFREG_CONFIG_ACCESS_MODE_BIT));

		/****************************/
		/********sdio io latch*******/
		ret=atbm_ahb_write_32(hw_priv,0x161010dc,0x0b);
        if(ret<0){
            wifi_printk(WIFI_IF,
                    "%s: 0x161010dc: can't write register.\n", __FUNCTION__);
            goto out;
        }
        /****************************/
        
        /*************reset chip***********/
		ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
        if (ret < 0) {
            wifi_printk(WIFI_IF,
                    "%s:chipreset: can't read " \
                    "config register.\n", __FUNCTION__);
            goto out;
        }
		val32 |= ATBM_HIFREG_CONFIG_CPU_RESET_BIT_2;
		ret = atbm_reg_write_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,val32);
        if (ret < 0) {
            wifi_printk(WIFI_IF,
                    "%s: chipreset: can't write " \
                    "config register.\n", __FUNCTION__);
            goto out;
        }
		val32 &= ~ATBM_BIT(22);
		ret = atbm_reg_write_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,val32);
        if (ret < 0) {
            wifi_printk(WIFI_IF,
                    "%s: chipreset2: can't write " \
                    "config register.\n", __FUNCTION__);
            goto out;
        }
		/*
		not reset wifi , insmod wifi success
		*/
		ret = atbm_ahb_read_32(hw_priv,0xab0016c,&val32);
		if(ret<0)
			wifi_printk(WIFI_ALWAYS,"read 0xab0016c err\n");
		wifi_printk(WIFI_ALWAYS,"%s:0xab0016c = [%x]\n",__FUNCTION__,val32);
		val32 |= ATBM_BIT(0);
		ret = atbm_ahb_write_32(hw_priv,0xab0016c,val32);
		if(ret<0)
			wifi_printk(WIFI_ALWAYS,"write 0xab0016c err\n");
		
		ret = atbm_ahb_read_32(hw_priv,0xab0016c,&val32);
		if(ret<0)
			wifi_printk(WIFI_ALWAYS,"read 0xab0016c err\n");
		wifi_printk(WIFI_ALWAYS,"%s:0xab0016c = [%x]\n",__FUNCTION__,val32);
		val32 &= ~ATBM_BIT(0);
		ret = atbm_ahb_write_32(hw_priv,0xab0016c,val32);
		if(ret<0)
			wifi_printk(WIFI_ALWAYS,"write 0xab0016c err\n");

		/*reset mac*/
		ret = atbm_ahb_read_32(hw_priv,0x16100074,&val32);
		if(ret<0)
			wifi_printk(WIFI_ALWAYS,"read 0xab0016c err\n");
		val32 |= ATBM_BIT(1);
		ret = atbm_ahb_write_32(hw_priv,0x16100074,val32);
		if(ret<0)
			wifi_printk(WIFI_ALWAYS,"write 0xab0016c err\n");
		
		ret = atbm_ahb_read_32(hw_priv,0x16100074,&val32);
		wifi_printk(WIFI_ALWAYS,"reset val(%x)\n",val32);
		val32 &= ~ATBM_BIT(1);
		ret = atbm_ahb_write_32(hw_priv,0x16100074,val32);
		if(ret<0)
			wifi_printk(WIFI_ALWAYS,"write 0xab0016c err\n");

		ret = atbm_ahb_read_32(hw_priv,0x16100074,&val32);
		wifi_printk(WIFI_ALWAYS,"after reset(%x)\n",val32);
	}

out:
	return ret;

}

static int atbm_load_firmware_generic(struct atbmwifi_common *priv, const atbm_uint8 *data,atbm_uint32 size,atbm_uint32 addr)
{
	int ret=0;
	atbm_uint32 put = 0;
	atbm_uint8 *buf = ATBM_NULL;


	buf = (atbm_uint8 *)atbm_kmalloc(priv->chip_priv->download_bs*2,GFP_KERNEL);
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

static int atbm_load_bt_firmware(struct atbmwifi_common *hw)
{
    static struct firmware_altobeam atbm_fw;
    struct bt_firmware_headr hdr;

    wifi_printk(WIFI_ALWAYS,"start load bt fw %d\r\n",sizeof(fw_code));

    
    memcpy(&hdr, firmware_headr, sizeof(struct firmware_headr));

    atbm_fw.fw_iccm = (unsigned  char *)fw_code;
    atbm_fw.fw_dccm  = fw_data;

    atbm_reg_write_bit(hw, 0x161000c0, 4, 4, 1);
    atbm_reg_write_bit(hw, 0x161000c0, 2, 0, 7);

    wifi_printk(WIFI_ALWAYS,"bt load itcm start P:%x, l:%x\n", BT_DOWNLOAD_ITCM_ADDR, hdr.iccm_len);
    atbm_load_firmware_generic(hw, atbm_fw.fw_iccm, hdr.iccm_len, BT_DOWNLOAD_ITCM_ADDR);
    wifi_printk(WIFI_ALWAYS,"bt load dtcm start P:%x, l:%x\n", BT_DOWNLOAD_DTCM_ADDR, hdr.dccm_len);
    atbm_load_firmware_generic(hw, atbm_fw.fw_dccm, hdr.dccm_len, BT_DOWNLOAD_DTCM_ADDR);

    atbm_reg_write_bit(hw, 0x161000c0, 31, 16, (BT_DOWNLOAD_ITCM_ADDR >> 16) & 0x0F);
    atbm_reg_write_bit(hw, 0x161000c0, 8, 8, 1);
    atbm_reg_write_bit(hw, 0x161000c0, 4, 4, 0);
 
    //atbm_reg_write_bit(hw, 0x14100100, 4, 4, 0);
    wifi_printk(WIFI_ALWAYS,"BT TEST FW\r\n");
    //atbm_reg_write_bit(hw, 0xC12004, 31, 0, 0x4321);
   
    return 0;
}

//atbm_initial_irq
int atbm_after_load_firmware(struct atbmwifi_common *hw_priv)
{
	int ret;
	atbm_uint32 val32;

	//enable gpio irq register,may need move to lmac/apb.c	SMU_Init
	ret=atbm_ahb_read_32(hw_priv,0x161000ac,&val32);
	val32&=0xFFFFF7F8;
	val32|=ATBM_BIT(12);
	ret=atbm_ahb_write_32(hw_priv,0x161000ac,val32);
	if(ret<0){
		wifi_printk(WIFI_IF,
			"%s: enable_irq: can't read " \
			"config register.\n", __FUNCTION__);

	}
	
#if (PROJ_TYPE>=ARES_A)
	ret=atbm_ahb_read_32(hw_priv,0x1610102c,&val32);
	if(ret<0){
		wifi_printk(WIFI_DBG_ERROR,
			"%s: 0x1610102c: can't read register.\n", __func__);
		goto out;
	}
	val32 &= ~(0xffff0000);
	val32 |= ATBM_BIT(0) | ATBM_BIT(1) | (DOWNLOAD_ITCM_ADDR);
	ret=atbm_ahb_write_32(hw_priv,0x1610102c,val32);
	if(ret<0){
		wifi_printk(WIFI_DBG_ERROR,
			"%s: 0x1610102c: can't write register.\n", __func__);
		goto out;
	}
	while(1)
	{
		ret=atbm_ahb_read_32(hw_priv,0x1610102c,&val32);
		if(ret<0){
			wifi_printk(WIFI_DBG_ERROR,
				"%s: 0x1610102c: can't read register.\n", __func__);
			goto out;
		}
		atbm_mdelay(10);
		break;
	}
#endif

	if(hw_priv->chip_ver){
		ret = atbm_reg_read_32(hw_priv, 0x1610032c, &val32);
		if (ret < 0) {
			wifi_printk(WIFI_ALWAYS, "%s:read 0x1610032c err\n", __func__);
			goto unsubscribe;
		}
	
		ret = atbm_reg_write_32(hw_priv, 0x16100298, val32 + 1);
		if (ret < 0) {
			wifi_printk(WIFI_ALWAYS, "%s:write 0x16100298 err\n", __func__);
			goto unsubscribe;
		}

		/* If device is CW1200 the IRQ enable/disable bits
		 * are in CONFIG register, clear cpu reset ,cpu will run */
		ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: enable_irq: can't read " \
				"config register.\n", __FUNCTION__);
			goto unsubscribe;
		}
		val32 |= ATBM_HIFREG_CONF_IRQ_RDY_ENABLE;
		//enable data1 IRQ
		val32 &= ~ATBM_HIFREG_CONFIG_CLEAR_INT_BIT;
		ret = atbm_reg_write_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: enable_irq: can't write " \
				"config register.\n", __FUNCTION__);
			goto unsubscribe;
		}
	    ret = atbm_reg_write_32(hw_priv, ATBM_6X65S_SDIO_FN1_RESET_REG, 0);
	    if(ret < 0) {
	        wifi_printk(WIFI_IF,
	            "%s: enable_irq: can't write " \
	            "config register.\n", __FUNCTION__);
	        goto unsubscribe;
	    }
	}else{
		/* If device is CW1200 the IRQ enable/disable bits
		 * are in CONFIG register, clear cpu reset ,cpu will run */
		ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: enable_irq: can't read " \
				"config register.\n", __FUNCTION__);
			goto unsubscribe;
		}
		val32 |= ATBM_HIFREG_CONF_IRQ_RDY_ENABLE;
		val32 &= ~ATBM_HIFREG_CONFIG_CPU_RESET_BIT;
		//enable data1 IRQ
		val32 &= ~ATBM_HIFREG_CONFIG_CLEAR_INT_BIT;
		ret = atbm_reg_write_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: enable_irq: can't write " \
				"config register.\n", __FUNCTION__);
			goto unsubscribe;
		}

		/* Configure device for MESSSAGE MODE */
		ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: set_mode: can't read config register.\n",
				__FUNCTION__);
			goto unsubscribe;
		}
		ret = atbm_reg_write_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,
			val32 & ~ATBM_HIFREG_CONFIG_ACCESS_MODE_BIT);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
				"%s: set_mode: can't write config register.\n",
				__FUNCTION__);
			goto unsubscribe;
		}
		/* Unless we read the CONFIG Register we are
		 * not able to get an interrupt */
		atbm_mdelay(10);
		atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);

		/*Support 62M clock*/
		ret = atbm_direct_read_reg_32(hw_priv, 0x161001f8, &val32);
		if(ret < 0){
			wifi_printk(WIFI_IF,
				"%s: can't read 0x161001f8.\n",
				__FUNCTION__);
			goto out;
		}

		val32 &= ~0xffe0;
		val32 |= (0x3 << 5) | (0x0 << 8) | (0x3 << 11) | (0x0 << 14);
		ret = atbm_direct_write_reg_32(hw_priv, 0x161001f8, val32);
		if(ret < 0){
			wifi_printk(WIFI_IF,
				"%s: can't write 0x161001f8.\n",
				__FUNCTION__);
			goto out;
		}
	}

	/*atbm receive packet form the device*/
	hw_priv->sbus_ops->sbus_memcpy_fromio(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
out:
	return ret;
unsubscribe:
	hw_priv->sbus_ops->irq_unsubscribe(hw_priv->sbus_priv);
	return ret;

}

atbm_void atbm_firmware_init_check(struct atbmwifi_common *hw_priv)
{
	atbm_uint16 ctrl_reg;
	atbm_uint32 reg;
	int ret;

	ATBM_WARN_ON(atbm_reg_write_16(hw_priv, ATBM_HIFREG_CONTROL_REG_ID,
					ATBM_HIFREG_CONT_WUP_BIT));

	if (atbm_reg_read_16(hw_priv,ATBM_HIFREG_CONTROL_REG_ID, &ctrl_reg))
		ATBM_WARN_ON(atbm_reg_read_16(hw_priv,ATBM_HIFREG_CONTROL_REG_ID,
						&ctrl_reg));

	ATBM_WARN_ON_FUNC(!(ctrl_reg & ATBM_HIFREG_CONT_RDY_BIT));

	if(hw_priv->chip_ver){
		ret = atbm_ahb_read_32(hw_priv, ATBM_SDIO_CHAN_NUM_CONFIG, &reg);

		if (ret == 0) {
			hw_priv->chip_priv->tx_channel_num = reg & 0x7f;
			hw_priv->chip_priv->rx_channel_num = (reg>>7) & 0x7f;
		}

		if ((hw_priv->chip_priv->tx_channel_num  == 0) || (hw_priv->chip_priv->rx_channel_num == 0)) {
			hw_priv->chip_priv->tx_channel_num = 64;
			hw_priv->chip_priv->rx_channel_num = 32;
			wifi_printk(WIFI_DBG_ERROR, "Get tx rx channels error %x\n", reg);
		}
		wifi_printk(WIFI_ALWAYS,"tx_channel_num %d, rx_channel_num %d\n", hw_priv->chip_priv->tx_channel_num, hw_priv->chip_priv->rx_channel_num);
	}
}

int atbm_load_firmware(struct atbmwifi_common *hw_priv)
{
	int ret;
	wifi_printk(WIFI_ALWAYS,"atbm_load_bt_firmware++\r\n");
    ret = atbm_load_bt_firmware(hw_priv);
	if(ret <0)
	{
		wifi_printk(WIFI_ALWAYS,"atbm_load_bt_firmware fail\r\n");
	}

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

