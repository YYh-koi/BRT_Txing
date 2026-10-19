#include "bt_utils.h"
#include "hci_ui.h"
#include "internal_gatt_fsm_client.h"
#include "internal_gatt_conn.h"
#include "internal_gatt_device.h"
#include "internal_gatt_utils.h"
#include "auto_gatt_fsm.h"
#include "att_ui.h"
#include "gap_ui.h"
#include "l2cap_ui.h"
#include "hci_le.h"

#define TAG "bse_gatt_fsm_client"
#include "brt_trace.h"
#define GATT_CLIENT_DBG(fmt, ...) bse_log_print(TAG, "\r\n", fmt, ## __VA_ARGS__);

#define HAL_CBACK(P_CB, P_CBACK, ...)           \
    if (P_CB && P_CB->P_CBACK) {                \
        GATT_CLIENT_DBG("HAL %s->%s", #P_CB, #P_CBACK); \
        P_CB->P_CBACK(__VA_ARGS__);             \
    }                                           \
    else {                                      \
        GATT_CLIENT_DBG("%s", "Callback is NULL"); \
    }

typedef struct gatt_client_user_data_tag {
	btgatt_client_callbacks_t *cbks;
} gatt_client_user_data_t;

static gatt_client_user_data_t *s_gatt_client_user_data = NULL;
#define GATT_CLIENT_CBACK(P_CBACK, ...)	\
	if (s_gatt_client_user_data != NULL) { \
		HAL_CBACK((s_gatt_client_user_data->cbks), P_CBACK, __VA_ARGS__) \
	}

static void hlp_gatt_notifi_callback(void *context, UINT8 ev, void *arg);
static void hlp_gatt_disconnect(app_ble_conn_info_t *conn);
static void hlp_gatt_rw_attribute_callback(void *context, UINT8 ev, void *arg);
static void hlp_gatt_register_notifi_callback(void *context, UINT8 ev, void *arg);
static void hlp_gatt_search_services_callback(void *context, UINT8 ev, void *arg);
static void hlp_gatt_configure_mtu_callback(void *context, UINT8 ev, void *arg);

typedef struct {
	bt_uuid_t app_uuid;
	int client_if;
} app_ble_app_info_t;

static app_ble_app_info_t s_app_info[BTGATT_MAX_CLIENT_APP_NUM];

#define APP_BLE_GENERAL_CONTEXT (0xAB)
#define IS_UUID_EQUAL(uuida, uuidb) ( !memcmp(uuida.uu, uuidb.uu, 16))

/*******************************************************************
  ************************* app management *************************
*******************************************************************/
static int app_info_add(bt_uuid_t app_uuid)
{
	int i = 0;
	for(i = 0; i < BTGATT_MAX_CLIENT_APP_NUM; i++) {
		if((s_app_info[i].client_if != BTGATT_APP_IF_INVALID) &&
		   IS_UUID_EQUAL(s_app_info[i].app_uuid, app_uuid)) {
			return BTGATT_APP_IF_INVALID;
		}
	}

	for(i = 0; i < BTGATT_MAX_CLIENT_APP_NUM; i++) {
		if(s_app_info[i].client_if == BTGATT_APP_IF_INVALID) {
			s_app_info[i].client_if = i + 1;
			s_app_info[i].app_uuid = app_uuid;
			return i + 1;
		}
	}
	return BTGATT_APP_IF_INVALID;
}

static int app_info_delete_by_client_if(int client_if)
{
	if(client_if > BTGATT_MAX_CLIENT_APP_NUM) {
		return BTGATT_APP_IF_INVALID;
	}

	s_app_info[client_if - 1].client_if = BTGATT_APP_IF_INVALID;

	return 0;
}

void gatt_fsm_client_init(btgatt_client_callbacks_t *callback)
{
	int i;
	if (s_gatt_client_user_data == NULL) {
		s_gatt_client_user_data = (gatt_client_user_data_t*) NEW(sizeof(gatt_client_user_data_t));
		memset(s_gatt_client_user_data, 0, sizeof(gatt_client_user_data_t));
	}

	memset(s_gatt_client_user_data, 0, sizeof(gatt_client_user_data_t));
	s_gatt_client_user_data->cbks = callback;
	GATT_NotificationCbk_Reg(hlp_gatt_notifi_callback);

	for(i = 0; i < BTGATT_MAX_CLIENT_APP_NUM; i++) {
		s_app_info[i].client_if = BTGATT_APP_IF_INVALID;
	}

	conn_info_init();
}

void gatt_fsm_client_done()
{
	/* TODO disconnect all connections */
	GATT_NotificationCbk_UnReg(hlp_gatt_notifi_callback);
	FREE(s_gatt_client_user_data);
	s_gatt_client_user_data = NULL;
}

void hlp_gatt_fsm_gatt_test(void *param)
{
	fsm_msg_gatt_test_t *msg = (fsm_msg_gatt_test_t *) param;
	// TODO
}

void hlp_gatt_fsm_gatt_client_get_device_type(void *param)
{
	fsm_msg_gatt_client_get_device_type_t *msg = (fsm_msg_gatt_client_get_device_type_t *) param;
	// TODO
}

void hlp_gatt_fsm_gatt_client_register_app(void *param)
{
	int i;
	int status = BT_STATUS_SUCCESS;
	fsm_msg_gatt_client_register_app_t *msg = (fsm_msg_gatt_client_register_app_t *) param;
	int client_if = app_info_add(msg->uuid);

	if (client_if < 0) {
		status = BT_STATUS_FAIL;
	}
	GATT_CLIENT_DBG("client_register_app: client_if=%d, status=%d", client_if, status);
	GATT_CLIENT_CBACK(client_registered, status,
	        client_if < 0 ? client_if : CLIENT_IF_TO_APP(client_if), msg->uuid);
}

void hlp_gatt_fsm_gatt_client_unregister_app(void *param)
{
	fsm_msg_gatt_client_unregister_app_t *msg = (fsm_msg_gatt_client_unregister_app_t *) param;
	UINT8 client_if = CLIENT_IF_TO_LOWER_LAYER(msg->client_if);

	gatt_conn_iterator_by_client_if(client_if, hlp_gatt_disconnect);
	app_info_delete_by_client_if(client_if);
}

static void GATTS_SetAdvData()
{
	UINT8 len;
	struct ListStru adlist;
	struct HCI_EIR_DataNodeStru *node;
	char *local_name = "BRT BLE";

	struct HCI_LE_Set_Advertising_DataStru *in =
	    (struct HCI_LE_Set_Advertising_DataStru *) NEW(sizeof(struct HCI_LE_Set_Advertising_DataStru));
	memset(in, 0, sizeof( struct HCI_LE_Set_Advertising_DataStru));
	memset(&adlist, 0, sizeof( struct ListStru));

	/* AD_Flags */
	node =(struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof( struct HCI_EIR_DataNodeStru));
	node->len = 1 + 1;
	node->eir_data_type = HCI_EIR_DATATYPE_FLAGS;
	node->u.flags = HCI_EIR_FLAGS_LE_GENERAL_DISCOVERABLE_MODE;
	node->u.flags |= HCI_EIR_FLAGS_BREDR_NOT_SUPPORTED;
	List_AddTail(&adlist, node);

	/* Service UUIDs */
	node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct HCI_EIR_DataNodeStru));
	node->len = 1 + 2;
	node->eir_data_type = HCI_EIR_DATATYPE_COMPLETE_LIST_OF_16BIT_SERVICE_CLASS_UUIDS;
	ENCODE2BYTE_LITTLE(&node->u.uuid16[0], 0xff00);
	List_AddTail(&adlist, node);

	/* Local Name */
	len = strlen(local_name);
	node =(struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof(struct  HCI_EIR_DataNodeStru) + len);
	node->len = 1 + len;
	node->eir_data_type = HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME;
	memcpy(node->u.name.data, local_name, len);
	List_AddTail(&adlist, node);

	/* Appearance */
	node = (struct HCI_EIR_DataNodeStru *)List_NodeNew(sizeof( struct HCI_EIR_DataNodeStru));
	node->len = 1 + 2;
	node->eir_data_type = HCI_EIR_DATATYPE_APPEARANCE;
	ENCODE2BYTE_LITTLE(node->u.appearence, 128);
	List_AddTail(&adlist, node);

	in->advertising_data_length = HCI_Extended_Inquiry_Response_Encode(&adlist, in->advertising_data, in->advertising_data + 31);

	GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_LE_SET_ADVERTISING_DATA, in, sizeof(*in), NULL);
}

void hlp_gatt_fsm_gatt_client_connect(void *param)
{
	fsm_msg_gatt_client_connect_t *msg = (fsm_msg_gatt_client_connect_t *) param;
	int client_if;
	bt_bdaddr_t bd_addr;
	struct HCI_AddressStru address;
	unsigned char is_direct;
	struct GAP_CreateConnectionReqStru conn_req;
	gatt_device_t *device = NULL;

	/* XXX
	int transport;
	uint8_t opportunistic;
	int initiating_phys;*/
	client_if = msg->client_if;
	bd_addr = msg->address;
	is_direct = msg->is_direct;

    GATT_CLIENT_DBG("[gatt]hlp_gatt_fsm_gatt_client_connect,dev_bd[%02X:%02X:%02X:%02X:%02X:%02X],msg->client_if[%d]", 
        bd_addr.address[5], bd_addr.address[4],bd_addr.address[3],bd_addr.address[2],bd_addr.address[1],bd_addr.address[0], msg->client_if);

	device = gatt_device_find(&bd_addr);

	if (device != NULL && device->conn_state == CONNECTION_STATE_CONNECTED) {
		GATT_CLIENT_DBG("device[%s] is not disconnected", get_addr_str(bd_addr.address));

		BLUEDROID_BD_TO_HCI_ADDR_STRU(&bd_addr, &address);
		GATT_CLIENT_DBG("HANDLE client : %d", device->conn_id);

		address.cap = HCI_ADDRESS_CAP_LE;
		address.atype = HCI_OWN_ADDRESS_TYPE_RANDOM_DEVICE_ADDRESS;
		conn_info_on_connected(client_if, &address, device->conn_id);

		gatt_device_add(&bd_addr, HCI_OWN_ADDRESS_TYPE_RANDOM_DEVICE_ADDRESS, is_direct, client_if);

		GATT_CLIENT_CBACK(connected, client_if, device->conn_id, BT_STATUS_SUCCESS, msg->address);
		return;
	}

	conn_req.addr.cap = HCI_ADDRESS_CAP_LE;
	conn_req.link_type = GAP_LINK_TYPE_LE;
	conn_req.addr.atype = (device != NULL) ? device->addr_type : HCI_OWN_ADDRESS_TYPE_RANDOM_DEVICE_ADDRESS;
        GATT_CLIENT_DBG("conn_req.atype = %d", conn_req.addr.atype);
	memcpy(conn_req.addr.bd, bd_addr.address, BD_ADDR_LEN);
	GATT_CLIENT_DBG("%s client if 0x%x", __func__, msg->client_if);
	gatt_device_add(&bd_addr, conn_req.addr.atype, is_direct, client_if);

	GAP_CreateConnectionA(NULL, (void *) APP_BLE_GENERAL_CONTEXT, &conn_req, NULL);
}

void hlp_gatt_fsm_gatt_client_disconnect(void *param)
{
	fsm_msg_gatt_client_disconnect_t *msg = (fsm_msg_gatt_client_disconnect_t *) param;
	int client_if;
	bt_bdaddr_t *bd_addr;
	int conn_id;
	app_ble_conn_info_t * conn_info = NULL;

	client_if = CLIENT_IF_TO_LOWER_LAYER(msg->client_if);
	conn_id = msg->conn_id;
	bd_addr = &(msg->address);
	conn_info = conn_info_get_by_addr(bd_addr);

	if (conn_info != NULL && conn_info->conn_id == conn_id) {
		/* do disconnect */
		GATT_TL_Disconnect(&(conn_info->dev_addr), NULL, NULL);
	} else {
		/* already disconnected */
		GATT_CLIENT_DBG("alrealy disconnected");
	}
}

void hlp_gatt_fsm_gatt_client_set_preferred_phy(void *param)
{
	fsm_msg_gatt_client_set_preferred_phy_t *msg = (fsm_msg_gatt_client_set_preferred_phy_t *) param;
	/* TODO */
}

void hlp_gatt_fsm_gatt_client_read_phy(void *param)
{
	fsm_msg_gatt_client_read_phy_t *msg = (fsm_msg_gatt_client_read_phy_t *) param;
	/* TODO */
}

void hlp_gatt_fsm_gatt_client_refresh(void *param)
{
	fsm_msg_gatt_client_refresh_t *msg = (fsm_msg_gatt_client_refresh_t *) param;
	/* TODO */
}

void hlp_gatt_fsm_gatt_client_search_service(void *param)
{
	fsm_msg_gatt_client_search_service_t *msg = (fsm_msg_gatt_client_search_service_t *) param;
	GATT_CLIENT_DBG("msg->conn_id:%d.", msg->conn_id);

	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);
	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		GATT_CLIENT_DBG("%s failed. conn_info state:%s.", __func__, conn_info == NULL ? "null" : conn_info->conn_state == CONNECTION_STATE_CONNECTED ? "connected" : "disconnected");
		return;
	}

	if (msg->search_all) {
		GATT_CLIENT_DBG("%s search all", __func__);
		// FIXME all "GET" command should fill the "tlin" param.
		struct GATT_TL_ConnectStru * tlin = &(conn_info->gatt_tl);
		tlin->cbk = (void *) hlp_gatt_search_services_callback;
		tlin->context = (void *)(intptr_t) conn_info->conn_id;

		tlin->require = GATT_TLREQUIRE_LE;/* FIXME, auto */
		tlin->sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1;
		tlin->timeout = 0xFFFFFFFF;/* Disconnect mannuly(keep link), 0 for disconnect now, 10~60000 for idle timeout */
		memcpy(&(tlin->addr), &(conn_info->dev_addr), sizeof(struct HCI_AddressStru));

		/* FIXME re-search? */
		conn_info->service_list.head = NULL;
		conn_info->service_list.tail = NULL;
		GATT_Discover_Services(tlin, NULL);
	} else {
		/* Discovers a service by UUID.
		 * This is exposed only for passing PTS tests.
		 * It should never be used by real applications. The service is not searched
		 * for characteristics and descriptors, or returned in any callback.*/
		 GATT_CLIENT_DBG("%s, %s", __func__, "search specified uuid service");
	}
}

void hlp_gatt_fsm_gatt_client_discover_service_by_uuid(void *param)
{
	fsm_msg_gatt_client_discover_service_by_uuid_t *msg = (fsm_msg_gatt_client_discover_service_by_uuid_t *) param;
	struct ATT_UUIDStru uuid;

	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);
	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		return;
	}

	struct GATT_TL_ConnectStru * tlin = &(conn_info->gatt_tl);
	tlin->cbk = (void *) hlp_gatt_search_services_callback;
	tlin->context = (void *)(intptr_t) conn_info->conn_id;

	tlin->require = GATT_TLREQUIRE_LE;/* FIXME, auto */
	tlin->sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1;
	tlin->timeout = 0xFFFFFFFF;/* Disconnect mannuly(keep link), 0 for disconnect now, 10~60000 for idle timeout */
	memcpy(&(tlin->addr), &(conn_info->dev_addr), sizeof(struct HCI_AddressStru));
	memcpy(uuid.u, msg->service_uuid.uu, 16);
	GATT_Discover_Services(tlin, &uuid);
}

/* Maximum AT event buffer is 256(512 if CONFIG_UNLIMIT_RESOURCE)
 * raw data convert to AT event, the size maybe double.
 * TODO raw_bytes_to_str_bytes to get the actual size.
 */
/* KCZ%04X%02X%04X -> 3 + 4 + 2 + 4 */
#define AT_EVENT_HEADER_SIZE 12
#define AT_EVENT_MAX_SIZE 512
#define GATT_DB_BUF_SIZE (AT_EVENT_MAX_SIZE / 2 - AT_EVENT_HEADER_SIZE)

static int check_gatt_db_buffer(uint16_t conn_id, int offset, int addition, uint8_t *data)
{
	if (offset + addition >= GATT_DB_BUF_SIZE) {
		GATT_CLIENT_CBACK(get_gatt_db, conn_id, 0, offset, data);
		return 0;
	}

	return offset;
}

void hlp_gatt_fsm_gatt_client_get_gatt_db(void *param)
{
	uint8_t is_completed = 0;
	uint8_t data[GATT_DB_BUF_SIZE];
	int offset = 0;
	int size = 0;
	uint16_t *ptr;
	fsm_msg_gatt_client_get_gatt_db_t *msg = (fsm_msg_gatt_client_get_gatt_db_t *) param;
	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);

	
	if((NULL != conn_info) && (conn_info->conn_state == CONNECTION_STATE_CONNECTED)) {
		struct GATT_ServiceStru * srv = NULL;
		struct GATT_IncludeStru *inc_srv = NULL;
		struct GATT_CharacteristicStru *c = NULL;
		struct GATT_DescriptorStru * d = NULL;
		
		size = 0; /* service number */
		srv = conn_info->service_list.head;
		while(srv) {
			srv = LNEXT(srv);
			size++; 
		}
		ptr = (uint16_t *)(data + offset);
		*ptr = size;
		offset += 2;

		size = 0;
		srv = conn_info->service_list.head;
		while(srv) {
			/**************************Calculate the total size of current service ***************************/
			size = 4 + 16; /* service declaration */
			size += 2; /* include services number */
			inc_srv = srv->include.head;
			while (inc_srv) {
				size += 22; /* Include service declaration: 2 + 2 + 2 + 16 */
				inc_srv = LNEXT(inc_srv);
			}

			size += 2; /* character number */
			c = srv->character.head;
			while(c) {
				size += 21; /* Character declaration: 2 + 2 + 16 + 1*/
				size += 2; /* descriptor number */

				d = c->descriptor.head;
				while(d) {
					size += 18; /* Descriptor declaration: 2 + 16 */
					d = LNEXT(d);
				}
				c = LNEXT(c);
			}

			offset = check_gatt_db_buffer(msg->conn_id, offset, 2, data);
			ptr = (uint16_t *)(data + offset);
			*ptr = size;
			offset += 2;

			/**************************fill the data of current service ***************************/
			offset = check_gatt_db_buffer(msg->conn_id, offset, 20, data);
			/* Service : 4 + 16 */
			ptr = (uint16_t *)(data + offset);
			*ptr++ = srv->hdl;
			*ptr++ = srv->endhdl;
			offset += 4;
			/* Service UUID (Attribute value) */
			memcpy(data + offset, srv->uuid.u, 16);
			offset += 16;

			offset = check_gatt_db_buffer(msg->conn_id, offset, 2, data);
			/* Include service number */
			inc_srv = srv->include.head;
			size = 0;
			while (inc_srv) {
				inc_srv = LNEXT(inc_srv);
				size++;
			}
			ptr = (uint16_t *)(data + offset);
			*ptr = size;
			offset += 2;

			inc_srv = srv->include.head;
			while (inc_srv) {
				offset = check_gatt_db_buffer(msg->conn_id, offset, 22, data);
				/* Include service: 2 + 2 + 2 + 16 */
				ptr = (uint16_t *)(data + offset);
				*ptr++ = inc_srv->hdl;
				*ptr++ = inc_srv->include_hdl;
				*ptr++ = inc_srv->endhdl;
				offset += 6;
				memcpy(data + offset, inc_srv->uuid.u, 16);
				offset += 16;

				inc_srv = LNEXT(inc_srv);
			}

			offset = check_gatt_db_buffer(msg->conn_id, offset, 2, data);
			/* character number */
			c = srv->character.head;
			size = 0;
			while(c) {
				size++;
				c = LNEXT(c);
			}
			ptr = (uint16_t *)(data + offset);
			*ptr = size;
			offset += 2;

			c = srv->character.head;
			while(c) {
				offset = check_gatt_db_buffer(msg->conn_id, offset, 21, data);
				/* Character: 2 + 2 + 16 + 1*/
				ptr = (uint16_t *)(data + offset);
				*ptr++ = c->hdl;
				*ptr = c->value_hdl;
				offset += 4;
				memcpy(data + offset, c->uuid.u, 16);
				offset += 16;
				*(data + offset) = c->properties;
				offset++;

				offset = check_gatt_db_buffer(msg->conn_id, offset, 2, data);
				/* descriptor number */
				d = c->descriptor.head;
				size = 0;
				while(d) {
					size++;
					d = LNEXT(d);
				}
				ptr = (uint16_t *)(data + offset);
				*ptr = size;
				offset += 2;

				d = c->descriptor.head;
				while(d) {
					offset = check_gatt_db_buffer(msg->conn_id, offset, 18, data);
					/* Descriptor: 2 + 16 */
					ptr = (uint16_t *)(data + offset);
					*ptr = d->hdl;
					offset += 2;
					memcpy(data + offset, d->uuid.u, 16);
					offset += 16;
					d = LNEXT(d);
				}
				c = LNEXT(c);
			}
			srv = LNEXT(srv);
		}

		if (offset > 0) {
			GATT_CLIENT_CBACK(get_gatt_db, msg->conn_id, 1, offset, data);
		}
	}
}

enum {
	ATTRIBUTE_RW_TYPE_READ_CHARACTER,
	ATTRIBUTE_RW_TYPE_READ_DESCRIPTOR,
	ATTRIBUTE_RW_TYPE_WRITE_CHARACTER,
	ATTRIBUTE_RW_TYPE_WRITE_DESCRIPOTOR,
};

void hlp_gatt_fsm_gatt_client_read_characteristic(void *param)
{
	UINT8 status = BT_STATUS_SUCCESS;
	fsm_msg_gatt_client_read_characteristic_t *msg = (fsm_msg_gatt_client_read_characteristic_t *) param;
	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);

	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	struct GATT_ServiceStru * srv;
    srv = conn_info->service_list.head;
	struct GATT_CharacteristicStru *c;
    c = NULL;
	while(srv) {
		c = srv->character.head;
		while(c) {
			if(msg->handle == c->hdl) {
				conn_info->gatt_tl.context = (void *)(intptr_t) (msg->conn_id + (ATTRIBUTE_RW_TYPE_READ_CHARACTER << 16));
				conn_info->gatt_tl.cbk = hlp_gatt_rw_attribute_callback;
				/* wll callback with event in order as below:
				 * GATT_EV_TASK_CREATE -> GATT_EV_HDLVALUE -> GATT_EV_TASK_COMPLETE */
				conn_info->gatt_tl.sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1; /* TODO msg->auth_req to LE_GAP_SECURITY_MODEX_LEVELX*/	
				GATT_Read_Value(&(conn_info->gatt_tl), c->value_hdl);
				break;
			}
			c = LNEXT(c);
		}
		srv = LNEXT(srv);
	}
	if(c == NULL) {
		//characteristic not found.
		status = BT_STATUS_FAIL;
	}

end:
	if (status != BT_STATUS_SUCCESS) {
		GATT_CLIENT_CBACK(read_characteristic, msg->conn_id, BT_STATUS_FAIL, msg->handle, 0, NULL); /* XXX data(NULL) event parameter can NOT be parsed*/
	}
}

void hlp_gatt_fsm_gatt_client_read_using_characteristic_uuid(void *param)
{
	UINT8 status = BT_STATUS_SUCCESS;
	fsm_msg_gatt_client_read_using_characteristic_uuid_t *msg = (fsm_msg_gatt_client_read_using_characteristic_uuid_t *) param;
	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);

	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	struct GATT_ServiceStru * srv;
    srv = conn_info->service_list.head;
	struct GATT_CharacteristicStru *c;
    c = NULL;
	while(srv) {
		if (srv->hdl <= msg->s_handle && srv->endhdl >= msg->e_handle) {
			c = srv->character.head;
			while(c) {
				if (memcmp(c->uuid.u, msg->uuid.uu, 16) == 0) {
					conn_info->gatt_tl.context = (void *)(intptr_t) (msg->conn_id + (ATTRIBUTE_RW_TYPE_READ_CHARACTER << 16));
					conn_info->gatt_tl.cbk = hlp_gatt_rw_attribute_callback;
					conn_info->gatt_tl.sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1; /* TODO msg->auth_req to LE_GAP_SECURITY_MODEX_LEVELX*/	
					GATT_Read_Value(&(conn_info->gatt_tl), c->value_hdl);
					break;
				}
				c = LNEXT(c);
			}
		}
		srv = LNEXT(srv);
	}

	if(c == NULL) {
		//characteristic not found.
		status = BT_STATUS_FAIL;
	}

end:
	if (status != BT_STATUS_SUCCESS) {
		GATT_CLIENT_CBACK(read_characteristic, msg->conn_id, status, 0, 0, NULL); /* XXX data(NULL) event parameter can NOT be parsed*/
	}
}

void hlp_gatt_fsm_gatt_client_read_descriptor(void *param)
{
	UINT8 status = BT_STATUS_SUCCESS;
	fsm_msg_gatt_client_read_descriptor_t *msg = (fsm_msg_gatt_client_read_descriptor_t *) param;
	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);

	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		status = BT_STATUS_NOMEM;
		goto end;
	}

	void *k;
    k = NULL;
	UINT8 type;
    type = 0;
	k = hlp_gatt_item_get_by_hdl(conn_info, &type, msg->handle);
	if((k == NULL) || (type != ATT_TYPE_DESCRIPTOR)) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	} else {
		struct GATT_DescriptorStru *d = (struct GATT_DescriptorStru *) k;
		conn_info->gatt_tl.context = (void *)(intptr_t) (msg->conn_id + (ATTRIBUTE_RW_TYPE_READ_DESCRIPTOR << 16));
		conn_info->gatt_tl.cbk = hlp_gatt_rw_attribute_callback;
		conn_info->gatt_tl.sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1; /* TODO msg->auth_req to LE_GAP_SECURITY_MODEX_LEVELX*/
		GATT_Read_Value(&(conn_info->gatt_tl), d->hdl);
	}
end:
	if (status != BT_STATUS_SUCCESS) {
		GATT_CLIENT_CBACK(read_descriptor, conn_info->conn_id, status, msg->handle, 0, NULL);
	}
}

void hlp_gatt_fsm_gatt_client_write_characteristic(void *param)
{
	UINT8 status = BT_STATUS_SUCCESS;
	fsm_msg_gatt_client_write_characteristic_t *msg = (fsm_msg_gatt_client_write_characteristic_t *) param;
	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);

	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		status = BT_STATUS_NOMEM;
		goto end;
	}

	if (msg->value_len == 0 || msg->value == NULL) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	struct GATT_ServiceStru * srv;
    srv = conn_info->service_list.head;
	struct GATT_CharacteristicStru *c;
    c = NULL;
	while(srv) {
		c = srv->character.head;
		while(c) {
			if(msg->handle == c->hdl) {
				UINT8 task;
				if((msg->write_type  == 1) || (msg->write_type  == 4)) { /* TODO */
					task = GATT_TASK_WRITE_WITHOUT_RESPONSE;
				} else {
					task = GATT_TASK_WRITE_VALUE;
				}

				struct ATT_HandleValueStru *hdl_val;
				hdl_val = NEW(sizeof(struct ATT_HandleValueStru) + msg->value_len);
				hdl_val->hdl = c->value_hdl;
				hdl_val->offset = 0;
				hdl_val->val.len = msg->value_len;
				memcpy(hdl_val->val.value, msg->value, msg->value_len);

				conn_info->gatt_tl.context = (void *)(intptr_t) (msg->conn_id + (ATTRIBUTE_RW_TYPE_WRITE_CHARACTER << 16));
				conn_info->gatt_tl.cbk = hlp_gatt_rw_attribute_callback;
				conn_info->gatt_tl.context = (void *)(intptr_t) c->value_hdl;
				conn_info->gatt_tl.sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1; /* TODO msg->auth_req to LE_GAP_SECURITY_MODEX_LEVELX*/
				GATT_Write_Value(&(conn_info->gatt_tl), hdl_val, task);
				break;
			}
			c = LNEXT(c);
		}
		srv = LNEXT(srv);
	}
	if(c == NULL) {
		//characteristic not found.
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

end:
	if (status != BT_STATUS_SUCCESS) {
		GATT_CLIENT_CBACK(write_characteristic, msg->conn_id, status, msg->handle);
	}

	FREE(msg->value);
}

void hlp_gatt_fsm_gatt_client_write_descriptor(void *param)
{
	UINT8 status = BT_STATUS_SUCCESS;
	fsm_msg_gatt_client_write_descriptor_t *msg = (fsm_msg_gatt_client_write_descriptor_t *) param;
	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(msg->conn_id);

	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		status = BT_STATUS_NOMEM;
		goto end;
	}

	if (msg->value_len == 0 || msg->value == NULL) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	UINT8 type;
    type = 0;
	struct GATT_DescriptorStru *d;
    d = (struct GATT_DescriptorStru *) hlp_gatt_item_get_by_hdl(conn_info, &type, msg->handle);
	if((d == NULL) || (type != ATT_TYPE_DESCRIPTOR)) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	struct ATT_HandleValueStru *hdl_val;
	hdl_val = NEW(sizeof(struct ATT_HandleValueStru) + msg->value_len);
	hdl_val->hdl = d->hdl;
	hdl_val->offset = 0;
	hdl_val->val.len = msg->value_len;
	memcpy(hdl_val->val.value, msg->value, msg->value_len);

	conn_info->gatt_tl.context = (void *)(intptr_t) (msg->conn_id + (ATTRIBUTE_RW_TYPE_WRITE_DESCRIPOTOR << 16));
	conn_info->gatt_tl.cbk = hlp_gatt_rw_attribute_callback;
	conn_info->gatt_tl.context = (void *)(intptr_t) d->hdl;
	conn_info->gatt_tl.sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1;
	GATT_Write_Value(&(conn_info->gatt_tl), hdl_val, GATT_TASK_WRITE_VALUE);

end:
	if (status != BT_STATUS_SUCCESS) {
		GATT_CLIENT_CBACK(write_descriptor, msg->conn_id, status, msg->handle);
	}

	FREE(msg->value);
}

void hlp_gatt_fsm_gatt_client_execute_write(void *param)
{
	fsm_msg_gatt_client_execute_write_t *msg = (fsm_msg_gatt_client_execute_write_t *) param;

	int conn_id = msg->conn_id;
	int execute = msg->execute;
	// TODO ATT_ExecuteWrite_Req();
}

void hlp_gatt_fsm_gatt_client_register_for_notifications(void *param)
{
	fsm_msg_gatt_client_register_for_notifications_t *msg = (fsm_msg_gatt_client_register_for_notifications_t *) param;
	bt_status_t status = BT_STATUS_SUCCESS;
	uint16_t conn_id = 0;
	void *k = NULL;
	UINT8 type = 0;

	app_ble_conn_info_t * conn_info =  conn_info_get_by_addr(&(msg->address));
	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	conn_id = conn_info->conn_id;
	k = hlp_gatt_item_get_by_hdl(conn_info, &type, msg->handle);
	if((k == NULL) || (type != ATT_TYPE_CHARACTER)) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	} else {
		struct GATT_CharacteristicStru *c = k;
		struct GATT_DescriptorStru *d = c->descriptor.head;
		while(d) {
			if((d->uuid.u[13] == 0x29) && (d->uuid.u[12] == 0x02)) {
				break;
			}
			d = LNEXT(d);
		}
		if(d) {
			//ccc found.
			struct ATT_HandleValueStru *hdl_val;
			hdl_val = NEW(sizeof(struct ATT_HandleValueStru) + 2);
			hdl_val->hdl = d->hdl;
			hdl_val->offset = 0;
			hdl_val->val.len = 2;
			if(c->properties & ATT_NOTIFY) {
				//set notify
				hdl_val->val.value[0] = msg->enable ? 1 : 0;
				hdl_val->val.value[1] = 0;
			} else if(c->properties & ATT_INDICATE) {
				//set indicate
				hdl_val->val.value[0] = msg->enable ? 2 : 0;
				hdl_val->val.value[1] = 0;
			} else {
				GATT_CLIENT_DBG("shall not come into here\n");
			}
			conn_info->gatt_tl.context = (void *)(intptr_t) d->hdl;
			conn_info->gatt_tl.sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1;
			conn_info->gatt_tl.cbk = (void *) hlp_gatt_register_notifi_callback;
			GATT_Write_Value(&(conn_info->gatt_tl), hdl_val, GATT_TASK_WRITE_VALUE);
		} else {
			//not descriptor could set to notify/indicate
			GATT_CLIENT_DBG("not descriptor could set to notify/indicate");
			status = BT_STATUS_PARM_INVALID;
			goto end;
		}

	}
end:
	if (status != BT_STATUS_SUCCESS) {
		GATT_CLIENT_CBACK(register_for_notifications, conn_id, status, msg->enable, msg->handle);
	}
}

void hlp_gatt_fsm_client_on_device_connected(struct HCI_AddressStru *addr, UINT16 acl_hdl)
{
	GATT_CLIENT_DBG("%s acl_hdl:0x%x.", __func__, acl_hdl);
	bt_bdaddr_t remote_addr;
	HCI_ADDR_STRU_TO_BLUEDROID_BD(addr, &remote_addr);
	app_ble_conn_info_t * conn = conn_info_get_by_addr(&remote_addr);

	if (conn != NULL && conn->conn_state == CONNECTION_STATE_CONNECTED) {
		/* Already connected XXX*/
	} else {
		gatt_device_t *dev = gatt_device_find(&remote_addr);
		if (dev != NULL) {
			conn_info_on_connected(dev->app_if, addr, acl_hdl);
			GATT_CLIENT_CBACK(connected, dev->app_if, acl_hdl, BT_STATUS_SUCCESS, remote_addr);
		}
	}
}

void hlp_gatt_fsm_client_on_device_disconnected(struct HCI_AddressStru *addr, UINT16 acl_hdl)
{
	GATT_CLIENT_DBG("%s acl_hdl:0x%x.", __func__, acl_hdl);
	bt_bdaddr_t remote_addr;
	HCI_ADDR_STRU_TO_BLUEDROID_BD(addr, &remote_addr);
	app_ble_conn_info_t * conn = conn_info_get_by_addr(&remote_addr);

	if (conn != NULL) {
		GATT_CLIENT_DBG("%s state:%d.", __func__, conn->conn_state);
	}
	if (conn != NULL && conn->conn_state == CONNECTION_STATE_CONNECTED) {
		gatt_device_t *dev = gatt_device_find(&remote_addr);
		if (dev != NULL) {
			conn_info_on_disconnected(addr, acl_hdl);
			GATT_CLIENT_CBACK(disconnected, dev->app_if, acl_hdl, BT_STATUS_SUCCESS, remote_addr);
		}
	} else {
		/* Already disconnected XXX*/
	}
}

static void hlp_gap_execute_command_callback(void *context, UINT16 result, struct GAP_ExecuteCommandCfmStru *cfm_par)
{
	bt_status_t status = BT_STATUS_SUCCESS;
	bt_bdaddr_t address;
	struct HCI_Read_RSSICompStru *evt_par = cfm_par->event_parameter;

	if (!cfm_par) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	if (!evt_par || cfm_par->size != sizeof(struct HCI_Read_RSSICompStru)) {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

	app_ble_conn_info_t *conn_info;
    conn_info = conn_info_get_by_conn_id(evt_par->handle);
	if (conn_info) {
		memcpy(address.address, conn_info->dev_addr.bd, BD_ADDR_LEN);
	} else {
		status = BT_STATUS_PARM_INVALID;
		goto end;
	}

end:
    if (evt_par != NULL) {
	    GATT_CLIENT_CBACK(read_remote_rssi, (UINT16)(intptr_t) context, address, evt_par->rssi, evt_par->status);
    } else {
        GATT_CLIENT_DBG("Error, can not get RSSI");
        GATT_CLIENT_CBACK(read_remote_rssi, (UINT16)(intptr_t) context, address, 0, 0);
    }
}

void hlp_gatt_fsm_gatt_client_read_remote_rssi(void *param)
{
	fsm_msg_gatt_client_read_remote_rssi_t *msg = (fsm_msg_gatt_client_read_remote_rssi_t *) param;
	struct HCI_Read_RSSIStru in_par;
	GATT_CLIENT_DBG("addr:%02x%02x%02x%02x%02x%02x", msg->address.address[5],msg->address.address[4],msg->address.address[3],msg->address.address[2],msg->address.address[1],msg->address.address[0]);
	app_ble_conn_info_t * conn_info =  conn_info_get_by_addr(&(msg->address));
	GATT_CLIENT_DBG("conn_info = %x,conn_state = %d\r\n",conn_info, conn_info->conn_state);
	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		GATT_CLIENT_CBACK(read_remote_rssi, 0, msg->address, 0, BT_STATUS_PARM_INVALID);
		return;
	}

	in_par.handle = conn_info->conn_id;
	GAP_ExecuteCommandA(NULL, (void *)(intptr_t) msg->client_if, HCI_OPS_READ_RSSI, &in_par,
                          sizeof(struct HCI_Read_RSSIStru), hlp_gap_execute_command_callback);
}

void hlp_gatt_fsm_gatt_client_configure_mtu(void *param)
{
	fsm_msg_gatt_client_configure_mtu_t *msg = (fsm_msg_gatt_client_configure_mtu_t *) param;
	int conn_id = msg->conn_id;
	int mtu = msg->mtu;
	struct GATT_TL_ConnectStru * tlin;

	GATT_CLIENT_DBG("Exchange_MTU --- %d - %d.", conn_id, mtu);
	app_ble_conn_info_t * conn_info =  conn_info_get_by_conn_id(conn_id);
	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		return;
	}

	tlin = &(conn_info->gatt_tl);
	tlin->cbk = (void *) hlp_gatt_configure_mtu_callback;
	tlin->context = (void*)(intptr_t) conn_info->conn_id;

	tlin->require = GATT_TLREQUIRE_LE;/* FIXME, auto */
	tlin->sec_mode = LE_GAP_SECURITY_MODE1_LEVEL1;
	tlin->timeout = 0xFFFFFFFF;
	GATT_Exchange_MTU(tlin, mtu);
}

void hlp_gatt_fsm_gatt_connection_parameter_update(void *param)
{
	fsm_msg_gatt_connection_parameter_update_t *msg = (fsm_msg_gatt_connection_parameter_update_t *) param;
	struct L2CAP_ConnectionParameterUpdateStru *req;
	gatt_device_t *dev;

	req = (struct L2CAP_ConnectionParameterUpdateStru *) NEW(sizeof(struct L2CAP_ConnectionParameterUpdateStru));
	/* Set Address req.addr */;
	memcpy(req->addr.bd, msg->address.address, BD_ADDR_LEN);
	dev = gatt_device_find(&msg->address);

	if (dev != NULL) {
		req->addr.atype = dev->addr_type;
	} else {
		req->addr.atype = HCI_OWN_ADDRESS_TYPE_RANDOM_DEVICE_ADDRESS;
	}
	req->addr.cap = HCI_ADDRESS_CAP_LE;
	req->interval_min = msg->min_interval;
	req->interval_max = msg->max_interval;
	req->slave_latency = msg->latency;
	req->timeout_multiplier = msg->timeout;
	L2CAP_Connection_Parameter_Update_Req(req);
	/* TODO callback ? */
}

static void hlp_gatt_disconnect(app_ble_conn_info_t *conn)
{
	if (conn && conn->conn_state == CONNECTION_STATE_CONNECTED) {
		GATT_TL_Disconnect(&(conn->dev_addr), NULL, NULL);
	}
}

static void hlp_gatt_write_value(UINT16 write_value_hdl, struct GATT_TaskCompleteStru * par)
{
	bt_status_t status = BT_STATUS_SUCCESS;

	if(!par) {
		return;
	}

	bt_bdaddr_t bda;
	HCI_ADDR_STRU_TO_BLUEDROID_BD(&(par->addr), &bda);
	app_ble_conn_info_t * conn_info = conn_info_get_by_addr(&bda);
	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		GATT_CLIENT_DBG("Can NOT found the connection.");
		status = BT_STATUS_PARM_INVALID;
		return;
	}

	void *k;
	UINT8 type = 0;
	k = hlp_gatt_item_get_by_hdl(conn_info, &type, write_value_hdl);
	if(k != NULL) {
		switch(type) {
		case ATT_TYPE_CHARACTER_VALUE: {
			struct GATT_CharacteristicStru *characteristic = k;
			struct GATT_ServiceStru * srv = hlp_gatt_find_service_by_char(conn_info->service_list, characteristic);
			if(srv) {
				GATT_CLIENT_CBACK(write_characteristic, conn_info->conn_id, par->errcode, characteristic->hdl);
			} else {
				GATT_CLIENT_DBG("not found service by char");
			}
		}
		break;
		case ATT_TYPE_DESCRIPTOR: {
			struct GATT_ServiceStru *srv = NULL;
			struct GATT_CharacteristicStru *characteristic = NULL;
			srv = hlp_gatt_find_srv_char_by_desc_hdl(write_value_hdl, conn_info->service_list, &characteristic);
			if(srv) {
				struct GATT_DescriptorStru *descriptor = k;
                /* FIXME XXX   just callback write_descriptor.
				if((descriptor->uuid.u[13] == 0x29) && (descriptor->uuid.u[12] == 0x02)) {
					if (descriptor->val != NULL) {
						GATT_CLIENT_CBACK(register_for_notifications, conn_info->conn_id, par->errcode, descriptor->val->value[0] > 0 ? 1 : 0, descriptor->hdl);
					}
				} else {
					GATT_CLIENT_CBACK(write_descriptor, conn_info->conn_id, par->errcode, descriptor->hdl);
				}
                */
                GATT_CLIENT_CBACK(write_descriptor, conn_info->conn_id, par->errcode, descriptor->hdl);
			} else {
				GATT_CLIENT_DBG("not found service by desc hdl");
			}
		}
		break;
		default: {
			GATT_CLIENT_DBG("write comple invalid handle type(%d)", type);
		}
		break;
		}
	} else {
		GATT_CLIENT_DBG("write comple handle invalid");
		//write cb failed
	}
}

static void hlp_gatt_read_task_complete(UINT16 read_value_hdl, struct GATT_TaskCompleteStru * par)
{
	bt_status_t status = BT_STATUS_SUCCESS;

	if (!par) {
		return;
	}

	bt_bdaddr_t bda;
	HCI_ADDR_STRU_TO_BLUEDROID_BD(&(par->addr), &bda);
	app_ble_conn_info_t * conn_info = conn_info_get_by_addr(&bda);
	if((NULL == conn_info) || (conn_info->conn_state != CONNECTION_STATE_CONNECTED)) {
		GATT_CLIENT_DBG("Can NOT found the connection.");
		status = BT_STATUS_PARM_INVALID;
		return;
	}

	void *k;
	UINT8 type = 0;
	k = hlp_gatt_item_get_by_hdl(conn_info, &type, read_value_hdl);
	if(k != NULL) {
		GATT_CLIENT_DBG("enter %s read_attribute type:%d.", __func__, type);
		switch(type) {
		case ATT_TYPE_CHARACTER_VALUE:
			{
				struct GATT_CharacteristicStru *c = k;
				GATT_CLIENT_DBG("%s --- par->errcode:0x%0x.", __func__, par->errcode);
				if (c->val != NULL) {
					GATT_CLIENT_DBG("%s --- val len:0x%0x.", __func__, c->val->len);
					GATT_CLIENT_CBACK(read_characteristic, conn_info->conn_id, par->errcode, read_value_hdl,
							(c->val->len > BTGATT_MAX_ATTR_LEN) ? BTGATT_MAX_ATTR_LEN : c->val->len, c->val->value);
					FREE(c->val);
					c->val = NULL;
				} else {
					GATT_CLIENT_CBACK(read_characteristic, conn_info->conn_id, par->errcode, read_value_hdl, 0, NULL);
				}
			}
			break;
		case ATT_TYPE_DESCRIPTOR:
			{
				struct GATT_DescriptorStru* desc = k;
				if(desc->val) {
					GATT_CLIENT_CBACK(read_descriptor, conn_info->conn_id, par->errcode, read_value_hdl, 
							(desc->val->len > BTGATT_MAX_ATTR_LEN) ? BTGATT_MAX_ATTR_LEN : desc->val->len, desc->val->value);
					FREE(desc->val);
					desc->val = NULL;
				} else {
					GATT_CLIENT_CBACK(read_descriptor, conn_info->conn_id, BT_STATUS_FAIL, read_value_hdl, 0, NULL);
				}
			}
			break;
		default:
			break;
		}
	} else {
		GATT_CLIENT_DBG("%s can NOT find read_value_hdl(0x%x)", __func__, read_value_hdl);
	}
}

static void hlp_gatt_rw_attribute_completed_callback(void *context, void *arg)
{
	struct GATT_TaskCompleteStru *in = arg;

	if (in == NULL) {
		GATT_CLIENT_DBG("%s param null.", __func__);
		return;
	}

	switch (in->task) {
		case GATT_TASK_WRITE_WITHOUT_RESPONSE:
		case GATT_TASK_WRITE_VALUE:
			hlp_gatt_write_value((UINT16)(intptr_t) context, (struct GATT_TaskCompleteStru *) arg);
			break;
		case GATT_TASK_READ_VALUE:
			hlp_gatt_read_task_complete((UINT16)(intptr_t) context, (struct GATT_TaskCompleteStru *) arg);
			break;
		default:
			break;
	}
	FREE(arg);
}

static void hlp_gatt_configure_mtu_callback(void *context, UINT8 ev, void *arg)
{
	switch (ev) {
		case GATT_EV_TASK_COMPLETE:
			{
				UINT16 mtu = 0;
				UINT16 conn_id = (UINT16)(intptr_t) context;
				bt_status_t status = BT_STATUS_SUCCESS;
				app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);

				if(conn_info == NULL) {
					GATT_CLIENT_DBG("Can NOT found the connection.");
					status = BT_STATUS_FAIL;
				} else {
					mtu = conn_info->mtu;
				}

				GATT_CLIENT_CBACK(configure_mtu, conn_id, status, mtu);
			}
			break;
	}

	FREE(arg);
}

static void hlp_gatt_register_notifi_callback(void *context, UINT8 ev, void *arg)
{
	switch (ev) {
		case GATT_EV_TASK_COMPLETE:
			{
				struct GATT_TaskCompleteStru *in = arg;
				bt_status_t status = BT_STATUS_SUCCESS;
				bt_bdaddr_t addr;
				HCI_ADDR_STRU_TO_BLUEDROID_BD(&(in->addr), &addr);
				uint16_t conn_id = 0;
				uint8_t registered = 1;

				app_ble_conn_info_t *conn = conn_info_get_by_addr(&addr);

				if (conn != NULL) {
					conn_id = conn->conn_id;
				} else {
					status = BT_STATUS_PARM_INVALID;
					registered = 0;
				}
				GATT_CLIENT_CBACK(register_for_notifications, conn_id, status, registered, (uint16_t)(intptr_t) context);
			}
			break;
	}

	FREE(arg);
}

static void hlp_gatt_descriptor_result(UINT16 conn_id, void *arg)
{
	struct GATT_DescriptorStru *desc = (struct GATT_DescriptorStru *) arg;

	if (gatt_db_add_descriptor_by_conn_id(conn_id, desc)) {
		GATT_CLIENT_DBG("%s new descriptor[0x%04X - prop:0x%04X]", __func__, desc->hdl, desc->properties);
	}
}

static void hlp_gatt_character_result(UINT16 conn_id, void *arg)
{
	struct GATT_CharacteristicStru *c = (struct GATT_CharacteristicStru *) arg;
	struct GATT_ServiceStru *service;
	service = (struct GATT_ServiceStru *) gatt_db_add_char_by_conn_id(conn_id, c);

	if (service != NULL) {
		GATT_CLIENT_DBG("%s new character[0x%04X - val:0x%04X] in service[0x%04X - 0x%04X]", __func__, c->hdl, c->value_hdl, service->hdl, service->endhdl);
	}
}

static void hlp_gatt_include_service_result(UINT16 conn_id, void *arg)
{
	struct GATT_IncludeStru * incl_service = (struct GATT_IncludeStru *) arg;

	if (gatt_db_add_include_service_by_conn_id(conn_id, incl_service)) {
		GATT_CLIENT_DBG("%s new include service[0x%04X - 0x%04X] include[0x%04X]", __func__, incl_service->hdl, incl_service->endhdl, incl_service->include_hdl);
	}
}

static void hlp_gatt_service_result(UINT16 conn_id, void *arg)
{
	struct GATT_ServiceStru * service = (struct GATT_ServiceStru *) arg;

	if (gatt_db_add_service_by_conn_id(conn_id, service)) {
		GATT_CLIENT_DBG("%s new service[0x%04X - 0x%04X]", __func__, service->hdl, service->endhdl);
	}
}

static void hlp_gatt_search_descriptor_task_completed_callback(UINT16 conn_id, void *arg)
{
	GATT_CLIENT_DBG("%s", __func__);
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru * service;
	struct GATT_CharacteristicStru *character;
	struct GATT_CharacteristicStru *next_char;
	struct GATT_TL_ConnectStru * tlin;
	
	if(conn_info == NULL) {
		GATT_CLIENT_CBACK(search_completed, conn_id, BT_STATUS_FAIL);
        return;
	}

	tlin = &(conn_info->gatt_tl);
	tlin->cbk = (void *) hlp_gatt_search_services_callback;
	tlin->context = (void *)(intptr_t) conn_info->conn_id;
	service = conn_info->cur_service;
	character = conn_info->cur_character;

	if (character != NULL) {
		character = conn_info->cur_character = LNEXT(character);

		if (character != NULL) {
			next_char = LNEXT(character);
			GATT_Discover_AllCharacteristic_Descriptors(tlin, character->value_hdl + 1, next_char == NULL ? service->endhdl: next_char->hdl - 1);
		} else {
			/* All characters have been discovered in current service */
			service = conn_info->cur_service = LNEXT(service);

			if (service != NULL) {
				tlin->cbk = (void *) hlp_gatt_search_services_callback;
				tlin->context = (void *)(intptr_t) conn_info->conn_id;
				GATT_Find_IncludedServices(tlin, service->hdl, service->endhdl);
			} else {
				GATT_CLIENT_CBACK(search_completed, conn_id, BT_STATUS_SUCCESS);
			}
		}
	} else {
		/* Can NOT happen*/
	}
}

static void hlp_gatt_search_character_task_completed_callback(UINT16 conn_id, void *arg)
{
	GATT_CLIENT_DBG("%s", __func__);
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru * service;
	struct GATT_CharacteristicStru *character;
	struct GATT_CharacteristicStru *next_char;
	struct GATT_TL_ConnectStru * tlin;
	
	if(conn_info == NULL) {
		GATT_CLIENT_CBACK(search_completed, conn_id, BT_STATUS_FAIL);
        return;
	}

	tlin = &(conn_info->gatt_tl);
	service = conn_info->cur_service;
	character = conn_info->cur_character = service->character.head;
	/* Begin to discover all descriptors of the first character in current service */
	if (character != NULL) {
		tlin->cbk = (void *) hlp_gatt_search_services_callback;
		tlin->context = (void *)(intptr_t) conn_info->conn_id;
		next_char = LNEXT(character);
		GATT_Discover_AllCharacteristic_Descriptors(tlin, character->value_hdl + 1, next_char == NULL ? service->endhdl: next_char->hdl - 1);
	} else {
		/* If current service has no characteristics, Begin to discover all characteristic of next service */
		service = LNEXT(service);
		if (service != NULL) {
			tlin->cbk = (void *) hlp_gatt_search_services_callback;
			conn_info->cur_service = service;
			GATT_Discover_Characteristics(tlin, conn_info->cur_service->hdl, conn_info->cur_service->endhdl, NULL);
		} else {
			GATT_CLIENT_CBACK(search_completed, conn_id, BT_STATUS_SUCCESS);
		}
	}
}

static void hlp_gatt_search_include_task_completed_callback(UINT16 conn_id, void *arg)
{
	GATT_CLIENT_DBG("%s", __func__);
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru * service;
	struct GATT_TL_ConnectStru * tlin;
	
	if(conn_info == NULL) {
		GATT_CLIENT_CBACK(search_completed, conn_id, BT_STATUS_FAIL);
        return;
	}

	tlin = &(conn_info->gatt_tl);
	/* Begin to discover all characteristics of the current service */
	if (conn_info->cur_service != NULL) {
		tlin->cbk = (void *) hlp_gatt_search_services_callback;
		tlin->context = (void *)(intptr_t) conn_info->conn_id;
		GATT_Discover_Characteristics(tlin, conn_info->cur_service->hdl, conn_info->cur_service->endhdl, NULL);
	}
}

static void hlp_gatt_search_task_completed_callback(UINT16 conn_id, void *arg)
{
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru * service;
	struct GATT_TL_ConnectStru * tlin;
	
	if(conn_info == NULL) {
		GATT_CLIENT_CBACK(search_completed, conn_id, BT_STATUS_FAIL);
        return;
	}

	tlin = &(conn_info->gatt_tl);
	/* Begin to discover all included services of the first service */
	service = conn_info->cur_service = (struct GATT_ServiceStru *) conn_info->service_list.head;
	if (service != NULL) {
		tlin->cbk = (void *) hlp_gatt_search_services_callback;
		tlin->context = (void *)(intptr_t) conn_info->conn_id;
		GATT_Find_IncludedServices(tlin, service->hdl, service->endhdl);
	} else {
		GATT_CLIENT_CBACK(search_completed, conn_id, BT_STATUS_SUCCESS);
	}
}

static void hlp_gatt_search_services_callback(void *context, UINT8 ev, void *arg)
{
	switch (ev) {
		case GATT_EV_TASK_CREATE:
			break;
		case GATT_EV_TASK_COMPLETE:
			{
				struct GATT_TaskCompleteStru *in = arg;
				switch (in->task) {
					case GATT_TASK_DISCOVER_ALLSERVICES:
					case GATT_TASK_DISCOVER_SERVICES_BYUUID:
						hlp_gatt_search_task_completed_callback((UINT16)(intptr_t) context, arg);
						break;
					case GATT_TASK_FIND_INCLUDED_SERVICES:
						hlp_gatt_search_include_task_completed_callback((UINT16)(intptr_t) context, arg);
						break;
					case GATT_TASK_DISCOVER_ALLCHARACTERISTIC_OFASERVICE:
						hlp_gatt_search_character_task_completed_callback((UINT16)(intptr_t) context, arg);
						break;
					case GATT_TASK_DISCOVER_ALLCHARACTERISTIC_DESCRIPTORS:
						hlp_gatt_search_descriptor_task_completed_callback((UINT16)(intptr_t) context, arg);
						break;
					default:
						break;
				}
				FREE(arg);
			}
			break;
		case GATT_EV_CREATE_SERVICE:
			hlp_gatt_service_result((UINT16)(intptr_t) context, arg);
			break;
		case GATT_EV_CREATE_INCLUDE:
			hlp_gatt_include_service_result((UINT16)(intptr_t) context, arg);
			break;
		case GATT_EV_CREATE_CHARACTER:
			hlp_gatt_character_result((UINT16)(intptr_t) context, arg);
			break;
		case GATT_EV_CREATE_DESCRIPTOR:
			hlp_gatt_descriptor_result((UINT16)(intptr_t) context, arg);
			break;
	}

}

static void hlp_gatt_handle_value(void *context, void *arg)
{
    struct ATT_HandleValueCbkStru *in = arg;
	if (in) {
		uint8_t is_notify = 1;
		uint16_t conn_id = 0;
		bt_bdaddr_t address;
		HCI_ADDR_STRU_TO_BLUEDROID_BD(&(in->addr), &address);
		app_ble_conn_info_t * conn_info = conn_info_get_by_addr(&address);
		conn_id = conn_info->conn_id;
		GATT_CLIENT_DBG("%s conn_id:%d, handle=0x%04x, value length=%d.", __FUNCTION__, conn_id, in->data.hdl, in->data.val.len);
		GATT_CLIENT_CBACK(notify, conn_id, address, in->data.hdl - 1/*XXX*/, is_notify, in->data.val.len, in->data.val.value + in->data.offset);
	}
	// >[ATT][PDU][*Recv][0x1b,HANDLE_VALUE_NOTIFICATION][Hdl:0x002d][Len:20][Hex:27,f1,e3,54,8b,8f,dd,a9,0d,8a,b7,87,3f,e0,2c,be,5d,5e,f4,d4]
}

static void hlp_gatt_rw_attriute_value(uint32_t context, void *arg)
{
	uint16_t conn_id = context & 0xFFFF;
	int type = context >> 16;

    struct ATT_HandleValueCbkStru *in = arg;
	if (in) {
		bt_bdaddr_t address;
		HCI_ADDR_STRU_TO_BLUEDROID_BD(&(in->addr), &address);

		switch (type) {
			case ATTRIBUTE_RW_TYPE_READ_CHARACTER:
				GATT_CLIENT_CBACK(read_characteristic, conn_id, 0, in->data.hdl - 1/*XXX*/, in->data.val.len, in->data.val.value + in->data.offset);
				break;
			case ATTRIBUTE_RW_TYPE_READ_DESCRIPTOR:
				GATT_CLIENT_CBACK(read_descriptor, conn_id, 0, in->data.hdl, in->data.val.len, in->data.val.value + in->data.offset);
				break;
			case ATTRIBUTE_RW_TYPE_WRITE_CHARACTER:
				GATT_CLIENT_CBACK(write_characteristic, conn_id, 0, in->data.hdl - 1/*XXX*/);
				break;
			case ATTRIBUTE_RW_TYPE_WRITE_DESCRIPOTOR:
				GATT_CLIENT_CBACK(write_descriptor, conn_id, 0, in->data.hdl);
				break;
		}
	}
}

static void hlp_gatt_notifi_callback(void *context, UINT8 ev, void *arg)
{
	GATT_CLIENT_DBG("%s(0x%x)", __func__, ev);
	return ;
	switch (ev) {
		case GATT_EV_HDLVALUE:
			hlp_gatt_handle_value(context, arg);
			break;
		case GATT_EV_MTU:   /* arg: (struct GATT_MTUCbkStru *) */
			{
				struct GATT_MTUCbkStru * cb = (struct GATT_MTUCbkStru *) arg;
				bt_bdaddr_t addr;

				if (arg == NULL) {
					return;
				}

				HCI_ADDR_STRU_TO_BLUEDROID_BD(&(cb->addr), &addr);
				app_ble_conn_info_t *conn = conn_info_get_by_addr(&addr);

				if (conn != NULL) {
					conn->mtu = cb->mtu;
					GATT_CLIENT_DBG("MTU --- %d.", conn->mtu);
				}
			}
			break;
		default:
			break;
	}

	FREE(arg);
}

static void hlp_gatt_rw_attribute_callback(void *context, UINT8 ev, void *arg)
{
	GATT_CLIENT_DBG("%s(ev=0x%x)", __func__, ev);
	switch (ev) {
	case GATT_EV_HDLVALUE:
		//Task OnGoing
		hlp_gatt_rw_attriute_value((uint32_t)(intptr_t) context, arg);
		break;
	case GATT_EV_CREATE_SERVICE:/* following create, local is client, get peer server`s */
		//hlp_gatt_service_result(context, arg);
		break;
	case GATT_EV_CREATE_INCLUDE:
		//hlp_gatt_included_result(context, arg);
		break;
	case GATT_EV_CREATE_CHARACTER:
		//hlp_gatt_characteristic_result(context, arg);
		break;
	case GATT_EV_CREATE_DESCRIPTOR:
		//hlp_gatt_characteristic_desc_result(context, arg);
		break;
	case GATT_EV_TASK_COMPLETE: 
		hlp_gatt_rw_attribute_completed_callback(context, arg);
		break;
	case GATT_EV_HDL_TREE:/* Local is server */
		break;
	case GATT_EV_TASK_CREATE:
		break;
	case GATT_EV_PEND_IND:/* struct ATT_PendIndStru *in */
		break;
	default:
		break;
	}
}

