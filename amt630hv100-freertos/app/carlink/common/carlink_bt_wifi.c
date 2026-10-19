//#include "os_adapt.h"
#include <FreeRTOS_POSIX.h>
#include <task.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "timer.h"
#include "iot_wifi.h"
#include "mmcsd_core.h"
#include "carlink_utils.h"
#if defined (BT_STACK_BARROT)
#include "brtbt_interface.h"
#define bt_send_at_command(cmd, len) \
	(printf("BT AT TX(%u): %.*s", (unsigned int)(len), (int)(len), (cmd)), \
	 brtbt_send_at_command((cmd), (len)))
#define bt_register_event_cb(...) brtbt_register_event_cb(__VA_ARGS__)
#define bt_main(...) brt_bt_main(__VA_ARGS__)
#else 
#include "console.h"
#include "fsc_bt.h"
#define bt_send_at_command(...) console_send_atcmd(__VA_ARGS__)
#define bt_register_event_cb(...) console_register_cb(__VA_ARGS__)
#define bt_main(...) fsc_bt_main(__VA_ARGS__)
#endif
#include "iap.h"
#if defined(USE_BW8201)
#include "bt_api.h"
#endif

#if defined (WIFI_CHIP_GT)
#include "atbm_wifi_driver_api.h"
#include "atbm_api.h"
#endif

#if (CARLINK_EC == 1) || (CARLINK_CP == 1) || (CARLINK_AA == 1) || (CARLINK_EY == 1)
#if !defined(USE_LWIP) || !USE_LWIP
#error "Carlink need lwip!"
#endif

#include "ethernet.h"
#include "tcpip.h"
#include "lwip/apps/lwiperf.h"
#include "dhcp.h"

#include "carlink_video.h"
#include "mycommon.h"
//#include "wifi_conf.h"
#include "carlink_common.h"

#define DEV_NAME_PREFIX   "AP630_CARLINK"

#ifndef CARLINK_STA_P2P_COEXIST_TEST
#define CARLINK_STA_P2P_COEXIST_TEST 1
#endif

#ifndef CARLINK_STA_P2P_DELAY_MS
#define CARLINK_STA_P2P_DELAY_MS 10000
#endif

#ifndef CARLINK_RSSI_POLL_ENABLE
#define CARLINK_RSSI_POLL_ENABLE 1
#endif

#ifndef CARLINK_RSSI_POLL_INTERVAL_MS
#define CARLINK_RSSI_POLL_INTERVAL_MS 5000
#endif

#ifndef CARLINK_BT_PROFILE_BEFORE_OPEN_DELAY_MS
#define CARLINK_BT_PROFILE_BEFORE_OPEN_DELAY_MS 1000
#endif

#ifndef CARLINK_BT_OPEN_AFTER_PROFILE_DELAY_MS
#define CARLINK_BT_OPEN_AFTER_PROFILE_DELAY_MS 1000
#endif

static char g_cp_bt_mac[13] = {0};
static bool g_cp_bt_mac_ready = false;

static uint8_t                carlink_p2p_name[64]     = {0};
static uint8_t                carlink_ap_ssid[64]     = {"ap63011"};
static uint8_t                carlink_ap_passwd[16]  = {"88888888"};
static uint8_t                carlink_wifi_mac[32] = {0};

extern int wps_connect_done;

static const uint8_t        ucIPAddress[4] = {192, 168, 13, 1};
static const uint8_t        ucNetMask[4] = {255, 255, 255, 0};
static const uint8_t        ucGatewayAddress[4] = {0, 0, 0, 0};

static int                  lwip_tcpip_init_done_flag = 0;

static int g_bt_wifi_init_flag = 0;
static int g_ble_cccd_test_sent = 0;
static int g_sta_p2p_test_started = 0;
static uint64_t g_gattdata_peer_rx_total = 0;
static unsigned int g_bt_open_generation = 0;
static int g_bt_profile_command_pending = 0;
#if CARLINK_RSSI_POLL_ENABLE
static char g_gatt_client_if[8] = {0};
static char g_gatt_addr[13] = {0};
static int g_gatt_connected = 0;
static int g_rssi_retry_pending = 0;
static unsigned int g_rssi_poll_generation = 0;
#endif
static pthread_mutex_t btwifiLocker =
{
        .xIsInitialized = pdFALSE,
        .xMutex = { { 0 } },
        .xTaskOwner = NULL,
		.xAttr = { .iType = 0 }
};

int start_ap(int channel, const char* ssid, const char* passwd, char need_passwd);
int start_p2p(const char *dev_name, const char *ssid, const char *passwd);
int start_sta_ext(const char* ssid, const char* passwd, char need_passwd);

static struct netif  gnetif[4];

extern int wifi_add_custom_ie(void *cus_ie, int ie_num);
extern int mmcsd_wait_sdio_ready(int32_t timeout);

static void carlink_reset_wifi_ap_info(const char *prefex);
static void carlink_start_wlan();
static void carlink_prepare_p2p_info(void);
static void bt_set_support_carplay_android_auto(void);
static void bt_send_profile_command(void);
static void taskBtOpenWithProfileDelay(void *param);
extern err_t dhcp_server_start(struct netif *netif, ip4_addr_t *start, ip4_addr_t *end);
extern err_t wlan_ethernetif_init(struct netif *netif);
#define lwip_ipv4_addr(addr) ((addr[0]) | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24))

void carlink_set_bt_mac(const char *mac, int mac_len)
{
	if (mac_len > 12)
		mac_len = 12;
	strncpy((void*)g_cp_bt_mac, (void*)mac, mac_len);
	g_cp_bt_mac_ready = true;
}

const char *carlink_get_bt_mac()
{
	if (!g_cp_bt_mac_ready)
		return NULL;
	return (const char *)g_cp_bt_mac;
}

const char *carlink_get_wifi_p2p_name()
{
	return (const char *)carlink_p2p_name;
}

const char *carlink_get_wifi_ssid()
{
#if 0 //defined(CHIPSET_SUPPORT_AIC8800D)
        uint8_t ssid[64] = {0};
        sprintf((char *)ssid, "DIRECT-%s", (char *)carlink_ap_ssid);
        memset(carlink_ap_ssid, 0, sizeof(carlink_ap_ssid));
        memcpy(carlink_ap_ssid, ssid, strlen(ssid));
#endif
        return (const char *)carlink_ap_ssid;

}


const char *carlink_get_wifi_mac()
{
	WIFI_GetMAC(carlink_wifi_mac);
	return (const char *)carlink_wifi_mac;
}

const char *carlink_get_wifi_passwd()
{
	return (const char *)carlink_ap_passwd;
}

void carlink_get_ap_ip_addr(char ip[4])
{
	memcpy((void*)ip, (void*)ucIPAddress, 4);
}

#if 0
void carlink_set_wifi_net_mac_addr(struct netif *netif)
{
	char mac[6] = {0xdc, 0x0d, 0x30, 0xa2, 0x70, 0xcd};
	const char* str_mac = carlink_get_wifi_mac();
	
	if (str_mac == NULL) {
		printf("%s get wlan mac failed\r\n", __func__);
	} else {
		sscanf(str_mac, "%02x:%02x:%02x:%02x:%02x:%02x", (uint32_t *)&mac[0], (uint32_t *)&mac[1], (uint32_t *)&mac[2], 
			(uint32_t *)&mac[3], (uint32_t *)&mac[4], (uint32_t *)&mac[5]);
	}

	printf("%s:%d str_mac: %02x:%02x:%02x:%02x:%02x:%02x \r\n", __func__, __LINE__, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        
        
        //printf("%s:%d Netif : %p", __func__, __LINE__, netif);

	/* set MAC hardware address */
	netif->hwaddr[0] = mac[0];  
	netif->hwaddr[1] = mac[1];  
	netif->hwaddr[2] = mac[2];  
	netif->hwaddr[3] = mac[3];  
	netif->hwaddr[4] = mac[4];  
	netif->hwaddr[5] = mac[5];
}
#endif

static void tcpip_init_done(void *arg)
{
	(void)arg;
}

static void carlink_lwiperf_report_cb_impl(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
	(void)arg;

	printf("lwiperf_report_cb_impl bytes:%d %d ms \r\n", bytes_transferred, ms_duration);
}

int carlink_wlan_tcp_ip_is_ready()
{
	return lwip_tcpip_init_done_flag;
}

//dd3000a0400000020022020961726B6D6963726F0003064C696E75780004030102030606ffffffffffff070666fadde250c0
static uint8_t carplay_vendor_ie[] = {
	0xdd, 0x30, 0x00, 0xa0, 0x40, 0x00, 0x00, 0x02, 0x00, 0x22, 0x02, 0x09, 0x61, 0x72, 0x6B, 0x6D, 
	0x69, 0x63, 0x72, 0x6F, 0x00, 0x03, 0x06, 0x4C, 0x69, 0x6E, 0x75, 0x78, 0x00, 0x04, 0x03, 0x01, 
	0x02, 0x03, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x06, 0x66, 0xfa, 0xdd, 0xe2, 
	0x50, 0xc0
};

void carlink_carplay_add_vendor_ie()
{
        wifi_ap_set_vendor_ie(carplay_vendor_ie);
}

void carlink_carplay_ie_replace_bt_mac(const char* btmac_str, int btmac_str_len)
{
	char btmac[6] = {0};
	string2hex((char *)btmac_str, btmac_str_len, btmac, 6);
	//memcpy((void*)g_link_info->btmac, (void*)btmac, 6);
	//sscanf(btmac_str, "%02x%02x%02x%02x%02x%02x", btmac[0], btmac[1], btmac[2], btmac[3], btmac[4], btmac[5]);
	carplay_vendor_ie[36] = btmac[0];
	carplay_vendor_ie[37] = btmac[1];
	carplay_vendor_ie[38] = btmac[2];
	carplay_vendor_ie[39] = btmac[3];
	carplay_vendor_ie[40] = btmac[4];
	carplay_vendor_ie[41] = btmac[5];
	//memcpy((void*)(carplay_vendor_ie + 36), (void*)btmac, 6);

	{
		int i;
		for (i = 0; i < sizeof(carplay_vendor_ie); i++) {
			printf("0x%02x, ", carplay_vendor_ie[i]);
		}printf("\r\n");
	}
}

#define ENABLE_REBOOT_WIFI  0
void carlink_restart_bt_wifi()
{
	carlink_bt_close();
#if ENABLE_REBOOT_WIFI
	netif_set_down(&gnetif[0]);
	vTaskDelay(pdMS_TO_TICKS(1000));
	WIFI_Off();
#endif
	vTaskDelay(pdMS_TO_TICKS(2000));
#if ENABLE_REBOOT_WIFI
	//start_ap(36, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, 1);
#if (CARLINK_EC)
	start_p2p((const char *)carlink_p2p_name, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd);
#else
	start_ap(36, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, 1);
#endif
	//carlink_set_wifi_net_mac_addr(&gnetif[0]);
	carlink_carplay_add_vendor_ie();
	netif_set_default(&gnetif[0]);

	netif_set_up(&gnetif[0]);
#endif
	printf("reopen bt\r\n");
	carlink_bt_open();
}

#if 1
static void carlink_start_wlan()
{
	char ap_prefix[5] = {0};

	ip4_addr_t ip_addr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
	ip4_addr_t dhcp_addr_start;
	ip4_addr_t dhcp_addr_end;
#if defined(USE_BW8201)
	fsc_bt_get_dev_addr();
#endif

#if defined(CHIPSET_SUPPORT_AIC8800D)
        g_cp_bt_mac_ready = true;
        memcpy(g_cp_bt_mac, "DC0D1023F235", 12);
#endif
#if defined(WIFI_CHIP_GT)
	while(!g_cp_bt_mac_ready) {
		vTaskDelay(pdMS_TO_TICKS(10));
	}
	/* Address of the already applied EY license */
	memcpy(g_cp_bt_mac, "58C587F4293D", 12);
	
	memcpy(ap_prefix, g_cp_bt_mac + 8, 4);
	carlink_reset_wifi_ap_info(ap_prefix);
	tcpip_init(tcpip_init_done, NULL);
#if (CARLINK_EY)
	char cmd_str[256] = {0};
	//#define A5_TEST_VAL "device=EY_58C587F4293E&ssid=EY_293D&pwd=88888888&name=DIRECT-EY_p2p_293D&mac=58C587F4293D&IP=192.168.13.1&key=EYMOTOR12FCB99FD224DB2B22F5A7703&w=1024&h=600"
	sprintf(cmd_str, "AT+GATTSET=AAA5device=EY_%s&ssid=%s&pwd=%s&name=%s&mac=%s&IP=192.168.13.1&key=EYMOTOR12FCB99FD224DB2B22F5A7703&w=1024&h=600\r\n", \
		g_cp_bt_mac, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, (const char *)carlink_p2p_name, g_cp_bt_mac);

	printf("\r\nhttps://appstore.eryanet.com?productid=f03aad9ac00ec5df&device=EY_%s&ssid=%s&pwd=%s&name=%s&mac=%s&IP=192.168.13.1&port1=11111&port2=11113&key=EYMOTOR12FCB99FD224DB2B22F5A7703&w=1024&h=600\r\n",
		g_cp_bt_mac, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, (const char *)carlink_p2p_name, g_cp_bt_mac);
	
	brtbt_send_at_command(cmd_str, strlen(cmd_str));//get mac addr
#endif
	start_p2p((const char *)carlink_p2p_name, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd);
	//start_ap(36, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, 1);
	lwip_tcpip_init_done_flag = 1;
	return;
	 
#endif
	while(!g_cp_bt_mac_ready) {
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	memcpy(ap_prefix, g_cp_bt_mac + 8, 4);
	carlink_reset_wifi_ap_info(ap_prefix);
#if !defined(CHIPSET_SUPPORT_AIC8800D)
#if (CARLINK_EC)
	start_p2p((const char *)carlink_p2p_name, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd);
#else
	start_ap(36, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, 1);
#endif
#endif
	ip_addr.addr = lwip_ipv4_addr(ucIPAddress);
	netmask.addr = lwip_ipv4_addr(ucNetMask);
	gw.addr      = lwip_ipv4_addr(ucGatewayAddress);
	printf("-----------tcpip_init-----------\n");
#if !defined(WIFI_CHIP_GT)	
	tcpip_init(tcpip_init_done, NULL);
#endif
	netif_add(&gnetif[0],
#if LWIP_IPV4
		&ip_addr, &netmask, &gw,
#endif
		NULL, wlan_ethernetif_init, tcpip_input);
#if defined(CHIPSET_SUPPORT_AIC8800D)
#if (CARLINK_EC)
	start_p2p((const char *)carlink_p2p_name, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd);
#else
	start_ap(36, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, 1);
#endif
#endif
	netif_set_default(&gnetif[0]);

	uint8_t 	 addr_start[4] = {192, 168, 13, 20};
	uint8_t 	 addr_end[4] = {192, 168, 13, 30};
	dhcp_addr_start.addr 	= lwip_ipv4_addr(addr_start);
	dhcp_addr_end.addr 		= lwip_ipv4_addr(addr_end);
	dhcp_server_start(&gnetif[0], &dhcp_addr_start, &dhcp_addr_end);


	netif_set_up(&gnetif[0]);
	lwiperf_start_tcp_server_default(carlink_lwiperf_report_cb_impl, NULL);
	lwip_tcpip_init_done_flag = 1;
	carlink_carplay_add_vendor_ie();
}
#else
static void carlink_start_wlan()
{
	int ret;

	tcpip_init(tcpip_init_done, NULL);
	lwip_tcpip_init_done_flag = 1;	
	ret = start_sta_ext("ghj", "12345678", 1);
#if CARLINK_STA_P2P_COEXIST_TEST
	printf("STA/P2P coexist test: start_sta_ext ret=%d\r\n", ret);
#endif
}
#endif

static void carlink_prepare_p2p_info(void)
{
	char ap_prefix[5] = {0};

#if defined(USE_BW8201)
	fsc_bt_get_dev_addr();
#endif

#if defined(CHIPSET_SUPPORT_AIC8800D)
	g_cp_bt_mac_ready = true;
	memcpy(g_cp_bt_mac, "DC0D1023F235", 12);
#endif

	while(!g_cp_bt_mac_ready) {
		vTaskDelay(pdMS_TO_TICKS(10));
	}

#if defined(WIFI_CHIP_GT) && (CARLINK_EY)
	/* Address of the already applied EY license. */
	memcpy(g_cp_bt_mac, "58C587F4293D", 12);
	g_cp_bt_mac[12] = 0;
#endif

	memcpy(ap_prefix, g_cp_bt_mac + 8, 4);
	carlink_reset_wifi_ap_info(ap_prefix);

#if (CARLINK_EY)
	{
		char cmd_str[256] = {0};

		sprintf(cmd_str, "AT+GATTSET=AAA5device=EY_%s&ssid=%s&pwd=%s&name=%s&mac=%s&IP=192.168.13.1&key=EYMOTOR12FCB99FD224DB2B22F5A7703&w=1024&h=600\r\n",
			g_cp_bt_mac, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, (const char *)carlink_p2p_name, g_cp_bt_mac);

		printf("\r\nhttps://appstore.eryanet.com?productid=f03aad9ac00ec5df&device=EY_%s&ssid=%s&pwd=%s&name=%s&mac=%s&IP=192.168.13.1&port1=11111&port2=11113&key=EYMOTOR12FCB99FD224DB2B22F5A7703&w=1024&h=600\r\n",
			g_cp_bt_mac, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd, (const char *)carlink_p2p_name, g_cp_bt_mac);

		bt_send_at_command(cmd_str, strlen(cmd_str));
	}
#endif
}

#include "lwip/dns.h"

static void dns_found_callback_impl(const char *name, const ip_addr_t *ipaddr, void *arg) {
    if (ipaddr != NULL) {
        printf("DNS resolved: %s -> %s\n", name, ipaddr_ntoa(ipaddr));
        // ´¦Àí½âÎö½á¹û
    } else {
        printf("DNS resolution failed for %s\n", name);
    }
}

void resolve_hostname_async(const char *hostname) {
    err_t err;
	ip_addr_t ipaddr;
    
    err = dns_gethostbyname(hostname, &ipaddr, dns_found_callback_impl, NULL);
    if (err == ERR_OK) {
        printf("Immediately resolved: %s -> %s\n", hostname, ipaddr_ntoa(&ipaddr));
    } else if (err == ERR_INPROGRESS) {
        printf("DNS resolution in progress for %s\n", hostname);
    } else {
        printf("DNS resolution error: %d\n", err);
    }
}

static void dump_ip_addr(const char *msg, const ip_addr_t *server_addr)
{
	printf("%s %d.%d.%d.%d \r\n", msg,
              ip4_addr1_16(ip_2_ip4(server_addr)), ip4_addr2_16(ip_2_ip4(server_addr)), ip4_addr3_16(ip_2_ip4(server_addr)), ip4_addr4_16(ip_2_ip4(server_addr)));
}

#if CARLINK_STA_P2P_COEXIST_TEST
static void sta_p2p_coexist_test_task(void *param)
{
	(void)param;

	printf("STA/P2P coexist test: delay %d ms before P2P\r\n", CARLINK_STA_P2P_DELAY_MS);
	vTaskDelay(pdMS_TO_TICKS(CARLINK_STA_P2P_DELAY_MS));
	carlink_prepare_p2p_info();
	printf("STA/P2P coexist test: start P2P after STA DHCP\r\n");
	start_p2p((const char *)carlink_p2p_name, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd);
	vTaskDelete(NULL);
}
#endif

static void dhcp_client_status_callback(struct netif *netif, int status, const ip_addr_t *server_addr)
{
	printf("dhcp_client_status_callback enter netif=%p gnetif0=%p status=%d server=%p\r\n",
	       netif, &gnetif[0], status, server_addr);
	if (status != 1 || ip4_addr_isany_val(*ip_2_ip4(&netif->ip_addr))) {
		printf("ignore dhcp cb: status=%d ip=%s\r\n", status, ipaddr_ntoa(&netif->ip_addr));
		return;
	}

	dump_ip_addr("dhcp server ip     :", (const ip_addr_t *)server_addr);
	dump_ip_addr("dhcp client ip     :", (const ip_addr_t *)&netif->ip_addr);
	dump_ip_addr("dhcp client netmask:", (const ip_addr_t *)&netif->netmask);
	dump_ip_addr("dhcp client gw     :", (const ip_addr_t *)&netif->gw);

	resolve_hostname_async("www.baidu.com");

#if CARLINK_STA_P2P_COEXIST_TEST
	if (!g_sta_p2p_test_started) {
		g_sta_p2p_test_started = 1;
		printf("STA/P2P coexist test: STA DHCP ready, schedule P2P\r\n");
		if (xTaskCreate(sta_p2p_coexist_test_task, "sta_p2p", 2048, NULL, 1, NULL) != pdPASS) {
			g_sta_p2p_test_started = 0;
			printf("STA/P2P coexist test: create task failed\r\n");
		}
	}
#endif
}

int start_sta_ext(const char* ssid, const char* passwd, char need_passwd)
{
	int ret = -1;
	ip4_addr_t ip_addr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
#if !defined(CHIPSET_SUPPORT_AIC8800D)
	ret = start_sta(ssid, passwd, need_passwd);
	if (ret != 0) {
		printf("start wifi sta failed\r\n");
		return ret;
	}
#endif
	if (lwip_tcpip_init_done_flag) {
		netif_remove(&gnetif[0]);
	}

	netif_add(&gnetif[0],
#if LWIP_IPV4
		&ip_addr, &netmask, &gw,
#endif
		NULL, wlan_ethernetif_init, tcpip_input);
	netif_set_default(&gnetif[0]);
#if defined(CHIPSET_SUPPORT_AIC8800D)
	ret = start_sta(ssid, passwd, need_passwd);
	if (ret != 0) {
		printf("start wifi sta failed\r\n");
		return ret;
	}
#endif
	netif_set_up(&gnetif[0]);
	dhcp_regisger_status_callback(dhcp_client_status_callback);
	dhcp_start(&gnetif[0]);

	return ret;
}

int __restart_p2p(const char *dev_name, const char *ssid, const char *passwd)
{
	int ret = -1;
	ip4_addr_t ip_addr;
	ip4_addr_t netmask;
	ip4_addr_t gw;

	ip_addr.addr = lwip_ipv4_addr(ucIPAddress);
	netmask.addr = lwip_ipv4_addr(ucNetMask);
	gw.addr      = lwip_ipv4_addr(ucGatewayAddress);
#if !defined(CHIPSET_SUPPORT_AIC8800D)
	ret = start_p2p(dev_name, ssid, passwd);
	if (ret != 0) {
		printf("restart wifi p2p failed\r\n");
		return ret;
	}
#endif
	if (!lwip_tcpip_init_done_flag) {
		printf("lwip tcpip is not inited\r\n");
		return ret;
	}

	dhcp_stop(&gnetif[0]);
	netif_remove(&gnetif[0]);

	netif_add(&gnetif[0],
#if LWIP_IPV4
		&ip_addr, &netmask, &gw,
#endif
		NULL, wlan_ethernetif_init, tcpip_input);
#if defined(CHIPSET_SUPPORT_AIC8800D)
	ret = start_p2p(dev_name, ssid, passwd);
	if (ret != 0) {
		printf("restart wifi p2p failed\r\n");
		return ret;
	}
#endif

	netif_set_up(&gnetif[0]);
	ret = 0;
	return ret;
}

int restart_p2p()
{
	if (!g_cp_bt_mac_ready) {
		printf("bt is not ready at %s", __func__);
		return -1;
	}
	return __restart_p2p((const char *)carlink_p2p_name, (const char *)carlink_ap_ssid, (const char *)carlink_ap_passwd);
}

static void carlink_reset_wifi_ap_info(const char *prefex)
{
	if (prefex) {
		memset(carlink_ap_ssid, 0, sizeof(carlink_ap_ssid));
    	sprintf((char *)carlink_ap_ssid,   "%s_%s", DEV_NAME_PREFIX, prefex);
		memset(carlink_p2p_name, 0, sizeof(carlink_p2p_name));
		sprintf((char *)carlink_p2p_name, "DIRECT-%s_p2p_%s", DEV_NAME_PREFIX, prefex);//do not delete "DIRECT-" !!!
	}
}

#if CARLINK_RSSI_POLL_ENABLE
static void rssi_delay_read_task(void *param)
{
	unsigned int generation = (unsigned int)(uintptr_t)param;

	vTaskDelay(pdMS_TO_TICKS(CARLINK_RSSI_POLL_INTERVAL_MS));

	if (generation == g_rssi_poll_generation &&
		g_gatt_connected && g_gatt_client_if[0] && g_gatt_addr[0]) {
		char cmd[64] = {0};

		snprintf(cmd, sizeof(cmd), "AT+KU=%s,%s\r\n", g_gatt_client_if, g_gatt_addr);
		printf("BT AT TX RSSI retry: %s", cmd);
		bt_send_at_command(cmd, strlen(cmd));
	}

	if (generation == g_rssi_poll_generation) {
		g_rssi_retry_pending = 0;
	}
	vTaskDelete(NULL);
}
#endif

static void carlink_bt_callback(char * cAtStr)
{
	char* cmd_para = NULL;
	printf("\r\nBT AT RX: %s %d\r\n", cAtStr, strlen(cAtStr));
	struct carlink_event ev;
	memset((void*)&ev, 0, sizeof(ev));
	ev.type = CARLINK_EVENT_NONE;
	    
	if (0) {
	} else if (0 == strncmp(cAtStr, "OK", 2)) {
		if (g_bt_profile_command_pending) {
			g_bt_profile_command_pending = 0;
			printf("[BT_WIFI_TRACE] profile command ok tick=%u\r\n", (unsigned int)xTaskGetTickCount());
		}
	} else if (strstr(cAtStr, "ERROR") != NULL) {
		if (g_bt_profile_command_pending) {
			g_bt_profile_command_pending = 0;
			printf("[BT_WIFI_TRACE] profile command error before open tick=%u\r\n",
				(unsigned int)xTaskGetTickCount());
		}
	} else if (0 == strncmp(cAtStr, "+IAPDATA=",      9)) {
	        char ble_buf[256] = {0};
		int data_len, i;
		int data_str_len = (strlen(cAtStr) - 9);
		cmd_para = cAtStr + 9;
		data_len = data_str_len / 2;
		string2hex(cmd_para, data_str_len, ble_buf, data_len);
		for (i = 0; i < data_len; i++) {
			printf("%02x ", ble_buf[i]);
		}printf("\r\n");
		//iap2_read_data_proc(ble_buf, data_len);
		carlink_rfcomm_data_read_hook(ble_buf, data_len);
	} else if (0 == strncmp(cAtStr, "+AAPDATA=",      9)) {
        char ble_buf[256] = {0};
		int data_len, i;
		int data_str_len = (strlen(cAtStr) - 9);
		cmd_para = cAtStr + 9;
		data_len = data_str_len / 2;
		string2hex(cmd_para, data_str_len, ble_buf, data_len);
		for (i = 0; i < data_len; i++) {
			printf("%02x ", ble_buf[i]);
		}printf("\r\n");
		//iap2_read_data_proc(ble_buf, data_len);
		carlink_rfcomm_data_read_hook(ble_buf, data_len);
	} else if (0 == strncmp(cAtStr, "+GATTDATA=",      10)) {
		char *gatt_data = cAtStr + 10;
		char *gatt_data_end = strpbrk(gatt_data, "\r\n");
		char *last_sep = NULL;
		char *p;
		int data_str_len;
		int data_len;

		if (gatt_data_end == NULL) {
			gatt_data_end = gatt_data + strlen(gatt_data);
		}
		for (p = gatt_data; p < gatt_data_end; p++) {
			if (*p == ',') {
				last_sep = p;
			}
		}
		if (last_sep != NULL) {
			gatt_data = last_sep + 1;
		}

		data_str_len = (int)(gatt_data_end - gatt_data);
		data_len = data_str_len / 2;
		g_gattdata_peer_rx_total += (uint64_t)data_len;
		printf("GATTDATA peer rx bytes=%d total=%" PRIu64 "\r\n", data_len, g_gattdata_peer_rx_total);
#if CARLINK_RSSI_POLL_ENABLE
	} else if (0 == strncmp(cAtStr, "+RSSI", 5)) {
		if (!g_rssi_retry_pending && g_gatt_connected && g_gatt_client_if[0] && g_gatt_addr[0]) {
			g_rssi_retry_pending = 1;
			if (xTaskCreate(rssi_delay_read_task, "rssi_retry", 1024, (void *)(uintptr_t)g_rssi_poll_generation, 1, NULL) != pdPASS) {
				g_rssi_retry_pending = 0;
			}
		}
#endif
	} else if (0 == strncmp(cAtStr, "+CCCDEN", 7)) {
		if (!g_ble_cccd_test_sent) {
			char cmd[64] = {0};
			unsigned int cccd_handle = 0;
			unsigned int value_handle = 0;

			if (sscanf(cAtStr, "+CCCDEN:%x", &cccd_handle) != 1 || cccd_handle == 0) {
				return;
			}
			value_handle = cccd_handle - 1;
			snprintf(cmd, sizeof(cmd), "AT+GATTDATASEND=%04X,74657374\r\n", value_handle);

			g_ble_cccd_test_sent = 1;
			printf("BT AT TX CCCD test: %s", cmd);
			bt_send_at_command(cmd, strlen(cmd));
		}
	} else if (0 == strncmp(cAtStr, "+GATTSTAT=1",11)) {
        printf("TRACE[%s][%d]:EC_EVENT_BT_DISCONNECT\r\n",__func__ ,__LINE__);
		g_ble_cccd_test_sent = 0;
#if CARLINK_RSSI_POLL_ENABLE
		g_gatt_connected = 0;
		g_gatt_client_if[0] = 0;
		g_gatt_addr[0] = 0;
		g_rssi_retry_pending = 0;
		g_rssi_poll_generation++;
#endif
		ev.type = CARLINK_EVENT_BT_DISCONNECT;
		ev.disable_filter = true;
		carlink_notify_event(&ev);
	} else if (0 == strncmp(cAtStr, "+GATTSTAT=3",11)) {
        printf("TRACE[%s][%d]:EC_EVENT_BT_CONNECT\r\n",__func__ ,__LINE__);
		g_ble_cccd_test_sent = 0;
		if (cAtStr[11] == ',') {
			char cmd_str[64] = {0};
			char *gatt_para = cAtStr + 12;
			char *gatt_para_end = strpbrk(gatt_para, "\r\n");
			char *gatt_para_sep = strchr(gatt_para, ',');
			int gatt_para_len = gatt_para_end ? (int)(gatt_para_end - gatt_para) : (int)strlen(gatt_para);

			if (gatt_para_sep && gatt_para_sep < gatt_para + gatt_para_len &&
				gatt_para_len > 0 && gatt_para_len < (int)(sizeof(cmd_str) - sizeof("AT+KU=\r\n"))) {
				char *addr = gatt_para_sep + 1;
				int client_if_len = (int)(gatt_para_sep - gatt_para);
				int addr_len = gatt_para_len - client_if_len - 1;

				if (addr_len == 12) {
#if CARLINK_RSSI_POLL_ENABLE
					snprintf(g_gatt_client_if, sizeof(g_gatt_client_if), "%.*s", client_if_len, gatt_para);
					snprintf(g_gatt_addr, sizeof(g_gatt_addr), "%.*s", addr_len, addr);
					g_gatt_connected = 1;
					g_rssi_retry_pending = 0;
					g_rssi_poll_generation++;
#endif
					snprintf(cmd_str, sizeof(cmd_str), "AT+KU=%.*s,%.*s\r\n", client_if_len, gatt_para, addr_len, addr);
					bt_send_at_command(cmd_str, strlen(cmd_str));
				}
			}
		}

		ev.type = CARLINK_EVENT_BT_CONNECT;
		ev.disable_filter = true;
		carlink_notify_event(&ev);
	} else if (0 == strncmp(cAtStr, "+ADDR=",       6)) {
		char cmd_str[64] = {0};
		sprintf(cmd_str, "AT+NAME=%s_%s\r\n", DEV_NAME_PREFIX, (cAtStr + 6 + 8));
		printf("ADDR:%s\r\n", cAtStr + 6);
		bt_send_at_command(cmd_str, strlen(cmd_str));//get mac addr
		memset(cmd_str, 0, sizeof(cmd_str));
		sprintf(cmd_str, "AT+LENAME=%s_%s\r\n", DEV_NAME_PREFIX, (cAtStr + 6 + 8));		
		bt_send_at_command(cmd_str, strlen(cmd_str));//get mac addr
		memcpy(g_cp_bt_mac, (cAtStr + 6), 12);
		carlink_carplay_ie_replace_bt_mac(cAtStr + 6, 12);
		g_cp_bt_mac_ready = true;
	} else if (0 == strncmp(cAtStr, "+VER",         4)) {
		char* cmd = "AT+ADDR\r\n";
		bt_send_at_command(cmd, strlen(cmd));//get mac addr
    } else if (0 == strncmp(cAtStr, "+NAME=", 6)) {
		char cmd_str[64] = {0};
		sprintf(cmd_str, "AT+LEADDR\r\n");
		bt_send_at_command(cmd_str, strlen(cmd_str));//get LE addr
	} else if (0 == strncmp(cAtStr, "+LEADDR=", 6)) {
	  char hexMacAddr[6] = {0};
	  char m_tmp_buf[64] = {0};
      string2hex(&cAtStr[8], 12, hexMacAddr, sizeof(hexMacAddr));
      sprintf(m_tmp_buf, "AT+ADVDATA=%s\r\n", hexMacAddr);
      bt_send_at_command(m_tmp_buf, 19);
      sprintf(m_tmp_buf, "AT+SSP=1\r\n");
      bt_send_at_command(m_tmp_buf, strlen(m_tmp_buf));
      sprintf(m_tmp_buf, "AT+PIN=0000\r\n");
      bt_send_at_command(m_tmp_buf, strlen(m_tmp_buf));
      sprintf(m_tmp_buf, "AT+PAIR=3\r\n");
      bt_send_at_command(m_tmp_buf, strlen(m_tmp_buf));
    } else if (0 == strncmp(cAtStr, "+GATTSENT=", 10)) {
        return;
    } else if (0 == strncmp(cAtStr, "+IAPSTAT=3", 10)) {
		ev.type = CARLINK_EVENT_BT_IAP_READY;
		ev.link_type = CARPLAY_WIRELESS;
		carlink_notify_event(&ev);
		
    } else if (0 == strncmp(cAtStr, "+IAPSTAT=1", 10)) {
		ev.type = CARLINK_EVENT_BT_DISCONNECT;
		ev.link_type = CARPLAY_WIRELESS;
		carlink_notify_event(&ev);
		
    } else if (0 == strncmp(cAtStr, "+AAPSTAT=3", 10)) {
		ev.type = CARLINK_EVENT_BT_AA_RFCOMM_READY;
		ev.link_type = AUTO_WIRELESS;
		carlink_notify_event(&ev);
		
    } else if (0 == strncmp(cAtStr, "+AAPSTAT=1", 10)) {
		ev.type = CARLINK_EVENT_BT_DISCONNECT;
		ev.link_type = AUTO_WIRELESS;
		carlink_notify_event(&ev);
    }
}

int carlink_iap_data_write(unsigned char *data, int len)
{
    uint8_t *at_str = NULL, *at_str_ptr = NULL;
    uint8_t *at_prefix = "AT+IAPSEND=";
    int prefix_len = strlen((char *)at_prefix);
    int cmd_len = (len * 2 + prefix_len + 2);

    at_str = (uint8_t *)malloc(cmd_len + 1);
    if (NULL == at_str)
        return -1;
	memset(at_str, 0, cmd_len + 1);
    at_str_ptr = at_str;
    sprintf((char*)at_str_ptr, "%s", (char*)at_prefix);
    at_str_ptr += prefix_len;
    hex2str(data, len, (char *)at_str_ptr);
    at_str_ptr 			+= (len * 2);
    *at_str_ptr++ 		= '\r';
    *at_str_ptr++		= '\n';
    bt_send_at_command((char *)at_str, cmd_len);
    free(at_str);
    return len;
}

int carlink_auto_rfcomm_data_write(unsigned char *data, int len)
{
    uint8_t *at_str = NULL, *at_str_ptr = NULL;
    uint8_t *at_prefix = "AT+AAPSEND=";
    int prefix_len = strlen((char *)at_prefix);
    int cmd_len = (len * 2 + prefix_len + 2);

    if (0) {
		int i;
		printf("AAPSend: ");
		for (i = 0; i < len; i++) {
			printf("%02x ", data[i]);
		}printf("\r\n");
	}

    at_str = (uint8_t *)malloc(cmd_len + 1);
    memset(at_str, 0, cmd_len + 1);
    if (NULL == at_str)
        return -1;
    at_str_ptr = at_str;
    sprintf((char*)at_str_ptr, "%s", (char*)at_prefix);
    at_str_ptr += prefix_len;
    hex2str(data, len, (char *)at_str_ptr);
    at_str_ptr 			+= (len * 2);
    *at_str_ptr++ 		= '\r';
    *at_str_ptr++		= '\n';
    bt_send_at_command((char *)at_str, cmd_len);
    free(at_str);
    return len;
}

void carlink_bt_open_nolock()
{
	unsigned int generation = ++g_bt_open_generation;
	printf("[BT_WIFI_TRACE] schedule bt profile/open gen=%u delay=%u tick=%u\r\n",
		generation, CARLINK_BT_PROFILE_BEFORE_OPEN_DELAY_MS, (unsigned int)xTaskGetTickCount());
	if (xTaskCreate(taskBtOpenWithProfileDelay, "btOpen", 1024, (void *)(uintptr_t)generation, 1, NULL) != pdPASS) {
		char at_str[] = {"AT+BTEN=1\r\n"};
		printf("[BT_WIFI_TRACE] create btOpen task failed, send inline gen=%u tick=%u\r\n",
			generation, (unsigned int)xTaskGetTickCount());
		vTaskDelay(pdMS_TO_TICKS(CARLINK_BT_PROFILE_BEFORE_OPEN_DELAY_MS));
		if (generation == g_bt_open_generation) {
			bt_send_profile_command();
			vTaskDelay(pdMS_TO_TICKS(CARLINK_BT_OPEN_AFTER_PROFILE_DELAY_MS));
			bt_send_at_command((char *)at_str, strlen(at_str));
		}
	}
}


void carlink_bt_open()
{
	pthread_mutex_lock(&btwifiLocker);
	if (0 == g_bt_wifi_init_flag) {
		goto exit;
	}
	carlink_bt_open_nolock();
exit:
	pthread_mutex_unlock(&btwifiLocker);
}

void carlink_bt_close()
{
	char at_str[] = {"AT+BTEN=0\r\n"};
	pthread_mutex_lock(&btwifiLocker);
	if (0 == g_bt_wifi_init_flag) {
		goto exit;
	}

	g_bt_open_generation++;
	bt_send_at_command((char *)at_str, strlen(at_str));
exit:
	pthread_mutex_unlock(&btwifiLocker);

}


static  void carlink_wifi_event_handler( WIFIEvent_t * xEvent )
{
    WIFIEventType_t xEventType = xEvent->xEventType;
	char mac_str[32] = {0};
	char *mac = NULL;
	struct carlink_event ev;
	memset((void*)&ev, 0, sizeof(ev));

	if (xEvent) {
		mac = (char *)xEvent->xInfo.xAPStationConnected.ucMac;
		sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	wps_connect_done = 0;

    if (0) {
    } else if (eWiFiEventConnected                    == xEventType) {// meter is sta
        printf("\r\n The meter is connected to ap \r\n");
    } else if (eWiFiEventDisconnected                == xEventType) {// meter is sta
        printf("\r\n The meter is disconnected from ap \r\n");
    } else if (eWiFiEventAPStationConnected       == xEventType) {// meter is ap
        printf("\r\n The meter in AP is connected by sta %s \r\n", mac_str);
    } else if (eWiFiEventAPStationDisconnected   == xEventType) {// meter is ap
        printf("\r\n The sta %s is disconnected from the meter \r\n", mac_str);

		ev.type = CARLINK_EVENT_WIFI_DISCONNECT;
		ev.disable_filter = true;
		memcpy((void*)ev.u.para, (void*)mac_str, strlen(mac_str));
		carlink_notify_event(&ev);
    }
}

#if ( ipconfigUSE_DHCP_HOOK != 0 )
eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
	eDHCPCallbackAnswer_t eReturn;
	char g_ip_str[32] = {0};

	sprintf(g_ip_str, "%d.%d.%d.%d\r\n", (ulIPAddress >> 0) & 0xFF, 
	(ulIPAddress >> 8) & 0xFF, (ulIPAddress >> 16) & 0xFF, (ulIPAddress >> 24) & 0xFF);
	printf("\r\n  eDHCPPhase:%d  ulIPAddress:%s state:%d \r\n", eDHCPPhase, g_ip_str, getDhcpClientState());
	if (getDhcpClientState() == 0)
		return eDHCPStopNoChanges;

	switch( eDHCPPhase )
	{
	case eDHCPPhaseFinished:
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

static BaseType_t carlink_wifi_init()
{
	static int wifi_sdio_status = MMCSD_HOST_UNPLUGED;

	if (wifi_sdio_status == MMCSD_HOST_PLUGED)
		return 0;
	WIFI_Context_init();
	WIFI_RegisterEvent(eWiFiEventMax, carlink_wifi_event_handler);
	for (;;) {
		wifi_sdio_status = mmcsd_wait_sdio_ready((int32_t)portMAX_DELAY);
		if (wifi_sdio_status == MMCSD_HOST_PLUGED) {
			printf("detect sdio device\r\n");
			break;
		}
	}
	return 0;
}

#if 0
static void bt_set_support_carplay() // cp
{
	char cmd_str[64] = {0};

	sprintf(cmd_str, "AT+PROFILE=33962\r\n");
	bt_send_at_command(cmd_str, strlen(cmd_str));
}
#endif

static void bt_set_support_carplay_android_auto(void)//auto + cp
{
	char cmd_str[64] = {0};

	sprintf(cmd_str, "AT+PROFILE=50859\r\n");
	bt_send_at_command(cmd_str, strlen(cmd_str));
}

static void bt_send_profile_command(void)
{
	g_bt_profile_command_pending = 1;
	bt_set_support_carplay_android_auto();
}

static void taskBtOpenWithProfileDelay(void *param)
{
	unsigned int generation = (unsigned int)(uintptr_t)param;
	char at_str[] = {"AT+BTEN=1\r\n"};

	printf("[BT_WIFI_TRACE] btOpen task delay gen=%u tick=%u\r\n",
		generation, (unsigned int)xTaskGetTickCount());
	vTaskDelay(pdMS_TO_TICKS(CARLINK_BT_PROFILE_BEFORE_OPEN_DELAY_MS));
	if (generation == g_bt_open_generation) {
		printf("[BT_WIFI_TRACE] btOpen task send profile/open gen=%u tick=%u\r\n",
			generation, (unsigned int)xTaskGetTickCount());
		bt_send_profile_command();
		vTaskDelay(pdMS_TO_TICKS(CARLINK_BT_OPEN_AFTER_PROFILE_DELAY_MS));
		bt_send_at_command((char *)at_str, strlen(at_str));
	} else {
		printf("[BT_WIFI_TRACE] btOpen task canceled gen=%u current=%u tick=%u\r\n",
			generation, g_bt_open_generation, (unsigned int)xTaskGetTickCount());
	}
	vTaskDelete(NULL);
}

static void  taskInitCarlinkWlanThread(void* param)
{
	printf("[BT_WIFI_TRACE] taskInitCarlinkWlanThread enter tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	carlink_start_wlan();
	printf("[BT_WIFI_TRACE] taskInitCarlinkWlanThread leave tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	vTaskDelete(NULL);
}

int carlink_test_start_ap()
{
	ip4_addr_t ip_addr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
	ip4_addr_t dhcp_addr_start;
	ip4_addr_t dhcp_addr_end;
	uint8_t addr_start[4] = {192, 168, 13, 20};
	uint8_t addr_end[4] = {192, 168, 13, 30};

	carlink_wifi_init();

	tcpip_init(tcpip_init_done, NULL);

	atbm_wifi_hw_init();
	vTaskDelay(pdMS_TO_TICKS(50));
	if (atbm_wifi_on(ATBM_WIFI_AP_MODE) == NULL) {
		printf("carlink AP test start failed\r\n");
		return -1;
	}
	atbm_wifi_ap_create("AP630_TEST", WLAN_WPA2_AUTH_PSK, WLAN_ENCRYPT_AES, "88888888", 36, 0);

	ip_addr.addr = lwip_ipv4_addr(ucIPAddress);
	netmask.addr = lwip_ipv4_addr(ucNetMask);
	gw.addr = lwip_ipv4_addr(ucGatewayAddress);

	netif_add(&gnetif[0],
#if LWIP_IPV4
		&ip_addr, &netmask, &gw,
#endif
		NULL, wlan_ethernetif_init, tcpip_input);
	netif_set_default(&gnetif[0]);

	dhcp_addr_start.addr = lwip_ipv4_addr(addr_start);
	dhcp_addr_end.addr = lwip_ipv4_addr(addr_end);
	dhcp_server_start(&gnetif[0], &dhcp_addr_start, &dhcp_addr_end);

	netif_set_up(&gnetif[0]);
	lwiperf_start_tcp_server_default(carlink_lwiperf_report_cb_impl, NULL);
	lwip_tcpip_init_done_flag = 1;

	printf("carlink AP test started: ssid=AP630_TEST passwd=88888888 ip=192.168.13.1 channel=36\r\n");
	return 0;
}

int carlink_test_scan()
{
#if defined(WIFI_CHIP_GT)
	char scan_buf[2732] = {0};
	WLAN_SCAN_RESULT *result = (WLAN_SCAN_RESULT *)scan_buf;
	WLAN_BSS_INFO *info = result->bss_info;
	int ret;

	carlink_wifi_init();

	printf("carlink scan test start\r\n");
	atbm_wifi_scan_open();
	ret = atbm_wifi_scan_start(scan_buf, sizeof(scan_buf));
	if (ret == 0) {
		printf("scan count=%d\r\n", result->count);
		for (int i = 0; i < result->count; i++) {
			char ssid[33] = {0};
			int ssid_len = info->SSID_len;

			if (ssid_len > 32) {
				ssid_len = 32;
			}
			memcpy(ssid, info->SSID, ssid_len);

			printf("APIndex:%d\r\n", i);
			printf("SSID:%s\r\n", ssid);
			printf("channel=%d\r\n", info->chanspec);
			printf("security=%d\r\n", info->security);
			printf("rssi=%d\r\n", info->RSSI);
			printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
				info->BSSID[0], info->BSSID[1], info->BSSID[2],
				info->BSSID[3], info->BSSID[4], info->BSSID[5]);
			info++;
		}
	} else {
		printf("scan failed ret=%d\r\n", ret);
	}
	/* Keep WiFi initialized after scan; atbm_wifi_scan_close() tears down
	 * driver tasks and can crash this firmware during standalone testing. */
	printf("carlink scan test end\r\n");
	return ret;
#else
	printf("carlink scan test unsupported wifi chip\r\n");
	return -1;
#endif
}

int carlink_bt_wifi_init()
{
	int retry_count = 0;
	printf("[BT_WIFI_TRACE] carlink_bt_wifi_init enter flag=%d tick=%u\r\n", g_bt_wifi_init_flag, (unsigned int)xTaskGetTickCount());
	pthread_mutex_lock(&btwifiLocker);
	if (g_bt_wifi_init_flag) {
		pthread_mutex_unlock(&btwifiLocker);
		printf("[BT_WIFI_TRACE] carlink_bt_wifi_init skip tick=%u\r\n", (unsigned int)xTaskGetTickCount());
		return 0;
	}
	//carlink_ey_video_init();
	printf("[BT_WIFI_TRACE] carlink_wifi_init enter tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	carlink_wifi_init();
	printf("[BT_WIFI_TRACE] carlink_wifi_init leave tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	printf("[BT_WIFI_TRACE] bt_main enter tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	bt_register_event_cb(NULL, carlink_bt_callback);
	bt_main();
	printf("[BT_WIFI_TRACE] bt_main leave tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	//bt_set_support_carplay();
#if defined(WIFI_CHIP_GT)
	/* Restore ATBM init sequencing: download WiFi+BT firmware and bring the chip
	 * up in STA mode BEFORE the BT stack opens the HCI port. WIFI_On() blocks
	 * until the ATBM firmware (including the BT controller firmware) is ready,
	 * so the controller can answer HCI_Reset when AT+BTEN=1 triggers the BT stack.
	 * wifi_initialize() also sets g_current_mode=Station and wifi_started=1, so the
	 * later start_sta() skips WIFI_Off()/re-init and does not reset the chip while
	 * BT is starting. */
	printf("[BT_WIFI_TRACE] wifi_initialize(STA) enter tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	wifi_initialize(eWiFiModeStation);
	printf("[BT_WIFI_TRACE] wifi_initialize(STA) leave tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	vTaskDelay(pdMS_TO_TICKS(50));
#endif
	carlink_bt_open_nolock();
	lwip_tcpip_init_done_flag = 0;
	xTaskCreate(taskInitCarlinkWlanThread, "initThread", 2048 * 4, NULL, 1, NULL);

	while(lwip_tcpip_init_done_flag == 0) {
		if (retry_count++ > 50)
			break;
		vTaskDelay(pdMS_TO_TICKS(300));
	}

	if (lwip_tcpip_init_done_flag) {
		g_bt_wifi_init_flag = 1;
		printf("bt wlan init ok\r\n");
	} else {
		g_bt_wifi_init_flag = 0;
		printf("bt wlan init failed tick=%u\r\n", (unsigned int)xTaskGetTickCount());
	}
	pthread_mutex_unlock(&btwifiLocker);
	printf("[BT_WIFI_TRACE] carlink_bt_wifi_init leave flag=%d tick=%u\r\n", g_bt_wifi_init_flag, (unsigned int)xTaskGetTickCount());
	return 0;
}
#endif
