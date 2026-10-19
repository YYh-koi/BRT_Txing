#include <unistd.h>
#include "auto_at.h"
#include "auto_at_internal.h"
#include "bt_event.h"
#include "btdatatype.h"
#include "bt_utils.h"
#include "brt_trace.h"
#include "porting_utils.h"
#include "auto_gap.h"
#include "auto_hf.h"
#include "auto_a2dp.h"
#include "auto_avrcp.h"
#include "auto_pbap.h"
#include "cmd_composition.h"
#include "cmd_operations.h"
#include <errno.h>
#include "internal_manager.h"
#ifdef CONFIG_FTP
#include "auto_ftp.h"
#endif

#ifdef CONFIG_SPP
#include "app_rfcomm.h"
#endif
#ifdef CONFIG_CBP
#include "auto_cb_screen.h"
#endif

#ifdef CONFIG_SPP_ANDROID
#include "debug_utils.h"
#endif
#include "bt_power_manager.h"
#include "brtbt_interface.h"

#include "cmd_composition.h"
#ifdef CONFIG_CHECK_LICENSE                                                                                             
#include "auto_license.h"
#endif

#ifdef CONFIG_ANCS_CLIENT
#include "ancs_client.h"
#endif
#define TAG "bse_at"

#ifdef CONFIG_ANCS_CLIENT
#include "ancs_client.h"
#endif

#ifdef CONFIG_GATT
#include "bt_common_types.h"
#include "auto_gatt_event.h"
#endif

#define TAG "bse_at"
static at_event_callback s_event_cb = NULL;
//static at_event_len_callback s_event_len_cb = NULL;
static SYSTEM_LOCK s_event_lock;
extern BOOL g_bt_opened;

extern char pbap_at_buff[5][32];
extern int pb_at_index;
extern int w_max_items; 
extern int w_pb_offset; 
extern int w_type;
extern int w_offset;

extern int d_disable_state;

static int w_pb_number = 0;


/*
static int pbap_cch_id = 0;
static int pbap_sim_id = 0;
static int pbap_iphone_id = 0;
*/
static void get_remote_device_name(const unsigned char *bd, char *name, int len)
{
    if (!bd || !name || len <= 0) return;

    struct BtList *dev_list = hlp_manager_get_device_list();

    if (!dev_list) return;

    bt_remote_device_t *p_device = (bt_remote_device_t *) dev_list->head;

    //int i = 0;
    while (p_device != NULL) {
        if (memcmp(p_device->bd, bd, BD_ADDR_LEN) == 0)
        {
            if (p_device->name)
            {
                int l = strlen(p_device->name);
                len = (l > len ? len : l);
                strncpy(name, p_device->name, len);
            }

            return;
        }

        p_device = (bt_remote_device_t *) NEXT_MEM(dev_list, p_device);
    }
}


void at_register_event_callback_internal(at_event_callback cb)
{
    s_event_cb = cb;
}


/* FIXME replace with an unified function. */
#if 0
static char HEX[12] = {'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F'};
static int HEX2[12] = {10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15};

static int get_hex(char a)
{
    int i;
    for(i=0;i<12;i++)
    {
        if (a == HEX[i]) return HEX2[i];
    }
    return 0;
}
#endif

void bt_event_lock_init(void)
{
    s_event_lock = CREATE_CRITICAL(NULL);
    evt_composition_init();
}
void bt_event_lock_done(void)
{
	if (s_event_lock)
	{
		FREE_CRITICAL(s_event_lock);
		s_event_lock = NULL;
	}
}

static unsigned char convert_a2dp_status(unsigned char status)
{
    switch (status)
    {
        case ST_A2DP_UNINIT:
            return 0;
        case ST_A2DP_INIT:
            return 0;
        case ST_A2DP_READY:
            return 1;
        case ST_A2DP_CONNECTING:
            return 2; // FIXME
        case ST_A2DP_CONNECTED:
            return 3;
        case ST_A2DP_PLAYING:
            return 4;
        case ST_A2DP_END:
            return 5;
    }

    return 0;
}
#if 0
static unsigned char convert_avrcp_status(unsigned char status)
{
    switch (status)
    {
        case ST_APP_AVRCP_UNINIT:
            return  0;
        case ST_APP_AVRCP_INIT:
            return 0;
        case ST_APP_AVRCP_READY:
            return 1;
        case ST_APP_AVRCP_CONNECTING:
            return 2; // FIXME
        case ST_APP_AVRCP_CONNECTED:
            return 3;
        case ST_APP_AVRCP_PLAYING:
            return 4;
        case ST_APP_AVRCP_PAUSED:
            return 5;
        case ST_APP_AVRCP_END:
            return 6;
    }

    return 0;
}
#endif
static unsigned char convert_hf_status(unsigned char status)
{
    switch (status)
    {
        case ST_HF_UNINIT:
            return 0;
        case ST_HF_INIT:
            return 0;
        case ST_HF_READY:
            return 1;
        case ST_HF_CONNECTING:
            return 2; // FIXME
        case ST_HF_CONNECTED:
            return 3;
        case ST_HF_INCOMING:
            return 4;
        case ST_HF_ONGOING:
            return 5;
        case ST_HF_HUNGUPING:
            return 6;
        case ST_HF_END:
            return 7;
    }
    return 0;
}
#if 0
static void get_pbap_name_and_number(char *data, char *name, char *number)
{
    int name_len;
    int number_len;
    char *pdata = data;
    char type = *data;
    char dummy;

    // type
    pdata++;
    // index
    pdata += 4;
    if (type == '5' || type == '6')
    {
        // %d%04X%X%02d%02d prefix:type(1) + index(4) + number_type(1) + name_len(2) + number_len(2) = 10
        // number_type
        pdata++;
    }

    dummy = pdata[2];
    pdata[2] = 0;
    name_len = strtol(pdata, NULL, 10);
    pdata[2] = dummy;
    pdata += 2;
    number_len = strtol(pdata, NULL, 10);
    pdata += 2;
    bse_log_print("at_event", "\r\n", "pbap name_len:%d, number_len:%d.", name_len, number_len);

    strncpy(name, pdata, (name_len > 127 ? 127 : name_len));
    pdata += name_len;
    strncpy(number, pdata, (number_len > 63 ? 63 : number_len));
    bse_log_print("at_event", "\r\n", "pbap name:%s, number:%s.", name, number);
}
#endif
/* Internal Functions Callback */
#ifdef CONFIG_UNLIMIT_RESOURCE
#define EVENT_BUF_LEN 512
#else
#define EVENT_BUF_LEN 256
#endif
char event_buffer[EVENT_BUF_LEN];
static int scan_index = 0;
static char scan_name_buff[30] = {0};
UINT32 d_song_length = 0;

void bt_change_event_buffer_ff(char *d_event_buffer, int d_len)
{
    int i;
    for(i=0; i < d_len; i++)
    {
        if(d_event_buffer[i] == ',')
        {
            d_event_buffer[i] = 0xff;
        }
    }
}

void bt_cancel_scan_name_buff()
{
     memset(scan_name_buff, 0, 30);
}

void bt_set_scan_name_buff(char *p, int len)
{
    memset(scan_name_buff, 0, 30);
    memcpy(scan_name_buff, p, len);
}

void bt_event_indicate(unsigned int event_code, void *param)
{
    UINT32 event_len;
    UINT8 bd[BD_ADDR_LEN];
    bt_remote_device_t *rmt_dev;

    ENTER_CRITICAL(s_event_lock);

    memset(event_buffer, 0, sizeof(event_buffer));
    bse_log_print("at_event", "\r\n", "%s(%d)%d.", __FUNCTION__, event_code, sizeof(event_buffer));
    switch(event_code)
    {
        case BT_EV_APP_VERSION:
            //evt_notify_version(event_buffer, EVENT_BUF_LEN, BT_APP_VERSION);
            sprintf(event_buffer, "\r\n+VER=%s\r\n", BT_APP_VERSION);
            //evt_notify_version_ex(event_buffer, EVENT_BUF_LEN, BT_APP_VERSION);
            break;
        case BT_EV_SYSTEM_INIT_COMPLETE:
            evt_bt_initialized(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_ENTER_PAIRING_MODE:
            evt_gap_pair_mode_entered(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_EXIT_PAIRING_MODE:
            evt_gap_pair_mode_leaved(event_buffer, EVENT_BUF_LEN, 1);
            break;
        case BT_EV_HF_EXIT_SCAN_MODE:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_CONNECTING:
        	{
        		struct connected_info_t *connected_info = (struct connected_info_t *)param;
        		memset(bd, 0, BD_ADDR_LEN);
        		   if (connected_info && connected_info->bd) {
                   			 memcpy(bd, (UINT8 *) connected_info->bd, BD_ADDR_LEN);
               	    }
            		//evt_hfp_connecting(event_buffer, EVENT_BUF_LEN,bd);
                    sprintf(event_buffer, "\r\n+HFPSTAT=%d\r\n", 2);
        	}
            break;
        case BT_EV_HF_DISCONNECTED:
        	{
        		struct connected_info_t *connected_info = (struct connected_info_t *)param;
        		memset(bd, 0, BD_ADDR_LEN);
	    		   if (connected_info && connected_info->bd) {
	               			 memcpy(bd, (UINT8 *) connected_info->bd, BD_ADDR_LEN);
	           	    }
            		//evt_hfp_disconnected(event_buffer, EVENT_BUF_LEN,bd);
                    sprintf(event_buffer, "\r\n+HFPSTAT=%d\r\n", 1);
        	}
            break;
        case BT_EV_HF_CONNECTED:
            {
                struct connected_info_t *connected_info = (struct connected_info_t *)param;
                int nlen = DEV_NAME_LEN + 1;
                char rmt_name[DEV_NAME_LEN + 1];
                memset(rmt_name, 0, nlen);
                memset(bd, 0, BD_ADDR_LEN);

                if (connected_info && connected_info->bd) {
                    memcpy(bd, (UINT8 *) connected_info->bd, BD_ADDR_LEN);
                }

                get_remote_device_name(bd, rmt_name, nlen);
                //evt_hfp_connected(event_buffer, EVENT_BUF_LEN, bd);
                sprintf(event_buffer, "\r\n+HFPSTAT=%d\r\n", 3);
            }
            break;
        case BT_EV_HF_CONN_CFM:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_OUTGOING:
            //evt_hfp_outgoing_call(event_buffer, EVENT_BUF_LEN, (char *)param);
            sprintf(event_buffer, "\r\n+HFPSTAT=%d,%s\r\n", 4, (char *) param);
            //sprintf(event_buffer, "\r\n+HFPMTPX=%d%c%d%c%s\r\n", 1, 0xff, 3, 0xff, (char *) param);
            break;
        case BT_EV_HF_INCOMMINGCALL:
            //evt_hfp_incoming_call(event_buffer, EVENT_BUF_LEN, (char *) param);
            sprintf(event_buffer, "\r\n+HFPSTAT=%d,%s\r\n", 5, (char *) param);
            //sprintf(event_buffer, "\r\n+HFPMTPX=%d%c%d%c%s\r\n", 1, 0xff, 5, 0xff, (char *) param);
            break;
        case BT_EV_HF_ONGOING:
            //evt_hfp_call_accepted(event_buffer, EVENT_BUF_LEN, (char *) param);
            sprintf(event_buffer, "\r\n+HFPSTAT=%d,%s\r\n", 6, (char *) param);
            //sprintf(event_buffer, "\r\n+HFPMTPX=%d%c%d%c%s\r\n", 1, 0xff, 0, 0xff, (char *) param);
            break;
        case BT_EV_HF_STANDBY:
            evt_hfp_call_rejected(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_AG_DEVICE_NAME: 
            if(param)
            {
                sprintf(event_buffer, "SA%s", (char *)param);
            }
            break;
        case BT_EV_HF_AG_DEVICE_VENDOR_NAME:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_AG_DEVICE_MODEL:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_SCO_CONNECTED:
            //evt_hfp_sco_established(event_buffer, EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+HFPAUDIO=%d\r\n", 1);
            break;
        case BT_EV_HF_SCO_DISCONNECTED:
            //evt_hfp_sco_released(event_buffer, EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+HFPAUDIO=%d\r\n", 0);
            break;
        case BT_EV_HF_RETURN_STATUS:
            {
                hf_status_t *status = (hf_status_t *) param;
                if (status != NULL)
                {
                    evt_hfp_status_ind(event_buffer, EVENT_BUF_LEN, status->bd, convert_hf_status(status->status));
                }
            }
            break;
        case BT_EV_HF_RINGTONE:
            evt_income_ringtone(event_buffer, EVENT_BUF_LEN,*((UINT8 *)param));
            break;
        case BT_EV_HF_NETWORK_OPERATOR_IND:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+HFPNET=%s\r\n", (char *)param);
            break;
        case BT_EV_HF_SIGNAL_STRENGTH_IND:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+HFPSIG=%d\r\n", (int )param);
            break;
        case BT_EV_HF_BATTERY_CHARGE_IND:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+HFPBATT=%d\r\n", (int )param);
            break;
        case BT_EV_HF_NETWORK_STATE_IND:
            sprintf(event_buffer, "\r\n+HFPROAM=%d\r\n", (int )param);
            break;
        case BT_EV_HF_RINGTONE_TERMINATED:
            sprintf(event_buffer, "\r\n+HFPIBR=%d\r\n", (int )param);
            break;
        case BT_EV_HF_RINGTONE_TERMINATED_EX:
            sprintf(event_buffer, "\r\n+HFPRINGL=%d\r\n", (int )param);
            break;
        case BT_EV_HF_SUBSCRIBER_NUMBER_IND:
            {
                sprintf(event_buffer, "\r\n+HFPNUM=%s\r\n", (char *)param);
                //evt_hfp_subscriber_number(event_buffer,EVENT_BUF_LEN,param);
            }
            break;
         case BT_EV_HF_3_WAY_CALL_NEW:
            {
                hf_3_way_call_t *p_call = (hf_3_way_call_t *)param;
                //evt_hf_3_ip_ind(event_buffer,EVENT_BUF_LEN,p_call->index, p_call->state, p_call->number);
                //bse_log_print("at_event", "\r\n","event_buffer = %s\n", event_buffer);
                //p_call->state:0:active  1:hold  2:outgoing  3:warning  4:inconming 5:waiting
                sprintf(event_buffer, "\r\n+HFPMTPX=%d%c%d%c%s\r\n", p_call->index, 0xff, p_call->state, 0xff, p_call->number);
            }
            break;
        case BT_EV_HF_3_WAY_CALL_STATE_IND:
            {
                hf_3_way_call_t *p_call = (hf_3_way_call_t *)param;
                //evt_hf_3_it_ind(event_buffer,EVENT_BUF_LEN,p_call->index, p_call->state, p_call->number);
                //bse_log_print("at_event", "\r\n","event_buffer = %s\n", event_buffer);
                
                sprintf(event_buffer, "\r\n+HFPMTPX=%d%c%d%c%s\r\n", p_call->index, 0xff, p_call->state, 0xff, p_call->number);
            }
            break;
        case BT_EV_HF_3_WAY_CALL_DONE:
            {
                bse_log_print("at_event", "\r\n","BT_EV_HF_3_WAY_CALL_DONE\n");
                hf_3_way_call_t *p_call = (hf_3_way_call_t *)param;
                //evt_hf_3_iq_ind( event_buffer,EVENT_BUF_LEN,p_call->index, p_call->state, p_call->number);
                //bse_log_print("at_event", "\r\n","event_buffer = %s\n", event_buffer);
                sprintf(event_buffer, "\r\n+HFPMTPX=%d%c%d%c%s\r\n", p_call->index, 0xff, 6/*p_call->state*/, 0xff, p_call->number);
            }
            break;
        case BT_EV_HF_VOICE_RECOGN_START:
            {
                //evt_hfp_voice_recogn_start(event_buffer,EVENT_BUF_LEN);
                sprintf(event_buffer, "\r\n+HFPVR=%d\r\n", 1);
            }
            break;
        case BT_EV_HF_VOICE_RECOGN_STOP:
            {
                //evt_hfp_voice_recogn_stop(event_buffers,EVENT_BUF_LEN);
                sprintf(event_buffer, "\r\n+HFPVR=%d\r\n", 0);
            }
            break;
        case BT_EV_HF_RINGTONE_ON:
            {
                sprintf(event_buffer, "\r\n+HFPRING=%d\r\n", 1);
            }
            break;
        case BT_EV_HF_RINGTONE_OFF:
            {
                sprintf(event_buffer, "\r\n+HFPRING=%d\r\n", 0);
            }
            break;
        case BT_EV_A2DP_AV_SUSPEND:
            //evt_a2dp_paused(event_buffer, EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+PLAYSTAT=%d\r\n", (unsigned int) 2);
            break;
        case BT_EV_A2DP_AV_PLAYING:
            //evt_a2dp_playing(event_buffer, EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+PLAYSTAT=%d\r\n", (unsigned int) 1);
            break;
        case BT_EV_A2DP_AV_OPEN:
            // FIXME
            {
                UINT8 *addr;
                addr = (UINT8 *) param;
                evt_a2dp_connected(event_buffer, EVENT_BUF_LEN,addr);
            }
            break;
        case BT_EV_A2DP_DISCONNECTED:
            {
                UINT8 *addr;
                addr = (UINT8 *) param;
                evt_a2dp_disconnected(event_buffer, EVENT_BUF_LEN,addr);
            }
            break;
        case BT_EV_A2DP_RETURN_STATUS:
             {
                a2dp_status_t *status = (a2dp_status_t *) param;
                if (status != NULL) {
                    evt_a2dp_status_ind(event_buffer, EVENT_BUF_LEN, status->bd, convert_a2dp_status(status->status));
                }
             }
            break;
        case BT_EV_AVRCP_CONN_COMPLETED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_DISC_COMPLETED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_ELEMENT_ATTRS:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_FOLDER_LIST_INFO:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_GET_PROFILE_CFG:
            sprintf(event_buffer, "\r\n+AVRCPCFG=%d\r\n", (UINT8)param);
            break;
        case BT_EV_AVRCP_NOW_PLAY_STATUS_IND:
            {
                struct AVRCPNowPlayStatusIndStru *p_data = (struct AVRCPNowPlayStatusIndStru *)param;
                //evt_avrcp_nowplay_status_ind(event_buffer, EVENT_BUF_LEN,param);
                int state = app_avrcp_get_status();
                if(state >= ST_APP_AVRCP_CONNECTED)
                {
                    state -= ST_APP_AVRCP_STOPPED;
                    d_song_length = p_data->song_length;
                    UINT32 d_song_position = p_data->song_position;
                    //sprintf(event_buffer, "\r\n+TRACKSTAT=%d,%d,%d\r\n", state, d_song_position / 1000, d_song_length / 1000);
                    sprintf(event_buffer, "\r\n+TRACKSTAT=%d,%u,%u\r\n", state, d_song_position, d_song_length);
                }
            }
            break;
        case BT_EV_AVRCP_ELEMENT_ATTRS_OF_NOW_PLAYING:
            {
                if (param)
                {
                    struct AVRCPDataIndStru *p_data = (struct AVRCPDataIndStru *)param;
				
                    if (NULL != p_data->title)
                    {
                        //evt_avrcp_id3_ind(event_buffer, EVENT_BUF_LEN, p_data->title, p_data->artist,    p_data->album, p_data->time);
                        sprintf(event_buffer, "\r\n+TRACKINFO=%s,%s,%s\r\n", p_data->title, p_data->artist, p_data->album);
                        bt_change_event_buffer_ff(event_buffer, strlen(event_buffer));
                    }       
                }
            }
            break;
        case BT_EV_AVRCP_PLAYBACK_STATUS_CHANGED:
            evt_avrcp_change_ind(event_buffer, EVENT_BUF_LEN,(UINT32)param);
            break;
        case BT_EV_AVRCP_PLAYBACK_POS_CHANGED:
            {
                int state = app_avrcp_get_status();
                //evt_avrcp_pos_changed(event_buffer, EVENT_BUF_LEN,(UINT32)param);
                if(state >= ST_APP_AVRCP_CONNECTED)
                {
                    state -= ST_APP_AVRCP_CONNECTED;
                    //sprintf(event_buffer, "\r\n+TRACKSTAT=%d,%d,%d\r\n", state, (UINT32)param / 1000, d_song_length / 1000);
                    sprintf(event_buffer, "\r\n+TRACKSTAT=%d,%u,%u\r\n", state, (UINT32)param, d_song_length);
                }
            }
            break;
        case BT_EV_AVRCP_ABS_VOL_IND:
            {
                sprintf(event_buffer, "\r\n+ABSVOL=%d\r\n", (UINT8)param);
            }
            break;
        case BT_EV_AVRCP_NOW_PLAYING_CONTENT_CHANGED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_ELEMENT_END:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_UPDATA_UIDS:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_BROWSING_CHANEL_COMPLETED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_AVRCP_RETURN_STATUS: 
            {
                avrcp_status_t *status = (avrcp_status_t *) param;

                if (status != NULL) {
                    evt_avrcp_status_ind(event_buffer, EVENT_BUF_LEN,status->bd , convert_hf_status(status->status));
                }
            }
            break;
#if 0
        case BT_EV_BTC_CALIBRATION_STARTED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_BTC_CALIBRATION_COMPLETE:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_BTC_CALIBRATION_CANCEL:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_BTC_CONNECTED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_BTC_DISCONNECTED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_BTC_PHONE_SCREEN_STATE_IND:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            break;
#endif
        case BT_EV_PBAP_CONNECT_SUCCESS:
            evt_pbap_status_ind(event_buffer, EVENT_BUF_LEN, 1);
            break;
        case BT_EV_PBAP_CONNECT_FAIL:
            evt_pbap_status_ind(event_buffer, EVENT_BUF_LEN, 0);
            break;
        case BT_EV_PBAP_DISCONNECTED:
            // TODO evt_(event_buffer, EVENT_BUF_LEN);
            pbap_at_buff_init();
            pbap_at_flag_init();
            break;
        case BT_EV_PBAP_CURR_CALL_PHONEBOOK_IND:
            sprintf(event_buffer, "\r\n+PBDATA=%s\r\n", param);
            break;
        case BT_EV_PBAP_PHONEBOOK_DATA_IND:
            {
                /*int index_s = 0;
                //evt_pbap_data_ind(event_buffer, EVENT_BUF_LEN, param);
                //sprintf(event_buffer, "\r\n+PBDATA=%s\r\n", param);
                char pb_buff[512] = {0};
                memcpy(pb_buff, param, strlen((char *)param));
                //bse_log_print("at_event", "\r\n","pb_buff[10] = %c\n", pb_buff[0]);
                switch (pb_buff[0])
                {
                case '0':
                    index_s = ++pbap_sim_id;
                    break;
                case '1':
                    index_s = ++pbap_iphone_id;
                    break;
                case '2':
                case '3':
                case '4':
                    index_s = ++pbap_cch_id;
                    break;
                default:
                    break;
                }
                //bse_log_print("at_event", "\r\n","index_s = %d\n", index_s);
                if(index_s)
                    sprintf(event_buffer, "\r\n+PBDATA=%s%c%d\r\n", param, 0xff, index_s);

                //bse_log_print("at_event", "\r\n","event_buffer = %s\n", event_buffer);*/
                static int h_offset = 50;
                static int p_pb_number = 0;
                char buff[256] = {0}; 
                char *p = (char *)param;
                int i = strlen(p);
                bse_log_print("at_event", "\r\n","i = %d\n", i);
                while(i--)
                {
                    bse_log_print("at_event", "\r\n","p[i] = %c i = %d\n", p[i], i);
                    if(p[i] < '0' || p[i] > '9')
                        break;
                }
                bse_log_print("at_event", "\r\n","%s\n", &p[i+1]);
                w_pb_number = atoi(&p[i+1]);
                
                bse_log_print("at_event", "\r\n","h_offset = %d, w_offset = %d w_pb_number = %d\n", h_offset, w_offset, w_pb_number);
                if(w_offset > 0 && w_type > 1)
                {
                    if(w_pb_number == 1)
                        p_pb_number = 0;

                    int c_pb_number = w_pb_number - p_pb_number;
                    
                    bse_log_print("at_event", "\r\n","c_pb_number = %d\n", c_pb_number);
                    h_offset += c_pb_number;
                    
                    p_pb_number = w_pb_number;

                    memcpy(buff, p, i+1);
                    sprintf(&buff[i+1], "%d", h_offset);
                    bse_log_print("at_event", "\r\n","buff = %s\n", buff);
                    sprintf(event_buffer, "\r\n+PBDATA=%s\r\n", buff); 
                    break;
                }
                h_offset = 50;
                sprintf(event_buffer, "\r\n+PBDATA=%s\r\n", param); 
            }
            break;
        case BT_EV_PBAP_STAT_EVENT:
            {
                sprintf(event_buffer, "\r\n+PBSTAT=%d\r\n", param);
            }break;
        case BT_EV_PBAP_GET_PHONEBOOK_FAIL:
            // FIXME
#if 0
            evt_pbap_status_ind(event_buffer, EVENT_BUF_LEN, 0);
            break;
#endif
        break;
        case BT_EV_PBAP_GET_PHONEBOOK_SUCCESS:
            {
                /*pbap_sim_id = 0;
                pbap_iphone_id = 0;
                pbap_cch_id = 0;*/
                //evt_pbap_sync_completed(event_buffer, EVENT_BUF_LEN);
                bse_log_print("at_event", "\r\n","w_type = %d w_pb_number = %d w_offset=%d w_max_items = %d\n", w_type, w_pb_number, w_offset, w_max_items);
                if(w_type > 1 && w_pb_number == 50 )
                {
                    w_offset += w_pb_number + w_pb_offset;
                    w_pb_number = 0;
                    if(w_max_items - w_offset > 0)
                    {
                        //char w_buff[32] = {0};
                        //sprintf(w_buff, "DOWN=%d,%d,%d", p_type, w_offset, 50);

                        //ATPbap(w_buff, strlen(w_buff));
                        w_type = (w_type == 5 ? 7 : w_type);
                        bse_log_print("at_event", "\r\n","w_type = %d\n", w_type);    
                        do_get_pbap_book(w_type, w_offset, 50);
                        break;
                    }
                } 
                
                pbap_at_flag_init();
                sprintf(event_buffer, "\r\n+PBDATA=%s %s\r\n", "E: Download complete", (char *)param);

                bse_log_print("at_event", "\r\n","pb_at_index = %d\n", pb_at_index);
                if(0/*pb_at_index != 0*/)
                {
                    ATPbap(pbap_at_buff[pb_at_index-1], strlen(pbap_at_buff[pb_at_index-1]));
                    memset(pbap_at_buff[pb_at_index-1], 0, 32);
                    pb_at_index--;
                    //break;
                }
            }
            break;
        case BT_EV_PBAP_GET_PHONEBOOK_SIZE_SUCCESS:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_PBAP_INDEX_DATA_IND:
            {
                //evt_pbap_contact_ind(event_buffer, EVENT_BUF_LEN, param);
                //sprintf(event_buffer, "\r\n+PBDATA=%s\r\n", param);
            }
            break;
        case BT_EV_PBAP_SUPPORTED_REPOS:
            evt_pbap_supported_repos_ind(event_buffer, EVENT_BUF_LEN, (unsigned char) param);
            break;
#ifdef CONFIG_ANCS_CLIENT
        case BT_EV_ANCS_CONNECTION_STATE:
            // sprintf(event_buffer, "\r\n+ANCSSTAT=%d\r\n", (unsigned int)param);
            break;
        case BT_EV_ANCS_IOS_NOTIFICATION: {
            ble_ancs_ios_notification_t *evt = (ble_ancs_ios_notification_t *)param;

            // sprintf(event_buffer, "\r\n+ANCSNT=%02x%02x%02x%02x%02x%02x%02x%02x\r\n",  \
            //                         evt->event_id, evt->event_flags, evt->category_id, evt->category_count \
            //                         , evt->notification_uid[3], evt->notification_uid[2], evt->notification_uid[1], evt->notification_uid[0]);
            } break;
        case BT_EV_ANCS_NOTIFICATION_ATTRIBUTE: {
            bse_log_print("at_event", "\r\n","BT_EV_ANCS_NOTIFICATION_ATTRIBUTE %s\n", (UINT8 *)param);
#if 0
            ble_ancs_notif_attribute_t *evt = (ble_ancs_notif_attribute_t *)param;
            int i,j;
            sprintf(event_buffer, "\r\n+ANCSNTATT=%02x%02x%02x%02x%02x", \
                                evt->command_id, evt->notification_uid[3], evt->notification_uid[2], evt->notification_uid[1], evt->notification_uid[0]);
            j = strlen(event_buffer);
            bse_log_print("at_event", "\r\n","event_buffer = %s j = %d\n", event_buffer, j);
            
            for(i=0; i<evt->attribute.len; i++)
            {
                //strcat(event_buffer, evt->attribute.data[i]);
                sprintf(&event_buffer[j], "%02x", evt->attribute.data[i]);
                j+=2;
            }
            bse_log_print("at_event", "\r\n","event_buffer = %s\n", event_buffer);
            strcat(event_buffer, "\r\n");
            bse_log_print("at_event", "\r\n","event_buffer = %s\n", event_buffer);
#else
            //sprintf(event_buffer, "\r\n+ANCSNTI=%s\r\n", (UINT8 *)param);
            //snprintf(event_buffer, EVENT_BUF_LEN, "\r\n+ANCSNTI=%s\r\n", (unsigned char*) param);
#endif
            } break;
        case BT_EV_EY_WIFI_PHONE_MESSAGE:
            UINT8 *s_par= (UINT8 *)param;
            int len = strlen(s_par);
            /*if(len >= EVENT_BUF_LEN)
            {
                snprintf(event_buffer, 512, "\r\n+ANCSNTA=%s\r\n", (unsigned char*) param);
                break;
            }*/
            //sprintf(event_buffer, "\r\n+ANCSNTA=%s\r\n", (unsigned char*) param);
            //snprintf(event_buffer, EVENT_BUF_LEN, "\r\n+ANCSNTA=%s\r\n", (unsigned char*) param);
            break;
#endif
#ifdef CONFIG_HID
        case BT_EV_BTC_CONNECTED:
            sprintf(event_buffer, "\r\n+HIDSTAT=3\r\n");
            break;
        case BT_EV_BTC_DISCONNECTED:
            sprintf(event_buffer, "\r\n+HIDSTAT=1\r\n");
            break;
#endif
#ifdef CONFIG_SPP
        case BT_EV_SPP_START:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_SPP_STOP:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_SPP_CONNECTED:
            //evt_spp_connected(event_buffer, EVENT_BUF_LEN, ((rfc_con_info_t *)param)->bd, ((rfc_con_info_t *)param)->handle);
            break;
        case BT_EV_SPP_DISCONNECTED:
            //evt_spp_disconnected(event_buffer, EVENT_BUF_LEN, (UINT32) param);
            break;
        case BT_EV_SPP_SEND_DATA_RET:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_SPP_DATA_IND:
            //{
                /*struct auto_spp_data_ind_stru *data_ind = (struct auto_spp_data_ind_stru *) param;

                if (data_ind)
                {
                	//debug_print_data("spp_data", data_ind->data, data_ind->data_len);
                    //event_len = evt_spp_data_received(event_buffer, EVENT_BUF_LEN, data_ind->handle, data_ind->data_len, data_ind->data);
                    //UART_Printf("[BRT]BT_EV_SPP_DATA_IND: event_len = %d, event_buffer = %s\r\n", event_len, event_buffer);

                    if (s_event_len_cb != NULL)
                    {
                        s_event_len_cb(event_buffer, event_len);
                    }
                }
            }
            LEAVE_CRITICAL(s_event_lock);
            return;
            */
           break;
        case BT_EV_SPP_REMOTE_RFC_CHANNEL_IND:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_SPP_REMOTE_RFC_CHANNEL_COMPLETED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_SPP_RFCOMM_LISTEN_RESULT:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_SPP_RFCOMM_CONN_RESULT:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;

//android auto evnet
        case BT_EV_AAP_CONNECTED:
            sprintf(event_buffer, "\r\n+AAPSTAT=3\r\n");
            break;
        case BT_EV_AAP_DISCONNECTED:
            sprintf(event_buffer, "\r\n+AAPSTAT=1\r\n");
            break;
        case BT_EV_AAP_DATA_IND:
            struct app_spp_data_ind_stru *aap_data_ind;
            UINT8 *aap_data;
            aap_data_ind = (struct app_spp_data_ind_stru *) param;
            aap_data = (UINT8 *)NEW((aap_data_ind->data_len * 2) + 1);
            byte_array_to_hex_string(aap_data, aap_data_ind->data, aap_data_ind->data_len);
            if (aap_data) {
                sprintf(event_buffer, "\r\n+AAPDATA=%s", aap_data);
    		}
            FREE(aap_data);
           break;

//carplay evnet
        case BT_EV_IAP2_CONNECTED:
            sprintf(event_buffer, "\r\n+IAPSTAT=3\r\n");
            break;
        case BT_EV_IAP2_DISCONNECTED:
            sprintf(event_buffer, "\r\n+IAPSTAT=1\r\n");
            break;
        case BT_EV_IAP2_DATA_IND:
            struct app_spp_data_ind_stru *iap_data_ind;
            UINT8 *iap_data;
//		UINT8 packet_tag[2] = {0xFF, 0x5A};
            iap_data_ind = (struct app_spp_data_ind_stru *) param;

//		if(memcmp(iap_data_ind->data, packet_tag, 2) != 0) {
			iap_data = (UINT8 *)NEW((iap_data_ind->data_len * 2) + 1);
			byte_array_to_hex_string(iap_data, iap_data_ind->data, iap_data_ind->data_len);
		 	if (iap_data) {
                		sprintf(event_buffer, "\r\n+IAPDATA=%s", iap_data);
    			}
			FREE(iap_data);
#if 0
		} else {			
			unsigned int offset = 0;
			unsigned int packet_len = 0;
			UINT8 *tmp_str = iap_data_ind->data;
			unsigned char *packet_data;
			do {
				if(memcmp(tmp_str, packet_tag, 2) == 0) {
					offset += 2;
					packet_len = (tmp_str[offset] << 8)| (tmp_str[offset+1]);
					offset += 2;

					bse_log_print("at_event", "\r\n", "iap_data_ind->data_len:%d, iap packet_len:[%d]\n", iap_data_ind->data_len, packet_len);
					packet_data = (unsigned char *)NEW(packet_len);
					memcpy(packet_data, tmp_str+(offset-4), packet_len);
					offset += packet_len -4;
					
					iap_data = (UINT8 *)NEW((packet_len * 2) + 1);
					byte_array_to_hex_string(iap_data, packet_data, packet_len);
					 if (iap_data) {
					 	sprintf(event_buffer, "\r\n+IAPDATA=%s", iap_data);
					 }
					 if(offset < iap_data_ind->data_len)
					{
						bse_log_print("at_event", "\r\n", "iap offset:[%d], event_buffer[%s] \n", offset, event_buffer);
						s_event_cb(event_buffer+2);
					}
				}

				FREE(packet_data);
				FREE(iap_data);
			} while(offset < iap_data_ind->data_len);	
		}
#endif            
            
           break;
#endif
//EY
	case BT_EV_EY_WIFI_SSID:{
		sprintf(event_buffer, "\r\n+WLSSID=%s", (char *)param);
	}
	break;
	case BT_EV_EY_WIFI_PASSWD:{
		sprintf(event_buffer, "\r\n+WLPASSWD=%s", (char *)param);
	}
	break;
	case BT_EV_EY_WIFI_READY:{
		sprintf(event_buffer, "\r\n+WLCONN");
	}
	break;
	case BT_EV_EY_WIFI_PHONE_TYPE:{
		sprintf(event_buffer, "\r\n+AAA5=%s", (char *)param);
	}
	break;

        case BT_EV_OTHER_LOCAL_STATE:{
            if((unsigned int)param == 0){
                d_disable_state = 0;
                usleep(500000);
            } else {
                d_disable_state = 1;
            }
                
            //evt_bt_init_sucess_ind(event_buffer, (int) param ,EVENT_BUF_LEN);
            sprintf(event_buffer, "\r\n+DEVSTAT=%d\r\n", (unsigned int)param);
            }
            break;
        case BT_EV_OTHER_PAIRIED_DEVICE_IND:
            if (param)
            {
                /*
                int index;
                char *str = (char *) param;

                index = *str - '0';
                str += 1;
                str += 8; // class of device

                parse_bd_address(str, bd);
                str += 12;
                evt_gap_paired_device_ind(event_buffer, EVENT_BUF_LEN, index, bd, str);*/
                int index;
                char *str = (char *) param;

                index = *str - '0';
                str += 1;
                str += 8; // class of device

                parse_bd_address(str, bd);
                str += 12;
                sprintf(event_buffer, "\r\n+PLIST=%d,%d,%02x%02x%02x%02x%02x%02x,%s\r\n", index, 0, bd[5], bd[4], bd[3], bd[2], bd[1], bd[0], str);
                int i,d_ind=0;
                for(i=0; i<strlen(event_buffer); i++)
                {
                    if(event_buffer[i] == ',')
                        d_ind++;
                    if(d_ind == 2)
                    {
                        if(event_buffer[i] >= 'a' && event_buffer[i] <= 'z')
                        {
                            event_buffer[i] -= 32;
                        }
                    }
                }
                bt_change_event_buffer_ff(event_buffer, strlen(event_buffer));
            }
            break;
        case BT_EV_OTHER_PAIRIED_DEVICE_END:
            sprintf(event_buffer, "\r\n+PLIST=E\r\n");
            break;
        case BT_EV_OTHER_AUTO_ANSWER_CONNECT_IND:
            if (param)
            {
                unsigned char auto_answer = *((char *) param) - '0';
                unsigned char auto_connect = *(((char *) param) + 1) - '0';
                evt_bt_config_ind(event_buffer, EVENT_BUF_LEN, auto_answer, auto_connect);
            }
            break;
        case BT_EV_OTHER_LOCAL_BD_IND:            
            //evt_local_bd_ind(event_buffer, EVENT_BUF_LEN,(char *)param);
            sprintf(event_buffer, "\r\n+ADDR=%s\r\n", (char *)param);
            break;
        case BT_EV_OTHER_LOCAL_LE_BD_IND:            
            //evt_local_bd_ind(event_buffer, EVENT_BUF_LEN,(char *)param);
            sprintf(event_buffer, "\r\n+LEADDR=%s\r\n", (char *)param);
            break;
        case BT_EV_OTHER_LOCAL_SCAN_MODE_IND:
            //evt_local_scan_mode_ind(event_buffer, EVENT_BUF_LEN, ( UINT8) param);
            sprintf(event_buffer, "\r\n+PAIR=%d\r\n", (unsigned int)param);
            break;
        case BT_EV_OTHER_LOCAL_DEVICE_NAME:
            //evt_gap_local_name(event_buffer, EVENT_BUF_LEN, (char *) param);
            sprintf(event_buffer, "\r\n+NAME=%s\r\n", (char *)param);
            break;
        //case BT_EV_OTHER_LOCAL_SCAN_MODE_IND:

            //break;
        case BT_EV_OTHER_LOCAL_PINCODE:
            //evt_gap_pincode_ind(event_buffer, EVENT_BUF_LEN, (char *) param);
            sprintf(event_buffer, "\r\n+PIN=%s\r\n", (char *)param);
            break;
        case BT_EV_OTHER_CURRENT_VOLUME:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_COMMAND_REPLY:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_DISCOVERY_STARTED:
            evt_discovery_started(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_DISCOVERY_DEVICE_FOUND:
            rmt_dev = (bt_remote_device_t *) param;
            if (rmt_dev)
            {
                //evt_gap_remote_dev_found(event_buffer, EVENT_BUF_LEN, rmt_dev->bd, rmt_dev->bond_state, rmt_dev->name);
                if(scan_name_buff[0]){
                    if(memcmp(rmt_dev->name, scan_name_buff, strlen(scan_name_buff)) == 0)
                        sprintf(event_buffer, "\r\n+SCAN=%d,%d,%d,%02x%02x%02x%02x%02x%02x,%s,%6x\r\n", 
                                                    ++scan_index, rmt_dev->rssi, rmt_dev->cap, rmt_dev->bd[5],rmt_dev->bd[4],rmt_dev->bd[3],
                                                    rmt_dev->bd[2],rmt_dev->bd[1],rmt_dev->bd[0], rmt_dev->name, rmt_dev->class_of_device);
                } else {
                    sprintf(event_buffer, "\r\n+SCAN=%d,%d,%d,%02x%02x%02x%02x%02x%02x,%s,%6x\r\n", 
                                                    ++scan_index, rmt_dev->rssi, rmt_dev->cap, rmt_dev->bd[5],rmt_dev->bd[4],rmt_dev->bd[3],
                                                    rmt_dev->bd[2],rmt_dev->bd[1],rmt_dev->bd[0], rmt_dev->name, rmt_dev->class_of_device);
                    
                } 
                int i,d_ind=0;
                for(i=0; i<strlen(event_buffer); i++)
                {
                    if(event_buffer[i] == ',')
                        d_ind++;
                    if(d_ind == 3)
                    {
                        if(event_buffer[i] >= 'a' && event_buffer[i] <= 'z')
                        {
                            event_buffer[i] -= 32;
                        }
                    }
                }

                bt_change_event_buffer_ff(event_buffer, strlen(event_buffer)); 
            }
            break;
        case BT_EV_DISCOVERY_DEVICE_UPDATED:
            // evt_(event_buffer, EVENT_BUF_LEN);
           
            break;
        case BT_EV_DISCOVERY_FINISHED:
            scan_index = 0;
            sprintf(event_buffer, "\r\n+SCAN=%c\r\n", 'E');
            //evt_discovery_finished(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_PINCODE_REQ:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_PASSKEY_REQ:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_PASSKEY_NOTIF:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_USER_CONFIRMATION:
        	{
            	struct HCI_User_Confirmation_RequestEvStru * par;
                par = (struct HCI_User_Confirmation_RequestEvStru *) param;
                if (par != NULL)
                {
                    //sprintf(event_buffer, "GPF%02X%02X%02X%02X%02X%02X%06d", 
                            //par->bd[5], par->bd[4], par->bd[3], par->bd[2], par->bd[1], par->bd[0], par->numeric_value);
                    char bon_name[DEV_NAME_LEN+1] = {0};
                    get_remote_device_name(par->bd, bon_name, DEV_NAME_LEN+1);

                    do_ssp_pair_remote_addr(par->bd);
                    sprintf(event_buffer, "\r\n+PAIRREQ=%06d,%02X%02X%02X%02X%02X%02X,%s\r\n", 
                            par->numeric_value, par->bd[5], par->bd[4], par->bd[3], par->bd[2], par->bd[1], par->bd[0], bon_name);
                }
            }
            break;
        case BT_EV_BOND_STATE_CHANGED:
            rmt_dev = (bt_remote_device_t *) param;
            //break;
            if (rmt_dev)
            {
                char bon_name[DEV_NAME_LEN+1] = {0};
                memset(bd, 0, BD_ADDR_LEN);
                int bond_state = 1;
                if(rmt_dev->bond_state == ST_BOND_BONDING)
                    break;
               
                if(rmt_dev->bond_state == ST_BOND_BONDED)
                    bond_state = 0;
                //evt_bond_state_changed(event_buffer, EVENT_BUF_LEN, rmt_dev->bd, rmt_dev->bond_state);
                get_remote_device_name(rmt_dev->bd, bon_name, DEV_NAME_LEN+1);
                if(bon_name[0]){//retry
                    hlp_manager_get_romete_name_ex(rmt_dev->bd, bon_name);
                }
                if(bon_name[0])
                {
                    sprintf(event_buffer, "\r\n+PAIRED=%d,%02X%02X%02X%02X%02X%02X,%s\r\n", 
                            bond_state, rmt_dev->bd[5], rmt_dev->bd[4], rmt_dev->bd[3], rmt_dev->bd[2], rmt_dev->bd[1], rmt_dev->bd[0], bon_name);
                }else{
                    sprintf(event_buffer, "\r\n+PAIRED=%d,%02X%02X%02X%02X%02X%02X,%s\r\n", 
                            bond_state, rmt_dev->bd[5], rmt_dev->bd[4], rmt_dev->bd[3], rmt_dev->bd[2], rmt_dev->bd[1], rmt_dev->bd[0], "123");
                }
                bt_change_event_buffer_ff(event_buffer, strlen(event_buffer));
            }
            break;
        case BT_EV_CONNECTION_DISCONNECTED: 
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PUSH_STARTED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PUSH_IN_PROGRESS:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PUSH_COMPLETED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PUSH_ABORTED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PULL_STARTED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PULL_IN_PROGRESS:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PULL_COMPLETED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_OPP_PULL_ABORTED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_MAP_CONNECTION_EVENT:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_MAP_NEW_SMS_ARRIVED:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_MAP_RETRIEVE_SMS_RESULT:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_MAP_GET_SMS_LIST_RESULT:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_MAP_SEND_SMS_RESULT:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_MAP_DELETE_SMS_RESULT:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_MAP_BMSG_DATA_IND:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
#ifdef CONFIG_GATT
        case BT_EV_GATT_SERVER_REGISTERED:
            break;
        case BT_EV_GATT_SERVER_CONNECT_CB:
            break;
        case BT_EV_GATT_CONNECTED:
        evt_param_connected_t *conn_param = (evt_param_connected_t *)param;
		 sprintf(event_buffer, "\r\n+GATTSTAT=3,%02X,%02X%02X%02X%02X%02X%02X\r\n", conn_param->client_if, 
            conn_param->address.address[5],conn_param->address.address[4],conn_param->address.address[3],conn_param->address.address[2],conn_param->address.address[1],conn_param->address.address[0]);
            break;
        case BT_EV_GATT_DISCONNECTED:
		 sprintf(event_buffer, "\r\n+GATTSTAT=1\r\n");
            break;
        case BT_EV_GATT_SERVICE_ADDED:
            {
                btgatt_db_element_t * tmp = (btgatt_db_element_t * )param;
                bse_log_print("at_event", "\r\n","------->BT_EV_GATT_SERVICE_ADDED:[%d]", tmp->type);
                switch(tmp->type){
                    case BTGATT_DB_PRIMARY_SERVICE:
                        break;
                    case BTGATT_DB_CHARACTERISTIC:
                        break;
                    case BTGATT_DB_DESCRIPTOR:
                        break;
                    case BTGATT_DB_SERVER_START:
                        break;
                    case BTGATT_DB_SERVER_END:
                        break;
                }
            }
            break;
        case BT_EV_GATT_SERVER_READ_CHARACTER:
            break;
        case BT_EV_GATT_SERVER_WRITE_CHARACTER:
		evt_param_server_write_characteristic_t *gatt_server_write_characteristic;
		gatt_server_write_characteristic = (evt_param_server_write_characteristic_t*)param;
            UINT8 *gatt_data_str;
            gatt_data_str = (UINT8 *)NEW((gatt_server_write_characteristic->data_len * 2) + 1);
            byte_array_to_hex_string(gatt_data_str, gatt_server_write_characteristic->data, gatt_server_write_characteristic->data_len);
            if (gatt_data_str) {
                sprintf(event_buffer, "\r\n+GATTDATA=%s", gatt_data_str);
    		}
            FREE(gatt_data_str);
            break;
        case BT_EV_GATT_SERVER_READ_DESCRIPTOR:
            break;
        case BT_EV_GATT_SERVER_WRITE_DESCRIPTOR:
            evt_param_server_write_descriptor_t *msg = (evt_param_server_write_descriptor_t*)param;
            if (clcast_ble_is_active()) {
                int enabled = (msg && msg->data_len > 0 && (msg->data[0] & 0x01)) ? 1 : 0;
                sprintf(event_buffer, "\r\n+GATTNOTIFY=%d\r\n", enabled);
            } else if (msg) {
                sprintf(event_buffer, "\r\n+CCCDEN:%04x\r\n",msg->handle);
            }
            break;
        case BT_EV_GATT_NOTIFICATION_SENT:
            {
                evt_param_notification_sent_t *sent = (evt_param_notification_sent_t *)param;
                int status = sent ? sent->status : -1;
                sprintf(event_buffer, "\r\n+GATTSENT=%d\r\n", status);
            }
            break;
        case BT_EV_GATT_SERVER_MTU_CHANGED:
            {
                evt_param_server_mtu_changed_t *mtu = (evt_param_server_mtu_changed_t *)param;
                int mtu_value = mtu ? mtu->mtu : 23;
                int max_payload = mtu_value > 3 ? mtu_value - 3 : 20;
                sprintf(event_buffer, "\r\n+GATTMTU=%d,%d\r\n", mtu_value, max_payload);
            }
            break;
#endif
        case BT_EV_UNDEFINE:
            // evt_(event_buffer, EVENT_BUF_LEN);
            break;
        case BT_EV_HF_EXIT_CFG:
            sprintf(event_buffer, "\r\n+HFPCFG=%d\r\n", (unsigned int) param);/*HFP*/
            break;
        case BT_EV_HF_DEV_INFO:
            rmt_dev = (bt_remote_device_t *)param;
            sprintf(event_buffer, "\r\n+HFPDEV=%02X%02X%02X%02X%02X%02X,%s\r\n", \
                            rmt_dev->bd[5], rmt_dev->bd[4], rmt_dev->bd[3], rmt_dev->bd[2], rmt_dev->bd[1], rmt_dev->bd[0], rmt_dev->name);
            bt_change_event_buffer_ff(event_buffer, strlen(event_buffer));
            break;
        case BT_EV_HF_LINK_LOST:
            sprintf(event_buffer, "\r\n+HFPLOST=%d\r\n", (unsigned int) param);/*HFP*/
            break;
        case BT_EV_HF_MIC_MUTE:
            sprintf(event_buffer, "\r\n+MICMUTE=%d\r\n", (unsigned int) param);/*HFP*/
            break;
        case BT_EV_A2DP_SET_MUTE:
            sprintf(event_buffer, "\r\n+A2DPMUTED=%d\r\n", (unsigned int) param);/*A2DP/AVRCP*/
            break;
        case BT_EV_A2DP_SET_SAMP:
            sprintf(event_buffer, "\r\n+A2DPSR=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_A2DP_GET_DEV:
            rmt_dev = (bt_remote_device_t *)param;
            sprintf(event_buffer, "\r\n+A2DPDEV=%02X%02X%02X%02X%02X%02X,%s\r\n", \
                            rmt_dev->bd[5], rmt_dev->bd[4], rmt_dev->bd[3], rmt_dev->bd[2], rmt_dev->bd[1], rmt_dev->bd[0], rmt_dev->name);
            bt_change_event_buffer_ff(event_buffer, strlen(event_buffer));
            break;
        case BT_EV_A2DP_PLAY_STAT:
             sprintf(event_buffer, "\r\n+PLAYSTAT=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_AVRCP_PLAYER_SETTTING_STATUS_IND:
            {
                sprintf(event_buffer, "\r\n+PLAYMODE=3,1\r\n");
                /*break;
                struct AVRCP_NumIDPairStru *in = (struct AVRCP_NumIDPairStru *)param;
                UINT8  repeat_value, shuffle_value, num = 0;
                for (num = 0;num < (in->num);num++) {   
                    switch (in->id[num].attr_id) {
                        case AVRCP_CMD_PASA_ID_REPEAT:
                            repeat_value = in->id[num].value_id;
                            break;
                        case AVRCP_CMD_PASA_ID_SHUFFLE:
                            shuffle_value = in->id[num].value_id;
                            break;
                        default:
                            break;
                    }
                }
                bse_log_print("at_event", "\r\n","repeat_value = %d shuffle_value = %d\n", repeat_value, shuffle_value);
                sprintf(event_buffer, "\r\n+PLAYMODE=%d,%d\r\n", repeat_value, shuffle_value);*/
            }
            break;
        case BT_EV_A2DP_LINK_LOST:
            sprintf(event_buffer, "\r\n+A2DPLOST=%d\r\n", (unsigned int) param);//d
            break;
        case BT_EV_AVRCP_BROW_STAT:
            sprintf(event_buffer, "\r\n+BROWSTAT=%d\r\n", (unsigned int) param);//d
            break;
        case BT_EV_AVRCP_BROW_DATA:
            sprintf(event_buffer, "\r\n+BROWDATA=%d\r\n", (unsigned int) param);//d
            break;
        case BT_EV_AVRCP_BIP_STAT:
            sprintf(event_buffer, "\r\n+BIPSTAT=%d\r\n", (unsigned int) param);//d
            break;
        case BT_EV_AVRCP_COVERART:
            sprintf(event_buffer, "\r\n+COVERART=%s\r\n", (char *) param);//d
            break;

        case BT_EV_PROFILE_IAPSTAT:
            sprintf(event_buffer, "\r\n+IAPSTAT=%d\r\n", (unsigned int) param);/*profile STAT*/
            break;
        case BT_EV_PROFILE_AAPSTAT:
            sprintf(event_buffer, "\r\n+AAPSTAT=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_PROFILE_HC1STAT:
            sprintf(event_buffer, "\r\n+HC1STAT=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_PROFILE_HC2STAT:
            sprintf(event_buffer, "\r\n+HC2STAT=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_PROFILE_GATTSTAT:
            sprintf(event_buffer, "\r\n+GATTSTAT=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_PROFILE_A2DPSTAT:
            sprintf(event_buffer, "\r\n+A2DPSTAT=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_PROFILE_AVRCPSTAT:
            sprintf(event_buffer, "\r\n+AVRCPSTAT=%d\r\n", (unsigned int)param);
            break;
        case BT_EV_PROFILE_HFPSTAT:
            sprintf(event_buffer, "\r\n+HFPSTAT=%d\r\n", (unsigned int)param);
            break;
        case BT_EV_PROFILE_PBSTAT:
            sprintf(event_buffer, "\r\n+PBSTAT=%d\r\n", (unsigned int)param);
            break;
        case BT_EV_SET_PROFILE_CONFIG:
            sprintf(event_buffer, "\r\n+PROFILE=%d\r\n", (unsigned int) param);/*GAP*/
            break;
        case BT_EV_SET_SPP_MODE:
            sprintf(event_buffer, "\r\n+SPP=%d\r\n", (unsigned int) param);
            break;
        case BT_EV_GET_LOCAL_COD:
            sprintf(event_buffer, "\r\n+COD=%s\r\n", (char *)param);
            break;
        case BT_EV_APP_PWR_STAT:
            sprintf(event_buffer, "\r\n+PWRSTAT=%d\r\n", (unsigned int)param);
            break;
        case BT_EV_SET_SSP_STAT:
            sprintf(event_buffer, "\r\n+SSP=%d\r\n", (unsigned int)param);
            break;
        case BT_APP_GAP_REMOVE_PLIST:
            {
                //bt_remote_device_t *p = (bt_remote_device_t *)param;
                sprintf(event_buffer, "\r\n+PLIST=%d\r\n", (unsigned int) param);
                break;
            }
        case BT_EV_EVENT_OK:
            sprintf(event_buffer, "\r\nOK\r\n");
            break;
         case BT_EV_EVENT_ERROR:
            sprintf(event_buffer, "\r\nERROR\r\n");
            break;
        case BT_EV_EVENT_PRIMITIVE:
            sprintf(event_buffer, "\r\n%s\r\n", param);
            break;
        case BT_EV_READ_RSSI:
            
            evt_param_read_remote_rssi_t *p_read_remote_rssi = (evt_param_read_remote_rssi_t*) param;
            bse_log_print("at_event", "\r\n", "read_rssi,rssi = %d", p_read_remote_rssi->rssi);
            unsigned char client_if;
            bt_bdaddr_t address;
            signed char rssi;
            unsigned char status;

            if (p_read_remote_rssi != NULL) {
            client_if = p_read_remote_rssi->client_if;
            address = p_read_remote_rssi->address;
            rssi = p_read_remote_rssi->rssi;
            status = p_read_remote_rssi->status;
            sprintf(event_buffer, "\r\n+RSSI:%02X,%02X%02X%02X%02X%02X%02X,%d,%02X\r\n",
                    client_if,
                    address.address[5], address.address[4], address.address[3],
                    address.address[2], address.address[1], address.address[0]
                    , rssi, status);
            }
            break;
        default:
            break;
    }

    event_len = strlen(event_buffer);
    //bse_log_print("at_event", "\r\n","len = %d", event_len);
    if (event_len > 0 && s_event_cb != NULL) {
        //bse_log_print("AT#EVT", "\r\n", "%s", event_buffer);
        //bse_log_print("+EVT", "\r\n", "%s", event_buffer);
        //bse_log_print("at_event", "\r\n", "[AT]   %s", &event_buffer[2]);
        if((event_buffer[2] == '+') || (memcmp(&event_buffer[2], "OK", 2) == 0) || (memcmp(&event_buffer[2], "ERROR", 5) == 0))//&& g_bt_opened
        {
            bse_log_print("at_event", "\r\n", "event_len:[%d]event_buffer:%s", event_len, event_buffer+2);
#if 0
            s_event_cb(event_buffer);
#else
		s_event_cb(event_buffer+2);
#endif
        }  
        //UART_Printf("[BRT] event_len = %d, event_buffer = %s", event_len, event_buffer);
    }
    LEAVE_CRITICAL(s_event_lock);
}
