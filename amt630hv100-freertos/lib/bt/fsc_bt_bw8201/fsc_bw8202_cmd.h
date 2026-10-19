/*
**     Filename  : fsc_bw8202_cmd.h
**     Project   : FSC Blueware
**     Date      : 2022/03/31
**     Abstract  :
**         This is bw8202 at cmd module.
**
**     Copyright : 2022 Feasycom Co., Ltd. All Rights Reserved.
**
**     mail      : support@feasycom.com
*/

/* MODULE bw8202. */
#ifndef FSC_BW8202_CMD_H
#define FSC_BW8202_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fsc_bw8202.h"
#include "fsc_bt_ota.h"

typedef int (* ota_handle_uart_data_func_t)(uint8_t *data,int size);

bool fsc_bw8202_cmd_init();
int fsc_bw8202_cmd_add(FSC_BW8202_UART_MSG_T * msg);
void fsc_bw8202_cmd_process(char *msg, int len);
#if 1//@martin
void fsc_set_ota_cmd_reply_route(ota_handle_uart_data_func_t func);
#endif

#ifdef __cplusplus
}
#endif

#endif //FSC_BW8202_CMD_H