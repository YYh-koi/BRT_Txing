/**
**     Filename  : fsc_bw8202.c
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

/** ================================================================ **/

/* #includes */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "FreeRTOS.h"
#include "chip.h"
#include "sysinfo.h"
#include "uart.h"
#include "fsc_bw8202.h"
#include "fsc_bw8202_cmd.h"
#include "fsc_bt_ota.h"
#include "BW8201_tianyouwei_20250227_BL_100_APP_121_Update.h"

#define printf_func   printf

static uint32  ota_image_size = 0; 

/*lib request to send size bytes data to uart of bt module*/
int fscbt_serial_write_packet(const char* packet, int size){
    //return fsc_bw8202_at_cmd_send_bytes(packet, size);
    return fsc_bw8202_at_cmd_send(packet, size);
}

/*lib request to change uart baudrate*/
int fscbt_serial_change_baudrate(uint32_t baudrate){
	fsc_bw8202_reset_baud(baudrate);
	return 1;
}

/*lib request to get ota image size*/
uint32_t fscbt_ota_get_image_size(){
#if OTA_TEST
    return sizeof(BW8201_tianyouwei_20250227_BL_100_APP_121_Update);
#else
	return 0;//realize by user
#endif
}

/*lib request to get ota image data*/
int fscbt_ota_get_image_data(uint8_t *buf, uint32_t offset, int size){

	if(size < 1){
		printf_func("Error! fscbt_ota_get_image_data - Invalid size(%d).\n", size);
		return (-1);
	}

    if( (offset + size) > fscbt_ota_get_image_size() ){
		printf_func("Error! fscbt_ota_get_image_data - Over flow.\n");
		return (-1);
	}

	printf("image - offset: %d  size: %d\n", offset, size);
#if OTA_TEST//realize by user
	memcpy(buf, (const char *)BW8201_tianyouwei_20250227_BL_100_APP_121_Update + offset, size);
#endif
	//memcpy(buf, (const char *)upgrade_buff + offset, size);
    return 0;
}

void fscbt_ota_state_cb_fun(uint8_t state, uint8_t percent){
#ifdef OTA_UPGRADE_TEST
    printf("fscbt_ota_state_cb_fun: state: %d   percent: %d\n", state, percent);
#endif 
	if(state == xmodem_ota_fail){
		fsc_set_ota_cmd_reply_route(NULL);

		if(UART_OTA_BAUD != UART_BT_BAUD){
		    fsc_bw8202_reset_baud(UART_BT_BAUD);
		}

		printf("Ota upgrading is failed.\n");
	}else if(state == xmodem_ota_success){
		fsc_set_ota_cmd_reply_route(NULL);

		if(UART_OTA_BAUD != UART_BT_BAUD){
		    fsc_bw8202_reset_baud(UART_BT_BAUD);
		}

        printf("Ota upgrading is successful.\n");
	}else {
        printf_func("Upgrading... state: %u  percent: %u.\n", state, percent);
	}
}

void ota_upgrade_demo() {
	//uint32 image_size = 0;
	debug_printf(">>>>>>>>>>>>Enter into ota_upgrade_demo\n");

	ota_image_size = fscbt_ota_get_image_size();

    fscbt_set_ota_callback_func(fscbt_ota_state_cb_fun);
	//first ,stop service.
	fsc_set_ota_cmd_reply_route(fscbt936_ota_handle_uart_data);
    fscbt936_ota_start_request(ota_image_size, UART_BT_BAUD, false);
}








