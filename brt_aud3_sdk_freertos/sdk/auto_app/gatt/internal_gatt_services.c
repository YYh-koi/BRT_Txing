#include "att_ui.h"
#include "internal_gatt_services.h"
#include "internal_gatt_utils.h"
#define LOG_TAG "hlp_gatt_services"
#include "brt_trace.h"
#include "bt_common_types.h"

#define LOG(fmt, ...) bse_log_print(LOG_TAG,"\r", fmt, ## __VA_ARGS__)

typedef struct tag_gatt_services {
	UINT8 app_if;
	struct ListStru service_list;
} gatt_services_t;

static UINT16 handle_ref;
/* App add sevice and its attributes in order. We can use a pointer to mark the current node.*/
void * cur_node;
static gatt_services_t s_services[BTGATT_MAX_SERVER_APP_NUM];

void gatt_services_init()
{
	int i;

	for(i=0; i<BTGATT_MAX_SERVER_APP_NUM; i++) {
		memset(&s_services[i], 0, sizeof(gatt_services_t));
		s_services[i].app_if = BTGATT_APP_IF_INVALID;
	}

	handle_ref = 0;
	cur_node = NULL;
}

UINT16 gatt_services_add_service(UINT8 server_if, btgatt_srvc_id_t srv)
{
	int i;
	gatt_services_t *srvs = NULL;
	gatt_element_srvc_t *svc;

	if (server_if > BTGATT_MAX_SERVER_APP_NUM) {
		return 0;
	}

	for(i=0; i<BTGATT_MAX_SERVER_APP_NUM; i++) {
		if (s_services[i].app_if == server_if) {
			srvs = &s_services[i];
			break;
		}
	}

	if (srvs == NULL) {
		for(i=0; i<BTGATT_MAX_SERVER_APP_NUM; i++) {
			// pick up the first available service
			if (s_services[i].app_if == BTGATT_APP_IF_INVALID) {
				srvs = &s_services[i];
				break;
			}
		}
	}

	if (srvs == NULL) {
		return 0;
	}

	srvs->app_if = server_if;
	svc = (gatt_element_srvc_t *) List_NodeNew(sizeof(gatt_element_srvc_t));
	svc->app_handle = ++handle_ref;
	svc->svc_type = srv.is_primary;
	svc->inst_id = srv.id.inst_id;
	memcpy(svc->uuid.uu, srv.id.uuid.uu, 16);
	cur_node = svc;
	List_AddTail(&srvs->service_list, svc);

	return svc->app_handle;
}

UINT16 gatt_services_add_include(UINT8 server_if, UINT16 svc_handle, UINT16 incl_svc_handle)
{
	gatt_element_srvc_t *svc = gatt_services_get_service_by_app_handle(server_if, svc_handle);
	gatt_element_srvc_t *incl_svc = gatt_services_get_service_by_app_handle(server_if, incl_svc_handle);
	gatt_element_incl_t *include;

	if (svc == NULL) {
		return 0;
	}

	if (incl_svc == NULL) {
		// FIXME
		LOG("Can NOT find include service(0x%04X).", incl_svc_handle);
	}

	include = (gatt_element_incl_t *) List_NodeNew(sizeof(gatt_element_incl_t));
	List_AddTail(&(svc->include), include);
	include->app_handle = ++handle_ref;
	include->svc_handle = svc_handle;
	include->incl_svc_handle = incl_svc_handle;

	return include->app_handle;
}

UINT16 gatt_services_add_character(UINT8 server_if, UINT16 svc_handle, bt_uuid_t *uuid, UINT8 properties, UINT16 permissions,UINT8* val, UINT16 val_len)
{
	gatt_element_srvc_t *svc = gatt_services_get_service_by_app_handle(server_if, svc_handle);
	gatt_element_char_t *character;

	if (svc == NULL) {
		return 0;
	}

	character = (gatt_element_char_t *) List_NodeNew(sizeof(gatt_element_char_t));
	List_AddTail(&svc->character, character);
	character->app_handle = ++handle_ref;
	character->svc_handle = svc_handle;
	character->properties = properties;
	character->permissions = permissions;

	memcpy(character->val,val,val_len>255?255:val_len);
	character->val_len = val_len>255?255:val_len;

	cur_node = character;
	memcpy(&(character->uuid), uuid, sizeof(bt_uuid_t));

	return character->app_handle;
}


UINT16 gatt_services_add_descriptor(UINT8 server_if, UINT16 svc_handle, bt_uuid_t *uuid, UINT16 permissions)
{
	gatt_element_srvc_t *svc = gatt_services_get_service_by_app_handle(server_if, svc_handle);
	gatt_element_char_t *character = (gatt_element_char_t *) cur_node;
	gatt_element_desc_t *descriptor;

	if (svc == NULL || character == NULL) {
		return 0;
	}

	/* avoid duplicate descriptor */
	descriptor = character->descriptor.head;
	while (descriptor) {
		if (memcmp(&(descriptor->uuid), uuid, sizeof(bt_uuid_t)) == 0) {
			return descriptor->app_handle;
		}
		descriptor = LNEXT(descriptor);
	}

	descriptor = (gatt_element_desc_t *) List_NodeNew(sizeof(gatt_element_desc_t));
	List_AddTail(&character->descriptor, descriptor);
	descriptor->app_handle = ++handle_ref;
	descriptor->svc_handle = svc_handle;
	descriptor->permissions = permissions;
	memcpy(&(descriptor->uuid), uuid, sizeof(bt_uuid_t));

	return descriptor->app_handle;
}

void gatt_services_delete_service(UINT8 server_if, UINT16 svc_handle)
{
	int i;
	gatt_services_t *srvs = NULL;
	gatt_element_srvc_t * svc;
	gatt_element_char_t *c;
	gatt_element_desc_t *d;

	for(i=0; i<BTGATT_MAX_SERVER_APP_NUM; i++) {
		if (s_services[i].app_if == server_if) {
			srvs = &s_services[i];
			break;
		}
	}

	svc = gatt_services_get_service_by_app_handle(server_if, svc_handle);

	if (svc == NULL) {
		return;
	}

	List_RemoveAll(&svc->include);/* Clear include */
	while ((c = List_RemoveHead(&svc->character)) != NULL) {
		while ((d = List_RemoveHead(&c->descriptor)) != NULL) {
			LFREE(d);
		}
		LFREE(c);
	}

	List_RemoveAt(&srvs->service_list, svc);
	LFREE(svc);
}

gatt_element_srvc_t *gatt_services_get_service_by_app_handle(UINT8 server_if, UINT16 svc_handle)
{
	int i;
	gatt_services_t *srvs = NULL;
	gatt_element_srvc_t *svc;

	if (server_if > BTGATT_MAX_SERVER_APP_NUM) {
		return NULL;
	}

	for(i=0; i<BTGATT_MAX_SERVER_APP_NUM; i++) {
		if (s_services[i].app_if == server_if) {
			srvs = &s_services[i];
			break;
		}
	}

	if (srvs == NULL) {
		return NULL;
	}

	svc = srvs->service_list.head;

	while (svc) {
		if (svc->app_handle == svc_handle) {
			return svc;
		}

		svc = LNEXT(svc);
	}

	return NULL;
}

gatt_element_srvc_t *gatt_services_get_service_by_stack_handle(UINT8 server_if, UINT16 handle)
{
	int i;
	gatt_services_t *srvs = NULL;
	gatt_element_srvc_t *svc;
	if (server_if > BTGATT_MAX_SERVER_APP_NUM) {
		return NULL;
	}
	
	for(i=0; i<BTGATT_MAX_SERVER_APP_NUM; i++) {
		if (s_services[i].app_if == server_if) {
			srvs = &s_services[i];
			break;
		}
	}

	if (srvs == NULL) {
		return NULL;
	}

	svc = srvs->service_list.head;
	
	while (svc) {
		svc->handle == handle;
		return svc;
	}

	return NULL;
}

static struct GATT_CharacteristicStru *find_character_in_service(struct GATT_ServiceStru *service, gatt_element_char_t *crt,int index)
{
	struct GATT_CharacteristicStru *character = service->character.head;
	bt_uuid_t out_uuid;
	revert_uuid(crt->uuid.uu, out_uuid.uu);

	while((character!=NULL) && (index != 0))
	{
		character = LNEXT(character);
		index --;
	}

	while (character) {
		//if (memcmp(character->uuid.u, crt->uuid.uu, 16) == 0)
		if (memcmp(character->uuid.u, out_uuid.uu, 16) == 0) {
			break;
		}

		character = LNEXT(character);
	}

	return character;
}

static struct GATT_DescriptorStru *find_descriptor_in_character(struct GATT_CharacteristicStru *character, gatt_element_desc_t *dsc)
{
	struct GATT_DescriptorStru *descriptor = character->descriptor.head;
	bt_uuid_t out_uuid;
	revert_uuid(dsc->uuid.uu, out_uuid.uu);

	while (descriptor) {
		//if (memcmp(descriptor->uuid.u, dsc->uuid.uu, 16) == 0)
		if (memcmp(descriptor->uuid.u, out_uuid.uu, 16) == 0) {
			break;
		}

		descriptor = LNEXT(descriptor);
	}

	return  descriptor;
}

static UINT16 find_include_handle(gatt_services_t *srvs, gatt_element_incl_t *inc)
{
	gatt_element_srvc_t *svc = srvs->service_list.head;

	while (svc) {
		/* XXX if (svc->app_handle == inc->svc_handle) */
		if (svc->app_handle == inc->incl_svc_handle) {
			return svc->handle;
		}

		svc = LNEXT(svc);
	}

	return 0;
}

static struct GATT_IncludeStru *find_include_in_service(struct GATT_ServiceStru *service, UINT16 incl_handle)
{
	struct GATT_IncludeStru *include = service->include.head;

	while (include) {
		if (include->include_hdl == incl_handle) {
			break;
		}

		include = LNEXT(include);
	}

	return include;
}

gatt_element_srvc_t * gatt_services_find_service(struct GATT_ServiceStru *service)
{
	int k = 0;
	gatt_services_t *srvs;
	gatt_element_srvc_t *svc = NULL;
	bt_uuid_t out_uuid;

	for(k = 0; k < BTGATT_MAX_SERVER_APP_NUM; k++) {
		if (s_services[k].app_if != BTGATT_APP_IF_INVALID) {
			srvs = &s_services[k];
			svc = srvs->service_list.head;
			revert_uuid(service->uuid.u, out_uuid.uu);

			while (svc) {
				if (memcmp(svc->uuid.uu, out_uuid.uu, 16) == 0) {
					svc->handle = service->hdl;
					svc->end_handle = service->endhdl;
					break;
				}

				svc = LNEXT(svc);
			}
		}

		if (svc) {
			return svc;
		}
	}
	return NULL;
}

void gatt_services_update_handles(struct GATT_ServiceStru *service)
{
	int k;
	gatt_services_t *srvs;
	gatt_element_srvc_t *svc = NULL;
	gatt_element_incl_t *inc = NULL;
	gatt_element_char_t *crt = NULL;
	gatt_element_desc_t *dsc = NULL;
	struct GATT_IncludeStru *i;
	struct GATT_CharacteristicStru *c;
	struct GATT_DescriptorStru *d;
	bt_uuid_t out_uuid;

	if (service == NULL) {
		return;
	}

	for(k=0; k<BTGATT_MAX_SERVER_APP_NUM; k++) {
		if (s_services[k].app_if != BTGATT_APP_IF_INVALID) {
			srvs = &s_services[k];
			svc = srvs->service_list.head;
			revert_uuid(service->uuid.u, out_uuid.uu);

			while (svc) {
				//if (memcmp(svc->uuid.uu, service->uuid.u, 16) == 0)
				if (memcmp(svc->uuid.uu, out_uuid.uu, 16) == 0) {
					svc->handle = service->hdl;
					svc->end_handle = service->endhdl;
					break;
				}

				svc = LNEXT(svc);
			}
		}

		if (svc) {
			break;
		}
	}

	if (svc) {
		crt = svc->character.head;
		int characteristic_index = 0;
		while (crt) {

			c = find_character_in_service(service, crt,characteristic_index);
			characteristic_index++;
			
			if (!c) {
				crt = LNEXT(crt);
				continue;
			}

			crt->handle = c->hdl;
			crt->value_hdl = c->value_hdl;
			dsc = crt->descriptor.head;

			LOG("c->hdl =%d c->value_hdl= %d", c->hdl,c->value_hdl);

			while (dsc) {
				d = find_descriptor_in_character(c, dsc);

				if (!d) {
					dsc = LNEXT(dsc);
					continue;
				}

				dsc->handle = d->hdl;
				dsc = LNEXT(dsc);
			}
			crt = LNEXT(crt);
		}

		inc = svc->include.head;

		while (inc) {
			i = find_include_in_service(service, find_include_handle(srvs, inc));

			if (i) {
				inc->handle = i->hdl;
			}

			inc = LNEXT(inc);
		}
	}
}

UINT16 gatt_services_get_stack_handle(UINT8 server_if, UINT16 app_handle)
{
	int k;
	gatt_services_t *srvs;
	gatt_element_srvc_t *svc = NULL;
	gatt_element_incl_t *inc = NULL;
	gatt_element_char_t *crt = NULL;
	gatt_element_desc_t *dsc = NULL;

	for(k=0; k<BTGATT_MAX_SERVER_APP_NUM; k++) {
		if (s_services[k].app_if == server_if) {
			srvs = &s_services[k];
			svc = srvs->service_list.head;

			while (svc) {
				if (svc->app_handle == app_handle) {
					return svc->handle;
				}

				crt = svc->character.head;

				while (crt) {
					if (crt->app_handle == app_handle) {
						return crt->handle;
					}

					dsc = crt->descriptor.head;

					while (dsc) {
						if (dsc->app_handle == app_handle) {
							return dsc->handle;
						}

						dsc = LNEXT(dsc);
					}
					crt = LNEXT(crt);
				}

				inc = svc->include.head;

				while (inc) {
					if (inc->app_handle == app_handle) {
						return inc->handle;
					}

					inc = LNEXT(inc);
				}

				svc = LNEXT(svc);
			}

			break;
		}
	}

	return 0;
}

UINT16 gatt_services_get_app_handle(UINT16 handle)
{
	int k;
	gatt_services_t *srvs;
	gatt_element_srvc_t *svc = NULL;
	gatt_element_incl_t *inc = NULL;
	gatt_element_char_t *crt = NULL;
	gatt_element_desc_t *dsc = NULL;

	for(k=0; k<BTGATT_MAX_SERVER_APP_NUM; k++) {
		if (s_services[k].app_if != BTGATT_APP_IF_INVALID) {
			srvs = &s_services[k];
			svc = srvs->service_list.head;

			while (svc) {
				if (svc->handle == handle) {
					return svc->app_handle;
				}

				crt = svc->character.head;

				while (crt) {
					if (crt->handle == handle || crt->value_hdl == handle) {
						return crt->app_handle;
					}

					dsc = crt->descriptor.head;

					while (dsc) {
						if (dsc->handle == handle) {
							return dsc->app_handle;
						}

						dsc = LNEXT(dsc);
					}
					crt = LNEXT(crt);
				}

				inc = svc->include.head;

				while (inc) {
					if (inc->handle == handle) {
						return inc->app_handle;
					}

					inc = LNEXT(inc);
				}

				svc = LNEXT(svc);
			}
		}
	}

	return 0;
}

UINT8 gatt_services_get_attribute_type_by_stack_handle(UINT16 handle)
{
	int k = 0;
	gatt_services_t *srvs;
	gatt_element_srvc_t *svc = NULL;
	gatt_element_incl_t *inc = NULL;
	gatt_element_char_t *crt = NULL;
	gatt_element_desc_t *dsc = NULL;
	
	for(k = 0; k < BTGATT_MAX_SERVER_APP_NUM; k++) {
		if (s_services[k].app_if != BTGATT_APP_IF_INVALID) {
			srvs = &s_services[k];
			svc = srvs->service_list.head;

			while (svc) {
				if (svc->handle == handle) {
					if (svc->svc_type == BTGATT_DB_PRIMARY_SERVICE) {
						return BTGATT_DB_PRIMARY_SERVICE;
					} else {
						return BTGATT_DB_SECONDARY_SERVICE;
					}
				}

				crt = svc->character.head;

				while (crt) {
					if (crt->handle == handle || crt->value_hdl == handle) {
						return BTGATT_DB_CHARACTERISTIC;
					}

					dsc = crt->descriptor.head;

					while (dsc) {
						if (dsc->handle == handle) {
							return BTGATT_DB_DESCRIPTOR;
						}

						dsc = LNEXT(dsc);
					}
					crt = LNEXT(crt);
				}

				inc = svc->include.head;

				while (inc) {
					if (inc->handle == handle) {
						return BTGATT_DB_INCLUDED_SERVICE;
					}

					inc = LNEXT(inc);
				}

				svc = LNEXT(svc);
			}
		}
	}

	return 0xFF;
}

