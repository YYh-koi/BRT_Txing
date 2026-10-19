#include <string.h>
#include "internal_gatt_device.h"
#include "internal_gatt_conn.h"
#include "sysdep.h"
#include "btlist.h"
#include "hci_le.h"
#include "auto_gatt_server.h"

#define MODULE_TAG "GATTS_DEV:"
#include "brt_trace.h"

#define LOG(fmt, ...) bse_log_print(MODULE_TAG, "\r\n", fmt, ## __VA_ARGS__)

//XXX  fsm_msg_gatt_client_disconnect_t fsm_msg_gatt_server_disconnect_t
//those two struct above are the same, and the same as the following struct
typedef struct {
	uint8_t server_if;
	bt_bdaddr_t address;
	uint16_t conn_id;
} fsm_msg_gatt_disconnection_t;

static struct ListStru s_gatt_devices;

void gatt_device_init()
{
	s_gatt_devices.head = NULL;
	s_gatt_devices.tail = NULL;
}

void gatt_device_done()
{
	List_RemoveAll(&s_gatt_devices);
}

void gatt_device_add(bt_bdaddr_t *addr, UINT8 addr_type, UINT8 auto_connect, UINT8 app_if)
{
	gatt_device_t *device = NULL;

	if (addr == NULL) {
		return;
	}
        LOG("%s.", __FUNCTION__);
	if ((device = gatt_device_find(addr)) != NULL) {
		LOG("found same device(%p) in %s.", device, __FUNCTION__);
		if (auto_connect != 0xFF) {
			device->auto_connect = auto_connect;
		}

		if (app_if != BTGATT_APP_IF_INVALID) {
			device->app_if = app_if;
		}
	} else {
	        LOG("%s,add.", __FUNCTION__);
		List_AddTail(&s_gatt_devices, (device = List_NodeNew(sizeof(gatt_device_t))));
		memset(device, 0, sizeof(gatt_device_t));
		// TODO device->app_if = BTGATT_APP_IF_INVALID;
		device->app_if = app_if;
		device->addr = *addr;
		device->addr_type = addr_type;
		device->auto_connect = auto_connect;
		device->conn_state = CONNECTION_STATE_DISCONNECTED;
	}
}

void gatt_device_delete(bt_bdaddr_t *addr)
{
	gatt_device_t *device = gatt_device_find(addr);

	if (device) {
		List_RemoveAt(&s_gatt_devices, device);
		LFREE(device);
	}
}

gatt_device_t *gatt_device_find(bt_bdaddr_t *addr)
{
	gatt_device_t *node = s_gatt_devices.head;

	if (addr == NULL) {
		return NULL;
	}

	while (node != NULL) {
		if (memcmp(&node->addr, addr, sizeof(bt_bdaddr_t)) == 0) {
                    LOG("%s.", __FUNCTION__);
			return node;
		}
		node = LNEXT(node);
	}

	return NULL;
}

void gatt_device_on_connected(bt_bdaddr_t *addr, UINT8 addr_type, UINT16 conn_id)
{
	gatt_device_t *device = gatt_device_find(addr);

	LOG("%s", __func__);
	LOG("addr:%02X:%02X:%02X:%02X:%02X:%02X.conn_id:%04X", addr->address[0], addr->address[1], addr->address[2],
	    addr->address[3], addr->address[4], addr->address[5], conn_id);
	if (device == NULL) {
		List_AddTail(&s_gatt_devices, (device = List_NodeNew(sizeof(gatt_device_t))));
		device->app_if = BTGATT_APP_IF_INVALID;
	}

	device->addr_type = addr_type;
	device->addr = *addr;
	device->conn_id = conn_id;
	device->conn_state = CONNECTION_STATE_CONNECTED;
}

void gatt_device_on_disconnected(bt_bdaddr_t *addr)
{
	gatt_device_t *device = gatt_device_find(addr);

	if (device == NULL) {
		return;
	}

	//device->app_if = BTGATT_APP_IF_INVALID;
	device->conn_id = 0;
	device->conn_state = CONNECTION_STATE_DISCONNECTED;
}

static UINT8 is_server_app(UINT8 app_if)
{
	UINT8 is_server_app = 1;
	if (app_if != BTGATT_APP_IF_INVALID) {
		is_server_app = ((app_if <= BTGATT_MAX_SERVER_APP_NUM && app_if > 0)) ? 1 : 0;
	}
	return is_server_app;
}

void gatt_device_disconnect_by_app_if(UINT8 app_if, target_action action)
{
	gatt_device_t *node = s_gatt_devices.head;
	fsm_msg_gatt_disconnection_t param;
	param.server_if = 0;
	param.conn_id = 0;
	
	while (node) {
		if (node->app_if == app_if && node -> conn_state == CONNECTION_STATE_CONNECTED) {
			memcpy(param.address.address, node->addr.address, 6);
			action(&param);
			return;
		}
		node = LNEXT(node);
	}
	LOG("No device is connected of this app_if: %d", app_if);
}

//FIXME the function name may be not appropriate
void gatt_device_iterator_by_action_type(int action_type, target_action action)
{
	LOG("%s type : %d", __func__, action_type);
	gatt_device_t *node = s_gatt_devices.head;
	fsm_msg_gatt_disconnection_t param;
	param.server_if = 0;
	param.conn_id = 0;
	
	while (node != NULL) {
		switch (action_type) {
		case DISCONNECT_ALL_LE:
			if (node->conn_state == CONNECTION_STATE_CONNECTED) {
				memcpy(param.address.address, node->addr.address, 6);
				action(&param);
			}
			break;
		case DISCONNECT_LE_AS_SERVER:
			if (node->conn_state == CONNECTION_STATE_CONNECTED) {
				if (is_server_app(node->app_if)) {
					LOG("Disconnect LE as Server");
					memcpy(param.address.address, node->addr.address, 6);
					action(&param);
				}
			}
			break;
		}
		node = LNEXT(node);
	}
}

gatt_device_t *gatt_device_iterator_to_compare_smp_address(void *addr, UINT8 connected)
{
    gatt_device_t *node = s_gatt_devices.head;
    struct HCI_AddressStru bd;

    struct HCI_AddressStru *hci_bd = (struct HCI_AddressStru *)addr;
   
    UINT8 result = 0xFF; 
    while (node) {
        bd.cap = HCI_ADDRESS_CAP_LE;//FIXME
        bd.atype = node->addr_type;
        memcpy(bd.bd, node->addr.address, BD_ADDR_LEN);

        result = SMP_CompareAddr(hci_bd, &bd);

        if (result == 1) {
            if (!connected && node->app_if != 0xFF) {
                return node;
            }
        }

        node = LNEXT(node);
    }

    return NULL;
}
gatt_device_t * gatt_find_connected_device_and_disc()
{
	gatt_device_t *node = s_gatt_devices.head;

	while (node != NULL) {
		if (node->conn_state == CONNECTION_STATE_CONNECTED) {
                    LOG("%s.", __FUNCTION__);
			app_gatt_server_disconnect(0,node->addr,node->conn_id);
		}
		node = LNEXT(node);
	}
	return node;
}

