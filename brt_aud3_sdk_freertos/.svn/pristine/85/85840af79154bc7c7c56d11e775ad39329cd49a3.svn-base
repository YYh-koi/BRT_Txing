#include "btdatatype.h"
#include <stdio.h>
#include <semaphore.h>
#include "btdatatype.h"
#include "btlist.h"
#include "cmd_composition.h"
#include "internal_manager.h"
#include "auto_avrcp.h"
#include "brt_trace.h"

#define UNKNOWN_TEL_NUMBER "********"
#define CONTACT_DELIMITER (0xFF)

typedef struct tag_connected_device
{
    unsigned char bd[BD_ADDR_LEN];
    char name[DEV_NAME_LEN + 1];
} connected_device_t;
extern BOOL pbag_history;
extern BOOL pbag_finsh;
extern sem_t g_pbag_sig;
static connected_device_t s_current_connected_dev;
static struct AVRCPNowPlayStatusIndStru s_avrcp_play_status = {0};

typedef struct tag_at_discovery_stru
{
    int index;
    int discovering;
    struct ListStru dev_list;
} at_discovery_t;

static at_discovery_t s_discovery_data;
static int evt_dev_exist(UINT8 *bd);

//IA HFP 断开连接
int evt_hfp_disconnected(char *buffer, unsigned short len,UINT8 *bd)
{
    memset(s_current_connected_dev.bd, 0, BD_ADDR_LEN);
    return snprintf(buffer, len,"IA%02X%02X%02X%02X%02X%02X", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
}

//IB HFP 建立连接
int evt_hfp_connected(char *buffer, unsigned short len, UINT8 *bd)
{
    memcpy(s_current_connected_dev.bd, bd, BD_ADDR_LEN);
    return snprintf(buffer, len,  "IB%02X%02X%02X%02X%02X%02X", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
}

//IC 电话呼出
int evt_hfp_outgoing_call(char *buffer, unsigned short len, char *number)
{
    //return snprintf(buffer, len, "%s\r\n", "IC");
    return snprintf(buffer, len, "IC%s\r\n", (number ? number : UNKNOWN_TEL_NUMBER));
}

//ID 来电
int evt_hfp_incoming_call(char *buffer, unsigned short len, char *number)
{
    // FIXME
    return snprintf(buffer, len, "ID%s\r\n", (number ? number : UNKNOWN_TEL_NUMBER));
    //return snprintf(buffer, len, "ID%s\r\nIQ%2d%s\r\n", (number ? number : UNKNOWN_TEL_NUMBER),
     //       strlen(UNKNOWN_TEL_NUMBER), UNKNOWN_TEL_NUMBER);
}

//IF 电话挂断或电话拒接
int evt_hfp_call_rejected(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "IF");
}

//IG 电话接听
int evt_hfp_call_accepted(char *buffer, unsigned short len, char *number)
{
    // FIXME
    //return snprintf(buffer, len, "IG\r\nIR%s\r\n", (number ? number : UNKNOWN_TEL_NUMBER));
    return snprintf(buffer, len, "IG\r\n");
}

//II 进入配对模式
int evt_gap_pair_mode_entered(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "II");
}

//IJ 退出配对模式
int evt_gap_pair_mode_leaved(char *buffer, unsigned short len, int result)
{
    return snprintf(buffer, len, "IJ%d\r\n", (result ? 1 : 0));
}

//IQ 来电人名指示
int evt_hfp_incoming_call_name(char *buffer, unsigned short len, char *name)
{
    return snprintf(buffer, len, "IQ%2d%s\r\n", (name ? strlen(name) : strlen(UNKNOWN_TEL_NUMBER)), (name ? name : UNKNOWN_TEL_NUMBER));
}

//IR 当前通话号码
int evt_hfp_current_call_number(char *buffer, unsigned short len, char *number)
{
    return snprintf(buffer, len, "IR%s\r\n", (number ? number : UNKNOWN_TEL_NUMBER));
}

// MW 上报版本号
int evt_notify_version(char *buffer, unsigned short len, char *version)
{
    return snprintf(buffer, len, "MW%s_%s\r\n", version, __DATE__);
}

// MW 上报版本号
int evt_notify_version_ex(char *buffer, unsigned short len, char *version)
{
    return snprintf(buffer, len, "+VER=%s_%s\r\n", version, __DATE__);
}

//IS 模块初始化完毕
int evt_bt_initialized(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s %s\r\n", "IS", __DATE__);
    return 0;
}

//IV HFP 连接中
int evt_hfp_connecting(char *buffer, unsigned short len,UINT8 *bd)
{
    return snprintf(buffer, len, "IV%02X%02X%02X%02X%02X%02X", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
}

//MA AV 暂停或停止
int evt_a2dp_paused(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "MA");
    return 0;
}

//MB AV 播放
int evt_a2dp_playing(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "MB");
    return 0;
}

//MC HFP 语音连接成功
int evt_hfp_sco_established(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "T1");
}

//MD HFP 语音断开连接
int evt_hfp_sco_released(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "T0");
}

//ME FIXME
int ME(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "ME");
}
int evt_local_bd_ind(char *buffer, unsigned short len,char * bd)
{
    return snprintf(buffer, len, "%s%s\r\n", "DB",bd);
    //return snprintf(buffer, len, "%s%s\r\n", "+ADDR=",bd);
}

int evt_local_scan_mode_ind(char *buffer, unsigned short len,UINT8 mode)
{
    return snprintf(buffer, len, "DM%1d\r\n",(mode == 3) ? 1 : 0);
}
//MF 报告模块配置信息
int evt_bt_config_ind(char *buffer, unsigned short len, unsigned char accept_call, unsigned char auto_connect)
{
    return snprintf(buffer, len, "MF%1d%1d\r\n", (accept_call ? 1 : 0), (auto_connect ? 1 : 0));
}

//MG 报告 HFP 状态
int evt_hfp_status_ind(char *buffer, unsigned short len, unsigned char *bd, unsigned char status)
{
#ifdef CONFIG_DEFINE_BT_EVENT
    return snprintf(buffer, len, "MG%02X%02X%02X%02X%02X%02X%1d\r\n", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0],status);
#else 
    if (status > 6 /*FIXME:ST_HF_ONGOING*/) {
        return 0;
    }
    return snprintf(buffer, len, "MG%1d\r\n", (int) status);
#endif
}
int evt_income_ringtone(char *buffer, unsigned short len,UINT8 state)
{

    snprintf(buffer, len, "VR%d\r\n", state);
    return 0;
}
int evt_hf_3_ip_ind(char *buffer, unsigned short len,UINT8 index, UINT8 state,UINT8 number[32] )
{
    snprintf(buffer, len, "%s%01x|%01x|%s\r\n", "IP", index, state, number);
    return 0;
}
int evt_hf_3_it_ind(char *buffer, unsigned short len,UINT8 index, UINT8 state,UINT8 number[32])
{
   snprintf(buffer, len, "%s%01x|%01x|%s\r\n", "IT", index,  state, number);
    return 0;
}
int evt_hf_3_iq_ind(char *buffer, unsigned short len,UINT8 index, UINT8 state,UINT8 number[32])
{
   snprintf(buffer, len, "%s%01x|%01x|%s\r\n", "IQ",index, state, number);
   return 0;
}

int evt_hfp_subscriber_number(char *buffer, unsigned short len,UINT8 number[32])
{
    snprintf(buffer, len, "IU%s\r\n",number);
    return 0;
}
int evt_hfp_voice_recogn_start(char *buffer, unsigned short len)
{
#if 1
    snprintf(buffer, len,"%s\r\n","VR1");
#else
    snprintf(buffer, len,"%s\r\n","IR0000");
#endif
    return 0;
}

int evt_hfp_voice_recogn_stop(char *buffer, unsigned short len)
{
#if 1
        snprintf(buffer, len,"%s\r\n","VR0");
#else
        snprintf(buffer, len,"%s\r\n","IF");
#endif
    return 0;
}

int evt_avrcp_nowplay_status_ind(char *buffer, unsigned short len, void *param)
{
    struct AVRCPNowPlayStatusIndStru *p_data = (struct AVRCPNowPlayStatusIndStru *)param;
    s_avrcp_play_status.play_status = p_data->play_status;
    s_avrcp_play_status.song_length = p_data->song_length;
    s_avrcp_play_status.song_position = p_data->song_position;
    
    snprintf(buffer, len, "RP%d%c%d%c%02x", p_data->song_length,0xff, p_data->song_position,0xff, p_data->play_status);
    return 0;
}

int evt_avrcp_change_ind(char * buffer, unsigned short len,int  status)
{
	if (status == TRUE) {
       	return snprintf(buffer, len, "%s\r\n", "MB");
    } else {
        return snprintf(buffer, len, "%s\r\n", "MA");
    }
	   return 0;
}
int evt_avrcp_pos_changed(char *buffer, unsigned short len, int time)
{

   return snprintf(buffer, len, "RP%d%c%d%c%02x", s_avrcp_play_status.song_length,0xff, time,0xff, s_avrcp_play_status.play_status);

}

//ML 报告 AVRCP 状态
int evt_avrcp_status_ind(char *buffer, unsigned short len, unsigned char *bd, unsigned char status)
{
    if (status >= 3/* FIXME: ST_APP_AVRCP_CONNECTED*/)
    {
        status = 3;
    }
#ifdef CONFIG_DEFINE_BT_EVENT
    return snprintf(buffer, len, "ML%02X%02X%02X%02X%02X%02X%1d\r\n", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0], status);
#else
    return snprintf(buffer, len, "ML%1d\r\n", (int) status);
#endif
}

//MM 报告模块配对显示名称
int evt_gap_local_name(char *buffer, unsigned short len, const char *name)
{
    //return snprintf(buffer, len, "MM%s\r\n", name);
    return snprintf(buffer, len, "+NAME=%s\r\n", name);
}

//MN 报告模块配对密码
int evt_gap_pincode_ind(char *buffer, unsigned short len, const char *pin)
{
    return snprintf(buffer, len, "MN%s\r\n", pin);
}

//MU 报告 A2DP 状态
int evt_a2dp_status_ind(char *buffer, unsigned short len, unsigned char *bd, unsigned char status)
{
#ifdef CONFIG_DEFINE_BT_EVENT
    return snprintf(buffer, len, "MU%02X%02X%02X%02X%02X%02X%1d\r\n", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0], status);
#else
    return snprintf(buffer, len, "MU%1d\r\n", (int) status);
#endif
}

//MW 报告软件版本
int evt_bt_version_ind(char *buffer, unsigned short len, const char *version)
{
    return snprintf(buffer, len, "MW%s\r\n", version);
}
int evt_bt_init_sucess_ind(char *buffer, int flag,unsigned short len)
{
   if (flag == 1) {
        return snprintf(buffer, len, "PG\r\n");
    } else {
        return snprintf(buffer, len, "PH\r\n");
    }
}

//MX 报告配对信息
//当 HFP 连接成功后,以 index=0 的形式报告当前连接设备信息
int evt_gap_paired_device_ind(char *buffer, unsigned short len, unsigned short index, unsigned char *bd, const char *name)
{
    return snprintf(buffer, len, "MX%d%02X%02X%02X%02X%02X%02X%s\r\n", 
            index, bd[5], bd[4], bd[3], bd[2], bd[1], bd[0], name);
}

//MY A2DP 断开连接
int evt_a2dp_connected(char *buffer, unsigned short len,char *bd)
{
	return snprintf(buffer, len,  "MO%02X%02X%02X%02X%02X%02X", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
}
int evt_a2dp_disconnected(char *buffer, unsigned short len,char *bd)
{
    return snprintf(buffer, len,  "MY%02X%02X%02X%02X%02X%02X", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
}

int evt_avrcp_id3_ind(char *buffer, unsigned short len, const char *name, const char *artist, const char *album, const char *time)
{
	if (NULL != name && 0 != strlen(name)) {
		snprintf(buffer, len, "MI%s%c%s%c%s%c%s\r\n", name, 0xff, artist, 0xff, album, 0xff, time);
	}
	return 0;
}

//PA AT 指令下载电话本指示
int evt_pbap_status_ind(char *buffer, unsigned short len, unsigned char status)
{
    return snprintf(buffer, len, "PL%d\r\n", (status ? 1 : 0));
}
int evt_pbap_data_ind(char *buffer, unsigned short len, char *param)
{
	return snprintf(buffer , len, "PD%s\r\n", param);
}
//PB 电话本信息
int evt_pbap_contact_ind(char *buffer, unsigned short len, char *param)
{
    return snprintf(buffer , len, "PB%s\r\n", param);
}

//PC 电话本/通话记录下载完毕
int evt_pbap_sync_completed(char *buffer, unsigned short len)
{

    if(pbag_history)
    {
    	return snprintf(buffer, len, "%s\r\n", "PE");
    }
    else
	return snprintf(buffer, len, "%s\r\n", "PC");
}

int evt_pbap_supported_repos_ind(char *buffer, unsigned short len, unsigned char repos)
{
    unsigned char val = 0;
    if (repos != 0xFF)
    {
        val = 1;
    }

    return snprintf(buffer, len, "PA%1d\r\n", val);
}

//PE 语音拨号开始
int evt_hfp_voice_dial_begin(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "PE");
}

//PF 语音拨号停止
int evt_hfp_voice_dial_end(char *buffer, unsigned short len)
{
    return snprintf(buffer, len, "%s\r\n", "PF");
}
//QA 搜索到的手机信息
int evt_gap_remote_dev_found(char *buffer, unsigned short len, const unsigned char *bd, UINT8 status, const char *name)
{
    if (bd && name && s_discovery_data.discovering)
    {
        bse_log_print("bse_gap", "\r\n", "%s name:%s ,status:%d", __FUNCTION__, name,status);
        if (evt_dev_exist(bd))
        {
            return 0;
        }

        UINT8 *addr = (UINT8 *) List_NodeNew(BD_ADDR_LEN);
        memcpy(addr, bd, BD_ADDR_LEN);
        s_discovery_data.index++;
        List_AddTail(&s_discovery_data.dev_list, addr);
        return snprintf(buffer, len, "SF%04d%d%02X%02X%02X%02X%02X%02X%s\r\n", s_discovery_data.index, status,
                bd[5], bd[4], bd[3], bd[2], bd[1], bd[0], name);
    }

    return 0;
}

//QC 搜索手机完成
int evt_discovery_finished(char *buffer, unsigned short len)
{
    s_discovery_data.discovering = 0;
    return snprintf(buffer, len, "%s\r\n", "SH");
}

//CD 通话中取号码回复
int evt_hfp_ongoing_call_number_ind(char *buffer, unsigned short len, const char *number)
{
    return snprintf(buffer, len, "CD%s\r\n", number);
}

//helper function
void evt_composition_init()
{
    memset(&s_discovery_data, 0, sizeof(at_discovery_t));
}

int evt_discovery_started(char *buffer, unsigned short len)
{
    bse_log_print("bse_gap", "\r\n", "%s", "discovery started.");
    
    List_RemoveAll(&s_discovery_data.dev_list);
    memset(&s_discovery_data, 0, sizeof(at_discovery_t));
    s_discovery_data.discovering = 1;
    snprintf(buffer, len, "SB\r\n");
    return 0;
}

int evt_discovery_get_device(int index, UINT8 *bd)
{
    UINT8 *rmt_dev = s_discovery_data.dev_list.head;
    int count = 1;

    while (rmt_dev != NULL)
    {
        if (count == index)
        {
            memcpy(bd, rmt_dev, BD_ADDR_LEN);
            return 1;
        }

        count++;
        rmt_dev = LNEXT(rmt_dev);
    }

    return 0;
}

static int evt_dev_exist(UINT8 *bd)
{
    UINT8 *rmt_dev = s_discovery_data.dev_list.head;

    while (rmt_dev != NULL)
    {
        if (memcmp(rmt_dev, bd, BD_ADDR_LEN) == 0)
        {
            return 1;
        }

        rmt_dev = LNEXT(rmt_dev);
    }

    return 0;
}

int evt_spp_connected(char *buffer, unsigned short len, UINT8 *bd, UINT32 handle)
{ 
    return snprintf(buffer, len, "SC%02x%02x%02x%02x%02x%02x%08x", bd[0], bd[1], bd[2], bd[3], bd[4], bd[5], handle);
}

int evt_spp_disconnected(char *buffer, unsigned short len, UINT32 handle)
{
    return snprintf(buffer, len, "SD%08x", handle);
}

int evt_spp_data_received(char *buffer, unsigned short len, UINT32 handle, UINT32 data_len, UINT8 *data)
{
    int slen;

    snprintf(buffer, len, "D%04X%08X", (8 + data_len), handle);
    slen = strlen(buffer);
    len -= slen;

    if (len > 1)
    {
        len--;
        len = len > data_len ? data_len : len;
        memcpy(buffer + slen, data, len);
    }
    else
    {
        len = 0;
    }

    return slen + len;
}

int evt_bond_state_changed(char *buffer, unsigned short len, const unsigned char *bd, unsigned char state)
{
    if (bd)
    {
        bse_log_print("bse_gap", "\r\n", "%s state:%d.", __FUNCTION__, state);
        if (state < 3)
        {
            return snprintf(buffer, len, "GPB%02X%02X%02X%02X%02X%02X%1d", bd[5], bd[4], bd[3],bd[2],bd[1],bd[0],state);
        }
    }

    return 0;
}


int evt_get_connected_device_bd(char *buffer, unsigned short len, UINT32 handle)
{
    snprintf(buffer, len, "AD%02X%02X%02X%02X%02X%02X", s_current_connected_dev.bd[5], s_current_connected_dev.bd[4],
        s_current_connected_dev.bd[3], s_current_connected_dev.bd[2], s_current_connected_dev.bd[1], s_current_connected_dev.bd[0]);
    return 0;
}

int evt_get_connected_device_name(char *buffer, unsigned short len, UINT32 handle)
{
    snprintf(buffer, len, "SA%s", s_current_connected_dev.name);
    return 0;
}
