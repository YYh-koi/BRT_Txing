#include "bt_utils.h"
#include "internal_gatt_fsm_server.h"
#include "auto_gatt_fsm.h"
#include "brt_trace.h"
#include "internal_gatt_services.h"
#include "internal_gatt_device.h"
#include "internal_gatt_conn.h"
#include "gap_ui.h"
#include "sdap_uistru.h"
//#include "bt_sdk_types.h"
#include "internal_gatt_utils.h"

#define TAG "[bse_gatt_fsm_server]"
#define GATT_SERVER_DBG(fmt, ...) bse_log_print(TAG, "\r\n", fmt, ## __VA_ARGS__);

#define HAL_CBACK(P_CB, P_CBACK, ...)           \
    if (P_CB && P_CB->P_CBACK) {                \
        GATT_SERVER_DBG("HAL %s->%s", #P_CB, #P_CBACK); \
        P_CB->P_CBACK(__VA_ARGS__);             \
    }                                           \
    else {                                      \
        GATT_SERVER_DBG("%s", "Callback is NULL"); \
    }

typedef struct gatt_server_user_data_tag {
	btgatt_server_callbacks_t *cbks;
	UINT8 s_cur_server_if;
} gatt_server_user_data_t;

static gatt_server_user_data_t *s_gatt_server_user_data = NULL;

#define GATT_SERVER_CBACK(P_CBACK, ...)	\
	if (s_gatt_server_user_data != NULL) { \
		HAL_CBACK((s_gatt_server_user_data->cbks), P_CBACK, __VA_ARGS__) \
	}

#define IS_UUID_EQUAL(uuida, uuidb) ( !memcmp(uuida.uu, uuidb.uu, 16))

#define TRANSPORT_AUTO		0
#define TRANSPORT_BREDR		1
#define TRANSPORT_LE		2

/* NOTE: Shall be synchronized with definition in att_hlp.h, e.g. ATT_OP_READ_REQ */
#define ATT_EWRF_CANCEL_ALL             0x00
#define ATT_EWRF_IMMEDIATELY_WRITE_ALL  0x01
#define ATT_OP_READ_REQ                 0x0A    /* ATT_OP_READ_REQ */                                                                 
#define ATT_OP_READ_BLOB_REQ            0x0C    /* ATT_OP_READ_BLOB_REQ */
#define ATT_OP_READ_MULTIPLE_REQ        0x0E    /* ATT_OP_READ_MULTIPLE_REQ */
#define ATT_OP_WRITE_REQ                0x12    /* ATT_OP_WRITE_REQ */
#define ATT_OP_PREPARE_WRITE_REQ        0x16    /* ATT_OP_PREPARE_WRITE_REQ */
#define ATT_OP_EXECUTE_WRITE_REQ        0x18    /* ATT_OP_EXECUTE_WRITE_REQ */
#define ATT_OP_WRITE_COMMAND            0x52    /* ATT_OP_WRITE_COMMAND */
#define ATT_OP_WRITE_COMMAND_SIGNED     0xD2    /* ATT_OP_WRITE_COMMAND_SIGNED */


typedef struct tag_attr_op_request {
	UINT16 request_id;
	void *req_param;
} attr_op_request_t;

typedef struct {
	bt_uuid_t app_uuid;
	int server_if;
} app_ble_server_info_t;

static struct ListStru s_attr_op_req_list;
static UINT16 s_req_id;
static app_ble_server_info_t s_server_info[BTGATT_MAX_SERVER_APP_NUM];
static uint8_t * s_elements_data = NULL;
static int s_elements_data_len = 0;
static UINT16 s_cur_srvc_handle = 0;

static void gatts_notification_cfm_cbk(void *context, UINT8 ev, void *arg);
static void gatts_common_service_cbk(void *context, UINT8 ev, void *arg);
static void hlp_gatt_server_save_primary_service_to_local(UINT8 server_if, btgatt_db_element_t *element);
static void hlp_gatt_server_save_secondary_service_to_local(UINT8 server_if, btgatt_db_element_t *element);
static void hlp_gatt_server_save_characteristic_to_local(UINT8 server_if, btgatt_db_element_t *element);
static void hlp_gatt_server_save_include_service_to_local(UINT8 server_if, btgatt_db_element_t *element);
static void hlp_gatt_server_save_descriptor_to_local(UINT8 server_if, btgatt_db_element_t *element);
static void hlp_gatt_server_start_service(UINT8 server_if, UINT16 svc_handle, UINT8 transport);
static void hlp_gatt_server_on_services_added(struct GATT_ServiceStru *service);

//FIXME just for test 
static void FCX_DBG_UUID(bt_uuid_t uuid)
{
	GATT_SERVER_DBG("uuid : [%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X]",
		uuid.uu[0], uuid.uu[1], uuid.uu[2], uuid.uu[3],
		uuid.uu[4], uuid.uu[5], uuid.uu[6], uuid.uu[7],
		uuid.uu[8], uuid.uu[9], uuid.uu[10], uuid.uu[11],
		uuid.uu[12], uuid.uu[13], uuid.uu[14], uuid.uu[15]);
}
/*******************************************************************
  ************************* app management *************************
*******************************************************************/

static int server_info_add(bt_uuid_t app_uuid)
{
	int i = 0;
	for (i = 0; i < BTGATT_MAX_SERVER_APP_NUM; i++) {
		if ((s_server_info[i].server_if != BTGATT_APP_IF_INVALID) &&
			IS_UUID_EQUAL(s_server_info[i].app_uuid, app_uuid)) {
			return BTGATT_APP_IF_INVALID;
		}
	}

	for (i = 0; i < BTGATT_MAX_SERVER_APP_NUM; i++) {
		if (s_server_info[i].server_if == BTGATT_APP_IF_INVALID) {
			s_server_info[i].server_if = i + 1;
			memcpy(&s_server_info[i].app_uuid, &app_uuid, sizeof(bt_uuid_t));
			return i + 1;
		}
	}

	return BTGATT_APP_IF_INVALID;
}

static int app_info_delete_by_server_if(int server_if)
{
	if(server_if > BTGATT_MAX_SERVER_APP_NUM) {
		return BTGATT_APP_IF_INVALID;
	}

	s_server_info[server_if - 1].server_if = BTGATT_APP_IF_INVALID;

	return 0;
}

void gatt_fsm_server_init(btgatt_server_callbacks_t *callback)
{
	if (s_gatt_server_user_data == NULL) {
		s_gatt_server_user_data = (gatt_server_user_data_t *) NEW(sizeof(gatt_server_user_data_t));
		memset(s_gatt_server_user_data, 0, sizeof(gatt_server_user_data_t));
	}

	memset(s_gatt_server_user_data, 0, sizeof(gatt_server_user_data_t));
	s_gatt_server_user_data->cbks = callback;
	GATT_NotificationCbk_Reg(gatts_notification_cfm_cbk);

	int i = 0;
	for (i; i < BTGATT_MAX_SERVER_APP_NUM; i++) {
		memset(&s_server_info[i], 0, sizeof(app_ble_server_info_t));
		s_server_info[i].server_if = BTGATT_APP_IF_INVALID;
	}

	gatt_services_init();

	s_attr_op_req_list.head = s_attr_op_req_list.tail = 0;
	s_req_id = 0;
}

void gatt_fsm_server_done()
{
    GATT_SERVER_DBG("gatt_fsm_server_done");
	GATT_NotificationCbk_UnReg(gatts_notification_cfm_cbk);
	FREE(s_gatt_server_user_data);

	List_RemoveAll(&s_attr_op_req_list);
    s_gatt_server_user_data = NULL;
}

void hlp_gatt_fsm_gatt_server_register_app(void *param)
{	
	int status = BT_STATUS_SUCCESS;
	fsm_msg_gatt_server_register_app_t *msg = (fsm_msg_gatt_server_register_app_t *) param;
	int server_if = server_info_add(msg->app_uuid);

	if (server_if < 0) {
		status = BT_STATUS_FAIL;
	}
	GATT_SERVER_DBG("server_register_app: server_if=%d, status=%d", server_if, status);
	s_gatt_server_user_data->s_cur_server_if = server_if;//FIXME
	//server_registered_cb
	GATT_SERVER_CBACK(server_registered, status, server_if < 0 ? server_if : server_if, msg->app_uuid);
}

void hlp_gatt_fsm_gatt_server_unregister_app(void *param)
{
	fsm_msg_gatt_server_unregister_app_t *msg = (fsm_msg_gatt_server_unregister_app_t *) param;
	//FIXME need disonnect or not ?? Test in phone, the result is to disconnect
	gatt_device_disconnect_by_app_if(msg->server_if, hlp_gatt_fsm_gatt_server_disconnect);

	app_info_delete_by_server_if(msg->server_if);
}

void hlp_gatt_fsm_gatt_server_connect(void *param)
{
	fsm_msg_gatt_server_connect_t *msg = (fsm_msg_gatt_server_connect_t *) param;
	
	struct GAP_CreateConnectionReqStru conn_req;
	gatt_device_t *device = gatt_device_find(&msg->address);

	if (device != NULL) {
		GATT_SERVER_DBG("device[%s] is not disconnected", get_addr_str(msg->address.address));
		return;
	}

	conn_req.addr.cap = HCI_ADDRESS_CAP_LE;
	conn_req.link_type = GAP_LINK_TYPE_LE;
	conn_req.addr.atype = (device != NULL) ? device->addr_type : HCI_OWN_ADDRESS_TYPE_RANDOM_DEVICE_ADDRESS;
	memcpy(conn_req.addr.bd, msg->address.address, BD_ADDR_LEN);
	/* save the device. update "is_direct" "server_if" if already exist in list. */
	gatt_device_add(&msg->address, conn_req.addr.atype, msg->is_direct, msg->server_if);

	GAP_CreateConnectionA(NULL, NULL, &conn_req, NULL);
}

void hlp_gatt_fsm_gatt_server_disconnect(void *param)
{
	fsm_msg_gatt_server_disconnect_t *msg = (fsm_msg_gatt_server_disconnect_t *) param;

	gatt_device_t *device = gatt_device_find(&msg->address);
	if (device != NULL && (device->conn_state == CONNECTION_STATE_CONNECTED)) {
		GATT_SERVER_DBG("To disconnect");
		struct HCI_AddressStru addr;
		addr.cap = HCI_ADDRESS_CAP_LE;
		addr.atype = device->addr_type;
		BLUEDROID_BD_TO_HCI_ADDR_STRU(&msg->address, &addr);

		GATT_TL_Disconnect(&addr, NULL, NULL);
	} else {
		GATT_SERVER_DBG("device is not connected");
	}
}

void hlp_gatt_fsm_gatt_server_set_preferred_phy(void *param)
{
	fsm_msg_gatt_server_set_preferred_phy_t *msg = (fsm_msg_gatt_server_set_preferred_phy_t *) param;
}

void hlp_gatt_fsm_gatt_server_read_phy(void *param)
{
	fsm_msg_gatt_server_read_phy_t *msg = (fsm_msg_gatt_server_read_phy_t *) param;
}

void hlp_gatt_fsm_gatt_server_add_service(void *param)
{
	fsm_msg_gatt_server_add_service_t *msg = (fsm_msg_gatt_server_add_service_t *) param;
	s_gatt_server_user_data->s_cur_server_if = msg->server_if;//FIXME

	GATT_SERVER_DBG("%s, server_if = %d, is_completed = %d, data_len = %d", __func__, msg->server_if, msg->is_completed, msg->svc_data_len);
	
	s_elements_data = (uint8_t *)realloc(s_elements_data, s_elements_data_len + msg->svc_data_len);
	if (msg->is_completed) {
		memcpy(s_elements_data + s_elements_data_len, msg->svc_data, msg->svc_data_len);
		s_elements_data_len += msg->svc_data_len;
		GATT_SERVER_DBG("s_len: %d", s_elements_data_len);
		btgatt_db_element_t * elements = (btgatt_db_element_t *)s_elements_data;
	
		int offset = 0;
		while (elements != NULL) {
			int type = (bt_gatt_db_attribute_type_t)elements->type;
			GATT_SERVER_DBG("id : %d, type : %d, property: %d, permissions : %d", elements->id, type, elements->properties, elements->permissions);
			bt_uuid_t uuid = elements->uuid;
			switch(type) {
				case BTGATT_DB_PRIMARY_SERVICE:
					hlp_gatt_server_save_primary_service_to_local(msg->server_if, elements);
					break;
				case BTGATT_DB_SECONDARY_SERVICE:
					hlp_gatt_server_save_secondary_service_to_local(msg->server_if, elements);
					break;
				case BTGATT_DB_INCLUDED_SERVICE:
					hlp_gatt_server_save_include_service_to_local(msg->server_if, elements);
					break;
				case BTGATT_DB_CHARACTERISTIC:
					hlp_gatt_server_save_characteristic_to_local(msg->server_if, elements);
					break;
				case BTGATT_DB_DESCRIPTOR:
					hlp_gatt_server_save_descriptor_to_local(msg->server_if, elements);
					break;
			}
			elements = elements + 1;
			offset += sizeof(btgatt_db_element_t);


			if (offset >= s_elements_data_len) {
				break;
			}
		}
		GATT_SERVER_DBG("clear s_len");
		s_elements_data_len = 0;
		if (s_elements_data) {
			free(s_elements_data);
			s_elements_data = NULL;
		}
		GATT_SERVER_DBG("To start service");//FIXME start here???
		hlp_gatt_server_start_service(msg->server_if, s_cur_srvc_handle, TRANSPORT_LE);
	} else {//is_completed == 0
		memcpy(s_elements_data + s_elements_data_len, msg->svc_data, msg->svc_data_len);
		s_elements_data_len += msg->svc_data_len;
	}

}

void hlp_gatt_server_save_primary_service_to_local_ext(UINT8 server_if, btgatt_db_element_t *element)
{
	hlp_gatt_server_save_primary_service_to_local(server_if, element);
}
void hlp_gatt_server_save_characteristic_to_local_ext(UINT8 server_if, btgatt_db_element_t *element)
{
	hlp_gatt_server_save_characteristic_to_local(server_if, element);
}
void hlp_gatt_server_save_descriptor_to_local_ext(UINT8 server_if, btgatt_db_element_t *element)
{
	hlp_gatt_server_save_descriptor_to_local(server_if, element);
}


void hlp_gatt_add_all_Service(UINT8 server_if)
{
	hlp_gatt_server_start_service(server_if, s_cur_srvc_handle, TRANSPORT_LE);
}

void hlp_gatt_fsm_gatt_server_disconnect_ext(char *param)
{
	fsm_msg_gatt_server_disconnect_t msg ;
	gatt_device_t *device = NULL;
	memcpy(&msg.address.address, &param,6);

	device = gatt_device_find(&msg.address);
	if (device != NULL && (device->conn_state == CONNECTION_STATE_CONNECTED)) {
		struct HCI_AddressStru addr;
		GATT_SERVER_DBG("To disconnect");

		addr.cap = HCI_ADDRESS_CAP_LE;
		addr.atype = device->addr_type;
		BLUEDROID_BD_TO_HCI_ADDR_STRU(&msg.address, &addr);

		GATT_TL_Disconnect(&addr, NULL, NULL);
	} else {
		GATT_SERVER_DBG("device is not connected");
	}
	
}

void hlp_gatt_fsm_gatt_server_send_response_ext(uint16_t handle,uint16_t trans_id,uint16_t value_len,uint8_t *val)
{	
    GATT_SERVER_DBG("hlp_gatt_fsm_gatt_server_send_response, handle[%d]", handle);
	struct ATT_UUIDStru uuid;
	struct HCI_AddressStru addr;
	struct ATT_PendIndStru *pend_ind = NULL;
	attr_op_request_t *req = s_attr_op_req_list.head;

	while (req) {
		if (req->request_id == trans_id) {
			break;
		}

		req = LNEXT(req);
	}

	if (!req) {
		GATT_SERVER_DBG("Invalid trans_id : %d", trans_id);
		GATT_SERVER_CBACK(response_send_completed, BT_STATUS_FAIL, handle);
		return;
	}

	pend_ind = (struct ATT_PendIndStru *)req->req_param;

	if (value_len>0) {
		struct ATT_ModifyValueStru *in;
		in = (struct ATT_ModifyValueStru *)NEW(sizeof(struct ATT_ModifyValueStru) + value_len);
		in->hdl = pend_ind->it.hdl;
		in->val.len = value_len;
		if (value_len) {
			memcpy(in->val.value, val, value_len);
		}
		//"in" is FREE in stack.
		GATT_Server_ModifyValue(in);
	} 

	if (value_len) {
		memcpy(pend_ind->it.val.value, val, value_len);
	}

	//'pend_ind' is FREE in stack
	GATT_Server_PendIND_Response(pend_ind);

	List_RemoveAt(&s_attr_op_req_list, req);
	LFREE(req);
	GATT_SERVER_CBACK(response_send_completed, BT_STATUS_SUCCESS, handle);
}

void hlp_gatt_fsm_gatt_server_send_notification_ext(uint16_t handle,uint16_t value_len,uint8_t *val)
{
	
    struct GATT_Server_NotifyIndicateStru *in = NEW(sizeof(struct GATT_Server_NotifyIndicateStru) + value_len);
	in->hdl = handle;
	in->val.len = value_len;
	memcpy(in->val.value, val, value_len);

	GATT_Server_NotifyIndicateEx(in);

	//GATT_SERVER_CBACK(notification_sent, msg->conn_id, BT_STATUS_SUCCESS);
}

void hlp_gatt_fsm_gatt_server_send_notification_sp(uint16_t handle,uint16_t value_len,uint8_t *val)
{
	
    struct GATT_Server_NotifyIndicateStru *in = NEW(sizeof(struct GATT_Server_NotifyIndicateStru) + value_len);
	in->hdl = handle;
	in->val.len = value_len;
	memcpy(in->val.value, val, value_len);

    //GATT_Server_Notify_Send(in);
    GATT_Server_NotifyIndicate(in);

	//GATT_SERVER_CBACK(notification_sent, msg->conn_id, BT_STATUS_SUCCESS);
}

void hlp_gatt_fsm_gatt_server_stop_service(void *param)
{
	fsm_msg_gatt_server_stop_service_t *msg = (fsm_msg_gatt_server_stop_service_t *) param;

	gatt_element_srvc_t * svc = gatt_services_get_service_by_stack_handle(msg->server_if, msg->svc_handle);

	if (svc) {
		GATT_Server_UnRegTree(svc->handle);
		GATT_SERVER_CBACK(service_stopped, BT_STATUS_SUCCESS, msg->server_if, msg->svc_handle);
	} else {
		GATT_SERVER_CBACK(service_stopped, BT_STATUS_FAIL, msg->server_if, msg->svc_handle);
	}

}

void hlp_gatt_fsm_gatt_server_delete_service(void *param)
{
	fsm_msg_gatt_server_delete_service_t *msg = (fsm_msg_gatt_server_delete_service_t *) param;

	gatt_element_srvc_t *svc = gatt_services_get_service_by_stack_handle(msg->server_if, s_cur_srvc_handle);
	if (svc) {
		// XXX when to unregister service tree?
		GATT_SERVER_DBG("-----delete service database!");
		GATT_Server_UnRegTree(svc->handle);
		UINT16 app_handle = gatt_services_get_app_handle(s_cur_srvc_handle);
		gatt_services_delete_service(msg->server_if, app_handle);
	}
	GATT_SERVER_CBACK(service_deleted, BT_STATUS_SUCCESS, msg->server_if, s_cur_srvc_handle);

	gatt_device_disconnect_by_app_if(msg->server_if, hlp_gatt_fsm_gatt_server_disconnect);

	app_info_delete_by_server_if(msg->server_if);
}

void hlp_gatt_fsm_gatt_server_send_indication(void *param)
{
	fsm_msg_gatt_server_send_indication_t *msg = (fsm_msg_gatt_server_send_indication_t *) param;
	
    struct GATT_Server_NotifyIndicateStru *in = NEW(sizeof(struct GATT_Server_NotifyIndicateStru) + msg->val_len);
	in->hdl = msg->attr_handle + 1;//chara_handle to value_handle
	in->val.len = msg->val_len;
	memcpy(in->val.value, msg->val, msg->val_len);

	GATT_Server_NotifyIndicate(in);

	GATT_SERVER_CBACK(notification_sent, msg->conn_id, BT_STATUS_SUCCESS);
}

void hlp_gatt_fsm_gatt_server_send_notification(void *param)
{
	fsm_msg_gatt_server_send_notification_t *msg = (fsm_msg_gatt_server_send_notification_t *) param;
	
    struct GATT_Server_NotifyIndicateStru *in = NEW(sizeof(struct GATT_Server_NotifyIndicateStru) + msg->val_len);
	in->hdl = msg->attr_handle + 1;//chara_handle to value_handle
	in->val.len = msg->val_len;
	memcpy(in->val.value, msg->val, msg->val_len);
    

	GATT_Server_NotifyIndicate(in);

	//GATT_SERVER_CBACK(notification_sent, msg->conn_id, BT_STATUS_SUCCESS);
}

void hlp_gatt_fsm_gatt_server_send_response(void *param)
{
	fsm_msg_gatt_server_send_response_t *msg = (fsm_msg_gatt_server_send_response_t *) param;
    GATT_SERVER_DBG("hlp_gatt_fsm_gatt_server_send_response, handle[%d]", msg->handle);

	struct ATT_UUIDStru uuid;
	struct HCI_AddressStru addr;
    struct ATT_PendIndStru *ind = NULL;
    struct ATT_PendIndStru *pend_ind = NULL;
    attr_op_request_t *req = s_attr_op_req_list.head;

	while (req) {
		if (req->request_id == msg->trans_id) {
			break;
		}

		req = LNEXT(req);
	}

	if (!req) {
		GATT_SERVER_DBG("Invalid trans_id : %d", msg->trans_id);
		GATT_SERVER_CBACK(response_send_completed, BT_STATUS_FAIL, msg->handle);
		return;
	}

	ind = (struct ATT_PendIndStru *)req->req_param;

	if (msg->val_len) {
		struct ATT_ModifyValueStru *in;
		in = (struct ATT_ModifyValueStru *)NEW(sizeof(struct ATT_ModifyValueStru) + msg->val_len);
		in->hdl = ind->it.hdl;
		in->val.len = msg->val_len;
 
		memcpy(in->val.value, msg->val, msg->val_len);
             
		GATT_Server_ModifyValue(in);
	} 

	pend_ind = (struct ATT_PendIndStru *) NEW(sizeof(struct ATT_PendIndStru) + msg->val_len);
	memcpy(pend_ind, ind, sizeof(struct ATT_PendIndStru));
    if (msg->val_len) {
		pend_ind->it.offset = msg->offset;
		pend_ind->it.val.len = msg->val_len;
		memcpy(pend_ind->it.val.value, msg->val, msg->val_len);
    }
    //'pend_ind' is FREE in stack
	GATT_Server_PendIND_Response(pend_ind);

	List_RemoveAt(&s_attr_op_req_list, req);
	FREE(req->req_param);
    LFREE(req);
	GATT_SERVER_CBACK(response_send_completed, BT_STATUS_SUCCESS, msg->handle);
}

void hlp_gatt_fsm_gatt_client_scan_filter_add(void *param)
{
	fsm_msg_gatt_client_scan_filter_add_t *msg = (fsm_msg_gatt_client_scan_filter_add_t *) param;
}

void hlp_gatt_fsm_gatt_client_scan_filter_delete(void *param)
{
	fsm_msg_gatt_client_scan_filter_delete_t *msg = (fsm_msg_gatt_client_scan_filter_delete_t *) param;
}

void hlp_gatt_fsm_gatt_client_scan_filter_param_add(void *param)
{
	fsm_msg_gatt_client_scan_filter_param_add_t *msg = (fsm_msg_gatt_client_scan_filter_param_add_t *) param;
}

void hlp_gatt_fsm_gatt_client_scan_filter_param_clear_all(void *param)
{
	fsm_msg_gatt_client_scan_filter_param_clear_all_t *msg = (fsm_msg_gatt_client_scan_filter_param_clear_all_t *) param;
}

void hlp_gatt_fsm_gatt_client_scan_filter_param_delete(void *param)
{
	fsm_msg_gatt_client_scan_filter_param_delete_t *msg = (fsm_msg_gatt_client_scan_filter_param_delete_t *) param;
}

void hlp_gatt_fsm_gatt_client_scan_filter_clear(void *param)
{
	fsm_msg_gatt_client_scan_filter_clear_t *msg = (fsm_msg_gatt_client_scan_filter_clear_t *) param;
}

void hlp_gatt_fsm_gatt_client_scan_filter_enable(void *param)
{
	fsm_msg_gatt_client_scan_filter_enable_t *msg = (fsm_msg_gatt_client_scan_filter_enable_t *) param;
}

void hlp_gatt_fsm_gatt_client_config_batch_scan_storage(void *param)
{
	fsm_msg_gatt_client_config_batch_scan_storage_t *msg = (fsm_msg_gatt_client_config_batch_scan_storage_t *) param;
}

void hlp_gatt_fsm_gatt_client_start_batch_scan(void *param)
{
	fsm_msg_gatt_client_start_batch_scan_t *msg = (fsm_msg_gatt_client_start_batch_scan_t *) param;
}

void hlp_gatt_fsm_gatt_client_stop_batch_scan(void *param)
{
	fsm_msg_gatt_client_stop_batch_scan_t *msg = (fsm_msg_gatt_client_stop_batch_scan_t *) param;
}

void hlp_gatt_fsm_gatt_client_read_scan_reports(void *param)
{
	fsm_msg_gatt_client_read_scan_reports_t *msg = (fsm_msg_gatt_client_read_scan_reports_t *) param;
}

void hlp_gatt_server_on_device_connected(void *bd, uint16_t acl_hdl)
{
	struct HCI_AddressStru *addr = (struct HCI_AddressStru *)bd;
	gatt_device_t *device;
	bt_bdaddr_t address;
	UINT8 conn_state = CONNECTION_STATE_DISCONNECTED;

	HCI_ADDR_STRU_TO_BLUEDROID_BD(addr, &address);
	device = gatt_device_find(&address);
	
	if (device) {
		conn_state = device->conn_state;	
	}

	if (conn_state == CONNECTION_STATE_CONNECTED) {
		GATT_SERVER_DBG("Error, on_device_connected, Another connection with the same device : %s",
			get_addr_str(device->addr.address));	
	} else {
		int i = 0;
		for (i; i < BTGATT_MAX_SERVER_APP_NUM; i++) {
			if (s_server_info[i].server_if != BTGATT_APP_IF_INVALID) {
				uint8_t server_if = s_server_info[i].server_if;
				//TODO compare with s_cur_server_if
				GATT_SERVER_DBG("s_cur_server_if: %d, server_if : %d", s_gatt_server_user_data->s_cur_server_if, server_if);
				//FIXME
				gatt_device_add(&address, addr->atype, 0xFF, server_if);
				GATT_SERVER_CBACK(client_connected, address, 1/*FIXME*/, acl_hdl, server_if);
			}
		}
	}

}

void hlp_gatt_server_on_device_disconnected(void *bd, uint16_t acl_hdl)
{
	struct HCI_AddressStru * addr = (struct HCI_AddressStru *)bd;
	gatt_device_t * device;
	bt_bdaddr_t address;
	UINT8 conn_state = CONNECTION_STATE_DISCONNECTED;

	HCI_ADDR_STRU_TO_BLUEDROID_BD(addr, &address);
	device = gatt_device_find(&address);

	if (device) {
		conn_state = device->conn_state;
	} else {
        GATT_SERVER_DBG("%s, No Device Found", __func__);
        return;
    }

	if (conn_state == CONNECTION_STATE_CONNECTED) {
		int i = 0;
		for (i; i < BTGATT_MAX_SERVER_APP_NUM; i++) {
			if (s_server_info[i].server_if != BTGATT_APP_IF_INVALID) {
				GATT_SERVER_CBACK(client_connected, address, 0/*FIXME*/, acl_hdl, s_server_info[i].server_if);
			}
		}
	} else {
		GATT_SERVER_DBG("Error: on_device_disconnected, connection not existed : %s", get_addr_str(device->addr.address));
	}
}

static UINT8 supportBLEAuthenticationfg = 0;
void hlp_gatt_support_BLE_Authentication(UINT8 ev)
{
	GATT_SERVER_DBG("hlp_gatt_support_BLE_Authentication ev = %d\n",ev);
	supportBLEAuthenticationfg = ev;
}

static void gatts_notification_cfm_cbk(void *context, UINT8 ev, void *arg)
{
	GATT_SERVER_DBG("---------%s- ------- ev = %d", __func__, ev);
	switch (ev) {
	case GATT_EV_MTU: {
		struct GATT_MTUCbkStru *param = (struct GATT_MTUCbkStru *) arg;
		bt_bdaddr_t addr;

		if (param == NULL) {
			return;
		}

		HCI_ADDR_STRU_TO_BLUEDROID_BD(&(param->addr), &addr);
		gatt_device_t *device = gatt_device_find(&addr);

		if (device != NULL) {//server_mtu_changed_cb
			GATT_SERVER_CBACK(server_mtu_changed, device->conn_id, param->mtu);
		}
        GATT_SERVER_DBG("gatts_notification_cfm_cbk, GATT_EV_MTU");
		break;
	}
	case GATT_EV_HDLVALUE_NOTIFY_CONFIRM: {
		struct ATT_Handle_Value_NotifyConfirmStru * param = (struct ATT_Handle_Value_NotifyConfirmStru *) arg;
		//TODO
		break;
	}
	default:
		break;
	}
}

static void hlp_gatt_server_save_primary_service_to_local(UINT8 server_if, btgatt_db_element_t *element)
{
	btgatt_srvc_id_t srv;

	srv.is_primary = element->type;//0 means primary
	srv.id.inst_id = element->attribute_handle;
	memcpy(srv.id.uuid.uu, element->uuid.uu, 16);
    GATT_SERVER_DBG("service uuid[%02x:%02x]",element->uuid.uu[2],element->uuid.uu[3]);
	s_cur_srvc_handle = gatt_services_add_service(server_if, srv);
}

static void hlp_gatt_server_save_secondary_service_to_local(UINT8 server_if, btgatt_db_element_t *element)
{
	btgatt_srvc_id_t srv;

	srv.is_primary = element->type;
	srv.id.inst_id = element->attribute_handle;
	memcpy(srv.id.uuid.uu, element->uuid.uu, 16);

	s_cur_srvc_handle = gatt_services_add_service(server_if, srv);
}

static void hlp_gatt_server_save_include_service_to_local(UINT8 server_if, btgatt_db_element_t *element)
{
	btgatt_srvc_id_t srv;

	srv.is_primary = element->type;
	srv.id.inst_id = element->attribute_handle;
	memcpy(srv.id.uuid.uu, element->uuid.uu, 16);

	//FIXME now element->attribute_handle is real handle in stack
	int incl_app_handle = gatt_services_get_app_handle(element->attribute_handle);
	GATT_SERVER_DBG("element_handle : %d, app_handle", element->attribute_handle, incl_app_handle);
	gatt_services_add_include(server_if, s_cur_srvc_handle, incl_app_handle);
}

static void hlp_gatt_server_save_characteristic_to_local(UINT8 server_if, btgatt_db_element_t *element) 
{
	GATT_SERVER_DBG("element->permissions: %d", element->permissions);
	gatt_services_add_character(server_if, s_cur_srvc_handle, &(element->uuid), element->properties, element->permissions,element->val,element->val_len);	
}

static void hlp_gatt_server_save_descriptor_to_local(UINT8 server_if, btgatt_db_element_t *element)
{
	GATT_SERVER_DBG("element->permissions: %d", element->properties);
	gatt_services_add_descriptor(server_if, s_cur_srvc_handle, &(element->uuid), element->properties);
}

static void hlp_gatt_server_start_service(UINT8 server_if, UINT16 svc_handle, UINT8 transport)
{
	struct GATT_ServiceStru *service = NULL;
	struct GATT_CharacteristicStru *crt;
	struct GATT_IncludeStru *in;

	gatt_element_srvc_t *srvc = NULL;
	gatt_element_incl_t *incl = NULL;
	gatt_element_char_t *c = NULL;
	gatt_element_desc_t *d = NULL;
	struct ATT_UUIDStru uuid;

	// TODO transport not used
	srvc = gatt_services_get_service_by_app_handle(server_if, svc_handle);

	if (srvc) {
		service = (struct GATT_ServiceStru *)List_NodeNew(sizeof(struct GATT_ServiceStru));
		service->cbk = (void *) gatts_common_service_cbk;
		revert_uuid(srvc->uuid.uu, service->uuid.u);
            GATT_SERVER_DBG("---start service uuid[%02x:%02x]",srvc->uuid.uu[2],srvc->uuid.uu[3]);
	
		incl = srvc->include.head;

		while (incl) {
			gatt_element_srvc_t *inc_svc = gatt_services_get_service_by_app_handle(server_if, incl->incl_svc_handle);
			List_AddTail(&service->include, (in = List_NodeNew(sizeof(struct GATT_IncludeStru))));

			if (inc_svc) {
				in->include_hdl = inc_svc->handle;
				in->endhdl = inc_svc->end_handle;
				revert_uuid(inc_svc->uuid.uu, in->uuid.u);
			}

			incl = LNEXT(incl);
		}

		c = srvc->character.head;

		while (c) {
			revert_uuid(c->uuid.uu, uuid.u);
			if(supportBLEAuthenticationfg){
				GATT_SERVER_DBG("supportBLEAuthenticationfg1 = %d\n",supportBLEAuthenticationfg);
				crt = GATT_Add_Characteristic(service, c->val, c->val_len, &uuid, c->properties | ATT_AUTHENTICATION_NEED );
			}else{
				crt = GATT_Add_Characteristic(service, c->val, c->val_len, &uuid, c->properties);
			}
			//crt = GATT_Add_Characteristic(service, NULL, 0, &uuid, c->properties |ATT_AUTHENTICATION_NEED );
          //GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE|ATT_AUTHENTICATION_NEED);
			d = c->descriptor.head;
            		GATT_SERVER_DBG("add characteristic----------");
			while(d) {
				struct GATT_DescriptorStru *desc;
				char CCCUuid[16] = {0x00,0x00,0x29,0x02, 0x00,0x00, 0x10,0x00, 0x80,0x00, 0x00,0x80,0x5F,0x9B,0x34,0xFb};
				char clcast_ccc_uuid[16] = {0x00,0x00,0x29,0x02, 0x00,0x00, 0x10,0x00, 0x80,0x00, 0xc9,0xef,0xde,0xfd,0xef,0x9f};
				char RRUuid[16] = {0x00,0x00,0x29,0x08, 0x00,0x00, 0x10,0x00, 0x80,0x00, 0x00,0x80,0x5F,0x9B,0x34,0xFb};
				char ec_eir_uuid[16] ={0x00, 0x00, 0x29, 0x02, 0xd6, 0xd8, 0xc7, 0xec, 0xbd, 0xf0, 0xea, 0xb1, 0xbf, 0xc6, 0xbc, 0xbc};
				char ey_eir_uuid[16] ={0x00, 0x00, 0xaa, 0xa0, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};
                		if(memcmp(d->uuid.uu,ec_eir_uuid,16) == 0){
					GATT_SERVER_DBG("add CCC to characteristic----------");
					GATT_SERVER_DBG("supportBLEAuthenticationfg1 = %d\n",supportBLEAuthenticationfg);
					if(supportBLEAuthenticationfg){
						GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE | ATT_AUTHENTICATION_NEED );
					}else{
						GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE);
					}
					//GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE |ATT_AUTHENTICATION_NEED);
				} else if(memcmp(d->uuid.uu,ey_eir_uuid,16) == 0){
					if(supportBLEAuthenticationfg){
						GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE | ATT_WRITE_NORSP |ATT_AUTHENTICATION_NEED );
					}else{
						//GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE|ATT_WRITE_NORSP);
						GATT_Add_Descriptor_CCC(crt, 0,ATT_READ|ATT_WRITE|ATT_WRITE_NORSP);
					}					
				} else if(memcmp(d->uuid.uu,CCCUuid,16) == 0 || memcmp(d->uuid.uu,clcast_ccc_uuid,16) == 0){
					GATT_SERVER_DBG("add CCC to characteristic----------");
					GATT_SERVER_DBG("supportBLEAuthenticationfg2 = %d\n",supportBLEAuthenticationfg);
					if(supportBLEAuthenticationfg){
						GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE | ATT_AUTHENTICATION_NEED );
					}else{
						GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE);
					}
					//GATT_Add_Descriptor_CCC(crt, ATT_CCCMASK_NOTIFICATION,ATT_READ|ATT_WRITE |ATT_AUTHENTICATION_NEED);
				} else if(memcmp(d->uuid.uu,RRUuid,16) == 0){
					GATT_SERVER_DBG("add RR to characteristic1----------");
					GATT_SERVER_DBG("supportBLEAuthenticationfg3 = %d\n",supportBLEAuthenticationfg);
					if(supportBLEAuthenticationfg){
						GATT_Add_Descriptor_ReportReference(crt, 0x03,0x01,ATT_READ | ATT_AUTHENTICATION_NEED );
					}else{
						GATT_Add_Descriptor_ReportReference(crt, 0x03,0x01,ATT_READ );
					}
					//GATT_Add_Descriptor_ReportReference(crt, 0x03,0x01,ATT_READ|ATT_AUTHENTICATION_NEED);
				}else{
					List_AddTail(&crt->descriptor, (desc = List_NodeNew(sizeof(struct GATT_DescriptorStru))));
					revert_uuid(d->uuid.uu, desc->uuid.u);
					
					desc->properties = d->permissions;
					GATT_SERVER_DBG(" desc->properties----------%d",desc->properties);
					// FIXME
					desc->val = NEW(sizeof(struct ATT_ValueStru) + 1);
					#if 1
					desc->val->value[0] = 0x03;
					desc->val->value[1] = 0x01;
					desc->val->len = 2;/*= 2 ????*/
					#else
					desc->val->len = 1;/*= 2 ????*/
					#endif
				}
			
				d = LNEXT(d);
			}

			c = LNEXT(c);
		}

		GATT_Server_RegTree(service);
	}
}

static void gatts_common_service_cbk(void *context, UINT8 ev, void *arg)
{
	GATT_SERVER_DBG("%s, ev ; %d", __func__, ev);
	switch (ev) {
		case GATT_EV_HDL_TREE: {
			struct GATT_ServiceStru * service = (struct GATT_ServiceStru *) arg;
			gatt_services_update_handles(service);
			GATT_SERVER_DBG("hlp_gatt_dbg_services");
			hlp_gatt_dbg_services(service);
			hlp_gatt_server_on_services_added(service);
			GATT_Free_Service(service);
			arg = NULL;
			break;
		}

		case GATT_EV_PEND_IND: {
			bt_bdaddr_t bda;
			gatt_device_t * dev;
			int conn_id = 0;
			UINT16 app_handle;
			bt_status_t status;
			UINT8 is_long = 0;
			UINT8 is_prep = 0;
			UINT8 need_rsp = 1; // FIXME what type of write request should response?
			struct ATT_PendIndStru *pend_ind = (struct ATT_PendIndStru *) arg;
			struct ATT_ValueStru *val = &(pend_ind->it.val);
			attr_op_request_t *req;
	
			//GATT_Server_PendIND_Response(pend_ind);
			memcpy(bda.address, pend_ind->addr.bd, 6);
			dev = gatt_device_find(&bda);
			status = (pend_ind->errcode == ATT_ERR_OK) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
			is_long = (pend_ind->op == ATT_OP_READ_BLOB_REQ)
			          || (pend_ind->op == ATT_OP_READ_MULTIPLE_REQ);
			is_prep = (pend_ind->op == ATT_OP_PREPARE_WRITE_REQ);
	
			if (dev) {
				conn_id = dev->conn_id;
			}
	
			List_AddTail(&s_attr_op_req_list, (req = List_NodeNew(sizeof(attr_op_request_t))));
			req->request_id = ++s_req_id;
			req->req_param = pend_ind;
			app_handle = gatt_services_get_app_handle(pend_ind->it.hdl);
	
			// This Attribute of this request is NOT created by App. Ignore it!
			if (!app_handle) {
                            GATT_SERVER_DBG("***********handle is null!");
				GATT_Server_PendIND_Response(pend_ind);
				return;
			}
	                GATT_SERVER_DBG("***********1, app_handle:0x%04X, pend_ind->it.hdl:%04X", app_handle, pend_ind->it.hdl);
			int attribute_type = gatt_services_get_attribute_type_by_stack_handle(pend_ind->it.hdl);
			switch (pend_ind->op) {
				case ATT_OP_READ_REQ:
				case ATT_OP_READ_BLOB_REQ:
				case ATT_OP_READ_MULTIPLE_REQ: {
					switch (attribute_type) {
						case BTGATT_DB_CHARACTERISTIC:
                            GATT_SERVER_DBG("***********2");
							GATT_SERVER_CBACK(server_read_characteristic, bda, conn_id, req->request_id, pend_ind->it.hdl/*value_handle to chara_handle*/, pend_ind->it.offset, is_long);
							break;
						case BTGATT_DB_DESCRIPTOR:
                            GATT_SERVER_DBG("***********3");
							GATT_SERVER_CBACK(server_read_descriptor, bda, conn_id, req->request_id, pend_ind->it.hdl, pend_ind->it.offset, is_long);
							break;
						default:
							GATT_SERVER_DBG("ERROR !!");
							break;
					}
					break;
				}
				case ATT_OP_WRITE_REQ:
				case ATT_OP_WRITE_COMMAND:
				case ATT_OP_WRITE_COMMAND_SIGNED:
				case ATT_OP_PREPARE_WRITE_REQ:
				case ATT_OP_EXECUTE_WRITE_REQ: {
					switch (attribute_type) {
						case BTGATT_DB_CHARACTERISTIC:
                            GATT_SERVER_DBG("***********4");//server_write_characteristic_cb
							GATT_SERVER_CBACK(server_write_characteristic, bda, conn_id, req->request_id, pend_ind->it.hdl /*value_handle to chara_handle*/, pend_ind->it.offset, val->len, need_rsp, is_prep, val->len, val->value);
							break;
						case BTGATT_DB_DESCRIPTOR:
                            GATT_SERVER_DBG("***********5");
							GATT_SERVER_CBACK(server_write_descriptor, bda, conn_id, req->request_id, pend_ind->it.hdl, pend_ind->it.offset, val->len, need_rsp, is_prep, val->len, val->value);
							break;
						default:
							GATT_SERVER_DBG("ERROR !!!");
							break;
					}
					break;
				}
				default:
					break;
			}
			arg = NULL;
			break;
		}

		case GATT_EV_EXECUTE_WRITE: {
			bt_bdaddr_t bda;
			gatt_device_t * dev;
			int conn_id = 0;
			struct ATT_ExecuteWriteCbkStru * param = (struct ATT_ExecuteWriteCbkStru *) arg;
			attr_op_request_t *req;
	
			memcpy(bda.address, param->addr.bd, 6);
			dev = gatt_device_find(&bda);
	
			if (dev) {
				conn_id = dev->conn_id;
			}
	
			List_AddTail(&s_attr_op_req_list, (req = List_NodeNew(sizeof(attr_op_request_t))));
			req->request_id = ++s_req_id;
			req->req_param = param;
			GATT_SERVER_CBACK(execute_write, bda, conn_id, req->request_id, param->flag == ATT_EWRF_IMMEDIATELY_WRITE_ALL);
			break;
		}

		default:
			break;
	}
	if (arg) {
		FREE(arg);
	}
}

static void hlp_gatt_server_fill_db_element(btgatt_db_element_t * elements, bt_uuid_t uuid, bt_gatt_db_attribute_type_t type, UINT16 handle, UINT16 end_handle, UINT8 properties, UINT16 permissions)
{
	memset(elements, 0, sizeof(btgatt_db_element_t));
	elements->id = 0;//FIXME
	memcpy(elements->uuid.uu, uuid.uu, 16);
	elements->type = type;
	elements->attribute_handle = handle;
	switch (type) {
		case BTGATT_DB_PRIMARY_SERVICE:
		case BTGATT_DB_SECONDARY_SERVICE:
			elements->start_handle = handle;
			elements->end_handle = end_handle;
			//has no properties and permissions
			break;
		case BTGATT_DB_CHARACTERISTIC:
			//has no start and end handle
			elements->properties = properties;
			elements->permissions = permissions;
                    GATT_SERVER_DBG("------handle(%d)",handle);
			break;
		case BTGATT_DB_DESCRIPTOR:
			//has no start_handle end_handle and properties
			elements->permissions = permissions;
			break;
	}
}

//FIXME
#define EVENT_BUF_LEN			512
#define MAX_CBACK_DATA_LEN		(EVENT_BUF_LEN / 2)
//no included service contained in this callback
static void hlp_gatt_server_on_services_added(struct GATT_ServiceStru *service)
{
	int num = 0;
	gatt_element_srvc_t *srvc = NULL;
	gatt_element_char_t *crt = NULL;
	gatt_element_desc_t *dsc = NULL;
	btgatt_db_element_t *elements = NULL;
	btgatt_db_element_t element_tmp;

	srvc = (gatt_element_srvc_t *)gatt_services_find_service(service);
	element_tmp.type = BTGATT_DB_SERVER_START;
	GATT_SERVER_CBACK(service_added,&element_tmp);
	if (srvc) {
		num++;
		elements = (btgatt_db_element_t *)realloc(elements, sizeof(btgatt_db_element_t));
		hlp_gatt_server_fill_db_element(elements, srvc->uuid, srvc->svc_type, srvc->handle, srvc->end_handle, 0, 0);
		GATT_SERVER_CBACK(service_added,elements);
		crt = srvc->character.head;
		while (crt) {
			num++;
			elements = (btgatt_db_element_t *)realloc(elements, sizeof(btgatt_db_element_t) * num);
			hlp_gatt_server_fill_db_element(elements + num - 1, crt->uuid, BTGATT_DB_CHARACTERISTIC, crt->handle, 0, crt->properties, crt->permissions);
			GATT_SERVER_CBACK(service_added,elements + num - 1);
			dsc = crt->descriptor.head;
			while (dsc) {
				num++;
				elements = (btgatt_db_element_t *)realloc(elements, sizeof(btgatt_db_element_t) * num);
				hlp_gatt_server_fill_db_element(elements + num - 1, dsc->uuid, BTGATT_DB_DESCRIPTOR, dsc->handle, 0, 0, dsc->permissions);
				GATT_SERVER_CBACK(service_added,elements + num - 1);
				dsc = LNEXT(dsc);
			}

			crt = LNEXT(crt);
		}
	}
	element_tmp.type = BTGATT_DB_SERVER_END;
	GATT_SERVER_CBACK(service_added,&element_tmp);
	//callback
	int total_data_len = num * sizeof(btgatt_db_element_t);
	GATT_SERVER_DBG("element_num : %d, elements_size: %d", num, num * sizeof (btgatt_db_element_t));

	int status = BT_STATUS_SUCCESS;
	if (elements == NULL) {
		status = BT_STATUS_FAIL;
		//GATT_SERVER_CBACK(service_added, status, s_gatt_server_user_data->s_cur_server_if, 0,1, 0, NULL);
		return;
	}

	int remain_data_len = total_data_len;
	UINT8 *p = (UINT8 *)elements;

	while (remain_data_len >= MAX_CBACK_DATA_LEN) {
		UINT8 * data = (UINT8 *)malloc(MAX_CBACK_DATA_LEN);
		memcpy(data, p, MAX_CBACK_DATA_LEN);
            //service_added_cb
		//GATT_SERVER_CBACK(service_added, status, s_gatt_server_user_data->s_cur_server_if, service->hdl,0/*not completed*/, MAX_CBACK_DATA_LEN, data);

		p += MAX_CBACK_DATA_LEN;
		remain_data_len -= MAX_CBACK_DATA_LEN;
		free(data);
		
	} 

	if (remain_data_len >= 0) {
		UINT8 * data = (UINT8 *)malloc(remain_data_len);
		memcpy(data, p, remain_data_len);
		//GATT_SERVER_CBACK(service_added, status, s_gatt_server_user_data->s_cur_server_if,service->hdl,1/*is_completed*/, remain_data_len, data);
		free(data);
	}

	if (elements) {
		free(elements);
	}
}

