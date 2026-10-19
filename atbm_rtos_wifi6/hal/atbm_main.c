/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#ifndef LINUX_OS
#include <string.h>
#include <stdio.h>
#endif
#include "atbm_hal.h"
#include "svn_version.h"
#if ATBM_SUPPORT_BRIDGE
#include "atbm_bridge.h"
#endif

#define CRONUS_USB_WIFI_FIRMWARE_NAME "firmware_cronus_usb_notxconfirm.h"
#define CRONUS_USB_COMBO_FIRMWARE_NAME "firmware_cronus_usb_combo_notxconfirm.h"
#define CRONUS_SDIO_WIFI_FIRMWARE_NAME "firmware_cronus_sdio_notxconfirm.h"
#define CRONUS_SDIO_COMBO_FIRMWARE_NAME "firmware_cronus_sdio_combo_notxconfirm.h"
#define CRONUS_LITE_USB_WIFI_FIRMWARE_NAME "firmware_cronus_lite_usb_notxconfirm.h"
#define CRONUS_LITE_USB_COMBO_FIRMWARE_NAME "firmware_cronus_lite_usb_combo_notxconfirm.h"
#define CRONUS_LITE_SDIO_WIFI_FIRMWARE_NAME "firmware_cronus_lite_sdio_notxconfirm.h"
#define CRONUS_LITE_SDIO_COMBO_FIRMWARE_NAME "firmware_cronus_lite_sdio_combo_notxconfirm.h"
#define OCEANUS_USB_WIFI_FIRMWARE_NAME "firmware_oceanus_usb_notxconfirm.h"
#define OCEANUS_USB_COMBO_FIRMWARE_NAME "firmware_oceanus_usb_combo_notxconfirm.h"
#define OCEANUS_SDIO_WIFI_FIRMWARE_NAME "firmware_oceanus_sdio_notxconfirm.h"
#define OCEANUS_SDIO_COMBO_FIRMWARE_NAME "firmware_oceanus_sdio_combo_notxconfirm.h"
#define COEUS_USB_WIFI_FIRMWARE_NAME "firmware_coeus_usb_notxconfirm.h"
#define COEUS_SDIO_WIFI_FIRMWARE_NAME "firmware_coeus_sdio_notxconfirm.h"

#define FIRM_FILE_NAME_INF_TYPE(alias,inf,type) alias##_##inf##_##type##_FIRMWARE_NAME

#if CONFIG_WIFI_BT_COMB
#define FIRMTYPE COMBO
#define FIRM_HEADR_NAME(alias) firmware_headr##alias##BLE
#define FIRM_ICCM_NAME(alias) fw_code##alias##BLE
#define FIRM_DCCM_NAME(alias) fw_data##alias##BLE
#define FIRM_SRAM_NAME(alias) fw_sram##alias##BLE
#define FIRM_FILE_NAME_INF(alias,inf) FIRM_FILE_NAME_INF_TYPE(alias,inf,COMBO)
#else
#define FIRMTYPE WIFI
#define FIRM_HEADR_NAME(alias) firmware_headr##alias
#define FIRM_ICCM_NAME(alias) fw_code##alias
#define FIRM_DCCM_NAME(alias) fw_data##alias
#define FIRM_SRAM_NAME(alias) fw_sram##alias
#define FIRM_FILE_NAME_INF(alias,inf) FIRM_FILE_NAME_INF_TYPE(alias,inf,WIFI)
#endif

#if ATBM_SDIO_BUS
#define FIRM_FILE_NAME(alias) FIRM_FILE_NAME_INF(alias,SDIO)
#elif ATBM_USB_BUS
#define FIRM_FILE_NAME(alias) FIRM_FILE_NAME_INF(alias,USB)
#endif

atbm_uint8 default_macaddr[6] = {0x00,0x00,0x11,0x32,0x43,0x69};
extern struct tcpip_opt lwip_tcp_opt;
struct atbmwifi_common g_hw_prv;
#if ATBM_SDIO_BUS
#define SET_SDIO_DOWNLOAD_BLOCKSIZE(hw_priv,type,bs) do { \
			hw_priv->sbus_ops->lock(hw_priv->sbus_priv);\
			ATBM_WARN_ON_FUNC(hw_priv->sbus_ops->set_block_size(hw_priv->sbus_priv,type,bs));\
			hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);  \
		}while(0)
#define INIT_SDIO_FUNCTION_4(func) atbm_sdio_check_init_func4(func)
#else
#define SET_SDIO_DOWNLOAD_BLOCKSIZE(hw_priv,type,bs) do{ }while(0)
#define INIT_SDIO_FUNCTION_4(func) (ATBM_NULL)
#endif

#define SET_SDIO_TRANSMIT_BLOCKSIZE(hw_priv) do{ \
			hw_priv->sbus_ops->lock(hw_priv->sbus_priv);\
			ATBM_WARN_ON_FUNC(hw_priv->sbus_ops->set_block_size(hw_priv->sbus_priv,0,ATBM_SDIO_BLOCK_SIZE));\
			hw_priv->sbus_ops->unlock(hw_priv->sbus_priv);\
	}while(0)

extern atbm_void atbm_get_efuse_data(struct atbmwifi_common *hw_priv);
extern atbm_void atbm_wifi_ticks_timer_init(atbm_void);
extern int atbm_wifi_add_interfaces(struct atbmwifi_common *hw_priv,enum atbm_nl80211_iftype iftype, char *if_name);
extern int atbm_wifi_remove_interfaces(struct atbmwifi_vif *priv);
extern atbm_int32 atbmwifi_enable_lmaclog(atbm_uint32 value );
extern atbm_void atbm_free_netdev(struct atbm_net_device * netdev);
extern 	int atbm_create_timerTask(struct atbmwifi_common *hw_priv);

static void atbm_dump_fw_regs(struct atbmwifi_common *hw_priv, const char *tag)
{
#if ATBM_SDIO_BUS
	atbm_uint32 val32 = 0;
	int ret;

	ret = atbm_reg_read_32(hw_priv, ATBM_HIFREG_CONFIG_REG_ID, &val32);
	wifi_printk(WIFI_ALWAYS, "atbm %s: cfg ret=%d val=0x%x\n", tag, ret, val32);
	ret = atbm_reg_read_32(hw_priv, 0x0014, &val32);
	wifi_printk(WIFI_ALWAYS, "atbm %s: fn1_reset ret=%d val=0x%x\n", tag, ret, val32);
	ret = atbm_ahb_read_32(hw_priv, 0x1610102c, &val32);
	wifi_printk(WIFI_ALWAYS, "atbm %s: 0x1610102c ret=%d val=0x%x\n", tag, ret, val32);
	ret = atbm_ahb_read_32(hw_priv, 0x161000c0, &val32);
	wifi_printk(WIFI_ALWAYS, "atbm %s: 0x161000c0 ret=%d val=0x%x\n", tag, ret, val32);
	ret = atbm_ahb_read_32(hw_priv, 0x161000ac, &val32);
	wifi_printk(WIFI_ALWAYS, "atbm %s: 0x161000ac ret=%d val=0x%x\n", tag, ret, val32);
#endif
}

struct atbmwifi_common *_atbmwifi_vifpriv_to_hwpriv(struct atbmwifi_vif *priv)
{
	return priv->hw_priv;
}
struct atbmwifi_vif *_atbmwifi_hwpriv_to_vifpriv(struct atbmwifi_common *hw_priv,int if_id)
{
	//struct atbmwifi_vif *vif;
	ATBM_WARN_ON_FUNC(-1 == if_id);
	if ((-1 == if_id) || (if_id >= ATBM_WIFI_MAX_VIFS)){
		return ATBM_NULL;
	}

	return hw_priv->vif_list[if_id];
}

static  int _atbmwifi_get_nr_hw_ifaces(struct atbmwifi_common *hw_priv)
{
	return 1;
}

/* TODO: use rates and channels from the device */
#define RATETAB_ENT(_rate, _rateid, _flags)		{_rate, _rateid, _flags}

/*
struct atbmwifi_ieee80211_rate {
atbm_uint16 bitrate;
atbm_uint8 hw_value;
atbm_uint8 rate_flag;
};

.bitrate	= (_rate),		\
.hw_value	= (_rateid),		\
.rate_flag	= (_flags),	\
*/

struct atbmwifi_ieee80211_rate atbmwifi_rates[ATBM_WIFI_RATE_SIZE] = {
	RATETAB_ENT(2,  0,   ATBM_IEEE80211_RT_BASIC|ATBM_IEEE80211_RT_11B),
	RATETAB_ENT(4,  1,   ATBM_IEEE80211_RT_BASIC|ATBM_IEEE80211_RT_11B),
	RATETAB_ENT(11,  2,   ATBM_IEEE80211_RT_BASIC|ATBM_IEEE80211_RT_11B),
	RATETAB_ENT(22, 3,   ATBM_IEEE80211_RT_BASIC|ATBM_IEEE80211_RT_11B),
	RATETAB_ENT(12,  6,   ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(18,  7,  ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(24, 8,  ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(36, 9,  ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(48, 10, ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(72, 11, ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(96, 12, ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(108, 13, ATBM_IEEE80211_RT_11G),
};

#if CONFIG_5G_SUPPORT
struct atbmwifi_ieee80211_rate atbmwifi_rates_5g[ATBM_WIFI_RATE_SIZE] = {	
	RATETAB_ENT(12,  6,   ATBM_IEEE80211_RT_BASIC|ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(18,  7,  ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(24, 8,  ATBM_IEEE80211_RT_BASIC|ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(36, 9,  ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(48, 10, ATBM_IEEE80211_RT_BASIC|ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(72, 11, ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(96, 12, ATBM_IEEE80211_RT_11G),
	RATETAB_ENT(108, 13, ATBM_IEEE80211_RT_11G),
};
#endif

//500k  unit
struct atbmwifi_ieee80211_rate atbm_mcs_rates[] = {
	RATETAB_ENT(13,  14, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(26, 15, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(39, 16, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(52, 17, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(78, 18, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(104, 19, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(117, 20, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(130, 21, ATBM_IEEE80211_TX_RC_MCS),
	RATETAB_ENT(12 , 22, ATBM_IEEE80211_TX_RC_MCS),
};
#define CHAN2G(_channel, _freq, _flags) {30, _channel, _flags, _freq}
#define CHAN5G(_channel, _flags) {				\
	.hw_value		= (_channel),			\
	.flags			= (_flags),			\
	.max_power		= 30,				\
}

const static struct atbmwifi_ieee80211_channel atbmwifi_2ghz_chantable_const[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

static struct atbmwifi_ieee80211_channel atbmwifi_2ghz_chantable[14] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

#if CONFIG_5G_SUPPORT
static struct atbmwifi_ieee80211_channel atbm_5ghz_chantable[] = {
	CHAN5G(34, 0),		CHAN5G(36, 0),
	CHAN5G(38, 0),		CHAN5G(40, 0),
	CHAN5G(42, 0),		CHAN5G(44, 0),
	CHAN5G(46, 0),		CHAN5G(48, 0),
	CHAN5G(52, 0),		CHAN5G(56, 0),
	CHAN5G(60, 0),		CHAN5G(64, 0),
	CHAN5G(100, 0),		CHAN5G(104, 0),
	CHAN5G(108, 0),		CHAN5G(112, 0),
	CHAN5G(116, 0),		CHAN5G(120, 0),
	CHAN5G(124, 0),		CHAN5G(128, 0),
	CHAN5G(132, 0),		CHAN5G(136, 0),
	CHAN5G(140, 0),		CHAN5G(149, 0),
	CHAN5G(153, 0),		CHAN5G(157, 0),
	CHAN5G(161, 0),		CHAN5G(165, 0),
#if 0
	CHAN5G(184, 0),		CHAN5G(188, 0),
	CHAN5G(192, 0),		CHAN5G(196, 0),
	CHAN5G(200, 0),		CHAN5G(204, 0),
	CHAN5G(208, 0),		CHAN5G(212, 0),
	CHAN5G(216, 0),
#endif
};
#endif

#if CONFIG_HE
#if CONFIG_ATBM_HE_BEAMFORM
#define ATBM_IEEE80211_HE_PHY_CAP6_BEAMFORM_ENABLE ATBM_IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMING_FB |\
				ATBM_IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB|\
				ATBM_IEEE80211_HE_PHY_CAP6_TRIG_CQI_FB
				
#define ATBM_IEEE80211_HE_PHY_CAP4_BEAMFORM_ENABLE ATBM_IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE |\
					ATBM_IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4
#else
#define ATBM_IEEE80211_HE_PHY_CAP6_BEAMFORM_ENABLE 0
#define ATBM_IEEE80211_HE_PHY_CAP4_BEAMFORM_ENABLE 0
#endif  //CONFIG_ATBM_HE_BEAMFORM
#endif

struct atbmwifi_ieee80211_supported_band atbmwifi_band_2ghz = {
	.channels = atbmwifi_2ghz_chantable,
	.n_channels = ATBM_ARRAY_SIZE(atbmwifi_2ghz_chantable),
	.bitrates =atbmwifi_g_rates,
	.n_bitrates = atbmwifi_g_rates_size,
	.band = ATBM_IEEE80211_BAND_2GHZ,
	.ht_cap = {
		.cap =  
#if BW_40M_SUPPORT
				ATBM_IEEE80211_HT_CAP_SUP_WIDTH_20_40|
				ATBM_IEEE80211_HT_CAP_DSSSCCK40|
				ATBM_IEEE80211_HT_CAP_SGI_40 |
#endif
				ATBM_IEEE80211_HT_CAP_SGI_20|
				(1 << ATBM_IEEE80211_HT_CAP_RX_STBC_SHIFT)|
				ATBM_IEEE80211_HT_CAP_LDPC_CODING,
		.ht_supported = 1,
		.ampdu_factor = ATBM_IEEE80211_HT_MAX_AMPDU_32K,
		.ampdu_density = ATBM_IEEE80211_HT_MPDU_DENSITY_NONE,
		.mcs.rx_mask[0] = 0xFF,
		.mcs.rx_highest = 0,
		.mcs.tx_params = ATBM_IEEE80211_HT_MCS_TX_DEFINED,
	},
#if CONFIG_VHT
	.vht_cap = 
	{
	   .vht_supported = ATBM_TRUE,
	   .cap = ATBM_IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895 |
		   ATBM_IEEE80211_VHT_CAP_RXLDPC |	
		   ATBM_IEEE80211_VHT_CAP_RXSTBC_1 |
		   ATBM_IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE |
		   ATBM_IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
		   ATBM_IEEE80211_VHT_CAP_HTC_VHT |
		   (1 << ATBM_IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT) |
		   /*IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK |*/
		   (2 << ATBM_IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT) |
		   ATBM_IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN |
		   ATBM_IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN |
		   0,
	   .vht_mcs = {
		   .rx_mcs_map = atbm_cpu_to_le16(ATBM_IEEE80211_VHT_MCS_SUPPORT_0_9 << 0 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 2 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 4 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 6 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 8 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 10 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 12 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 14),
		   .rx_highest = 0,
		   .tx_mcs_map =  atbm_cpu_to_le16(ATBM_IEEE80211_VHT_MCS_SUPPORT_0_9 << 0 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 2 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 4 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 6 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 8 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 10 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 12 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 14),
		   .tx_highest = 0,
	   },
	},
#endif
#if CONFIG_HE
	.iftype_data = {
		/* TODO: should we support other types, e.g., P2P?*/
		.types_mask = ATBM_BIT(ATBM_NL80211_IFTYPE_STATION) |
				  ATBM_BIT(ATBM_NL80211_IFTYPE_AP),
		.he_cap = {
			.has_he = ATBM_TRUE,
			.he_cap_elem = {
				.mac_cap_info[0] =
					ATBM_IEEE80211_HE_MAC_CAP0_HTC_HE,
				.mac_cap_info[1] =
					ATBM_IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_16US |
					ATBM_IEEE80211_HE_MAC_CAP1_MULTI_TID_AGG_RX_QOS_1,
				.mac_cap_info[2] =
					ATBM_IEEE80211_HE_MAC_CAP2_BSR |
					ATBM_IEEE80211_HE_MAC_CAP2_MU_CASCADING ,/*|
					ATBM_IEEE80211_HE_MAC_CAP2_ACK_EN,*/
				.mac_cap_info[3] =
					ATBM_IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_EXT_0,
				.mac_cap_info[4] = ATBM_IEEE80211_HE_MAC_CAP4_AMSDU_IN_AMPDU,
				.phy_cap_info[0] = 
					ATBM_IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G|
	                ATBM_IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_RU_MAPPING_IN_2G,
				.phy_cap_info[1] =
					ATBM_IEEE80211_HE_PHY_CAP1_DEVICE_CLASS_A |
					ATBM_IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD,
				.phy_cap_info[2] =
					ATBM_IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
					ATBM_IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ,
					//ATBM_IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
					//ATBM_IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO,
				.phy_cap_info[3] = ATBM_IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
					ATBM_IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM,
				.phy_cap_info[4] = ATBM_IEEE80211_HE_PHY_CAP4_BEAMFORM_ENABLE /*IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE |
						ATBM_IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4*/,
				.mac_cap_info[5] = ATBM_IEEE80211_HE_MAC_CAP5_OM_CTRL_UL_MU_DATA_DIS_RX,
				.phy_cap_info[6] = ATBM_IEEE80211_HE_PHY_CAP6_BEAMFORM_ENABLE,
					//ATBM_IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE,
					//ATBM_IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO,
				.phy_cap_info[7] =ATBM_IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI,
				.phy_cap_info[8] =ATBM_IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI |
					ATBM_IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G|
					ATBM_IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_484,
				.phy_cap_info[9] =	ATBM_IEEE80211_HE_PHY_CAP9_NON_TRIGGERED_CQI_FEEDBACK |
				ATBM_IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU|
					ATBM_IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU|
					ATBM_IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB|
					ATBM_IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB|
					ATBM_IEEE80211_HE_PHY_CAP9_NOMIMAL_PKT_PADDING_8US,

				/*
				 * Leave all the other PHY capability bytes
				 * unset, as DCM, beam forming, RU and PPE
				 * threshold information are not supported
				 */
			},
			.he_mcs_nss_supp = {
				.rx_mcs_80 = atbm_cpu_to_le16(0xfffe),
				.tx_mcs_80 = atbm_cpu_to_le16(0xfffe),
				.rx_mcs_160 = atbm_cpu_to_le16(0xffff),
				.tx_mcs_160 = atbm_cpu_to_le16(0xffff),
				.rx_mcs_80p80 = atbm_cpu_to_le16(0xffff),
				.tx_mcs_80p80 = atbm_cpu_to_le16(0xffff),
			},
			.ppe_thres[0] = 0,
		},
	},
#endif
};


#if CONFIG_5G_SUPPORT
static struct atbmwifi_ieee80211_supported_band atbmwifi_band_5ghz = {
	.channels = atbm_5ghz_chantable,
	.n_channels = ATBM_ARRAY_SIZE(atbm_5ghz_chantable),
	.bitrates = atbmwifi_a_rates,
	.n_bitrates = atbmwifi_a_rates_size,
	.band = ATBM_IEEE80211_BAND_5GHZ,
	.ht_cap = {
		.cap =	
#if BW_40M_SUPPORT
				ATBM_IEEE80211_HT_CAP_SUP_WIDTH_20_40|
				ATBM_IEEE80211_HT_CAP_DSSSCCK40|
				ATBM_IEEE80211_HT_CAP_SGI_40 |
#endif
				ATBM_IEEE80211_HT_CAP_SGI_20|
				(1 << ATBM_IEEE80211_HT_CAP_RX_STBC_SHIFT)|
				ATBM_IEEE80211_HT_CAP_LDPC_CODING,
		.ht_supported = 1,
		.ampdu_factor = ATBM_IEEE80211_HT_MAX_AMPDU_8K,
		.ampdu_density = ATBM_IEEE80211_HT_MPDU_DENSITY_NONE,
		.mcs = {
			.rx_mask[0] = 0xFF,
			.rx_highest = __atbm_cpu_to_le16(0),
			.tx_params = ATBM_IEEE80211_HT_MCS_TX_DEFINED,
		},
	},
#if CONFIG_VHT
	.vht_cap = 
	{
	   .vht_supported = ATBM_TRUE,
	   .cap = ATBM_IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895 |
		   ATBM_IEEE80211_VHT_CAP_RXLDPC |	
		   ATBM_IEEE80211_VHT_CAP_RXSTBC_1 |
		   ATBM_IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE |
		   ATBM_IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
		   ATBM_IEEE80211_VHT_CAP_HTC_VHT |
		   (1 << ATBM_IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT) |
		   /*IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK |*/
		   (2 << ATBM_IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT) |
		   ATBM_IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN |
		   ATBM_IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN |
		   0,
	   .vht_mcs = {
		   .rx_mcs_map = atbm_cpu_to_le16(ATBM_IEEE80211_VHT_MCS_SUPPORT_0_9 << 0 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 2 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 4 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 6 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 8 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 10 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 12 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 14),
		   .rx_highest = 0,
		   .tx_mcs_map =  atbm_cpu_to_le16(ATBM_IEEE80211_VHT_MCS_SUPPORT_0_9 << 0 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 2 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 4 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 6 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 8 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 10 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 12 |
		   ATBM_IEEE80211_VHT_MCS_NOT_SUPPORTED << 14),
		   .tx_highest = 0,
	   },
	},
#endif
#if CONFIG_HE
	.iftype_data = {
		/* TODO: should we support other types, e.g., P2P?*/
		.types_mask = ATBM_BIT(ATBM_NL80211_IFTYPE_STATION) |
				  ATBM_BIT(ATBM_NL80211_IFTYPE_AP),
		.he_cap = {
			.has_he = ATBM_TRUE,
			.he_cap_elem = {
				.mac_cap_info[0] =
					ATBM_IEEE80211_HE_MAC_CAP0_HTC_HE
					,
				.mac_cap_info[1] =
					ATBM_IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_16US |
					ATBM_IEEE80211_HE_MAC_CAP1_MULTI_TID_AGG_RX_QOS_1,
				.mac_cap_info[2] =
					ATBM_IEEE80211_HE_MAC_CAP2_BSR |
					ATBM_IEEE80211_HE_MAC_CAP2_MU_CASCADING ,/*|
					ATBM_IEEE80211_HE_MAC_CAP2_ACK_EN,*/
				.mac_cap_info[3] =
					ATBM_IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_EXT_0,
				.mac_cap_info[4] = ATBM_IEEE80211_HE_MAC_CAP4_AMSDU_IN_AMPDU,
				.phy_cap_info[0] = 
					ATBM_IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G,
				.phy_cap_info[1] =
					ATBM_IEEE80211_HE_PHY_CAP1_DEVICE_CLASS_A |
					ATBM_IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD,
				.phy_cap_info[2] =
					ATBM_IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
					ATBM_IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ,
					//ATBM_IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
					//ATBM_IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO,
				.phy_cap_info[3] = ATBM_IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
					ATBM_IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM,
				.phy_cap_info[4] = ATBM_IEEE80211_HE_PHY_CAP4_BEAMFORM_ENABLE /*IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE |
						ATBM_IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4*/,
				.phy_cap_info[6] = ATBM_IEEE80211_HE_PHY_CAP6_BEAMFORM_ENABLE|
					ATBM_IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE,
					//ATBM_IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO,
				.phy_cap_info[7] =ATBM_IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI,
				.phy_cap_info[8] =ATBM_IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI |
					//ATBM_IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G|
					ATBM_IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_484,
				.phy_cap_info[9] =	ATBM_IEEE80211_HE_PHY_CAP9_NON_TRIGGERED_CQI_FEEDBACK |
				ATBM_IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU|
					ATBM_IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU|
					//ATBM_IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB|
					ATBM_IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB|
					ATBM_IEEE80211_HE_PHY_CAP9_NOMIMAL_PKT_PADDING_16US,

				/*
				 * Leave all the other PHY capability bytes
				 * unset, as DCM, beam forming, RU and PPE
				 * threshold information are not supported
				 */
			},
			.he_mcs_nss_supp = {
				.rx_mcs_80 = atbm_cpu_to_le16(0xfffc),
				.tx_mcs_80 = atbm_cpu_to_le16(0xfffc),
				.rx_mcs_160 = atbm_cpu_to_le16(0xffff),
				.tx_mcs_160 = atbm_cpu_to_le16(0xffff),
				.rx_mcs_80p80 = atbm_cpu_to_le16(0xffff),
				.tx_mcs_80p80 = atbm_cpu_to_le16(0xffff),
			},
			.ppe_thres[0] =0x39,
			.ppe_thres[1] =0x1c,
			.ppe_thres[2] =0xc7,
			.ppe_thres[3] =0x71,
			.ppe_thres[4] =0x1c,
			.ppe_thres[5] =0x07,
		},
	},
#endif
};
#endif

#if SUPPORT_CRONUS
#include FIRM_FILE_NAME(CRONUS)
struct atbm_chip_priv chip_priv_cronus = {
	.support_5g = 0,
	.support_vht = 0,
	.disable_tb = 1,
	.chip_cat = CRONUS_CATEGORY,
	.fw_altobeam = {
		.hdr = (struct firmware_headr *)FIRM_HEADR_NAME(),
		.fw_iccm = FIRM_ICCM_NAME(),
		.fw_dccm = FIRM_DCCM_NAME(),
		.fw_sram = FIRM_SRAM_NAME(),
	},
	.reset_reg = 0x16101000,
	.reset_bit = 0x100,
	.mem_access_bit = 0,
	.hif_version = ATBM_TXRX_OPS_V0,
	.download_bs = DOWNLOAD_BLOCK_SIZE_V0,
	.tx_channel_num = 64,
	.rx_channel_num = 4,
};
#endif

#if SUPPORT_CRONUS_LITE
#include FIRM_FILE_NAME(CRONUS_LITE)
struct atbm_chip_priv chip_priv_cronus_lite = {
	.support_5g = 0,
	.support_vht = 1,
	.disable_tb = 0,
	.chip_cat = CLITE_CATEGORY,
	.fw_altobeam = {
		.hdr = (struct firmware_headr *)FIRM_HEADR_NAME(CRONUS_LITE),
		.fw_iccm = FIRM_ICCM_NAME(CRONUS_LITE),
		.fw_dccm = FIRM_DCCM_NAME(CRONUS_LITE),
		.fw_sram = FIRM_SRAM_NAME(CRONUS_LITE),
	},
	.reset_reg = 0x16101000,
	.reset_bit = 0x100,
	.mem_access_bit = 0,
	.hif_version = ATBM_TXRX_OPS_V0,
	.download_bs = DOWNLOAD_BLOCK_SIZE_V0,
	.tx_channel_num = 64,
	.rx_channel_num = 4,
};
#endif

#if SUPPORT_OCEANUS
#include FIRM_FILE_NAME(OCEANUS)
struct atbm_chip_priv chip_priv_oceanus = {
	.support_5g = 1,
	.support_vht = 1,
	.disable_tb = 0,
	.chip_cat = OCEANUS_CATEGORY,
	.fw_altobeam = {
		.hdr = (struct firmware_headr *)FIRM_HEADR_NAME(OCEANUS),
		.fw_iccm = FIRM_ICCM_NAME(OCEANUS),
		.fw_dccm = FIRM_DCCM_NAME(OCEANUS),
		.fw_sram = FIRM_SRAM_NAME(OCEANUS),
	},
	.reset_reg = 0x16101000,
	.reset_bit = 0x100,
	.mem_access_bit = 0,
	.hif_version = ATBM_TXRX_OPS_V0,
	.download_bs = DOWNLOAD_BLOCK_SIZE_V0,
	.tx_channel_num = 64,
	.rx_channel_num = 4,
};
#endif

#if SUPPORT_COEUS
#include FIRM_FILE_NAME(COEUS)
struct atbm_chip_priv chip_priv_coeus = {
	.support_5g = 1,
	.support_vht = 1,
	.disable_tb = 0,
	.chip_cat = COEUS_CATEGORY,
	.fw_altobeam = {
		.hdr = (struct firmware_headr *)FIRM_HEADR_NAME(6165),
		.fw_iccm = FIRM_ICCM_NAME(6165),
		.fw_dccm = FIRM_DCCM_NAME(6165),
		.fw_sram = FIRM_SRAM_NAME(6165),
	},
	.reset_reg = 0x16100074,
	.reset_bit = 0x80000000,
	.mem_access_bit = 0x1000000,
	.hif_version = ATBM_TXRX_OPS_V1,
	.download_bs = DOWNLOAD_BLOCK_SIZE_V1,
	.tx_channel_num = 64,
	.rx_channel_num = 32,
};
#endif

void atbm_init_chip_priv(struct atbmwifi_common *hw_priv){
	atbm_uint32 chipver = 0;

	hw_priv->sbus_priv->cmd_func = INIT_SDIO_FUNCTION_4(hw_priv->sbus_priv->func);
	if(hw_priv->sbus_priv->cmd_func != ATBM_NULL){
		hw_priv->chip_ver = 1;
	}
	atbm_direct_read_reg_32(hw_priv,0x0acc017c,&chipver);
    chipver&=0xff;
	hw_priv->chip_id = chipver;
	wifi_printk(WIFI_ALWAYS,"atbm_init_chip_priv : chip_id = %x \n",hw_priv->chip_id);
	switch(chipver){
#if SUPPORT_CRONUS
		case 0x5a:
			hw_priv->chip_priv =  &chip_priv_cronus;
			break;
#endif
#if SUPPORT_CRONUS_LITE
		case 0x5c:
			hw_priv->chip_priv =  &chip_priv_cronus_lite;
			break;
#endif
#if SUPPORT_OCEANUS
		case 0x30:
			hw_priv->chip_priv = &chip_priv_oceanus;
			break;
#endif
#if SUPPORT_COEUS
		case 0x31:
			hw_priv->chip_priv = &chip_priv_coeus;
			break;
#endif
		default:
			ATBM_BUG_ON(1);
	}
}

/****************************************************************************/
int Atbmwifi_halEntry(struct sbus_priv *sbus)
{ 
	int Status;
	int if_id;
	int i;
	int ret = 0;

	struct atbmwifi_common *hw_priv;
	struct wsm_operational_mode mode;

	atbm_memset(&mode, 0, sizeof(struct wsm_operational_mode));
#if SUPPORT_LIGHT_SLEEP
	mode.power_mode = wsm_power_mode_quiescent;
#else
	mode.power_mode = wsm_power_mode_active;
#endif
	mode.disableMoreFlagUsage = ATBM_TRUE;
	wifi_printk(WIFI_ALWAYS,"atbm: Atbmwifi_halEntry() <===\n");	
	/*Init static and global struct */
	net_device_ops_init();

	hw_priv = &g_hw_prv;
	atbm_memset(&g_hw_prv,0 ,sizeof(struct atbmwifi_common));
#if ATBM_USB_BUS
	hw_priv->sbus_ops = &atbm_usb_sbus_ops;
#else	
	hw_priv->sbus_ops = &atbm_sdio_sbus_ops;
#endif
	hw_priv->sbus_priv = sbus;
	sbus->core = hw_priv;
	///////////////////////hw_priv;////////
	/*Atbm work queue Task*/
	atbm_create_workqueue(hw_priv);
	/*Atbm timer Task*/
	atbm_create_timerTask(hw_priv);
	atbm_os_mutexLockInit(&hw_priv->wsm_cmd_mux);
	ATBM_INIT_LIST_HEAD(&hw_priv->event_queue); 
	ATBM_INIT_LIST_HEAD(&hw_priv->tx_urb_cmp); 												\
	atbm_spin_lock_init(&hw_priv->event_queue_lock); 	
	atbm_spin_lock_init(&hw_priv->tx_com_lock); 	
	atbm_spin_lock_init(&hw_priv->rx_com_lock); 	
	atbm_skb_queue_head_init(&hw_priv->rx_frame_queue);
	atbm_skb_queue_head_init(&hw_priv->rx_frame_free);
	atbm_skb_queue_head_init(&hw_priv->tx_frame_queue);
	atbm_skb_queue_head_init(&hw_priv->tx_frame_free);
#if ATBM_SDIO_BUS
	atbm_os_init_waitevent(&hw_priv->wsm_synchanl_done);
	hw_priv->wsm_sync_channl=atbm_init_work(hw_priv, wsm_sync_channl_reset,hw_priv);
#endif
#if CONFIG_ATBM_SUPPORT_CSA
	hw_priv->csa_work = atbm_init_work(hw_priv, atbm_ieee80211_csa_process_work, hw_priv);
	ATBM_INIT_LIST_HEAD(&hw_priv->csa_pending);
	ATBM_INIT_LIST_HEAD(&hw_priv->csa_req);
	hw_priv->next_csa_state = CSA_IDLE;
#endif
	if(atbmwifi_band_2ghz.ht_cap.cap & ATBM_IEEE80211_HT_CAP_SUP_WIDTH_20_40) {				
		hw_priv->channel_type = ATBM_NL80211_CHAN_HT40PLUS;
	}
	else {
		hw_priv->channel_type = ATBM_NL80211_CHAN_HT20;
	}
	hw_priv->bands[ATBM_IEEE80211_BAND_2GHZ] = &atbmwifi_band_2ghz;
#if CONFIG_5G_SUPPORT
	hw_priv->bands[ATBM_IEEE80211_BAND_5GHZ] = &atbmwifi_band_5ghz;
#endif
	hw_priv->short_frame_max_tx_count  = TEST_SHORT_RETRY_NUM;
	hw_priv->long_frame_max_tx_count  = TEST_LONG_RETRY_NUM;

#if ATBM_PKG_REORDER
	hw_priv->ba_tid_tx_mask = ATBMWIFI__TX_BLOCK_ACK_ENABLED_FOR_ALL_TID;
#if CONFIG_WPA2_REINSTALL_CERTIFICATION
	//	#if wpa2 reinstall certification need close ampdu,because rx PN reorder
	hw_priv->ba_tid_rx_mask = ATBMWIFI__RX_BLOCK_ACK_DISABLED_FOR_ALL_TID;
#else //CONFIG_WPA2_REINSTALL_CERTIFICATION
	hw_priv->ba_tid_rx_mask = ATBMWIFI__TX_BLOCK_ACK_ENABLED_FOR_ALL_TID;
#endif //CONFIG_WPA2_REINSTALL_CERTIFICATION
#else //ATBM_PKG_REORDER
	hw_priv->ba_tid_tx_mask = ATBMWIFI__RX_BLOCK_ACK_DISABLED_FOR_ALL_TID;
	hw_priv->ba_tid_rx_mask = ATBMWIFI__RX_BLOCK_ACK_DISABLED_FOR_ALL_TID;
#endif //ATBM_PKG_REORDER
	hw_priv->max_rx_aggregation_subframes = 8;//IEEE80211_MAX_AMPDU_BUF;
	hw_priv->max_tx_aggregation_subframes = 8;//IEEE80211_MAX_AMPDU_BUF;
	atbm_spin_lock_init(&hw_priv->aggr_lock);
	hw_priv->wsm_cbc.scan_complete = atbmwifi_scan_complete_cb;
	hw_priv->wsm_cbc.suspend_resume = atbm_suspend_resume;
	/*Queue init*/
	wsm_buf_init(&hw_priv->wsm_cmd_buf);
	atbm_spin_lock_init(&hw_priv->wsm_cmd.lock);
	atbm_os_init_waitevent(&hw_priv->wsm_startup_done);
	/*Get chip specific config*/
	atbm_init_chip_priv(hw_priv);
	/*Set usb endpoint according to chip type*/
	SET_USB_EP_NUM(hw_priv);
	/*Register bh*/
	ret=atbm_register_bh(hw_priv);
	if (ret!=0){
		goto AtbmMain_ERR;
	}
	/*Set DownLoad firmware Blocksize*/
	SET_SDIO_DOWNLOAD_BLOCKSIZE(hw_priv, hw_priv->chip_ver, hw_priv->chip_priv->download_bs);
	/*Start download fw*/
	ret = atbm_load_firmware(hw_priv);
	if (ret){
		wifi_printk(WIFI_ALWAYS,"DownLoad FwErr,Pls check\n");
		goto AtbmMain_ERR;
	}
	/*Timer Init*/
	atbm_timer_init(hw_priv);

#if ATBM_SDIO_BUS
	/*Set Transimit Frame Blocksize*/
	SET_SDIO_TRANSMIT_BLOCKSIZE(hw_priv);
	hw_priv->init_done = 1;
	/* Register Interrupt Handler */
	ret = hw_priv->sbus_ops->irq_subscribe(hw_priv->sbus_priv,
		(sbus_irq_handler)atbm_irq_handler, hw_priv);
	if (ret < 0) {
		wifi_printk(WIFI_IF,
			"%s: can't register IRQ handler.\n", __FUNCTION__);
	}
#endif
__wait_start_up:
	if (atbm_os_wait_event_timeout(&hw_priv->wsm_startup_done,2*ATBM_HZ) < 0) {
		if(!hw_priv->wsm_caps.firmwareReady){
			wifi_printk(WIFI_ALWAYS,"wait_event_interruptible_timeout wsm_startup_done timeout ERROR !!\n");
			goto AtbmMain_ERR;
		}
	}
	else {
		if(!hw_priv->wsm_caps.firmwareReady){
			atbm_dump_fw_regs(hw_priv, "fw_not_ready");
			wifi_printk(WIFI_ALWAYS,"atbm: Atbmwifi_halEntry(), FW is not ready(%dms).\n", atbm_GetOsTimeMs());
			goto __wait_start_up;
		}
	}
	atbm_dump_fw_regs(hw_priv, "fw_ready");
	atbm_firmware_init_check(hw_priv);

	/*Queue stats init*/
	if (atbm_unlikely(atbmwifi_queue_stats_init(&hw_priv->tx_queue_stats,
			WLAN_LINK_ID_MAX,
			hw_priv))) {
		ret = -2;
		goto AtbmMain_ERR;
	}

	/*Queue init*/
	hw_priv->vif0_throttle = ATBM_WIFI_MAX_QUEUE_SZ-16;
	for (i = 0; i < 4; ++i) {
		if (atbm_unlikely(atbmwifi_queue_init(&hw_priv->tx_queue[i],
				&hw_priv->tx_queue_stats, i, ATBM_WIFI_MAX_QUEUE_SZ))) { 
			ret = -3;
			goto AtbmMain_ERR;
		}
	}

	for (if_id = 0; if_id < _atbmwifi_get_nr_hw_ifaces(hw_priv); if_id++) { 
		wsm_set_operational_mode(hw_priv, &mode, if_id);
		/* Enable multi-TX confirmation */
		//wsm_use_multi_tx_conf(hw_priv, ATBM_TRUE, if_id);
	}

	/*get mac addr from efuse*/
	atbm_get_mac_address(hw_priv);
	/*Get efuse other Information*/
	atbm_get_efuse_data(hw_priv);

	/*Timer list initial*/
	atbm_wifi_ticks_timer_init();
	/*Intial Support coutry */
	atbmwifi_ieee80211_channel_country(hw_priv,country_chinese);
	/*mac80211 stack control & initial*/
	atbmwifi_netstack_init(hw_priv);
	
	/*Init the two virtul interface,Here need intial two netDev*/
	/*1 Add Station interface */
	atbm_wifi_add_interfaces(hw_priv,ATBM_NL80211_IFTYPE_STATION,"wlan0");
	/*2 Add P2p Interface*/
	atbm_wifi_add_interfaces(hw_priv,ATBM_NL80211_IFTYPE_AP,"p2p0");
	/*MacAddr Set only one times*/
	atbmwifi_setup_mac(hw_priv);

	/* Intial End Open Lmac log*/
	atbmwifi_enable_lmaclog(0);

#if ATBM_SUPPORT_BRIDGE
	atbm_brpool_init(hw_priv);
#endif
#if CONFIG_WIFI_BT_COMB
	ieee80211_ble_dev_int(hw_priv);
	atbm_ble_start();
#endif
	return ret;
AtbmMain_ERR:
	wifi_printk(WIFI_ALWAYS,"atbm: Atbmwifi_halEntry() <===error\n");
	return ret;
}
atbm_void atbm_core_release(struct atbmwifi_common *hw_priv)
{
	struct atbmwifi_vif *priv;
	int queue_id;
	int i;

	atbm_for_each_vif(hw_priv,priv,i){
		if(priv== NULL){
			continue;
		}

		//tcp_opt->net_disable(priv->ndev);
		/*Flush the queue */
		//__atbm_flush(hw_priv, ATBM_TRUE, priv->if_id);
		if(priv->enabled){
			/*Free wpa supplicant & hostapd mem*/
			if(atbmwifi_is_sta_mode(priv->iftype))
				atbmwifi_stop_sta(priv);
			if(atbmwifi_is_ap_mode(priv->iftype))
				atbmwifi_stop_ap(priv);
		}
		/*Free netdev*/
		atbm_wifi_remove_interfaces(priv);
	}

	//hmac_rc_deinit();
	atbmwifi_netstack_deinit();
#if CONFIG_WIFI_BT_COMB
	atbm_ble_stop();
	ieee80211_ble_dev_deregister(hw_priv);
#endif
	/*Destory the atbm_bh|*/
	atbm_unregister_bh(hw_priv);
	/*Destory the atbm_workQueue*/
	atbm_destory_task_work(hw_priv);
	/*Destory the atbm_wpa_event*/
	atbmwifi_wpa_event_destory();
	/*Free atbm timer*/
	atbm_timer_free(hw_priv);

	/*Free TxQueue Timer*/
	for (queue_id = 0; queue_id < 4; ++queue_id){
		atbmwifi_queue_deinit(&hw_priv->tx_queue[queue_id]);
	}

	for (i = 0; i < ATBM_WIFI_MAX_VIFS; i++) {
		if (hw_priv->tx_queue_stats.link_map_cache[i]) {
			atbm_kfree(hw_priv->tx_queue_stats.link_map_cache[i]);
			hw_priv->tx_queue_stats.link_map_cache[i] = ATBM_NULL;
		}
	}
	wsm_buf_deinit(&hw_priv->wsm_cmd_buf);
	atbm_os_delete_waitevent(&hw_priv->wsm_synchanl_done);
	atbm_os_delete_waitevent(&hw_priv->wsm_startup_done);
}
