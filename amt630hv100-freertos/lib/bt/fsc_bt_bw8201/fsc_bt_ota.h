
#ifndef __FSC_BT_OTA_H__
#define __FSC_BT_OTA_H__
#if defined __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#if 1 //@martin
#define OTA_UPGRADE_FAILED (0xFF)
#define OTA_UPGRADE_SUCCESS (0x00)
#endif 

#define SYNC_CHAR     'S'

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

enum xmodem_ota_state
{
	xmodem_ota_standby,
	xmodem_ota_switch_mode,
	xmodem_ota_synchronize,
	xmodem_ota_send,
	xmodem_ota_recv,
	xmodem_ota_transfer_complete,
	xmodem_ota_success,
#if 1 //@martin
    xmodem_ota_fail,
#endif 
	xmodem_ota_state_max
};

/***********************************************************************
# implementation by host, call from lib
***********************************************************************/

void fscbt_sleep_ms(uint32_t ms);

typedef void (*fscbt_ota_timer_cb_t) (void);
typedef void (*fscbt_ota_state_cb_t) (uint8_t state,uint8_t percent);

int fscbt_ota_start_timer(int timeout_ms,fscbt_ota_timer_cb_t callback);
int fscbt_ota_stop_timer(void);
int fscbt_ota_check_timer(void);

/*lib request to send size bytes data to uart of bt module*/
int fscbt_serial_write_packet(const char* packet,int size);

/*lib request to change uart baudrate*/
int fscbt_serial_change_baudrate(uint32_t baudrate);

/*lib request to get ota image size*/
uint32_t fscbt_ota_get_image_size();

/*lib request to get ota image data*/
int fscbt_ota_get_image_data(uint8_t *buf, uint32_t offset, int size);

void fscbt_ota_debug_print(const char* Format, ... );

void fscbt_set_ota_callback_func(fscbt_ota_state_cb_t func);
fscbt_ota_state_cb_t fscbt_get_ota_callback_func();


/***********************************************************************
# implementation by lib, call from host
***********************************************************************/

void fscbt1026_ota_start_request(uint32_t image_size,uint32_t ota_baudrate,fscbt_ota_state_cb_t cb);
int fscbt1026_ota_handle_uart_data(uint8_t *data,int size);

void fscbt936_ota_start_request(uint32_t image_size,uint32_t ota_baudrate, uint8_t is_resume);
int fscbt936_ota_handle_uart_data(uint8_t *data,int size);

void fscbt_ota_reset_request();
uint8_t is_ota_reply_characters(const char ch);


/***********************************************************************/

#if defined __cplusplus
}
#endif
#endif

