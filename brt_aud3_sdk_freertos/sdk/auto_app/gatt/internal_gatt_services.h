/*This file caches the Gatt Services */

#ifndef _HLP_GATT_SERVICES_H__
#define _HLP_GATT_SERVICES_H__

#include "btdatatype.h"
#include "bt_gatt_types.h"
#include "btlist.h"
#include "att_ui.h"
#include "bt_utils.h"

typedef struct tag_gatt_element_servide {
	UINT16      app_handle; /* handle exported to Application */ //Now just used in local SDK
	UINT16      handle; /* handle to reference data in stack */
	UINT16      end_handle;
	UINT8       svc_type;
	UINT16      inst_id;
	bt_uuid_t   uuid;
	struct ListStru include;
	struct ListStru character;
} gatt_element_srvc_t;

typedef struct tag_gatt_element_include {
	UINT16      app_handle; /* handle exported to Application */ //Now just used in local SDK
	UINT16      handle; /* handle to reference data in stack */
	UINT16      svc_handle;//app_handle
	UINT16      incl_svc_handle;//app_handle
} gatt_element_incl_t;

typedef struct tag_gatt_element_character {
	UINT16      app_handle; /* handle exported to Application */ //Now jusy used in local SDK
	UINT16      handle; /* handle to reference data in stack */
	UINT16      value_hdl;
	UINT16      svc_handle; //app_handle
	bt_uuid_t   uuid;
	UINT8       properties;
	UINT16      permissions;
	struct ListStru descriptor;
	UINT8       val[256];
	UINT16      val_len;
} gatt_element_char_t;

typedef struct tag_gatt_element_descriptor {
	UINT16      app_handle; /* handle exported to Application */
	UINT16      handle; /* handle to reference data in stack */
	UINT16      svc_handle; //app_handle
	bt_uuid_t   uuid;
	UINT16      permissions;
} gatt_element_desc_t;

void gatt_services_init();
UINT16 gatt_services_add_service(UINT8 server_if, btgatt_srvc_id_t srv);
UINT16 gatt_services_add_include(UINT8 server_if, UINT16 svc_handle, UINT16 incl_svc_handle);
UINT16 gatt_services_add_character(UINT8 server_if, UINT16 svc_handle, bt_uuid_t *uuid, UINT8 properties, UINT16 permissions,UINT8* val, UINT16 val_len);
UINT16 gatt_services_add_descriptor(UINT8 server_if, UINT16 svc_handle, bt_uuid_t *uuid, UINT16 permissions);
gatt_element_srvc_t * gatt_services_get_service_by_app_handle(UINT8 server_if, UINT16 svc_handle);
gatt_element_srvc_t * gatt_services_get_service_by_stack_handle(UINT8 server_if, UINT16 handle);
gatt_element_srvc_t * gatt_services_find_service(struct GATT_ServiceStru *service);
void gatt_services_delete_service(UINT8 server_if, UINT16 svc_handle);
void gatt_services_update_handles(struct GATT_ServiceStru *service);
UINT16 gatt_services_get_stack_handle(UINT8 server_if, UINT16 app_handle);
UINT16 gatt_services_get_app_handle(UINT16 handle);
UINT8 gatt_services_get_attribute_type_by_stack_handle(UINT16 handle);

#endif  /*_HLP_GATT_SERVICES_H__*/
