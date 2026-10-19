#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <lwip/netif.h>
#include "lwip/inet.h"
#include "atbm_hal.h"
#include "atbm_etf.h"
#include "atbm_wifi_driver_api.h"

typedef void(*T_pfCMD)(int argc, char **argv);
typedef struct _wifi_cmd_entry
{
    char cmd_name[32];
    T_pfCMD cmd_entry;
}wifi_cmd_entry_t;

extern struct atbmwifi_common g_hw_prv;
#define ATBM6062_WIFI_DRV_LIB_VER    "libdrv_wifi_atbm606x_v0.0.1"

int atbm_wifi_init()
{
	iot_printf( "%s\n", ATBM6062_WIFI_DRV_LIB_VER);
#if ATBM_USB_BUS
extern int atbm_akwifi_setup_usb();
	if(atbm_akwifi_setup_usb() < 0 ){
		drv_printk(C2, M_DRVSYS, "atbm_akwifi_setup_usb error\n");
		return -1;
	}  
#endif
	atbm_wifi_hw_init();
	return 0;
}

void app_wifi_scan(int argc, char **argv)
{
    struct atbmwifi_vif *priv = g_hw_prv.vif_list[0];

    atbm_wifi_on(ATBM_WIFI_STA_MODE);
    atbmwifi_scan_process(priv);
}

void app_wifi_join(int argc, char **argv)
{
    char ssid[40]={0};
    char pwd[80]={0};
    if(argc < 3){
        iot_printf("need more arg!\n");
        return;
    }
    strcpy(ssid,argv[2]);
    if(argc >=4){
        strcpy(pwd,argv[3]);
    }
    atbm_wifi_on(ATBM_WIFI_STA_MODE);
    atbm_wifi_sta_join_ap(ssid, NULL, 0, 0,pwd);
}
void app_wifi_disc(int argc, char **argv)
{
    atbm_wifi_sta_disjoin_ap();
}

extern char ConvertHexChar(char ch);
int atbm_str2mac(char *macstr, unsigned char *buf)
{
	char tmp = 0;
	for(int i =0; i< 5; i++)
	{
		tmp = ConvertHexChar(*macstr++);
		if(tmp < 0){
			return -1;
		}
		buf[i] = tmp*16;
		tmp = ConvertHexChar(*macstr++);
		if(tmp < 0){
			return -1;
		}
		buf[i] = buf[i]+tmp;
		macstr++;
	}
	tmp= ConvertHexChar(*macstr++);
	if(tmp < 0){
		return -1;
	}
	buf[5] = tmp*16;
	tmp = ConvertHexChar(*macstr);
	if(tmp < 0){
		return -1;
	}
	buf[5] = buf[5]+tmp;
	return 0;
}
static void UartCmd_StartTx(int argc, char **args){
	atbm_uint32 channel, mode,  rateIdx,  bw,  chOff,  ldpc,  packetLen;
	atbm_uint32 flag = 0, i = 0, j = 0;
	
	if(argc < 9){
		printf("need more parameter,please try again!\n");
	}
	channel = atoi(args[2]);
	mode = atoi(args[3]);
	rateIdx = atoi(args[4]);
	bw = atoi(args[5]);
	chOff = atoi(args[6]);
	ldpc = atoi(args[7]);
	packetLen = atoi(args[8]);


	printf("channel:%d, mode:%d,  rateIdx:%d,  bw:%d,  chOff:%d,  ldpc:%d,  packetLen:%d\n", channel, mode,  rateIdx,  bw,  chOff,  ldpc,  packetLen);
	if(atbm_etf_start_tx(channel, mode,  rateIdx,  bw,  chOff,  ldpc,  packetLen) < 0){
		printf("start tx failed!\n");
	}
}

static void UartCmd_StopTx(int argc, char **args){
	atbm_etf_stop_tx();
}

static void UartCmd_StartRx(int argc, char **args){
	atbm_uint32 channel , bw,  chOff,mode; 
	if(argc < 6){
		printf("need more parameter,please try again!\n");
		return;
	}
	channel = atoi(args[2]);
	bw = atoi(args[3]);
	chOff = atoi(args[4]);
	mode = atoi(args[5]);
	
	printf("channel : %d bw: %d chOff:%d %s\n", channel, bw, chOff,mode ? "DSSS":"OFDM");
	if(atbm_etf_start_rx(channel, bw, chOff,mode) < 0){
	    printf("start etf rx failed!\n");
		return;
	}
}

static void UartCmd_StopRx(int argc, char **args)
{
	//atbm_etf_stop_rx();
}
static void UartCmd_SetPowerTarget(int argc, char **args)
{
	atbm_int32 i = 0;
	atbm_int32 neg = 0, flag = 0;
	atbm_int32 powerTar = 0;
	char *pos = args[2];

	for(i=0;pos[i] != '\0';i++){
		if(pos[i] == '-'){
			neg = 1;
			continue;
		}
		if(pos[i] == '.'){
			flag = 1;
			continue;
		}	
		powerTar = powerTar* 10 +(pos[i] - 0x30);
	}

	if(flag == 0)
		powerTar = powerTar * 10;
	
	if(neg)
		powerTar = 0 - powerTar;
	
	atbm_etf_set_powerTarget(powerTar);
}

static void UartCmd_GetPowerTarget(int argc, char **args)
{
	atbm_etf_get_powerTarget();
}
extern atbm_int32 atbmwifi_enable_lmaclog(atbm_uint32 value );
static void atbm_enable_logprint(int argc, char **args)
{
	int enable;
	if(argc < 3){
		printf("need more params!!\n");
		return;
	}
	enable = atoi(args[2]);
	atbmwifi_enable_lmaclog(enable);
}

static void atbm_wifi_fwcmd(int argc, char **args)
{
	char cmd[64]={0};
	int cmd_len=0;
	if(argc < 3){
		printf("need more params!!\n");
		return;
	}
	cmd_len=strlen(args[2]);
	if(cmd_len >sizeof(cmd)){
		printf("cmd too long \n");
		return ;
	}
	strcpy(cmd,args[2]);
	cmd[cmd_len]=0;
	atbmwifi_enable_lmaclog(1);
	wsm_write_mib(&g_hw_prv, WSM_MIB_ID_FW_CMD,cmd, cmd_len+1, 0);
}

extern int atbm_hex2byte(const char *hex);
extern int DCXOCodeWrite(struct atbmwifi_common *hw_priv,atbm_uint8 data);
extern int atbm_save_efuse(struct atbmwifi_common *hw_priv, struct efuse_headr *efuse_save);
static void atbm_wifi_efuse_set(int argc, char **args)
{
	int iError = -1;
	atbm_uint32 writeEfuseFlag = 0;//0:write to register;1:write to efuse
	ATBM_BOOL bDcxo = 0, bGain = 0;
	char cmdBuf[128] = {0};
	struct efuse_headr efuse_data_local;
	struct atbmwifi_vif *vif=_atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	struct atbmwifi_common *hw_priv = vif->hw_priv;
	memset(&efuse_data_local, 0, sizeof(struct efuse_headr));
	
	memcpy(&efuse_data_local, &hw_priv->efuse, sizeof(struct efuse_headr));
	if(argc < 4){
		printk("need more params!!\n");
		return;
	}
	if(memcmp(args[2], "setEfuse_dcxo", 13) == 0)
	{
		if(argc < 4){
			printk("setEfuse_deltagain need more params!!\n");
			return;
		}
		bDcxo = 1;
		efuse_data_local.dcxo_trim = atoi(args[3]);
		if(argc == 5)
			writeEfuseFlag = atoi(args[4]);
		printk("set efuse data is dcxo[%d]\n",efuse_data_local.dcxo_trim);
	}
	else if(memcmp(args[2], "setEfuse_deltagain", 18) == 0)
	{
		if(argc < 6){
			printk("setEfuse_deltagain need more params!!\n");
			return;
		}
		bGain = 1;
		efuse_data_local.delta_gain1 = atoi(args[3]);
		efuse_data_local.delta_gain2 = atoi(args[4]);
		efuse_data_local.delta_gain3 = atoi(args[5]);
		if(argc == 7)
			writeEfuseFlag = atoi(args[6]);
		printk("set efuse data is delta_gain[%d,%d,%d]\n",
			efuse_data_local.delta_gain1,efuse_data_local.delta_gain2,efuse_data_local.delta_gain3);
	}
	else if(memcmp(args[2], "setEfuse_mac", 12) == 0)
	{
		if(argc < 4){
			printk("setEfuse_mac need more params!!\n");
			return;
		}
		atbm_str2mac(args[3],efuse_data_local.mac);
		writeEfuseFlag = 1;
		printk("set efuse data is mac["MACSTR"]\n",MAC2STR(efuse_data_local.mac));

	}
	// aden add
	else if(strcmp(args[2], "setEfuse_version") == 0)
	{
		if(argc < 4){
			printf("setEfuse_version need more params!!\n");
			return;
		}
		efuse_data_local.version=atoi(args[3]);
		writeEfuseFlag = 1;
		printf("set efuse version %d\n",efuse_data_local.version);
	}
	if(writeEfuseFlag == 1)
	{
		iError = atbm_save_efuse(hw_priv, &efuse_data_local);
		if(iError){
			printk("set efuse data failed:%d!!!\n", iError);
		}
		else
		{
			memcpy(&hw_priv->efuse, &efuse_data_local, sizeof(efuse_data_local));
			printk("set efuse data success\n");
		}
	}
	else
	{
		if(bDcxo)
		{
			iError = DCXOCodeWrite(hw_priv, efuse_data_local.dcxo_trim);
			if(iError)
				printk("DCXOCodeWrite failed:%d!!!\n", iError);
			else
				printk("DCXOCodeWrite success\n");
		}
		if(bGain)
		{
			memset(cmdBuf, 0, sizeof(cmdBuf));
			sprintf(cmdBuf, "set_txpwr_and_dcxo,%d,%d,%d,%d ", efuse_data_local.delta_gain1,
				efuse_data_local.delta_gain2, efuse_data_local.delta_gain3, efuse_data_local.dcxo_trim);
			
			printk("cmd: %s\n", cmdBuf);
			iError = wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmdBuf, strlen(cmdBuf), 0);
			if(iError){
				printk("%s: write mib failed(%d). \n",__func__, iError);
			}
			else
				printk("deltagain write success\n");
		}
	}
	
}

static void atbm_wifi_set_all_efuse(int argc, char **args)
{
	int iError = -1;
	struct atbmwifi_vif *vif=_atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	struct atbmwifi_common *hw_priv = vif->hw_priv;
	int efuse_remainbit = 0;
	struct efuse_headr tmpEfuse;

	memset(&tmpEfuse, 0, sizeof(tmpEfuse));
	memcpy(&tmpEfuse, &hw_priv->efuse, sizeof(struct efuse_headr));	

	if(argc < 7)
	{
		printk("need more params\n");
		return ;
	}
	
	tmpEfuse.dcxo_trim = atoi(args[2]);
	if((tmpEfuse.dcxo_trim < 0) || (tmpEfuse.dcxo_trim > 127))
	{
		printk("invalid dcxo:%d\n", tmpEfuse.dcxo_trim);
		return ;
	}
	
	tmpEfuse.delta_gain1 = atoi(args[3]);
	if((tmpEfuse.delta_gain1 < 0) || (tmpEfuse.delta_gain1 > 31))
	{
		printk("invalid delta_gain1:%d\n", tmpEfuse.delta_gain1);
		return ;
	}
	tmpEfuse.delta_gain2 = atoi(args[4]);
	if((tmpEfuse.delta_gain2 < 0) || (tmpEfuse.delta_gain2 > 31))
	{
		printk("invalid delta_gain2:%d\n", tmpEfuse.delta_gain2);
		return ;
	}
	tmpEfuse.delta_gain3 = atoi(args[5]);
	if((tmpEfuse.delta_gain3 < 0) || (tmpEfuse.delta_gain3 > 31))
	{
		printk("invalid delta_gain3:%d\n", tmpEfuse.delta_gain3);
		return ;
	}
	tmpEfuse.PowerSupplySel = 0;
	tmpEfuse.topref_ctrl_bias_res_trim = 0;
	atbm_str2mac(args[6],tmpEfuse.mac);
	if (args[7])
	{
		tmpEfuse.Tj_room = atoi(args[7]);
	}
	if (args[8])
	{
		tmpEfuse.version=atoi(args[8]);
	}
	printk("efuse data:[%d,%d,%d,%d,%d,%d,%d,%d,"MACSTR"]\n",
		tmpEfuse.version,tmpEfuse.dcxo_trim,
		tmpEfuse.delta_gain1,tmpEfuse.delta_gain2,tmpEfuse.delta_gain3,
		tmpEfuse.Tj_room,tmpEfuse.PowerSupplySel,tmpEfuse.topref_ctrl_bias_res_trim,
		MAC2STR(tmpEfuse.mac));

	iError = atbm_save_efuse(hw_priv, &tmpEfuse);
	if (iError == 0)
	{
		printk("setEfuse success \n");
		memcpy(&hw_priv->efuse, &tmpEfuse, sizeof(tmpEfuse));
	}else
	{
		printk("setEfuse failed [%d]\n", iError);
	}
	iError = wsm_get_efuse_remain_bit(hw_priv, &efuse_remainbit, sizeof(efuse_remainbit));
	printk("##after write efuse_remainbit:%d##\n", efuse_remainbit);

}

static void atbm_wifi_efuse_get(int argc, char **args)
{
	int iError;
	struct efuse_headr efuse_data_local;
	struct atbmwifi_vif *vif=_atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	struct atbmwifi_common *hw_priv = vif->hw_priv;
	memset(&efuse_data_local, 0, sizeof(struct efuse_headr));
	iError = wsm_get_efuse_data(hw_priv, &efuse_data_local, sizeof(struct efuse_headr));
	if(iError){
		printk("get efuse data error!!\n");
	}else{
		printk("Get efuse data is [%d,%d,%d,%d,%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x]\n",
				efuse_data_local.version,efuse_data_local.dcxo_trim,efuse_data_local.delta_gain1,efuse_data_local.delta_gain2,efuse_data_local.delta_gain3,
				efuse_data_local.Tj_room,efuse_data_local.topref_ctrl_bias_res_trim,efuse_data_local.PowerSupplySel,efuse_data_local.mac[0],efuse_data_local.mac[1],
				efuse_data_local.mac[2],efuse_data_local.mac[3],efuse_data_local.mac[4],efuse_data_local.mac[5]);
	}
}
//get first efuse data
static void atbm_wifi_efuse_getfirst(int argc, char **args)
{
	int iError;
	struct efuse_headr tmpEfuse;
	struct atbmwifi_vif *vif=_atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	struct atbmwifi_common *hw_priv = vif->hw_priv;
	memset(&tmpEfuse, 0, sizeof(struct efuse_headr));
	iError = wsm_get_efuse_first_data(hw_priv, &tmpEfuse, sizeof(struct efuse_headr));
	if(iError){
		printk("get efuse first data error!!\n");
	}else{
		printk("efuse first data is [%d,%d,%d,%d,%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x]\n",
				tmpEfuse.version,tmpEfuse.dcxo_trim,tmpEfuse.delta_gain1,tmpEfuse.delta_gain2,tmpEfuse.delta_gain3,
				tmpEfuse.Tj_room,tmpEfuse.topref_ctrl_bias_res_trim,tmpEfuse.PowerSupplySel,tmpEfuse.mac[0],tmpEfuse.mac[1],
				tmpEfuse.mac[2],tmpEfuse.mac[3],tmpEfuse.mac[4],tmpEfuse.mac[5]);
	}
}
//get all efuse data for debug
static void atbm_wifi_get_efuse_all_data(int argc, char **args)
{
	int iError,i = 0;
	struct atbmwifi_vif *vif=_atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	struct atbmwifi_common *hw_priv = vif->hw_priv;
	atbm_uint8 buffer[128] = {0};

	memset(buffer,0, sizeof(buffer));
	if ((iError = wsm_get_efuse_all_data(hw_priv, (void *)&buffer, sizeof(buffer))) == 0){	
		printk("all efuse data:");
		for(i=0;i<109;i++)
			printk("%02x ",buffer[i]);
		printk("\n");
	}
	else{
		printk("read all efuse failed\n");
	}
}

//get efuse remain sapce
static void atbm_wifi_get_efuse_free_space(int argc, char **args)
{
	int iError;
	struct atbmwifi_vif *vif=_atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	struct atbmwifi_common *hw_priv = vif->hw_priv;
	int efuse_remainbit = 0;
	iError = wsm_get_efuse_remain_bit(hw_priv, &efuse_remainbit, sizeof(int));
	
	if(iError){
		printk("get efuse first data error!!\n");
	}else{
		printk("efuse free space:[%d] bit \n", efuse_remainbit);
	}

}
//get first mac
static void atbm_wifi_get_First_mac(int argc, char **args)
{
	//int index;
	atbm_uint8 macAddr[6] = {0};
	int iError;
	struct atbmwifi_vif *vif=_atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	struct atbmwifi_common *hw_priv = vif->hw_priv;

	if ((iError = wsm_get_efuse_first_mac(hw_priv, &macAddr[0])) == 0){
		printk("firstMacAddr:[%02x:%02x:%02x:%02x:%02x:%02x]\n", macAddr[0], macAddr[1], macAddr[2], 
			macAddr[3], macAddr[4], macAddr[5]);
	}
	else{
		printk("read mac address failed\n");
	}

}

static void UartCmd_MFGCarrierTone(int argc, char **args)
{
 	atbm_uint32 uiChannel;
	atbm_int32	iError;

 	uiChannel = atoi(args[2]);
 	printk("Ch : %d \n", uiChannel);
 	iError = atbm_wifi_mfg_CarrierTone(uiChannel);
	printk("CarrierTone Status : %d \n", iError);
}

void get_netif_status(int argc, char **args)
{
    extern struct netif *netif_list;
    struct netif *netif;

    for(netif=netif_list;netif;netif=netif->next){
        iot_printf("network interface device: %c%c   %s\r\n", netif->name[0],netif->name[1],
                (netif == netif_default) ? " (Default)" : "");
        iot_printf("MTU: %d\r\n", netif->mtu);

        /* 6 - MAC address, 8 - IEMI */
        if (netif->hwaddr_len == 6)
        {
            iot_printf("MAC: ");
            for (int index = 0; index < netif->hwaddr_len; index++)
            {
                iot_printf("%02x ", netif->hwaddr[index]);
            }
        }
        iot_printf("\r\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP)
            iot_printf(" UP");
        else
            iot_printf(" DOWN");
        if (netif->flags & NETIF_FLAG_LINK_UP)
            iot_printf(" LINK_UP");
        else
            iot_printf(" LINK_DOWN");
        if (netif->flags & NETIF_FLAG_ETHERNET)
            iot_printf(" INTERNET_UP");
        else
            iot_printf(" INTERNET_DOWN");

        if (netif->flags & NETIF_FLAG_ETHARP)
            iot_printf(" ETHARP");
        if (netif->flags & NETIF_FLAG_BROADCAST)
            iot_printf(" BROADCAST");
        if (netif->flags & NETIF_FLAG_IGMP)
            iot_printf(" IGMP");
        iot_printf("\r\n");
        iot_printf("ip address: %s\r\n", inet_ntoa(netif->ip_addr));
        iot_printf("gw address: %s\r\n", inet_ntoa(netif->gw));
        iot_printf("net mask  : %s\r\n", inet_ntoa(netif->netmask));
        if (netif->next)
        {
            iot_printf("\r\n");
        }
    }
}

void UartCmd_set_adaptive(int argc, char **args)
{
	int enable;

	if(argc < 3){
		printk("need more params!!\n");
		return;
	}
	enable = atoi(args[2]);

	if( !atbmwifi_dev_set_adaptive(enable) )
	{
		iot_printf("set adaptive %d\n", enable);
	}else
	{
		iot_printf("cmd err!\n");
	}

}
#if 0
void UartCmd_set_txpower_mode(int argc, char **args)
{
	int value;
	int ret;
	
	if(argc < 3){
		printk("need more params!!\n");
		return;
	}
	value = atoi(args[2]);

	ret = atbm_set_txpower_mode(value);
	if(ret)
		printf("set failed.\n");
	else 
		printf("cmd ok\n");
}

static void UartCmd_set_txpower(int argc, char **args){
	atbm_uint32 function_index,mode,rate,bw,power;
	start_tx_param_t tx_param;
	
	if(argc < 7){
		printf("need more parameter,please try again!\n");
		return;
	}
	function_index = atoi(args[2]);
	mode = atoi(args[3]);
	rate = atoi(args[4]);
	bw = atoi(args[5]);
	power = atoi(args[6]);
	

	printf("%s:function_index[%d],mode[%d],rate[%d],bw[%d]\n", __func__,
			function_index,mode,rate,bw);
	
	tx_param.mode = mode;
	tx_param.rate_id = rate;
	tx_param.bw = bw;
	tx_param.power = power;
	
	if(atbm_common_set_txpower(function_index, &tx_param) < 0){
		printf("set tx power failed!\n");
	}
}

static void UartCmd_StartTx_Internal(int argc, char **args){
	atbm_uint32 channel, mode,  rateIdx,  bw,  chOff,  ldpc,  packetLen, precom, precom_temp;
	atbm_uint32 flag = 0, i = 0, j = 0;
	start_tx_param_t tx_param;
	
	if(argc < 9){
		printf("need more parameter,please try again!\n");
		return;
	}
	channel = atoi(args[2]);
	mode = atoi(args[3]);
	rateIdx = atoi(args[4]);
	bw = atoi(args[5]);
	chOff = atoi(args[6]);
	ldpc = atoi(args[7]);
	packetLen = atoi(args[8]);
	if(argc == 10)
	{
		precom_temp = atoi(args[9]);
		printf("tx use precom:%d\n", precom_temp);
	}
//
	if(packetLen)
		{
			/*
			*parase precom
			*/
			//ATBM_WEXT_PROCESS_PARAMS(pos,len,precom_temp,atbm_accsii_to_int,false,exit,ret);
			switch(precom_temp){
			case 1://FCC
			case 2:
				if(bw != 0)
					precom = 0; 
				break;
			case 3://SRRC 10dBm 
			case 6://SRRC 3dBm
				precom = (precom_temp==3)?9:10; 
				break;
			case 4://FCC
				if(mode == 0)//DSSS
					precom = 8;
				else{//OFDM
					if(bw == 0){
						//if((bw == 0) && (channel != 1) && (channel != 11))
							//precom = 0;	
						if(channel==11){
							bw = 1;
							precom = 3;
							if(mode == 3)
								ldpc = 1;
						}
						else
							precom = 6;
					}
					else{
						precom = (channel==9)?4:7;
						//if((bw == 1) && (channel != 3) && (channel != 9))
							//precom = 0;	
					}
				}
				break;
			case 5://CE
				if(mode == 0)//DSSS
					precom = 8;
				else{//OFDM
					if(bw == 0){
						//if((bw == 0) && (channel != 1) && (channel != 13))
							//precom = 0;	
						if(channel==13){
							bw = 1;
							precom = 3;
							if(mode == 3)
								ldpc = 1;
						}
						else
							precom = 6;
					}
					else{
						precom = (channel==11)?4:7;
						//if((bw == 1) && (channel != 3) && (channel != 11))
							//precom = 0;	
					}
				}
				break;
			default:
				precom = 0;
				break;
			}	
		}else{
			printf("packetLen err\n");
		}
		
	printf("%s:channel[%d],mode[%d],rateIdx[%d],bw[%d],chOff[%d],ldpc[%d],packetLen[%d],precom[%d]\n", __func__,
			channel,mode,rateIdx,bw,chOff,ldpc,packetLen,precom);
	
	tx_param.channel = channel;
	tx_param.mode = mode;
	tx_param.rate_id = rateIdx;
	tx_param.bw = bw;
	tx_param.chOff = chOff;
	tx_param.ldpc = ldpc;
	tx_param.pktlen = packetLen;
	tx_param.precom = precom;
	
	if(atbm_internal_start_tx(&tx_param) < 0){
		printf("start tx failed!\n");
	}
}

static void UartCmd_StopTx_Internal(int argc, char **args){
	atbm_internal_stop_tx();
}

static void UartCmd_StartRx_Internal(int argc, char **args){
	atbm_uint32 channel , bw,  chOff,mode; 
	start_rx_param_t rx_param;
	
	if(argc < 6){
		printf("need more parameter,please try again!\n");
		return;
	}
	channel = atoi(args[2]);
	bw = atoi(args[3]);
	chOff = atoi(args[4]);
	mode = atoi(args[5]);
	
	printf("channel : %d bw: %d chOff:%d %s\n", channel, bw, chOff,mode ? "DSSS":"OFDM");
	rx_param.channel = channel;
	rx_param.bw = bw;
	rx_param.chOff = chOff;
	rx_param.mode = mode;
	
	if(atbm_internal_start_rx(&rx_param) < 0){
	    printf("start etf rx failed!\n");
		return;
	}
}

static void UartCmd_StopRx_Internal(int argc, char **args)
{
	get_result_rx_data rx_data;
	atbm_internal_stop_rx(&rx_data);
}
#endif

/* wifi test command entry table, we can add new cmd hear */
static wifi_cmd_entry_t s_test_cmd_table[] = 
{
	//{"start_tx_internal",       UartCmd_StartTx_Internal},
	//{"stop_tx_internal",       	UartCmd_StopTx_Internal},
	//{"start_rx_internal",       UartCmd_StartRx_Internal},
	//{"stop_rx_internal",       	UartCmd_StopRx_Internal},

/* WIFI test cmd start*/
	{"start_tx",       		UartCmd_StartTx},
	{"stop_tx",       		UartCmd_StopTx},
	{"start_rx",       		UartCmd_StartRx},
	{"stop_rx",       		UartCmd_StopRx},
	{"set_power_target",       UartCmd_SetPowerTarget},
	{"get_power_target",       UartCmd_GetPowerTarget},
    {"lmac_log",            atbm_enable_logprint},
    {"efuse_set",           atbm_wifi_efuse_set},
    {"efuse_setall",        atbm_wifi_set_all_efuse},
    {"efuse_get",           atbm_wifi_efuse_get},
    {"efuse_getfirst",      atbm_wifi_efuse_getfirst},
    {"efuse_getall",        atbm_wifi_get_efuse_all_data},
    {"efuse_getfirstmac",   atbm_wifi_get_First_mac},
    {"efuse_getfreespace",  atbm_wifi_get_efuse_free_space},
    {"mfgcarriertone",      UartCmd_MFGCarrierTone},
#if CONFIG_P2P
	{"p2p_start",			wifi_p2p_start},
	{"p2p_go",				wifi_p2p_go_start},
	{"p2p_find",			wifi_p2p_find},
	{"p2p_find_stop",		wifi_p2p_find_stop},
	{"p2p_stop",			wifi_p2p_stop},
#endif
	{"scan",				app_wifi_scan},
    {"join",				app_wifi_join},
    {"disc",                app_wifi_disc},
	{"fwcmd",				atbm_wifi_fwcmd},
    {"netif",               get_netif_status},
	{"set_adaptive", 		UartCmd_set_adaptive},
	//{"set_txpower_mode", 	UartCmd_set_txpower_mode},
	//{"set_txpower", 		UartCmd_set_txpower},
};

static int cmd_entry_check(wifi_cmd_entry_t * cmd_table, int argc, char **args)
{
	if(args[1]==NULL){
		return -1;
	}
    //find the right cmd name
	int cmd_cnt = sizeof(s_test_cmd_table) / sizeof(s_test_cmd_table[0]);
    for(int i = 0; i<cmd_cnt; i++)
    {
        if (strcmp(cmd_table[i].cmd_name, args[1]) == 0)
        {
            //find and execute entry function
            cmd_table[i].cmd_entry(argc, args);
            return 0;
        }
    }
    return -1;
}
static void wifi_test_help()
{
	int cnt=sizeof(s_test_cmd_table)/sizeof(s_test_cmd_table[0]);
	iot_printf("atbm_cmd_list:\n");
	for(int i=0;i<cnt;i++){
		printk("\t%s\n",s_test_cmd_table[i].cmd_name);
	}
}
void atbm_wifi_test_func(int argc, char **args)
{
    struct atbmwifi_vif *priv = _atbmwifi_hwpriv_to_vifpriv(&g_hw_prv,0);
	if(!priv){
		atbm_wifi_init();
		return ;
	}
    if(cmd_entry_check(s_test_cmd_table, argc, args) != 0)
    {       
		wifi_test_help();
    }
	
}

