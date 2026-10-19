/*
**     Filename  : fsc_bw8202.h
**     Project   : FSC Blueware
**     Date      : 2022/12/31
**     Abstract  :
**         This is bw8202 module.
**
**     Copyright : 2022 Feasycom Co., Ltd. All Rights Reserved.
**
**     mail      : support@feasycom.com
*/

/* MODULE bw8202. */
#ifndef _FSC_BW8202_H
#define _FSC_BW8202_H

#ifdef __cplusplus
extern "C" {
#endif

#include "chip.h"
#include "board.h"
#include "config.h"
#include "fscbt_interface.h"
#include "bt_api.h"

#define UART_BT_MSG_TAG1      0x0D
#define UART_BT_MSG_TAG2      0x0A

#define UART_BT_MSG_MAX_LEN   512 //@martin 256=>1030  for OTA upgrading.
#define UART_BT_DATA_MAX_LEN  4096
#define UART_BT_MSG_MIN_LEN   0x04
#define UART_BT_QUEUE_LEN     0x0A

#define UART_BW8202_PORT     	UART_ID1
#define UART_BT_BAUD        	115200
#define UART_OTA_BAUD           921600 //@martin

#define debug_printf(fmt, args...)  //do {printf(fmt, ##args);} while(0) //printf
#define error_printf(fmt, args...)  do {printf(fmt, ##args);} while(0)  
#define info_printf(fmt, args...)  //do {printf(fmt, ##args);} while(0)  


#define UART_BT_PARAM_LEN       30
#define UART_BT_PARAM_NUMBER    5

#define BT_SCAN_DEVICE_COUNT    10
#define BW8202_RESET_PIN        24
#define TEXT_PARAM_LEN      30


typedef struct{
	DashBTHfpState_t hfp_state;
	DashBTA2dpState_t a2dp_state;
	DashBTAvrcpState_t avrcp_state;
}theBT_t;

typedef struct {
    char *buffer;//Command string.
    char *data;//
//  uint8_t len;  @martin
    uint32_t len;
    uint8_t repeat;
    uint8_t is_binary;
} FSC_BW8202_UART_MSG_T;//Whether it is binary data ,Exsample: "COVERDATA",	"SPPDATA", "GATTDATA"

FSC_BW8202_UART_MSG_T *bw8202_msg_malloc(uint32_t len);
void bw8202_msg_free(FSC_BW8202_UART_MSG_T *msg);

int fsc_bt_bw8202_init(uint32_t uart_id,uint32_t baud,void (*Callback)(DashBTEvent_t, void *));
int fsc_bw8202_msg_send(const char *buff);
int fsc_bw8202_at_cmd_send(const char *buff, int size);
int fsc_bw8202_at_cmd_send_bytes(const char *buff, int len);
void fsc_bt_init(void);
int bt_interface_send(const char* cAtCmd, unsigned short length);
void fsc_bw8202_reset_baud(uint32_t baud);

#ifdef __cplusplus
}
#endif

#endif //_FSC_BW8202_H
