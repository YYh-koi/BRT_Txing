/**
  * This file defines APIs to save and lookup the cached LE devices.
  */

#ifndef _HLP_GATT_DEVICE_H__
#define _HLP_GATT_DEVICE_H__

#include "btdatatype.h"
#include "bt_gatt_types.h"
//action type
enum {
    DISCONNECT_ALL_LE,
    DISCONNECT_LE_AS_CLIENT,
    DISCONNECT_LE_AS_SERVER,
};

typedef struct tag_gatt_device {
	bt_bdaddr_t addr;
	UINT8       addr_type;
	UINT8       conn_state;
	UINT16      conn_id;
	UINT8       auto_connect; /* Whether connect directly or delay connect when device is in range. */
	UINT8       app_if;
} gatt_device_t;

typedef void (*target_action)(void *param);
#define HCI_OWN_ADDRESS_TYPE_UNKNOWN    0xFF

void gatt_device_init();
void gatt_device_done();
void gatt_device_add(bt_bdaddr_t *addr, UINT8 addr_type, UINT8 auto_connect, UINT8 app_if);
void gatt_device_delete(bt_bdaddr_t *addr);
gatt_device_t *gatt_device_find(bt_bdaddr_t *addr);
void gatt_device_on_connected(bt_bdaddr_t *addr, UINT8 addr_type, UINT16 conn_id);
void gatt_device_on_disconnected(bt_bdaddr_t *addr);
void gatt_device_iterator_by_action_type(int action_type, target_action action);
void gatt_device_disconnect_by_app_if(UINT8 app_if, target_action action);
void hlp_gatt_support_BLE_Authentication(UINT8 ev);
gatt_device_t *gatt_device_iterator_to_compare_smp_address(void * hci_bd, UINT8 connected);
gatt_device_t * gatt_find_connected_device_and_disc();
#endif  /*_HLP_GATT_DEVICE_H__*/
