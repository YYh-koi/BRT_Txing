#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "board.h"
#if !USE_LWIP
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
#else
#include "ethernet.h"
#include "tcpip.h"
#include "lwip/apps/lwiperf.h"
#include "dhcp.h"
#include "lwip/sockets.h"
#endif

#include "mmcsd_core.h"
#include "mfcapi.h"
#include "lcd.h"
#include "timer.h"
#if !USE_LWIP
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_DHCP_Server.h"
#endif
#include "carlink_video.h"
#include "carlink_ey_audio.h"
#include "carlink_ey.h"
#if defined (BT_STACK_BARROT)
#include "brtbt_interface.h"
#define bt_send_at_command(...) brtbt_send_at_command(__VA_ARGS__)
#define bt_register_event_cb(...) brtbt_register_event_cb(__VA_ARGS__)
#define bt_main(...) brt_bt_main(__VA_ARGS__)
#else 
#include "console.h"
#include "fsc_bt.h"
#define bt_send_at_command(...) console_send_atcmd(__VA_ARGS__)
#define bt_register_event_cb(...) console_register_cb(__VA_ARGS__)
#define bt_main(...) fsc_bt_main(__VA_ARGS__)
#endif
#include "iot_wifi.h"
#include "carlink_utils.h"
#include "carlink_common.h"
#include "time.h"
#include "cJSON.h"
#include "FreeRTOS_Sockets.h"
#include "carlink_ey_server.h"

#if CARLINK_EY

#define EY_BT_EVENT_CONNECT               0
#define EY_BT_EVENT_DISCONNECT            1
#define EY_BT_EVENT_NOTIFY_PHONE_TYPE     2
#define EY_BT_EVENT_NOTIFY_SSID           3
#define EY_BT_EVENT_NOTIFY_PASSWD         4
#define EY_BT_EVENT_NOTIFY_CONNECT_STATUS 5
#define EY_BT_EVENT_NOTIFY_PHONE_AP_READY 6
#define EY_BT_EVENT_NOTIFY_USE_TCP        7
#define EY_BT_EVENT_BLE_CONNECT           8
#define EY_CARLINK_EVENT_CONNECT           9
#define EY_CARLINK_EVENT_DISCONNECT           10
struct bt_event
{
    int type;
    uint8_t para[32];
};

static uint8_t                gbt_ready = 0;
static TaskHandle_t       ey_bt_task = NULL;
static TaskHandle_t       ey_network_task = NULL;
static TaskHandle_t       ey_heartbeat_task = NULL;
static QueueHandle_t     ey_bt_event_queue = NULL;
static const uint8_t        ucIPAddress[4] = {192, 168, 13, 1};
static const uint8_t        ucNetMask[4] = {255, 255, 255, 0};
static const uint8_t        ucGatewayAddress[4] = {0, 0, 0, 0};
static const uint8_t        ucDNSServerAddress[4] = {8, 8, 8, 8};
static const uint8_t        ucMACAddress[6] = {0x30, 0x4a, 0x26, 0x78, 0xfd, 0x12};
static uint8_t                ap_ssid[64]     = {"ap63011"};
static uint8_t                ap_passwd[16]  = {"88888888"};
static uint8_t                gphone_type = 0;
//static uint8_t                guse_tcp = 1;
static int                      gWifiOn = 0;
//static int                      gWIfiConnecting = 0;
#define MAX_JSON_LENGTH        4096
#define MAX_JSON_QUEUE_LENGTH  10

typedef struct {
    uint8_t magic[2];        // "EY"
    uint32_t json_length;    
    uint32_t binary_length; 
} __attribute__((packed)) msg_header_11113;


typedef struct {
    //int socket;
    uint8_t* data;
    int length;
    uint8_t* bindata;
    int binlength;
} json_data_message_t;
typedef struct {
    int type;
} activate_info_t;
#if !USE_LWIP
static Socket_t              gClientSocket = FREERTOS_INVALID_SOCKET, gXClientSocket = FREERTOS_INVALID_SOCKET;
static SemaphoreHandle_t    gSocketMutex = NULL;
#else
#define lwip_ipv4_addr(addr) ((addr[0]) | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24))

static struct netif  gnetif[4];
static int  lwip_tcpip_init_done_flag = 0;
static int  lwip_dhcp_server_start_flag = 0;

extern err_t dhcp_server_start(struct netif *netif, ip4_addr_t *start, ip4_addr_t *end);
extern err_t wlan_ethernetif_init(struct netif *netif);

static void notify_phone_ip_addr(uint32_t ipAddr);
static TaskHandle_t xJSONTaskHandle = NULL;
static QueueHandle_t xJSONDataQueue = NULL;
static SemaphoreHandle_t xJsonSocketMutex = NULL;
static SemaphoreHandle_t xJsonQueueMutex = NULL;


static void ey_dump_ip_addr(const char *msg, const ip_addr_t *server_addr)
{
	printf("%s %d.%d.%d.%d \r\n", msg,
              ip4_addr1_16(ip_2_ip4(server_addr)), ip4_addr2_16(ip_2_ip4(server_addr)), ip4_addr3_16(ip_2_ip4(server_addr)), ip4_addr4_16(ip_2_ip4(server_addr)));
}

static void ey_dhcp_client_status_callback(struct netif *netif, int status, const ip_addr_t *server_addr)
{
	if (&gnetif[0] == netif) {
		ey_dump_ip_addr("dhcp server ip     :", (const ip_addr_t *)server_addr);
		ey_dump_ip_addr("dhcp client ip     :", (const ip_addr_t *)&netif->ip_addr);
		ey_dump_ip_addr("dhcp client netmask:", (const ip_addr_t *)&netif->netmask);
		ey_dump_ip_addr("dhcp client gw     :", (const ip_addr_t *)&netif->gw);

		printf("ey dhcp addr : %08x\r\n", netif->ip_addr.addr);

		notify_phone_ip_addr((uint32_t)netif->ip_addr.addr);
	}
}

static void ey_lwiperf_report_cb_impl(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
	(void)arg;

	printf("lwiperf_report_cb_impl bytes:%d %d ms \r\n", bytes_transferred, ms_duration);
}


static void ey_lwip_tcpip_init_done(void *arg)
{
	(void)arg;
}

static void ey_lwip_server_init()
{
	ip4_addr_t ip_addr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
	ip4_addr_t dhcp_addr_start;
	ip4_addr_t dhcp_addr_end;

	ip_addr.addr = lwip_ipv4_addr(ucIPAddress);
	netmask.addr = lwip_ipv4_addr(ucNetMask);
	gw.addr      = lwip_ipv4_addr(ucGatewayAddress);
	if (!lwip_tcpip_init_done_flag) {
		tcpip_init(ey_lwip_tcpip_init_done, NULL);
		lwip_tcpip_init_done_flag = 1;
	} else {
		netif_set_down(&gnetif[0]);
		netif_remove(&gnetif[0]);
	}
	netif_add(&gnetif[0],
#if LWIP_IPV4
		&ip_addr, &netmask, &gw,
#endif
		NULL, wlan_ethernetif_init, tcpip_input);
	
	netif_set_default(&gnetif[0]);

	if (!lwip_dhcp_server_start_flag) {
		uint8_t 	 addr_start[4] = {192, 168, 13, 20};
		uint8_t 	 addr_end[4] = {192, 168, 13, 30};

		/*memcpy(addr_start, ucIPAddress, 4);
		memcpy(addr_end,   ucIPAddress, 4);
		addr_end[3] = 30;*/
		dhcp_addr_start.addr 	= lwip_ipv4_addr(addr_start);
		dhcp_addr_end.addr 		= lwip_ipv4_addr(addr_end);
		dhcp_server_start(&gnetif[0], &dhcp_addr_start, &dhcp_addr_end);

		lwiperf_start_tcp_server_default(ey_lwiperf_report_cb_impl, NULL);//iperf test
		lwip_dhcp_server_start_flag = 1;
	}
}

static void ey_lwip_client_init()
{
	ip4_addr_t ip_addr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
	ip4_addr_t dhcp_addr_start;
	ip4_addr_t dhcp_addr_end;

	ip_addr.addr = lwip_ipv4_addr(ucIPAddress);
	netmask.addr = lwip_ipv4_addr(ucNetMask);
	gw.addr      = lwip_ipv4_addr(ucGatewayAddress);
	if (!lwip_tcpip_init_done_flag) {
		tcpip_init(ey_lwip_tcpip_init_done, NULL);
		lwip_tcpip_init_done_flag = 1;
	} else {
		netif_set_down(&gnetif[0]);
		netif_remove(&gnetif[0]);
	}

	netif_add(&gnetif[0],
#if LWIP_IPV4
		&ip_addr, &netmask, &gw,
#endif
		NULL, wlan_ethernetif_init, tcpip_input);
	netif_set_default(&gnetif[0]);
	dhcp_regisger_status_callback(ey_dhcp_client_status_callback);
	netif_set_up(&gnetif[0]);
	dhcp_start(&gnetif[0]);
}

#endif

void cmd_test(const char* temp_uart_buf);

static void ey_reset_wifi_ap_info(const char *btmac)
{
    sprintf((char *)ap_ssid,   "AP630_%s", btmac);
}

static void notify_carlink_state(int state)
{
    struct bt_event ev = {0};
    ev.type = -1;

    if (state == 0) {
        ev.type = EY_CARLINK_EVENT_DISCONNECT;

        ark_lcd_osd_enable(LCD_VIDEO_LAYER, 0);
        ark_lcd_osd_enable(LCD_UI_LAYER, 1);
        ark_lcd_set_osd_sync(LCD_VIDEO_LAYER);
        ark_lcd_set_osd_sync(LCD_UI_LAYER);

    } else {
        ev.type = EY_CARLINK_EVENT_CONNECT;

        ark_lcd_osd_enable(LCD_VIDEO_LAYER, 1);
        ark_lcd_osd_enable(LCD_UI_LAYER, 0);
        ark_lcd_set_osd_sync(LCD_VIDEO_LAYER);
        ark_lcd_set_osd_sync(LCD_UI_LAYER);
    }

    if (0 != ev.type && NULL != ey_bt_event_queue) {
        xQueueSend(ey_bt_event_queue, &ev, 0);
    }
}

static void notify_phone_ip_addr(uint32_t ipAddr);
static void ey_wifi_start_ap(const uint8_t* ssid, const uint8_t* passwd, int channel)
{
    char cmd[64] = {0};
    uint32_t IPAddress = (32 << 24) | (13 << 16) | (168 << 8) | (192 << 0);

    if (gWifiOn == 0) {
        printf("Turning on wifi ap...\r\n");
        WIFI_SetMode(eWiFiModeAP);
        gWifiOn = 1;
        //WIFI_On();
        sprintf(cmd, "wifi_ap %s %d %s", ssid, channel, passwd);
        cmd_test(cmd);

        IPAddress = (20 << 24) | (13 << 16) | (168 << 8) | (192 << 0);
#if !USE_LWIP
        dhcpserver_start(ucIPAddress, IPAddress, 10);
#endif
    }
}

#define ucNumNetworks  12

static int ey_wifi_connect_remote_ap(const char* ssid, const char* passwd)
{
    WIFINetworkParams_t xNetworkParams = {0};
    WIFIReturnCode_t xWifiStatus;
    int retry  = 0;

    WIFI_Context_init();

    if (gWifiOn == 0) {
        WIFI_SetMode(eWiFiModeStation);

        printf("Turning on wifi...\r\n");
        xWifiStatus = WIFI_On();

        printf("Checking status...\r\n");
        if( xWifiStatus == eWiFiSuccess ) {
        printf("WiFi module initialized.\r\n");
        } else {
        printf("WiFi module failed to initialize.\r\n" );
        return -1;
        }
        gWifiOn = 1;
    }

	memset(&xNetworkParams, 0, sizeof(xNetworkParams));
	xNetworkParams.ucSSIDLength = strlen( ssid );
	memcpy(xNetworkParams.ucSSID, ssid, xNetworkParams.ucSSIDLength);
	xNetworkParams.xPassword.xWPA.ucLength = strlen( passwd );
	memcpy(xNetworkParams.xPassword.xWPA.cPassphrase, passwd, xNetworkParams.xPassword.xWPA.ucLength);
	xNetworkParams.xSecurity = eWiFiSecurityWPA2;
wifi_conn_retry:

	xWifiStatus = WIFI_ConnectAP( &( xNetworkParams ) );

	if( xWifiStatus == eWiFiSuccess ) {
		printf( "WiFi Connected to AP.\r\n" );
	} else {
		printf( "WiFi failed to connect to AP.\r\n" );
		if (retry++ < 15) {vTaskDelay(pdMS_TO_TICKS(300));
			goto wifi_conn_retry;}
		return -1;
	}
	return 0;
}

#if ( ipconfigUSE_DHCP_HOOK != 0 && !USE_LWIP )
eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
    eDHCPCallbackAnswer_t eReturn;
    //uint32_t ulStaticIPAddress, ulStaticNetMask;
    char ip_str[20] = {0};
    //const uint8_t        ucIPAddressDef[4] = {192, 168, 13, 37};
    
    sprintf(ip_str, "%d.%d.%d.%d\r\n", (ulIPAddress >> 0) & 0xFF, 
    	(ulIPAddress >> 8) & 0xFF, (ulIPAddress >> 16) & 0xFF, (ulIPAddress >> 24) & 0xFF);
    printf("\r\n  eDHCPPhase:%d  ulIPAddress:%s state:%d \r\n", eDHCPPhase, ip_str, getDhcpClientState());
    if (getDhcpClientState() == 0)
        return eDHCPStopNoChanges;

    switch( eDHCPPhase )
    {
        case eDHCPPhaseFinished:
            notify_phone_ip_addr(ulIPAddress);//FreeRTOS_GetIPAddress()
            eReturn = eDHCPContinue;
        break;
        case eDHCPPhasePreDiscover  :
            eReturn = eDHCPContinue;
        break;

        case eDHCPPhasePreRequest  :
            eReturn = eDHCPContinue;
        break;
        default :
            eReturn = eDHCPContinue;
        break;
    }

    return eReturn;
}
#endif
static void notify_phone_ip_addr(uint32_t ipAddr)
{
    char at_cmd[64] = {0};

    if (!gbt_ready)
        return;
    memset(at_cmd, 0, sizeof(at_cmd));
    //uint32_t IPAddress = (32 << 24) | (13 << 16) | (168 << 8) | (192 << 0);
    sprintf(at_cmd, "AT+IPADDR=%d.%d.%d.%d\r\n", (ipAddr >> 0) & 0xFF, 
    	(ipAddr >> 8) & 0xFF, (ipAddr >> 16) & 0xFF, (ipAddr >> 24) & 0xFF);
    printf("at_cmd:%s\r\n", at_cmd);
    bt_send_at_command(at_cmd, strlen(at_cmd));
}

static void notify_iphone_ssid_passwd()
{
    char at_cmd[128]            = {0};
    char bt_ssid_hex_str[64]    = {0};
    char bt_passwd_hex_str[32]  = {0};

    if (!gbt_ready)
        return;
    
    memset(at_cmd, 0, sizeof(at_cmd));

    str2asiistr((const char *)ap_ssid, strlen((const char *)ap_ssid), bt_ssid_hex_str, 2 * strlen((const char *)ap_ssid));
    printf("bt_ssid_hex_str:%s\r\n", bt_ssid_hex_str);
    sprintf(at_cmd, "AT+GATTSET=AAA2%s\r\n", bt_ssid_hex_str);
    printf("ssid   at:%s     |||  \r\n", at_cmd);
    bt_send_at_command(at_cmd, strlen(at_cmd));

    memset(at_cmd, 0, sizeof(at_cmd));
    str2asiistr((const char *)ap_passwd, strlen((const char *)ap_passwd), bt_passwd_hex_str, 2 * strlen((const char *)ap_passwd));
    sprintf(at_cmd, "AT+GATTSET=AAA3%s\r\n", bt_passwd_hex_str);
    printf("passwd at:%s\r\n", at_cmd);
    bt_send_at_command(at_cmd, strlen(at_cmd));
}

static void notify_iphone_ap_connect_status(int status)
{
    char at_cmd[128]             = {0};
    char json_str[64]              = {0};
    char json_hex_str[128]     = {0};

    if (!gbt_ready)
        return;

    sprintf(json_str, "{\"fun\": \"hotspot\",\"type\": \"%d\"}", status);
    printf("\r\n%s    \r\n", json_str);
    str2asiistr(json_str, strlen(json_str), json_hex_str, 2 * strlen(json_str));
    sprintf(at_cmd, "AT+GATTSET=AAA7%s\r\n", json_hex_str);
    printf("ssid   at:%s\r\n", at_cmd);
    bt_send_at_command(at_cmd, strlen(at_cmd));
}

#if !USE_LWIP
static int readn(Socket_t xSocket, uint8_t * pvBuffer, size_t uxBufferLength)
{
    int totalLen = 0, lBytes, retry_count = 0;

    while (uxBufferLength > 0) {
        lBytes = FreeRTOS_recv( xSocket, (void *)pvBuffer, uxBufferLength, 0);
        if (lBytes < 0) {
            printf("FreeRTOS_recv err:%d\r\n", lBytes);
            totalLen = -1;
            break;
        }
        if (lBytes == 0) {
            printf("FreeRTOS_recv lBytes:%d\r\n", lBytes);
            if (retry_count++ > 1)
                break;
        }
        pvBuffer       += lBytes;
        uxBufferLength -= lBytes;
        totalLen       += lBytes;
    }
    return totalLen;
}


static void set_socket_win(Socket_t xSockets)
{
    WinProperties_t xWinProperties;

    memset(&xWinProperties, '\0', sizeof xWinProperties);

    xWinProperties.lTxBufSize   = ipconfigIPERF_TX_BUFSIZE;	/* Units of bytes. */
    xWinProperties.lTxWinSize   = ipconfigIPERF_TX_WINSIZE;	/* Size in units of MSS */
    xWinProperties.lRxBufSize   = ipconfigIPERF_RX_BUFSIZE;	/* Units of bytes. */
    xWinProperties.lRxWinSize   = ipconfigIPERF_RX_WINSIZE; /* Size in units of MSS */

    FreeRTOS_setsockopt( xSockets, 0, FREERTOS_SO_WIN_PROPERTIES, ( void * ) &xWinProperties, sizeof( xWinProperties ) );
}

static void ey_send_hu_info(Socket_t xSockets)
{
    uint8_t reply[13] = {0};

    reply[0] = 'O'; reply[1] = 'K';
    WRITE_LE16(reply + 2, get_carlink_video_width());
    WRITE_LE16(reply + 4, get_carlink_video_height());
    WRITE_LE16(reply + 6, get_carlink_video_width());
    WRITE_LE16(reply + 8, get_carlink_video_height());
    reply[10] = get_carlink_video_fps();
    WRITE_LE16(reply + 11, 2000);

    FreeRTOS_send(xSockets, reply, sizeof(reply), 0);
}
uint32_t gey_ts = 0;
#define EY_DEBUG  0

static void ey_network_task_proc(void* arg)
{
    SocketSet_t xFD_Set;
    BaseType_t xResult;
    Socket_t xSockets = FREERTOS_INVALID_SOCKET;
    Socket_t xClientSocket = FREERTOS_INVALID_SOCKET;
    Socket_t xXSSockets = FREERTOS_INVALID_SOCKET;
    Socket_t xXSClientSocket = FREERTOS_INVALID_SOCKET;
    static const TickType_t xNoTimeOut = pdMS_TO_TICKS( 2000 );
    struct freertos_sockaddr xAddress, xRemoteAddr;
    uint32_t xClientLength = sizeof( struct freertos_sockaddr );
    int32_t lBytes;
    uint8_t *header_ptr;
    msg_header   header;
#if EY_DEBUG
    void *h264SrcBuf = NULL;
#endif
    uint8_t *h264SrcBufPtr = NULL;
    int32_t h264SrcSize = 0;
    int i;
    video_frame_s* frame = NULL;
    TickType_t xBlockingTime = pdMS_TO_TICKS( 2000 );uint32_t ts/*, r_ts*/;
    int cur_data_size = 0, pre_data_size = 0, retry_count;
    int err_flag = 0;

#if EY_DEBUG
    h264SrcBuf = pvPortMalloc(H264DEC_INBUF_SIZE);
    if(!h264SrcBuf) {
        printf("%s, h264SrcBuf pvPortMalloc failed.\n", __func__);
        return ;
    }
#endif
#if DISABLE_CARLINK_H264_FRAME_BUF
	void* video_handle = NULL;
#endif
    xFD_Set = FreeRTOS_CreateSocketSet();
    xSockets = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
    configASSERT( xSockets != FREERTOS_INVALID_SOCKET );
    FreeRTOS_setsockopt(xSockets, 0, FREERTOS_SO_RCVTIMEO, &xNoTimeOut, sizeof(xNoTimeOut));
    //FreeRTOS_setsockopt(xXSSockets, 0, FREERTOS_SO_REUSE_LISTEN_SOCKET, (void *)&option, sizeof(option));
    set_socket_win(xSockets);
    xAddress.sin_port = ( uint16_t ) 11111;
    xAddress.sin_port = FreeRTOS_htons( xAddress.sin_port );
    FreeRTOS_bind( xSockets, &xAddress, sizeof( xAddress ) );
    FreeRTOS_listen( xSockets, 1 );

    xXSSockets = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
    configASSERT( xXSSockets != FREERTOS_INVALID_SOCKET );
    FreeRTOS_setsockopt(xXSSockets, 0, FREERTOS_SO_RCVTIMEO, &xNoTimeOut, sizeof(xNoTimeOut));
    //FreeRTOS_setsockopt(xXSSockets, 0, FREERTOS_SO_REUSE_LISTEN_SOCKET, (void *)&option, sizeof(option));
    set_socket_win(xXSSockets);
    xAddress.sin_port = ( uint16_t ) 11113;
    xAddress.sin_port = FreeRTOS_htons( xAddress.sin_port );
    FreeRTOS_bind( xXSSockets, &xAddress, sizeof( xAddress ) );
    FreeRTOS_listen( xXSSockets, 1 );

    while(1) {
        if (err_flag != 0) {
            xSemaphoreTake(gSocketMutex, portMAX_DELAY);
            if (FREERTOS_INVALID_SOCKET != xClientSocket) {
                FreeRTOS_FD_CLR(xClientSocket, xFD_Set, eSELECT_READ);
                FreeRTOS_closesocket(xClientSocket);
                xClientSocket = FREERTOS_INVALID_SOCKET;
            }
            if (FREERTOS_INVALID_SOCKET != xXSClientSocket) {
                FreeRTOS_FD_CLR(xXSClientSocket, xFD_Set, eSELECT_READ);
                FreeRTOS_closesocket(xXSClientSocket);
                xXSClientSocket = FREERTOS_INVALID_SOCKET;
            }
            
            gClientSocket   = FREERTOS_INVALID_SOCKET;
            gXClientSocket = FREERTOS_INVALID_SOCKET;
            xSemaphoreGive(gSocketMutex);
            notify_carlink_state(0);
            err_flag = 0;
        }
        FreeRTOS_FD_CLR(xSockets, xFD_Set, eSELECT_READ);
        FreeRTOS_FD_SET(xSockets, xFD_Set, eSELECT_READ);
        FreeRTOS_FD_CLR(xXSSockets, xFD_Set, eSELECT_READ);
        FreeRTOS_FD_SET(xXSSockets, xFD_Set, eSELECT_READ);
        if (xClientSocket && xClientSocket != FREERTOS_INVALID_SOCKET) {
            FreeRTOS_FD_CLR(xClientSocket, xFD_Set, eSELECT_READ);
            FreeRTOS_FD_SET(xClientSocket, xFD_Set, eSELECT_READ);
        }
        if (xXSClientSocket && xXSClientSocket != FREERTOS_INVALID_SOCKET) {
            FreeRTOS_FD_CLR(xXSClientSocket, xFD_Set, eSELECT_READ);
            FreeRTOS_FD_SET(xXSClientSocket, xFD_Set, eSELECT_READ);
        }
        xResult = FreeRTOS_select( xFD_Set, xBlockingTime );
        if (xResult == 0) {//printf("select timeout\r\n");
            if (FREERTOS_INVALID_SOCKET != xClientSocket) {//maybe socket don't recv close info;
                if (cur_data_size == pre_data_size) {// no data recv
                    printf("no data, retry count:%d\r\n", retry_count);
                    if (retry_count++ >= 1) {// no data for 4s
                        err_flag = 1;
                        notify_carlink_state(0);
                    }
                }
                pre_data_size = cur_data_size;
            }
            xBlockingTime = pdMS_TO_TICKS( 2000 );
            continue;
        }
        if (xResult < 0) {
            break;
        }
        pre_data_size = cur_data_size;

        if (0){
        } else if(FreeRTOS_FD_ISSET ( xSockets, xFD_Set )) {
            xClientSocket = FreeRTOS_accept( xSockets, &xRemoteAddr, &xClientLength);
            if((xClientSocket != NULL) && (xClientSocket != FREERTOS_INVALID_SOCKET)) {
                char pucBuffer[32] = {0};
                FreeRTOS_FD_CLR(xClientSocket, xFD_Set, eSELECT_READ);
                FreeRTOS_FD_SET(xClientSocket, xFD_Set, eSELECT_READ);
                FreeRTOS_GetRemoteAddress( xClientSocket, ( struct freertos_sockaddr * ) &xRemoteAddr );
                FreeRTOS_inet_ntoa(xRemoteAddr.sin_addr, pucBuffer );
                set_socket_win(xClientSocket);
                printf("\r\nServer 11111: Received a connection from %s:%u\r\n", pucBuffer, FreeRTOS_ntohs(xRemoteAddr.sin_port));
                cur_data_size = 0;
                pre_data_size = 0;
                retry_count = 0;
                h264_dec_ctx_init();
                notify_carlink_state(1);
                xSemaphoreTake(gSocketMutex, portMAX_DELAY);
                gClientSocket = xClientSocket;
                xSemaphoreGive(gSocketMutex);
            }
            continue;
        } else if (FreeRTOS_FD_ISSET ( xXSSockets, xFD_Set )) {
            xXSClientSocket = FreeRTOS_accept( xXSSockets, &xRemoteAddr, &xClientLength);
            if ((xXSClientSocket != NULL) && (xXSClientSocket != FREERTOS_INVALID_SOCKET)) {
                char pucBuffer[32] = {0};
                FreeRTOS_FD_CLR(xXSClientSocket, xFD_Set, eSELECT_READ);
                FreeRTOS_FD_SET(xXSClientSocket, xFD_Set, eSELECT_READ);
                FreeRTOS_GetRemoteAddress( xXSClientSocket, ( struct freertos_sockaddr * ) &xRemoteAddr );
                FreeRTOS_inet_ntoa(xRemoteAddr.sin_addr, pucBuffer );
                printf("\r\nServer 11113: Received a connection from %s:%u\r\n", pucBuffer, FreeRTOS_ntohs(xRemoteAddr.sin_port));
                set_socket_win(xXSClientSocket);
                xSemaphoreTake(gSocketMutex, portMAX_DELAY);
                gXClientSocket = xXSClientSocket;
                xSemaphoreGive(gSocketMutex);
            }
            continue;
        } else if (FreeRTOS_FD_ISSET ( xXSClientSocket, xFD_Set )) {
            lBytes = FreeRTOS_recv( xXSClientSocket, (void*)&header, sizeof(header), 0);
            if (lBytes < 0) {
                printf("FreeRTOS_recv port 11113 err:%d\r\n", lBytes);
                err_flag = 1;
            } else {
                cur_data_size += lBytes;
            }
            
            continue;
        }

        lBytes = FreeRTOS_recv( xClientSocket, (void*)&header, sizeof(header), 0);

        if (lBytes < 0) {
            printf("FreeRTOS_recv port 11111 err:%d\r\n", lBytes);
            err_flag = 1;
            continue;
        }
        cur_data_size += lBytes;

        if (lBytes == 0)
            continue;

        if (!(header.sync_word[0] == 0xAA && header.sync_word[1] == 0xBB && header.sync_word[2] == 0xCC))
            continue;

        if (header.type == 4) {
            uint16_t height, width;
            header_ptr = (uint8_t *)&header;
            for (i = 0; i < lBytes; i++) {
                printf("%02x ", header_ptr[i]);
            }printf("\r\n");

            height = (header_ptr[12] | (header_ptr[13] << 8));
            width  = (header_ptr[14] | (header_ptr[15] << 8));
            printf("#receive phone info width:%d height:%d\r\n", width, height);gey_ts = get_timer(0);//r_ts = header.ts;

            ey_send_hu_info(xClientSocket);
            continue;
        } else if (header.type == 6) {
            printf("heart beat\r\n");
            continue;
        } else if (header.type != 0) {
            continue;
        }
        
        h264SrcSize    = header.payload_size - 4;//skip timestamp
        if (h264SrcSize <= 0 || h264SrcSize > H264_FRAME_BUF_SIZE) {
            printf("h264 data len:%d err\r\n", h264SrcSize);
            continue;
        }
#if EY_DEBUG
        h264SrcBufPtr  = (uint8_t *)h264SrcBuf;
#else
#if !DISABLE_CARLINK_H264_FRAME_BUF
get_retry:
        frame = get_h264_frame_buf();
        if (NULL == frame) {
            printf("h264 frame is empty\r\n");
            vTaskDelay(pdMS_TO_TICKS(20));
            goto get_retry;
            //continue;
        }

        h264SrcBufPtr  = (uint8_t *)frame->cur;
        frame->len     = h264SrcSize;
#else
		if (NULL == video_handle) {
			video_handle = h264_video_player_init();
		}
		if (NULL == h264SrcBufPtr)
			h264SrcBufPtr = pvPortMalloc(H264DEC_INBUF_SIZE);
		if (NULL == h264SrcBufPtr) {
			printf("h264SrcBufPtr malloc failed\n");
			return;
		}
#endif
#endif
        //ts = get_timer(0);
        //printf("Size:%05d lts:%03d rts:%06d\r\n", h264SrcSize, (ts - gey_ts) / 1000, header.ts - r_ts);
        gey_ts = ts;//r_ts = header.ts;
        lBytes = readn(xClientSocket, h264SrcBufPtr, h264SrcSize);
#if !DISABLE_CARLINK_H264_FRAME_BUF
        if (h264SrcSize != lBytes) {
            set_h264_frame_free(frame);
            if (lBytes == -1) {
                printf("###FreeRTOS_recv port 11111 err:%d\r\n", lBytes);
                err_flag = 1;
            } else{
                cur_data_size += lBytes;
            }
        } else {
            cur_data_size += lBytes;
            notify_h264_frame_ready(&frame);
        }
#else
		 if (h264SrcSize == lBytes) {
		 	h264_video_player_proc(video_handle, h264SrcBufPtr, h264SrcSize);
		 }
#endif
    }

    printf("%s exit...\r\n", __func__);
    xSemaphoreTake(gSocketMutex, portMAX_DELAY);
    if (FREERTOS_INVALID_SOCKET != xSockets)
        FreeRTOS_closesocket(xSockets);
    if (FREERTOS_INVALID_SOCKET != xXSSockets)
        FreeRTOS_closesocket(xXSSockets);
    if (FREERTOS_INVALID_SOCKET != xClientSocket)
        FreeRTOS_closesocket(xClientSocket);
    if (FREERTOS_INVALID_SOCKET != xXSClientSocket)
        FreeRTOS_closesocket(xXSClientSocket);
    gClientSocket   = FREERTOS_INVALID_SOCKET;
    gXClientSocket = FREERTOS_INVALID_SOCKET;
    xSemaphoreGive(gSocketMutex);
#if DISABLE_CARLINK_H264_FRAME_BUF
	h264_video_player_uninit(video_handle);
	if (NULL == h264SrcBufPtr) {
		free(h264SrcBufPtr);
	}
#endif
    vTaskDelete(NULL);
}
#else

static struct timespec timeRecvLast = {0,0};
static int xClientSocket = -1;
static int xXSClientSocket = -1;
static fd_set xFD_Set;
static int heartbeat_flag = 0;
static struct timespec timelast = {0,0};
static int process_flag = 0;
static uint16_t gH264Index = 0;
static int gH264Frames = 0;
static int gCRC16Log = 0;

#define CRC16_ENABLE 0
#ifdef CRC16_ENABLE

//2字节大小端转换 unsigned char
#define sw16(x) \
    ((uint16_t)( \
    (((uint16_t)(x) & (uint16_t)0x00ff) << 8 ) | \
    (((uint16_t)(x) & (uint16_t)0xff00) >> 8 ) ))

unsigned int  read2BytesToInt2( unsigned char *buffer)
{
    return ntohs(*( unsigned short *)(buffer));
}

// 利用32位累加器和循环展开优化的16位校验和
uint16_t calculate_checksum16_fast(const unsigned char *data, size_t length) {
    uint32_t sum = 0;
    size_t i = 0;
    
    // 按4字节块处理（精确匹配Java的LITTLE_ENDIAN解析）
    for (; i + 3 < length; i += 4) {
        // 小端序解析：低地址字节对应整数低位
        uint32_t val = 
            ((uint32_t)data[i] & 0xFF) |
            (((uint32_t)data[i+1] & 0xFF) << 8) |
            (((uint32_t)data[i+2] & 0xFF) << 16) |
            (((uint32_t)data[i+3] & 0xFF) << 24);
        sum += val;
    }
    
    // 处理剩余字节（与Java保持一致的累加方式）
    for (; i < length; i++) {
        sum += (uint32_t)(data[i] & 0xFF);
    }
    
    // 折叠为16位（最多两次运算）
    sum = (sum & 0xFFFF) + (sum >> 16);
    if (sum > 0xFFFF) {
        sum -= 0x10000;  // 替代第二次加法，结果相同但更直观
    }
    
    return (uint16_t)sum;
}

#endif

static int readn(int xSocket, uint8_t *pvBuffer, size_t uxBufferLength, int timeout_ms)
{
    int totalLen = 0, lBytes, retry_count = 0;
    process_flag = 21;
    while (uxBufferLength > 0) {
        process_flag = 22;
        lBytes = recv(xSocket, (void *)pvBuffer, uxBufferLength, 0);
        if (lBytes <= 0) {
            printf("FreeRTOS_recv err:%d\r\n", lBytes);
            totalLen = -1;
            process_flag = 23;
            break;
        }

        pvBuffer += lBytes;
        uxBufferLength -= lBytes;
        totalLen += lBytes;
    }
    process_flag = 24;

    return totalLen;
}


static void set_socket_win(int xSockets)
{
	(void)xSockets;
}

static void ey_send_hu_info(int xSockets)
{
    uint8_t reply[13] = {0};

    reply[0] = 'O'; reply[1] = 'K';
    WRITE_LE16(reply + 2, get_carlink_video_width());
    WRITE_LE16(reply + 4, get_carlink_video_height());
    WRITE_LE16(reply + 6, get_carlink_video_width());
    WRITE_LE16(reply + 8, get_carlink_video_height());
    reply[10] = get_carlink_video_fps();
    WRITE_LE16(reply + 11, 2000);

	send(xSockets, reply, sizeof(reply), 0);
}



//增加心跳检测任务
static void clockTime(struct timespec *time_t)
{
    clock_gettime(CLOCK_MONOTONIC, time_t);
}

static double clockDiff( struct timespec t0,  struct timespec t1,  bool isAbs)
{
    if ((t0.tv_sec == 0 && t0.tv_nsec == 0 ) || (t1.tv_sec == 0 && t1.tv_nsec == 0))
    {
        
        return 0.0;
    }
    double time0, time1;

    time0 = t0.tv_sec + (t0.tv_nsec/1000000000.0);
    time1 = t1.tv_sec + (t1.tv_nsec/1000000000.0);

    time1 = time0 - time1;

    if (time1 < 0 && isAbs)
    {
        time1 = -time1;
    }

    return time1;

}

static void ey_heartbeat_task_proc(void* arg){
    printf("ey_heartbeat_task_proc start \r\n");
    while (1)
    {
        // printf("ey_heartbeat_task_proc run heartbeat_flag:%d tv_nsec:%ld\r\n",heartbeat_flag,timeRecvLast.tv_nsec );
        struct timespec time;
        clockTime(&time);

        if(heartbeat_flag){

            if (timeRecvLast.tv_nsec != 0 && clockDiff(time, timeRecvLast, true) > 5.0)
            {
                printf("ey_heartbeat_task_proc timeout!!!! 5s\r\n");

                if (xClientSocket != -1) {
                    printf("xClientSocket close:%d \r\n",xClientSocket);
                    shutdown(xClientSocket, SHUT_RDWR);
                    FD_CLR(xClientSocket, &xFD_Set);
                    close(xClientSocket);
                    xClientSocket = -1;
                }
                if (xXSClientSocket != -1) {
                    printf("xXSClientSocket close:%d \r\n",xXSClientSocket);
                    shutdown(xXSClientSocket, SHUT_RDWR);
                    FD_CLR(xXSClientSocket, &xFD_Set);
                    close(xXSClientSocket);
                    xXSClientSocket = -1;
                }

                timeRecvLast.tv_nsec = 0;
            }
        }

        if(timelast.tv_nsec == 0 || clockDiff(time, timelast, true) > 5.0){
            clockTime(&timelast);
            printf("ey ==> process_flag:%d frame rate:%df/s  CRC Show:%s\r\n",process_flag,gH264Frames/5,gCRC16Log?"true":"false");
            gH264Frames = 0;

        }

        usleep(100000); //100ms

    }

    printf("ey_heartbeat_task_proc end \r\n");
    
    vTaskDelete(NULL);
}
void parse_activate_info(cJSON* root) {
    cJSON* type_item = cJSON_GetObjectItem(root, "type");
    if (type_item && cJSON_IsString(type_item)) {
        int type = atoi(type_item->valuestring);
        printf("Activation type: %d\r\n", type);
        
        switch (type) {
            case 0: 
                  printf("Not activated\r\n"); 
                  break;
            case 1: 
                  printf("Activated\r\n"); 
                  break;
            case 2:
                  printf("Device not supported\r\n"); 
                  break;
            case 3: 
                  printf("Poor network\r\n"); 
                  break;
            default: 
                  printf("Unknown status\r\n"); 
                  break;
        }
    }
}

void process_json_message(const char* json_str) {
    cJSON* root = cJSON_Parse(json_str);
    if (!root) {
        printf("JSON parse error: %s\r\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON* fun_item = cJSON_GetObjectItem(root, "fun");
    if (!fun_item || !cJSON_IsString(fun_item)) {
        cJSON_Delete(root);
        printf("Missing or invalid 'fun' field\r\n");
        return;
    }

    const char* fun_name = fun_item->valuestring;
    printf("\r\n fun-->: %s\r\n", fun_name);

    if (strcmp(fun_name, "navi") == 0) {
    } else if (strcmp(fun_name, "time") == 0) {
    } else if (strcmp(fun_name, "weather") == 0) {

    } else if (strcmp(fun_name, "activate") == 0) {
        parse_activate_info(root);
    } else if (strcmp(fun_name, "mirror") == 0) {
    } else if (strcmp(fun_name, "disconnect") == 0) { //手机申请断开连接
        printf("recv disconnect cmd from phone\r\n");
        // if (xClientSocket != -1) {
        //     printf("xClientSocket close:%d \r\n",xClientSocket);
        //     shutdown(xClientSocket, SHUT_RDWR);
        //     FD_CLR(xClientSocket, &xFD_Set);
        //     close(xClientSocket);
        //     xClientSocket = -1;
        // }
        // if (xXSClientSocket != -1) {
        //     printf("xXSClientSocket close:%d \r\n",xXSClientSocket);
        //     shutdown(xXSClientSocket, SHUT_RDWR);
        //     FD_CLR(xXSClientSocket, &xFD_Set);
        //     close(xXSClientSocket);
        //     xXSClientSocket = -1;
        // }

        kick_all_clients();
    } 
    else {
        printf("Unknown function: %s\r\n", fun_name);
    }

    cJSON_Delete(root);

    printf("process_json_message end\r\n");
}


static int read_11113_data(int socket, uint8_t** data) {
    msg_header_11113 header;
    int result;
    fd_set xFD_Set;
    
    //if (xSemaphoreTake(xJsonSocketMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
    //    return -1;
    //}
    
    result = readn( socket, (void*)&header, sizeof(header), 4000);//recv(socket, &header, sizeof(header), MSG_WAITALL);
    if (result < 0) {
        //xSemaphoreGive(xJsonSocketMutex);
        //vTaskDelay(pdMS_TO_TICKS(50));
        switch(errno)
        {
        case ECONNRESET:
              printf("Connection reset\r\n");
              break;
        case ETIMEDOUT:
              printf("Connection timed out\r\n");
              break;
        case ENOTCONN:
              printf("Not Connected\r\n");
              break;
        case EBADF:
              printf("Bad file descriptor\r\n");
              break;
        default:
          printf("Other error :%d\r\n",errno);
              break;
        }
        close(socket); 
        FD_CLR(socket, &xFD_Set);
        return -1;
    }
    
    if (header.magic[0] != 'E' || header.magic[1] != 'Y') {
        //xSemaphoreGive(xJsonSocketMutex);
        return -1;
    }
    
    uint32_t json_length = ntohl(header.json_length);
    if (json_length == 0 || json_length > MAX_JSON_LENGTH) {
        //xSemaphoreGive(xJsonSocketMutex);
        return -1;
    }
    
    *data = pvPortMalloc(json_length + 1);
    if (!*data) {
        //xSemaphoreGive(xJsonSocketMutex);
        return -1;
    }
    
    result = readn(socket, *data, json_length,4000);
    if (result != json_length) {
        vPortFree(*data);
        *data = NULL;
        //xSemaphoreGive(xJsonSocketMutex);
        return -1;
    }
    
    (*data)[json_length] = '\0';
    //xSemaphoreGive(xJsonSocketMutex);
    return json_length;
}

static void json_data_task(void* pvParameters) {
    json_data_message_t message;
         
    while (1) {

        if (xQueueReceive(xJSONDataQueue, &message, portMAX_DELAY) == pdPASS) {
            if (message.data && message.length > 0) {
                printf("Received [%d-%d] JSON data: %s\n",message.length,message.binlength, (char*)message.data);
               process_json_message((const char*)message.data);
               printf("json_data_task end vPortFree data\r\n");
                vPortFree(message.data);
                printf("json_data_task end vPortFree bindata\r\n");
                if(message.binlength > 0 && message.bindata){
                    vPortFree(message.bindata);
                }
                
                printf("json_data_task end\r\n");
            }
        }      
    }
    
    vTaskDelete(NULL);
}
uint32_t gey_ts = 0;
#define EY_DEBUG  0
static void ey_network_task_proc(void *arg)
{
    printf("ey_network_task_proc start.\n");

    //xVideoSocketMutex = xSemaphoreCreateMutex();
    xJsonSocketMutex = xSemaphoreCreateMutex();
    xJSONDataQueue = xQueueCreate(MAX_JSON_QUEUE_LENGTH, sizeof(json_data_message_t));
    xJsonQueueMutex = xSemaphoreCreateMutex();
    
    
    //if (!xVideoSocketMutex || !xJsonSocketMutex || !xJSONDataQueue) {
    if (!xJsonSocketMutex || !xJSONDataQueue) {
        printf("Failed to create sync primitives\n");
        vTaskDelete(NULL);
        return;
    }
    
    xTaskCreate(json_data_task, "JSONProcessor", 4096, NULL, 25, &xJSONTaskHandle);
    // fd_set xFD_Set;
    int xResult;
    int xSockets = -1;
    // int xClientSocket = -1;
    int xXSSockets = -1;
    // int xXSClientSocket = -1;
    static const struct timeval xNoTimeOut = {2, 0};
    struct sockaddr_in xAddress, xRemoteAddr;
    socklen_t xClientLength = sizeof(struct sockaddr_in);
    int32_t lBytes;
    uint8_t *header_ptr;
    msg_header header;
#if EY_DEBUG
    void *h264SrcBuf = NULL;
#endif
    uint8_t *h264SrcBufPtr = NULL;
    int32_t h264SrcSize = 0;
    int i;
    video_frame_s *frame = NULL;
    struct timeval xBlockingTime = {2, 0};
    uint32_t ts /*, r_ts*/;
    int cur_data_size = 0, pre_data_size = 0, retry_count;
    int err_flag = 0;
    int h264_show_flag = 0;
    int h264_recv_sum = 0;

#if EY_DEBUG
    h264SrcBuf = pvPortMalloc(H264DEC_INBUF_SIZE);
    if (!h264SrcBuf){
        printf("%s, h264SrcBuf pvPortMalloc failed.\n", __func__);
        return;
    }
#endif
#if DISABLE_CARLINK_H264_FRAME_BUF
    void *video_handle = NULL;
#endif

    xSockets = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    configASSERT(xSockets != -1);
    setsockopt(xSockets, SOL_SOCKET, SO_RCVTIMEO, &xNoTimeOut, sizeof(xNoTimeOut));
    set_socket_win(xSockets);
    xAddress.sin_port = htons(11111);
    xAddress.sin_family = AF_INET;
    xAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(xSockets, (struct sockaddr *)&xAddress, sizeof(xAddress));
    listen(xSockets, 1);

    xXSSockets = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    configASSERT(xXSSockets != -1);
    setsockopt(xXSSockets, SOL_SOCKET, SO_RCVTIMEO, &xNoTimeOut, sizeof(xNoTimeOut));
    set_socket_win(xXSSockets);
    xAddress.sin_port = htons(11113);
    xAddress.sin_family = AF_INET;
    xAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(xXSSockets, (struct sockaddr *)&xAddress, sizeof(xAddress));
    listen(xXSSockets, 1);

    while (1)    {
        // printf("while err_flag :%d \r\n",err_flag);

        process_flag = 1;

        if (err_flag != 0)        {
            if (xClientSocket != -1)            {
                printf("xClientSocket close:%d \r\n", xClientSocket);
                shutdown(xClientSocket, SHUT_RDWR);
                FD_CLR(xClientSocket, &xFD_Set);
                close(xClientSocket);
                xClientSocket = -1;
            }
            if (xXSClientSocket != -1)            {
                printf("xXSClientSocket close:%d \r\n", xXSClientSocket);
                shutdown(xXSClientSocket, SHUT_RDWR);
                FD_CLR(xXSClientSocket, &xFD_Set);
                close(xXSClientSocket);
                xXSClientSocket = -1;
            }

            notify_carlink_state(0);
            err_flag = 0;
            h264_show_flag = 0;
            h264_recv_sum = 0;
            timeRecvLast.tv_nsec = 0;
            heartbeat_flag = 0;
            gH264Index = 0;
            gCRC16Log = 0;
            printf("reset conntion !\r\n");
        }
        FD_ZERO(&xFD_Set);
        FD_SET(xSockets, &xFD_Set);
        FD_SET(xXSSockets, &xFD_Set);
        if (xClientSocket != -1)        {
            FD_SET(xClientSocket, &xFD_Set);
        }
        if (xXSClientSocket != -1)        {
            FD_SET(xXSClientSocket, &xFD_Set);
        }

        process_flag = 2;

        xResult = select(FD_SETSIZE, &xFD_Set, NULL, NULL, &xBlockingTime);
        if (xResult == 0)        { // printf("select timeout\r\n");
            if (xClientSocket != -1)            { // maybe socket don't recv close info;
                if (cur_data_size == pre_data_size)                { // no data recv
                    printf("no data, retry count:%d\r\n", retry_count);
                    if (retry_count++ >= 1)                    { // no data for 4s
                        err_flag = 1;
                        notify_carlink_state(0);
                    }
                }
                pre_data_size = cur_data_size;
            }
            xBlockingTime.tv_sec = 2;
            xBlockingTime.tv_usec = 0;
            continue;
        }
        if (xResult < 0){
            break;
        }
        pre_data_size = cur_data_size;

        if (FD_ISSET ( xSockets, &xFD_Set )) {
            process_flag = 3;

            if (xClientSocket != -1)
            {
                printf("xClientSocket:%d reset wait new conntion \r\n", xClientSocket);
                err_flag = 1;
                continue; // 过来新的连接关闭掉旧的，重新走连接流程
            }
            xClientSocket = accept(xSockets, (struct sockaddr *)&xRemoteAddr, &xClientLength);

            if (xClientSocket != -1){

                char pucBuffer[32] = {0};
                FD_CLR(xClientSocket, &xFD_Set);
                FD_SET(xClientSocket, &xFD_Set);
                inet_ntop(AF_INET, &xRemoteAddr.sin_addr, pucBuffer, sizeof(pucBuffer));
                set_socket_win(xClientSocket);
                printf("\r\nServer 11111: Received a connection from %s:%u xClientSocket:%d \r\n", pucBuffer, ntohs(xRemoteAddr.sin_port), xClientSocket);
                cur_data_size = 0;
                pre_data_size = 0;
                retry_count = 0;
                heartbeat_flag = 1;
                clockTime(&timeRecvLast);
                h264_dec_ctx_init();
                // notify_carlink_state(1);
            }
            continue;
        }else if (FD_ISSET(xXSSockets, &xFD_Set)){
            process_flag = 4;

            if (xXSClientSocket != -1)
            {
                printf("xXSClientSocket close:%d reset wait new conntion \r\n", xXSClientSocket);
                err_flag = 1;
                continue; // 过来新的连接关闭掉旧的，重新走连接流程
            }

            xXSClientSocket = accept(xXSSockets, (struct sockaddr *)&xRemoteAddr, &xClientLength);
            if (xXSClientSocket != -1){

                char pucBuffer[32] = {0};
                FD_CLR(xXSClientSocket, &xFD_Set);
                FD_SET(xXSClientSocket, &xFD_Set);
                inet_ntop(AF_INET, &xRemoteAddr.sin_addr, pucBuffer, sizeof(pucBuffer));
                printf("\r\nServer 11113: Received a connection from %s:%u xXSClientSocket:%d \r\n", pucBuffer, ntohs(xRemoteAddr.sin_port), xXSClientSocket);
                set_socket_win(xXSClientSocket);
            }
            continue;
        }else if (FD_ISSET(xXSClientSocket, &xFD_Set)){
            process_flag = 5;
            json_data_message_t message;
            //message.socket = xXSClientSocket;
            message.data = NULL;
            message.length = read_11113_data(xXSClientSocket, &message.data);
            if (message.length > 0) {
                
                if (xQueueSend(xJSONDataQueue, &message, 100) != pdPASS) {
                    printf("JSON queue full, dropping data\n");
                    if (message.data) vPortFree(message.data);
                }        


            } else if (message.length < 0){
              
                printf("recv port 11113 err:%d\r\n", message.length);
                err_flag = 1;
                
            }
//#endif            //here?
            continue;
        }
        else if (FD_ISSET(xClientSocket, &xFD_Set))
        {

            process_flag = 6;
            lBytes = readn(xClientSocket, (void *)&header, sizeof(header),4000);

            if (lBytes != sizeof(header)){
                printf("recv port 11111 err:%d\r\n", lBytes);
                err_flag = 1;
                continue;
            }

            if (!(header.sync_word[0] == 0xAA && header.sync_word[1] == 0xBB && header.sync_word[2] == 0xCC)){
                printf("ERROR header sync_word[%02x][%02x][%02x]\r\n", header.sync_word[0], header.sync_word[1], header.sync_word[2]);
                continue;
            }
                

            if (header.type == 4){
                uint16_t height, width;
                header_ptr = (uint8_t *)&header;
                for (i = 0; i < lBytes; i++){
                    printf("%02x ", header_ptr[i]);
                }
                printf("\r\n");

                height = (header_ptr[12] | (header_ptr[13] << 8));
                width = (header_ptr[14] | (header_ptr[15] << 8));
                printf("#receive phone info width:%d height:%d\r\n", width, height);
                gey_ts = get_timer(0); // r_ts = header.ts;

                ey_send_hu_info(xClientSocket);
                continue;
            }else if (header.type == 6){
                printf("heart beat\r\n");

                static uint8_t heartbeatback[] = {
                    0xAA, 0xBB, 0xCC, 0x0F, 0x00, 0x00, 0x00, 0x00,
                    0x0D, 0x00, 0x00, 0x00, // 13经过sw32转换后的4字节（固定值）
                    '{', '\"', 'v', 'e', 'r', '\"', ':', '\"', '2', '.', '0', '\"', '}'};
                send(xClientSocket, heartbeatback, sizeof(heartbeatback), 0);

                continue;
            }else if (header.type != 0){
                printf("ERROR header type[%02x]\r\n", header.type);
                continue;
            }

            h264SrcSize = header.payload_size - 4; // skip timestamp
            if (h264SrcSize <= 0 || h264SrcSize > H264_FRAME_BUF_SIZE){
                printf("h264 data len:%d err\r\n", h264SrcSize);
                continue;
            }

#ifdef CRC16_ENABLE
            uint16_t index, crc16;
            header_ptr = (uint8_t *)&header;

            index = read2BytesToInt2(header_ptr + 4);
            crc16 = read2BytesToInt2(header_ptr + 6);
            index = sw16(index);
            crc16 = sw16(crc16);

            // printf(" ---- index:%d crc16:%04x ---- \r\n", index, crc16);

            if (crc16 > 0)
            {
                gCRC16Log = 1;
            }
            if (gCRC16Log)
            {
                if (index != gH264Index)
                {
                    printf("ERROR: index error index:%d - %d crc16:%04x header[%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x] \r\n", index, gH264Index, crc16,
                           header_ptr[0], header_ptr[1], header_ptr[2], header_ptr[3], header_ptr[4], header_ptr[5], header_ptr[6], header_ptr[7], header_ptr[8], header_ptr[9], header_ptr[10], header_ptr[11]);
                }
                gH264Index = index + 1;
            }
#endif

            clockTime(&timeRecvLast);
#if EY_DEBUG
            h264SrcBufPtr = (uint8_t *)h264SrcBuf;
#else
#if !DISABLE_CARLINK_H264_FRAME_BUF
        get_retry:
            frame = get_h264_frame_buf();
            if (NULL == frame){
                printf("h264 frame is empty\r\n");
                vTaskDelay(pdMS_TO_TICKS(20));
                goto get_retry;
                // continue;
            }

            h264SrcBufPtr = (uint8_t *)frame->cur;
            frame->len = h264SrcSize;
#else
            if (NULL == video_handle){
                video_handle = h264_video_player_init();
            }
            if (NULL == h264SrcBufPtr){
                h264SrcBufPtr = pvPortMalloc(H264DEC_INBUF_SIZE);
            }
            if (NULL == h264SrcBufPtr){
                printf("h264SrcBufPtr malloc failed\n");
                break;
            }
#endif
#endif
            process_flag = 7;
            // ts = get_timer(0);
            // printf("Size:%05d lts:%03d rts:%06d\r\n", h264SrcSize, (ts - gey_ts) / 1000, header.ts - r_ts);
            gey_ts = ts; // r_ts = header.ts;
            lBytes = readn(xClientSocket,h264SrcBufPtr, h264SrcSize, 4000);
            process_flag = 8;

#if !DISABLE_CARLINK_H264_FRAME_BUF
            if (h264SrcSize != lBytes){
                set_h264_frame_free(frame);
                if (lBytes == -1){
                    printf("###recv port 11111 err:%d\r\n", lBytes);
                    err_flag = 1;
                }else{
                    cur_data_size += lBytes;
                }
            }else{
                cur_data_size += lBytes;
                notify_h264_frame_ready(&frame);
            }
#else
            if (h264SrcSize == lBytes){

                if (h264_recv_sum < 100)
                {
                    h264_recv_sum++;
                    // 延迟10帧数据后显示解码层
                    if (h264_show_flag == 0 && h264_recv_sum == 10)
                    {
                        h264_show_flag = 1;
                        printf("##############  h264 show  #################\n");
                        notify_carlink_state(1);
                    }
                }

#ifdef CRC16_ENABLE
                if (gCRC16Log)
                {
                    uint16_t h264_crc16 = calculate_checksum16_fast(h264SrcBufPtr, h264SrcSize);
                    // printf("CRC index:%d crc16:%04x - %04x header[%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x] len:%d \r\n", index, crc16,h264_crc16,
                    //         header_ptr[0],header_ptr[1],header_ptr[2],header_ptr[3],header_ptr[4],header_ptr[5],header_ptr[6],header_ptr[7],header_ptr[8],header_ptr[9],header_ptr[10],header_ptr[11],h264SrcSize);

                    if (h264_crc16 != crc16)
                    {
                        printf("ERROR: CRC16 error index:%d crc16:%04x - %04x header[%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x] len:%d \r\n", index, crc16, h264_crc16,
                               header_ptr[0], header_ptr[1], header_ptr[2], header_ptr[3], header_ptr[4], header_ptr[5], header_ptr[6], header_ptr[7], header_ptr[8], header_ptr[9], header_ptr[10], header_ptr[11], h264SrcSize);
                        if (h264SrcSize < 33)
                        {
                            printf("h264 data len:%d [%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ]\r\n", h264SrcSize, h264SrcBufPtr[0], h264SrcBufPtr[1], h264SrcBufPtr[2], h264SrcBufPtr[3], h264SrcBufPtr[4], h264SrcBufPtr[5], h264SrcBufPtr[6], h264SrcBufPtr[7], h264SrcBufPtr[8], h264SrcBufPtr[9], h264SrcBufPtr[10], h264SrcBufPtr[11], h264SrcBufPtr[12], h264SrcBufPtr[13], h264SrcBufPtr[14], h264SrcBufPtr[15], h264SrcBufPtr[16], h264SrcBufPtr[17], h264SrcBufPtr[18], h264SrcBufPtr[19], h264SrcBufPtr[20], h264SrcBufPtr[21], h264SrcBufPtr[22], h264SrcBufPtr[23], h264SrcBufPtr[24], h264SrcBufPtr[25], h264SrcBufPtr[26], h264SrcBufPtr[27], h264SrcBufPtr[28], h264SrcBufPtr[29], h264SrcBufPtr[30], h264SrcBufPtr[31]);
                        }
                    }
                }
#endif
                gH264Frames++;
                h264_video_player_proc(video_handle, h264SrcBufPtr, h264SrcSize);
            }
            else
            {
                printf("recv port 11111 err:%d\r\n", lBytes);
                err_flag = 1;
            }
#endif
        }
    }

    printf("%s exit...\r\n", __func__);
    if (xSockets != -1)
        close(xSockets);
    if (xXSSockets != -1)
        close(xXSSockets);
    if (xClientSocket != -1)
        close(xClientSocket);
    if (xXSClientSocket != -1)
        close(xXSClientSocket);
    if (xJSONTaskHandle) vTaskDelete(xJSONTaskHandle);
    if (xJSONDataQueue) vQueueDelete(xJSONDataQueue);
    //if (xVideoSocketMutex) vSemaphoreDelete(xVideoSocketMutex);
    if (xJsonSocketMutex) vSemaphoreDelete(xJsonSocketMutex);
#if DISABLE_CARLINK_H264_FRAME_BUF
    h264_video_player_uninit(video_handle);
    if (NULL == h264SrcBufPtr){
        free(h264SrcBufPtr);
    }
#endif
    vTaskDelete(NULL);
}
#endif



static int h264_show_flag = 0;
static int h264_recv_sum = 0;
static void *video_handle = NULL;
void ey_server_connected(int is_connected){
    if(is_connected){

    }else{
        //关闭解码页面，显示二维码
        h264_recv_sum = 0;
        h264_show_flag = 0;

        notify_carlink_state(0);

    }
}

void ey_server_h264_recv(uint8_t *data,size_t data_len){
    if(data_len > 0){
        if (NULL == video_handle){
            video_handle = h264_video_player_init();
        }

        if (h264_recv_sum < 100)
        {
            h264_recv_sum++;
            // 延迟10帧数据后显示解码层
            if (h264_show_flag == 0 && h264_recv_sum == 10)
            {
                h264_show_flag = 1;
                printf("##############  h264 show  #################\n");
                notify_carlink_state(1);
            }
        }

        h264_video_player_proc(video_handle, data, data_len);
    }
}

void ey_server_11113_recv(uint8_t *json,size_t json_len, uint8_t *binary,size_t binary_len){

    printf("json_len:%d binary_len:%d\r\n",json_len,binary_len);

    if(json_len > 0){
        json_data_message_t message;
        memset(&message,0x00,sizeof(json_data_message_t));

        message.data = (uint8_t *)pvPortMalloc(json_len+1);
        if (NULL == message.data) {
            printf("json_data_message_t malloc failed json_len:%d\r\n",json_len);
            return;
        }
        memcpy(message.data,json,json_len);
        message.length = json_len;
        message.data[json_len] = 0x00;

        if (binary_len > 0)
        {
            message.bindata = (uint8_t *)pvPortMalloc(binary_len+1);
            if (NULL == message.bindata) {
                printf("json_data_message_t malloc failed json_len:%d\r\n",binary_len);
                return;
            }
            memcpy(message.bindata,binary,binary_len);
            message.binlength = binary_len;
            message.bindata[binary_len] = 0x00;
        }
        
        if (xQueueSend(xJSONDataQueue, &message, 100) != pdPASS) {
            printf("JSON queue full, dropping data\n");
            if (message.data) vPortFree(message.data);
        }        
    }
}

void  er_server_end(){
    if (xJSONTaskHandle) vTaskDelete(xJSONTaskHandle);
    if (xJSONDataQueue) vQueueDelete(xJSONDataQueue);
    //if (xVideoSocketMutex) vSemaphoreDelete(xVideoSocketMutex);
    if (xJsonSocketMutex) vSemaphoreDelete(xJsonSocketMutex);
    if(video_handle) h264_video_player_uninit(video_handle);
}

static int ey_start_network()
{
	BaseType_t ret;

    xJsonSocketMutex = xSemaphoreCreateMutex();
    xJSONDataQueue = xQueueCreate(MAX_JSON_QUEUE_LENGTH, sizeof(json_data_message_t));
    xJsonQueueMutex = xSemaphoreCreateMutex();
    
    //if (!xVideoSocketMutex || !xJsonSocketMutex || !xJSONDataQueue) {
    if (!xJsonSocketMutex || !xJSONDataQueue) {
        printf("Failed to create sync primitives\n");
        vTaskDelete(NULL);
        return -1;
    }
    
    xTaskCreate(json_data_task, "JSONProcessor", 4096, NULL, 25, &xJSONTaskHandle);

    ey_servers_init();
	return ret;
}

void ey_wifi_event_handler( WIFIEvent_t * xEvent )
{
    WIFIEventType_t xEventType = xEvent->xEventType;

    if (0) {
    } else if (eWiFiEventConnected                    == xEventType) {// meter is sta
        notify_iphone_ap_connect_status(0);
        printf("\r\n The meter is connected to ap \r\n");
    } else if (eWiFiEventDisconnected                == xEventType) {// meter is sta
        printf("\r\n The meter is disconnected from ap \r\n");
#if !USE_LWIP
        xSemaphoreTake(gSocketMutex, portMAX_DELAY);
        if (gClientSocket != FREERTOS_INVALID_SOCKET)
            FreeRTOS_SignalSocket(gClientSocket);// wake up socket recv
        if (gXClientSocket != FREERTOS_INVALID_SOCKET)
            FreeRTOS_SignalSocket(gClientSocket);
        xSemaphoreGive(gSocketMutex);
#endif
        notify_phone_ip_addr(0);
        notify_carlink_state(0);
    } else if (eWiFiEventAPStationConnected       == xEventType) {// meter is ap
        printf("\r\n The meter in AP is connected by sta %s \r\n", xEvent->xInfo.xAPStationConnected.ucMac);
    } else if (eWiFiEventAPStationDisconnected   == xEventType) {// meter is ap
        printf("\r\n The sta %s is disconnected from the meter \r\n", xEvent->xInfo.xAPStationDisconnected.ucMac);
        //notify_phone_ip_addr(0);
    }
}


int mmcsd_wait_sdio_ready(int32_t timeout);
static int carlink_ey_network_init()
{
    printf("carlink_ey_network_init\r\n");
    BaseType_t ret = 0;
    unsigned int status;
#if !USE_LWIP
    gSocketMutex = xSemaphoreCreateMutex();
#endif
    // WIFI_Context_init();
    // WIFI_RegisterEvent(eWiFiEventMax, ey_wifi_event_handler);
    // for (;;) {
    //     status = mmcsd_wait_sdio_ready((int32_t)portMAX_DELAY);
    //     if (status == MMCSD_HOST_PLUGED) {
    //         printf("detect sdio device\r\n");
    //         break;
    //     }
    // }
// #if !USE_LWIP
//     printf("FreeRTOS_IPInit\r\n");
//     ret = FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress,ucDNSServerAddress, ucMACAddress);
// #else
//     printf("ey_lwip_server_init\r\n");
// 	ey_lwip_server_init();
// #endif

    printf("ey_start_network ret:%d\r\n", ret);
    ey_start_network();
    return (int)ret;
}

static void carlink_ey_network_uninit()
{
}


static void ey_bt_callback(char * cAtStr)
{
    char* cmd = NULL, *cmd_para = NULL;
    struct bt_event ev = {0};
    ev.type = -1;
    printf("\r\nbt_callback %s\r\n", cAtStr);
    if (0) {
    } else if (0 == strncmp(cAtStr, "+WLCONN",      7)) {
        ev.type = EY_BT_EVENT_NOTIFY_PHONE_AP_READY;

    } else if (0 == strncmp(cAtStr, "+ADDR=",       6)) {
        char cmd_str[64] = {0};
        sprintf(cmd_str, "AT+NAME=EY_%s\r\n", (cAtStr + 6));
        ey_reset_wifi_ap_info(cAtStr + 6);
        printf("ADDR:%s\r\n", cAtStr + 6);
        bt_send_at_command(cmd_str, strlen(cmd_str));//get mac addr
        memset(cmd_str, 0, sizeof(cmd_str));
        sprintf(cmd_str, "AT+LENAME=EY_%s\r\n", (cAtStr + 6));
        bt_send_at_command(cmd_str, strlen(cmd_str));//get mac addr
        gbt_ready = 1;
    } else if (0 == strncmp(cAtStr, "+WLSSID=",     8)) {
        ev.type = EY_BT_EVENT_NOTIFY_SSID;
        cmd_para = cAtStr + 8;
        memcpy((void*)ev.para, (void*)cmd_para, strlen(cmd_para));
    } else if (0 == strncmp(cAtStr, "+WLPASSWD=",   10)) {
        ev.type = EY_BT_EVENT_NOTIFY_PASSWD;
        cmd_para = cAtStr + 10;
        memcpy((void*)ev.para, (void*)cmd_para, strlen(cmd_para));
    } else if (0 == strncmp(cAtStr, "+HFPSTAT=1",   10)) {
        ev.type = EY_BT_EVENT_DISCONNECT;
    } else if (0 == strncmp(cAtStr, "+HFPDEV=",     8)) {
        ev.type = EY_BT_EVENT_CONNECT;
        cmd_para = cAtStr + 8;
        memcpy((void*)ev.para, (void*)cmd_para, 12);
    } else if (0 == strncmp(cAtStr, "+AAA5=",       6)) {
        ev.type = EY_BT_EVENT_NOTIFY_PHONE_TYPE;
        cmd_para = cAtStr + 6;
        memcpy((void*)ev.para, (void*)cmd_para, strlen(cmd_para));
    } else if (0 == strncmp(cAtStr, "+AAA7=",       6)) {
        ev.type = EY_BT_EVENT_NOTIFY_CONNECT_STATUS;
        cmd_para = cAtStr + 7;
        memcpy((void*)ev.para, (void*)cmd_para, strlen(cmd_para));
    } else if (0 == strncmp(cAtStr, "+GATTSTAT=3",  11)) {
        ev.type = EY_BT_EVENT_BLE_CONNECT;
    } else if (0 == strncmp(cAtStr, "+AAA9",        5)) {
        ev.type = EY_BT_EVENT_NOTIFY_USE_TCP;
    } else if (0 == strncmp(cAtStr, "+VER",         4)) {
        cmd = "AT+ADDR\r\n";
        bt_send_at_command(cmd, strlen(cmd));//get mac addr
    }

    if (-1 != ev.type && NULL != ey_bt_event_queue) {
        xQueueSend(ey_bt_event_queue, &ev, 0);
    }
}

static void ey_bt_task_proc(void* arg)
{
    struct bt_event ev;
    char* cmd_para      = NULL;
    char at_cmd[128]     = {0};
    char phone_type     = -1;
    char ssid[32]       = {0};
    char passwd[32]     = {0};
    char bt_mac[12 + 1] = {0};
    int ret             = -1;
    int carlink_ready_flag = 0;

    for (;;) {
        memset((void*)&ev, 0, sizeof(ev));
        if (xQueueReceive(ey_bt_event_queue, &ev,  portMAX_DELAY) != pdPASS) {
            printf("%s xQueueReceive err!\r\n", __func__);
            continue;
        }
        cmd_para = (char*)ev.para;
        if (0) {
        } else if (EY_BT_EVENT_NOTIFY_CONNECT_STATUS == ev.type) {
        } else if (EY_BT_EVENT_CONNECT               == ev.type) {
            memcpy(bt_mac, cmd_para, 12);
            bt_mac[12] = '0';

        } else if (EY_BT_EVENT_DISCONNECT            == ev.type) {
            memset(ssid,   0, sizeof(ssid));
            memset(passwd, 0, sizeof(passwd));
            memset(bt_mac, 0, sizeof(bt_mac));
            phone_type = -1;
            notify_iphone_ssid_passwd();
            notify_phone_ip_addr(0);
        } else if (EY_BT_EVENT_NOTIFY_PHONE_TYPE     == ev.type) {
            string2hex(cmd_para, strlen(cmd_para), &phone_type, 2);
            printf("\r\nphone type:%d\n", phone_type);
            if (phone_type != gphone_type) {
                gWifiOn = 0;// reboot wifi
                WIFI_Off();
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
            gphone_type = phone_type;
            if (0 == phone_type) {//android
                char type_str[5] = {0};
                char *tmp_buf = "-1";
                str2asiistr(tmp_buf, strlen(tmp_buf), type_str, 2 * strlen(tmp_buf));
                sprintf(at_cmd, "AT+GATTSET=AAA2%s\r\n", type_str);
                printf("\r\nat:%s\r\n", at_cmd);
                bt_send_at_command(at_cmd, strlen(at_cmd));
            } else if (1 == phone_type) {//iphone  
#if !USE_LWIP
                setDhcpClientState(0);
                vDHCPProcess(1, eInitialWait);
                xSendDHCPEvent();
#else
				netif_set_down(&gnetif[0]);
#endif
                notify_iphone_ssid_passwd();
                ey_wifi_start_ap(ap_ssid, ap_passwd, 36);
#if USE_LWIP
				
				ey_lwip_server_init();
				netif_set_up(&gnetif[0]);

#endif
                uint32_t ap_addr = (ucIPAddress[0]) | (ucIPAddress[1] << 8) | (ucIPAddress[2] << 16) | (ucIPAddress[3] << 24);
                notify_phone_ip_addr(ap_addr);
            }
        } else if (EY_BT_EVENT_NOTIFY_SSID           == ev.type) {
            memcpy(ssid,    cmd_para, strlen(cmd_para) + 1);
            printf("\r\nssid:%s\r\n", ssid);
        } else if (EY_BT_EVENT_NOTIFY_PASSWD         == ev.type) {
            memcpy(passwd,  cmd_para, strlen(cmd_para) + 1);
            printf("\r\npasswd:%s\r\n", passwd);
        } else if (EY_BT_EVENT_NOTIFY_PHONE_AP_READY == ev.type) {
            printf("\r\nssid:%s passwd:%s\r\n", ssid, passwd);
#if !USE_LWIP
            setDhcpClientState(1);
            vDHCPProcess(1, eInitialWait);
            xSendDHCPEvent();
#else
			netif_set_down(&gnetif[0]);
#endif
            notify_iphone_ap_connect_status(4);
            ret = ey_wifi_connect_remote_ap(ssid, passwd);
#if !USE_LWIP
            if (ret == 0) {
                vDHCPProcess(1, eWaitingSendFirstDiscover);
                xSendDHCPEvent();
            }
#else
			ey_lwip_client_init();
#endif
        } else if (EY_BT_EVENT_NOTIFY_USE_TCP        == ev.type) {
        } else if (EY_BT_EVENT_BLE_CONNECT           == ev.type) {
            char bt_mac_hex_str[25] = {0};
            str2asiistr(bt_mac, 12, bt_mac_hex_str, 24);
            sprintf(at_cmd, "AT+GATTSEND=AAA5%s\r\n", bt_mac_hex_str);
            printf("\r\nat:%s\r\n", at_cmd);
            bt_send_at_command(at_cmd, strlen(at_cmd));
        } else if (EY_CARLINK_EVENT_CONNECT         == ev.type) {
            carlink_ready_flag = 1;
            printf("EY_CARLINK_EVENT_CONNECT\r\n");
            set_carlink_display_state(1);
 #if EY_CARLINK_NOTIFY_HOOK
            carlink_ey_notify_state_hook(CARLINK_EY_READY);
 #endif
        } else if (EY_CARLINK_EVENT_DISCONNECT    == ev.type) {
            if (carlink_ready_flag == 1) {
                printf("EY_CARLINK_EVENT_DISCONNECT\r\n");
                carlink_ready_flag = 0;
                set_carlink_display_state(0);
#if EY_CARLINK_NOTIFY_HOOK
                carlink_ey_notify_state_hook(CARLINK_EY_EXIT);
#endif
            }
        }
    }

	vTaskDelete(NULL);
}

static int carlink_ey_bt_init()
{
    int ret = -1;

    ret = bt_main();
    if (ret != 0)
        return ret;
    bt_register_event_cb(NULL, ey_bt_callback);

    ey_bt_event_queue = xQueueCreate(1, sizeof(struct bt_event));
    ret = xTaskCreate(ey_bt_task_proc, "ey_bt", 2048 * 8, NULL, configMAX_PRIORITIES - 1, &ey_bt_task);
    return ret;
}

static void carlink_ey_bt_uninit()
{
}

static void carlink_ey_init_thread(void *param)
{
    carlink_ey_audio_init();
    carlink_ey_video_init();

    int ret = carlink_common_init();
    carlink_bt_wifi_init();

    carlink_ey_network_init();
    // carlink_ey_bt_init();
    //ey_wifi_start_ap(ap_ssid, ap_passwd, 36);
    vTaskDelete(NULL);
}

int carlink_ey_init()
{
	//避免wifi模块异常后导致初始化接口阻塞，影响机器正常启动。
	xTaskCreate(carlink_ey_init_thread, "carlink_ey_init", 1024 * 16, NULL, 8, NULL);
    return 0;
}

void carlink_ey_uninit()
{
    carlink_ey_bt_uninit();
    carlink_ey_network_uninit();
}
#else
int carlink_ey_init()
{
    return 0;
}
void carlink_ey_uninit()
{
}
#endif
