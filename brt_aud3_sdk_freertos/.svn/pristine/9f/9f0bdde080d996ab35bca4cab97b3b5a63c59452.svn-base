#ifndef __INTERNAL_GATT_FSM_SERVER_H__
#define __INTERNAL_GATT_FSM_SERVER_H__

#include "bt_gatt.h"

void gatt_fsm_server_init(btgatt_server_callbacks_t *callback);
void gatt_fsm_server_done();

void hlp_gatt_fsm_gatt_server_register_app(void *param);
void hlp_gatt_fsm_gatt_server_unregister_app(void *param);
void hlp_gatt_fsm_gatt_server_connect(void *param);
void hlp_gatt_fsm_gatt_server_disconnect(void *param);
void hlp_gatt_fsm_gatt_server_set_preferred_phy(void *param);
void hlp_gatt_fsm_gatt_server_read_phy(void *param);
void hlp_gatt_fsm_gatt_server_add_service(void *param);
void hlp_gatt_fsm_gatt_server_stop_service(void *param);
void hlp_gatt_fsm_gatt_server_delete_service(void *param);
void hlp_gatt_fsm_gatt_server_send_indication(void *param);
void hlp_gatt_fsm_gatt_server_send_notification(void *param);
void hlp_gatt_fsm_gatt_server_send_response(void *param);
void hlp_gatt_fsm_gatt_client_scan_filter_add(void *param);
void hlp_gatt_fsm_gatt_client_scan_filter_delete(void *param);
void hlp_gatt_fsm_gatt_client_scan_filter_param_add(void *param);
void hlp_gatt_fsm_gatt_client_scan_filter_param_clear_all(void *param);
void hlp_gatt_fsm_gatt_client_scan_filter_param_delete(void *param);
void hlp_gatt_fsm_gatt_client_scan_filter_clear(void *param);
void hlp_gatt_fsm_gatt_client_scan_filter_enable(void *param);
void hlp_gatt_fsm_gatt_client_config_batch_scan_storage(void *param);
void hlp_gatt_fsm_gatt_client_start_batch_scan(void *param);
void hlp_gatt_fsm_gatt_client_stop_batch_scan(void *param);
void hlp_gatt_fsm_gatt_client_read_scan_reports(void *param);

void hlp_gatt_server_on_device_connected(void *bd, uint16_t acl_hdl);
void hlp_gatt_server_on_device_disconnected(void *bd, uint16_t acl_hdl);
void hlp_gatt_server_save_primary_service_to_local_ext(unsigned char server_if, btgatt_db_element_t *element);
void hlp_gatt_server_save_characteristic_to_local_ext(unsigned char server_if, btgatt_db_element_t *element);
void hlp_gatt_server_save_descriptor_to_local_ext(unsigned char server_if, btgatt_db_element_t *element);
void hlp_gatt_add_all_Service(unsigned char server_if);
void hlp_gatt_fsm_gatt_server_disconnect_ext(char *param);
void hlp_gatt_fsm_gatt_server_send_response_ext(uint16_t handle,uint16_t trans_id,uint16_t value_len,uint8_t *val);
void hlp_gatt_fsm_gatt_server_send_notification_ext(uint16_t handle,uint16_t value_len,uint8_t *val);

#endif /* __INTERNAL_GATT_FSM_SERVER_H__ */
