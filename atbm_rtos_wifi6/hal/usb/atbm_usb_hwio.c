/**************************************************************************************************************
 * altobeam RTOS
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/


#include "atbm_hal.h"


int atbm_ep0_read(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
				atbm_void *buf, atbm_uint32 buf_len)
{

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	return hw_priv->sbus_ops->sbus_read_sync(hw_priv->sbus_priv,
						  addr,
						  buf, buf_len); 
}

int atbm_ep0_write(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
				const atbm_void *buf, atbm_uint32 buf_len)
{
	ATBM_BUG_ON(!hw_priv->sbus_ops);
	return hw_priv->sbus_ops->sbus_write_sync(hw_priv->sbus_priv,
						addr,buf, buf_len);
}

int atbm_fw_write(struct atbmwifi_common *priv, atbm_uint32 addr, const atbm_void *buf,
                        atbm_uint32 buf_len)
{
	return atbm_ep0_write(priv,  addr, buf, buf_len);
}

int atbm_direct_read_reg_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val)
{
    int ret;

	ret= atbm_ep0_read(hw_priv, addr, val, sizeof(int));

	if (ret <= 0) {
		*val = 0xff;
		wifi_printk(WIFI_IF,
			"%s:  can't write " \
			"config register.\n", __FUNCTION__);
		goto out;
	}

out:
	return ret;
}
int atbm_direct_write_reg_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 val)
{
	int ret = atbm_ep0_write(hw_priv, addr, &val, sizeof(val));

	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s:  can't write " \
			"config register.\n", __FUNCTION__);
		goto out;
	}

out:
	return ret;
}

int atbm_before_load_firmware(struct atbmwifi_common *hw_priv)
{
	atbm_uint32 val;
	atbm_direct_write_reg_32(hw_priv,0x16100008,0xffffffff);
	atbm_direct_write_reg_32(hw_priv,0x0b000130,0xf);
	atbm_direct_write_reg_32(hw_priv,0x0b000134,0xf);

	atbm_direct_read_reg_32(hw_priv,hw_priv->chip_priv->reset_reg, &val);	
	val |= hw_priv->chip_priv->reset_bit;
	atbm_direct_write_reg_32(hw_priv,hw_priv->chip_priv->reset_reg, val);

	atbm_direct_read_reg_32(hw_priv,0x1610007c, &val);	
	val |= ATBM_BIT(1);
	atbm_direct_write_reg_32(hw_priv,0x1610007c, val);

    atbm_direct_read_reg_32(hw_priv,0x1610014c,&val);
    //val &= ~BIT(19);
    val |= ATBM_BIT(19);
    atbm_direct_write_reg_32(hw_priv,0x1610014c,val);

    atbm_direct_read_reg_32(hw_priv,0x1610014c,&val);

    wifi_printk(WIFI_ALWAYS, "usb loadfw(%x)\n",val);

	return 0;
}

int atbm_after_load_firmware(struct atbmwifi_common *hw_priv)
{
	atbm_uint32 regdata;

	atbm_direct_write_reg_32(hw_priv,0x16100008, 0);
	atbm_direct_write_reg_32(hw_priv,0x0b000130, 0);
	atbm_direct_write_reg_32(hw_priv,0x0b000134, 0);

	atbm_direct_read_reg_32(hw_priv,0x1610102c, &regdata);
	wifi_printk(WIFI_ALWAYS, "atbm_after_load_firmware: 0x1610102c=0x%x\n", regdata);
	regdata &= ~(0xffff0000);

	regdata |= ATBM_BIT(0) | ATBM_BIT(1) | (0x6 << 16);
	atbm_direct_write_reg_32(hw_priv,0x1610102c, regdata);
	atbm_direct_read_reg_32(hw_priv,0x1610102c, &regdata);
	wifi_printk(WIFI_ALWAYS, "atbm_after_load_firmware: 0x1610102c=0x%x\n", regdata);
	atbm_direct_read_reg_32(hw_priv,hw_priv->chip_priv->reset_reg, &regdata);
	regdata &= ~hw_priv->chip_priv->reset_bit;
	atbm_direct_write_reg_32(hw_priv,hw_priv->chip_priv->reset_reg,regdata);
	atbm_direct_read_reg_32(hw_priv,0x16101000, &regdata);
	wifi_printk(WIFI_ALWAYS, "atbm_after_load_firmware: 0x16101000=0x%x\n", regdata);

	wifi_printk(WIFI_ALWAYS, "lmac_start\n");

	hw_priv->init_done = 1;
	/*atbm receive packet form the device*/
	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	hw_priv->sbus_ops->sbus_memcpy_fromio(hw_priv->sbus_priv,0x2,ATBM_NULL,RX_BUFFER_SIZE);
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return 0;
}
atbm_void atbm_firmware_init_check(struct atbmwifi_common *hw_priv)
{


}

