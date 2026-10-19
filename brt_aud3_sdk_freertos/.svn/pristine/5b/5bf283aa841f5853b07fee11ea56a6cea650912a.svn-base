#ifndef _HLP_STREAM_H_
#define _HLP_STREAM_H_

#include "btdatatype.h"
#include "vendor_csr.h"
#ifdef CONFIG_BT_CHIP_CSR8350
//#define  USE_RINGTONE_FASTPIPE
//************BCCMD************
// BCCMD message Types
#define BCCMD_GETREQ 0x0000 // Client requests to be given the value of varid
#define BCCMD_GETRESP 0x0001 // Client requests to set the value of varid
#define BCCMDN_SETREQ 0x0002 // Response to GETREQ or SETREQ.

#define SAMPLE_RATE_48000						0xbb80
#define SAMPLE_RATE_44100						0xac44
#define SAMPLE_RATE_16000						0x3e80
#define SAMPLE_RATE_8000						0x1f40

#define OPSINK(opid) opid + 0xa000
#define OPSRC(opid) opid + 0x2000
#define OPSINKONE(opid) (OPSINK(opid) + 1)
#define OPSRCONE(opid) (OPSRC(opid) + 1)
#define HDR_SIZE 13

typedef enum PIO_Index {
	PIO2 = 2,
	PIO3,
	PIO4,
	PIO5,
	PIO6,
	PIO7,
	PIO8,
	PIO9,
} PIO_INDEX_MLTI;

typedef enum PCM_CHANNEL_Index {
	PCM1 = 0,
	PCM2,
} PCM_CHANNEL;

typedef enum PCM_Pin_define{
	PCM_IN = 4,
	PCM_OUT,
	PCM_SYNC,
	PCM_CLK,
}PCM_PIN_DEFINE;

#ifdef CONFIG_OS_WINCE
#pragma pack(push,1)//设为1字节对齐
#endif

#ifdef CONFIG_OS_WINCE
#pragma pack(pop)//还原为原来的字节对齐方式
#endif

typedef enum CAP_IDStru {
    PATCH_AUDTO_CAP,
    BASIC_MONO_AUDIO_CAP = 0x0001,
    BASIC_STEREO_AUDIO_CAP,
    SCO_SEND_CAP,
    SCO_RECEIVE_CAP,
    WBS_SEND_CAP,
    WBS_RECEIVE_CAP,
    SBC_A2DP_ENCODER_CAP,
    RESAMPLER_CAP = 0x0009,
    APTX_A2DP_ENCODER_CAP = 0x000b,
    SBC_DECODEC_CAP = 0x000D,
    APTX_DECODEC_CAP = 0x0010,
    RINGTONE_CAP,

    CVCHF1MIC_SEND_FE_CAP = 0x001A,
    CVCHF_RECEIVE_FE_CAP,
    CVCHF1MIC_SEND_NB_CAP,
    CVCHF_RECEIVE_NB_CAP,
    CVCHF1MIC_SEND_WB_CAP,
    CVCHF_RECEIVE_WB_CAP,
    CVCHF2MIC_SEND_NB_CAP,
    CVCHF2MIC_SEND_WB_CAP,
    CVCHF2MIC_SEND_FE_CAP,
} CAPABILITY_ID;

typedef enum Oper_MsgStru {
    SET_RINGTONE_PARAMS = 0x0000,
    SET_TONE_VOLUME,
    RINGTONE_ENABLE_FADE_OUT,
    RINGTONE_DISABLE_FADE_OUT,
    AUDIO_ENABLE_FADE_OUT,
    AUDIO_DISABLE_FADE_OUT,
    SET_SBC_PARAMS_MANUL = 7,
    FADE_OUT_COMPLTE = 0x0001,
    SET_RINGTONE_MUSIC = 0x03e8,
} OPERATORMSG_ID;

typedef enum Oper_MsgParam{
	ENABLE_FADE_OUT = 2,
	DISABLE_FADE_OUT,
}OPERATORMSG_PARAMID;

// set cvc params;
typedef enum Oper_CvcMsgIdStru {
    CVC_ENABLE_FADE_OUT,
    CVC_DISABLE_FADE_OUT,

    CVC_VOLUME,
    CVC_GET_PARAM,
    CVC_GET_PARAM_RESP = 3,
    CVC_SET_PARAM,
    CVC_SET_GROUP_PARAM,
    CVC_SET_MODE,
    CVC_GET_STATE = 8,
    CVC_GET_STATE_RESP = 8,
    CVC_SET_STATE,
};

typedef enum Source_TypeStru {
    PCM_TYPE = 0x0001,
    IIS_TYPE,
    CODEC_TYPE,
    FM_TYPE,
    SPDIF_TYPE,
    DIGITAL_MICROPHONE_TYPE,
    L2CAP_TYPE,
    FASTPIPE_TYPE,
    SCO_TYPE,
} SOURCE_TYPE;

typedef enum CVC2MicLisenceStatus_TypeStru {
    CVC_CODEC_MSG = 0x1006,
    FADE_OUT_COMPLETE_MSG = 0x1,
    SECURITY_PASSED_MSG = 0x100c,
    SECURITY_FAILED_MSG = 0x1013,
};

typedef enum Stream_Config_KeyStru {
    PCM_SYNC_RATE = 0x0100,
    PCM_MASTER_CLOCK_RATE,
    PCM_MASTER_MODE,
    PCM_SLOT_COUNT,
    PCM_MANCH_MODE_ENABLE,
    PCM_SHORT_SYNC_ENABLE,
    PCM_MANCHESTER_SLAVE_MODE,
    PCM_SIGN_EXTEND_MODE,
    PCM_LSB_FIRST_MODE,
    PCM_TX_TRI_STATE_MODE,
    PCM_TX_TRI_STATE_RISING_EDGE_MODE,
    PCM_SYNC_SUPPRESS_ENABLE,
    PCM_GCI_MODE,
    PCM_MUTE_ENABLE,
    PCM_LONG_LENGTH_SYNC,
    PCM_SAMPLE_RISING_EDGE,
    PCM_RX_RATE_DELAY = 0x0113,
    PCM_SAMPLE_FORMAT,
    PCM_MANCHESTER_MODE_RX_OFFSET,
    PCM_AUDIO_GAIN,

    I2S_SYNC_RATE = 0x0200,
    I2S_MASTER_CLOCK_RATE,
    I2S_MASTER_MODE,
    I2S_JUSTIFY_FORMAT,
    I2S_LEFT_JUSTIFY_DELAY,
    I2S_CHANNEL_POLARITY,
    I2S_AUDIO_ATTENUATION_ENABLE,
    I2S_AUDIO_ATTENUATION,
    I2S_JUSTIFY_RESOLUTION,
    I2S_CROP_ENABLE,
    I2S_BITS_PER_SAMPLE,
    I2S_TX_START_SAMPLE,
    I2S_RX_START_SAMPLE,

    //DSPconfig L2CAP MAX_PDU_LENGTH
    L2CAP_DSP_PDU_DEFINE1 = 0x2A0,
    L2CAP_DSP_PDU_DEFAULT = 0x2DA,

    CODEC_INPUT_SAMPLE_RATE = 0x0300,
    CODEC_OUTPUT_SAMPLE_RATE,
    CODEC_INPUT_GAIN,
    CODEC_OUTPUT_GAIN,
    CODEC_RAW_INPUT_GAIN,
    CODEC_RAW_OUTPUT_GAIN,
    CODEC_OUTPUT_GAIN_BOOST_ENABLE,
    CODEC_SIDE_TONE_GAIN,
    CODEC_SIDE_TONE_ENABLE,
    CODEC_MIC_INPUT_GAIN_ENABLE,
    CODEC_LOW_POWER_OUTPUT_STAGE,
    CODEC_QUALITY_MODE,
    CODEC_OUTPUT_INTERPOLATION_FILTER_MODE,
    CODEC_OUTPUT_POWER_MODE,
    CODEC_SIDE_TONE_SOURCE,
    CODEC_SIDETONE_SOURCE_MASK = 0x0311,
    CODEC_INDIVIDUAL_SIDETONE_GAIN,
    CODEC_INDIVIDUAL_SIDETONE_ENABLE,

    FM_INPUT_SAMPLE_RATE = 0x0400,
    FM_OUTPUT_SAMPLE_RATE,
    FM_INPUT_GAIN,
    FM_OUTPUT_GAIN,

    SPDIF_OUTPUT_SAMPLE_RATE = 0x0500,
    SPDIF_INPUT_CHANNEL_STATUS_REPORT_MODE,
    SPDIF_OUTPUT_CHANNEL_STATUS_DUPLICATE_ENABLE,
    SPDIF_OUTPUT_CHANNEL_STATUS_WORD,
    SPDIF_INPUT_AUTO_RATE_DETECT,

    DIGITAL_MIC_INPUT_SAMPLE_RATE = 0x0600,
    DIGITAL_MIC_INPUT_GAIN,
    DIGITAL_MIC_SIDE_TONE_GAIN,
    DIGITAL_MIC_SIDE_TONE_ENABLE,
    DIGITAL_MIC_CLOCK_RATE,
    DIGITAL_MIC_INDIVIDUAL_SIDETONE_GAIN = 0x0606,

    AUDIO_CHANNEL_MUTE_ENABLE = 0x0700,
    AUDIO_PRECISION,

    L2CAP_ATU = 0x0800,	//Between 1 and the MTU for the L2CAP channel;
    AUTOFLUSH,			// 0 (disabled) or 1 (enabled);
    TOKENS_ALLOCATED,	//At least enough tokens to send a PDU on the L2CAP channel of the size specified by the ATU;
    L2CAP_MAXIMUM_PDU_LENGTH,	//L2CAP PDUs that violate this will be discarded;
} STREAM_CONFIGKEY;

enum {
    STREAM_SCO,
    STREAM_A2DP,
    STREAM_RINGTONE,
	STREAM_LOCALAUDIO,
	STREAM_A2DP_SRC,
    STREAM_IDLE
};
typedef struct _STREAMTYPE_NAME {
	UINT8 stream_type;
	char  name[15];
} STREAM_TYPE_NAME;
//************BCCMD*Struct***********
typedef struct _BCCMD_HDR_ {
	UINT16 Type;
	UINT16 Length;
	UINT16 Seqno;
	UINT16 Varid;
	UINT16 Status;
} BCCMD_HDR, *PBCCMD_HDR;

typedef struct _BCCMD_ {
	BCCMD_HDR BcCmdHdr;
	UINT8 Payload[1];
} BCCMD, *PBCCMD;

typedef struct _DSP_RINGTONE_MANAGER {
	UINT16 sbc_ringtone_opid;
	UINT16 sbc_ringtone_sinkid1 ;
	UINT16 sbc_ringtone_sinkid2;
	UINT16 create_tingtone_SeqNum;
	UINT16 sink1_ringtone_SeqNum;
	UINT16 sink2_ringtone_SeqNum;
	UINT16 ringtone_stop_SeqNum;
	UINT16 sbc_ringtone_tid;
	UINT8  stream_ringtone_status;

#ifdef USE_RINGTONE_FASTPIPE
	UINT16 sbc_ringtone_src;
	UINT16 sbc_fastpipe_handle;
	UINT16 sbc_ringtone_fastpipe_tid;
	UINT32 fastpipe_first_sendlength;
#endif
} DSP_RINGTONE_MANAGER,*P_DSP_RINGTONE_MANAGER;

typedef struct _DSP_A2DP_MANAGER {
	UINT16 a2dp_getsink1_SeqNum;
	UINT16 a2dp_getsink2_SeqNum;
	UINT16 a2dp_tostop_SeqNum;
	UINT16 a2dp_createto_operatemsg_SeqNum;
	UINT16 a2dp_create_sbcoper_SeqNum;
	UINT16 a2dp_get_sbcsrc_SeqNum;
	UINT16 a2dp_confige_SeqNum;

	UINT16 sbc_a2dp_sink1;
	UINT16 sbc_a2dp_sink2;
	UINT16 sbc_a2dp_opid;
	UINT16 sbc_a2dp_tid1;
	UINT16 sbc_a2dp_tid2;
	UINT16 sbc_a2dp_src_id;
	UINT16 sbc_a2dp_src_tid;
	UINT16 acl_handle;
	UINT16 sbc_a2dp_cid;
	UINT8  stream_a2dp_status;
} DSP_A2DP_MANAGER,*P_DSP_A2DP_MANAGER;

typedef struct _DSP_SCO_MANAGER {
	UINT16 create_scosend_SeqNum;
	UINT16 create_scorecv_SeqNum;
	UINT16 create_cvchfmic_send_SeqNum;
	UINT16 create_cvchf_receive_SeqNum;
	UINT16 get_sco_src_SeqNum;
	UINT16 get_sco_sink_SeqNum;
	UINT16 get_codec_src_SeqNum;
	UINT16 get_codec_src2_SeqNum;
	UINT16 get_codec_sink1_SeqNum;
	UINT16 get_codec_sink2_SeqNum;
	UINT16 start_sco_SeqNum; 
	UINT16 sco_opermsg_tostop_SeqNum;
	UINT16 sco_config_tofadeout_connect;

	UINT16 sbc_sco_send_opid;
	UINT16 sbc_sco_receive_opid;
	UINT16 sbc_cvchf1mic_send_opid;
	UINT16 sbc_cvchf2mic_send_opid;
	UINT16 sbc_cvchf_receive_opid;
	UINT16 sbc_sco_srcid;
	UINT16 sbc_sco_sinkid;
	UINT16 sbc_codec_srcid;
	UINT16 sbc_codec_src2id;
	UINT16 sbc_codec_sinkid1;
	UINT16 sbc_codec_sinkid2;
	UINT8 sbc_sco_receive_fadeout_completed;
	UINT8 sbc_sco_send_fadeout_completed;
	UINT8 sbc_sco_cvchf_receive_fadeout_completed;
	UINT8 sbc_sco_cvchf1mic_send_fadeout_completed;
	UINT8 sbc_sco_cvchf2mic_send_fadeout_completed;

	UINT16 sbc_codec_sink_tid;
	UINT16 sbc_cvc1hfmic_send_tid;
	UINT16 sbc_cvc2hfmic_send_tid;
	UINT16 sbc_cvc2hfmic_send_tid2;
	UINT16 sbc_sco_receive_tid;
	UINT16 sbc_sco_send_tid2;
	UINT16 sbc_cvchf_receive_tid;
	UINT16 sbc_cvchf1mic_tid;
	UINT16 sbc_cvchf2mic_tid;
	UINT16 sbc_sco_send_tid1;
	UINT16 sco_hdl;
	UINT8  stream_sco_status;
	UINT16 sbc_sco_mic_gain;
	UINT16 sbc_sco_spk_gain;
	UINT16 change_sco_sink_SeqNum;
	UINT16 sbc_connect_sco_sink_SeqNum;
	UINT16 sbc_connect_sco_src_SeqNum;
} DSP_SCO_MANAGER,*P_DSP_SCO_MANAGER;

int hlp_audio_8350patch();
int hlp_audio_open( UINT16 cid, UINT16 acl_handle, UINT8 type);
int hlp_audio_start( UINT8 type);
int hlp_audio_stop( UINT8 type);
int hlp_audio_close( UINT8 type);
int hlp_audio_event_cfm( void* param);
/****************internal_use****************************/
int enable_sco_streams( WORD sco_status);
int stream_get_source( WORD src_type, WORD opt1, WORD opt2);
int stream_get_sink( WORD sink_type, WORD opt1, WORD opt2);
int stream_close_source( WORD src_id);
int stream_close_sink( WORD sink_id);
int hlp_close_source_resp(VENDOR_RESP* param);
int stream_configure( WORD sid, WORD key, INT32 key_value);
int stream_sync_sid( WORD sid1, WORD sid2);
int stream_alias_sink( WORD sinkid1, WORD sinkid2);
int create_operator_c( WORD capability_id, WORD item_count, WORD skip_count);
int stream_connect( WORD sbc_srcId, WORD sbc_sinkId);
int start_operators( WORD count, WORD* list_ids);
int stream_external_detach( WORD sid);
int stop_operators( WORD count, WORD *list_ids);
int stream_transform_disconnect( WORD tid);
int destroy_operators( WORD count, WORD* list_ids);
int reset_operators( WORD count, WORD *list_ids);
#ifdef USE_RINGTONE_FASTPIPE
int fastpipe_enable( WORD buffer_lenth);
int fastpipe_resize( WORD buffer_lenth);
int fastpipe_create( WORD *arg_list);
#endif
unsigned char get_codec_type(void);
void set_codec_type(unsigned char nType);
void hlp_get_sink_resp( void* param);
void hlp_sbc_config_resp( void* param);
void hlp_sbc_sync_resp( void* param);
void hlp_create_c_resp( void* param);
void hlp_operator_msg_resp( void* param);
void hlp_sbc_connect_resp( void* param);
void hlp_get_source_resp( void* param);
void hlp_dsp_config_resp( void* param);
//void hlp_msg_from_operator_resp( void* param);
void hlp_sbc_disconnect_resp( void* param);
void hlp_start_operator_resp( void* param);
void hlp_stop_operator_resp( void* param);
void hlp_close_sink_resp( void* param);
void hlp_destroy_operate_resp( void* param);
void hlp_sco_alias_sink_resp(void* param);
void hlp_sync_sink_resp(void* param);

#ifdef USE_RINGTONE_FASTPIPE
void hlp_fastpipe_enable_resp( void* param);
void hlp_fastpipe_resize_resp( void* param);
void hlp_fastpipe_create_resp( void* param);
#endif

void hlp_set_sco_config();
void hlp_stream_set_type( UINT8 stream_type);
UINT8 hlp_stream_get_type();
void hlp_set_patchindex_0();
char * hlp_findname(UINT8 type);
//use bccmd set dsp
void Dsp_manager_SetBccmd(WORD wBccmdVarid,WORD wValuelen ,WORD *pValueSet);
UINT8 hlp_audio_vendorCmd_resp(void* cfm_par);
UINT8 hlp_audio_vendorCmd_getreq(void* cfm_par_event);
void hlp_vendor_cmd_resp_error(WORD cmd,VENDOR_RESP* pVendresp);
void Pio32_Set_Direction(WORD ioIndex, WORD direction);
void Pio32_Set_HighLow(WORD ioIndex, WORD HighOrLow);
WORD getbit(void * param, int nIndex, int nIndex2);
int dspmanager_config_request();
int message_from_operator( void* param);
int get_chip_version(void);

#ifdef USE_RINGTONE_FASTPIPE
void hlp_fastpipe_sendwavfile( UINT8* filename);
void hlp_fastpipe_data( UINT8* data);
#endif

int hlp_GetLocalMicGain();
void hlp_SetLocalMicGain(UINT16 nMicGain);
void hlp_SetCVCMode(UINT nmode);
void hlp_SetA2DPStream_AptxMode(BOOL baptx,int nsamplerate);
BOOL hlp_GetA2DPStream_AptxMode(int* pnRate);
int hlp_GetLocalSpkGain();
void hlp_SetLocalSpkGain(UINT16 nSpkGain);
void hlp_SetChannelMuteMic(unsigned int flag);
unsigned int hlp_getChannelMuteState();
void hlp_stream_fadeout_ringtone(void);
UINT8 hlp_process_HQ_msg(void* cfm_par_event);
void stream_a2dp_stop_timer();
void stream_sco_stop_timer();
void stream_ringtone_stop_timer();
void hlp_csr_8350_call_use_mic(int flag);

#endif//end CONFIG_BT_CHIP_CSR8350
#endif
