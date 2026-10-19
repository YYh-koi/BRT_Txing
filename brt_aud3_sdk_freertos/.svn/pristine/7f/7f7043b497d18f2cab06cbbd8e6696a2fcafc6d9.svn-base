#include "bt_manager.h"
#include <unistd.h>
#include "brt_trace.h"
#include "profile.h"
#include "profiles_def.h"
#include "bt_functions.h"
#include "Auto_at_internal.h"
#include "internal_manager.h"
#include "auto_gatt_event.h"
#include "att_uuid.h"
#include "ey_extent.h"

#define TAG "bse_ey"

void ey_event_cbk_t(unsigned int event,  void *param);
module_event_cbk_t ey_event_cb = ey_event_cbk_t;

typedef struct {
	bt_uuid_t  uuid;
	uint16_t handle;	
} headle_uuid_map;

headle_uuid_map headle_uuid_table[32];
int headle_uuid_cnt  = 0;


void ey_event_cbk_t(unsigned int event,  void *param)
{
	bse_log_api(TAG,"ey_event_cbk_t, event=%d\r\n", event);
	
	switch (event)
	{
		  case EV_GATT_SERVICE_ADDED:
		  {
			 btgatt_db_element_t  *element = (btgatt_db_element_t*)param;
			 bse_log_api(TAG,"handle = 0x%04X, uuid=0x%02X, 0x%02X", element->attribute_handle, element->uuid.uu[2],  element->uuid.uu[3]);
			 headle_uuid_table[headle_uuid_cnt].handle= element->attribute_handle;
			 memcpy(headle_uuid_table[headle_uuid_cnt].uuid.uu, element->uuid.uu, 16);
			 bse_log_api(TAG,"table handle = 0x%04X, uuid=0x%02X, 0x%02X", element->attribute_handle, headle_uuid_table[headle_uuid_cnt].uuid.uu[2],  headle_uuid_table[headle_uuid_cnt].uuid.uu[3]);
			 headle_uuid_cnt++; 	
		  }
		  break;
		   case EV_GATT_SERVER_READ_CHARACTERISTIC:
		  {
			 evt_param_server_read_characteristic_t *msg = (evt_param_server_read_characteristic_t*)param;

			   bt_uuid_t  tmp_uuid;
			    for(int i=0; i<headle_uuid_cnt; i++ ) {
				 if(headle_uuid_table[i].handle == (msg->handle - 1)) {
					tmp_uuid = headle_uuid_table[i].uuid;
				 }
			   }
			     bse_log_api(TAG,"zjgtest handle = 0x%04X, uuid=0x%02X, 0x%02X\r\n", msg->handle, tmp_uuid.uu[2], tmp_uuid.uu[3]);
				if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA5) {
					if((ey_wifi_info.wifi_go_data == '\0')|| (strncmp(ey_wifi_info.wifi_go_data,"000000",6) == 0)) {
						#if 0 //if use ey, phone is used wifi ap, open this branch
						unsigned char a5_val[64] = {0} ;
						UINT8 *p_bd = hlp_manager_get_local_bd_addr();
						sprintf(a5_val, "%02X%02X%02X%02X%02X%02X&%s", p_bd[5], p_bd[4], p_bd[3], p_bd[2], p_bd[1], p_bd[0] - 1, "EYMOTOR12FCB99FD224DB2B22F5A7703");
						bse_log_api(TAG,"a5_val:%s, a5_val len=%d\r\n", a5_val, strlen(a5_val));
						app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0, strlen(a5_val), a5_val);
						#else
						app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0, 0, 0);
						#endif
						
					} else {
						app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0, strlen(ey_wifi_info.wifi_go_data), ey_wifi_info.wifi_go_data);
					}
				}else if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA1) {
					app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0,strlen(ey_wifi_info.wifi_ip_addr), ey_wifi_info.wifi_ip_addr);
				}else if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA7) {
					UINT8 *a7_val;
					int a7_val_len = strlen(ey_wifi_info.wifi_status) /2;
					a7_val =  (UINT8 *)NEW(a7_val_len);
					hex_string_to_byte_array(strlen(ey_wifi_info.wifi_status), ey_wifi_info.wifi_status , a7_val);
					app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0, a7_val_len, a7_val);
					FREE(a7_val);
				} else {
					app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0,0,0);
				}
		  }
		  break;
		   case EV_GATT_SERVER_WRITE_CHARACTERISTIC:
		  {
		  	evt_param_server_write_characteristic_t *msg = (evt_param_server_write_characteristic_t*)param;
			    bt_uuid_t  tmp_uuid;
			    for(int i=0; i<headle_uuid_cnt; i++ ) {
				 if(headle_uuid_table[i].handle == (msg->handle - 1)) {
					tmp_uuid = headle_uuid_table[i].uuid;
				 }
			   }
			   bse_log_api(TAG,"zjgtest uuid1 = %02x, uuid2 = %02x\r\n", tmp_uuid.uu[2], tmp_uuid.uu[3]);	
			  if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA2) {
			  	bse_log_api(TAG,"zjgtest len = %d, ssid:%s\r\n", msg->data_len, msg->data);				
				bt_event_indicate(BT_EV_EY_WIFI_SSID, msg->data);
			  } else if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA3){
			       bse_log_api(TAG,"zjgtest len = %d, passwd:%s\r\n", msg->data_len, msg->data);
				bt_event_indicate(BT_EV_EY_WIFI_PASSWD, msg->data);
			  }else if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA4){
			       bse_log_api(TAG,"zjgtest len = %d, phone ap status:%s\r\n", msg->data_len, msg->data);
				bt_event_indicate(BT_EV_EY_WIFI_READY, msg->data);
			  }else if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA5){
			       bse_log_api(TAG,"zjgtest len = %d, phone type:%s\r\n", msg->data_len, msg->data);
				 UINT8 *AAA5_data;
				AAA5_data = (UINT8 *)NEW((msg->data_len * 2) + 1);
				byte_array_to_hex_string(AAA5_data, msg->data, msg->data_len);
				bt_event_indicate(BT_EV_EY_WIFI_PHONE_TYPE, AAA5_data);
				FREE(AAA5_data);
			  }else if(tmp_uuid.uu[2] ==  0xAA && tmp_uuid.uu[3] == 0xA6){
				bse_log_api(TAG,"zjgtest len = %d\r\n", msg->data_len);
				UINT8 *AAA6_data;
				AAA6_data = (UINT8 *)NEW((msg->data_len * 2) + 1);
				byte_array_to_hex_string(AAA6_data, msg->data, msg->data_len);
				bt_event_indicate(BT_EV_EY_WIFI_PHONE_MESSAGE, AAA6_data);
				FREE(AAA6_data);
			  }
		   }
		   break;
            
	}
}

void ey_set_ap_ssid(UINT8 *uuid_p, UCHAR *str)
{
	memset(ey_wifi_info.wifi_ssid, 0, sizeof(ey_wifi_info.wifi_ssid));
	strcpy(ey_wifi_info.wifi_ssid, str);
	bse_log_api(TAG,"ey_wifi_info.wifi_ssid:[%s]\r\n", ey_wifi_info.wifi_ssid);
}

void ey_set_ap_passwd(UINT8 *uuid_p, UCHAR *str)
{
	memset(ey_wifi_info.wifi_passwd, 0, sizeof(ey_wifi_info.wifi_passwd));
	strcpy(ey_wifi_info.wifi_passwd, str);
	bse_log_api(TAG,"ey_wifi_info.wifi_passwd:[%s]\r\n", ey_wifi_info.wifi_passwd);
}

void ey_set_ap_connect_status(UINT8 *uuid_p, UCHAR *str)
{
	memset(ey_wifi_info.wifi_status, 0, sizeof(ey_wifi_info.wifi_status));
	strcpy(ey_wifi_info.wifi_status, str);
	bse_log_api(TAG,"ey_wifi_info.wifi_status:[%s]\r\n", ey_wifi_info.wifi_status);
}

void ey_set_ip_addr(UINT8 *uuid_p, UCHAR *str)
{
	memset(ey_wifi_info.wifi_ip_addr, 0, sizeof(ey_wifi_info.wifi_ip_addr));
	strcpy(ey_wifi_info.wifi_ip_addr, str);
	bse_log_api(TAG,"ey_wifi_info.wifi_ip_addr:[%s]\r\n", ey_wifi_info.wifi_ip_addr);
}

void ey_set_p2p_go_data(UINT8 *uuid_p, UCHAR *str)
{
	memset(ey_wifi_info.wifi_go_data, 0, sizeof(ey_wifi_info.wifi_go_data));
	strcpy(ey_wifi_info.wifi_go_data, str);
	bse_log_api(TAG,"ey_wifi_info.wifi_go_data:[%s]\r\n", ey_wifi_info.wifi_go_data);
}