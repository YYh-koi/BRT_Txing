/****************************************************************************
 *
 *  Name:       btsnoopdisp.c
 *
 *  Function:   this file contains functions to generate a BTSNOOP file
 *
 *
 ****************************************************************************/
#include "btdatatype.h"
#include <stdio.h>
//#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
//#include <sys/prctl.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>

//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <netdb.h>

/* for gettimeofday */
#include <time.h>
/* for the S_* open parameters */
//#include <sys/stat.h>
/* for write */
#include <unistd.h>
/* for O_* open parameters */
#include <fcntl.h>
/* defines the O_* open parameters */
#include <fcntl.h>
#include <string.h>

#ifndef MODULE_TAG
#define MODULE_TAG "btsnoop"
#endif
#include "bt_debug.h"
#include "bt_snoop.h"
#include "porting_utils.h"
#include "global.h"

#ifdef CONFIG_OPEN_FILTER_DATA
#include "porting.h"
#include "bt_snoop.h"

typedef struct {
	uint16_t cid;
	long stream_handle;
	uint16_t used;
} avdtp_cid_t;

#define MAX_AVDTP_CONN_NUM 4
static avdtp_cid_t avdtp_cids[MAX_AVDTP_CONN_NUM];
#endif


//#define BTSNOOP_DBG
#ifdef BTSNOOP_DBG
#define SNOOPDBG(fmt, arg...)  bse_print_va("btsnoop", " %s[%s]: " fmt, __FUNCTION__ , get_current_time_string(), ## arg)
#else
#define SNOOPDBG(param, arg...) {}
#endif

#undef BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD
//#define BTSNOOP_EXT_PARSER_INCLUDED TRUE
#define BTSNOOPDISP_INCLUDED FALSE
static pthread_mutex_t utils_mutex;
static unsigned char s_buffer[1024];
static unsigned int s_btsnoop_size = 0;
#ifdef BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD

static struct ListStru s_btsnoop_data_list;
static unsigned char s_btsnoop_capture_ev = 0;
void *btsnoop_msg_ev;
static unsigned int btsnoop_capture_lock;
extern void *List_RemoveHead(struct ListStru *List);

struct btsnoop_capture_data_t {
        unsigned char *p_buf;
        unsigned char event;
        uint32_t len;
        uint8_t is_rcvd;
};
enum {/* mask value of s_btsnoop_capture_ev */
        CAPTURE_READY = 0x01,                   /* quit  following event/timer */
        CAPTURE_IDLE = 0x02                             /* BTSNOOP CAPTURE is idle now */
};

#endif

enum {
    BTSNOOP_UNINIT,
    BTSNOOP_INIT,
};
static int s_btsnoop_status = BTSNOOP_UNINIT;
static void set_btsnoop_status(int status)
{
	s_btsnoop_status = status;
}

static int get_btsnoop_status()
{
	return s_btsnoop_status;
}

static void utils_init(void)
{
	pthread_mutex_init(&utils_mutex, NULL);
}

static void utils_lock(void)
{
	pthread_mutex_lock(&utils_mutex);
}

static void utils_unlock(void)
{
	pthread_mutex_unlock(&utils_mutex);
}

void hex_dump(unsigned char *data, size_t len)
{
	unsigned long i, j, l;
	unsigned char tmp_str[140];
	unsigned char tmp_str1[10];

	for (i = 0; i < len; i += 16) {
		int n ;

		tmp_str[0] = '\0';
		n = i ;

		for (j = 0; j < 4; j++) {
			l = n % 16;

			if (l >= 10)
				tmp_str[3 - j] = (unsigned char)('A' + l - 10);
			else
				tmp_str[3 - j] = (unsigned char)(l + '0');

			n >>= 4 ;
		}

		tmp_str[4] = '\0';
		strcat((char *) tmp_str, ": ");

		/*
		   Output the hex bytes
		 */
		for (j = i; j < (i + 16); j ++) {
			int m ;

			if (j < len) {
				m = ((unsigned int)((unsigned char) * (data + j))) / 16 ;

				if (m >= 10)
					tmp_str1[0] = 'A' + (unsigned char) m - 10;
				else
					tmp_str1[0] = (unsigned char) m + '0';

				m = ((unsigned int)((unsigned char) * (data + j))) % 16 ;

				if (m >= 10)
					tmp_str1[1] = 'A' + (unsigned char) m - 10;
				else
					tmp_str1[1] = (unsigned char) m + '0';

				tmp_str1[2] = '\0';
				strcat((char *) tmp_str, (char *) tmp_str1);
				strcat((char *) tmp_str, " ");
			} else {
				strcat((char *) tmp_str, "   ");
			}
		}

		strcat((char *) tmp_str, "  ");
		l = strlen((char *) tmp_str);

		/* Output the ASCII bytes */
		for (j = i; j < (i + 16); j++) {
			if (j < len) {
				char c = * (data + j);

				if (c < ' ' || c > 'z') {
					c = '.';
				}

				tmp_str[l ++] = c;
			} else {
				tmp_str[l ++] = ' ';
			}
		}

		tmp_str[l ++] = '\r';
		tmp_str[l ++] = '\n';
		tmp_str[l ++] = '\0';
		SNOOPDBG("%s", (const char *) tmp_str);     //add (const char*)
	}
}

/* file descriptor of the BT snoop file (by default, -1 means disabled) */
int hci_btsnoop_fd = -1;

/* Macro to perform a multiplication of 2 unsigned 32bit values and store the result
 * in an unsigned 64 bit value (as two 32 bit variables):
 * u64 = u32In1 * u32In2
 * u32OutLow = u64[31:0]
 * u32OutHi = u64[63:32]
 * basically the algorithm:
 * (hi1*2^16 + lo1)*(hi2*2^16 + lo2) = lo1*lo2 + (hi1*hi2)*2^32 + (hi1*lo2 + hi2*lo1)*2^16
 * and the carry is propagated 16 bit by 16 bit:
 * result[15:0] = lo1*lo2 & 0xFFFF
 * result[31:16] = ((lo1*lo2) >> 16) + (hi1*lo2 + hi2*lo1)
 * and so on
 */
#define HCIDISP_MULT_64(u32In1, u32In2, u32OutLo, u32OutHi)                             \
    do {                                                                                    \
        uint32_t u32In1Tmp = u32In1;                                                          \
        uint32_t u32In2Tmp = u32In2;                                                          \
        uint32_t u32Tmp, u32Carry;                                                            \
        u32OutLo = (u32In1Tmp & 0xFFFF) * (u32In2Tmp & 0xFFFF);              /*lo1*lo2*/    \
        u32OutHi = ((u32In1Tmp >> 16) & 0xFFFF) * ((u32In2Tmp >> 16) & 0xFFFF); /*hi1*hi2*/ \
        u32Tmp = (u32In1Tmp & 0xFFFF) * ((u32In2Tmp >> 16) & 0xFFFF);  /*lo1*hi2*/          \
        u32Carry = (uint32_t)((u32OutLo>>16)&0xFFFF);                                         \
        u32Carry += (u32Tmp&0xFFFF);                                                        \
        u32OutLo += (u32Tmp << 16) ;                                                        \
        u32OutHi += (u32Tmp >> 16);                                                         \
        u32Tmp = ((u32In1Tmp >> 16) & 0xFFFF) * (u32In2Tmp & 0xFFFF);                       \
        u32Carry += (u32Tmp)&0xFFFF;                                                        \
        u32Carry>>=16;                                                                      \
        u32OutLo += (u32Tmp << 16);                                                         \
        u32OutHi += (u32Tmp >> 16);                                                         \
        u32OutHi += u32Carry;                                                               \
    } while (0)

/* Macro to make an addition of 2 64 bit values:
 * result = (u32OutHi & u32OutLo) + (u32InHi & u32InLo)
 * u32OutHi = result[63:32]
 * u32OutLo = result[31:0]
 */
#define HCIDISP_ADD_64(u32InLo, u32InHi, u32OutLo, u32OutHi)                            \
    do {                                                                                    \
        (u32OutLo) += (u32InLo);                                                            \
        if ((u32OutLo) < (u32InLo)) (u32OutHi)++;                                           \
        (u32OutHi) += (u32InHi);                                                            \
    } while (0)

/* EPOCH in microseconds since 01/01/0000 : 0x00dcddb3.0f2f8000 */
#define BTSNOOP_EPOCH_HI 0x00dcddb3U
#define BTSNOOP_EPOCH_LO 0x0f2f8000U

/*******************************************************************************
 **
 ** Function         tv_to_btsnoop_ts
 **
 ** Description      This function generate a BT Snoop timestamp.
 **
 ** Returns          void
 **
 ** NOTE
 ** The return value is 64 bit as 2 32 bit variables out_lo and * out_hi.
 ** A BT Snoop timestamp is the number of microseconds since 01/01/0000.
 ** The timeval structure contains the number of microseconds since EPOCH
 ** (01/01/1970) encoded as: tv.tv_sec, number of seconds since EPOCH and
 ** tv_usec, number of microseconds in current second
 **
 ** Therefore the algorithm is:
 **  result = tv.tv_sec * 1000000
 **  result += tv.tv_usec
 **  result += EPOCH_OFFSET
 *******************************************************************************/
static void tv_to_btsnoop_ts(uint32_t *out_lo, uint32_t *out_hi, struct timeval *tv)
{
	/* multiply the seconds by 1000000 */
	HCIDISP_MULT_64(tv->tv_sec, 0xf4240, *out_lo, *out_hi);

	/* add the microseconds */
	HCIDISP_ADD_64((uint32_t)tv->tv_usec, 0, *out_lo, *out_hi);

	/* add the epoch */
	HCIDISP_ADD_64(BTSNOOP_EPOCH_LO, BTSNOOP_EPOCH_HI, *out_lo, *out_hi);
}

/*******************************************************************************
 **
 ** Function         l_to_be
 **
 ** Description      Function to convert a 32 bit value into big endian format
 **
 ** Returns          32 bit value in big endian format
*******************************************************************************/
static uint32_t l_to_be(uint32_t x)
{
#if __BIG_ENDIAN != TRUE
	x = (x >> 24) |
	    ((x >> 8) & 0xFF00) |
	    ((x << 8) & 0xFF0000) |
	    (x << 24);
#endif
	return x;
}

/*******************************************************************************
 **
 ** Function         btsnoop_is_open
 **
 ** Description      Function to check if BTSNOOP is open
 **
 ** Returns          1 if open otherwise 0
*******************************************************************************/
int btsnoop_is_open(void)
{
#if defined(BTSNOOPDISP_INCLUDED) && (BTSNOOPDISP_INCLUDED == TRUE)
	SNOOPDBG("btsnoop_is_open: snoop fd = %d\n", hci_btsnoop_fd);

	if (hci_btsnoop_fd != -1) {
		return 1;
	}

	return 0;
#else
	return 2;  /* Snoop not available  */
#endif
}

/*******************************************************************************
 **
 ** Function         btsnoop_log_open
 **
 ** Description      Function to open the BTSNOOP file
 **
 ** Returns          None
*******************************************************************************/
static int btsnoop_log_open(char *btsnoop_logfile)
{
#if defined(BTSNOOPDISP_INCLUDED) && (BTSNOOPDISP_INCLUDED == TRUE)
	hci_btsnoop_fd = -1;

	SNOOPDBG("btsnoop_log_open: snoop log file = %s\n", btsnoop_logfile);

	/* write the BT snoop header */
	if ((btsnoop_logfile != NULL) && (strlen(btsnoop_logfile) != 0)) {
		hci_btsnoop_fd = open(btsnoop_logfile, \
		                      O_WRONLY | O_CREAT | O_TRUNC, \
		                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

		if (hci_btsnoop_fd == -1) {
			perror("open");
			SNOOPDBG("btsnoop_log_open: Unable to open snoop log file\n");
			hci_btsnoop_fd = -1;
			return 0;
		}
		chmod(btsnoop_logfile, 0666);
		write(hci_btsnoop_fd, "btsnoop\0\0\0\0\1\0\0\x3\xea", 16);
		return 1;
	}

#endif
	return 2;  /* Snoop not available  */
}

/*******************************************************************************
 **
 ** Function         btsnoop_log_close
 **
 ** Description      Function to close the BTSNOOP file
 **
 ** Returns          None
*******************************************************************************/
static int btsnoop_log_close(void)
{
#if defined(BTSNOOPDISP_INCLUDED) && (BTSNOOPDISP_INCLUDED == TRUE)

	/* write the BT snoop header */
	if (hci_btsnoop_fd != -1) {
		SNOOPDBG("btsnoop_log_close: Closing snoop log file\n");
		close(hci_btsnoop_fd);
		hci_btsnoop_fd = -1;
		return 1;
	}

	return 0;
#else
	return 2;  /* Snoop not available  */
#endif
}

/*******************************************************************************
 **
 ** Function         btsnoop_hci_cmd
 **
 ** Description      Function to add a command in the BTSNOOP file
 **
 ** Returns          None
*******************************************************************************/
void btsnoop_hci_cmd(unsigned char *p)
{

	if (hci_btsnoop_fd != -1) {
		uint32_t value, value_hi;
		struct timeval tv;

		/* since these display functions are called from different contexts */
		utils_lock();

		/* store the length in both original and included fields */
		SNOOPDBG("btsnoop_hci_cmd: fd = %d, len:%d", hci_btsnoop_fd, p[2]);
		value = l_to_be(p[2] + 4);
		write(hci_btsnoop_fd, &value, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* flags: command sent from the host */
		value = l_to_be(2);
		write(hci_btsnoop_fd, &value, 4);
		/* drops: none */
		value = 0;
		write(hci_btsnoop_fd, &value, 4);
		/* time */
		gettimeofday(&tv, NULL);
		tv_to_btsnoop_ts(&value, &value_hi, &tv);
		value_hi = l_to_be(value_hi);
		value = l_to_be(value);
		write(hci_btsnoop_fd, &value_hi, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* data */
		write(hci_btsnoop_fd, "\x1", 1);
		write(hci_btsnoop_fd, p, p[2] + 3);

		/* since these display functions are called from different contexts */
		utils_unlock();
	}
}

/*******************************************************************************
 **
 ** Function         btsnoop_hci_evt
 **
 ** Description      Function to add a event in the BTSNOOP file
 **
 ** Returns          None
*******************************************************************************/
void btsnoop_hci_evt(unsigned char *p)
{

	if (hci_btsnoop_fd != -1) {
		uint32_t value, value_hi;
		struct timeval tv;

		/* since these display functions are called from different contexts */
		utils_lock();

		SNOOPDBG("btsnoop_hci_evt: fd = %d, len = %d.", hci_btsnoop_fd, p[1]);
		/* store the length in both original and included fields */
		value = l_to_be(p[1] + 3);
		write(hci_btsnoop_fd, &value, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* flags: event received in the host */
		value = l_to_be(3);
		write(hci_btsnoop_fd, &value, 4);
		/* drops: none */
		value = 0;
		write(hci_btsnoop_fd, &value, 4);
		/* time */
		gettimeofday(&tv, NULL);
		tv_to_btsnoop_ts(&value, &value_hi, &tv);
		value_hi = l_to_be(value_hi);
		value = l_to_be(value);
		write(hci_btsnoop_fd, &value_hi, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* data */
		write(hci_btsnoop_fd, "\x4", 1);
		write(hci_btsnoop_fd, p, p[1] + 2);

		/* since these display functions are called from different contexts */
		utils_unlock();
	}
}

/*******************************************************************************
 **
 ** Function         btsnoop_sco_data
 **
 ** Description      Function to add a SCO data packet in the BTSNOOP file
 **
 ** Returns          None
*******************************************************************************/
void btsnoop_sco_data(unsigned char *p, uint8_t is_rcvd)
{
	SNOOPDBG("btsnoop_sco_data: fd = %d", hci_btsnoop_fd);

	if (hci_btsnoop_fd != -1) {
		uint32_t value, value_hi;
		struct timeval tv;

		/* since these display functions are called from different contexts */
		utils_lock();

		/* store the length in both original and included fields */
		value = l_to_be(p[2] + 4);
		write(hci_btsnoop_fd, &value, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* flags: data can be sent or received */
		value = l_to_be(is_rcvd ? 1 : 0);
		write(hci_btsnoop_fd, &value, 4);
		/* drops: none */
		value = 0;
		write(hci_btsnoop_fd, &value, 4);
		/* time */
		gettimeofday(&tv, NULL);
		tv_to_btsnoop_ts(&value, &value_hi, &tv);
		value_hi = l_to_be(value_hi);
		value = l_to_be(value);
		write(hci_btsnoop_fd, &value_hi, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* data */
		write(hci_btsnoop_fd, "\x3", 1);
		write(hci_btsnoop_fd, p, p[2] + 3);

		/* since these display functions are called from different contexts */
		utils_unlock();
	}
}

/*******************************************************************************
 **
 ** Function         btsnoop_acl_data
 **
 ** Description      Function to add an ACL data packet in the BTSNOOP file
 **
 ** Returns          None
*******************************************************************************/
void btsnoop_acl_data(unsigned char *p, uint8_t is_rcvd)
{

	if (hci_btsnoop_fd != -1) {
		uint32_t value, value_hi;
		struct timeval tv;

		/* since these display functions are called from different contexts */
		utils_lock();

		/* store the length in both original and included fields */
		value = l_to_be((p[3] << 8) + p[2] + 5);
		SNOOPDBG("btsnoop_acl_data: fd = %d, len:%d", hci_btsnoop_fd, (value - 5));
		write(hci_btsnoop_fd, &value, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* flags: data can be sent or received */
		value = l_to_be(is_rcvd ? 1 : 0);
		write(hci_btsnoop_fd, &value, 4);
		/* drops: none */
		value = 0;
		write(hci_btsnoop_fd, &value, 4);
		/* time */
		gettimeofday(&tv, NULL);
		tv_to_btsnoop_ts(&value, &value_hi, &tv);
		value_hi = l_to_be(value_hi);
		value = l_to_be(value);
		write(hci_btsnoop_fd, &value_hi, 4);
		write(hci_btsnoop_fd, &value, 4);
		/* data */
		write(hci_btsnoop_fd, "\x2", 1);
		write(hci_btsnoop_fd, p, (p[3] << 8) + p[2] + 4);

		/* since these display functions are called from different contexts */
		utils_unlock();
	}
}


/********************************************************************************
 ** API allow external realtime parsing of output using e.g hcidump
 *********************************************************************************/

#define EXT_PARSER_PORT 4338

static pthread_t thread_id;
static int s_listen = -1;
static int ext_parser_fd = -1;

static void ext_parser_detached(void);

static int ext_parser_accept(int port)
{
	socklen_t           clilen;
	struct sockaddr_in  cliaddr, servaddr;
	int s, srvlen;
	int n = 1;
	int size_n;
	int result = 0;

	SNOOPDBG("waiting for connection on port %d", port);
	if (s_listen > 0) {
		SNOOPDBG("The previous listen socket did not shutdown!");
		close(s_listen);
		s_listen = -1;
	}

	s_listen = socket(AF_INET, SOCK_STREAM, 0);

	if (s_listen < 0) {
		SNOOPDBG("listener not created: listen fd %d", s_listen);
		return -1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	srvlen = sizeof(servaddr);

	/* allow reuse of sock addr upon bind */
	result = setsockopt(s_listen, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

	if (result < 0) {
		perror("setsockopt");
	}

	result = bind(s_listen, (struct sockaddr *) &servaddr, srvlen);

	if (result < 0)
		perror("bind");

	result = listen(s_listen, 1);

	if (result < 0)
		perror("listen");

	clilen = sizeof(struct sockaddr_in);

	s = accept(s_listen, (struct sockaddr *) &cliaddr, &clilen);

	if (s < 0) {
		perror("accept");
		return -1;
	}

	SNOOPDBG("connected (%d)", s);

	return s;
}

static int send_ext_parser(char *p, int len)
{
	int n = 0;

	/* check if io socket is connected */
	if (ext_parser_fd == -1)
		return 0;

	SNOOPDBG("write %d to snoop socket\n", len);

	utils_lock();
	if (ext_parser_fd > 0) {
		n = write(ext_parser_fd, p, len);
	}
	utils_unlock();

	if (n <= 0) {
		ext_parser_detached();
	}

	return n;
}

static void ext_parser_detached(void)
{
	//SNOOPDBG("ext parser detached");

	utils_lock();
	if (ext_parser_fd > 0)
		close(ext_parser_fd);

	if (s_listen > 0)
		close(s_listen);

	ext_parser_fd = -1;
	s_listen = -1;
	utils_unlock();
}

static void interruptFn(int sig)
{
	//SNOOPDBG("interruptFn");
	//pthread_exit(0);
}

static void ext_parser_thread(void *param)
{
	int fd;
	int sig = SIGUSR2;
	sigset_t sigSet;
	sigemptyset(&sigSet);
	sigaddset(&sigSet, sig);

	//SNOOPDBG("ext_parser_thread");

	prctl(PR_SET_NAME, (unsigned long)"BtsnoopExtParser", 0, 0, 0);

	pthread_sigmask(SIG_UNBLOCK, &sigSet, NULL);

	struct sigaction act;
	act.sa_handler = interruptFn;
	sigaction(sig, &act, NULL);

	do {
		fd = ext_parser_accept(EXT_PARSER_PORT);

		utils_lock();
		ext_parser_fd = fd;

		SNOOPDBG("ext parser attached on fd %d\n", ext_parser_fd);
		utils_unlock();
	} while (1);
}

void btsnoop_stop_listener(void)
{
	ext_parser_detached();
}

#ifdef BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD
void btsnoop_capture_loop(void)
{
	for (; get_btsnoop_status() == BTSNOOP_INIT; ) {
		ENTER_CRITICAL(btsnoop_capture_lock);
		struct btsnoop_capture_data_t *node ;
		node = List_RemoveHead(&s_btsnoop_data_list);
		LEAVE_CRITICAL(btsnoop_capture_lock);
		
		if (node != NULL) {/* Internal divisive events */
			s_btsnoop_capture_ev &= ~CAPTURE_IDLE;
			
			SNOOPDBG("btsnoop capture event: %d", node->event);
			switch (node->event) {
			case 1: // COMMAND
				btsnoop_hci_cmd(node->p_buf);
				break;

			case 2: {// ACL
				//btsnoop_acl_data(node->p_buf, node->is_rcvd);
#ifdef CONFIG_OPEN_FILTER_DATA
				if (!(porting_get_debug_level() & BSE_DEBUG_BTSNOOP_A2DP_AUDIO)) {//no need to filter, all data write to snoop
					btsnoop_acl_data(node->p_buf, node->is_rcvd);
					break;
				}
				//printf("btsnoop node->is_rcvd = %d node->len = %d\n", node->is_rcvd, node->len);
				uint8_t hit = 0;//hit means no need to write to snoop
				if (node->is_rcvd) {//for a2dp_sink and pbap PCE ? a2dp_src is the same ????
					if (node->len >= 9) {					
						if (brt_debug_check_avdtp_media(node->p_buf) == 1) {
							//no need to write avdtp media data
							//LOG(" %s[%s]: l2cap_cid = %04X", __FUNCTION__ , get_current_time_string(), l2cap_cid);
							hit = 1;
						} /*else if (brt_debug_check_pbap_tl_hdl(node->p_buf) == 1) {
							hit = 1;
						}*/
					}
				} else {//for a2dp_src. do this for seperate close or open
					if (node->len >= 9) {
						if (brt_debug_check_avdtp_src_media(node->p_buf) == 1) {
							hit = 1;
						}
					}
				}
				//printf("btsnoop 111 hit = %d\n", hit);
				if (hit) {
				} else {
					btsnoop_acl_data(node->p_buf, node->is_rcvd);
				}
#else
				btsnoop_acl_data(node->p_buf, node->is_rcvd);
#endif
			}
				break;

			case 3: // SCO
				btsnoop_sco_data(node->p_buf, node->is_rcvd);
				break;

			case 4: // EVENT
				btsnoop_hci_evt(node->p_buf);
				break;
			}

			if (node->p_buf != NULL) {
				FREE(node->p_buf);
			}
			LFREE(node);
		} else {/* This path go idle, wait for new ev */
			SNOOPDBG("btsnoop file waiting for capture!");
			s_btsnoop_capture_ev |= CAPTURE_IDLE;
			WAITFOR_SINGLEOBJECT(btsnoop_msg_ev);
		}
	}

	FREE_CRITICAL(btsnoop_capture_lock);
	btsnoop_capture_lock = (unsigned int)NULL;
	CLOSE_EVENT(btsnoop_msg_ev);
	btsnoop_msg_ev = NULL;
}
#endif

static uint32_t user_btsnoop_size = 1024;
void btsnoop_init(void)
{
	if(get_btsnoop_status() != BTSNOOP_UNINIT) {
		return ;
	}
	set_btsnoop_status(BTSNOOP_INIT);
#if defined(BTSNOOP_EXT_PARSER_INCLUDED) && (BTSNOOP_EXT_PARSER_INCLUDED == TRUE)
	SNOOPDBG("btsnoop_init");

	utils_init();

	/* always setup ext listener port */
	if (pthread_create(&thread_id, NULL,
	                   (void *)ext_parser_thread, NULL) != 0)
		perror("pthread_create");

#endif
	user_btsnoop_size = get_snoop_file_size();
	SNOOPDBG("user_btsnoop_size = %d\n", user_btsnoop_size);
#ifdef BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD
	SNOOPDBG("BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD");
	s_btsnoop_capture_ev = CAPTURE_READY | CAPTURE_IDLE;
	btsnoop_msg_ev = CREATE_EVENT(NULL);
	btsnoop_capture_lock = CREATE_CRITICAL(NULL);
	memset(&s_btsnoop_data_list, 0, sizeof(struct ListStru));
	CREATE_THREAD(btsnoop_capture_loop);
#endif
	
}

void btsnoop_open(char *p_path)
{
#if defined(BTSNOOPDISP_INCLUDED) && (BTSNOOPDISP_INCLUDED == TRUE)
	SNOOPDBG("btsnoop_open");
	btsnoop_log_open(p_path);
#endif // BTSNOOPDISP_INCLUDED
}

void btsnoop_close(void)
{
#if defined(BTSNOOPDISP_INCLUDED) && (BTSNOOPDISP_INCLUDED == TRUE)
	SNOOPDBG("btsnoop_close");
	btsnoop_log_close();
#endif
}

void btsnoop_cleanup(void)
{
#if defined(BTSNOOP_EXT_PARSER_INCLUDED) && (BTSNOOP_EXT_PARSER_INCLUDED == TRUE)
	SNOOPDBG("btsnoop_cleanup");
	pthread_kill(thread_id, SIGUSR2);
	pthread_join(thread_id, NULL);
	ext_parser_detached();
#endif
    if(btsnoop_msg_ev != NULL) {
        set_btsnoop_status(BTSNOOP_UNINIT);
        SET_EVENT(btsnoop_msg_ev);
    }
}


#define HCIT_TYPE_COMMAND   1
#define HCIT_TYPE_ACL_DATA  2
#define HCIT_TYPE_SCO_DATA  3
#define HCIT_TYPE_EVENT     4

//void btsnoop_capture(HC_BT_HDR *p_buf, unsigned char is_rcvd)
void btsnoop_capture(unsigned char *p_buf, uint32_t len, uint8_t is_rcvd)
{
	//unsigned char *p = (uint8_t *)(p_buf + 1) + p_buf->offset;
	unsigned char *p = p_buf;
	unsigned char event = *p_buf;
	uint32_t sent = 0;
	uint32_t l;
	uint32_t file_size;
	char *buffer;
	struct stat stbuf;
	int fd;

#if defined(BTSNOOP_EXT_PARSER_INCLUDED) && (BTSNOOP_EXT_PARSER_INCLUDED == TRUE)
	s_buffer[0] = is_rcvd;
	while (sent < len) {
		l = (len - sent) > 1023 ? 1023 : (len - sent);
		memcpy(s_buffer + 1, p + sent, l);
		send_ext_parser((char *)s_buffer, l + 1);
		sent += l;
	}
	return;
#endif

#if defined(BTSNOOPDISP_INCLUDED) && (BTSNOOPDISP_INCLUDED == TRUE)

	if (hci_btsnoop_fd == -1)
		return;
	
	if ((fstat(hci_btsnoop_fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
		/* Handle error */
		SNOOPDBG("get snoop file size failed(%s)!\r\n", strerror(errno));
		return;
	}

	file_size = stbuf.st_size;
	if ((file_size+len) >= user_btsnoop_size*1024*1024) {
		SNOOPDBG("btsnoop file full!max = %dM\n", user_btsnoop_size);
		return;
	}
#if 0
	switch (p_buf->event & MSG_EVT_MASK) {
	case MSG_HC_TO_STACK_HCI_EVT:
		SNOOPDBG("TYPE : EVT");
		btsnoop_hci_evt(p);
		break;

	case MSG_HC_TO_STACK_HCI_ACL:
	case MSG_STACK_TO_HC_HCI_ACL:
		SNOOPDBG("TYPE : ACL");
		btsnoop_acl_data(p, is_rcvd);
		break;

	case MSG_HC_TO_STACK_HCI_SCO:
	case MSG_STACK_TO_HC_HCI_SCO:
		SNOOPDBG("TYPE : SCO");
		btsnoop_sco_data(p, is_rcvd);
		break;

	case MSG_STACK_TO_HC_HCI_CMD:
		SNOOPDBG("TYPE : CMD");
		btsnoop_hci_cmd(p);
		break;
	}
#else

#ifndef BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD
	p += 1;
	switch (event) {
	case 1: // COMMAND
		btsnoop_hci_cmd(p);
		break;

	case 2: // ACL
		btsnoop_acl_data(p, is_rcvd);
		break;

	case 3: // SCO
		btsnoop_sco_data(p, is_rcvd);
		break;

	case 4: // EVENT
		btsnoop_hci_evt(p);
		break;
	}
	
#else
	struct btsnoop_capture_data_t *node;
	node = (struct btsnoop_capture_data_t *)List_NodeNew(sizeof(struct btsnoop_capture_data_t));
	SNOOPDBG("btsnoop capture data start!");
	ENTER_CRITICAL(btsnoop_capture_lock);
	if (node != NULL) {
		if (s_btsnoop_capture_ev & CAPTURE_READY) {
			SNOOPDBG("btsnoop capture data start! ready!");
			node->p_buf = (unsigned char*)malloc(len - 1);
			memcpy(node->p_buf, p_buf + 1, len - 1);
			//node->p_buf = p_buf;
			node->event = event;
			node->len = len;
			node->is_rcvd = is_rcvd;
			SNOOPDBG("btsnoop capture data [%d,%d,%d]", node->event, node->len, node->is_rcvd);
			List_AddTail(&s_btsnoop_data_list, node);
		
			if (s_btsnoop_capture_ev & CAPTURE_IDLE) {
				SNOOPDBG("btsnoop capture data send!");
				SET_EVENT(btsnoop_msg_ev);
			}
		}
	}
	LEAVE_CRITICAL(btsnoop_capture_lock);

#endif //BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD

	
	
#endif

#endif // BTSNOOPDISP_INCLUDED
}

#ifdef CONFIG_OPEN_FILTER_DATA
void brt_debug_save_avdtp_cid(uint16_t cid, long stream_handle)
{
	int i = 0;
	for (i = 0; i < MAX_AVDTP_CONN_NUM; i++) {
		if (avdtp_cids[i].used == 0) {
			avdtp_cids[i].used = 1;
			avdtp_cids[i].cid = cid;
			avdtp_cids[i].stream_handle = stream_handle;
			return;
		}
	}
}

void brt_debug_clean_avdtp_cid(long stream_handle)
{
	int i = 0;
	for (i = 0; i < MAX_AVDTP_CONN_NUM; i++) {
		if (avdtp_cids[i].used == 1 && avdtp_cids[i].stream_handle == stream_handle) {
			avdtp_cids[i].used = 0;
			avdtp_cids[i].cid = 0;
			avdtp_cids[i].stream_handle = 0;
			return;
		}
	}
}

void brt_debug_cleanup_all_avdtp()
{
	int i = 0;
	for (i = 0; i < MAX_AVDTP_CONN_NUM; i++) {
		avdtp_cids[i].cid = 0;
		avdtp_cids[i].stream_handle = 0;
		avdtp_cids[i].used = 0;
	}
}

int brt_debug_check_avdtp_media(const uint8_t *ori_p)
{
	uint16_t l2cap_cid = 0;
	//unsigned char *dp = ori_p + 7;
	unsigned char *dp = ori_p + 6;
	//l2cap_cid = *dp + (*(dp + 1) << 8);
	l2cap_cid = *dp | (*(dp + 1) << 8);
	//printf("------------------------------dp2 = %d *dp = %d *(dp + 1) << 8 = %d (*(dp2 + 1) << 8) = %d, l2cap_cid = %d *dp2 | (*(dp2 + 1) << 8) = %d",*dp2,  *dp, *(dp + 1) << 8, (*(dp2 + 1) << 8), l2cap_cid, *dp2 | (*(dp2 + 1) << 8));
	int i = 0;
	for (i = 0; i < MAX_AVDTP_CONN_NUM; i++) {
		if (avdtp_cids[i].used == 1 && avdtp_cids[i].cid == l2cap_cid) {
			return 1;
		}
	}

	return 0;
}

static uint8_t s_a2dpsrc_recved_first_packet = 0;
int brt_debug_check_avdtp_src_media(const uint8_t *ori_p)
{
	uint16_t l2cap_cid = 0;
	unsigned char *dp = ori_p + 7;//for tl handle
	l2cap_cid = *dp + (*(dp + 1) << 8);

	uint8_t *pb_bound_byte = ori_p + 2;
	uint8_t pb_boundary_flag = (uint8_t)(((*pb_bound_byte) >> 4) & 0x03);

	uint8_t is_first_a2dpsrc_media_packet = 0;

	if (pb_boundary_flag == 0x02) {//first a2dp media

		//check is the taget a2dp src remote_cid
		int i = 0;
		for (i = 0; i < MAX_AVDTP_CONN_NUM; i++) {
			if (avdtp_cids[i].used == 1 && avdtp_cids[i].cid == l2cap_cid) {
				is_first_a2dpsrc_media_packet = 1;
				break;
			}
		}

		if (is_first_a2dpsrc_media_packet == 1) {
			s_a2dpsrc_recved_first_packet = 1;
		} else {
			s_a2dpsrc_recved_first_packet = 0;
		}
	} else {//continuous packet or other
		if (s_a2dpsrc_recved_first_packet == 1) {
			return 1;
		}
	}

	return is_first_a2dpsrc_media_packet;
}


static uint16_t s_pbap_tl_hdl = 0;
static uint8_t s_pbap_over_l2cap = 0;
static uint8_t s_pbap_recved_first_packet = 0;
void brt_debug_save_pbap_tl_hdl(uint16_t tl_handle, uint8_t over_l2cap)
{
	s_pbap_tl_hdl = tl_handle;
	s_pbap_over_l2cap = over_l2cap;
}

void brt_debug_clean_pbap_tl_hdl()
{
	s_pbap_tl_hdl = 0;
	s_pbap_over_l2cap = 0;
}

int brt_debug_check_pbap_tl_hdl(const uint8_t *p)
{
	if (s_pbap_over_l2cap == 1) {
		uint16_t l2cap_cid = 0;
		unsigned char *dp = p + 7;//for tl handle
		l2cap_cid = *dp + (*(dp + 1) << 8);

		uint8_t *pb_bound_byte = p + 2;
		uint8_t pb_boundary_flag = (uint8_t)(((*pb_bound_byte) >> 4) & 0x03);

		if (pb_boundary_flag == 0x02) {//first packet
			if (l2cap_cid == s_pbap_tl_hdl) {//first pbap packet
				s_pbap_recved_first_packet = 1;//should have continuous data
				return 1;
			} else {//not first pbap packet
				s_pbap_recved_first_packet = 0;
			}
		} else {//continuous packet or other
			//have recved first pbap, wait for the follow, we assume they are linked
			if (s_pbap_recved_first_packet == 1) {				
				return 1;
			}
		}	
	}

	return 0;
}
#endif

