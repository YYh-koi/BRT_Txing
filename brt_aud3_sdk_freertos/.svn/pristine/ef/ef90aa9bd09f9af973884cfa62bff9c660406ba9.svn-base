/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
inter.c
Abstract:
This file includes the functions for inter.
Author:
Zeng Yongping
Revision History:2001.9
---------------------------------------------------------------------------*/
#include <asm/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include <net/if.h>
#include <linux/sockios.h>
#include <sys/prctl.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include <netutils/ifc.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_tun.h>
#include <linux/wireless.h>

#include <net/route.h>

#include "sysdep.h"
#include "bluenic_if.h"
#include "btdatatype.h"
#include "pan_ui.h"
#include "bt_debug.h"

#define LOG_TAG "pan_inter"

#define VIRTUAL_LAN_DRIVER "/dev/tun"
#define TAP_IF_NAME "bt-pan"
#define ETH_ADDR_LEN        6
#define NIC_ADDR_LEN 6
#define MAX_PACKET_LEN 2000

#define PROPERTY_KEY_MAX 32
#define PROPERTY_VALUE_MAX 92

#define PAN_PROTOCOL_IP        0x0800
#define PAN_PROTOCOL_ARP       0x0806

#ifdef CONFIG_PAN
static unsigned char g_exitflag = 0;
static int g_nicmgr = -1;

static int local_bd_addr[BD_ADDR_LEN] = {0};
static int local_nic_addr[NIC_ADDR_LEN] = {0};
static const int def_src[BD_ADDR_LEN] = {0};

static BOOL btnic_add(UCHAR *bd_addr);
static char send_msg[MAX_MSG_BUF_LEN];
static void *rcv_msg(void *param);
static int set_NIC_up(const char *devname, UCHAR* addr);
static int set_NIC_down(const char *devname);
static int tap_if_up(const char *devname, UINT8 *addr);

pthread_t g_rthread_id = -1;
typedef void (*data_rcv_cb)(unsigned char *data, BOOL is_general_bd);
static data_rcv_cb s_data_rcv_cb = NULL;

typedef struct eth_hdr {
	unsigned char h_dest[ETH_ADDR_LEN];
	unsigned char h_src[ETH_ADDR_LEN];
	short         h_proto;
} tETH_HDR;
static BOOL BridgeSend2Drv(const char* buf, UINT16 len)
{
	if(g_nicmgr) {
		int ret = write(g_nicmgr, buf, len);
		return TRUE;
	}
	return FALSE;
}
/*
 * */
BOOL api_send_func(struct PAN_EthFrameStru *pfrm, HANDLE port_handle)
{
	bse_print_x(LOG_TAG, "[PAN_DBG]send data from BT to NIC");
	return BridgeSend2Drv(pfrm->frame, pfrm->len);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
This is the function first called to start inter. It opens BTMgr device,
creates the event shared by inter and BTMgr, and creates a thread to receive
messages from BTMgr.
Arguments:
none
Return Value:
BOOL
---------------------------------------------------------------------------*/
BOOL Inter_Init(UCHAR *local_addr , data_rcv_cb cb)
{
	struct ifreq ifr;
	int err;

	if (g_nicmgr != -1) {
		Inter_Done();
	}

	/* open the clone device */
	system("insmod /system/lib/modules/tun.ko");
	if ((g_nicmgr = open(VIRTUAL_LAN_DRIVER, O_RDWR)) == -1) {
		return FALSE;
	}
	s_data_rcv_cb = cb;
	g_exitflag = 0;
//    btnic_add(bd_addr);

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	strncpy(ifr.ifr_name, TAP_IF_NAME, IFNAMSIZ);

	/* try to create the device */
	if( (err = ioctl(g_nicmgr, TUNSETIFF, (void *) &ifr)) < 0 ) { //|| tap_setup_ip(TAP_IF_NAME) == FALSE)
		bse_print_va(LOG_TAG, "ioctl error:%d, errno:%s", err, strerror(errno));
		close(g_nicmgr);
		g_nicmgr = -1;
		return FALSE;
	}

	if(tap_if_up(TAP_IF_NAME, local_addr) != 0) {
		return FALSE;
	}

	pthread_create(&g_rthread_id, NULL, rcv_msg, NULL);
	return TRUE;
}

static int tap_if_up(const char *devname, UINT8 *addr)
{
	struct ifreq ifr;
	int sk, err;

	sk = socket(AF_INET, SOCK_DGRAM, 0);

	/*get mac addr*/
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, IFNAMSIZ - 1);
	err = ioctl(sk, SIOCGIFHWADDR, &ifr);
	if(err < 0) {
		bse_print_va(LOG_TAG, "Could not get network hardware for interface:%s, errno:%s", devname, strerror(errno));
		close(sk);
		return -1;
	}

	/*set mac addr*/
	strncpy(ifr.ifr_name, devname, IFNAMSIZ - 1);
	memcpy(ifr.ifr_hwaddr.sa_data, addr, 6);
	err = ioctl(sk, SIOCSIFHWADDR, (caddr_t)&ifr);

	if (err < 0) {
		bse_print_va(LOG_TAG, "Could not set bt address for interface:%s, errno:%s", devname, strerror(errno));
		close(sk);
		return -1;
	}

	/*bring it up*/
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, IF_NAMESIZE - 1);

	ifr.ifr_flags |= IFF_UP;
	ifr.ifr_flags |= IFF_MULTICAST;

	err = ioctl(sk, SIOCSIFFLAGS, (caddr_t) &ifr);
	if (err < 0) {
		bse_print_va(LOG_TAG, "Could not bring up network interface:%s, errno:%d", devname, errno);
		close(sk);
		return -1;
	}

	close(sk);

	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
This is the function last called to stop inter. It ends the thread responsible
to receive messages from BTMgr, deletes the event and closes BTMgr.
Arguments:
none
Return Value:
none
---------------------------------------------------------------------------*/
void Inter_Done(void)
{
	int i, result;
	void *status;

	if (g_rthread_id != -1) {
		result = pthread_kill(g_rthread_id, SIGQUIT);

		if (result != 0) {
			bse_print_va(LOG_TAG, "pthread_kill error: %d", result);
		}

		result = pthread_join(g_rthread_id, &status);

		if (result != 0) {
			bse_print_va(LOG_TAG, "pthread_join error: recv_msg thread is not joined, result is %d", result);
		}

		g_rthread_id = -1;
	}

	g_exitflag = 1;
	for (i = 0; i < 2; i ++)
		sleep(1);

	if (g_nicmgr != -1) {
		set_NIC_down(TAP_IF_NAME);
		close(g_nicmgr);
		g_nicmgr = -1;
	}

	dhcp_stop(TAP_IF_NAME);
	s_data_rcv_cb = NULL;

	return ;
}

static inline int should_forward(tETH_HDR* hdr)
{
	bse_print_va(LOG_TAG, "Should forward hdr->h_proto is %d\r\n", hdr->h_proto);
	if(ntohs(hdr->h_proto) == ETH_P_IP || ntohs(hdr->h_proto) == ETH_P_ARP)
		return TRUE;
	return FALSE;
}


static BOOL check_general_bd(UINT8* dst_bd)
{
	int i = 0;
	for(i = 0; i < 6; i++) {
		//bse_print_va(LOG_TAG, "dst[%d] is %02x\r\n", i, dst_bd[i]);
		if(dst_bd[i] != 0xff) {
			return FALSE;
		}
		//TODO
	}
	return TRUE;

}

static void process_msg(void *packet, int size)
{
	struct PAN_EthFrameStru *pfrm;
	tETH_HDR eth_hdr;
	BOOL is_general_bd = TRUE;

	memcpy(&eth_hdr, packet, sizeof(tETH_HDR));

	if(should_forward(&eth_hdr)) {
		pfrm = (struct PAN_EthFrameStru *) NEW(sizeof(struct PAN_EthFrameStru) + size);
		if (pfrm == NULL) {
			return ;
		}
		memset(pfrm, 0, sizeof(struct PAN_EthFrameStru));
		memcpy(pfrm->frame, packet, size);
		pfrm->len = size;

		is_general_bd = check_general_bd(&eth_hdr.h_dest);

		if(s_data_rcv_cb) {
			s_data_rcv_cb((unsigned char *)pfrm, is_general_bd);
		}
	}
	return;
}

static void signal_handler(int sig)
{
	pthread_exit(0);
}

static char packet[MAX_PACKET_LEN];
static void *rcv_msg(void *param)
{
	int size;
	ssize_t len;

	sleep(1);
	signal(SIGQUIT, signal_handler);
	len = MAX_MSG_BUF_LEN;

	while (!g_exitflag) {
		memset(packet, 0, MAX_PACKET_LEN);
		if ((size = read(g_nicmgr, packet, len)) == -1) {
			break;
		}

		if (size == 0) {
			continue;
		}

		process_msg(packet, size);
	}
	g_rthread_id = -1;

	return NULL;
}

static int set_NIC_down(const char *devname)
{
	struct ifreq ifr;
	int sd, ret;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, IF_NAMESIZE - 1);

	ifr.ifr_flags &= ~IFF_UP;

	ret = ioctl(sd, SIOCSIFFLAGS, (caddr_t) &ifr);

	close(sd);

	return 0;
}

#endif
