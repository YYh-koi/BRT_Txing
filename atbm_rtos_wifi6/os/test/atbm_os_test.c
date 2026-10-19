#include "atbm_hal.h"
#include "atbm_os_test.h"
#include "atbm_os_thread.h"
//#include "os_drv.h"
extern struct atbmwifi_common g_hw_prv;
extern int atbm_direct_read_reg_32(struct atbmwifi_common *hw_priv, atbm_uint32 addr, atbm_uint32 *val);
struct packed_test{
	char data1;
	int data2;
	short data3;
}atbm_packed;

int atbm_packed_test(){
	if(sizeof(struct packed_test) != 7)
		return -1;
	return 0;
}

int atbm_GetOsTimeTest(){
	if((atbm_GetOsTime() <= 0) ||(atbm_GetOsTimeMs() <= 0)){
		return -1;
	}
	return 0;
}

int atbm_SleepMsTest(){
	atbm_uint32 start, end;
	start = atbm_GetOsTime();
	atbm_SleepMs(1000);
	end = atbm_GetOsTime();
	if((end - start) > 1100 || (end - start < 900)){
		return -1;
	}
	start = atbm_GetOsTime();
	atbm_mdelay(1000);
	end = atbm_GetOsTime();
	if((end - start) > 1100 || (end - start < 900)){
		return -2;
	}
	return 0;
}

int atbm_thread_start = 0;
static int atbm_thread_arg_test_fail = 0;
void atbm_test_thread(atbm_void *p_arg){
	if(((int)p_arg) != 0x5555){
		atbm_thread_arg_test_fail = 1;
	}
	atbm_thread_start = 1;
	while(1){
		atbm_SleepMs(100);
	}
}

int atbm_thread_test(){
	int ret=0;
	int retry = 3;
	int arg = 0x5555;
	pAtbm_thread_t thread;
	//wifi_printk(WIFI_ALWAYS, "retry:%p arg:%p thread:%p atbm_thread_start:%p atbm_thread_arg_test_fail:%p\n", &retry, &arg, &thread, &atbm_thread_start, &atbm_thread_arg_test_fail);
	thread = atbm_createThread("Atbm_TEST",atbm_test_thread, (void*)arg, TEST_TASK_PRIO);
	if(!thread){
		ret= -1;
		goto out;
	}
	while((--retry) >= 0){
		if(atbm_thread_arg_test_fail){
			ret= -2;
			goto out;
		}
		if(atbm_thread_start){
			break;
		}
		atbm_SleepMs(10);
	}
	if(retry < 0){
		ret= -3;
		goto out;
	}

out:
	if (thread)
	{
		atbm_stopThread(thread);

	}
	atbm_thread_arg_test_fail = 0;
	atbm_thread_start = 0;
	return ret;
}

atbm_mutex test_mutex;
int atbm_mutextest_thread_start = 0;
void atbm_mutextest_thread(atbm_void *p_arg){
	atbm_os_mutexLock(&test_mutex, 0xffffffff);
	atbm_mutextest_thread_start = 1;
	atbm_SleepMs(1000);
	atbm_mutextest_thread_start = 2;
	atbm_os_mutexUnLock(&test_mutex);
	atbm_SleepMs(1000);
	atbm_os_mutexLock(&test_mutex, 0xffffffff);
	atbm_mutextest_thread_start = 3;
	atbm_os_mutexUnLock(&test_mutex);
	while(1){
		atbm_SleepMs(1000);
	}
}

int atbm_mutex_test(){
	pAtbm_thread_t thread=NULL;
	int ret=0;
	if(atbm_os_mutexLockInit(&test_mutex)){
		ret=-1;
		goto out;
	}
	thread = atbm_createThread("Atbm_TEST",atbm_mutextest_thread, NULL, TEST_TASK_PRIO);
	while(!atbm_mutextest_thread_start){
		atbm_SleepMs(10);
	}
	if(atbm_mutextest_thread_start != 1){
		ret=-2;
		goto out;
	}
	if(atbm_os_mutexLock(&test_mutex, 10*ATBM_HZ)){
		ret=-3;
		goto out;
	}
	atbm_os_mutexUnLock(&test_mutex);
	if(atbm_mutextest_thread_start == 1){
		ret=-4;
		goto out;
	}
	while(atbm_mutextest_thread_start != 3){
		atbm_SleepMs(10);
	}
	if(atbm_os_DeleteMutex(&test_mutex)){
		ret=-5;
		goto out;
	}
out:
	atbm_stopThread(thread);
	atbm_mutextest_thread_start = 0;
	return ret;
}

atbm_os_wait_queue_head_t test_event;
void atbm_waitEventtest_thread(atbm_void *p_arg){
	atbm_os_wakeup_event(&test_event);
	while(1){
		atbm_SleepMs(1000);
	}
}

int atbm_waitEvent_test(){
	int status;
	int ret=0;
	pAtbm_thread_t thread;
	atbm_uint32 start, end;

	atbm_os_init_waitevent(&test_event);
	thread = atbm_createThread("Atbm_TEST",atbm_waitEventtest_thread, NULL, TEST_TASK_PRIO);
	status = atbm_os_wait_event_timeout(&test_event, 10*ATBM_HZ);
	if(status != 1){
		ret=-1;
		goto out;
	}
	start = atbm_GetOsTime();
	status = atbm_os_wait_event_timeout(&test_event, ATBM_HZ);
	if(status != 0){
		ret=-2;
		goto out;
	}
	end = atbm_GetOsTime();
	if((end - start < 900) || (end - start > 1100)){
		printf("end - start=%d\n",end - start);
		ret=-3;
		goto out;
	}
out:
	atbm_stopThread(thread);
	return ret;
}

void atbm_timeout_func(void * userdata1){
	atbm_os_wait_queue_head_t *event = (atbm_os_wait_queue_head_t *)userdata1;
	atbm_os_wakeup_event(event);
}

int atbm_timer_test(){
	OS_TIMER Timer;
	atbm_os_wait_queue_head_t event;
	atbm_uint32 start, end;

	atbm_os_init_waitevent(&event);
	if(atbm_InitTimer(&Timer, atbm_timeout_func, &event))
		return -1;
	start = atbm_GetOsTime();
	if(atbm_StartTimer(&Timer, ATBM_HZ))
		return -2;
	atbm_os_wait_event_timeout(&event, 10*ATBM_HZ);
	end = atbm_GetOsTime();
	if((end - start < 900) || (end - start > 1100)){
		return -3;
	}
	return 0;
}

int msgPool[4];

void atbm_MsgQtest_thread(atbm_void *p_arg){
	int flag = 0x5555;
	atbm_os_msgq *msgQ = (atbm_os_msgq *)p_arg;
	atbm_os_MsgQ_Send(msgQ, &flag, 4, 0xffffffff);
	while(1){
		atbm_SleepMs(1000);
	}
}

int atbm_MsgQ_test(){
	int ret=0;
	atbm_os_msgq msgQ;
	pAtbm_thread_t thread=NULL;
	int flag;
	if(atbm_os_MsgQ_Create(&msgQ, msgPool, sizeof(int), 4)){
		ret=-1;
		goto out;
	}
	thread = atbm_createThread("Atbm_TEST",atbm_MsgQtest_thread, &msgQ, TEST_TASK_PRIO);
	if(atbm_os_MsgQ_Recv(&msgQ, &flag, sizeof(int), 0xffffffff)){
		ret=-2;
		goto out;
	}
	if(flag != 0x5555){
		ret=-3;
		goto out;		
	}
out:
	atbm_stopThread(thread);
	atbm_os_MsgQ_Delete(&msgQ);
	return ret;
}

int atbm_timesync_test(){
	unsigned int pre, now;
	struct atbmwifi_common	*hw_priv = &g_hw_prv;
	if(atbm_direct_read_reg_32(hw_priv, 0x1640006c, &pre) < 0){
		return -1;
	}
	atbm_SleepMs(1000);
	if(atbm_direct_read_reg_32(hw_priv, 0x1640006c, &now) < 0){
		return -2;
	}
	if((now - pre < 950000) || (now - pre > 1050000)){
		wifi_printk(WIFI_ALWAYS, "F:%s now-pre=%d\n", __func__, (now - pre));
		return -3;
	}
	return 0;
}


int atbm_txrx_test(){
	int time = 10000;
	int ret=0;
	struct atbmwifi_common	*hw_priv = &g_hw_prv;
	unsigned int band_width = 0;
	unsigned int start = atbm_GetOsTime();
	unsigned int now = start;

	while((atbm_GetOsTime() - start) < time){
		if(wsm_txrx_data_test(hw_priv, 1500, 0)){
			ret=-1;
			goto out;
		}
		band_width += 1500;
		if((atbm_GetOsTime() - now) >= ATBM_HZ){
			band_width = band_width << 3;
			wifi_printk(WIFI_ALWAYS, "band width %uM%uK\n", band_width>>20, (band_width>>10)&0x3ff);
			now = atbm_GetOsTime();
			band_width = 0;
		}
	}
out:
	return ret;
}

struct atbm_test_item items[] = {
	{"atbm_packed", atbm_packed_test},
	{"atbm_GetOsTime", atbm_GetOsTimeTest},
	{"atbm_SleepMs", atbm_SleepMsTest},
	{"atbm_thread", atbm_thread_test},
	{"atbm_mutex", atbm_mutex_test},
	{"atbm_waitEvent", atbm_waitEvent_test},
	{"atbm_timer", atbm_timer_test},
	{"atbm_MsgQ", atbm_MsgQ_test},
	/*following test items can only be executed after the setup of SDIO/USB interface*/
	{"atbm_timesync", atbm_timesync_test},
	{"atbm_txrx", atbm_txrx_test},
	{NULL, NULL},
};

int atbm_func_test(){
	int i, ret;
	for(i = 0; items[i].test_func != NULL; i++){
		ret = items[i].test_func();
		if(ret){
			wifi_printk(WIFI_ALWAYS, "%d.Test item[%s] failed[%d]!!\n", i+1, items[i].name, ret);
		}else{
			wifi_printk(WIFI_ALWAYS, "%d.Test item[%s] passed!!\n", i+1, items[i].name);
		}
	}
	return 0;
}

int atbm_func_test_item(int item){
	int ret;

	ret = items[item-1].test_func();
	if(ret){
		wifi_printk(WIFI_ALWAYS, "%d.Test item[%s] failed[%d]!!\n", item, items[item-1].name, ret);
	}else{
		wifi_printk(WIFI_ALWAYS, "%d.Test item[%s] passed!!\n", item, items[item-1].name);
	}
	return 0;
}

//#include "cmdsys.h"
#include "netif.h"

int scan_ap(void)
{
	int i;	
	char *buff;
	buff = malloc(sizeof(char)*2732);
	WLAN_SCAN_RESULT *result = (WLAN_SCAN_RESULT *)buff;
	memset(buff, 0, 2732);
	WLAN_BSS_INFO *info = result->bss_info;
	if(atbm_wifi_get_current_mode() != 0)
	{
		atbm_wifi_on(ATBM_WIFI_STA_MODE);
	}
	atbm_wifi_scan_network(buff,2732);
	for(i =0;i < result->count; i++)
	{
		printf("\nAPIndex:%d\n",i);
		printf("SSID:%s\n",(char *)info->SSID);
		//printf("channel=%d\n",info->ctl_ch);//>channel
		printf("channel=%d\n",info->chanspec);
		printf("security:%d\n",info->security);	
		printf("rssi:%d\n",info->RSSI);	

		printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",info->BSSID[0],info->BSSID[1],info->BSSID[2],info->BSSID[3],info->BSSID[4],info->BSSID[5]);	
		info++;
	}
	free(buff);
	return 0;
}
#if 1
#include "inet.h"
int atbm_ifconfig_test(void)
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
	return 0;	
}
#endif

#if 0
MAINFUNC_ENTRY(atbm, argc, argv)
{
	unsigned int opt,ret,cnt,ms,chan_id;
	char ssid[32];
	char pwd[64];
	char buf[256];
	int rssi;

	if(argc < 2){
		printf("atbm cmd type : atbm [funtion num] [...]\n");
		printf("atbm 0                          atbm os func test\n");
		printf("atbm 1            				atbm_wifi_hw_init  \n");
		printf("atbm 2            				atbm scan AP\n ");
		printf("atbm 3 ssid password 			atbm sta jion ap\n");
		printf("atbm 4           				atbm sta disconnect ap\n");
		printf("atbm 5 ssid password chan_id 	atbm start AP\n");
		printf("atbm 6           				atbm close AP\n");
		return 0;

	}

	memset(ssid,0,32);
	memset(pwd,0,64);
	sscanf(argv[1], "%d",&opt);
	if(argc > 2)
		sscanf(argv[2], "%s",ssid);
	if(argc > 3)
		sscanf(argv[3], "%s",pwd);	

	printf("atbm opt : %d \n", opt);
	printf("ssid : %s\n", ssid);
	printf("password : %s\n", pwd);

	switch(opt)
	{
		case 0:
			atbm_func_test();
			break;
		case 1:
			atbm_wifi_hw_init();
			break;
		case 2:
			scan_ap();	
			break;
		case 3:
			ret = atbm_wifi_get_current_mode();
			if( ret != 0)
			{
				atbm_wifi_on(ATBM_WIFI_STA_MODE);
			}
			atbm_wifi_sta_join_ap(ssid, NULL, 0, 0, pwd);
			break;
		case 4:
			ret=atbm_wifi_sta_disjoin_ap();
			atbm_SleepMs(200);
			break;
		case 5:
			sscanf(argv[4], "%d",&chan_id);
			printf("AP channel : %d\n", chan_id);
			atbm_wifi_on(ATBM_NL80211_IFTYPE_AP);
			ret = atbm_wifi_ap_create(ssid, (pwd && pwd[0]) ? 0x4 : 0, 0, pwd, chan_id, 0);
			break;
		case 6:
			printf("atbm wifi mode AP set off.\n");
			atbm_wifi_off(ATBM_WIFI_AP_MODE);
			break;
		case 7:
			printf("test ifconfig\n");
			atbm_ifconfig_test();
			break;
		case 8:
			printf("sta mod\n");
			atbm_wifi_on(ATBM_WIFI_STA_MODE);
			break;
		case 9:
			printf("open ap\n");
			atbm_wifi_on(ATBM_NL80211_IFTYPE_AP);
			ret = atbm_wifi_ap_create(ssid, 0, 0, pwd, 0, 0);
			break;
		case 10:
			printf("5G ap\n");
			sscanf(argv[4], "%d",&chan_id);
			printf("AP channel : %d\n", chan_id);
			atbm_wifi_on(ATBM_NL80211_IFTYPE_AP);
			atbm_SleepMs(200);
			atbm_ap_set_work_mode(ATBM_WIFI_AP_5G_MODE);
			ret = atbm_wifi_ap_create(ssid, (pwd && pwd[0]) ? 0x4 : 0, 0, pwd, chan_id, 0);
			break;
		case 11:
			printf("24G ap\n");
			sscanf(argv[4], "%d",&chan_id);
			printf("AP channel : %d\n", chan_id);
			atbm_wifi_on(ATBM_NL80211_IFTYPE_AP);
			atbm_SleepMs(200);
			atbm_ap_set_work_mode(ATBM_WIFI_AP_24G_MODE);
			ret = atbm_wifi_ap_create(ssid, (pwd && pwd[0]) ? 0x4 : 0, 0, pwd, chan_id, 0);
			break;
		default:
			break;

	}



	return 0;
}


extern atbm_int32 atbmwifi_enable_lmaclog(atbm_uint32 value );
MAINFUNC_ENTRY(lmac_log, argc, args)
{
	int enable=0;

	if(argc <2){
		return -1;
	}
	enable=atoi(args[1]);
	atbmwifi_enable_lmaclog(enable);
	return 0;
}

extern int ping_custom_sync(const ip_addr_t* ping_addr, uint32_t count, uint32_t delay_ms);
MAINFUNC_ENTRY(ping, argc, argv)
{
	unsigned int ip[4];
	int cnt, ms, ret;
	ip_addr_t ip_t;

	if(argc < 4)
	{
		printf("Usage: ping [ip addr] [cnt] [timeout] \n");
		return 0;
	}
	sscanf(argv[1], "%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
	sscanf(argv[2], "%d",&cnt);
	sscanf(argv[3], "%d",&ms);

	printf("\n    %d.%d.%d.%d    cnt=%d ms=%d \n", ip[0],ip[1],ip[2],ip[3],cnt,ms);

	IP4_ADDR(&ip_t,ip[0],ip[1],ip[2],ip[3]);
	ret= ping_custom_sync(&ip_t, cnt, ms);
	printf("suc=%d fail=%d\n",  ret,cnt-ret);
	return 0;
}

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
extern atbm_int8 atbm_wifi_get_bssid(atbm_uint8 *bssid, atbm_int32 if_id);
MAINFUNC_ENTRY(stat, argc, argv)
{
	ATBM_WLAN_CONNECTION_INFO info;
	unsigned char bssid[6]={0};
	memset(&info,0,sizeof(ATBM_WLAN_CONNECTION_INFO));
	if(!atbm_wifi_isconnected(0)){
		iot_printf("wifi is not connected!\n");
		return 0;
	}
	atbm_wifi_get_bssid(bssid, 0);
	atbm_wifi_get_connected_info(&info);
	iot_printf("bssid:"MACSTR"\n",MAC2STR(bssid));
	iot_printf("ssid:%s \n",info.Ssid);
	iot_printf("rssi:%d \n",info.Rssi);
	//iot_printf("tx_rate:%d\n",atbmwifi_get_current_txrate());
	iot_printf("channel:%d \n",info.channel);
	return 0;
}


extern void atbm_wifi_test_func(int argc, char **args);

MAINFUNC_ENTRY(atbm_wifi_test, argc, argv)
{
	atbm_wifi_test_func(argc, argv);
	return 0;		
}

MAINFUNC_ENTRY(lmaclog, argc, argv)
{
	//atbm_wifi_test_func(argc, argv);
	int level;
	int ret;

	if(argc < 2)
	{
		printf("too few args.\n");
		return 0;
	}
	
	sscanf(argv[1], "%d",&level);

	//ret = atbmwifi_lmac_setdebug(level);
	if(ret)
		printf("set gain err\n");
	
	return 0;		
}

MAINFUNC_ENTRY(ccastart, argc, argv)
{
	//atbm_wifi_test_func(argc, argv);
	int level;
	int ret;

	if(argc < 2)
	{
		printf("too few args.\n");
		return 0;
	}
	
	sscanf(argv[1], "%d",&level);

	//ret = atbmwifi_lmac_debug_setdebug(level);
	if(ret)
		printf("set gain err\n");
	
	return 0;		
}
#endif


