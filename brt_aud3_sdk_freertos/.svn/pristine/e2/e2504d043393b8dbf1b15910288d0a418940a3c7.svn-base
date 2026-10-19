#ifndef __HLP_LE_ADVERTISER_FSM_H__
#define __HLP_LE_ADVERTISER_FSM_H__

#include "bt_gatt_server.h"

void hlp_le_advertiser_fsm_init(btgatt_server_callbacks_t *callback);
void hlp_le_advertiser_fsm_done();
void hlp_gatt_fsm_gatt_advm_start_advertising_set(void *param);
void hlp_gatt_fsm_gatt_advm_getownaddress(void *param);
void hlp_gatt_fsm_gatt_advm_stopadvertisingset(void *param);
void hlp_gatt_fsm_gatt_advm_enableadvertisingset(void *param);
void hlp_gatt_fsm_gatt_advm_set_advertising_data(void *param);
void hlp_gatt_fsm_gatt_advm_set_scan_response_data(void *param);
void hlp_gatt_fsm_gatt_advm_set_advertising_parameters(void *param);
void hlp_gatt_fsm_gatt_advm_set_periodic_advertising_parameters(void *param);
void hlp_gatt_fsm_gatt_advm_set_periodic_advertising_data(void *param);
void hlp_gatt_fsm_gatt_advm_set_periodic_advertising_enable(void *param);
void hlp_gatt_server_set_adv_data(short adv_data_len, uint8_t *adv_data, int reg_id);
void hlp_gatt_server_set_scan_response_data(short scan_resp_data_len, uint8_t *scan_resp_data, int reg_id);
void hlp_gatt_server_start_advertising_ex(int reg_id);

void hlp_le_start_adv(char* advData,int advLen);
void hlp_le_start_adv_ex(char* advData,int advLen);
void hlp_le_start_adv_scan_response_ex(char* advData,int advLen);
void hlp_le_stop_adv(void);

#endif
/* __HLP_LE_ADVERTISER_FSM_H__ */
