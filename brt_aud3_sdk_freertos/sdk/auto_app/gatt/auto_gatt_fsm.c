#include "btdatatype.h"
#include "fsm.h"
#include "auto_gatt_fsm.h"
#include "auto_gatt.h"
#include "internal_gatt_fsm_client.h"
#include "internal_gatt_fsm_server.h"
#include "le/hlp_le_scanner_fsm.h"
#include "le/hlp_le_advertiser_fsm.h"
#include "brt_trace.h"

//#undef FsmEventCx
//#define FsmEventCx(fi_, ev_, func_, arg_) FsmEventFunc_External(fi_, func_, FSMFree, arg_, (ev_))

#define LOG(fmt, ...) bse_log_print("GATTC_FSM", "", fmt, ## __VA_ARGS__)

static struct FsmInst *g_app_gatt_fsminst = NULL;
static void hlp_gatt_fsm_proc(struct FsmInst *fi, unsigned char event, void *arg);

void gatt_fsm_init(btgatt_callbacks_t *callbacks)
{
	g_app_gatt_fsminst = FsmInstNew(0, "AppGattFsm");
	gatt_fsm_client_init((btgatt_client_callbacks_t *)callbacks->client);
	gatt_fsm_server_init((btgatt_server_callbacks_t *)callbacks->server);
	hlp_le_scanner_fsm_init((btgatt_client_callbacks_t *)callbacks->client); /* XXX */
	hlp_le_advertiser_fsm_init((btgatt_server_callbacks_t *)callbacks->server); /* XXX */
}

int gatt_fsm_done()
{
	int i = 0;
	int conn_cnt = 0;

	gatt_fsm_client_done();
	gatt_fsm_server_done();
	hlp_le_scanner_fsm_done();
	hlp_le_advertiser_fsm_done();

	if (g_app_gatt_fsminst) {
		FsmInstFree(g_app_gatt_fsminst);
		g_app_gatt_fsminst = NULL;
	}

	return conn_cnt;
}

void gatt_fsm_send_msg(uint8_t evt, void *param)
{
	FsmEventCx(g_app_gatt_fsminst, evt, hlp_gatt_fsm_proc, (void *)param);
}

static void hlp_gatt_fsm_proc(struct FsmInst *fi, unsigned char event, void *param)
{
	 LOG("hlp_gatt_fsm_proc, evnet:%x", event);
	switch (event) {
	case FSM_EV_CMD_GATT_TEST:
		hlp_gatt_fsm_gatt_test(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_GET_DEVICE_TYPE:
		hlp_gatt_fsm_gatt_client_get_device_type(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_REGISTER_APP:
		hlp_gatt_fsm_gatt_client_register_app(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_UNREGISTER_APP:
		hlp_gatt_fsm_gatt_client_unregister_app(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_CONNECT:
		hlp_gatt_fsm_gatt_client_connect(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_DISCONNECT:
		hlp_gatt_fsm_gatt_client_disconnect(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SET_PREFERRED_PHY:
		hlp_gatt_fsm_gatt_client_set_preferred_phy(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_READ_PHY:
		hlp_gatt_fsm_gatt_client_read_phy(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_REFRESH:
		hlp_gatt_fsm_gatt_client_refresh(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SEARCH_SERVICE:
		hlp_gatt_fsm_gatt_client_search_service(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_DISCOVER_SERVICE_BY_UUID:
		hlp_gatt_fsm_gatt_client_discover_service_by_uuid(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_GET_GATT_DB:
		hlp_gatt_fsm_gatt_client_get_gatt_db(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_READ_CHARACTERISTIC:
		hlp_gatt_fsm_gatt_client_read_characteristic(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_READ_USING_CHARACTERISTIC_UUID:
		hlp_gatt_fsm_gatt_client_read_using_characteristic_uuid(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_READ_DESCRIPTOR:
		hlp_gatt_fsm_gatt_client_read_descriptor(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_WRITE_CHARACTERISTIC:
		hlp_gatt_fsm_gatt_client_write_characteristic(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_WRITE_DESCRIPTOR:
		hlp_gatt_fsm_gatt_client_write_descriptor(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_EXECUTE_WRITE:
		hlp_gatt_fsm_gatt_client_execute_write(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_REGISTER_FOR_NOTIFICATIONS:
		hlp_gatt_fsm_gatt_client_register_for_notifications(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_READ_REMOTE_RSSI:
		hlp_gatt_fsm_gatt_client_read_remote_rssi(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_CONFIGURE_MTU:
		hlp_gatt_fsm_gatt_client_configure_mtu(param);
		break;
	case FSM_EV_CMD_GATT_CONNECTION_PARAMETER_UPDATE:
		hlp_gatt_fsm_gatt_connection_parameter_update(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_REGISTER_APP:
		hlp_gatt_fsm_gatt_server_register_app(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_UNREGISTER_APP:
		hlp_gatt_fsm_gatt_server_unregister_app(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_CONNECT:
		hlp_gatt_fsm_gatt_server_connect(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_DISCONNECT:
		hlp_gatt_fsm_gatt_server_disconnect(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_SET_PREFERRED_PHY:
		hlp_gatt_fsm_gatt_server_set_preferred_phy(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_READ_PHY:
		hlp_gatt_fsm_gatt_server_read_phy(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_ADD_SERVICE:
		hlp_gatt_fsm_gatt_server_add_service(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_STOP_SERVICE:
		hlp_gatt_fsm_gatt_server_stop_service(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_DELETE_SERVICE:
		hlp_gatt_fsm_gatt_server_delete_service(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_SEND_INDICATION:
		hlp_gatt_fsm_gatt_server_send_indication(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_SEND_NOTIFICATION:
		hlp_gatt_fsm_gatt_server_send_notification(param);
		break;
	case FSM_EV_CMD_GATT_SERVER_SEND_RESPONSE:
		hlp_gatt_fsm_gatt_server_send_response(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_START_ADVERTISING_SET:
		hlp_gatt_fsm_gatt_advm_start_advertising_set(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_GETOWNADDRESS:
		hlp_gatt_fsm_gatt_advm_getownaddress(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_STOPADVERTISINGSET:
		hlp_gatt_fsm_gatt_advm_stopadvertisingset(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_ENABLEADVERTISINGSET:
		hlp_gatt_fsm_gatt_advm_enableadvertisingset(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_SET_ADVERTISING_DATA:
		hlp_gatt_fsm_gatt_advm_set_advertising_data(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_SET_SCAN_RESPONSE_DATA:
		hlp_gatt_fsm_gatt_advm_set_scan_response_data(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_SET_ADVERTISING_PARAMETERS:
		hlp_gatt_fsm_gatt_advm_set_advertising_parameters(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_PARAMETERS:
		hlp_gatt_fsm_gatt_advm_set_periodic_advertising_parameters(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_DATA:
		hlp_gatt_fsm_gatt_advm_set_periodic_advertising_data(param);
		break;
	case FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_ENABLE:
		hlp_gatt_fsm_gatt_advm_set_periodic_advertising_enable(param);
		break;
	case FSM_EV_CMD_GATT_SCAN_PERIODIC_STARTSYNC:
		hlp_gatt_fsm_gatt_scan_periodic_startsync(param);
		break;
	case FSM_EV_CMD_GATT_SCAN_PERIODIC_STOPSYNC:
		hlp_gatt_fsm_gatt_scan_periodic_stopsync(param);
		break;
	case FSM_EV_CMD_GATT_SCAN_REGISTER_SCANNER:
		hlp_gatt_fsm_gatt_scan_register_scanner(param);
		break;
	case FSM_EV_CMD_GATT_SCAN_UNREGISTER_SCANNER:
		hlp_gatt_fsm_gatt_scan_unregister_scanner(param);
		break;
	case FSM_EV_CMD_GATT_SCAN_CLIENT_SCAN:
		hlp_gatt_fsm_gatt_scan_client_scan(param);
		break;
	case FSM_EV_CMD_GATT_SCAN_SET_SCAN_PARAMETERS:
		hlp_gatt_fsm_gatt_scan_set_scan_parameters(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_ADD:
		hlp_gatt_fsm_gatt_client_scan_filter_add(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_DELETE:
		hlp_gatt_fsm_gatt_client_scan_filter_delete(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_ADD:
		hlp_gatt_fsm_gatt_client_scan_filter_param_add(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_CLEAR_ALL:
		hlp_gatt_fsm_gatt_client_scan_filter_param_clear_all(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_DELETE:
		hlp_gatt_fsm_gatt_client_scan_filter_param_delete(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_CLEAR:
		hlp_gatt_fsm_gatt_client_scan_filter_clear(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_ENABLE:
		hlp_gatt_fsm_gatt_client_scan_filter_enable(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_CONFIG_BATCH_SCAN_STORAGE:
		hlp_gatt_fsm_gatt_client_config_batch_scan_storage(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_START_BATCH_SCAN:
		hlp_gatt_fsm_gatt_client_start_batch_scan(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_STOP_BATCH_SCAN:
		hlp_gatt_fsm_gatt_client_stop_batch_scan(param);
		break;
	case FSM_EV_CMD_GATT_CLIENT_READ_SCAN_REPORTS:
		hlp_gatt_fsm_gatt_client_read_scan_reports(param);
		break;
	}
	FREE(param);
}

