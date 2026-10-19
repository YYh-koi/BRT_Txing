/**
  * Connection management for LE connection.
 */

#ifndef _HLP_GATT_CONN_H__
#define _HLP_GATT_CONN_H__

#include "btdatatype.h"
#include "bt_gatt_types.h"
#include "att_ui.h"

#define INVALID_HANDLE (-1)
#define MAX_BLE_CONNECTION_COUNT (10)
#define INVALID_CONNECTION_ID (-1)

#define CLIENT_IF_TO_APP(x) ((x) + BTGATT_MAX_SERVER_APP_NUM)
#define CLIENT_IF_TO_LOWER_LAYER(x) ((x) - BTGATT_MAX_SERVER_APP_NUM)

typedef enum {
    CONNECTION_STATE_DISCONNECTED = 0,
    CONNECTION_STATE_CONNECTING,     /* not used */
    CONNECTION_STATE_CONNECTED,      //connected means gatt database has been cached.
} connection_state_type;

typedef struct {
	//lock_t lock;
	struct HCI_AddressStru dev_addr;
	/* 0: disconnected, 1: connecting, 2: connected */
	connection_state_type conn_state;
	unsigned char/*bool*/ is_direct;
	int conn_id; /* equal to acl handle. */
	int client_if; /* invalid when connection state is disconnected. */
	struct GATT_TL_ConnectStru  gatt_tl;
	struct ListStru service_list; /* node is struct GATT_ServiceStru service; */
	struct GATT_ServiceStru *cur_service;
	struct GATT_CharacteristicStru *cur_character;
	struct GATT_DescriptorStru *cur_desc;
	/* a valid handle on which a read request has been sent, but response has not been received. */
	UINT16 handle_in_reading;
	UINT16 mtu;
} app_ble_conn_info_t;

void conn_info_init(void);
/* Callback definition which to operate on each connection. */
typedef void (*gatt_conn_iterator_cbk)(app_ble_conn_info_t *conn);
void gatt_conn_iterator_by_client_if(UINT8 client_if, gatt_conn_iterator_cbk cbk);

app_ble_conn_info_t *conn_info_get_by_conn_id(int conn_id);
app_ble_conn_info_t *conn_info_get_by_addr(bt_bdaddr_t *rmt_addr);
app_ble_conn_info_t *conn_info_add(int client_if, struct HCI_AddressStru* address, UINT8 is_direct);
int conn_info_is_exist(struct HCI_AddressStru* address);
app_ble_conn_info_t *conn_info_on_connected(int client_if, struct HCI_AddressStru* address, UINT16 acl_hdl);
void conn_info_on_disconnected(struct HCI_AddressStru* address, UINT16 acl_hdl);
int gatt_db_add_service_by_conn_id(UINT16 conn_id, struct GATT_ServiceStru *service);
int gatt_db_add_include_service_by_conn_id(UINT16 conn_id, struct GATT_IncludeStru *include);
struct GATT_IncludeStru *hlp_gatt_find_next_incl_service(UINT16 conn_id, btgatt_srvc_id_t *srvc_id, btgatt_srvc_id_t *start_incl_srvc_id);
struct GATT_CharacteristicStru * hlp_gatt_find_next_char(UINT16 conn_id, btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *start_char_id);
int gatt_db_add_descriptor_by_conn_id(UINT16 conn_id, struct GATT_DescriptorStru *desc);
struct GATT_DescriptorStru *hlp_gatt_find_next_descriptor(UINT16 conn_id, btgatt_gatt_id_t *char_id, btgatt_gatt_id_t *start_descr_id);
void * gatt_db_add_char_by_conn_id(UINT16 conn_id, struct GATT_CharacteristicStru *c);
void *hlp_gatt_item_get_by_hdl(app_ble_conn_info_t *conn_info, UINT8 *type, UINT16 hdl);
struct GATT_ServiceStru * hlp_gatt_find_service_by_char(struct ListStru service_list, struct GATT_CharacteristicStru *characteristic);
struct GATT_ServiceStru * hlp_gatt_find_srv_char_by_desc_hdl(UINT16 desc_hdl, struct ListStru service_list, struct GATT_CharacteristicStru **characteristic);
struct GATT_ServiceStru * hlp_gatt_find_service_by_hdl(UINT16 conn_id, UINT16 hdl);

#endif  /*_HLP_GATT_CONN_H__*/

