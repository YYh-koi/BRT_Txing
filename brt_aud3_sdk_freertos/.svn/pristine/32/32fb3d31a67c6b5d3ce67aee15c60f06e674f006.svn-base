#include "global.h"
#include "sdap_ui.h"
#include "hid_ui.h"
#include "brt_trace.h"
#include "auto_hid.h"
#include "internal_hid.h"

#ifdef CONFIG_HID
#define TAG "bse_ancs"

static INT16 s_x_current = 0;
static INT16 s_y_current = 0;

static const UCHAR KeyboardHidReportDesc[] = {
	0x05, 0x01,	//Usage Page (Generic Desktop),
	0x09, 0x06,	//Usage (Keyboard),
	0xA1, 0x01,	//Collection (Application),
	0x05, 0x07,	//Usage Page (Key Codes);
	0x19, 0xE0,	//Usage Minimum (224),
	0x29, 0xE7,	//Usage Maximum (231),
	0x15, 0x00,	//Logical Minimum (0),
	0x25, 0x01,	//Logical Maximum (1),
	0x75, 0x01,	//Report Size (1),
	0x95, 0x08,	//Report Count (8),
	0x81, 0x02,	//Input (Data, Variable, Absolute), {LeftCtrl, LeftShift, LeftAlt, LeftWin, RightCtrl, RightShift, RightAlt, RightWin}
	0x95, 0x01,	//Report Count (1),
	0x75, 0x08,	//Report Size (8),
	0x81, 0x01,	//Input (Constant), {Padding}
	0x95, 0x06,	//Report Count (6),
	0x75, 0x08,	//Report Size (8),
	0x15, 0x00,	//Logical Minimum (0),
	0x25, 0x65,	//Logical Maximum(101),
	0x05, 0x07,	//Usage Page (Key Codes),
	0x19, 0x00,	//Usage Minimum (0),
	0x29, 0x65,	//Usage Maximum (101),
	0x81, 0x00,	//Input (Data, Array), {Key arrays}
	0x95, 0x05,	//Report Count (5),
	0x75, 0x01,	//Report Size (1),
	0x05, 0x08,	//Usage Page (Page# for LEDs),
	0x19, 0x01,	//Usage Minimum (1),
	0x29, 0x05,	//Usage Maximum (5),
	0x91, 0x02,	//Output (Data, Variable, Absolute), {NumLock, CapsLock, ScrollLock, Compose, Kana}
	0x95, 0x01,	//Report Count (1),
	0x75, 0x03,	//Report Size (3),
	0x91, 0x01,	//Output (Constant), {Padding}
	0xC0 //End Collection
};

static const UCHAR MouseHidReportDesc[] = {
	//Mouse Report(ID = 0x11)
	0x05, 0x01,            //USAGE_PAGE £¨Generic Desktop)
	0x09, 0x02,            //USAGE (Mouse)
	0xA1, 0x01,            //COLLECTION (Application)
	0x85, 0x11,            //REPORT_ID  £¨17£©
	0x09, 0x01,			   //Usage (Pointer),
	0xA1, 0x00,			   //Collection (Physical)
	0x05, 0x09,            //USAGE_PAGE £¨Button)
	0x19, 0x01,            //USAGE_MINIMUM(Button 1)
	0x29, 0x02,            //USAGE_MAXIMUM(Button 2)
	0x95, 0x02,            //REPORT_COUNT(2)
	0x75, 0x01,            //REPORT_SIZE(1)
	0x25, 0x01,            //LOGICAL_MAXIMUM(1)
	0x81, 0x02,            //INPUT(Data,Var,Abs)
	0x95, 0x06,            //REPORT_COUNT(6)
	0x81, 0x05,            //INPUT(Cnst,Ary,Abs)
	0x05, 0x01 ,           //USAGE_PAGE £¨Generic Desktop)
	0x09, 0x30,            //USAGE(X)
	0x09, 0x31,            //USAGE(Y)
	0x75, 0x10,            //REPORT_SIZE(16)
	0x16, 0x01, 0xF8,      //LOGICAL_MINIMUM(-2047)
	0x26, 0xFF, 0x07,      //LOGICAL_MAXIMUM(2047)
	0x95, 0x02,            //REPORT_COUNT(2)
	0x81, 0x06,            //INPUT(Data,Var,Rel)
	0xC0,					  //END_COLLECTION
	0xC0,                  //END_COLLECTION

	//Keyboard Report(ID = 0x01)
	0x05, 0x0C, 			//Usage Page(Consumer Page)
	0x09, 0x01, 			//Usage(Consumer Control)
	0xA1, 0x01, 			//Collection(Application)
	0x85, 0x01, 		//Report ID(1)
	0x15, 0x00, 		//Logical Min(0)
	0x25, 0x01, 		//Logical Max(1)
	0x75, 0x01, 		//Report Size(1)
	0x95, 0x08, 		//Report Count(6)
	0x0A, 0x23, 0x02,	//Usage(AC Home)
	0x0A, 0x21, 0x02,	//Usage(AC Search)
	0x0A, 0x24, 0x02,	//Usage(AC Back)

	0x09, 0x40, 		//Usage (Menu)
	0x09, 0xE9, 		//Usage (Volume Increment)
	0x09, 0xEA, 		//Usage (Volume Decrement)
	0x81, 0x02, 		//Input(Data,Var,Abs)

	0x05, 0x07, 		//Usage Page(Keyboard/Keypad Page)
	0x75, 0x01, 		//Report Size(1)
	0x95, 0x08, 		//Report Count(2)
	0x09, 0x28, 		//Usage(Keyboard Enter)
	0x09, 0xE0, 		//Usage(Keyboard LeftControl)
	0x81, 0x02, 		//Input(Data,Var,Abs)
	0xC0,

#if 0
	//Touch Report(ID = 2)
	0x05, 0x0D,				//USAGE_PAGE (Digitizers)
	0x09, 0x04, 			//USAGE (Touch Screen)
	0xA1, 0x01,				//COLLECTION (Application)
	0x85, 0x02,				//REPORT_ID (Touch-02)
	0x09, 0x22,				//USAGE (Finger)
	//1st Touch
	0xA1, 0x02,				//COLLECTION (Logical)
	0x09, 0x42,				//USAGE (Tip Switch)
	0x15, 0x00,				//LOGICAL_MINIMUM (0)
	0x25, 0x01,				//LOGICAL_MAXIMUM(1)
	0x75, 0x01,				//Report Size(1)
	0x95, 0x01,				//Report Count(1)
	0x81, 0x02,				//Input(Variable)
	0x09, 0x32,				//USAGE(In Range)
	0x81, 0x02,				//Input(Variable)
	0x09, 0x47,				//USAGE(Reserved)
	0x81, 0x02,				//Input(Variable)
	0x95, 0x05,				//Report Count(5)
	0x81, 0x03,				//Input(Variable,Constant)
	0x09, 0x51,				//USAGE(Contact ID)
	0x75, 0x08,				//Report Size(8)
	0x95, 0x01,				//Report Count(1)
	0x81, 0x02,				//Input(Variable)
	0x05, 0x01,				//USAGE_PAGE £¨Generic Desktop)
	0x75, 0x10,				//REPORT_SIZE (16)
	0x55, 0x00,				//UNIT_EXPONENT (-3)
	0x65, 0x00,				//UNIT(0)
	0x09, 0x30,				//USAGE (X)
	0x35, 0x00,				//PHYSICAL_MINIMUM (0)
	0x46, 0x00, 0x00,		//PHYSICAL_MAXIMUM (0)
	0x16, 0x00, 0x00,		//LOGICAL_MINIMUM(0)
	0x26, 0xE0, 0x01,		//LOGICAL_MAXIMUM(480)
	0x81, 0x02,				//INPUT (Data,Var,Abs)
	0x09, 0x31,				//USAGE (Y)
	0x16, 0x00, 0x00,		//LOGICAL_MINIMUM(0)
	0x26, 0x20, 0x03,		//LOGICAL_MAXIMUM(800)
	0x81, 0x02,				//INPUT (Data,Var,Abs),
	0xC0,					//END_COLLECTION

	//Count
	0x05, 0x0D,				//USAGE_PAGE (Digitizers)
	0x09, 0x54,				//USAGE()
	0x95, 0x01,				//Report Count(1)
	0x75, 0x08,				//Report Size(8)
	0x15, 0x00,				//Logical Min(0)
	0x25, 0x08,				//Logical Max(8)
	0x55, 0x00,				//Unit Exponent(0)
	0x81, 0x02,				//INPUT (Data,Var,Abs),
	0xC0 ,                  //END_COLLECTION
#endif
};

static const UCHAR RemcontrolHidReportDesc[] = {
    // 媒体控制HID报告描述符
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2) - 媒体控制报告
    
    // 媒体控制按键（独立Usage ID方式）
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x09, 0xB5,        //   Usage (Next Track)
    0x09, 0xB6,        //   Usage (Previous Track)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)  
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0xB7,        //   Usage (Stop)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x07,        //   Report Count (7) - 7个按键
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // 填充位
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x01,        //   Report Size (1) 
    0x81, 0x01,        //   Input (Const,Array,Abs)
    
    0xC0               // End Collection
};

void app_hid_init(module_event_cbk_t p_event_cbk)
{
	if (hlp_hid_get_status() == ST_APP_HID_UNINIT) {
		hlp_hid_init(p_event_cbk);
	}
}

UINT32 app_hid_get_status(void)
{
	return hlp_hid_get_status();
}

void app_hid_server_start(void)
{
	struct AppHidInfo hid_server_info;

	if (hlp_hid_get_status() == ST_APP_HID_INIT) {
		hid_server_info.sub_cls = HID_SUBCLASS_REMCONTROL;
		switch (hid_server_info.sub_cls) {
		case HID_SUBCLASS_KEYBOARD:
			hid_server_info.desc_list = (UINT8 *)KeyboardHidReportDesc;
			hid_server_info.desc_len = sizeof(KeyboardHidReportDesc);
			break;
		case HID_SUBCLASS_POINT:
			hid_server_info.desc_list = (UINT8 *)MouseHidReportDesc;
			hid_server_info.desc_len = sizeof(MouseHidReportDesc);
			break;
        case HID_SUBCLASS_REMCONTROL:
			hid_server_info.desc_list = (UINT8 *)RemcontrolHidReportDesc;
			hid_server_info.desc_len = sizeof(RemcontrolHidReportDesc);
			break;
		default:
			bse_log_error(TAG, ">>>>>>>>>>>>>>>>> app_hid_server_start Sub Class Error!\r\n");
			return;
		}

		hlp_hid_server_start(&hid_server_info);
	}
}

void app_hid_server_start_by_user_param(UINT32 device_type, UINT8 *user_desc, UINT32 len)
{
	struct AppHidInfo hid_server_info;

	if (hlp_hid_get_status() == ST_APP_HID_INIT) {
		memset(&hid_server_info, 0, sizeof(hid_server_info));

		hid_server_info.sub_cls = device_type;
		if (user_desc == NULL || len == 0) { /* Default Report Desciptor list */
			switch (hid_server_info.sub_cls) {
			case HID_SUBCLASS_KEYBOARD:
				hid_server_info.desc_list = (UINT8 *)KeyboardHidReportDesc;
				hid_server_info.desc_len = sizeof(KeyboardHidReportDesc);
				break;
			case HID_SUBCLASS_POINT:
				hid_server_info.desc_list = (UINT8 *)MouseHidReportDesc;
				hid_server_info.desc_len = sizeof(MouseHidReportDesc);
				break;
			default:
				bse_log_error(TAG, ">>>>>>>>>>>>>>>>> app_hid_server_start Sub Class Error!\r\n");
				return;
			}
		}

		hlp_hid_server_start(&hid_server_info);
	}
}

void app_hid_server_stop(void)
{
	//TODO
}

void app_hid_done(void)
{
	if (hlp_hid_get_status() >= ST_APP_HID_READY) {
		hlp_hid_server_stop();
	}
	if (hlp_hid_get_status() == ST_APP_HID_INIT) {
		hlp_hid_done();
	}
}

void app_hid_connect(UINT8* bd_addr)
{
	if ((hlp_hid_get_status() == ST_APP_HID_READY) && (bd_addr != NULL)) {

	    bse_log_info(TAG, "app_hid_connect bd_addr[%02X%02X%02X%02X%02X%02X]\r\n", 
	            bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
		struct HCI_AddressStru addr_stru;

		memset(&addr_stru, 0, sizeof(addr_stru));
		addr_stru.cap = HCI_ADDRESS_CAP_BREDR;
		addr_stru.atype = HCI_OWN_ADDRESS_TYPE_PUBLIC_DEVICE_ADDRESS;
		memcpy(addr_stru.bd, bd_addr, sizeof(addr_stru.bd));

		hlp_hid_set_status(ST_APP_HID_CONNECTING);
		hlp_hid_cable_plug(hlp_hid_new_conn_info(&addr_stru));
		hlp_hid_connect(&addr_stru);
	}
}

void app_hid_disconnect(HANDLE conn_hdl)
{
	if (hlp_hid_get_status() >= ST_APP_HID_CONNECTED) {
		hlp_hid_disconnect(conn_hdl);
	}
}

void app_hid_send_mouse_data(UINT8 button, INT16 x, INT16 y)
{
	if (hlp_hid_get_status() >= ST_APP_HID_CONNECTED) {
		struct AppHidMouseDataStru mouse_data;

		mouse_data.report_id = 0x11;
		mouse_data.button = button;
		mouse_data.x = x - s_x_current;
		mouse_data.y = y - s_y_current;

		hlp_hid_data_req(&mouse_data, sizeof(struct AppHidMouseDataStru));
		s_x_current = x;
		s_y_current = y;

		if (button == 0) {
			mouse_data.x = MIN_X_OF_MOUSE;
			mouse_data.y = MIN_Y_OF_MOUSE;
			hlp_hid_data_req(&mouse_data, sizeof(struct AppHidMouseDataStru));
			s_x_current = 0;
			s_y_current = 0;
		}
	}
}

void app_hid_send_key_value(UINT8 key_value)
{
	if (hlp_hid_get_status() >= ST_APP_HID_CONNECTED) {
		struct AppHidKeyboardDataStru key_data;

		key_data.report_id = 0x02;
		if(key_value > 0) {
		    key_data.key_value = (0x01 << (key_value - 1));
        } else {
            key_data.key_value = 0x0;
        }
		bse_log_print("hid_key", "\r\n", "key_data.key_value:0x%X\n", key_data.key_value);
		hlp_hid_data_req(&key_data, sizeof(struct AppHidKeyboardDataStru));
	}
}

void app_hid_send_media_value(UINT8 comm_value)
{
	if (hlp_hid_get_status() >= ST_APP_HID_CONNECTED) {

		bse_log_print("hid_command", "\r\n", "comm_value :%d\n", comm_value);

		struct AppHidMediaDataStru command_data;

		command_data.report_id = 0x02;
		/*if(comm_value > 0) {
		    command_data.command_value = comm_value;
        } else {
            command_data.command_value = 0x0;
        }*/
		switch (comm_value)
		{
		case HID_MEDIA_KEY_REALSE:
			command_data.command_value = 0x0;
			break;
		case HID_MEDIA_KEY_PLAY_PUASE:
			command_data.command_value = 0x01;
			break;
		case HID_MEDIA_KEY_NEXT:
			command_data.command_value = 0x02;
			break;
		case HID_MEDIA_KEY_PREV:
			command_data.command_value = 0x04;
			break;
		case HID_MEDIA_KEY_SABSUP:
			command_data.command_value = 0x08;
			break;
		case HID_MEDIA_KEY_SABSDOWN:
			command_data.command_value = 0x10;
			break;
		default:
			command_data.command_value = 0x0;
			break;
		}
		bse_log_print("hid_command", "\r\n", "command_data.command_value:%d\n", command_data.command_value);
		hlp_hid_data_req(&command_data, sizeof(struct AppHidMediaDataStru));
	}
}

profile_t g_hid_profile = {
	app_hid_init,
	app_hid_done,
	app_hid_server_start,
	app_hid_server_stop,
};

#endif /* CONFIG_HID */

