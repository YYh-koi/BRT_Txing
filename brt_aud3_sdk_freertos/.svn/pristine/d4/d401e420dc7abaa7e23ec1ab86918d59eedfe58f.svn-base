#include "auto_at.h"
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "auto_hf.h"
#include "brt_trace.h"
#include "debug_utils.h"
#include "bt_utils.h"
#include "bt_audio_hw.h"

#define TAG "bse_at"
unsigned long vf_time;
unsigned long close_time;
extern BOOL g_sco_connect;
BOOL g_bt_opened = FALSE;
static int process_C(const char *cmd, unsigned short len);
static int process_D(const char *cmd, unsigned short len);
static int process_M(const char *cmd, unsigned short len);
static int process_P(const char *cmd, unsigned short len);
static int process_S(const char *cmd, unsigned short len);
static int process_V(const char *cmd, unsigned short len);


// cmd[0]: C M R T S H P G O A
void at_process_interpret(const char *head, const char *data, unsigned short len)
{
    if (!head)
    {
        return; 
    }
    bse_log_print("AT#CMD", "\r\n", "AT#%s", data);
    if ((*head == 'A' && *(head + 1) == 'T' && g_bt_opened) || 
    (!g_bt_opened && *data=='P'))
    {
        char *cmd = (char *) data;
        unsigned char err = 0;

        //bse_print_va("AT#CMD", "AT#%s", cmd);

        switch (*cmd)
        {
            case 'C':
                err = process_C(cmd + 1, len - 1);
                break;
            case 'D':
                err = process_D(cmd + 1, len - 1);
                break;
            case 'M':
                err = process_M(cmd + 1, len - 1);
                break;
            case 'P':
                err = process_P(cmd + 1, len - 1);
                break;
            case 'S':
                err = process_S(cmd + 1, len - 1);
                break;
            case 'V':
                err = process_V(cmd + 1,len - 1);
                break;
            default:
                err = AT_CMD_REPLY_NOT_SUPPORT;	
                break;
        }

    }
}

static int process_C(const char *cmd, unsigned short len)
{

    int ret = 0;

    if (!cmd || len <= 0)
    {
        return ret;
    }
    bt_dev_bt_manager_bind_hfp_a2dp(TRUE);
    switch (*cmd)
    {
        case 'A':
            ret = do_enter_pair_mode(cmd + 1, len - 1);
            break;
        case 'B':
            ret = do_leave_pair_mode(cmd + 1, len - 1);
            break;
        case 'C':
            ret = do_connect_device(cmd + 1, len - 1);
            break;
        case 'D':
            ret = do_disconnect_hfp(cmd + 1, len - 1);
            break;
        case 'E':
            ret = do_accept_call(cmd + 1, len - 1);
            break;
        case 'F':
            ret = do_reject_call(cmd + 1, len - 1);
            break;
        case 'G':
            ret = do_hangup_call(cmd + 1, len - 1);
            break;
        case 'H':
            ret = do_redial_last_call(cmd + 1, len - 1);
            break;
        case 'I':
            ret = do_voice_dial(cmd + 1, len - 1);
            break;
        case 'J':
            ret = do_cancel_voice_dial(cmd + 1, len - 1);
            break;
        case 'K':
            ret = do_increase_volume(cmd + 1, len - 1);
            break;
        case 'L':
            ret = do_decrease_volume(cmd + 1, len - 1);
            break;
        case 'M':
            //ret = do_mic_mute_unmute(cmd + 1, len - 1);
            break;
        case 'N':
            if(!g_sco_connect)
                ret = do_switch_audio(cmd + 1, len - 1);
            break;
        case 'P':
            if(g_sco_connect){  
                static int s_hf_sco_time = 0;
                bse_log_print("AT#CMD", "\r\n", "GetCurrTime()=%d s_hf_sco_time=%d\r\n", GetCurrTime(), s_hf_sco_time);
                if (abs(GetCurrTime() - s_hf_sco_time) >= 1000) {
                    bse_log_print("AT#CMD", "\r\n", "audio_close\r\n");
                    s_hf_sco_time = GetCurrTime();  
                    bt_porting_audio_close(AUDIO_TYPE_SCO);
                    //usleep(1000 * 100);
                    ret = do_switch_audio(cmd + 1, len - 1);
                }
            }
            break;
        case 'O':
            ret = do_switch_audio(cmd + 1, len - 1);
            break;
        case 'Q':          
            app_hf_3_way_control(0, HF_3_WAY_CONTROL_0);
            break;
        case 'R':
            app_hf_3_way_control(0, HF_3_WAY_CONTROL_1);
            break;
        case 'S':
            app_hf_3_way_control(0, HF_3_WAY_CONTROL_2);
            break;
        case 'V':
            ret = do_delete_all_paired_devices(cmd + 1, len - 1);
        case 'T':
            ret = do_remove_paired_devices(cmd + 1, len - 1);
            break;
        case 'W':
            ret = do_dial(cmd + 1, len - 1);
            break;
        case 'X':
            ret = do_dtmf(cmd + 1, len - 1);
            break;
        case 'Y':
            ret = do_get_hfp_status(cmd + 1, len - 1);
            break;
        case 'Z':
            ret = do_reset_bt(cmd + 1, len - 1);
            break;
    }

    return ret;
}
static int process_D(const char *cmd, unsigned short len)
{

    int ret = 0;

    if (!cmd || len <= 0)
    {
        return ret;
    }

    switch (*cmd)
    {

        case 'B':
            ret = do_pair_device(cmd + 1, len - 1);
            break;
        case 'A':
            ret = do_connect_a2dp(cmd + 1, len - 1);
            break;
        case 'C':
            ret = do_disconnect_a2dp(cmd + 1, len - 1);
            break;
        case 'D':
            ret = do_connect_a2dp_devices(cmd + 1, len -1);
            break;
        case 'E':
            ret = do_set_scan_mode(cmd + 1,len -1);
            break;
        case 'F':
           ret = do_get_bt_state(cmd + 1,len -1);
            break;
        case 'H':
            ret = do_get_scan_mode(cmd + 1,len -1);
        case 'I':
        	ret = do_ssp_user_confirm(cmd + 1,len -1);
            break;

        }
    return ret;
}

static int process_M(const char *cmd, unsigned short len)
{

    int ret = 0;

    if (!cmd || len <= 0)
    {
        return ret;
    }

    switch (*cmd)
    {
        case 'A':
            ret = do_music_play(cmd + 1, len - 1);
            break;
	case 'B':
            ret = do_music_pause(cmd + 1,len -1);
            break;
        case 'C':
            ret = do_music_stop(cmd + 1, len - 1);
            break;
        case 'D':
            ret = do_music_next(cmd + 1, len - 1);
            break;
        case 'E':
            ret = do_music_previous(cmd + 1, len - 1);
            break;
        case 'F':
            ret = do_inquiry_bt_info(cmd + 1, len - 1);
            break;
        case 'G':
            ret = do_enable_auto_connect(cmd + 1, len - 1);
            break;
        case 'H':
            ret = do_disable_auto_connect(cmd + 1, len - 1);
            break;
        case 'I':
            ret = do_connect_device(cmd + 1, len - 1);
            break;
        case 'J':
            ret = do_disconnect_a2dp(cmd + 1, len - 1);
            break;
        case 'K':
            ret = do_get_avrcp_playing_status(cmd + 1, len - 1);
            break;
        case 'L':
            ret = bt_avrcp_get_nowplaying_info(cmd + 1, len - 1);
            break;
        case 'M':
            ret = do_modify_bt_name(cmd + 1, len - 1);
            break;
        case 'N':
            ret = do_modify_paircode(cmd + 1, len - 1);
            break;
        case 'O':
            ret = do_get_avrcp_status(cmd + 1, len - 1);
            break;
        case 'P':
            ret = do_enable_auto_accept_call(cmd + 1, len - 1);
            break;
        case 'Q':
            ret = do_disable_auto_accept_call(cmd + 1, len - 1);
            break;
        case 'R':
            ret = do_avrcp_fast_play_forward(cmd +1 , len -1);
            break;
        case 'T':
            ret = do_avrcp_fast_play_rewind(cmd +1 , len -1);
            break;
        case 'V':
            ret = do_get_a2dp_status(cmd + 1, len - 1);
            break;
        case 'X':
            ret = do_get_paired_devices(cmd + 1, len - 1);
            break;
        case 'Y':
            //ret = do_get_bt_version(cmd + 1, len - 1);
            break;
    }

    return ret;
}

static int process_P(const char *cmd, unsigned short len)
{

    int ret = 0;

    if (!cmd || len <= 0)
    {
        return ret;
    }

    switch (*cmd)
    {
        case'1':
            bse_log_print("AT#CMD", "\r\n", "AT#%s power on", "P1");
            ret = do_open_bt();
            break;
        case'0':
            bse_log_print("AT#CMD", "\r\n", "AT#%s power off", "P0");
            ret = do_close_bt();
            break;
        case 'A':
            ret = do_get_sim_contacts(cmd + 1, len - 1);
            break;
        case 'B':
            ret = do_get_phone_contacts(cmd + 1, len - 1);
            break;
        case 'D':
            ret = do_get_local_contacts(cmd +1,len -1);
            break;
        case 'H':
            ret = do_get_outgoing_call(cmd + 1, len - 1);
            break;
        case 'I':
            ret = do_get_incoming_call(cmd + 1, len - 1);
            break;
        case 'J':
            ret = do_get_missed_call(cmd + 1, len - 1);
            break;
        case 'K':
            ret = do_get_history_call(cmd + 1, len - 1);
            break;
        case 'L':
            ret = do_get_next_five_contacts(cmd + 1, len - 1);
            break;
        case 'M':
            ret = do_get_previous_five_contacts(cmd + 1, len - 1);
            break;
        case 'N':
            //
            break;
        case 'X':
            ret = do_get_all_contacts(cmd + 1, len - 1);
            break;
        case 'Z':
            ret = do_abort_pbap(cmd + 1, len - 1);
            break;
        case 'R':
            
            break;
        case 'S':
            
            break;
        case 'T':
            ret = do_connect_device(cmd + 1, len - 1);
            break;
    }

    return ret;
}

static int process_S(const char *cmd, unsigned short len)
{
    int ret = 0;

    if (!cmd || len <= 0)
    {
        return ret;
    }
    bt_dev_bt_manager_bind_hfp_a2dp(TRUE);
    switch (*cmd)
    {
        case 'D':
            ret = do_discovery(cmd + 1, len - 1);
            break;
        case 'T':
            ret = do_cancel_discovery(cmd + 1, len - 1);
            break;
        case 'C':
            ret = do_connect_hfp(cmd + 1, len - 1);
            break;
        case 'E'    :
            ret = do_disconnect_hfp(cmd + 1, len - 1);
            break;
        case'X':
            ret = do_set_debug_level(cmd + 1,len -1);
            break;
    }

    return ret;
}

extern int upio_set_bluetooth_power(int on);

static int process_V(const char *cmd, unsigned short len)
{
    int ret = 0;

    if (!cmd || len <= 0)
    {
        return ret;
    }
    bt_dev_bt_manager_bind_hfp_a2dp(TRUE);
    
    switch (*cmd)
    {
        case 'A':
            ret = bt_a2dp_mute(TRUE);
            break;
        case 'B':
            ret = bt_a2dp_mute(FALSE);
            break;
        case 'E':
            ret = do_get_bt_bd();
            break;
        case '1':
            bse_log_print("AT#CMD", "\r\n", "AT#%s power on", cmd);
            upio_set_bluetooth_power(1);
            break;
        case '0':
            bse_log_print("AT#CMD", "\r\n", "AT#%s power off", cmd);
	    upio_set_bluetooth_power(0);
            break;
        case '2':
            bse_log_print("AT#CMD", "\r\n", "test quit", cmd);
            usleep(6*1000*1000);
            break;
#ifdef CONFIG_SCO_OVER_PCM
        case 'v':
            reset_audio_player();
            break;
#endif
        default:
            break;	
    }

    return ret;
}


