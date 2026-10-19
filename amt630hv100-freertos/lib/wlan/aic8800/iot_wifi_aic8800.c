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

#include "semphr.h"
#include "board.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_DHCP_Server.h"
#if USE_LWIP
#include "dhcp.h"
#endif
#include "aicwf_main.h"

static AIC_WIFI_MODE g_wifi_mode = WIFI_MODE_STA;
static SemaphoreHandle_t xWiFiSem;
static bool wifi_started;
static bool wifi_init_done = false;

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_On( void )
{
	WIFIReturnCode_t ret = eWiFiFailure;
        
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
        
        if (!wifi_init_done) {
          //aic_wifi_reset();
          aic_sdio_drv_register();
          wifi_init_done = true;
        }
        
	if (wifi_started) {
		ret = eWiFiSuccess;
		goto exit;
	}
		
    if (wifi_on(g_wifi_mode) < 0) {
		printf("\r\nopen wifi failed \r\n");
		goto exit;
	}
	
	wifi_started = true;
	ret = eWiFiSuccess;
exit:
	xSemaphoreGive(xWiFiSem);
    return ret;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
    wifi_off();
	wifi_started = false;
	xSemaphoreGive(xWiFiSem);
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

static int convertWIFISecurity2aic_security(WIFISecurity_t type)
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
#if 0
static WIFISecurity_t convertaic_security2WIFISecurity(int type)
{
	WIFISecurity_t	security_type = eWiFiSecurityNotSupported;

	switch (type) {
	case WLAN_WPA_AUTH_NONE:
		security_type = eWiFiSecurityOpen;
		break;
	case WLAN_WPA2_AUTH_PSK:
		security_type = eWiFiSecurityWPA2;
		break;
	default:
		break;
	}
	return security_type;
}
#endif

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
	int	security_type;
	int ret = AIC_ERROR;
    if (pxNetworkParams == NULL)
		return eWiFiFailure;
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
	security_type = convertWIFISecurity2aic_security(pxNetworkParams->xSecurity);
	if (security_type == WLAN_WPA2_AUTH_PSK) {
		ret = wpa_sta_connect((const char *)pxNetworkParams->ucSSID,
			(const char *)pxNetworkParams->xPassword.xWPA.cPassphrase,
			0);
	} else if (security_type == WLAN_WPA_AUTH_NONE) {
		ret = wpa_sta_connect((const char *)"", (const char *)"", 0);
	}
	xSemaphoreGive(xWiFiSem);
	if (ret == AIC_ERROR)
		return eWiFiFailure;
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
	xSemaphoreTake(xWiFiSem, portMAX_DELAY );
    wpa_sta_disconnect();
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
		g_wifi_mode = WIFI_MODE_STA;
		break;
	case eWiFiModeAP:
		ret = eWiFiSuccess;
		g_wifi_mode = WIFI_MODE_AP;
		break;
	case eWiFiModeP2P:
		ret = eWiFiSuccess;
		g_wifi_mode = WIFI_MODE_P2P;
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
	case WIFI_MODE_STA:
		ret = eWiFiSuccess;
		*pxDeviceMode = eWiFiModeStation;
		break;
	case WIFI_MODE_AP:
		ret = eWiFiSuccess;
		*pxDeviceMode = eWiFiModeAP;
		break;
	case WIFI_MODE_P2P:
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

int wifi_get_mac_address(char * mac)
{
    char buf[32] = {0};
    char *ptr = NULL;
    struct mac_addr *addr = aic_get_bssid();
    ptr = (char *)(addr->array);
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
    memcpy(mac, buf, strlen(buf));
    return 0;
}

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    struct mac_addr *addr = aic_get_bssid();
    
    memcpy((void*)pucMac, (void*)addr->array, 6);
	
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
	
    return eWiFiNotSupported;
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
    return eWiFiNotSupported;
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
#if 0
WIFIReturnCode_t WIFI_RegisterEvent( WIFIEventType_t xEventType,
                                     WIFIEventHandler_t xHandler )
{
    return (WIFIReturnCode_t)wifi_register_user_event(xEventType, xHandler);
}
#endif
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

#if !USE_LWIP
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

static const uint8_t ucIPAddressAp[4] = {192, 168, 13, 1};
#else
void start_ping(const char *ipaddr)
{
	(void)ipaddr;
}
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
	AIC_WIFI_SECURITY_TYPE security_type = KEY_WPA2;
#if !USE_LWIP
	uint32_t IPAddress;
	setDhcpClientState(0);
	vDHCPProcess(1, eInitialWait);
	xSendDHCPEvent();
#endif
	WIFI_Context_init();
	if (g_current_mode != eWiFiModeAP || !wifi_started) {
		printf("current mode is not ap\r\n");
		g_current_mode = eWiFiModeAP;
		WIFI_SetMode(eWiFiModeAP);
		
		if (wifi_started) {
			printf("wifi is start, so reboot wifi\r\n");
			WIFI_Off();
		} else {
                  WIFI_On();
                }
		wifi_started = true;
		if (!need_passwd)
			security_type = KEY_NONE;
		if (ssid == NULL) {
			ssid = "";
		}

		if (passwd == NULL) {
			passwd = "";
		}
		aic_wlan_start_ap(ssid, passwd, channel, security_type);
	}
#if !USE_LWIP
	IPAddress = (20 << 24) | (13 << 16) | (168 << 8) | (192 << 0);
	dhcpserver_start(ucIPAddressAp, IPAddress, 10);
#endif
	return 0;
}

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

    /* Setup parameters. */
    memset(&xNetworkParams, 0, sizeof(xNetworkParams));
	if (ssid) {
		xNetworkParams.ucSSIDLength = strlen( ssid );
		memcpy(xNetworkParams.ucSSID, ssid, xNetworkParams.ucSSIDLength);
	}

	if (passwd) {
		xNetworkParams.xPassword.xWPA.ucLength = strlen( passwd );
		memcpy(xNetworkParams.xPassword.xWPA.cPassphrase, passwd, xNetworkParams.xPassword.xWPA.ucLength);
	}
    if (need_passwd)
        xNetworkParams.xSecurity = eWiFiSecurityWPA2;
    else
        xNetworkParams.xSecurity = eWiFiSecurityOpen;

retry:
    // Connect!
    xWifiStatus = WIFI_ConnectAP( &( xNetworkParams ) );

    if( xWifiStatus == eWiFiSuccess ){
        printf( "WiFi Connected to AP.\r\n" );
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

    return 0;
}

int start_p2p(const char *dev_name, const char *ssid, const char *passwd)
{
	WIFI_Context_init();
	if (g_current_mode != eWiFiModeP2P || !wifi_started) {
		//char mac[6] = {0};
		printf("current mode is not p2p\r\n");
		//g_current_mode = eWiFiModeP2P;
		WIFI_SetMode(eWiFiModeP2P);

		if (wifi_started) {
			printf("wifi is start, so reboot wifi\r\n");
			WIFI_Off();
		} else {
                    WIFI_On();
                }

		wifi_started = true;
		aic_wlan_start_p2p(dev_name, ssid, passwd);
	}
	return 0;
}


void iwpriv_dbg_btco()
{
	cmd_test("iwpriv dbg btco");
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
	cmd_test("wifi_debug log btco on");
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


