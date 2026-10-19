/*************************************************************************
    > File Name: auto_app/dun/dun_thread.c
    >
    >
    > Created Time: Mon 24 Aug 2015 07:44:05 PM CST
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <cutils/sockets.h>

#include "btdatatype.h"
#include "global.h"
#include "brt_trace.h"
#define LOG(fmt, ...) bse_log_info("dun_thread", fmt,  ## __VA_ARGS__)

#define BUFF_SIZE 1024
static void *receive_message_from_duntty(void *arg);
static void dun_send_data(unsigned char *data, int length);
static int dun_buffer_check(unsigned char *buffer, int length);
void print_hex(unsigned char *data, int length);


typedef struct tag_context {
	pthread_t schedule_thread_id;
	int schedule_running;
	/* dun_tty & state */
	int dun_tty;
	int dun_tty_running;
	pthread_mutex_t dun_tty_lock;
	pthread_t dun_tty_thread_id;
	unsigned char buffer[BUFF_SIZE];
	char event_cache[BUFF_SIZE];
	int offset;
	pthread_mutex_t write_lock;
} dun_context_t;

static dun_context_t s_context;

void fork_dun_recive_thread(int dun_tty)
{
	pthread_mutex_lock(&s_context.dun_tty_lock);
	s_context.dun_tty = dun_tty;
	s_context.dun_tty_running = 1;
	s_context.offset = 0;
	pthread_mutex_unlock(&s_context.dun_tty_lock);

//	memset(s_context.buffer, 0, sizeof(s_context.buffer));

	pthread_create(&s_context.dun_tty_thread_id, NULL, receive_message_from_duntty, (void *)dun_tty);
	pthread_detach(s_context.dun_tty_thread_id);
}

void interrupt_dun_recive_thread()
{
	s_context.dun_tty_running = 0;
}

static void *receive_message_from_duntty(void *arg)
{
	int rc;
	int offset;
	int len;
	int dun_tty;

	dun_tty = (int) arg;

	if (dun_tty < 0) {
		return 0;
	}
	while (s_context.dun_tty_running) {
		if (s_context.offset >= BUFF_SIZE) {
			/* should Not reach here! */
			break;
		}
		rc = read(dun_tty, s_context.buffer + s_context.offset, BUFF_SIZE - s_context.offset);
		if(rc > 0) {
			offset = rc;//s_context.offset;
			s_context.offset += rc;
			do {
				{
					dun_send_data(s_context.buffer, offset);
				}
				s_context.offset -= offset;
				offset = 0;

			} while(offset > 0);

		} else {
			s_context.dun_tty_running = 0;
		}

	}
	return 0;
}

static int dun_buffer_check(unsigned char *buffer, int length)
{
	unsigned char *occur;

	if (!buffer || length <= 0) {
		return 0;
	}

	if ((occur = (unsigned char *)strstr((const char *) buffer, "AT#")) != NULL) {
		occur = buffer + 3; // length of "AT#"

		for (; occur < buffer + length - 1; occur++) {
			if (*occur == '\r' && *(occur  + 1) == '\n') {
				return occur + 2 - buffer;
			}
		}
	}

	return 0;
}
#if 0
struct BuffStru {
	WORD len; /* length of whole buffer, including data before off */
	WORD off;
	UCHAR data[1];
};
#endif
static UCHAR s_ppp_buf[256] = {0};
static int s_ppp_len = 0;
static BOOL check_buf_tail(unsigned char *data)
{
	if(*(data) == 0x0d) {
		return 1;
	}
	return 0;
}
static SYSTEM_LOCK s_dun_lock;
static int p_buf_count = 0;
static int p_buf_offset = 0;
struct BuffStru* p_buf[100];

static void dun_send_data(unsigned char *data, int length)
{
	if (data && length > 0) {
		if(length != 1) {
			struct BuffStru *buf = (struct BuffStru *)malloc(sizeof(struct BuffStru) + length + 1);
			memset(buf, 0, sizeof(struct BuffStru) + length + 1);
			//		char *p = (char *) malloc(length + 1);
			if (data) {
				buf->len = length;
				buf->off = 0;
				memcpy(buf->data, data, length);
				print_hex(data, length);
				DUN_TL_DataSend(buf);
				free(buf);
			}
		} else {
			ENTER_CRITICAL(s_dun_lock);
			if(check_buf_tail(data)) {
				usleep(100);
				s_ppp_buf[s_ppp_len] = (unsigned char)(*data);
				s_ppp_len ++;

				struct BuffStru *buf = (struct BuffStru *)malloc(sizeof(struct BuffStru) + s_ppp_len + 1);
				memset(buf, 0, sizeof(struct BuffStru) + s_ppp_len + 1);
				buf->len = s_ppp_len;
				buf->off = 0;
				memcpy(buf->data, s_ppp_buf, s_ppp_len);
				print_hex(s_ppp_buf, 256);//s_ppp_len);
				DUN_TL_DataSend(buf);
				memset(s_ppp_buf, 0, 256);
				free(buf);
				s_ppp_len = 0;
			} else {
				s_ppp_buf[s_ppp_len] = (unsigned char)(*data);
				s_ppp_len ++;
			}
			ENTER_CRITICAL(s_dun_lock);
		}
	}
}

void print_hex(unsigned char *data, int len)
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
		LOG("%s", (const char *) tmp_str);     //add (const char*)
	}
}
