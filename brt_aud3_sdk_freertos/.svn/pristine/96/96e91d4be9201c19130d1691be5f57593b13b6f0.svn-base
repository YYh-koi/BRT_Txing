#include "sysdep.h"
#include "btlist.h"
#include "gap_ui.h"
#include "bt_utils.h"
#include "auto_gatt_fsm.h"
#include "internal_gatt_device.h"
/* export the remote deivce to gap */
#include "internal_manager.h"
#include "internal_gap.h"
#include "hlp_le_scanner_fsm.h"

#define TAG "bse_le_scan:   "
#include "brt_trace.h"

#define LOG(fmt, ...) bse_log_print(TAG, "", fmt, ## __VA_ARGS__);

#define HAL_CBACK(P_CB, P_CBACK, ...)           \
    if (P_CB && P_CB->P_CBACK) {                \
        LOG("HAL %s->%s", #P_CB, #P_CBACK); \
        P_CB->P_CBACK(__VA_ARGS__);             \
    }                                           \
    else {                                      \
        LOG("%s", "Callback is NULL"); \
    }

#define LE_SCANNER_CBACK(P_CBACK, ...)	\
	if (s_user_data != NULL) { \
		HAL_CBACK((s_user_data->cbks), P_CBACK, __VA_ARGS__) \
	}

#define APP_BLE_GENERAL_CONTEXT (0xAB)
#define ET_LEGACY_MASK 		0x10
#define ET_CONNECTABLE_MASK	0x01

typedef struct {
	int scanner_id;
	bt_uuid_t app_uuid;
} scanner_t;

typedef struct le_scanner_user_data_tag {
	btgatt_client_callbacks_t *cbks;
	struct ListStru scanners;
	uint8_t scanner_id_base;
} le_scanner_user_data_t;

static le_scanner_user_data_t *s_user_data = NULL;
static UINT8 s_ble_scan_state = 0;
static tGAP_IndCbk_Result le_scan_result_callback(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type, void *par, UINT32 size);
static tGAP_IndCbk_Result le_scan_device_info_updated(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type,
        struct GAP_RemoteDeviceInfoStru *src_dev, UINT32 size);

static void on_scan_complete_cb(void *context, UINT16 result, struct GAP_DeviceDiscoveryCfmStru *cfm_par);

void hlp_le_scanner_fsm_init(btgatt_client_callbacks_t *callback)
{
	if (s_user_data == NULL) {
		s_user_data = (le_scanner_user_data_t*) NEW(sizeof(le_scanner_user_data_t));
		memset(s_user_data, 0, sizeof(le_scanner_user_data_t));
	}

	s_user_data->cbks = callback;
	GAP_RegisterIndCbk(NULL, GAP_IND_INQUIRY_RESULT, (GAP_IndicationCbk *)le_scan_result_callback);
    GAP_RegisterIndCbk(NULL, GAP_IND_REMOTE_INFO_UPDATE, (GAP_IndicationCbk *)le_scan_device_info_updated);
}

void hlp_le_scanner_fsm_done()
{
    GAP_UnregisterIndCbk(NULL, (GAP_IndicationCbk *)le_scan_result_callback);
    GAP_UnregisterIndCbk(NULL, (GAP_IndicationCbk *)le_scan_device_info_updated);
	if (s_user_data != NULL && (&(s_user_data->scanners)) != NULL) {
		List_RemoveAll(&(s_user_data->scanners));
	}
	FREE(s_user_data);
	s_user_data = NULL;
}

void hlp_gatt_fsm_gatt_scan_periodic_startsync(void *param)
{
	fsm_msg_gatt_scan_periodic_startsync_t *msg = (fsm_msg_gatt_scan_periodic_startsync_t *) param;
}

void hlp_gatt_fsm_gatt_scan_periodic_stopsync(void *param)
{
	fsm_msg_gatt_scan_periodic_stopsync_t *msg = (fsm_msg_gatt_scan_periodic_stopsync_t *) param;
}

void hlp_gatt_fsm_gatt_scan_register_scanner(void *param)
{
	fsm_msg_gatt_scan_register_scanner_t *msg = (fsm_msg_gatt_scan_register_scanner_t *) param;
	scanner_t *scanner;
	int scanner_id = s_user_data->scanner_id_base;
	
	scanner_id = (scanner_id + 1) % 255;
	List_AddTail(&s_user_data->scanners, (scanner = List_NodeNew(sizeof(scanner_t))));
	memcpy(&scanner->app_uuid, &msg->app_uuid, sizeof(bt_uuid_t));
	s_user_data->scanner_id_base = scanner->scanner_id = scanner_id;
	LE_SCANNER_CBACK(scanner_registered, BT_STATUS_SUCCESS, scanner->scanner_id, msg->app_uuid);
}

void hlp_gatt_fsm_gatt_scan_unregister_scanner(void *param)
{
	fsm_msg_gatt_scan_unregister_scanner_t *msg = (fsm_msg_gatt_scan_unregister_scanner_t *) param;
	scanner_t *scanner;

	scanner = s_user_data->scanners.head;
	while ((scanner != NULL) && (scanner->scanner_id != msg->scanner_id)) {
		scanner = LNEXT(scanner);
	}

	if (scanner != NULL) {
		List_RemoveAt(&s_user_data->scanners, scanner);
	}
}

void hlp_gatt_fsm_gatt_scan_client_scan(void *param)
{
	fsm_msg_gatt_scan_client_scan_t *msg = (fsm_msg_gatt_scan_client_scan_t *) param;
	int start = msg->start;

	if(start) {
		struct GAP_ScanParametersStru scan_params;
		scan_params.discoery_mode = GAP_LE_GENERAL_DISCOVERABLE;
		scan_params.speed = GAP_LE_ADV_SCAN_FAST;

		UINT16  ret = GAP_LESetScanParameters(NULL, &scan_params);

		struct GAP_DeviceDiscoveryReqStru gap_disc_req_param;
		gap_disc_req_param.flag = GAP_DEVICE_DISCOVERY_LE;//GAP_DEVICE_DISCOVERY_BOTH_BREDR_LE;
		gap_disc_req_param.num_responses = 0;
		gap_disc_req_param.inquiry_len = 0x30;
		ret = GAP_DeviceDiscovery(NULL, (void *) APP_BLE_GENERAL_CONTEXT, &gap_disc_req_param, on_scan_complete_cb);
             s_ble_scan_state = 1;
	} else {
		GAP_CancelDeviceDiscovery(NULL, (void *) APP_BLE_GENERAL_CONTEXT);
	}
}

static void hlp_le_scan_parameter_callback(void *context, UINT16 result, struct GAP_ExecuteCommandCfmStru *cfm_par)                                                                                        
{
	/* TODO XXX GATT_CLIENT_CBACK(scan_param_setup_completed, (UINT16)(intptr_t) context, address, evt_par->rssi, evt_par->status);*/
}

/* TODO save the parameters for next use. map the parameters to client_if(scanner_id?) */
void hlp_gatt_fsm_gatt_scan_set_scan_parameters(void *param)
{
	fsm_msg_gatt_scan_set_scan_parameters_t *msg = (fsm_msg_gatt_scan_set_scan_parameters_t *) param;
	struct HCI_LE_Set_Scan_ParametersStru par;

	par.le_scan_type = HCI_LE_SCAN_TYPE_ACTIVE_SCANNING; /* XXX */
	par.scanning_filter_policy = HCI_SCANNING_FILTER_POLICY_ACCEPT_ALL; /*XXX*/
	par.le_scan_interval = msg->scan_interval;
	par.le_scan_window = msg->scan_window;                                                                                                                                                 

	GAP_ExecuteCommandA(NULL, (void *)(intptr_t) msg->client_if, HCI_OPS_READ_RSSI, &par, sizeof(struct HCI_LE_Set_Scan_ParametersStru), hlp_le_scan_parameter_callback);
}

static uint8_t* get_adv_field_by_type(uint8_t* ad, size_t ad_len, uint8_t type, uint8_t* length_out)
{
	size_t pos = 0;

	while (pos != ad_len) {
		uint8_t len = ad[pos];

		if (len == 0) break;
		if (pos + len >= ad_len) break;

		uint8_t adv_type = ad[pos + 1];

		LOG("adv_type = 0x%x", adv_type);
		if (adv_type == type) {
			*length_out = len - 1;
			return (ad + pos + 2);
		}

		pos += len + 1; /* skip the length of data */
	}

	*length_out = 0;
	return NULL;
}

static tGAP_IndCbk_Result le_scan_result_callback(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type, void *par, UINT32 size)
{
	struct GAP_RemoteDeviceInfoStru *p_remote_info = (struct GAP_RemoteDeviceInfoStru *)par;
	bt_bdaddr_t remote_addr;
	unsigned char *bd;
	uint8_t event_type = ET_LEGACY_MASK | ET_CONNECTABLE_MASK; /* TODO */
	uint8_t address_type;
	uint8_t primary_phy = 0; /* TODO*/
	uint8_t secondary_phy = 0;
	uint8_t advertising_sid = 0;
	uint8_t tx_power = 0;
	uint8_t rssi;
	uint16_t periodic_adv_int = 0; /* TODO*/
	uint8_t adv_data_len;
	uint8_t type_len;
	uint8_t *type_ptr;
        LOG("---->le_scan_result_callback,(%d)",s_ble_scan_state);
	LOG("scan_%s, cap:%s.", ind_type == GAP_IND_INQUIRY_RESULT ? "result" : "complete",
			p_remote_info == NULL ? "unknown" : (p_remote_info->addr.cap == HCI_ADDRESS_CAP_LE ? "LE" : "BREDR"));

	if(s_ble_scan_state == 0 ||ind_type != GAP_IND_INQUIRY_RESULT) {
		return GAP_IND_RESULT_NOT_CARE;
	}

	if((p_remote_info == NULL) ||!(p_remote_info->addr.cap == HCI_ADDRESS_CAP_LE)) {
		return GAP_IND_RESULT_NOT_CARE;
	}

	rssi = p_remote_info->rssi;
	address_type = p_remote_info->addr.atype; /*XXX*/

	HCI_ADDR_STRU_TO_BLUEDROID_BD(&(p_remote_info->addr), &remote_addr);
	bd = remote_addr.address;
	gatt_device_add(&remote_addr, p_remote_info->addr.atype, 0xFF, BTGATT_APP_IF_INVALID);

	adv_data_len = p_remote_info->size - sizeof(*p_remote_info);
	type_ptr = get_adv_field_by_type(p_remote_info->eir, adv_data_len, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, &type_len);
	if (type_len > 0 && type_ptr != NULL) {
		bt_remote_device_t dev;
		memset(&dev, 0, sizeof(bt_remote_device_t));
		memcpy(dev.bd, remote_addr.address, BD_ADDR_LEN);
		dev.cap = HCI_ADDRESS_CAP_LE;
		LOG("%s name_len:%d, name:%s.", __func__, type_len, type_ptr);
		memcpy(dev.name, type_ptr, type_len > DEV_NAME_LEN ? DEV_NAME_LEN : type_len);
		//zjgtet on_le_scan_result(&dev);
	}

	type_ptr = get_adv_field_by_type(p_remote_info->eir, adv_data_len, HCI_EIR_DATATYPE_TX_POWER_LEVEL, &type_len);
	if (type_len > 0) {
		tx_power = *type_ptr;
	}

	//debug_print_data(TAG, p_remote_info->eir, adv_data_len);
	LE_SCANNER_CBACK(scan_result, event_type, address_type, remote_addr, primary_phy, secondary_phy, advertising_sid, tx_power, rssi, periodic_adv_int, adv_data_len, p_remote_info->eir);

	return GAP_IND_RESULT_NOT_CARE;
}
static tGAP_IndCbk_Result le_scan_device_info_updated(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type,
        struct GAP_RemoteDeviceInfoStru *src_dev, UINT32 size)
{
    struct GAP_RemoteDeviceInfoStru *p_remote_info = (struct GAP_RemoteDeviceInfoStru *)src_dev;
        bt_bdaddr_t remote_addr;
        unsigned char *bd;
        uint8_t event_type = ET_LEGACY_MASK | ET_CONNECTABLE_MASK; /* TODO */
        uint8_t address_type;
        uint8_t primary_phy = 0; /* TODO*/
        uint8_t secondary_phy = 0;
        uint8_t advertising_sid = 0;
        uint8_t tx_power = 0;
        uint8_t rssi;
        uint16_t periodic_adv_int = 0; /* TODO*/
        uint8_t adv_data_len;
        uint8_t type_len;
        uint8_t *type_ptr;
        LOG("----->le_scan_device_info_updated");
        LOG("scan_%s, cap:%s.", ind_type == GAP_IND_INQUIRY_RESULT ? "result" : "complete",
                p_remote_info == NULL ? "unknown" : (p_remote_info->addr.cap == HCI_ADDRESS_CAP_LE ? "LE" : "BREDR"));
    
        if(ind_type != GAP_IND_INQUIRY_RESULT) {
            //return GAP_IND_RESULT_NOT_CARE;
        }
    
        if((p_remote_info == NULL) ||!(p_remote_info->addr.cap == HCI_ADDRESS_CAP_LE)) {
            return GAP_IND_RESULT_NOT_CARE;
        }
    
        rssi = p_remote_info->rssi;
        address_type = p_remote_info->addr.atype; /*XXX*/
    
        HCI_ADDR_STRU_TO_BLUEDROID_BD(&(p_remote_info->addr), &remote_addr);
        bd = remote_addr.address;
        gatt_device_add(&remote_addr, p_remote_info->addr.atype, 0xFF, BTGATT_APP_IF_INVALID);
    
        adv_data_len = p_remote_info->size - sizeof(*p_remote_info);
        type_ptr = get_adv_field_by_type(p_remote_info->eir, adv_data_len, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, &type_len);
        if (type_len > 0 && type_ptr != NULL) {
            bt_remote_device_t dev;
            memset(&dev, 0, sizeof(bt_remote_device_t));
            memcpy(dev.bd, remote_addr.address, BD_ADDR_LEN);
            dev.cap = HCI_ADDRESS_CAP_LE;
            LOG("%s name_len:%d, name:%s.", __func__, type_len, type_ptr);
            memcpy(dev.name, type_ptr, type_len > DEV_NAME_LEN ? DEV_NAME_LEN : type_len);
            //zjgtest on_le_scan_result(&dev);
        }
    
        type_ptr = get_adv_field_by_type(p_remote_info->eir, adv_data_len, HCI_EIR_DATATYPE_TX_POWER_LEVEL, &type_len);
        if (type_len > 0) {
            tx_power = *type_ptr;
        }
    
        //debug_print_data(TAG, p_remote_info->eir, adv_data_len);
        LE_SCANNER_CBACK(scan_result, event_type, address_type, remote_addr, primary_phy, secondary_phy, advertising_sid, tx_power, rssi, periodic_adv_int, adv_data_len, p_remote_info->eir);

	return GAP_IND_RESULT_NOT_CARE;
}

static void on_scan_complete_cb(void *context, UINT16 result, struct GAP_DeviceDiscoveryCfmStru *cfm_par)
{
	LOG("on_scan_complete_cb,result = 0X%02X", result);
        s_ble_scan_state = 0;
	/* TODO */
}

