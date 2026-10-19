#include <stdio.h>
#include "internal_gatt_conn.h"
#define LOG_TAG "GATT_CONN"
#define MODULE_TAG LOG_TAG
#include "brt_trace.h"
#include "bt_utils.h"

static app_ble_conn_info_t s_conn_list[MAX_BLE_CONNECTION_COUNT];

static void hlp_gatt_db_free(struct ListStru service_list)
{
	struct GATT_ServiceStru *cur_service;
	BRT_DBG("enter hlp_gatt_db_free");
	while ((cur_service = List_RemoveHead(&service_list)) != NULL) {
		BRT_DBG("service:%p", cur_service);
		GATT_Free_Service(cur_service);
	}

	service_list.head = NULL;
	service_list.tail = NULL;
	BRT_DBG("exit hlp_gatt_db_free");
}

void conn_info_init(void)
{
	int i = 0;
	for(i = 0; i < MAX_BLE_CONNECTION_COUNT; i++) {
		memset(&s_conn_list[i], 0, sizeof(app_ble_conn_info_t));
		s_conn_list[i].client_if = BTGATT_APP_IF_INVALID;
		s_conn_list[i].conn_id = INVALID_CONNECTION_ID;
		s_conn_list[i].conn_state = CONNECTION_STATE_DISCONNECTED;
	}

}

static app_ble_conn_info_t *conn_info_find_avail(struct HCI_AddressStru *address)
{
	app_ble_conn_info_t *conn_info = NULL;
	int i, j = -1;

	for(i = 0; i < MAX_BLE_CONNECTION_COUNT; i++) {
		/* find the first empty connection. */
		if(s_conn_list[i].conn_id == INVALID_CONNECTION_ID && (j < 0)) {
			j = i;
			conn_info = &(s_conn_list[i]);
		}
		/* if connection with the same address exit. return it.*/
		if(!memcmp(s_conn_list[i].dev_addr.bd, address->bd, BD_ADDR_LEN))
			//if(!memcmp(&(s_conn_list[i].dev_addr), address, sizeof(struct HCI_AddressStru)))
		{
			conn_info = &(s_conn_list[i]);
			break;
		}
	}

	if(NULL == conn_info) {
		BRT_DBG("Too many connection!");
	}

	return conn_info;
}

void gatt_conn_iterator_by_client_if(UINT8 client_if, gatt_conn_iterator_cbk cbk)
{
	int i = 0;
	for(i = 0; i < MAX_BLE_CONNECTION_COUNT; i++) {
		if(s_conn_list[i].client_if == client_if) {
			if (cbk) {
				cbk(&s_conn_list[i]);
			}
		}
	}
}

/*******************************************************************
  ********************** connection management *********************
*******************************************************************/
app_ble_conn_info_t * conn_info_get_by_conn_id(int conn_id)
{
	int i = 0;
	for( i = 0; i < MAX_BLE_CONNECTION_COUNT; i++) {
		if((s_conn_list[i].conn_id == conn_id) &&
		   (s_conn_list[i].conn_state == CONNECTION_STATE_CONNECTED)) {
			return &(s_conn_list[i]);
		}
	}

	return NULL;
}

app_ble_conn_info_t * conn_info_get_by_addr(bt_bdaddr_t *rmt_addr)
{
	int i = 0;
	for(i = 0; i < MAX_BLE_CONNECTION_COUNT; i++) {
		if(!memcmp(s_conn_list[i].dev_addr.bd, rmt_addr->address, BD_ADDR_LEN)) {
			return &(s_conn_list[i]);
		}
	}
	return NULL;
}

int conn_info_is_exist(struct HCI_AddressStru* address)
{
	int i;
	for(i = 0; i < MAX_BLE_CONNECTION_COUNT; i++) {
		if((!memcmp(s_conn_list[i].dev_addr.bd, address->bd, BD_ADDR_LEN))
		   && (s_conn_list[i].conn_state == CONNECTION_STATE_CONNECTED)) {
			return s_conn_list[i].conn_id;
		}
	}

	return 0;
}

app_ble_conn_info_t *conn_info_on_connected(int client_if, struct HCI_AddressStru* address, UINT16 acl_hdl)
{
	int i = 0;
	BRT_DBG("client_if:%d, acl:%d", client_if, acl_hdl);
	BRT_DBG("addr:%02x%02x%02x%02x%02x%02x", address->bd[5],address->bd[4],address->bd[3],address->bd[2],address->bd[1],address->bd[0]);
	app_ble_conn_info_t *conn_info = conn_info_find_avail(address);
	client_if = CLIENT_IF_TO_LOWER_LAYER(client_if);
	// TODO check client_if
	if (conn_info == NULL) {
		return NULL;
	}

	BRT_DBG("conn_info:%p, service head:%p, service tail:%p", conn_info, conn_info->service_list.head, conn_info->service_list.tail);
	conn_info->client_if = client_if;
	conn_info->is_direct = 1;
	conn_info->conn_id = acl_hdl;
	conn_info->dev_addr = *address;
	conn_info->conn_state = CONNECTION_STATE_CONNECTED;
	memcpy(&(conn_info->gatt_tl.addr), address, sizeof(struct HCI_AddressStru));

	return conn_info;
}

void conn_info_on_disconnected(struct HCI_AddressStru* address, UINT16 acl_hdl)
{
	int i;
	app_ble_conn_info_t *conn_info = NULL;

	for(i = 0; i < MAX_BLE_CONNECTION_COUNT; i++) {
		if(!memcmp(s_conn_list[i].dev_addr.bd, address->bd, BD_ADDR_LEN)) {
			conn_info = &(s_conn_list[i]);
			break;
		}
	}

	if (conn_info->conn_state == CONNECTION_STATE_CONNECTED) {
		hlp_gatt_db_free(conn_info->service_list);
	}

	conn_info->conn_state = CONNECTION_STATE_DISCONNECTED;
	conn_info->conn_id = INVALID_CONNECTION_ID;
}

app_ble_conn_info_t *conn_info_add(int client_if, struct HCI_AddressStru* address, UINT8 is_direct)
{
	// TODO check client_if
	app_ble_conn_info_t * conn_info = conn_info_find_avail(address);

	if(NULL == conn_info) {
		return NULL;
	}

	conn_info->dev_addr = *address;
	conn_info->client_if = CLIENT_IF_TO_LOWER_LAYER(client_if);
	conn_info->is_direct = is_direct;
	conn_info->conn_state = CONNECTION_STATE_DISCONNECTED;
	memcpy(&(conn_info->gatt_tl.addr), address, sizeof(struct HCI_AddressStru));

	return conn_info;
}

int hlp_gatt_get_service_by_hdl(UINT16 conn_id, UINT16 handle, btgatt_srvc_id_t *srvc_id)
{
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru *s;

	if(conn_info == NULL || srvc_id == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return 0;
	}

	s = conn_info->service_list.head;
	while (s) {
		if (s->hdl <= handle && s->endhdl >= handle) {
			srvc_id->id.inst_id = s->hdl;
			memcpy(&(srvc_id->id.uuid.uu), &(s->uuid.u), 16);

			return 1;
		}
		s = LNEXT(s);
	}

	return 0;
}

void *hlp_gatt_item_get_by_hdl(app_ble_conn_info_t *conn_info, UINT8 *type, UINT16 hdl)
{
	struct GATT_ServiceStru *s = conn_info->service_list.head;
	struct GATT_CharacteristicStru *c;
	struct GATT_DescriptorStru *d;
	struct GATT_IncludeStru *inc;

	while (s) {
		if (s->hdl == hdl) {
			*type = ATT_TYPE_SERVICE;
			return s;
		}

		inc = s->include.head;
		while (inc) {
			if (inc->hdl == hdl) {
				*type = ATT_TYPE_INCLUDE;
				return inc;
			}
			inc = LNEXT(inc);
		}

		c = s->character.head;
		while (c) {
			if (c->hdl == hdl) { //Declearation handle...
				*type = ATT_TYPE_CHARACTER;
				return c;
			}
			if (c->value_hdl == hdl) { //Value handle...
				*type = ATT_TYPE_CHARACTER_VALUE;
				return c;
			}
			d = c->descriptor.head;
			while (d) {
				if (d->hdl == hdl) {
					*type = ATT_TYPE_DESCRIPTOR;
					return d;
				}
				d = LNEXT(d);
			}
			c = LNEXT(c);
		}
		s = LNEXT(s);
	}

	return NULL;
}

int gatt_db_add_service_by_conn_id(UINT16 conn_id, struct GATT_ServiceStru *service)
{
	void *k;
	UINT8 type = 0;
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);

	BRT_DBG("get conn_info:%p, service:%p", conn_info, service);
	if(conn_info == NULL || service == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return 0;
	}

	if ((k = hlp_gatt_item_get_by_hdl(conn_info, &type, service->hdl)) != NULL) { /* Kill Duplicated */
		if (type == ATT_TYPE_SERVICE) {
			List_RemoveAt(&(conn_info->service_list), k);
			GATT_Free_Service(k);
		} else {
			BRT_DBG("Found, but not a service,Hdl:0x%x,Type:%d\r\n", service->hdl, type);
		}
	}
	List_AddTail(&(conn_info->service_list), service);

	return 1;
}

int gatt_db_add_include_service_by_conn_id(UINT16 conn_id, struct GATT_IncludeStru *include)
{
	void *k;
	UINT8 type = 0;
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru *s;

	BRT_DBG("get conn_info:%p, include service:%p", conn_info, include);
	if(conn_info == NULL || include == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return 0;
	}

	// TODO
#if 0
	if ((k = hlp_gatt_item_get_by_hdl(conn_info, &type, include->hdl)) != NULL) { /* Kill Duplicated */
		if (type == ATT_TYPE_INCLUDE) {
			List_RemoveAt(&(conn_info->cur_service->include), k);
			LFREE(k);
		} else {
			BRT_DBG("Found, but not a include,Hdl:0x%x,Type:%d\r\n", include->hdl, type);
		}
	}

	List_AddTail(&(conn_info->cur_service->include), include);
#endif
	s = conn_info->service_list.head;
	while (s) {
		if (s->hdl <= include->hdl && s->endhdl >= include->hdl) {
			List_AddTail(&(s->include), include);
			break;
		}
		s = LNEXT(s);
	}

	return 1;
}

void * gatt_db_add_char_by_conn_id(UINT16 conn_id, struct GATT_CharacteristicStru *c)
{
	void *k;
	UINT8 type = 0;
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru *s;

	BRT_DBG("get conn_info:%p, character:%p", conn_info, c);
	if(conn_info == NULL || c == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return NULL;
	}

#if 0
	if ((k = hlp_gatt_item_get_by_hdl(conn_info, &type, c->hdl)) != NULL) { /* Kill Duplicated */
		if (type == ATT_TYPE_CHARACTER) {
			/* UPF43, should make sure if k in this service or not??? maybe in others! */
			List_RemoveAt(&(conn_info->cur_service->character), k);
			GATT_Free_Character(k);
		} else {
			BRT_DBG("Found, but not a character,Hdl:0x%x,Type:%d", c->hdl, type);
		}
	}
#endif

	s = conn_info->service_list.head;
	while (s) {
		if (s->hdl <= c->hdl && s->endhdl >= c->hdl) {
			List_AddTail(&(s->character), c);
			break;
		}
		s = LNEXT(s);
	}

	return s;
}

int gatt_db_add_descriptor_by_conn_id(UINT16 conn_id, struct GATT_DescriptorStru *desc)
{
	void *k;
	UINT8 type = 0;
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru *s;
	struct GATT_CharacteristicStru *c;

	BRT_DBG("get conn_info:%p, descriptor:%p", conn_info, desc);
	if(conn_info == NULL || desc == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return 0;
	}

#if 0
	if ((k = hlp_gatt_item_get_by_hdl(conn_info, &type, desc->hdl)) != NULL) { /* Kill Duplicated */
		if (type == ATT_TYPE_DESCRIPTOR) {
			List_RemoveAt(&(conn_info->cur_character->descriptor), k);
			GATT_Free_Descriptor(k);
		} else {
			BRT_DBG("Found, but not a descriptor,Hdl:0x%x,Type:%d\r\n", desc->hdl, type);
		}
	}
#endif

	s = conn_info->service_list.head;
	while (s) {
		if (s->hdl <= desc->hdl && s->endhdl >= desc->hdl) {
			c = s->character.head;
			while (c) {
				if (c == conn_info->cur_character) {
					List_AddTail(&(c->descriptor), desc);
					break;
				}
				c = LNEXT(c);
			}
			break;
		}
		s = LNEXT(s);
	}

	return 1;
}

struct GATT_ServiceStru * hlp_gatt_find_service_by_hdl(UINT16 conn_id, UINT16 hdl)
{
	struct ListStru service_list;
	struct GATT_ServiceStru * srv;
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);

	if (conn_info == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return NULL;
	}

	service_list = conn_info->service_list;
	srv = service_list.head;
	while(srv) {
		if(srv->hdl == hdl) {
			break;
		}
		srv = LNEXT(srv);
	}
	return srv;
}

struct GATT_ServiceStru * hlp_gatt_find_service_by_char(struct ListStru service_list, struct GATT_CharacteristicStru *characteristic)
{
	struct GATT_ServiceStru * srv = service_list.head;
	struct GATT_CharacteristicStru *c = NULL;
	while(srv ) {
		c = srv->character.head;
		while(c) {
			if(c == characteristic) {
				break;
			}
			c = LNEXT(c);
		}
		if(c) {
			break;
		} else {
			srv = LNEXT(srv);
		}

	}
	return srv;
}

struct GATT_ServiceStru * hlp_gatt_find_srv_char_by_desc_hdl(UINT16 desc_hdl, struct ListStru service_list, struct GATT_CharacteristicStru **characteristic)
{
	struct GATT_ServiceStru * srv = service_list.head;
	struct GATT_CharacteristicStru *c;
	struct GATT_DescriptorStru *desc;
	c = NULL;

	while(srv) {
		c = srv->character.head;
		while(c) {
			desc = c->descriptor.head;
			while(desc) {
				if(desc->hdl == desc_hdl) {
					*characteristic = c;
					break;
				}
				desc = LNEXT(desc);
			}
			if(desc) {
				break;
			}
			c = LNEXT(c);
		}

		if(c) {
			break;
		}
		srv = LNEXT(srv);
	}

	return srv;
}

//struct GATT_CharacteristicStru * hlp_gatt_find_next_char(UINT16 conn_id, struct GATT_ServiceStru * srvc, struct GATT_CharacteristicStru *start_char)
struct GATT_CharacteristicStru * hlp_gatt_find_next_char(UINT16 conn_id, btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *start_char_id)
{
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru * s;
	struct GATT_CharacteristicStru *c;

	if (conn_info == NULL || srvc_id == NULL || start_char_id == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return NULL;
	}

	s = conn_info->service_list.head;
	while (s) {
		if (s->hdl == srvc_id->id.inst_id) {
			c = s->character.head;

			while (c) {
				/* XXX character list is ordered? */
				if (c->hdl > start_char_id->inst_id) {
					return c;
				}
				c = LNEXT(c);
			}

			break;
		}
		s = LNEXT(s);
	}

	return NULL;
}

struct GATT_IncludeStru *hlp_gatt_find_next_incl_service(UINT16 conn_id, btgatt_srvc_id_t *srvc_id, btgatt_srvc_id_t *start_incl_srvc_id)
{
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru *s;
	struct GATT_IncludeStru *c;

	if (conn_info == NULL || srvc_id == NULL || start_incl_srvc_id == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return NULL;
	}

	s = conn_info->service_list.head;
	while (s) {
		if (s->hdl == srvc_id->id.inst_id) {
			c = s->include.head;

			while (c) {
				/* XXX include services list is ordered? */
				if (c->hdl > start_incl_srvc_id->id.inst_id) {
					return c;
				}
				c = LNEXT(c);
			}

			break;
		}
		s = LNEXT(s);
	}

	return NULL;
}

struct GATT_DescriptorStru *hlp_gatt_find_next_descriptor(UINT16 conn_id, btgatt_gatt_id_t *char_id, btgatt_gatt_id_t *start_descr_id)
{
	app_ble_conn_info_t * conn_info = conn_info_get_by_conn_id(conn_id);
	struct GATT_ServiceStru * s;
	struct GATT_CharacteristicStru *c;
	struct GATT_DescriptorStru *d;

	if (conn_info == NULL || char_id == NULL || start_descr_id == NULL) {
		BRT_DBG("Can NOT found the connection.");
		return NULL;
	}

	s = conn_info->service_list.head;
	while (s) {
		if (s->hdl <= char_id->inst_id && s->endhdl >= char_id->inst_id) {
			c = s->character.head;
			while (c) {
				if (c->hdl == char_id->inst_id) {
					d = c->descriptor.head;
					while(d) {
						/* XXX descriptor list is ordered? */
						if (d->hdl > start_descr_id->inst_id) {
							return d;
						}
						d = LNEXT(d);
					}
					break;
				}
				c = LNEXT(c);
			}

			break;
		}
		s = LNEXT(s);
	}

	return NULL;
}
