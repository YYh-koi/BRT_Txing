
#ifndef _BT_API_H__
#define _BT_API_H__
#if defined __cplusplus
extern "C" {
#endif


#define BT_DEBUG   printf

typedef signed char			int8;
typedef unsigned char 		uint8;
typedef signed short 		int16;
typedef unsigned short 		uint16;
typedef signed int 			int32;
typedef unsigned int 		uint32;

#ifndef NULL
#define	NULL	0
#endif

#define the_smaller(x,y)		(((x) < (y)) ? (x) : (y)) 
#define the_bigger(x,y)		(((x) < (y)) ? (y) : (x)) 
#define DEVICE_MASK_PWR_BIT							0x01
#define DEVICE_MASK_PAIRING_BIT						0x02
#define DEVICE_MASK_ADVERTISING_BIT					0x04
#define DEVICE_MASK_INQUIRING_BIT					0x08
#define DEVICE_MASK_SCANNING_BIT					0x10

typedef struct BTAddr_{
	uint8 address[6];
}BTAddr_t;

typedef enum{
	FSC_EVENT_NO,
	FSC_EVENT_DEV_PWR_IND,
	FSC_EVENT_VERSION_IND,
	FSC_EVENT_BAUD_IND,
	FSC_EVENT_SPKVOL_IND,
	FSC_EVENT_PROFILE_IND,
	FSC_EVENT_DEV_STATE_IND,
	FSC_EVENT_BT_NAME_IND,
	FSC_EVENT_BT_LENAME_IND,
	FSC_EVENT_BT_ADDR_IND,
	FSC_EVENT_BT_LEADDR_IND,
	FSC_EVENT_DEV_SSP_IND,
	FSC_EVENT_BT_PIN_IND,
	FSC_EVENT_BT_PAIR_IND,
	FSC_EVENT_BT_PAGE_IND,
	FSC_EVENT_INQUIRY_IND,
	FSC_EVENT_PLIST_IND,
	FSC_EVENT_PAIRED_RESULT_IND,
	FSC_EVENT_AUD_ROUTE_IND,
	FSC_EVENT_DEV_FOCUS_IND,
	FSC_EVENT_DEV_ADV_IND,
	FSC_EVENT_DEV_COVERART_IND,
	FSC_EVENT_HFP_CFG_IND,
	FSC_EVENT_HFP_STATE_IND,
	FSC_EVENT_HFP_CID_IND,
	FSC_EVENT_HFP_CIE_IND,
	FSC_EVENT_HFP_PHONE_INFO_IND,
	FSC_EVENT_HFP_PRIMARY_INFO_IND,
	FSC_EVENT_HFP_SECONDARY_INFO_IND,
	FSC_EVENT_HFP_AUDIO_IND,
	FSC_EVENT_HFP_SIG_IND,
	FSC_EVENT_HFP_ROAM_IND,
	FSC_EVENT_HFP_BATT_IND,
	FSC_EVENT_HFP_MANU_IND,
	FSC_EVENT_REMOTE_NUM_IND,
	FSC_EVENT_REMOTE_NET_IND,
	FSC_EVENT_HFP_VR_IND,
	FSC_EVENT_A2DP_STATE_IND,
	FSC_EVENT_A2DP_MUTE_IND,
	FSC_EVENT_A2DP_PHONE_INFO_IND,
	FSC_EVENT_A2DP_PRIMARY_INFO_IND,
	FSC_EVENT_A2DP_SECONDARY_INFO_IND,
	FSC_EVENT_AVRCP_STATE_IND,
	FSC_EVENT_AVRCP_CFG_IND,
	FSC_EVENT_PLAY_STATE_IND,
	FSC_EVENT_TRACK_STATE_IND,
	FSC_EVENT_TRACK_ID3_IND,
	FSC_EVENT_AVRCP_CTPLAY_IND,
	FSC_EVENT_AVRCP_CTPAUSE_IND,
	FSC_EVENT_AVRCP_CTSTOP_IND,
	FSC_EVENT_AVRCP_CTFWD_IND,
	FSC_EVENT_AVRCP_CTBACK_IND,
	FSC_EVENT_COVER_DATA_IND,
	FSC_EVENT_BIP_STATE_IND,
	FSC_EVENT_PBAP_STATE_IND,
	FSC_EVENT_PBAP_CNT_IND,
	FSC_EVENT_PBAP_DATA_IND,
	FSC_EVENT_SPP_STATE_IND,
	FSC_EVENT_SPP_DATA_IND,
	FSC_EVENT_GATT_STATE_IND,
	FSC_EVENT_GATT_DATA_IND,
	FSC_EVENT_GATT_UUID_IND,
	
}DashBTEvent_t;

//device power on or off
typedef enum{
	DEVICE_PWR_OFF,
	DEVICE_PWR_ON
}DashBTDevPwrState_t;

//device enable state
typedef enum{
	DEVICE_DISENABLE,
	DEVICE_ENABLE
}DashBTDevEnableState_t;

typedef enum{
	USER_SET_BT_NAME,			//设置蓝牙名称
	USER_SET_BT_LENAME,			//设置BLE名称
	USER_SET_BT_HAVE_SUFFIX,		//设置名称加后缀
	USER_SET_BT_NO_SUFFIX,		//设置名称不加后缀
}DashBTNameSet_t;

//SET&GET VOLUME CMD
typedef enum{
	SPEAKER_VOLUME_MINUS,		//音量减1
	SPEAKER_VOLUME_PLUS,			//音量加1
}DashBTSetVolume_t;

typedef enum{
	DELETE_PAIRED_ALL,			// 清除所有记录
	DELETE_PAIRED_INDEX,			// 清除索引对应的记录
	DELETE_PAIRED_ADDR,			// 清除地址码对应的记录
}DashBTRecordDelete_t;

typedef enum{
	DISCONN_DEV_PHONE,			//断开手机
	DISCONN_DEV_HEAD,			//断开耳机
	DISCONN_DEV_ALL,				//断开所有连接设备
}DashBTDisconnAction_t;

typedef enum{
	SSP_LEGACY_PAIRING,			//(0) Legacy pairing, use pin code for pairing
	SSP_SIMPLE_AUTO_PAIRING,		//(1) Secure simple pairing, auto pairing
	SSP_CONFIRM_PAIRING,			//(2) Secure simple pairing, display yes/no in pairing
	SSP_PASSKEY_PAIRING,			//(3) Secure simple pairing, passkey compare,
}DashBTSspState_t;

typedef enum{
	REJECT_REQUEST,				//拒绝请求
	ACCEPT_REQUEST,				//接收请求
}DashBTReject_t;

typedef enum{
	PAIR_MODE_NONE,							//不可发现不可连接
	PAIR_MODE_DISCOVERABLE_UNCONNECTABLE,	//可发现不可连接
	PAIR_MODE_UNDISCOVERABLE_CONNECTABLE,	//不可发现能连接
	PAIR_MODE_DISCOVERABLE_CONNECTABLE,		//可发现能连接
}DashBTPairMode_t;

typedef enum{
	LEAVE_ALL_DISCOVERABLE_MODE,	//Leave BR/EDR/BLE discoverable mode (stop advertising/broadcasting)
	ENTER_EDR_DISCOVERABLE_MODE,	//Enter BR/EDR discoverable mode (start broadcasting)
	ENTER_BLE_DISCOVERABLE_MODE,	//Enter BLE discoverable mode (start advertising)
	ENTER_ALL_DISCOVERABLE_MODE,	//Enter BR/EDR/BLE discoverable mode (start advertising/broadcasting)
}DashBTDiscoverableMode_t;

typedef enum{
	EDR_UNCONNECTABLE,
	EDR_CONNECTABLE,
}DashBTEdrConnectable_t;

typedef enum{
	NONE = -1,
	PRIMARY_DEVICE,		//主头盔
	SECONDARY_DEIVCE,	//副头盔
	
}DashBTPrimDev_t;

typedef enum{
	AUD_LOCAL_DEVICE,		//切换到本地音频发射模式
	AUD_PRIMARY_DEVICE,		//切换到主耳通话模式(手机蓝牙通话使用此模式),默认此模式
	AUD_SECONDARY_DEIVCE,	//切换到副耳通话模式
	AUD_PRIMARY_LOCAL,		//切换到主耳本地通话模式(声音从模块的spk和mic输出输入)
	AUD_SECONDARY_LOCAL,		//切换到副耳本地通话模式(声音从模块的spk和mic输出输入)
	
}DashBTAudrouteDev_t;

typedef enum{
	HSCFG_GET,			//获取HSCFG状态
	HSCFG_ON,			//打开 耳机控制播放/暂停,上下曲,接听挂断等功能
	HSCFG_OFF,			//关闭 耳机端的控制功能
}DashBTHSCfg_t;

typedef enum
{
	HFP_UNSUPPORTED,
	HFP_STANDBY,
	HFP_CONNECTING,
	HFP_CONNECTED,
	HFP_OUTGOINGCALL,
	HFP_INCOMINGCALL,
	HFP_ACTIVECALL,
	HFP_END_STATE,
}DashBTHfpState_t;

typedef enum
{
	AUDIO_IN_REMOTE,
	AUDIO_IN_MODULE,
}DashBTHfpAudio_t;

typedef enum
{
	VOICE_WAKE_STOP,			//停止语音识别
	VOICE_WAKE_START,		//开始语音识别
}DashBTHfpVR_t;

typedef enum
{
	A2DP_UNSUPPORTED,
	A2DP_STANDBY,
	A2DP_CONNECTING,
	A2DP_CONNECTED,
	A2DP_PAUSED,
	A2DP_STREAMING,
	A2DP_END_STATE,
}DashBTA2dpState_t;

typedef enum{
	A2DP_MUTE_OFF,			//关闭 A2DP MUTE 静音
	A2DP_MUTE_ON,			//打开 A2DP MUTE 静音
}DashBTA2dpMute_t;

typedef enum
{
	AVRCP_UNSUPPORTED,
	AVRCP_STANDBY,
	AVRCP_CONNECTING,
	AVRCP_CONNECTED,
}DashBTAvrcpState_t;

typedef enum
{
    AVRCP_PLAY_STATE_STOPPED,
    AVRCP_PLAY_STATE_PLAYING,
    AVRCP_PLAY_STATE_PAUSED,
    AVRCP_PLAY_STATE_FWD,
    AVRCP_PLAY_STATE_REV,
} DashBTPlayState_t;

typedef enum
{
	BIP_UNSUPPORTED,
	BIP_STANDBY,
	BIP_CONNECTING,
	BIP_CONNECTED,
	BIP_DOWNLOADING,
}DashBTBipState_t;

typedef enum
{
	PBAP_UNSUPPORTED,
	PBAP_STANDBY,
	PBAP_CONNECTING,
	PBAP_CONNECTED,
	PBAP_DOWNLOADING,
}DashBTPbapState_t;

typedef enum
{
	PBAP_DATA_TYPE_SIM,
	PBAP_DATA_TYPE_PHONE,
	PBAP_DATA_TYPE_RECV_CALL,
	PBAP_DATA_TYPE_DIAL_CALL,
	PBAP_DATA_TYPE_MISS_CALL,
	PBAP_DATA_TYPE_EVERY_CALL,
	PBAP_DATA_TYPE_END,
}DashBTPbapDataType_t;

typedef enum
{
	SPP_UNSUPPORTED,
	SPP_STANDBY,
	SPP_CONNECTING,
	SPP_CONNECTED,
}DashBTSppState_t;

typedef enum
{
	GATT_UNSUPPORTED,
	GATT_STANDBY,
	GATT_CONNECTING,
	GATT_CONNECTED,
}DashBTGattState_t;

typedef struct
{
	DashBTDevPwrState_t pwrstate;
}FSC_EVENT_PWR_STATE_IND_T;

typedef struct
{
	char mode_type[8];
	int mode_version;
	char mode_birthday[10];
}FSC_EVENT_VERSION_IND_T;

typedef struct
{
	uint32 cur_baud;
}FSC_EVENT_BAUD_IND_T;

typedef struct
{
	uint8 a2dp_volume;
	uint8 hfp_volume;
}FSC_EVENT_SPKVOL_IND_T;

typedef struct
{
	uint8 devstate;
}FSC_EVENT_DEV_STATE_IND_T;

typedef struct
{
	char name[32];
}FSC_EVENT_BT_NAME_IND_T;

typedef struct
{
	BTAddr_t addr;
}FSC_EVENT_BT_ADDR_IND_T;

typedef struct
{
	DashBTSspState_t ssp_state;
}FSC_EVENT_DEV_SSP_IND_T;

typedef struct
{
	char pin_word[16];
}FSC_EVENT_BT_PIN_IND_T;

typedef struct
{
	DashBTDiscoverableMode_t pair_action;
}FSC_EVENT_BT_PAIR_IND_T;

typedef struct
{
	DashBTEdrConnectable_t page_action;
}FSC_EVENT_BT_PAGE_IND_T;

typedef struct
{
	uint8 scan_end;
	uint8 order;
	int8 rssi;
	uint8 dev_type;
	BTAddr_t addr;
	char name[32];
	int dev_cod;

}FSC_EVENT_INQUIRY_IND_T;

typedef struct
{
	uint8 list_end;
	uint8 order;
	uint32 profile;
	BTAddr_t addr;
	char name[50];
}FSC_EVENT_PLIST_IND_T;

typedef struct
{
	uint8 paired_cause;	//配对结果 (0)-成功 (1~255)-失败原因
	BTAddr_t addr;
}FSC_EVENT_PAIRED_RESULT_IND_T;

typedef struct
{
	uint8 route;
}FSC_EVENT_AUD_ROUTE_IND_T;

typedef struct
{
	BTAddr_t primary_addr;
	BTAddr_t secondary_addr;
}FSC_EVENT_DEV_FOCUS_IND_T;

typedef struct
{
	uint8 adv_len;
	char adv[66];
	char adv_name[64];

}FSC_EVENT_DEV_ADV_IND_T;

typedef struct
{
	uint8 coverart;				//支持专辑图片
}FSC_EVENT_DEV_COVERART_IND_T;

typedef struct
{
	DashBTHfpState_t phone_state;
	DashBTHfpState_t primary_state;
	DashBTHfpState_t secondary_state;
}FSC_EVENT_HFP_STATE_IND_T;

typedef struct
{
	char call_number[25];		//来电/去电电话号码
}FSC_EVENT_HFP_CID_IND_T;

typedef struct
{
	char call_name[50];		//来电/去电电话名称
}FSC_EVENT_HFP_CIE_IND_T;

typedef struct
{
	BTAddr_t remote_addr;
	char remote_name[100];
}FSC_EVENT_HFP_INFO_IND_T;

typedef struct
{
	DashBTHfpAudio_t phone_audio;
	DashBTHfpAudio_t primary_audio;
	DashBTHfpAudio_t secondary_audio;
}FSC_EVENT_HFP_AUDIO_IND_T;

typedef struct
{
	uint8 sig;				//远端设备网络信号强度(0~5)
}FSC_EVENT_HFP_SIG_IND_T;

typedef struct
{
	uint8 roam;				//远端设备漫游状态(0/1)
}FSC_EVENT_HFP_ROAM_IND_T;

typedef struct
{
	int8 phone_batt;			//(-1~5) 远程设备的电池电量, 手机端的电池电量,-1 表示未获取到电池电量;
	int8 primary_batt;		//(-1~9) 远程设备的电池电量, 第一路耳机的电池电量,-1 表示未获取到电池电量;
	int8 secondary_batt;		//(-1~9) 远程设备的电池电量, 第二路耳机的电池电量,-1 表示未获取到电池电量;
}FSC_EVENT_HFP_BATT_IND_T;

typedef struct
{
	char manu_name[100];		//远端设备制造商
}FSC_EVENT_HFP_MANU_IND_T;

typedef struct
{
	char remote_num[25];		//远端设备本机号码
}FSC_EVENT_REMOTE_NUM_IND_T;

typedef struct
{
	char remote_net[50];		//远端设备网络运营商
}FSC_EVENT_REMOTE_NET_IND_T;

typedef struct
{
	DashBTHfpVR_t voice_remote;		//远端设备语音唤醒
}FSC_EVENT_HFP_VR_IND_T;

typedef struct
{
	DashBTA2dpState_t phone_state;
	DashBTA2dpState_t primary_state;
	DashBTA2dpState_t secondary_state;
}FSC_EVENT_A2DP_STATE_IND_T;

typedef struct
{
	DashBTA2dpMute_t mute_state;
}FSC_EVENT_A2DP_MUTE_IND_T;

typedef struct
{
	BTAddr_t remote_addr;
	char remote_name[100];
}FSC_EVENT_A2DP_INFO_IND_T;

typedef struct
{
	DashBTAvrcpState_t phone_state;
	DashBTAvrcpState_t primary_state;
	DashBTAvrcpState_t secondary_state;
}FSC_EVENT_AVRCP_STATE_IND_T;

typedef struct
{
	uint8 cfg;
}FSC_EVENT_AVRCP_CFG_IND_T;

typedef struct
{
	DashBTPlayState_t play_state;
}FSC_EVENT_PLAY_STATE_IND_T;

typedef struct
{
	DashBTPlayState_t play_state;
	uint16 cursenconds;
	uint16 totaltime;
}FSC_EVENT_TRACK_STATE_IND_T;

typedef struct
{
	char id3_title[128];
	char id3_artist[128];
	char id3_album[128];
}FSC_EVENT_TRACK_ID3_IND_T;

typedef struct
{
	uint16 data_len;
}FSC_EVENT_COVER_DATA_IND_T;

typedef struct
{
	DashBTBipState_t bip_state;
}FSC_EVENT_BIP_STATE_IND_T;


typedef struct
{
	DashBTPbapState_t pbap_state;
}FSC_EVENT_PBAP_STATE_IND_T;

typedef struct
{
	uint16 pbap_count;
}FSC_EVENT_PBAP_CNT_IND_T;

typedef struct
{
	DashBTPbapDataType_t type;
	char name[100];
	char num[25];
	char time[20];
}FSC_EVENT_PBAP_DATA_IND_T;

typedef struct
{
	DashBTSppState_t spp_state;
}FSC_EVENT_SPP_STATE_IND_T;

typedef struct
{
	uint16 data_len;
}FSC_EVENT_SPP_DATA_IND_T;

typedef struct
{
	DashBTGattState_t gatt_state;
}FSC_EVENT_GATT_STATE_IND_T;

typedef struct
{
	uint16 data_len;
}FSC_EVENT_GATT_DATA_IND_T;

typedef struct
{
	uint8 uuid_state;
}FSC_EVENT_GATT_UUID_IND_T;


//typedef void (*DashBT_StatusCBK)(DashBTEvent_t, void *);
extern void (*g_callback)(DashBTEvent_t, void *);


//蓝牙功能接口封装
int fsc_bt_set_enable(DashBTDevEnableState_t enable);
int fsc_bt_get_version(void);
int fsc_bt_get_dev_state(void);
int fsc_bt_get_dev_name(void);
int fsc_bt_get_dev_addr(void);
int fsc_bt_get_headset(void);
int fsc_bt_get_paired_record(void);
int fsc_bt_set_baud(uint16 baudrate);				//设置蓝牙波特率
int fsc_bt_set_autoconn(uint16 reconnect);			//设置上电回连
int fsc_bt_set_name(DashBTNameSet_t event,const char* s);		//设置蓝牙名称
int fsc_bt_set_ssp(DashBTSspState_t ssp);			//设置BR/EDR 配对模式
int fsc_bt_reject_or_accept_request(DashBTReject_t event,BTAddr_t* addr);		//接受/拒绝远端的配对请求
int fsc_bt_delete_paired_record(DashBTRecordDelete_t del,uint8 para,BTAddr_t* addr);		//删除配对记录
int fsc_bt_get_discover_connect_mode(void);							//获取蓝牙可发现可连接模式
int fsc_bt_set_discover_connect_mode(DashBTPairMode_t mode);		//设置蓝牙可发现可连接模式
int fsc_bt_set_inquiry(DashBTDevEnableState_t enable);		//蓝牙搜索附近设备
int fsc_bt_inquiry_tyre_pressure(const char* name, uint8 time_count);			//扫描胎压  time: count * 1.28s
int fsc_bt_reset(void);		//复位蓝牙
int fsc_bt_resume_default(void);		//蓝牙恢复出厂设置
int fsc_bt_set_speaker_volume(DashBTSetVolume_t volume);		//音量操作
int fsc_bt_set_headset(DashBTPrimDev_t device,BTAddr_t* addr);		//设置耳机连接
int fsc_get_connected_device_info(void);		//获取连接设备信息
int fsc_bt_connect_device(BTAddr_t* addr, uint8 hfp_a2dp);						//连接设备
int fsc_bt_disconnect_action(DashBTDisconnAction_t action);		//断开连接设置
int fsc_bt_set_hs_mode(DashBTHSCfg_t hscfg);
int fsc_bt_get_dev_rssi(BTAddr_t* addr);

int fsc_bt_get_hfp_state(void);
int fsc_bt_hfp_dial_last_call(void);				//拨打最后一个电话
int fsc_bt_hfp_dial_number_call(char* para);		//拨打指定电话
int fsc_bt_hfp_send_dtmf(char para);		//拨打指定电话
int fsc_bt_hfp_answ_call(void);		//接听电话
int fsc_bt_hfp_chup_call(void);			//挂断电话
int fsc_bt_hfp_adts_call(DashBTHfpAudio_t para);			//通话中语音切换
int fsc_bt_hfp_vr_action(DashBTHfpVR_t action);		//语音识别
int fsc_bt_sco_primary_device_switching(DashBTAudrouteDev_t device);		//音频路由设置
int fsc_bt_hfp_vgs_set(uint8 para);				//调整通话时耳机端spkeaker输出音量

int fsc_bt_set_music_play(void);                //播放 
int fsc_bt_set_music_pause(void);                 //暂停 
int fsc_bt_set_music_playpause(void);        //播放、暂停    
int fsc_bt_set_music_backward(void);        //上一曲     
int fsc_bt_set_music_forward(void);                //下一曲
int fsc_bt_set_music_mute(DashBTA2dpMute_t a2dpmute);	//静音 A2DP 音频
int fsc_bt_get_avrcp_cfg(void);				//读 AVRCP 配置
int fsc_bt_set_avrcp_cfg(uint8 para);			//写 AVRCP 配置

int fsc_bt_get_phonebook_state(void);
int fsc_bt_phonebook_connect(void);
int fsc_bt_phonebook_disconnect(void);
int fsc_bt_phonebook_download(DashBTPbapDataType_t para);
int fsc_bt_phonebook_abort(void);
int fsc_bt_get_spp_state(void);
int fsc_spp_data_send(char *para);		//发送SPP数据
int fsc_bt_get_gatt_state(void);
int fsc_gatt_data_send(char *para);		//发送BLE数据
int fsc_bt_get_gatt_uuid(void);
int fsc_bt_set_gatt_uuid(DashBTDevEnableState_t enable);



#if defined __cplusplus
}
#endif
#endif
