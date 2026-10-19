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

struct atbm_sdio_hw_ops{
	int (*sdio_ahb_read_unlock)(struct atbmwifi_common *hw_priv, atbm_uint32 addr,	atbm_void *buf, atbm_size_t buf_len);
	int (*sdio_ahb_write_unlock)(struct atbmwifi_common *priv, atbm_uint32 addr, const atbm_void *buf,	atbm_uint32 buf_len);
	int (*sdio_direct_read_unlock)(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val);
	int (*sdio_direct_write_unlock)(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 val);
	int (*sdio_get_rx_len)(struct atbmwifi_common *hw_priv, int next_len);
	int (*sdio_get_tx_free_buffers)(struct atbmwifi_common *hw_priv);
	int (*sdio_data_read)(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_void *buf, atbm_uint32 buf_len, int buf_id);
	int (*sdio_data_write)(struct atbmwifi_common *hw_priv, atbm_uint16 addr, const void *buf, atbm_uint32 buf_len, int buf_id);
};

struct atbm_sdio_hw_ops atbm_hw_ops[2];

static int __atbm_reg_read(struct atbmwifi_common *hw_priv, atbm_uint16 addr,
				atbm_void *buf, atbm_uint32 buf_len)
{
	atbm_uint16 addr_sdio;
	atbm_uint32 sdio_reg_addr_17bit ;

	/* Check if buffer is aligned to 4 byte boundary */
	if (ATBM_WARN_ON(((unsigned long)buf & 3) && (buf_len > 4))) {
		wifi_printk(WIFI_IF,
			   "%s: buffer is not aligned.\n", __FUNCTION__);
		return -ATBM_EINVAL;
	}

	/* Convert to SDIO Register Address */
	addr_sdio = SPI_REG_ADDR_TO_SDIO(addr);
	sdio_reg_addr_17bit = SDIO_ADDR17BIT_V1(0, 0, 0, addr_sdio);

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	return hw_priv->sbus_ops->sbus_read_sync(hw_priv->sbus_priv,
						  sdio_reg_addr_17bit,
						  buf, buf_len);
}

static int __atbm_reg_write(struct atbmwifi_common *hw_priv, atbm_uint16 addr,
				const atbm_void *buf, atbm_uint32 buf_len)
{
	atbm_uint16 addr_sdio;
	atbm_uint32 sdio_reg_addr_17bit ;


	/* Convert to SDIO Register Address */
	addr_sdio = SPI_REG_ADDR_TO_SDIO(addr);
	sdio_reg_addr_17bit = SDIO_ADDR17BIT_V1(0, 0, 0, addr_sdio);

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	return hw_priv->sbus_ops->sbus_write_sync(hw_priv->sbus_priv,
						sdio_reg_addr_17bit,
						buf, buf_len);
}

int atbm_reg_read_unlock(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_void *buf,
			atbm_uint32 buf_len)
{
	int ret = 0;
	int retry=0;
	ATBM_BUG_ON(!hw_priv->sbus_ops);
	while (retry <= 3) {
		ret = __atbm_reg_read(hw_priv, addr, buf, buf_len);
		if(ret){		
			wifi_printk(WIFI_ALWAYS,"%s\n",__func__);
			retry++;
		}else{
			break;
		}
	}
	return ret;
}

int atbm_reg_write_unlock(struct atbmwifi_common *hw_priv, atbm_uint16 addr, const atbm_void *buf,
			atbm_uint32 buf_len)
{
	int ret = 0;
	int retry=0;
	ATBM_BUG_ON(!hw_priv->sbus_ops);
	while (retry <= 3) {
		ret = __atbm_reg_write(hw_priv, addr, buf, buf_len);	
		if(ret){	
			wifi_printk(WIFI_ALWAYS,"%s\n",__func__);
			retry++;
		}else{
			break;
		}
	}
	return ret;
}

int atbm_reg_read(struct atbmwifi_common *hw_priv, atbm_uint16 addr, atbm_void *buf,
			atbm_uint32 buf_len)
{
	int ret;
	ATBM_BUG_ON(!hw_priv->sbus_ops);
	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	ret = __atbm_reg_read(hw_priv, addr, buf, buf_len);
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}

int atbm_reg_write(struct atbmwifi_common *hw_priv, atbm_uint16 addr, const atbm_void *buf,
			atbm_uint32 buf_len)
{
	int ret;
	ATBM_BUG_ON(!hw_priv->sbus_ops);
	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	ret = __atbm_reg_write(hw_priv, addr, buf, buf_len);
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}

int atbm_reg_read_16(struct atbmwifi_common *hw_priv,
				atbm_uint16 addr, atbm_uint16 *val)
{
	atbm_uint32 bigVal;
	int ret;
	ret = atbm_reg_read(hw_priv, addr, &bigVal, sizeof(bigVal));
	*val = (atbm_uint16)bigVal;
	return ret;
}

int atbm_reg_write_16(struct atbmwifi_common *hw_priv,
				atbm_uint16 addr, atbm_uint16 val)
{
	atbm_uint32 bigVal = (atbm_uint32)val;
	return atbm_reg_write(hw_priv, addr, &bigVal, sizeof(bigVal));
}

static inline int atbm_reg_read_unlock_32(struct atbmwifi_common *hw_priv,
				     atbm_uint16 addr, atbm_uint32 *val)
{
	return atbm_reg_read_unlock(hw_priv, addr, val, sizeof(int));
}
static inline int atbm_reg_write_unlock_32(struct atbmwifi_common *hw_priv,
				      atbm_uint16 addr, atbm_uint32 val)
{
	return atbm_reg_write_unlock(hw_priv, addr, &val, sizeof(int));
}

int atbm_reg_read_32(struct atbmwifi_common *hw_priv,
				atbm_uint16 addr, atbm_uint32 *val)
{
	return atbm_reg_read(hw_priv, addr, val, sizeof(int));
}

int atbm_reg_write_32(struct atbmwifi_common *hw_priv,
				atbm_uint16 addr, atbm_uint32 val)
{
	return atbm_reg_write(hw_priv, addr, &val, sizeof(int));
}

static int __atbm_reg_read_32(struct atbmwifi_common *hw_priv,
					atbm_uint16 addr, atbm_uint32 *val)
{
	return __atbm_reg_read(hw_priv, addr, val, sizeof(*val));
}

static int __atbm_reg_write_32(struct atbmwifi_common *hw_priv,
					atbm_uint16 addr, atbm_uint32 val)
{
	return __atbm_reg_write(hw_priv, addr, &val, sizeof(val));
}

static int atbm_bh_read_ctrl_reg(struct atbmwifi_common *hw_priv,
					  atbm_uint16 *ctrl_reg)
{
	int ret=0,retry=0;
	while (retry <= MAX_RETRY) {
		ret = atbm_reg_read_16(hw_priv,
				ATBM_HIFREG_CONTROL_REG_ID, ctrl_reg);			
		if(!ret){	
				break;
		}else{
			/*reset sdio internel reg by send cmd52 to abort*/
			if(hw_priv->sbus_ops->abort){
				ATBM_WARN_ON(hw_priv->sbus_ops->abort(hw_priv->sbus_priv));
			}
			retry++;
			atbm_mdelay(retry);
			wifi_printk(WIFI_BH,"[BH] Failed to read control register.ret=%x\n",ret);
		}
	}
	return ret;
}

static int __atbm_sdio_fn4_reg_read(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
	void *buf, atbm_uint32 buf_len)
{
	/* Check if buffer is aligned to 4 byte boundary */
	if (ATBM_WARN_ON(((unsigned long)buf & 3) && (buf_len > 4))) {
		wifi_printk(WIFI_ALWAYS,
			"%s:%d:buffer is not aligned..\n", __func__,__LINE__);
		return -ATBM_EINVAL;
	}

	return __atbm_sdio_memcpy_fromio(hw_priv->sbus_priv->cmd_func, buf, addr, buf_len);
}

static int __atbm_sdio_fn4_reg_write(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
	void *buf, atbm_uint32 buf_len)
{
	/* Check if buffer is aligned to 4 byte boundary */
	if (ATBM_WARN_ON(((unsigned long)buf & 3) && (buf_len > 4))) {
		wifi_printk(WIFI_ALWAYS,
			"%s:%d:buffer is not aligned..\n", __func__,__LINE__);
		return -ATBM_EINVAL;
	}

	return __atbm_sdio_memcpy_toio(hw_priv->sbus_priv->cmd_func, addr, buf, buf_len);
}

//usb
int atbm_indirect_read_unlock(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_void *buf,
			 atbm_uint32 buf_len, atbm_uint32 prefetch, atbm_uint16 port_addr)
{
	atbm_uint32 val32 = 0;
	int i, ret;

	if ((buf_len / 2) >= 0x1000) {
		wifi_printk(WIFI_IF,
				"%s: Can't read more than 0xfff words.\n",
				__FUNCTION__);
		ATBM_WARN_ON_FUNC(1);
		return -ATBM_EINVAL;
	}

	/* Write address */
	ret = __atbm_reg_write_32(hw_priv, ATBM_HIFREG_SRAM_BASE_ADDR_REG_ID,
					addr);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
				"%s: Can't write address register.\n",
				__FUNCTION__);
		goto out;
	}

	/* Read CONFIG Register Value - We will read 32 bits */
	ret = __atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
				"%s: Can't read config register.\n",
				__FUNCTION__);
		goto out;
	}

	/* Set PREFETCH bit */
	ret = __atbm_reg_write_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,
					val32 | prefetch);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
				"%s: Can't write prefetch bit.\n",
				__FUNCTION__);
		goto out;
	}

	/* Check for PRE-FETCH bit to be cleared */
	for (i = 0; i < 20; i++) {
		ret = __atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,
					   &val32);
		if (ret < 0) {
			wifi_printk(WIFI_IF,
					"%s: Can't check prefetch bit.\n",
					__FUNCTION__);
			goto out;
		}
		if (!(val32 & prefetch))
			break;

		atbm_mdelay(i);
	}

	if (val32 & prefetch) {
		wifi_printk(WIFI_IF,
				"%s: Prefetch bit is not cleared.\n",
				__FUNCTION__);
		goto out;
	}

	/* Read data port */
	ret = __atbm_reg_read(hw_priv, port_addr, buf, buf_len);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
				"%s: Can't read data port.\n",
				__FUNCTION__);
		goto out;
	}

out:
	return ret;
}

int atbm_ahb_read_unlock_v0(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
				atbm_void *buf, atbm_size_t buf_len){
	return atbm_indirect_read_unlock(hw_priv, addr, buf, buf_len,
			ATBM_HIFREG_CONFIG_AHB_PFETCH_BIT, ATBM_HIFREG_AHB_DPORT_REG_ID);
}

int atbm_ahb_write_unlock_v0(struct atbmwifi_common *priv, atbm_uint32 addr, const void *buf,
                        atbm_uint32 buf_len)
{
    int ret;
	int retry=0;
	//printk(KERN_ERR "%s: addr %x\n",__func__,addr);
    if (buf_len  >= 512) {
            wifi_printk(WIFI_ALWAYS,
                            "%s: Can't wrire more than 0xfff words.\n",
                            __func__);
            ATBM_WARN_ON_FUNC(1);
			wifi_printk(WIFI_ALWAYS, "%s:EXIT (1) \n",__func__);
            return -1;
    }
    /* Write address */
	while(retry<=3){
        ret = __atbm_reg_write_32(priv, ATBM_HIFREG_SRAM_BASE_ADDR_REG_ID, addr);
        if (ret < 0) {
                wifi_printk(WIFI_ALWAYS,
                                "%s: Can't write address register.\n",
                                __func__);
				retry++;
        }else{
			break;
		}
	}
	retry=0;
    /* Write data port */
	while(retry<=3){
        ret = __atbm_reg_write(priv, ATBM_HIFREG_AHB_DPORT_REG_ID,
                                        buf, buf_len);
        if (ret < 0) {
                wifi_printk(WIFI_ALWAYS, "%s: Can't write data port.\n",
                                __func__);
				retry++;
        }else{
			break;
		}
	}
//out:
    return ret;
}

int atbm_ahb_read_unlock_v1(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
				atbm_void *buf, atbm_size_t buf_len){
	int ret;

	ret = __atbm_reg_write_32(hw_priv,ATBM_6X65S_SDIO_FN4_MEM_BASE_0,addr);

	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:%d: can't read " \
			"data port, addr[%x:%x].\n", __func__,__LINE__, addr, ATBM_6X65S_SDIO_FN4_MEM_BASE_0);
		return ret;
	}
	ret =  __atbm_sdio_fn4_reg_read(hw_priv,0,buf,buf_len);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:%d: can't read " \
			"data port, addr[%x:%x].\n", __func__,__LINE__, addr, ATBM_6X65S_SDIO_FN4_MEM_BASE_0);
		return ret;
	}
	wifi_printk(WIFI_DBG_MSG, "r addr:%x %x\n", addr, *((atbm_uint32*)buf));
	return ret;
}			


int atbm_ahb_write_unlock_v1(struct atbmwifi_common *hw_priv, atbm_uint32 addr, const atbm_void *buf,
						atbm_uint32 buf_len){
	int ret;

	ret = __atbm_reg_write_32(hw_priv,ATBM_6X65S_SDIO_FN4_MEM_BASE_0,addr);
	
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:%d: can't read " \
			"data port, addr[%x:%x].\n", __func__,__LINE__, addr, ATBM_6X65S_SDIO_FN4_MEM_BASE_0);
		return ret;
	}
	ret =  __atbm_sdio_fn4_reg_write(hw_priv,0,(atbm_void *)buf,buf_len);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:%d: can't read " \
			"data port, addr[%x:%x].\n", __func__,__LINE__, addr, ATBM_6X65S_SDIO_FN4_MEM_BASE_0);
		return ret;
	}
	wifi_printk(WIFI_DBG_MSG, "w addr:%x %x\n", addr, *((atbm_uint32*)buf));
	return ret;
}

static inline int atbm_ahb_read_unlock_32(struct atbmwifi_common *hw_priv,
					 atbm_uint32 addr, atbm_uint32 *val)
{
	return atbm_ahb_read_unlock(hw_priv, addr, val, sizeof(int));
}

static inline int atbm_ahb_write_unlock_32(struct atbmwifi_common *hw_priv,
					  atbm_uint32 addr, atbm_uint32 val)
{
	return  atbm_ahb_write_unlock(hw_priv, addr, &val, sizeof(val));
}


int atbm_direct_write_unlock_v0(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 val)
{
    int ret;
	atbm_uint32 val32;
	atbm_uint32 orig_config_data = 0;

	/* Checking for access mode */
	ret = atbm_reg_read_unlock_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s: can't read " \
			"config register.\n", __func__);
		goto out1;
	}
	orig_config_data = val32;
	val32 |= ATBM_HIFREG_CONFIG_ACCESS_MODE_BIT;
	ret = atbm_reg_write_unlock_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,val32);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:  can't write " \
			"config register.\n", __func__);
		goto out;
	}
	ret = atbm_ahb_write_unlock_32(hw_priv,addr,val);

	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:  can't write " \
			"config register.\n", __func__);
		goto out;
	}

out:
	ret = atbm_reg_write_unlock_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,orig_config_data);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s: enable_irq: can't write " \
			"config register.\n", __func__);
		goto out;
	}
out1:
	return ret;
}

int atbm_direct_read_unlock_v0(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val)
{
    int ret;
	atbm_uint32 val32;
	atbm_uint32 orig_config_data = 0;

	/* Checking for access mode */
	ret = atbm_reg_read_unlock_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s: can't read " \
			"config register.\n", __func__);
		goto out1;
	}
	orig_config_data = val32;
	val32 |= ATBM_HIFREG_CONFIG_ACCESS_MODE_BIT;
	ret = atbm_reg_write_unlock_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,val32);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:  can't write " \
			"config register.\n", __func__);
		goto out;
	}
	ret = atbm_ahb_read_unlock_32(hw_priv,addr,val);
	//printk("val=%x\n",val);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s:  can't write " \
			"config register.\n", __func__);
		goto out;
	}

out:
	ret = atbm_reg_write_unlock_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID,orig_config_data);
	if (ret < 0) {
		wifi_printk(WIFI_ALWAYS,
			"%s: enable_irq: can't write " \
			"config register.\n", __func__);
		goto out;
	}
out1:
	return ret;
}

static int atbm_direct_read_unlock_v1(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val){
	return atbm_ahb_read_unlock_v1(hw_priv, addr, val, sizeof(atbm_uint32));
}

static int atbm_direct_write_unlock_v1(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 val){
	return atbm_ahb_write_unlock_v1(hw_priv, addr, &val, sizeof(atbm_uint32));
}


static int atbm_get_rx_len_v0(struct atbmwifi_common *hw_priv, int next_len){
	atbm_uint32 read_len;
	atbm_uint32 read_len_lsb = 0;
	atbm_uint32 read_len_msb = 0;
	atbm_uint16 ctrl_reg = (atbm_uint16)next_len;

	if ((ctrl_reg == 0) && ATBM_WARN_ON(atbm_bh_read_ctrl_reg(
			hw_priv, &ctrl_reg))){			
		return 0;
	}

	read_len_lsb = (ctrl_reg & ATBM_HIFREG_CONT_NEXT_LEN_LSB_MASK)*2;
	read_len_msb = (ctrl_reg & ATBM_HIFREG_CONT_NEXT_LEN_MSB_MASK)*2;
	read_len=((read_len_msb>>2)+read_len_lsb);

	if(read_len == 0){
		return 0;
	}

	if (ATBM_WARN_ON(((read_len < sizeof(struct wsm_hdr)) || (read_len > EFFECTIVE_BUF_SIZE)))) {
		wifi_printk(WIFI_ALWAYS,"Invalid read len: %d",
			read_len);
		return 0;
	}
	return (read_len + 2);
}

static int atbm_get_rx_len_v1(struct atbmwifi_common *hw_priv, int next_len){
	int read_len;

	if(next_len){
		wifi_printk(WIFI_DBG_MSG, "retrive rx len:%d\n", next_len);
		return next_len;
	}

	if (ATBM_WARN_ON(atbm_reg_read_32(hw_priv, ATBM_6X65S_SDIO_FN1_RX_TOT_LEN,&read_len))){
		return 0;
	}

	wifi_printk(WIFI_DBG_MSG, "raw rx len:%d\n", read_len);
	return read_len;
}

static int atbm_get_tx_free_buffers_v0(struct atbmwifi_common *hw_priv){
	int ret;
	atbm_uint32 hw_xmited;

	ret = atbm_direct_read_reg_32(hw_priv,hw_priv->wsm_caps.NumOfHwXmitedAddr,&hw_xmited);	
	if(ret){
		return 0;
	}
	return (hw_priv->wsm_caps.numInpChBufs) - (hw_priv->n_xmits-hw_xmited);
}

static int atbm_get_tx_free_buffers_v1(struct atbmwifi_common *hw_priv){
	int ret;
	atbm_uint32 channels;
	atbm_uint16 free_buffers;
	
	ret = atbm_reg_read_32(hw_priv, ATBM_6X65S_SDIO_FN1_TX_TOT_LEN, &channels);
	if(ret){
		return 0;
	}
	free_buffers = (atbm_uint16)((atbm_uint16)(channels>>16) - (atbm_uint16)hw_priv->n_xmits);

	return (int)free_buffers;
}

static int __atbm_data_read_v0(struct atbmwifi_common *hw_priv, atbm_uint16 addr,
				atbm_void *buf, atbm_uint32 buf_len, int buf_id)
{
	atbm_uint16 addr_sdio;
	atbm_uint32 sdio_reg_addr_17bit ;

	/* Check if buffer is aligned to 4 byte boundary */
	if (ATBM_WARN_ON(((unsigned long)buf & 3) && (buf_len > 4))) {
		wifi_printk(WIFI_IF,
			   "%s: buffer is not aligned.\n", __FUNCTION__);
		return -ATBM_EINVAL;
	}

	/* Convert to SDIO Register Address */
	addr_sdio = SPI_REG_ADDR_TO_SDIO(addr);
	sdio_reg_addr_17bit = SDIO_ADDR17BIT_V0(buf_id + 1, hw_priv->chip_ver, 0, addr_sdio);

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	hw_priv->buf_id_rx_offset = 1;
	return __atbm_sdio_memcpy_fromio(hw_priv->sbus_priv->func,
						  buf,
						  sdio_reg_addr_17bit,
						  buf_len);
}

static int __atbm_data_read_v1(struct atbmwifi_common *hw_priv, atbm_uint16 addr,
				atbm_void *buf, atbm_uint32 buf_len, int buf_id)
{
	atbm_uint32 sdio_reg_addr_17bit ;
	int ret;

	/* Check if buffer is aligned to 4 byte boundary */
	if (ATBM_WARN_ON(((unsigned long)buf & 3) && (buf_len > 4))) {
		wifi_printk(WIFI_IF,
			   "%s: buffer is not aligned.\n", __FUNCTION__);
		return -ATBM_EINVAL;
	}

	/* Convert to SDIO Register Address */
	sdio_reg_addr_17bit = SDIO_ADDR17BIT_V1(buf_id, 1, 0, 0);

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	ret = __atbm_sdio_memcpy_fromio(hw_priv->sbus_priv->func,
						  buf,
						  sdio_reg_addr_17bit,
						  buf_len);
	hw_priv->buf_id_rx_offset = atbm_le16_to_cpu(((atbm_uint16 *)buf)[buf_len/2 - 2]);
	return ret;
}

static int __atbm_data_write_v0(struct atbmwifi_common *hw_priv, atbm_uint16 addr,
				const void *buf, atbm_uint32 buf_len, int buf_id)
{
	atbm_uint16 addr_sdio;
	atbm_uint32 sdio_reg_addr_17bit ;


	/* Convert to SDIO Register Address */
	addr_sdio = SPI_REG_ADDR_TO_SDIO(addr);
	sdio_reg_addr_17bit = SDIO_ADDR17BIT_V0(buf_id, 0, 0, addr_sdio);

	ATBM_BUG_ON(!hw_priv->sbus_ops);

	return __atbm_sdio_memcpy_toio(hw_priv->sbus_priv->func,
						sdio_reg_addr_17bit,
						(atbm_void *)buf, buf_len);
}

static int __atbm_data_write_v1(struct atbmwifi_common *hw_priv, atbm_uint16 addr,
				const void *buf, atbm_uint32 buf_len, int buf_id)
{
	atbm_uint16 offset;
	atbm_uint32 sdio_reg_addr_17bit ;
	int ret;

	/* Convert to SDIO Register Address */
	offset = ATBM_SDIO_SPI_DATA_LEN_OFFSET_TO_SDIO(buf_len%ATBM_SDIO_BLOCK_SIZE);
	sdio_reg_addr_17bit = SDIO_ADDR17BIT_V1(buf_id, 1, 0, offset);

	ATBM_BUG_ON(!hw_priv->sbus_ops);

	return __atbm_sdio_memcpy_toio(hw_priv->sbus_priv->func,
						sdio_reg_addr_17bit,
						(atbm_void *)buf, ATBM_ALIGN(buf_len,ATBM_SDIO_BLOCK_SIZE));
}

#if ATBM_SUPPORT_SG
static int __atbm_data_write_sgs(struct atbmwifi_common *hw_priv, atbm_uint16 addr,
				const void **buf, atbm_uint32 *buf_len, int buf_id, int count)
{
	atbm_uint16 addr_sdio;
	atbm_uint32 sdio_reg_addr_17bit ;


	/* Convert to SDIO Register Address */
	addr_sdio = SPI_REG_ADDR_TO_SDIO(addr);
	sdio_reg_addr_17bit = SDIO_ADDR17BIT(buf_id, 0, 0, addr_sdio);

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	return __atbm_sdio_memcpy_toio_sgs(hw_priv->sbus_priv->func,
						sdio_reg_addr_17bit,
						buf, (int *)buf_len, count);
}
#endif

//sdio
#if ATBM_SUPPORT_SG
int atbm_data_write_sgs(struct atbmwifi_common *hw_priv, const atbm_void **buf,
			atbm_size_t *buf_len)
{
	int ret = 0, retry = 1;
	int buf_id_tx;

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	buf_id_tx = hw_priv->buf_id_tx;
	while (retry <= MAX_RETRY) {
		ret = __atbm_data_write_sgs(hw_priv,
				ATBM_HIFREG_IN_OUT_QUEUE_REG_ID, buf,
				(atbm_uint32 *)buf_len, buf_id_tx, hw_priv->buf_id_tx_offset);
			if (!ret) {		
//				buf_id_tx =  ((buf_id_tx + 1) >= hw_priv->wsm_caps_numInpChBufs)? 0: (buf_id_tx + 1) ;
				buf_id_tx = (buf_id_tx+hw_priv->buf_id_tx_offset)&(0x3f);
				hw_priv->buf_id_tx = buf_id_tx;
				break;
			} else {
				retry++;
				atbm_mdelay(1);
			wifi_printk(WIFI_ALWAYS, "%s,%d,error :[%d]\n",
					__FUNCTION__, __LINE__, ret);
		}
	}
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}
#endif


struct atbm_sdio_hw_ops atbm_hw_ops[2] = {
	//for cronus, cronus_lite, oceanus
	{
		.sdio_ahb_read_unlock = atbm_ahb_read_unlock_v0,
		.sdio_ahb_write_unlock = atbm_ahb_write_unlock_v0,
		.sdio_direct_read_unlock = atbm_direct_read_unlock_v0,
		.sdio_direct_write_unlock = atbm_direct_write_unlock_v0,
		.sdio_get_rx_len = atbm_get_rx_len_v0,
		.sdio_get_tx_free_buffers = atbm_get_tx_free_buffers_v0,
		.sdio_data_read = __atbm_data_read_v0,
		.sdio_data_write = __atbm_data_write_v0,
	},
	//for coeus
	{
		.sdio_ahb_read_unlock = atbm_ahb_read_unlock_v1,
		.sdio_ahb_write_unlock = atbm_ahb_write_unlock_v1,
		.sdio_direct_read_unlock = atbm_direct_read_unlock_v1,
		.sdio_direct_write_unlock = atbm_direct_write_unlock_v1,
		.sdio_get_rx_len = atbm_get_rx_len_v1,
		.sdio_get_tx_free_buffers = atbm_get_tx_free_buffers_v1,
		.sdio_data_read = __atbm_data_read_v1,
		.sdio_data_write = __atbm_data_write_v1,
	},
};

int atbm_ahb_read_unlock(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
				atbm_void *buf, atbm_size_t buf_len)
{
	return atbm_hw_ops[hw_priv->chip_ver].sdio_ahb_read_unlock(hw_priv, addr, buf, buf_len);
}

int atbm_ahb_write_unlock(struct atbmwifi_common *hw_priv, atbm_uint32 addr, const atbm_void *buf,
						atbm_uint32 buf_len){
	return atbm_hw_ops[hw_priv->chip_ver].sdio_ahb_write_unlock(hw_priv, addr, buf, buf_len);
}

int atbm_ahb_read(struct atbmwifi_common *hw_priv, atbm_uint32 addr,
				atbm_void *buf, atbm_size_t buf_len)
{
	int ret;
	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	ret = atbm_ahb_read_unlock(hw_priv, addr, buf, buf_len);
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}

int atbm_ahb_write(struct atbmwifi_common *hw_priv, atbm_uint32 addr, const atbm_void *buf,
						atbm_uint32 buf_len){
	int ret;

	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	ret = atbm_ahb_write_unlock(hw_priv, addr, buf, buf_len);
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}

int atbm_ahb_read_32(struct atbmwifi_common *hw_priv,
				atbm_uint32 addr, atbm_uint32 *val)
{
	return atbm_ahb_read(hw_priv, addr, val, sizeof(int));
}

int atbm_ahb_write_32(struct atbmwifi_common *hw_priv,
				atbm_uint32 addr, atbm_uint32 val)
{
	int ret = atbm_ahb_write(hw_priv, addr, &val, sizeof(val));
	if((addr&WRITE_32K_ADDR_MSK)==WRITE_32K_ADDR)
	{
		atbm_mdelay(10);
	}
	return ret;
}


int atbm_fw_write(struct atbmwifi_common *priv, atbm_uint32 addr, const atbm_void *buf,
						atbm_uint32 buf_len)
{
	return atbm_ahb_write(priv,  addr, buf, buf_len);		
}

int atbm_direct_read_unlock(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val)
{

	return atbm_hw_ops[hw_priv->chip_ver].sdio_direct_read_unlock(hw_priv, addr, val);
}

int atbm_direct_write_unlock(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 val)
{
	return atbm_hw_ops[hw_priv->chip_ver].sdio_direct_write_unlock(hw_priv, addr, val);
}


int atbm_direct_read_reg_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val)
{
	int ret;

	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	ret = atbm_hw_ops[hw_priv->chip_ver].sdio_direct_read_unlock(hw_priv, addr, val);
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}


int atbm_direct_write_reg_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 val)
{
	int ret;

	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	ret = atbm_hw_ops[hw_priv->chip_ver].sdio_direct_write_unlock(hw_priv, addr, val);
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}

int atbm_sdio_get_rx_len(struct atbmwifi_common *hw_priv, int next_len){
	return atbm_hw_ops[hw_priv->chip_ver].sdio_get_rx_len(hw_priv, next_len);
}

int atbm_sdio_get_tx_free_buffers(struct atbmwifi_common *hw_priv){
	return atbm_hw_ops[hw_priv->chip_ver].sdio_get_tx_free_buffers(hw_priv);
}

int atbm_data_read(struct atbmwifi_common *hw_priv, atbm_void *buf, atbm_uint32 buf_len){
	int ret = 0, retry = 1;
	ATBM_BUG_ON(!hw_priv->sbus_ops);
	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
	{
		int buf_id_rx = hw_priv->buf_id_rx;
		while (retry <= MAX_RETRY) {
			ret = atbm_hw_ops[hw_priv->chip_ver].sdio_data_read(hw_priv,
					ATBM_HIFREG_IN_OUT_QUEUE_REG_ID, buf,
					buf_len, buf_id_rx);			
			if (!ret) {
				buf_id_rx = (buf_id_rx + hw_priv->buf_id_rx_offset) % hw_priv->chip_priv->rx_channel_num;
				hw_priv->buf_id_rx = buf_id_rx;
				break;
			} else {
				retry++;
				atbm_mdelay(1);
				wifi_printk(WIFI_ALWAYS, "%s,error :[%d]\n",
						__FUNCTION__, ret);
			}
		}
	}
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}
int atbm_data_write(struct atbmwifi_common *hw_priv, const atbm_void *buf, atbm_size_t buf_len){
	int ret = 0, retry = 1;
	int buf_id_tx;

	ATBM_BUG_ON(!hw_priv->sbus_ops);
	hw_priv->sbus_ops->lock(hw_priv->sbus_priv);

	buf_id_tx = hw_priv->buf_id_tx;
	while (retry <= MAX_RETRY) {
		ret = atbm_hw_ops[hw_priv->chip_ver].sdio_data_write(hw_priv,
				ATBM_HIFREG_IN_OUT_QUEUE_REG_ID, buf,
				buf_len, buf_id_tx);
		if (!ret) { 	
//				buf_id_tx =  ((buf_id_tx + 1) >= hw_priv->wsm_caps_numInpChBufs)? 0: (buf_id_tx + 1) ;
			buf_id_tx = (buf_id_tx+hw_priv->buf_id_tx_offset) % hw_priv->chip_priv->tx_channel_num;
			hw_priv->buf_id_tx = buf_id_tx;
			break;
		} else {
			retry++;
			atbm_mdelay(1);
			wifi_printk(WIFI_ALWAYS, "%s,%d,error :[%d]\n",
				__FUNCTION__, __LINE__, ret);
		}
	}
	hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
	return ret;
}

//just ARESB have this function
//used this function to clear sdio rtl bug register
// if not do this sdio direct mode (wr/read reigster) will not work
int atbm_data_force_write(struct atbmwifi_common *hw_priv, const void *buf,
                        int buf_len)
{
        int ret = 0, retry = 1;
        int buf_id_tx;
        ATBM_BUG_ON(!hw_priv->sbus_ops);
        hw_priv->sbus_ops->lock(hw_priv->sbus_priv);
        buf_id_tx = ((hw_priv->buf_id_tx-1)&0x3f)+64;
		wifi_printk(WIFI_ALWAYS,"buf_id_tx =%d %s\n",buf_id_tx,__func__);
        while (retry <= MAX_RETRY) {
                ret = atbm_hw_ops[hw_priv->chip_ver].sdio_data_write(hw_priv,
                                ATBM_HIFREG_IN_OUT_QUEUE_REG_ID, buf,
                                buf_len, buf_id_tx);
                if (!ret) {				
                        buf_id_tx =  ((buf_id_tx + hw_priv->buf_id_tx_offset)&0x3f);
                        hw_priv->buf_id_tx = buf_id_tx;
                        break;
                } else {
                        retry++;
                        atbm_mdelay(1000);
                       
                }
        }
        hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);
        return ret;
}

int __atbm_irq_enable(struct atbmwifi_common *priv, int enable)
{
	atbm_uint32 val32;
	int ret;

	ret = __atbm_reg_read_32(priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
	if (ret < 0) {
		wifi_printk(WIFI_IF,"Can't read config register.\n");
		return ret;
	}

	if (enable){
		if(val32 & ATBM_HIFREG_CONF_IRQ_RDY_ENABLE)
			return ret;
		val32 |= ATBM_HIFREG_CONF_IRQ_RDY_ENABLE;

	}
	else {
		if((val32 & ATBM_HIFREG_CONF_IRQ_RDY_ENABLE)==0)
			return ret;
		val32 &= ~ATBM_HIFREG_CONF_IRQ_RDY_ENABLE;
	}
	ret = __atbm_reg_write_32(priv, ATBM_HIFREG_CONFIG_REG_ID, val32);
	if (ret < 0) {
		wifi_printk(WIFI_IF,"Can't write config register.\n");
		return ret;
	}
	return 0;
}

