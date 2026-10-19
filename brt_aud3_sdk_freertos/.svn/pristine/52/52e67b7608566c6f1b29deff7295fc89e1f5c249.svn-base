#include "bt_utils.h"
#include "ble_advertiser.h"
#include "gap_ui.h"
#include "att_ui.h"
#include "auto_gatt_fsm.h"
#include "internal_gatt_device.h"
#include "internal_gatt_fsm_server.h"
#include "hlp_le_advertiser_fsm.h"
//#include "bt_sdk_types.h"

#define TAG "bse_le_adv"
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

#define LE_ADVERTISER_CBACK(P_CBACK, ...)	\
	if (s_le_advertiser_user_data != NULL) { \
		HAL_CBACK((s_le_advertiser_user_data->cbks), P_CBACK, __VA_ARGS__) \
	}

typedef struct le_advertiser_user_data_tag{
	btgatt_server_callbacks_t *cbks;
	tBTM_BLE_ADV_PARAMS adv_param;
	tBLE_PERIODIC_ADV_PARAMS periodic_adv_param;
	int advm_ids[BTGATT_MAX_SERVER_APP_NUM];
} le_advertiser_user_data_t;

static le_advertiser_user_data_t *s_le_advertiser_user_data = NULL;

static void hlp_gatt_server_set_adv_settings(short adv_set_param_len, tBTM_BLE_ADV_PARAMS *adv_set_param, int reg_id);
static void hlp_gatt_serevr_set_periodic_adv_data(short periodic_adv_data_len, tBLE_PERIODIC_ADV_PARAMS *periodic_adv_data, int reg_id);
static void hlp_gatt_server_set_periodic_data(short periodic_data_len, uint8_t *periodic_data, int reg_id);
static void hlp_gatt_server_parse_data(short adv_data_len, uint8_t *adv_data, int reg_id, struct ListStru *adlist, struct HCI_EIR_DataNodeStru *node, struct HCI_LE_Set_Advertising_DataStru *in);
static void hlp_gatt_server_start_advertising(int reg_id);
static void hlp_gatt_server_stop_advertising(int advertiser_id);
static void hlp_gatt_server_disconnect_device(unsigned char server_if, bt_bdaddr_t address, short conn_id);

void hlp_le_advertiser_fsm_init(btgatt_server_callbacks_t *callback)
{
	if (s_le_advertiser_user_data == NULL) {
		s_le_advertiser_user_data = (le_advertiser_user_data_t *)NEW(sizeof(le_advertiser_user_data_t));
		memset(s_le_advertiser_user_data, 0, sizeof(le_advertiser_user_data_t));
	}
	s_le_advertiser_user_data->cbks = callback;
}

void hlp_le_advertiser_fsm_done()
{
	FREE(s_le_advertiser_user_data);
	s_le_advertiser_user_data = NULL;
}

void hlp_le_start_adv(char* advData,int advLen)
{
	hlp_gatt_server_set_adv_data( advLen,advData, 0);
	hlp_gatt_server_start_advertising(0);
}

void hlp_le_start_adv_ex(char* advData,int advLen)
{
	hlp_gatt_server_set_adv_data( advLen,advData, 0);
	hlp_gatt_server_start_advertising_ex(0);
	
}

void hlp_le_start_adv_scan_response_ex(char* advData,int advLen)
{
	hlp_gatt_server_set_scan_response_data( advLen,advData, 0);
}


void hlp_le_stop_adv()
{
	hlp_gatt_server_stop_advertising(0);
}

void hlp_gatt_fsm_gatt_advm_start_advertising_set(void *param)
{
	fsm_msg_gatt_advm_start_advertising_set_t *msg = (fsm_msg_gatt_advm_start_advertising_set_t *) param;

	uint16_t adv_set_param_len = msg->param_len;
	LOG("adv_set_param_len: %d", adv_set_param_len);
	tBTM_BLE_ADV_PARAMS *adv_set_param = NULL;
	if (adv_set_param_len == sizeof(tBTM_BLE_ADV_PARAMS)) {
		adv_set_param = (tBTM_BLE_ADV_PARAMS *)msg->param;
	} 

	uint16_t adv_data_len = msg->adv_data_len;
	LOG("adv_data_len : %d", adv_data_len);
	uint8_t * adv_data = msg->adv_data;

	uint16_t scan_resp_data_len = msg->scan_resp_data_len;
	LOG("scan_resp_data_len : %d", scan_resp_data_len);
	uint8_t *scan_resp_data = msg->scan_resp_data;

	uint16_t periodic_adv_data_len = msg->periodic_adv_data_len;
	LOG("period_adv_set_param_len: %d", periodic_adv_data_len);
	tBLE_PERIODIC_ADV_PARAMS * periodic_adv_data = NULL;
	if (periodic_adv_data_len == sizeof(tBLE_PERIODIC_ADV_PARAMS)) {
		periodic_adv_data = (tBLE_PERIODIC_ADV_PARAMS *)msg->period_adv_data;
	}

	uint16_t periodic_data_len = msg->periodic_data_len;
	LOG("period_data_len: %d", periodic_data_len);
	uint8_t * periodic_data = msg->periodic_data;

	int duration = msg->duration;
	LOG("duration : %d", duration);
	int max_ext_adv_events = msg->max_ext_adv_events;
	LOG("max_ext_adv_events : %d", max_ext_adv_events);
	int reg_id = msg->reg_id & 0xFFFFFFFF;
	LOG("reg_id : %d", reg_id);

	hlp_gatt_server_set_adv_settings(adv_set_param_len, adv_set_param, reg_id);

        /*adv_data's format just use EIR dataformat.*/
	hlp_gatt_server_set_adv_data(adv_data_len, adv_data, reg_id);
         /*scan_resp_data's format just use EIR dataformat.*/
	hlp_gatt_server_set_scan_response_data(scan_resp_data_len, scan_resp_data, reg_id);
	
	hlp_gatt_serevr_set_periodic_adv_data(periodic_adv_data_len, periodic_adv_data, reg_id);

	hlp_gatt_server_set_periodic_data(periodic_data_len, periodic_data, reg_id);

	hlp_gatt_server_start_advertising(reg_id);
}

void hlp_gatt_fsm_gatt_advm_getownaddress(void *param)
{
	fsm_msg_gatt_advm_getownaddress_t *msg = (fsm_msg_gatt_advm_getownaddress_t *) param;
}

void hlp_gatt_fsm_gatt_advm_stopadvertisingset(void *param)
{
	fsm_msg_gatt_advm_stopadvertisingset_t *msg = (fsm_msg_gatt_advm_stopadvertisingset_t *) param;
	hlp_gatt_server_stop_advertising(msg->advertiser_id);
}

void hlp_gatt_fsm_gatt_advm_enableadvertisingset(void *param)
{
	fsm_msg_gatt_advm_enableadvertisingset_t *msg = (fsm_msg_gatt_advm_enableadvertisingset_t *) param;
}

void hlp_gatt_fsm_gatt_advm_set_advertising_data(void *param)
{
	fsm_msg_gatt_advm_set_advertising_data_t *msg = (fsm_msg_gatt_advm_set_advertising_data_t *) param;
}

void hlp_gatt_fsm_gatt_advm_set_scan_response_data(void *param)
{
	fsm_msg_gatt_advm_set_scan_response_data_t *msg = (fsm_msg_gatt_advm_set_scan_response_data_t *) param;
}

void hlp_gatt_fsm_gatt_advm_set_advertising_parameters(void *param)
{
	fsm_msg_gatt_advm_set_advertising_parameters_t *msg = (fsm_msg_gatt_advm_set_advertising_parameters_t *) param;
}

void hlp_gatt_fsm_gatt_advm_set_periodic_advertising_parameters(void *param)
{
	fsm_msg_gatt_advm_set_periodic_advertising_parameters_t *msg = (fsm_msg_gatt_advm_set_periodic_advertising_parameters_t *) param;
}

void hlp_gatt_fsm_gatt_advm_set_periodic_advertising_data(void *param)
{
	fsm_msg_gatt_advm_set_periodic_advertising_data_t *msg = (fsm_msg_gatt_advm_set_periodic_advertising_data_t *) param;
}

void hlp_gatt_fsm_gatt_advm_set_periodic_advertising_enable(void *param)
{
	fsm_msg_gatt_advm_set_periodic_advertising_enable_t *msg = (fsm_msg_gatt_advm_set_periodic_advertising_enable_t *) param;
}

static void hlp_gatt_server_set_adv_settings(short adv_set_param_len, tBTM_BLE_ADV_PARAMS *adv_set_param, int reg_id)
{
	if (adv_set_param_len == 0 || adv_set_param == NULL) {
		return;
	}
	
	memcpy(&s_le_advertiser_user_data->adv_param, adv_set_param, sizeof(tBTM_BLE_ADV_PARAMS));

	int min = adv_set_param->adv_int_min;
	int max = adv_set_param->adv_int_max;

	LOG("intervale %d -> %d", min, max);
}

void hlp_gatt_server_set_adv_data(short adv_data_len, uint8_t *adv_data, int reg_id)
{
	if (adv_data_len <= 0) {
		return;
	}
	LOG("%s", __func__);


	struct ListStru adlist;
	struct HCI_EIR_DataNodeStru *node;
	struct HCI_LE_Set_Advertising_DataStru *in = (struct HCI_LE_Set_Advertising_DataStru *) NEW(sizeof(struct HCI_LE_Set_Advertising_DataStru));
	memset(in, 0, sizeof( struct HCI_LE_Set_Advertising_DataStru));
	memset(&adlist, 0, sizeof( struct ListStru));
	
	
	/*interval range this is just for adv_data only*/
    #if 0
	node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct HCI_EIR_DataNodeStru));
	node->len = 1 + 4;
	node->eir_data_type = HCI_EIR_DATATYPE_SLAVE_CONNECTION_INTERVAL_RANGE;
	ENCODE2BYTE_LITTLE(&node->u.slave_connection_interval_range.conn_interval_min[0], s_le_advertiser_user_data->adv_param.adv_int_min);
	ENCODE2BYTE_LITTLE(&node->u.slave_connection_interval_range.conn_interval_max[0], s_le_advertiser_user_data->adv_param.adv_int_max);
	List_AddTail(&adlist, node);
	#endif
#if 0
	/* Appearance */
	node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof( struct HCI_EIR_DataNodeStru));
	node->len = 1 + 2;
	node->eir_data_type = HCI_EIR_DATATYPE_APPEARANCE;
	ENCODE2BYTE_LITTLE(node->u.appearence, 0);
	List_AddTail(&adlist, node);
#endif

	hlp_gatt_server_parse_data(adv_data_len, adv_data, reg_id, &adlist, node, in);

	in->advertising_data_length = HCI_Extended_Inquiry_Response_Encode(&adlist, in->advertising_data, in->advertising_data + 31);

	GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_LE_SET_ADVERTISING_DATA, in, sizeof(*in), NULL);
}

void hlp_gatt_server_set_scan_response_data(short scan_resp_data_len, uint8_t *scan_resp_data, int reg_id)
{
	if (scan_resp_data_len <= 0 || scan_resp_data == NULL) {
		return;
	}
	LOG("%s", __func__);
	
	struct ListStru adlist;
	struct HCI_EIR_DataNodeStru *node;
	struct HCI_LE_Set_Advertising_DataStru *in = (struct HCI_LE_Set_Advertising_DataStru *) NEW(sizeof(struct HCI_LE_Set_Advertising_DataStru));

	memset(in, 0, sizeof( struct HCI_LE_Set_Advertising_DataStru));
	memset(&adlist, 0, sizeof( struct ListStru));

		/*AD FLAGS this is just for adv_data only*/
	node =(struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof( struct HCI_EIR_DataNodeStru));
	node->len = 1 + 1;
	node->eir_data_type = HCI_EIR_DATATYPE_FLAGS;
	node->u.flags = HCI_EIR_FLAGS_LE_GENERAL_DISCOVERABLE_MODE;
	//node->u.flags |= HCI_EIR_FLAGS_BREDR_NOT_SUPPORTED;
	List_AddTail(&adlist, node);

	hlp_gatt_server_parse_data(scan_resp_data_len, scan_resp_data, reg_id, &adlist, node, in);

	in->advertising_data_length = HCI_Extended_Inquiry_Response_Encode(&adlist, in->advertising_data, in->advertising_data + 31);
	GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_LE_SET_SCAN_RESPONSE_DATA, in, sizeof(*in), NULL);
}

static void hlp_gatt_serevr_set_periodic_adv_data(short periodic_adv_data_len, tBLE_PERIODIC_ADV_PARAMS *periodic_adv_data, int reg_id)
{
	if (periodic_adv_data_len == 0 || periodic_adv_data == NULL) {
		return;
	}
	LOG("%s", __func__);
	tBLE_PERIODIC_ADV_PARAMS *data = (tBLE_PERIODIC_ADV_PARAMS *)periodic_adv_data;

	memcpy(&s_le_advertiser_user_data->periodic_adv_param, data, sizeof(tBLE_PERIODIC_ADV_PARAMS));
}

static void hlp_gatt_server_set_periodic_data(short periodic_data_len, uint8_t *periodic_data, int reg_id)
{
	if (periodic_data_len <= 0 || periodic_data == NULL) {
		return;
	}
	LOG("%s", __func__);
	
	//TODO
#if 0
	struct ListStru adlist;
	struct HCI_EIR_DataNodeStru *node;
	struct HCI_LE_Set_Advertising_DataStru *in = (struct HCI_LE_Set_Advertising_DataStru *) NEW(sizeof(struct HCI_LE_Set_Advertising_DataStru));

	memset(in, 0, sizeof( struct HCI_LE_Set_Advertising_DataStru));
	memset(&adlist, 0, sizeof( struct ListStru));
	hlp_gatt_server_parse_data(periodic_data_len, periodic_data, reg_id, &adlist, node, in);
#endif
}

static void hlp_gatt_server_start_advertising(int reg_id)
{
	struct GAP_AdvParametersStru adv_params;

	adv_params.type = HCI_ADVERTISING_TYPE_NON_CONNECTABLE_UNDIRECTED;
	adv_params.discoery_mode = GAP_LE_GENERAL_DISCOVERABLE;
	adv_params.speed = GAP_LE_ADV_SCAN_FAST;
	adv_params.adv_filter_policy = HCI_ADVERTISING_FILTER_POLICY_SCAN_ANY_CONNECT_ANY;

	int ret = GAP_LESetAdvertisingParameters(NULL, &adv_params);

	int status = BT_STATUS_FAIL;
	if (ret == HCI_STATUS_OK) {
		status = BT_STATUS_SUCCESS;
	}
    //advertising_set_started_cb
	LE_ADVERTISER_CBACK(advertising_set_started, reg_id, reg_id/*advertiser_id*/, s_le_advertiser_user_data->adv_param.tx_power, status);
	
	LE_ADVERTISER_CBACK(advertising_enabled, reg_id/*advertiser_id*/, 1, status);
}

void hlp_gatt_server_start_advertising_ex(int reg_id)
{
	struct GAP_AdvParametersStru adv_params;

	adv_params.type = HCI_ADVERTISING_TYPE_CONNECTABLE_UNDIRECTED;
	adv_params.discoery_mode = GAP_LE_GENERAL_DISCOVERABLE;
	adv_params.speed = GAP_LE_ADV_SCAN_FAST;
	adv_params.adv_filter_policy = HCI_ADVERTISING_FILTER_POLICY_SCAN_ANY_CONNECT_ANY;

	int ret = GAP_LESetAdvertisingParameters(NULL, &adv_params);

	int status = BT_STATUS_FAIL;
	if (ret == HCI_STATUS_OK) {
		status = BT_STATUS_SUCCESS;
	}
    //advertising_set_started_cb
	LE_ADVERTISER_CBACK(advertising_set_started, reg_id, reg_id/*advertiser_id*/, s_le_advertiser_user_data->adv_param.tx_power, status);
	
	LE_ADVERTISER_CBACK(advertising_enabled, reg_id/*advertiser_id*/, 1, status);
}
static void hlp_gatt_server_stop_advertising(int advertiser_id)
{
	LOG("%s", __func__);
	struct GAP_AdvParametersStru adv_params;

	adv_params.type = HCI_ADVERTISING_TYPE_NON_CONNECTABLE_UNDIRECTED;
	adv_params.discoery_mode = GAP_LE_NONE_DISCOVERABLE;
	adv_params.speed = GAP_LE_ADV_SCAN_FAST;
	adv_params.adv_filter_policy = HCI_ADVERTISING_FILTER_POLICY_SCAN_ANY_CONNECT_ANY;

	//XXX disconnect all connected devices when stop adv
	//gatt_device_iterator_by_action_type(DISCONNECT_LE_AS_SERVER, hlp_gatt_fsm_gatt_server_disconnect);

	int ret = GAP_LESetAdvertisingParameters(NULL, &adv_params);

	int status = BT_STATUS_FAIL;
	if (ret == HCI_STATUS_OK) {
		status = BT_STATUS_SUCCESS;
	}
	LE_ADVERTISER_CBACK(advertising_enabled, advertiser_id, 0, status);
}

static bt_uuid_t get_uuid128(UINT8 type, UINT8 *data) 
{
	bt_uuid_t uuid = {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10,
					0x00, 0x00, 0x00, 0x00};
	switch (type) {
		case HCI_EIR_DATATYPE_COMPLETE_LIST_OF_16BIT_SERVICE_CLASS_UUIDS:
			uuid.uu[12] = *(data);
			uuid.uu[13] = *(data + 1);
			break;
		case HCI_EIR_DATATYPE_COMPLETE_LIST_OF_32BIT_SERVICE_CLASS_UUIDS:
			uuid.uu[12] = *(data);
			uuid.uu[13] = *(data + 1);
			uuid.uu[14] = *(data + 2);
			uuid.uu[15] = *(data + 3);
			break;
	}

	return uuid;
}

static void hlp_gatt_server_parse_data(short adv_data_len, uint8_t *adv_data, int reg_id, struct ListStru *adlist, struct HCI_EIR_DataNodeStru *node, struct HCI_LE_Set_Advertising_DataStru *in)
{

	uint8_t *name = NULL;
	int name_len = 0;

	uint8_t *cod = NULL;
	int cod_len = 0;

	uint8_t *manufacturer_data = NULL;
	int manufacturer_data_len = 0;

	uint8_t tx_power_len = 0;
	uint8_t tx_power;

	int uuid_len_tmp;
	bt_uuid_t *uuid128_tmp = NULL;
	int uuid_len = 0;
	bt_uuid_t *uuid128 = NULL;
        unsigned char *uuid16 = NULL;
        unsigned char *uuid16_tmp = NULL;
	int service_data_len_tmp = 0;
	uint8_t * service_data_tmp = NULL;
	int service_data_len = 0;
	uint8_t * service_data = NULL;

	int i = 0;
	int j = 0;
	int k = 0;//uuid numbers
	//parse adv data
	uint8_t * p = adv_data;
	int len;
	int type;
	/* format : len(byte)|type(byte)|value (*)*/
	for (i; i < adv_data_len; i++) {
		len = p[0];
		type = p[1];
		p += 2;//offset len + type
		LOG("EIR STR len(%d),type(%x)",len,type);
		switch (type) {
			case HCI_EIR_DATATYPE_SHORTENED_LOCAL_NAME:
			case HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME:
				if(len > 0) {
					name_len = len - 1;
					name = p;
					p += name_len;
				}
				break;
			case HCI_EIR_DATATYPE_APPEARANCE :
				if(len == 3){
					cod_len = len -1;
					cod = p;
					p += cod_len;
				}else{
					p += (len -1);
				}
				break;
			case HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA:
				manufacturer_data_len= len - 1;
				manufacturer_data = p;	
				p += manufacturer_data_len;
				break;
			case HCI_EIR_DATATYPE_TX_POWER_LEVEL:
				tx_power_len = len - 1;
				if (tx_power_len != 1) {
					LOG("tx_power error");
				} else {
					tx_power = *(p);//always 0
				}
				p += tx_power_len;
				break;
			case HCI_EIR_DATATYPE_COMPLETE_LIST_OF_16BIT_SERVICE_CLASS_UUIDS:
				uuid_len = len - 1;
				j = 0;
                #if 1
                            uuid16 = (unsigned char*)malloc(uuid_len);
                            memcpy(uuid16,p,uuid_len);
                            p+=uuid_len;
                #else
				for (j; j < uuid_len_tmp; ) {
					bt_uuid_t uuid = get_uuid128(HCI_EIR_DATATYPE_COMPLETE_LIST_OF_16BIT_SERVICE_CLASS_UUIDS, p);
					uuid_len += 16;
					uuid128 = (bt_uuid_t *)realloc(uuid128, uuid_len);
					uuid128_tmp = uuid128 + k;
					memcpy(uuid128_tmp->uu, uuid.uu, 16);
					j += 2;
					p += 2;
					k++;
				}
                #endif
				break;
			case HCI_EIR_DATATYPE_COMPLETE_LIST_OF_32BIT_SERVICE_CLASS_UUIDS:
				j = 0;
				LOG("UUID[%02x:%02x:%02x:%02X]", *p ,*(p + 1), *(p + 2), *(p + 3));//FIXME not work
				for (j; j < uuid_len_tmp; ) {
					bt_uuid_t uuid = get_uuid128(HCI_EIR_DATATYPE_COMPLETE_LIST_OF_32BIT_SERVICE_CLASS_UUIDS, p);
					uuid_len += 16;
					uuid128 = (bt_uuid_t *)realloc(uuid128, uuid_len);
					uuid128_tmp = uuid128 + k;
					memcpy(uuid128_tmp->uu, uuid.uu, 16);
					j += 4;
					p += 4;
					k++;
				}
				break;
			case HCI_EIR_DATATYPE_COMPLETE_LIST_OF_128BIT_SERVICE_CLASS_UUIDS: 
				uuid_len_tmp = len - 1;
				j = 0;
				for (j; j < uuid_len_tmp; ) {
					bt_uuid_t uuid;
					uuid_len += 16;
					memcpy(uuid.uu, p, 16);
					uuid128 = (bt_uuid_t *)realloc(uuid128, uuid_len);
					uuid128_tmp = uuid128 + k;
					memcpy(uuid128_tmp->uu, uuid.uu, 16);
					j += 16;
					p += 16;
					k++;
				}
				break;
			case HCI_EIR_DATATYPE_SERVICE_DATA://16 bit uuid
			case 20://32 bit uuid
			case 21://128 bit uuid
				service_data_len_tmp = len - 1;
				service_data = (uint8_t *)realloc(service_data, service_data_len + service_data_len_tmp);
				service_data_tmp = service_data + (service_data_len);
				memcpy(service_data_tmp, p, service_data_len_tmp);
				service_data_len += service_data_len_tmp;
				p += service_data_len_tmp;
				break;
			//TODO other 
			default:
				p += (len - 1);
				break;
		}
		i += (len + 1);
	}

		/**manufacturer*/
	if (manufacturer_data) {
		node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct HCI_EIR_DataNodeStru) + manufacturer_data_len - 2);
		node->len = 1 + manufacturer_data_len;
		node->eir_data_type = HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA;
		memcpy(&node->u.manufacturer_data, manufacturer_data, manufacturer_data_len);
		List_AddTail(adlist, node);
	}

	   /*16bit Service UUIDs */
    if (uuid_len > 0 && uuid16) {
		struct ATT_UUIDStru uu;
		UINT8 *p;
		UINT16 ulen = 0;

		node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct HCI_EIR_DataNodeStru) + uuid_len);
		node->len = 1 + uuid_len;
		node->eir_data_type = HCI_EIR_DATATYPE_COMPLETE_LIST_OF_16BIT_SERVICE_CLASS_UUIDS;
		p = node->u.uuid16;
       
		do {
			memcpy(p+ulen, uuid16 + ulen, 2);
			ulen += 2; // FIXME all service is uuid128?
		} while (ulen < uuid_len);

		List_AddTail(adlist, node);

		free(uuid16);
	}

	
	/** service Data */
	if (service_data_len > 0 && service_data) {
		// FIXME
		node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct HCI_EIR_DataNodeStru) + service_data_len - 2);
		node->len = 1 + service_data_len;
		node->eir_data_type = HCI_EIR_DATATYPE_SERVICE_DATA;
		memcpy(&node->u.service_data, service_data, service_data_len);
		List_AddTail(adlist, node);
	}

	//TODO
	if (tx_power_len > 0) {
#if 0
		node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct HCI_EIR_DataNodeStru));
		node->len = 1 + 1;
		node->eir_data_type = HCI_EIR_DATATYPE_TX_POWER_LEVEL;
		node->u.tx_power_level = tx_power;
		List_AddTail(adlist, node);
#endif
	}

	/*128bit Service UUIDs */
	if (uuid_len > 0 && uuid128) {
		UINT16 u16;
		struct ATT_UUIDStru uu;
		UINT8 *p;
		UINT16 ulen = 0;
		UINT8 count = uuid_len / 16;

		node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct HCI_EIR_DataNodeStru) + (count << 4));
		node->len = 1 + (count << 4);
		node->eir_data_type = HCI_EIR_DATATYPE_COMPLETE_LIST_OF_128BIT_SERVICE_CLASS_UUIDS;
		p = node->u.uuid128;
		do {
			memcpy(p, (uint8_t *)uuid128 + ulen, 16);
			ulen += 16; // FIXME all service is uuid128?
		} while (ulen < uuid_len);

		List_AddTail(adlist, node);

		free(uuid128);
	}

	if (name) {
		char * local_name = (char *)NEW(name_len + 1);
		memset(local_name, 0, name_len + 1);
		memcpy(local_name, name, name_len);
            LOG("*******eir name:%s,name_len(%d)",local_name,name_len);
		int len = 0;
		len = strlen(local_name);
		node =(struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct  HCI_EIR_DataNodeStru) + len);
		node->len = 1 + len;
		node->eir_data_type = HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME;
		memcpy(node->u.name.data, local_name, len);
		List_AddTail(adlist, node);

		FREE(local_name);
	}
	if(cod){
		char * local_cod = (char *)NEW(cod_len + 1);
		memset(local_cod, 0, cod_len + 1);
		memcpy(local_cod, cod, cod_len);
           
		int len = 0;
		len = strlen(local_cod);
		node =(struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct  HCI_EIR_DataNodeStru) + len);
		node->len = 1 + len;
		node->eir_data_type = HCI_EIR_DATATYPE_APPEARANCE;
		memcpy(node->u.appearence, local_cod, len);
		List_AddTail(adlist, node);

		FREE(local_cod);
	}
}
