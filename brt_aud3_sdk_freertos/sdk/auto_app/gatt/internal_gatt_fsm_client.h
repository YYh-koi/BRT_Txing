#ifndef __INTERNAL_GATT_FSM_CLIENT_H__
#define __INTERNAL_GATT_FSM_CLIENT_H__

#include "bt_gatt.h"
#include "hci_ui.h"

void gatt_fsm_client_init(btgatt_client_callbacks_t *callback);
void gatt_fsm_client_done();

void hlp_gatt_fsm_gatt_test(void *param);
void hlp_gatt_fsm_gatt_client_get_device_type(void *param);
void hlp_gatt_fsm_gatt_client_register_app(void *param);
void hlp_gatt_fsm_gatt_client_unregister_app(void *param);
void hlp_gatt_fsm_gatt_client_connect(void *param);
void hlp_gatt_fsm_gatt_client_disconnect(void *param);
void hlp_gatt_fsm_gatt_client_set_preferred_phy(void *param);
void hlp_gatt_fsm_gatt_client_read_phy(void *param);
void hlp_gatt_fsm_gatt_client_refresh(void *param);
void hlp_gatt_fsm_gatt_client_search_service(void *param);
void hlp_gatt_fsm_gatt_client_discover_service_by_uuid(void *param);
void hlp_gatt_fsm_gatt_client_get_gatt_db(void *param);
void hlp_gatt_fsm_gatt_client_read_characteristic(void *param);
void hlp_gatt_fsm_gatt_client_read_using_characteristic_uuid(void *param);
void hlp_gatt_fsm_gatt_client_read_descriptor(void *param);
void hlp_gatt_fsm_gatt_client_write_characteristic(void *param);
void hlp_gatt_fsm_gatt_client_write_descriptor(void *param);
void hlp_gatt_fsm_gatt_client_execute_write(void *param);
void hlp_gatt_fsm_gatt_client_register_for_notifications(void *param);
void hlp_gatt_fsm_gatt_client_read_remote_rssi(void *param);
void hlp_gatt_fsm_gatt_client_configure_mtu(void *param);
void hlp_gatt_fsm_gatt_connection_parameter_update(void *param);

void hlp_gatt_fsm_client_on_device_connected(struct HCI_AddressStru *addr, UINT16 acl_hdl);
void hlp_gatt_fsm_client_on_device_disconnected(struct HCI_AddressStru *addr, UINT16 acl_hdl);
#endif /* __INTERNAL_GATT_FSM_CLIENT_H__ */
