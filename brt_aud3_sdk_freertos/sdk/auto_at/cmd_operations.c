#include "btdatatype.h"
#include <semaphore.h>
#include "porting_utils.h"
//#include "TL.h"
#include "gap_ui.h"
#include "auto_commfunc.h"
#include "auto_avrcp.h"
#include "internal_manager.h"
#include "auto_pbap.h"
#include "auto_hf.h"
#include "auto_gap.h"
#include "auto_hid.h"
#include "auto_avrcp.h"
#include "avrcp_ui.h"
#include "auto_hid.h"
#include "auto_map.h"
#include "bt_event.h"
#include "bt_manager.h"
#include "btinit.h"
#include "bt_functions.h"
//#include "Trace.c"
#ifdef CONFIG_CBP
#include "auto_cb_screen.h"
#endif

#include "internal_cb_screen.h"

#ifdef CONFIG_SPP
//#include "auto_rfcomm_sdp.h"
#endif

#ifdef CONFIG_SPP_ANDROID
#include "auto_rfcomm.h"
#endif

#ifdef CONFIG_GATT
#include "bluedroid/bt_bluedroid_types.h"
#include "bt_gatt_client.h"
#endif
#include "cmd_composition.h"
#include "cmd_operations.h"
#include "Bt_power_manager.h"
#include "auto_at_internal.h"

static unsigned char pbap_target = 0;
BOOL pbag_history = FALSE;
sem_t g_pbag_sig;
BOOL pbag_finsh = FALSE;

int do_enter_pair_mode(const char *param, unsigned short len)
{
    bt_hf_set_pair_mode(1);
    return 0;
}

int do_leave_pair_mode(const char *param, unsigned short len)
{
    bt_hf_set_pair_mode(0);
    return 0;
}

int do_set_scan_mode(const char *param, unsigned short len)
{   
    UINT8 scan_mode = 0;
    if(*param == '0' && len){
        scan_mode = 2;
    }else if(*param == '1'){
        scan_mode = 3;
    }else{
        return 1;
    }
    bt_set_scan_mode(scan_mode);
    return 0;
}

int do_set_pair_mode_ex(int value)
{   

    //bt_set_scan_mode(value);
    //return 0;

    int scan_mode = do_get_scan_mode_ex();
    printf("pair_scan_mode = %d\n", scan_mode);
    if(scan_mode < 0)
        return -1;

    if(scan_mode < 2)//not conn
    {
        if(value)
            bt_set_scan_mode(1);
        else
            bt_set_scan_mode(0);
    }else{
        if(value)
            bt_set_scan_mode(3);
        else
            bt_set_scan_mode(2);
    }
    return 0;
}

int do_set_page_mode_ex(int value)
{   
    int scan_mode = do_get_scan_mode_ex();
    printf("page_scan_mode = %d\n", scan_mode);
    if(scan_mode < 0)
        return -1;
    if(scan_mode % 2){//scan
        if(value)
            bt_set_scan_mode(3);
        else
            bt_set_scan_mode(1);
    }else{
        if(value)
            bt_set_scan_mode(2);
        else
            bt_set_scan_mode(0);
    }
	return 0;
}

int do_set_page_mode(int value)
{
    int page_mode = do_get_scan_mode_ex();
    if(page_mode < 0)
        return -1;
    if(page_mode < 2){
        if(value)
            bt_set_scan_mode(1);
        else
            bt_set_scan_mode(0);
    }else{
        if(value)
            bt_set_scan_mode(3);
        else
            bt_set_scan_mode(2);
    }
	return 0;
}

int do_get_device_cod()
{
    bt_get_device_cod();
    return 0;
}

int do_get_scan_mode(const char *param, unsigned short len)
{   
    bt_get_scan_mode();
    return 0;
}

int do_get_scan_mode_ex()
{   
    return bt_get_scan_mode_ex();
}

int do_get_config_hfp()
{
    bt_get_config_hfp();
     return 0;
}

int do_connect_hfp(const char *param, unsigned short len)
{
    UINT8 bd_addr[BD_ADDR_LEN];

    if (!param || strlen(param) == 0 || len <= 0)
    {
        app_dev_get_address(0, bd_addr);
        bt_hf_connect(0, bd_addr);
    }
    else if (len < 3) // FIXME index length?
    {
        int index = param[0] - '1';

        index = index < 0 ? 0 : index;
        app_dev_get_address(param[0] -'1', bd_addr);
        bt_hf_connect(0, bd_addr);
    }
    else // BD
    {
        int i;

        for (i = 0; i < 6; i++)
        {
            bd_addr[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        }
        bt_hf_connect(0, bd_addr);
    }

    return 0;
}

int do_disconnect_hfp(const char *param, unsigned short len)
{
    bt_hf_disconnect();
    return 0;
}
int do_set_debug_level(const char *param,unsigned short len)
{
    int level =  0x10000000;
    bt_set_debug_level(level);
    return 0;
}

int do_accept_call(const char *param, unsigned short len)
{
    bt_hf_answer_call();
    return 0;
}

int do_reject_call(const char *param, unsigned short len)
{
    bt_hf_deny_call();
    return 0;
}

int do_hangup_call(const char *param, unsigned short len)
{
    bt_hf_hangup();
    return 0;
}

int do_redial_last_call(const char *param, unsigned short len)
{
    bt_hf_repeat();
    return 0;
}

int do_voice_dial(const char *param, unsigned short len)
{
    bt_hf_speech_recognition(1);
    return 0;
}

int do_cancel_voice_dial(const char *param, unsigned short len)
{
    bt_hf_speech_recognition(0);
    return 0;
}

int do_increase_volume(const char *param, unsigned short len)
{
    // TODO
    return 0;
}

int do_decrease_volume(const char *param, unsigned short len)
{
    // TODO
    return 0;
}

int do_mic_mute_unmute(const char *param, unsigned short len)
{
    static int flag = 1;

    flag = !flag;
    bt_hf_mute_micro_phone(flag);
    return 0;
}

int do_get_current_call_number(const char *param, unsigned short len)
{
    // TODO 输出为 CD<number>
    return 0;
}

int do_switch_audio(const char *param, unsigned short len)
{
    
    //static int s_hf_sco_time = 0;
    //if (abs(GetCurrTime() - s_hf_sco_time) >= 1000) {
        //s_hf_sco_time = GetCurrTime();  
        bt_hf_switch_audio();
    //}
    return 0;
}

int do_set_micmute_stat(int flag)
{
    bt_hf_set_micmute_stat(flag);
    return 0;
}

int do_delete_all_paired_devices(const char *param, unsigned short len)
{
    bt_dev_remove_device(0);
    // FIXME
    bt_hf_disconnect();

    return 0;
}

int do_remove_paired_devices(const char *param, unsigned short len)
{  
    if (param && (len > 0)) {
        int i;
        unsigned char bd[BD_ADDR_LEN];

        for (i = 0; i < 6; i++) {
            bd[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        }
        bt_remove_paired_device(bd);
    }else{
        bt_remove_paired_device(NULL);
    }
    return 0;
}

int do_set_max_bond_devices(int value)
{
    if(value < 8)
    {
        bt_set_max_bond_devices(value);
        return 0;
    }
    return 1;
}

int do_dial(const char *param, unsigned short len)
{
    if (param && len > 0)
    {
        bt_hf_dial(param);
    }

    return 0;
}

int do_dtmf(const char *param, unsigned short len)
{
    if (param && len > 0)
    {
        bt_hf_send_dtmf(param);
    }

    return 0;
}

int do_get_hfp_status(const char *param, unsigned short len)
{
    bt_hf_query_status();

    return 0;
}

int do_reset_bt(const char *param, unsigned short len)
{
    bt_reset();

    return 0;
}

int do_action_on_bluetooth_enabled_ex()
{
    bt_action_on_bluetooth_enabled_ex();
    return 0;
}

#ifdef CONFIG_BIND_PROFILES
int do_avrcp_connect(char *param, int len, int d_flag)
{
    
    UINT8 bd_addr[BD_ADDR_LEN];
    bse_log_print("command", "%s len = %d d_flag = %d", __func__, len, d_flag);
    if (!d_flag)
    {
        bt_avrcp_connect_last();
    }
    else if (len < 3) // FIXME index length?
    {
        app_dev_get_address(param[0] -'1', bd_addr);
        bt_avrcp_connect(bd_addr);
    }
    else // BD
    {
        int i;

        for (i = 0; i < 6; i++)
        {
            bd_addr[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        }
        bt_avrcp_connect(bd_addr);
    }
    return 0;
}

int do_avrcp_set_config(UINT8 param)
{
    bt_avrcp_set_config(param);
}

int do_avrcp_get_config()
{
    bt_avrcp_get_config();
}

int do_avrcp_disconnect()
{
    bt_avrcp_disconnect();
}
#endif
int do_avrcp_bip_art()
{
    bt_avrcp_bip_art();
}

int do_music_play_pause()
{
    bt_avrcp_play_pause();
    return 0;
}

#ifdef CONFIG_ABS_VOL
int do_music_volume_up()
{
    bt_avrcp_volume_up();
    return 0;
}

int do_music_volume_down()
{
    bt_avrcp_volume_down();
    return 0;
}
int do_send_abs_vol(const char *val)
{
    printf("abs vol:%s\n", val);
    int vol = atoi(val);
    bt_avrcp_send_abs_vol(vol);
    return 0;
}
#endif

int do_music_play(const char *param, unsigned short len)
{
    //static int play = 1;

    //if (play)
    //{
        bt_avrcp_play();
    //}
    //else
    //{
     //   bt_avrcp_pause();
    //}

    //play = !play;

    return 0;
}
int do_music_pause(const char *param, unsigned short len)
{
	bt_avrcp_pause();
	return 0;
}

int do_music_stop(const char *param, unsigned short len)
{
	
    bt_avrcp_stop();

    return 0;
}

int do_music_next(const char *param, unsigned short len)
{
    bt_avrcp_next();

    return 0;
}

int do_music_previous(const char *param, unsigned short len)
{
    bt_avrcp_previous();

    return 0;
}

int do_inquiry_bt_info(const char *param, unsigned short len)
{
    bt_dev_get_local_flag();

    return 0;
}

int do_enable_auto_connect(const char *param, unsigned short len)
{
    bt_dev_set_local_auto_connect(1);

    return 0;
}

int do_disable_auto_connect(const char *param, unsigned short len)
{
    bt_dev_set_local_auto_connect(0);

    return 0;
}

int do_connect_a2dp(const char *param, unsigned short len)
{
    UINT8 bd_addr[BD_ADDR_LEN];

    if (len <= 0)
    {
        bt_a2dp_connect_last();
    }
    else if (len < 3) // FIXME index length?
    {
        app_dev_get_address(param[0] -'1', bd_addr);
        bt_a2dp_connect(bd_addr);
    }
    else // BD
    {
        int i;

        for (i = 0; i < 6; i++)
        {
            bd_addr[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        }
        bt_a2dp_connect(bd_addr);
    }
    return 0;
}

int do_connect_a2dp_devices(const char *param, unsigned short len)
{
    int i;
    unsigned char bd[BD_ADDR_LEN];
    if (strlen(param) >= 12) //12 for bd_addr,1 for "C"
    {
        for (i = 0;i < 6;i++) {
            bd[5 - i] = ctoi(param[i * 2])*16 + ctoi(param[i * 2 + 1]);
        }
        bt_a2dp_connect(bd);
    } else {
        return 1;
    }
    return 0;
}


int do_disconnect_a2dp(const char *param, unsigned short len)
{
    bt_a2dp_disconnect_current();

    return 0;
}

int do_a2dp_mute(BOOL ismute)
{
	bt_a2dp_mute(ismute);
	return 0;
}

int do_modify_paircode_ex()
{
    //bt_manager_set_local_device_pin_ex();
    return 0;
}

int do_do_modify_bt_name_ex(UINT8 *name)
{
    bt_manager_set_local_device_name_ex(name);
    return 0;
}

int do_modify_bt_name(const char *param, unsigned short len)
{
    
    if (param && len > 0)
    {
        printf("len > 0\n");
        bt_dev_operate_local_name(param);
    }
    else
    {
        printf("len < 0\n");
        //char tmp[1] = {'\0'};
        bt_dev_operate_local_name(NULL);
    }
    
    return 0;
}
int do_get_bt_bd(void)
{
    bt_dev_get_local_bd();
    return 0;
}

int do_get_bt_le_bd()
{
    bt_dev_get_local_le_bd();
    return 0;
}

char *do_get_bt_bd_ex(char *bd_buff)
{
    bt_dev_get_local_bd_ex(bd_buff);
    return NULL;
}

int do_modify_paircode(const char *param, unsigned short len)
{
    if (param && len > 0)
    {
        bt_dev_operate_local_pin(param);
    }
    else
    {
        //char tmp[1] = {'\0'};
        bt_dev_operate_local_pin(NULL);
    }

    return 0;
}

int do_gap_set_ssp_mode(int value)
{
    bt_set_spp_mode(value);
    return 0;
}

int do_gap_set_usr_pair_mode(int value)
{
    bt_set_usr_pair_mode(value);
    return 0;
}

int do_gap_set_ssp_cap_mode(int value)
{
    bt_set_ssp_cap_mode(value);
    return 0;
}

int do_gap_set_io_cap_mode(int value)
{
    bt_set_io_cap_mode(value);
    return 0;
}

int do_get_bt_state(const char *param, unsigned short len)
{
	if(bt_get_power_state())
	{
            bt_dev_operate_local_name(NULL);

            //hf state

            //a2dp state
            
	}
	return 0;
}
int do_get_avrcp_status(const char *param, unsigned short len)
{
    bt_avrcp_query_status();

    return 0;
}

int do_enable_auto_accept_call(const char *param, unsigned short len)
{
    bt_dev_set_local_auto_answer(1);

    return 0;
}

int do_disable_auto_accept_call(const char *param, unsigned short len)
{
    bt_dev_set_local_auto_answer(0);

    return 0;
}
int do_avrcp_fast_play_forward(const char *param, unsigned short len)
{
    if(*param == '1')
         bt_avrcp_fast_forward(TRUE);
    else if(*param == '0')
         bt_avrcp_fast_forward(FALSE);
    return 0;
}
int do_avrcp_fast_play_rewind(const char *param, unsigned short len)
{
    if(*param == '1')
         bt_avrcp_fast_rewind(TRUE);
    else if(*param == '0')
         bt_avrcp_fast_rewind(FALSE);
    return 0;

}

int do_avrcp_set_player_app(UINT32 eID, UINT32 eValue)
{
    bt_avrcp_set_player_app(eID, eValue);
    return 0;
}

int do_avrcp_get_folder_list(UINT32 start_item, UINT32 end_item)
{
    bt_avrcp_get_folder_list(start_item, end_item);
    return 0;
}

int do_get_avrcp_playing_status(const char *param, unsigned short len)
{
    bt_avrcp_get_play_status();

    return 0;
}
int do_get_a2dp_status(const char *param, unsigned short len)
{
    bt_a2dp_query_status();

    return 0;
}

int do_get_paired_devices(const char *param, unsigned short len)
{
#if 0
    if (hlp_hf_get_status() >= ST_HF_CONNECTED)
    {
        UINT8 device_info[274];
        UINT8 *bd = get_connected_device_bd();
        char *name = get_connected_device_name();
        snprintf(device_info, sizeof(device_info), "000000000%02x%02x%02x%02x%02x%02x%s", bd[5],
                bd[4], bd[3], bd[2], bd[1], bd[0], name);

        bt_event_indicate(BT_EV_OTHER_PAIRIED_DEVICE_IND, device_info);
    }
    else
#endif
    {
        bt_dev_get_pairied_device();
    }

    return 0;
}

int do_set_bt_profile()
{

    return 0;
}

int do_get_bt_profile()
{

    return 0;
}

int do_open_bt()
{
    bt_enable_bluetooth();
    return 0;
}

int do_open_bt_ex()
{
    bt_enable_bluetooth_ex();
    return 0;
}

int do_close_bt()
{
     bt_disable_bluetooth();
     return 0;
}

int do_pbap_connect()
{
    UINT8 bd_addr[BD_ADDR_LEN] = {0};

    app_dev_get_address(0, bd_addr);
    bt_pbap_connect(bd_addr);
}

int do_get_pbap_book(int type, int offset,int count)
{
    //printf("%s\n",__func__);
    if(count > 0)
        bt_pbap_get_phonebook(type, offset, count);
    else
        bt_pbap_get_phonebook(type, 0, 50);
}

int do_get_pbap_get_vcardlisting(UCHAR *number)
{
    bt_pbap_get_vcardlisting(number);
}

int do_get_sim_contacts(const char *param, unsigned short len)
{
    pbag_history = FALSE;
    pbap_target = PT_PBAP_SIM_IDX;
    bt_pbap_get_phonebook(PT_PBAP_SIM_IDX, 0, 0xFFFF);
    bt_event_indicate(BT_EV_PBAP_SUPPORTED_REPOS, (void *)1);

    return 0;
}

int do_get_next_contact(const char *param, unsigned short len)
{
    // TODO
    return 0;
}

int do_get_previous_contact(const char *param, unsigned short len)
{
    // modify 20161116, test SC command
    if (param && len > 0)
    {
        int i;
        unsigned char bd[BD_ADDR_LEN];

        for (i = 0; i < 6; i++)
        {
            bd[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        }

        bse_log_print("at_command", "\r\n", "bd = %02X:%02X:%02X:%02X:%02X:%02X:.", bd[0], bd[1], bd[2], bd[3], bd[4], bd[5]);
        app_gap_create_bond(bd);
    }
    // modify 20161116, test SC command
    
    return 0;
}

int do_get_outgoing_call(const char *param, unsigned short len)
{
    pbag_history = TRUE;
    pbap_target = PT_PBAP_OCH;
    bt_pbap_get_phonebook(PT_PBAP_OCH, 0, PABP_CALLOG_SIZE);//add by cjs, 0411
    bt_event_indicate(BT_EV_PBAP_SUPPORTED_REPOS, (void *)1);

    return 0;
}

int do_get_incoming_call(const char *param, unsigned short len)
{
    pbag_history = TRUE;
    pbap_target = PT_PBAP_ICH;
    bt_pbap_get_phonebook(PT_PBAP_ICH, 0, PABP_CALLOG_SIZE);
    bt_event_indicate(BT_EV_PBAP_SUPPORTED_REPOS, (void *)1);
    return 0;
}

int do_get_missed_call(const char *param, unsigned short len)
{
    pbag_history = TRUE;
    pbap_target = PT_PBAP_MCH;
    bt_pbap_get_phonebook(PT_PBAP_MCH, 0, PABP_CALLOG_SIZE);
    bt_event_indicate(BT_EV_PBAP_SUPPORTED_REPOS, (void *)1);
    return 0;
}
int do_get_history_call(const char *param, unsigned short len)
{
#ifdef CONFIG_CARLIFE_CERTIFICATE
	pbag_history = TRUE;
	pbap_target = PT_PBAP_CCH;
	bt_pbap_get_phonebook(PT_PBAP_CCH, 0, PABP_CALLOG_SIZE);
	bt_event_indicate(BT_EV_PBAP_SUPPORTED_REPOS, 1);
#else
	do_get_outgoing_call(NULL,0);
#endif
	return 0;
}

int do_pbap_disc_stop()
{
    bt_pbap_disc_stop();
    return 0;
}

int do_get_history_call_ex(UINT16 offset, UINT16 count)
{
	bt_pbap_get_phonebook(PT_PBAP_CCH, offset, count);

	return 0;
}

int do_get_next_five_contacts(const char *param, unsigned short len)
{
    // TODO
    return 0;
}

int do_get_previous_five_contacts(const char *param, unsigned short len)
{
    // TODO
    return 0;
}

int do_get_phone_contacts(const char *param, unsigned short len)
{
    pbag_history = FALSE;
    pbap_target = PT_PBAP_TEL_IDX;
    bt_pbap_get_phonebook(PT_PBAP_TEL_IDX, 0, 0xFFFF);
    bt_event_indicate(BT_EV_PBAP_SUPPORTED_REPOS, (void *)1);

    return 0;
}
int do_get_local_contacts(const char *param, unsigned short len)
{

      bt_pbap_get_phonebook(PT_PBAP_TEL_IDX, 0, 0xFFFF);
      
    return 0;

}


int do_get_all_contacts(const char *param, unsigned short len)
{
    bt_pbap_get_phonebook(pbap_target, 0, 0xFFFF);

    return 0;
}

int do_abort_pbap(const char *param, unsigned short len)
{
    pbap_target = 0;
    bt_pbap_cancel_get_phonebook();
    return 0;
}

int do_discovery(const char *param, unsigned short len)
{
    static int s_discovery_time = 0;
    if (abs(GetCurrTime() - s_discovery_time) >= 2000) {
        s_discovery_time = GetCurrTime();
        bt_start_discovery(0, 1);
    } else {
       // UART_Printf("discovery busy!\r\n");
        bt_event_indicate(BT_EV_DISCOVERY_FINISHED, NULL);
    }

    return 0;
}

int do_inquery_start(int time_len, char type)
{
    bt_start_discovery(time_len, type);
    return 0;
}

int do_cancel_discovery(const char *param, unsigned short len)
{
    bt_cancel_discovery();

    return 0;
}

int do_connect_device(const char *param, unsigned short len)
{
    bse_log_print("at_command","\r\n","param:%s,len:%d",param,len);
    if (param && len > 0)
    {
        long index = strtol(param, NULL, 10);
        int i;
        unsigned char bd[BD_ADDR_LEN];
        if(strlen(param) >= 12) //12 for bd_addr,1 for "C"
        {
                for(i=0;i<6;i++)
                {
                    bd[5-i] = ctoi(param[i*2])*16+ctoi(param[i*2+1]);
                }
                bt_hf_connect(0, bd);
       }	
         else if (evt_discovery_get_device(index, bd))
        {
            bt_cancel_discovery();  
            bt_hf_connect(0, bd);
        }
    }
    bt_dev_set_local_auto_connect(0);
    return 0;
}

int do_pair_device(const char *param, unsigned short len)
{
    if (param && len > 0)
    {
        int i;
        unsigned char bd[BD_ADDR_LEN];

        for (i = 0; i < 6; i++)
        {
            bd[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        }

        bse_log_print("at_command", "\r\n", "bd = %02X:%02X:%02X:%02X:%02X:%02X:.", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
        app_gap_create_bond(bd);
    }

    return 0;
}

int do_ssp_user_confirm(const char *param, unsigned short len)
{
    if (param && len > 0)
    {
        int i;
        unsigned char bd[BD_ADDR_LEN];
        UINT8 accept = *(param+12) - '0';
                

        for (i = 0; i < 6; i++)
        {
            bd[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        }

        bse_log_print("at_command", "\r\n", "bd = %02X:%02X:%02X:%02X:%02X:%02X: accept %d.", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0], accept);
        app_gap_user_confirmation(bd, accept);
    }

    return 0;
}
static UINT8 btad_buff[6] = {0};
void do_ssp_pair_remote_addr(UINT8 *addr_buff)
{
    memset(btad_buff, 0, 6);
    memcpy(btad_buff, addr_buff, 6);
}

int do_ssp_user_confirm_ex(UINT8* addr, UINT8 accept)
{
    
        // int i;
        // unsigned char bd[BD_ADDR_LEN];
                
        // for (i = 0; i < 6; i++)
        // {
        //     bd[5 - i] = ctoi(param[i * 2]) * 16 + ctoi(param[i * 2 + 1]);
        // }

        // bse_log_print("at_command", "\r\n", "bd = %02X:%02X:%02X:%02X:%02X:%02X: accept %d.", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0], accept);
        //do_get_bt_bd_ex(bd_buff);
    if(addr == NULL){
        app_gap_user_confirmation(btad_buff, accept);
    } else {
        app_gap_user_confirmation(addr, accept);
    }
    

    return 0;
}

int do_get_le_local_name()
{
    bt_gatt_get_le_local_name();
    return 0;
}

int do_le_start_adv(char* advData,int advLen)
{
    bt_le_start_adv(advData, advLen);
    return 0;
}

int do_le_stop_adv()
{
    bt_le_stop_adv();
    return 0;
}

#define WL_DEBUG
#ifdef WL_DEBUG

int do_get_sdk_profile()
{
    bt_get_sdk_prfile();
    return 0;
}

int do_set_sdk_profile(UINT16 value)
{
    bt_set_sdk_prfile(value);
    return 0;
}

int do_get_bt_sdk_version()
{
    bt_get_sdk_version();
    return 0;
}

int do_discnnect_all_profile()
{
    bt_discnnect_all_profile();
    return 0;
}

#endif
#ifdef CONFIG_SPP
int do_spp_get_status()
{
    bt_spp_get_status();
}

int do_spp_connect(UINT16 channel, UINT8 *addr, char *uuid)
{
    bt_spp_connect(channel, addr, uuid);
}

int do_spp_disconnect()
{
    bt_spp_disconnect();
}

int do_spp_send_data(UINT8 *data, UINT32 len)
{
    bt_spp_send_data(data, len);
}

UINT16 do_spp_get_server_channel()
{
    app_spp_get_server_channel();
}

int do_aap_send_data(UINT8 *data, UINT32 len)
{
    bt_aap_send_data(data, len);
}

int do_iap_send_data(UINT8 *data, UINT32 len)
{
    bt_iap_send_data(data, len);
}
#endif

#ifdef CONFIG_HID 
void do_hid_connect(void)
{
    bt_hid_connect();
}

void do_hid_disconnect(void)
{
     bt_hid_disconnect();
}

void do_hid_get_status(void)
{
    //bt_hid_get_status();
}

void do_hid_send_coordinate(UINT8 button, INT16 x, INT16 y)
{
    bt_hid_send_coordinate(button, x, y);
}

void do_hid_send_key_value(UINT8 key_value)
{
    bt_hid_send_key_value(key_value);
}

void do_hid_send_media_value(UINT8 value)
{
    bt_hid_send_media_value(value);
}
#endif
