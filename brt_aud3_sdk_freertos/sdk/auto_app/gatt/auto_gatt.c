#include "gap_ui.h"
#include "auto_gatt.h"
#include "auto_gatt_event.h"
#include "auto_gatt_fsm.h"
#include "auto_gatt_client.h"
#include "auto_gatt_server.h"

#include "bt_gatt.h"
#include "bt_utils.h"
#include "internal_gatt_device.h"
#include "bt_event.h"
#define TAG "auto_gatt"
#define LOG(fmt, ...) bse_log_print(TAG, "", fmt, ##__VA_ARGS__)

static tGAP_IndCbk_Result gatt_connection_cbk(HANDLE tl_handle, void *context,
        tGAP_Ind_Type ind_type, void *par, UINT32 size);

void app_gatt_init(module_event_cbk_t callback)
{
	LOG("app_gatt_init");
	btgatt_callbacks_t*  gatt_cbks;

	GAP_RegisterIndCbk(NULL, GAP_IND_CONNECTION_EVENT, gatt_connection_cbk);
	gatt_cbks = app_gatt_event_init(callback);
	gatt_fsm_init(gatt_cbks);
       gatt_device_init();
	app_gatt_client_init();
	app_gatt_server_init();
}

void app_gatt_done()
{
	app_gatt_event_done();
	app_gatt_client_done();
	app_gatt_server_done();
	gatt_fsm_done();
	GAP_UnregisterIndCbk(NULL, gatt_connection_cbk);
}
int app_ble_cleanup()
{
    if(gatt_find_connected_device_and_disc()==NULL)
		return 0;
	else
		return 1;
}
static tGAP_IndCbk_Result gatt_connection_cbk(HANDLE tl_handle, void *context,
        tGAP_Ind_Type ind_type, void *par, UINT32 size)
{
	LOG("%s", __func__);
	struct GAP_ConnectionEventStru *conn_ev = (struct GAP_ConnectionEventStru *)par;
	gatt_device_t *device = NULL;
	bt_bdaddr_t bdaddr;
	UINT8 is_server_app = 1;

	if((GAP_IND_CONNECTION_EVENT != ind_type) ||
	   (conn_ev == NULL) ||
	   (GAP_LINK_TYPE_LE != conn_ev->link_type)) {
		return GAP_IND_RESULT_NOT_CARE;
	}

	/* Check current connection is created by client or server */
	HCI_ADDR_STRU_TO_BLUEDROID_BD(&(conn_ev->addr), &bdaddr);
    LOG("addr = [%02X:%02X:%02X:%02X:%02X:%02X], connected(1-yes, 0-no) ? %d", bdaddr.address[5], bdaddr.address[4],bdaddr.address[3],bdaddr.address[2],bdaddr.address[1],bdaddr.address[0], conn_ev->connected);
	device = gatt_device_find(&bdaddr);

    //This happens when pairing is required.
    //FIXME fcx Connection state changed.But the device cannot be found because the address has changed or the deivce is a Peripheral.
    if (device == NULL) {
        LOG(" +++ Connection state changed, but cannot find the device in deviceList");
        if (!conn_ev->connected) {
            LOG("[Error]. Disconnected. Which means Address has changed!!!");
        
           //FIXME If is connceted. How to distinguish gatt Central and Peripheral.
            device =  gatt_device_iterator_to_compare_smp_address(&conn_ev->addr, conn_ev->connected);
            
            if (device == NULL) {
                LOG("SMP_CompareAddr still found nothing");
            } else {
                LOG("SMP_CompareAddr found -> addr[%02X:%02X:%02X:%02X:%02X:%02X]", 
                        device->addr.address[5], device->addr.address[4], device->addr.address[3],
                        device->addr.address[2], device->addr.address[1], device->addr.address[0]);
                //FIXME fcx change struct HCI_AddressStru, is this good.
                memcpy(conn_ev->addr.bd, device->addr.address, BD_ADDR_LEN);
                memcpy(bdaddr.address, device->addr.address, BD_ADDR_LEN);
            }
        } else {
            LOG("[Warning]. Connected. The device is server or SMP address changed");
        }

    }

	if (device) {
        LOG("app_if = %d", device->app_if);
		if (device->app_if != BTGATT_APP_IF_INVALID) {
			is_server_app = ((device->app_if <= BTGATT_MAX_SERVER_APP_NUM && device->app_if > 0)) ? 1 : 0;
		} else {
            //FIXME added by fcx. Rarely reach here. Currently GATT_SERVER`s app_if will not be 0XFF.
            LOG("[Warning], app_if is INVALID, it means local is CLIENT ?");
            is_server_app = 0;
		}
	}
	
	LOG("device_is_server(1-yes, 0-no) ? : %d", is_server_app);

	if(conn_ev->connected) {
		if (is_server_app) {
			app_gatt_server_on_device_connected(&conn_ev->addr, conn_ev->acl_hdl);
		} else {
			app_gatt_client_on_device_connected(&conn_ev->addr, conn_ev->acl_hdl);
		}

		gatt_device_on_connected(&bdaddr, conn_ev->addr.atype, conn_ev->acl_hdl);
	} else {
		if (is_server_app) {
			app_gatt_server_on_device_disconnected(&conn_ev->addr, conn_ev->acl_hdl);
		} else {
			app_gatt_client_on_device_disconnected(&conn_ev->addr, conn_ev->acl_hdl);
		}

		gatt_device_on_disconnected(&bdaddr);
	}

	return GAP_IND_RESULT_NOT_CARE;
}

profile_t g_gatt_profile =
{
    app_gatt_init,
    app_gatt_done,
    NULL,
    NULL,
};
