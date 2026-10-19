/*
 * FreeRTOS Wi-Fi V1.0.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file iot_wifi.c
 * @brief Wi-Fi Interface.
 */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* Socket and Wi-Fi interface includes. */
#include "FreeRTOS.h"
#include "iot_wifi.h"

/* Wi-Fi configuration includes. */
#include "aws_wifi_config.h"
#include "board.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_DHCP_Server.h"
#if USE_LWIP
#include "dhcp.h"
#endif
#include "semphr.h"

#include "atbm_type.h"
#include "atbm_wifi_driver_api.h"
#include "atbm_api.h"

int atbm_func_test();
static ATBM_WIFI_MODE g_wifi_mode = ATBM_WIFI_STA_MODE;
static SemaphoreHandle_t xWiFiSem;
static bool wifi_started;
static int g_sta_channel;
int wps_connect_done;
/*-----------------------------------------------------------*/

#define ATBM_DEFAULT_P2P_CHANNEL 36

static int get_sta_channel_or_default(int default_channel)
{
	ATBM_WLAN_CONNECTION_INFO info;
	int ret;

	memset(&info, 0, sizeof(info));
	ret = atbm_wifi_get_connected_info_vif(0, &info);
	if (ret == 0 && info.channel > 0) {
		g_sta_channel = info.channel;
		printf("sta connected channel=%d\r\n", g_sta_channel);
		return g_sta_channel;
	}

	printf("sta connected info ret=%d ssid_len=%d ssid:%.*s bssid:%02x:%02x:%02x:%02x:%02x:%02x channel=%d\r\n",
		ret, info.Ssid_len, info.Ssid_len, info.Ssid,
		info.bssid[0], info.bssid[1], info.bssid[2],
		info.bssid[3], info.bssid[4], info.bssid[5],
		(int)info.channel);

	if (g_sta_channel > 0) {
		printf("sta channel unavailable ret=%d channel=%d, use cached channel=%d\r\n",
			ret, (int)info.channel, g_sta_channel);
		return g_sta_channel;
	}

	printf("sta channel unavailable ret=%d channel=%d, use default channel=%d\r\n",
		ret, (int)info.channel, default_channel);
	return default_channel;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_On( void )
{
	WIFIReturnCode_t ret = eWiFiFailure;
	char *countrycode={"CN"}; 

	printf("[BT_WIFI_TRACE] WIFI_On enter started=%d mode=%d tick=%u\r\n", wifi_started, g_wifi_mode, (unsigned int)xTaskGetTickCount());
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
	if (wifi_started) {
		ret = eWiFiSuccess;
		goto exit;
	}
	printf("wext_set_adaptivity_th_l2h_ini\r\n");
	//wext_set_adaptivity_th_l2h_ini(-17);

	wifi_started = true;
	atbm_void* handle = NULL;
	printf("[BT_WIFI_TRACE] WIFI_On atbm_wifi_hw_init enter tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	atbm_wifi_hw_init();
	printf("[BT_WIFI_TRACE] WIFI_On atbm_wifi_hw_init leave tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	handle = atbm_wifi_on(g_wifi_mode);
	printf("[BT_WIFI_TRACE] WIFI_On atbm_wifi_on handle=0x%x tick=%u\r\n", (unsigned int)handle, (unsigned int)xTaskGetTickCount());
	if (handle) {
		wifi_started = true;
		ret = eWiFiSuccess;
	} else {
		wifi_started = false;
		ret = eWiFiFailure;
	}
exit:
	xSemaphoreGive(xWiFiSem);
	printf("[BT_WIFI_TRACE] WIFI_On leave ret=%d started=%d tick=%u\r\n", ret, wifi_started, (unsigned int)xTaskGetTickCount());
    return ret;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
	int if_id = 0;
	if (ATBM_WIFI_AP_MODE == g_wifi_mode || ATBM_WIFI_P2P_GO == g_wifi_mode ) {
		if_id = 1;
	}
	atbm_wifi_off(if_id);
	atbm_wifi_hw_deinit();
	wifi_started = false;
	xSemaphoreGive(xWiFiSem);
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

static int convertWIFISecurity2atbm_security(WIFISecurity_t type)
{
	int	security_type = WLAN_WPA_AUTH_NONE;

	switch (type) {
	case eWiFiSecurityOpen:
		security_type = WLAN_WPA_AUTH_NONE;
		break;
	case eWiFiSecurityWPA2:
		security_type = WLAN_WPA2_AUTH_PSK;
		break;
	default:
		security_type = WLAN_WPA_AUTH_NONE;
		break;
	}
	return security_type;
}

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
	int	security_type;
	int ret = -1;
       if (pxNetworkParams == NULL)
		return eWiFiFailure;
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
	security_type = convertWIFISecurity2atbm_security(pxNetworkParams->xSecurity);
	printf("WIFI_ConnectAP:ucSSID:%s, cPassphrase:%s, security_type:%d\r\n", (char *)pxNetworkParams->ucSSID, (char *)pxNetworkParams->xPassword.xWPA.cPassphrase, security_type);
	ret = atbm_wifi_sta_join_ap((char *)pxNetworkParams->ucSSID, NULL, security_type, WLAN_ENCRYPT_NONE, (char *)pxNetworkParams->xPassword.xWPA.cPassphrase);

	printf("WIFI_ConnectAP ret:%d\r\n", ret); 
	xSemaphoreGive(xWiFiSem);
	if (ret == -1) {

		return eWiFiFailure;
	}
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
	xSemaphoreGive(xWiFiSem);
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Reset( void )
{
    return eWiFiSuccess;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
	WIFIReturnCode_t ret = eWiFiNotSupported; 
	switch (xDeviceMode) {
	case eWiFiModeStation:
		ret = eWiFiSuccess;
		g_wifi_mode = ATBM_WIFI_STA_MODE;
		break;
	case eWiFiModeAP:
		ret = eWiFiSuccess;
		g_wifi_mode = ATBM_WIFI_AP_MODE;
		break;
	case eWiFiModeP2P:
		ret = eWiFiSuccess;
		g_wifi_mode = ATBM_WIFI_P2P_GO;
		break;
	default:
		break;
			
	}
    return ret;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t * pxDeviceMode )
{
    WIFIReturnCode_t ret = eWiFiNotSupported;
	if (NULL == pxDeviceMode)
		return ret;
	switch (g_wifi_mode) {
	case ATBM_WIFI_STA_MODE:
		ret = eWiFiSuccess;
		*pxDeviceMode = eWiFiModeStation;
		break;
	case ATBM_WIFI_AP_MODE:
		ret = eWiFiSuccess;
		*pxDeviceMode = eWiFiModeAP;
		break;
	case ATBM_WIFI_P2P_GO:
		ret = eWiFiSuccess;
		*pxDeviceMode = eWiFiModeP2P;
		break;
	default:
		break;
			
	}
    return ret;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t * pusIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t * pxNetworkProfile,
                                  uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
	int ret = -1;
	char mac[6] = {0};
	ret = atbm_wifi_get_mac_address(mac);
	if (ret < 0) {
		printf("get wlan mac failed\r\n");
	} else {
		sprintf(pucMac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t * pxBuffer,
                            uint8_t ucNumNetworks )
{
	WIFIReturnCode_t ret = eWiFiNotSupported;
    return ret;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartAP( void )
{
	WIFIReturnCode_t xWifiStatus;    
	WIFI_SetMode(eWiFiModeAP);
	
	xWifiStatus = WIFI_On();
    return xWifiStatus;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StopAP( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    WIFIReturnCode_t xWifiStatus;
	int ret = -1;
	if (ret < 0)
		xWifiStatus = eWiFiFailure;
	else
		xWifiStatus = eWiFiSuccess;
    return xWifiStatus;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void * pvOptionValue )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t * pxPMModeType,
                                 void * pvOptionValue )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

static WIFIEventHandler_t gRtosWifiEventHandler = NULL;
static int atbm_wifi_event_callback(int AP_sta_mode, int eventid, atbm_void *param)
{
	printf("%s:AP_sta_mode:%d, eventid:%d\n", __func__, AP_sta_mode, eventid);
	if(AP_sta_mode == ATBM_WIFI_AP_MODE ||  AP_sta_mode == ATBM_WIFI_P2P_GO){
		atbm_uint8 * staMacAddr ;
		switch(eventid){

			case ATBM_WIFI_DEAUTH_EVENT:
				staMacAddr =(atbm_uint8 *) param;	
				printf("DEAUTH_EVENT staMacAddr:[%02x:%02x:%02x:%02x:%02x:%02x]\n", staMacAddr[0], staMacAddr[1], staMacAddr[2], staMacAddr[3], staMacAddr[4], staMacAddr[5]);
				if (NULL != gRtosWifiEventHandler) {
				        WIFIEvent_t ev;
				        memset((void*)&ev, 0, sizeof(WIFIEvent_t));
				        ev.xEventType = eWiFiEventAPStationDisconnected;
				        memcpy(ev.xInfo.xAPStationDisconnected.ucMac, staMacAddr, 6);
					 printf("%s:%d\r\n", __func__, __LINE__);
				        gRtosWifiEventHandler(&ev);
			    }
				break;
			case ATBM_WIFI_AUTH_EVENT:
				break;
			case ATBM_WIFI_ASSOC_EVENT:
				break;
			case ATBM_WIFI_ASSOCRSP_TXOK_EVENT: 
				staMacAddr =(atbm_uint8 *) param;
				printf("ASSOCRSP_TXOK_EVENT staMacAddr:[%02x:%02x:%02x:%02x:%02x:%02x]\n", staMacAddr[0], staMacAddr[1], staMacAddr[2], staMacAddr[3], staMacAddr[4], staMacAddr[5]);
			       if (NULL != gRtosWifiEventHandler) {
        				WIFIEvent_t ev;
       				 memset((void*)&ev, 0, sizeof(WIFIEvent_t));
        				ev.xEventType = eWiFiEventAPStationConnected;
       				 memcpy(ev.xInfo.xAPStationConnected.ucMac, staMacAddr, 6);
					 printf("%s:%d\r\n", __func__, __LINE__);
       			 	gRtosWifiEventHandler(&ev);
    				}
				break;
			case ATBM_WIFI_DEASSOC_EVENT:
				break;
			case ATBM_WIFI_JOIN_EVENT:			
				staMacAddr =(atbm_uint8 *) param;
				break;
			case ATBM_WIFI_ENABLE_NET_EVENT:
				break;
			default:
				break;
		}
	}
	else {	
		switch(eventid){
			case ATBM_WIFI_SCANSTART_EVENT:
				break;
			case ATBM_WIFI_SCANDONE_EVENT:				
//				event.event_type = WLAN_E_SCAN_COMPLETE;
//				WLAN_SYS_StatusCallback(&event);	
				break;
			case ATBM_WIFI_DEAUTH_EVENT:				
				
				break;
			case ATBM_WIFI_AUTH_EVENT:
				break;
			case ATBM_WIFI_ASSOC_EVENT:				
				if (AP_sta_mode != ATBM_WIFI_STA_MODE) {
					printf("ignore non-STA assoc mode:%d\r\n", AP_sta_mode);
					break;
				}
				  if (NULL != gRtosWifiEventHandler) {
					        WIFIEvent_t ev;
					        memset((void*)&ev, 0, sizeof(WIFIEvent_t));
					        ev.xEventType = eWiFiEventConnected;
				        //memcpy(ev.xInfo.xConnected.xConnectionInfo.ucBSSID, buf, ETH_ALEN);
				        gRtosWifiEventHandler(&ev);
				  }
				break;
			case ATBM_WIFI_DEASSOC_EVENT:
					if (AP_sta_mode != ATBM_WIFI_STA_MODE) {
						printf("ignore non-STA deassoc mode:%d\r\n", AP_sta_mode);
						break;
					}
				    if (NULL != gRtosWifiEventHandler) {
					        WIFIEvent_t ev;
					        memset((void*)&ev, 0, sizeof(WIFIEvent_t));
				        ev.xEventType = eWiFiEventDisconnected;
				        //ev.xInfo.xDisconnected.xReason = (WIFIReason_t)disconn_reason;
				        gRtosWifiEventHandler(&ev);
				    }
				break;
			case ATBM_WIFI_JOIN_EVENT:
				break;
			default:
				break;
		}
	}
	return 0;
}


int wifi_register_user_event(WIFIEventType_t xEventType, WIFIEventHandler_t xHandler)
{
    (void)xEventType;
    gRtosWifiEventHandler = xHandler;
    return 0;
}

WIFIReturnCode_t WIFI_RegisterEvent( WIFIEventType_t xEventType,
                                     WIFIEventHandler_t xHandler )
{
    atbmwifi_event_register_cb((atbmwifi_event_callback)atbm_wifi_event_callback);
    return (WIFIReturnCode_t)wifi_register_user_event(xEventType, xHandler);
}
/*-----------------------------------------------------------*/

BaseType_t WIFI_IsConnected( const WIFINetworkParams_t * pxNetworkParams )
{
	/* FIX ME. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartScan( WIFIScanConfig_t * pxScanConfig )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetScanResults( const WIFIScanResult_t ** pxBuffer,
                                      uint16_t * ucNumNetworks )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartConnectAP( const WIFINetworkParams_t * pxNetworkParams )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartDisconnect( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetConnectionInfo( WIFIConnectionInfo_t * pxConnectionInfo )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetIPInfo( WIFIIPConfiguration_t * pxIPConfig )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetRSSI( int8_t * pcRSSI )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetStationList( WIFIStationInfo_t * pxStationList,
                                      uint8_t * pcStationListSize )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartDisconnectStation( uint8_t * pucMac )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetMAC( uint8_t * pucMac )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetCountryCode( const char * pcCountryCode )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetCountryCode( char * pcCountryCode )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetStatistic( WIFIStatisticInfo_t * pxStats )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetCapability( WIFICapabilityInfo_t * pxCaps )
{
    /* FIX ME. */
    return eWiFiNotSupported;    
}
/*-----------------------------------------------------------*/

int WIFI_Context_init()
{
	if (xWiFiSem == NULL)
		xWiFiSem = xSemaphoreCreateMutex();

	return 0;
}


#define ucNumNetworks  12
extern void cmd_test(const char* temp_uart_buf);

static TaskHandle_t       ping_task_handle = NULL;
static int flag_start_ping = 0;
static QueueHandle_t     ping_event_queue = NULL;
static BaseType_t xPingTotalSuccess = 0, xPingSendCount = 0;
struct ping_para
{
    uint32_t addr; 
    int         start;
};

 void vApplicationPingReplyHook( ePingReplyStatus_t eStatus,
								 uint16_t usIdentifier )
 {
	 if( eStatus == eSuccess )
	 {
		 //FreeRTOS_printf( ( "\r\nPing response received. ID: %d\r\n", usIdentifier ) );
		 printf("\r\nPing response received. ID: %d eStatus:%d\r\n", usIdentifier, eStatus);
 
		 /* Increment successful ping replies. */
		 xPingTotalSuccess++;
	 }
 }

static void ping_task_proc(void* arg)
{
    struct ping_para para = {0};

    for (;;) {
        if (flag_start_ping == 0) {printf("\r\n%s:%d\r\n", __func__, __LINE__);
            memset((void*)&para, 0, sizeof(para));
            if (xQueueReceive(ping_event_queue, &para,  portMAX_DELAY) != pdPASS) {
                printf("%s xQueueReceive err!\r\n", __func__);
                continue;
            }
            flag_start_ping = para.start;
        } else {
            FreeRTOS_SendPingRequest(para.addr, 64, 1000);
            xPingSendCount++;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void start_ping(const char *ipaddr)
{
    uint8_t ipAddress[4] = {0};
    uint32_t addr;
    struct ping_para para = {0};
    
    sscanf(ipaddr, "%d.%d.%d.%d", (int*)&ipAddress[0], (int*)&ipAddress[1], (int*)&ipAddress[2], (int*)&ipAddress[3]);
    addr = (ipAddress[3] << 24) | (ipAddress[2] << 16) | (ipAddress[1] << 8) | (ipAddress[0] << 0);
    printf("ipAddress[1] %d.%d.%d.%d\r\n", ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
    para.addr = addr;
    

    if (ping_event_queue == NULL) {
        ping_event_queue = xQueueCreate(1, sizeof(struct ping_para));
    }

    if (ping_task_handle == NULL) {
        xTaskCreate(ping_task_proc, "ping task",  512, NULL, configMAX_PRIORITIES / 3, &ping_task_handle);
    }

    if (flag_start_ping == 0) {
        printf("\r\nping start!\r\n");
        para.start = 1;
        xPingTotalSuccess = 0;
        xPingSendCount = 0;
        if (ping_event_queue) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            xQueueSend(ping_event_queue, &para, 0);
        }
    } else {
        printf("\r\nping stop!\r\n");
        printf("\r\n%d packets transmitted, %d received, %d%% packet loss\r\n", xPingSendCount, xPingTotalSuccess, (xPingSendCount - xPingTotalSuccess) * 100 / xPingSendCount );
        flag_start_ping = 0;
    }
}

#if !USE_LWIP
static const uint8_t ucIPAddressAp[4] = {192, 168, 13, 1};
#endif
static WIFIDeviceMode_t g_current_mode = eWiFiModeNotSupported;
int wifi_initialize(WIFIDeviceMode_t mode)
{
	WIFIReturnCode_t xWifiStatus;

	WIFI_Context_init();

	if (!(mode == eWiFiModeStation || mode ==eWiFiModeAP)) {
		printf("wifi mode is not supported\r\n");
		return -1;
	}
	
	if (g_current_mode != mode && g_current_mode != eWiFiModeNotSupported) {
		WIFI_Off();
	}

	WIFI_SetMode(mode);
	g_current_mode = mode;

	xWifiStatus = WIFI_On();
	if( xWifiStatus == eWiFiSuccess ) {
		printf("WiFi module initialized.\r\n");
		return 0;
	} else {
		printf("WiFi module failed to initialize.\r\n" );
		return -1;
	}
}

int start_ap(int channel, const char* ssid, const char* passwd, char need_passwd)
{
	char cmd[128] = {0};
	WIFIReturnCode_t xWifiStatus;
#if !USE_LWIP
	uint32_t IPAddress;
	setDhcpClientState(0);
	vDHCPProcess(1, eInitialWait);
	xSendDHCPEvent();
#endif
	WIFI_Context_init();
	if (g_current_mode != eWiFiModeAP || !wifi_started) {
		printf("atbm current mode is not ap\r\n");
		g_current_mode = eWiFiModeAP;
		WIFI_SetMode(eWiFiModeAP);
		
		if (wifi_started) {
			printf("wifi is start, so reboot wifi\r\n");
			WIFI_Off();
		}
		if (need_passwd)
		    sprintf(cmd, "wifi_ap %s %d %s", ssid, channel, passwd);
		else
		    sprintf(cmd, "wifi_ap %s %d", ssid, channel);
		xWifiStatus = WIFI_On();
		if (xWifiStatus != eWiFiSuccess) {
			printf("WiFi module failed to initialize.\r\n");
			return -1;
		}
#if 1
		atbm_wifi_ap_create((char *)ssid, WLAN_WPA2_AUTH_PSK, WLAN_ENCRYPT_AES, (char *)passwd, channel, 0);

#else
		atbm_func_test();
#endif
	}
#if !USE_LWIP
	IPAddress = (20 << 24) | (13 << 16) | (168 << 8) | (192 << 0);
	dhcpserver_start(ucIPAddressAp, IPAddress, 10);
#endif
	return 0;
}

#if USE_LWIP
struct netif *get_lwip_net_interface();
#endif
int start_sta(const char* ssid, const char* passwd, char need_passwd)
{
    WIFINetworkParams_t xNetworkParams = {0};
    WIFIReturnCode_t xWifiStatus;
    int retry_cnt = 0;
#if !USE_LWIP
    setDhcpClientState(1);
    vDHCPProcess(1, eInitialWait);
    xSendDHCPEvent();
#endif
    WIFI_Context_init();
    if (g_current_mode != eWiFiModeStation) {
	    g_current_mode = eWiFiModeStation;
		 WIFI_SetMode(eWiFiModeStation);
		 printf("Current mode is not sta, so reboot wifi\r\n");
        WIFI_Off();
     	  vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    xWifiStatus = WIFI_On();

    if( xWifiStatus == eWiFiSuccess ) {
        printf("WiFi module initialized.\r\n");
    } else {
        printf("WiFi module failed to initialize.\r\n" );
        // Handle module init failure
        return -1;
    }

    /* Some boards might require additional initialization steps to use the Wi-Fi library. */
	char *buff;
	buff = malloc(sizeof(char)*2732);
	WLAN_SCAN_RESULT *result = (WLAN_SCAN_RESULT *)buff;
	memset(buff, 0, 2732);
	WLAN_BSS_INFO *info = result->bss_info;
	atbm_wifi_scan_network(buff,2732);
	for(int i =0;i < result->count; i++)
	{
		char scan_ssid[33] = {0};
		int scan_ssid_len = info->SSID_len;
		int scan_channel = info->chanspec ? info->chanspec : info->ctl_ch;

		if (scan_ssid_len > 32) {
			scan_ssid_len = 32;
		}
		memcpy(scan_ssid, info->SSID, scan_ssid_len);

		printf("\nAPIndex:%d\n",i);
		printf("SSID:%s\n",(char *)info->SSID);
		printf("channel=%d ctl_ch=%d\n",info->chanspec, info->ctl_ch);
		printf("security:%d\n",info->security);	
		printf("rssi:%d\n",info->RSSI);	
		if (strcmp((char *)info->SSID, ssid) == 0 && scan_channel > 0) {
			g_sta_channel = scan_channel;
			printf("sta scan matched ssid:%s channel=%d\r\n", scan_ssid, g_sta_channel);
		}

		printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",info->BSSID[0],info->BSSID[1],info->BSSID[2],info->BSSID[3],info->BSSID[4],info->BSSID[5]);	
		info++;
	}
	free(buff);
    /* Setup parameters. */
    memset(&xNetworkParams, 0, sizeof(xNetworkParams));
    xNetworkParams.ucSSIDLength = strlen( ssid );
    memcpy(xNetworkParams.ucSSID, ssid, xNetworkParams.ucSSIDLength);
    xNetworkParams.xPassword.xWPA.ucLength = strlen( passwd );
    memcpy(xNetworkParams.xPassword.xWPA.cPassphrase, passwd, xNetworkParams.xPassword.xWPA.ucLength);
    if (need_passwd)
        xNetworkParams.xSecurity = eWiFiSecurityWPA2;
    else
        xNetworkParams.xSecurity = eWiFiSecurityOpen;

retry:
    // Connect!
    xWifiStatus = WIFI_ConnectAP( &( xNetworkParams ) );

    if( xWifiStatus == eWiFiSuccess ){
        printf( "WiFi Connected to AP.\r\n" );
		get_sta_channel_or_default(ATBM_DEFAULT_P2P_CHANNEL);
    } else {
        printf( "WiFi failed to connect to AP.\r\n" );
        // Handle connection failure

        if (retry_cnt++ < 5)
            goto retry;
        return -1;
    }
#if !USE_LWIP
    vDHCPProcess(1, eWaitingSendFirstDiscover);
    xSendDHCPEvent();
#endif
exit:
    return 0;
}

int start_p2p(const char *dev_name, const char *ssid, const char *passwd)
{
	//char cmd[128] = {0};
	int p2p_channel;
#if !USE_LWIP
	uint32_t IPAddress;
	setDhcpClientState(0);
#endif

	WIFI_Context_init();
	if (g_current_mode != eWiFiModeP2P || !wifi_started) {
		//char mac[6] = {0};
		printf("atbm current mode is not p2p\r\n");
		g_current_mode = eWiFiModeP2P;
		//WIFI_SetMode(eWiFiModeP2P);

		if (wifi_started) {
			printf("wifi is started, keep existing STA for P2P coexist\r\n");
		}

		wifi_started = true;
		(void)dev_name;
		p2p_channel = get_sta_channel_or_default(ATBM_DEFAULT_P2P_CHANNEL);

		printf("start_p2p:dev_name:%s, (char *)ssid:%s, passwd:%s\r\n", (char *)dev_name, (char *)ssid, (char *)passwd);
		printf("start_p2p: selected channel=%d\r\n", p2p_channel);
		atbm_wifi_p2p_user_config((char *)dev_name,  (char *)ssid, (char *)passwd, p2p_channel);
		atbm_wifi_p2p_start();
		vTaskDelay(pdMS_TO_TICKS(2000));
		atbm_wifi_p2p_go_start();
		vTaskDelay(pdMS_TO_TICKS(50));
	}
#if !USE_LWIP
	IPAddress = (20 << 24) | (13 << 16) | (168 << 8) | (192 << 0);
	dhcpserver_start(ucIPAddressAp, IPAddress, 10);
#else
	//dhcpd_start("wi");
#endif

	return 0;
}


void iwpriv_dbg_btco()
{
}

/*static void iwpriv_task_proc(void* arg)
{
	vTaskDelay(pdMS_TO_TICKS(2000));
	while(1) {
		cmd_test("iwpriv dbg btco");
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
	vTaskDelete(NULL);
}*/

void enable_btco_log()
{
	//xTaskCreate(iwpriv_task_proc, "iwpriv_task",  4096, NULL, configMAX_PRIORITIES / 3, NULL);
}

//static rtw_custom_ie_t g_wifi_ie[1] = {{NULL, PROBE_RSP | BEACON}};

void wifi_ap_set_vendor_ie(const uint8_t* ie)
{
	//g_wifi_ie->ie = (__u8 *)ie;
	//wifi_add_custom_ie((void *)g_wifi_ie, 1);
}

#if (!CARLINK_EY && !CARLINK_EC) && (USE_LWIP == 1)
eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress )
{
	(void)eDHCPPhase;
	(void)ulIPAddress;
	return eDHCPStopNoChanges;
}
#endif

