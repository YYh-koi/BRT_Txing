#include "btdatatype.h"
#include "vendor_interface.h"
#include "auto_commfunc.h"
#include "profile.h" //module_event_cbk_t 
#include "auto_gap.h" // EV_APP_GAP_XXX
#include "brt_trace.h"
#include <fcntl.h>

#define TAG "vendor_xxx"

#ifdef TAG
#define LOG(fmt, ...) bse_log_print(TAG, "\r\n", fmt, ##__VA_ARGS__)
#else
#define LOG(...) ((void*)0)
#endif


//Some addr configured in eeprom
#define I8008_MIC_GAIN_ADDR                     0x00C5
#define I8008_LOCAL_VERSION_ADDR1               0X0FF6
#define I8008_LOCAL_VERSION_ADDR2               0X0FF7
#define BT_CHIP_ISSC_SPK_CTL_PIO                4
#define FLAG_ISSC_AUDIO_PARAM                   0xAE
#define FLAG_ISSC_FIREWARE_VERSION_PARAM        0xAF

#define I8008_VENDOR_CMD_WRITE_EEPROM           0x27
#define I8008_VENDOR_CMD_READ_EEPROM            0x29
#define I8008_VENDOR_CMD_TONE_GENERATION        0x39
#define I8008_VENDOR_CMD_AVDTP_LINK_INFO        0x63
#define I8008_VENDOR_CMD_PLAY_RING              0x66

static void vendor_issc_init(vendor_interface_callback callback);
static void vendor_issc_done();
static void vendor_issc_pre_reset();
static void vendor_issc_post_reset();
static void vendor_issc_cmd_cbk(void *context, UINT8 result, void *cfm_par);
static void vendor_issc_event_ind(void *param, UINT32 size);
static void vendor_issc_on_gap_started();
static char* vendor_issc_get_firmware_version();
static UINT8* vendor_issc_get_device_info(); /* get device information(Local config, Remote device list) from EEPROM.*/

static void vendor_issc_ringtone_start(UINT8 count);
static void vendor_issc_ringtone_stop();
static void vendor_issc_ringtone_close();
static void vendor_issc_on_sco_established(UINT16 handle);
static void vendor_issc_on_sco_released();
static void vendor_issc_on_codec_configured(UINT8 select_codec);

static void vendor_issc_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle);
static void vendor_issc_on_stream_start();
static void vendor_issc_on_stream_suspend();
static void vendor_issc_on_stream_done();
static void vendor_issc_on_stream_configured(UINT8 codec, UINT16 sample);

static void vendor_issc_set_mic_gain(UINT8 gain);
static void vendor_issc_get_mic_gain();
static void vendor_issc_mic_control(UINT8 flag); /* mute/unmute */
static void vendor_issc_set_local_volume(UINT8 volume);
static int vendor_issc_get_local_volume();
static int vendor_issc_update_audio_param();

static char s_firmware_version[MAX_LOCAL_FIRMWARE_VERSION_LEN];
static struct DevInfoGotEvStru *s_dev_info = NULL; // XXX

static vendor_interface_callback s_event_cbk = NULL;
static vendor_interface_t s_vendor_itf;

static BOOL is_mic_gain_address(UINT8 low, UINT8 high);
static BOOL is_eeprom_address(UINT8 low, UINT8 high);
static void vendor_issc_get_local_firmware_version(UINT8 h_addr, UINT8 l_addr, UINT8 value);
static void vendor_issc_event_indcate(UINT32 event, void *param);

static void app_comm_SetPOLow(unsigned char nIndex);
static void app_comm_SetPOHigh(unsigned char nIndex);
static void app_comm_Set8008PO(unsigned char nPOIndex,unsigned char bHigh);
static void app_comm_get_local_version(void);
static void app_comm_enable_stream_process(int bEnable,unsigned short acl_handle,unsigned short channel_cid);
static void app_comm_set_audio_cbk(unsigned char h_addr, unsigned char l_addr, unsigned char data);

vendor_interface_t *vendor_interface_create()
{
	memset(s_firmware_version, 0, MAX_LOCAL_FIRMWARE_VERSION_LEN);
	memset(&s_vendor_itf, 0, sizeof(vendor_interface_t));

	s_vendor_itf.vendor_init = vendor_issc_init;
	s_vendor_itf.vendor_done = vendor_issc_done;
	s_vendor_itf.vendor_pre_reset = vendor_issc_pre_reset;
	s_vendor_itf.vendor_post_reset = vendor_issc_post_reset;
	s_vendor_itf.vendor_cmd_cbk = vendor_issc_cmd_cbk;
	s_vendor_itf.vendor_event_ind = vendor_issc_event_ind;
	s_vendor_itf.vendor_on_gap_started = vendor_issc_on_gap_started;
	s_vendor_itf.vendor_get_firmware_version = vendor_issc_get_firmware_version;
	s_vendor_itf.vendor_get_device_info = vendor_issc_get_device_info;

	s_vendor_itf.vendor_ringtone_start = vendor_issc_ringtone_start;
	s_vendor_itf.vendor_ringtone_stop = vendor_issc_ringtone_stop;
	s_vendor_itf.vendor_ringtone_close = vendor_issc_ringtone_close;
	s_vendor_itf.vendor_on_sco_established = vendor_issc_on_sco_established;
	s_vendor_itf.vendor_on_sco_released = vendor_issc_on_sco_released;
	s_vendor_itf.vendor_on_codec_configured = vendor_issc_on_codec_configured;

	s_vendor_itf.vendor_on_stream_open = vendor_issc_on_stream_open;
	s_vendor_itf.vendor_on_stream_start = vendor_issc_on_stream_start;
	s_vendor_itf.vendor_on_stream_suspend = vendor_issc_on_stream_suspend;
	s_vendor_itf.vendor_on_stream_done = vendor_issc_on_stream_done;
	s_vendor_itf.vendor_on_stream_configured = vendor_issc_on_stream_configured;

	s_vendor_itf.vendor_ringtone_start = vendor_issc_ringtone_start;
	s_vendor_itf.vendor_ringtone_stop = vendor_issc_ringtone_stop;
	s_vendor_itf.vendor_ringtone_close = vendor_issc_ringtone_close;

	s_vendor_itf.vendor_set_mic_gain = vendor_issc_set_mic_gain;
	s_vendor_itf.vendor_get_mic_gain = vendor_issc_get_mic_gain;
	s_vendor_itf.vendor_mic_control = vendor_issc_mic_control;
	s_vendor_itf.vendor_set_local_volume = vendor_issc_set_local_volume;
	s_vendor_itf.vendor_get_local_volume = vendor_issc_get_local_volume;
	s_vendor_itf.vendor_update_audio_param = vendor_issc_update_audio_param;

	return &s_vendor_itf;
}

void vendor_interface_destroy()
{
	vendor_issc_done();
}

static void vendor_issc_init(vendor_interface_callback callback)
{
	s_event_cbk = callback;
}

static void vendor_issc_done()
{
	if (s_dev_info != NULL) {
		FREE(s_dev_info);
		s_dev_info = NULL;
	}
}

static void vendor_issc_pre_reset()
{
	app_comm_Set8008PO(BT_CHIP_ISSC_SPK_CTL_PIO, 1);
}

static void vendor_issc_post_reset()
{
	app_comm_Set8008PO(BT_CHIP_ISSC_SPK_CTL_PIO, 0);	//Set speaker open
	app_comm_get_local_version();
}

static void vendor_issc_cmd_cbk(void *context, UINT8 result, void *cfm_par)
{
	unsigned short flag = (unsigned short) context;
	UINT8 param = HIBYTE(flag);

	LOG("vendor_cmd_cbk(flag=%x,OCF=%x)", param, LOBYTE(flag));

	if (param == FLAG_ISSC_AUDIO_PARAM) {
		if (LOBYTE(flag) == I8008_VENDOR_CMD_READ_EEPROM) {
			struct HCI_VendorEvStru *vendor_ev = (struct HCI_VendorEvStru *)(cfm_par+1);
			UINT8 low = vendor_ev->vendordata[4];
			UINT8 high = vendor_ev->vendordata[5];
			UINT8 value = vendor_ev->vendordata[7];

			if (result == HCI_STATUS_OK) {
				app_comm_set_audio_cbk(low, high, value);
			}
		}
		return;
	}

	switch(LOBYTE(flag)) {
	case I8008_VENDOR_CMD_WRITE_EEPROM:
		break;
	case I8008_VENDOR_CMD_READ_EEPROM:
		if (result == HCI_STATUS_OK) {
			struct HCI_VendorEvStru *vendor_ev = (struct HCI_VendorEvStru *)(cfm_par+1);
			UINT8 low = vendor_ev->vendordata[4];
			UINT8 high = vendor_ev->vendordata[5];
			UINT8 value = vendor_ev->vendordata[7];

			if ((vendor_ev->vendordata_plen == 0x08) && is_mic_gain_address(low, high)) {
				vendor_issc_event_indcate(VENDOR_EV_MIC_GAIN, (void *) value);
				return;
			} else if (is_eeprom_address(low, high)) {
				vendor_issc_get_local_firmware_version(low, high, value);
				return;
			}

			vendor_issc_event_indcate(VENDOR_EV_POST_RESET_HARDWARE_FINISHED, (void *) TRUE);
		} else if (result == HCI_EVO_COMMAND_ERROR) {
			vendor_issc_event_indcate(VENDOR_EV_POST_RESET_HARDWARE_FINISHED, (void *) FALSE);
		} else if (result == HCI_EVO_COMMAND_COMPLETE) {
			// TODO when HCI_EVO_COMMAND_COMPLETE occurs?
			struct HCI_VendorEvStru *vendor_ev = (struct HCI_VendorEvStru *) cfm_par;

			if ((vendor_ev->vendordata_plen == 0x05) && is_mic_gain_address(vendor_ev->vendordata[1], vendor_ev->vendordata[2])) {
				vendor_issc_event_indcate(VENDOR_EV_MIC_GAIN, (void *) (vendor_ev->vendordata[4]));
			} else {
#ifdef CONFIG_SAVE_INFO_TO_MODULE
				if (param == 0) {
					return;//Sent by Test Command
				} else if (param == 1) {
					int len = sizeof(struct DevInfoGotEvStru)+vendor_ev->vendordata_plen;
					s_dev_info = (struct DevInfoGotEvStru *) NEW(len);
					memcpy(s_dev_info, vendor_ev, len);
				}
#endif
				vendor_issc_event_indcate(VENDOR_EV_POST_RESET_HARDWARE_FINISHED, (void *) FALSE);
			}
		}
		break;
#ifdef CONFIG_COMPLEX_RINGTONE
	case I8008_VENDOR_CMD_TONE_GENERATION:
		if(param == 0x01) {
			LOG("Start Ring\r\n");
			FsmEvent(g_ring_fsminst,EV_RING_STARTED,0);
		} else {
			LOG("Stop Ring\r\n");
			FsmEvent(g_ring_fsminst,EV_RING_STOPED,0);
		}
		break;
	case I8008_VENDOR_CMD_AVDTP_LINK_INFO:
		LOG("Enable or Disable Streaming Process OK\r\n");
		break;
	case I8008_VENDOR_CMD_PLAY_RING:
		switch(param) {
		case 0:
			LOG("Play Ring First...\r\n");
			FsmEvent(g_ring_fsminst,EV_RING_PLAY,0);
			break;
		case 1:
			LOG("Play Ring Continue...\r\n");
			FsmEvent(g_ring_fsminst,EV_RING_PLAY,0);
			break;
		case 2:
			LOG("Play Ring End\r\n");
			FsmEvent(g_ring_fsminst,EV_RING_END,0);
			break;
		}
		break;
#endif
	default:
		break;
	}
}

static void vendor_issc_event_ind(void *param, UINT32 size)
{
}

static void vendor_issc_on_gap_started()
{
}

static char* vendor_issc_get_firmware_version()
{
	return s_firmware_version;
}

static UINT8* vendor_issc_get_device_info()
{
	return s_dev_info;
}

static void vendor_issc_ringtone_start(UINT8 count)
{
	app_comm_play_ring(1, 32);
}

static void vendor_issc_ringtone_stop()
{
	app_comm_play_ring(0, 0);
}

static void vendor_issc_ringtone_close()
{
	app_comm_play_ring(0, 0);
}

static void vendor_issc_on_sco_established(UINT16 handle)
{
}

static void vendor_issc_on_sco_released()
{
}

static void vendor_issc_on_codec_configured(UINT8 select_codec)
{
}


static void vendor_issc_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle)
{
	app_comm_enable_stream_process(1, acl_handle, l2cap_cid);
}

static void vendor_issc_on_stream_start()
{
}

static void vendor_issc_on_stream_suspend()
{
}

static void vendor_issc_on_stream_done()
{
	app_comm_enable_stream_process(0,0,0);
}

static void vendor_issc_on_stream_configured(UINT8 codec, UINT16 sample)
{
}

#define BT_E2_FILE_PATH "/data/misc/brlink/DSP_parameter.txt"
#define MAX_LINE 64
unsigned int wr_e2[MAX_LINE*16][2];
static int isWriting = 0;

//01 29 fc 03 00 18 08
//29:ogf
//03:paramlen
//00 18:addr
//08:len
int app_comm_read_eeprom(unsigned char nFlag,unsigned short Addr,int len)
{
	unsigned char param_len = 3;
	unsigned char *param = NEW(param_len);
	param[0] = HIBYTE(Addr);
	param[1] = LOBYTE(Addr);
	param[2] = len;
	app_gap_vendor_command(nFlag, I8008_VENDOR_CMD_READ_EEPROM, param, param_len);
	FREE(param);

	return 1;
}

//01 27 fc 07 00 10 04 00 11 22 33
//27:ogf
//07:param len
//00 10:addr
//04:len
//00 11 22 33:data
int app_comm_write_eeprom(UINT8 nFlag,UINT16 Addr,int len,UCHAR *data)
{
	unsigned char param_len = 3+len;
	unsigned char *param = NEW(param_len);
	param[0] = HIBYTE(Addr);
	param[1] = LOBYTE(Addr);
	param[2] = len;
	memcpy(param+3, data, len);
	app_gap_vendor_command(nFlag, I8008_VENDOR_CMD_WRITE_EEPROM, param, param_len);
	FREE(param);

	return 1;
}

static void vendor_issc_event_indcate(UINT32 event, void *param)
{
	if (s_event_cbk != NULL) {
		s_event_cbk(event, param);
	}
}

static BOOL is_mic_gain_address(UINT8 low, UINT8 high)
{
	BOOL ret = (low == HIBYTE(I8008_MIC_GAIN_ADDR));
	ret &= (high == LOBYTE(I8008_MIC_GAIN_ADDR));

	return ret;
}

static BOOL is_eeprom_address(UINT8 low, UINT8 high)
{
	BOOL ret = FALSE;

	ret |= ((low == HIBYTE(I8008_LOCAL_VERSION_ADDR1)) &&
	        (high == LOBYTE(I8008_LOCAL_VERSION_ADDR1)));

	ret |= ((low == HIBYTE(I8008_LOCAL_VERSION_ADDR2)) &&
	        (high == LOBYTE(I8008_LOCAL_VERSION_ADDR2)));

	return ret;
}

static void vendor_issc_get_local_firmware_version(UINT8 h_addr, UINT8 l_addr, UINT8 value)
{
	UINT16 addr_type = h_addr<<8|l_addr;

	switch (addr_type) {
	case I8008_LOCAL_VERSION_ADDR1:
		sprintf(s_firmware_version, "%02X", value);
		app_comm_read_eeprom(1, I8008_LOCAL_VERSION_ADDR2, 1);
		break;
	case I8008_LOCAL_VERSION_ADDR2:
		sprintf(s_firmware_version+2, "%02X", value);
		app_comm_read_eeprom(1, EEPROM_START_ADDR, DEV_INFO_LEN);
		break;
	default:
		LOG("local_version addr is error!");
		break;
	}
}

static void app_comm_SetPOLow(unsigned char nIndex)
{
	unsigned char nvalue = 1;
	unsigned char CmdLow[] = { 0xFF,0x00,0x00,0x00,
	                           0x00,0x00,0x00,0x00,
	                           0xFF,0x00,0x00,0x00
	                         };

	CmdLow[0] = (CmdLow[0])&(nvalue<<nIndex);
	CmdLow[8] = (CmdLow[8])&(nvalue<<nIndex);

	app_gap_vendor_command(0, 0x31, CmdLow, sizeof(CmdLow));
}

static void app_comm_SetPOHigh(unsigned char nIndex)
{
	unsigned char nvalue = 1;
	unsigned char CmdHigh[] = { 0xFF,0x00,0x00,0x00,
	                            0xFF,0x00,0x00,0x00,
	                            0xFF,0x00,0x00,0x00
	                          };

	CmdHigh[0] = (CmdHigh[0])&(nvalue<<nIndex);
	CmdHigh[4] = (CmdHigh[4])&(nvalue<<nIndex);
	CmdHigh[8] = (CmdHigh[8])&(nvalue<<nIndex);

	app_gap_vendor_command(0, 0x31, CmdHigh, sizeof(CmdHigh));
}

static void app_comm_Set8008PO(unsigned char nPOIndex,unsigned char bHigh)
{
	if (bHigh) {
		app_comm_SetPOHigh(nPOIndex);
	} else {
		app_comm_SetPOLow(nPOIndex);
	}
}

static void app_comm_get_local_version(void)
{
	UINT8 param[3];
	UINT16 version_addr = I8008_LOCAL_VERSION_ADDR1;
	param[0] = HIBYTE(version_addr);
	param[1] = LOBYTE(version_addr);
	param[2] = 0x01;

	app_gap_vendor_command(0, I8008_VENDOR_CMD_READ_EEPROM, param, sizeof(param));
}

static void app_comm_enable_stream_process(BOOL bEnable,UINT16 acl_handle,UINT16 channel_cid)
{
	static unsigned short s_acl_handle = 0,s_channel_cid = 0;
	unsigned char param_len = 5;
	unsigned char *param = NEW(param_len);
	if(bEnable) {
		param[4] = 1;
		s_acl_handle = acl_handle;
		s_channel_cid = channel_cid;
	} else {
		param[4] = 0;
	}
	param[0] = LOBYTE(s_acl_handle);
	param[1] = HIBYTE(s_acl_handle);
	param[2] = LOBYTE(s_channel_cid);
	param[3] = HIBYTE(s_channel_cid);
	if(bEnable) {
		unsigned char cmd[] = {0x01, I8008_VENDOR_CMD_AVDTP_LINK_INFO, 0xFC, 0x05, 0x01, 0x00, 0x48, 0x00, 0x01};
		memcpy(cmd+4,param,5);
		LOG("app_comm_enable_stream_process %d:hdl=%d,cid=%d\r\n",bEnable,acl_handle,channel_cid);
		hlp_tl_send_data(0,cmd,sizeof(cmd));
	} else {
		LOG("app_comm_enable_stream_process %d\r\n",bEnable);
		app_gap_vendor_command(0, I8008_VENDOR_CMD_AVDTP_LINK_INFO, param, param_len);
	}
	FREE(param);
}

static void get_audio_param(unsigned int Addr, int len)
{
	unsigned char param_len = 3;
	unsigned char *param = NEW(param_len);
	memset(param, 0, param_len);

	param[0] = HIBYTE(Addr);
	param[1] = LOBYTE(Addr);
	param[2] = len;
	app_gap_vendor_command(FLAG_ISSC_AUDIO_PARAM, I8008_VENDOR_CMD_READ_EEPROM, param, param_len);

	FREE(param);
}

static void set_audio_param(unsigned int Addr, int len, UCHAR *data)
{
	unsigned char param_len = 3+len;
	unsigned char *param = NEW(param_len);
	memset(param, 0, param_len);

	param[0] = HIBYTE(Addr);
	param[1] = LOBYTE(Addr);
	param[2] = len;
	memcpy(param+3, data, len);

	app_gap_vendor_command(FLAG_ISSC_AUDIO_PARAM, I8008_VENDOR_CMD_WRITE_EEPROM, param, param_len);

	FREE(param);
}


static void l_to_c(unsigned char *h_v, unsigned *l_v, unsigned short l)
{
	if (l < 0x10) {
		*h_v = '0';
		*l_v = l;
		if (*l_v>9) {
			*l_v = *l_v-10+'A';
		} else {
			*l_v=*l_v+'0';
		}
	} else {
		*h_v = l/0x10;
		*l_v = l%0x10;
		if (*h_v > 9) {
			*h_v = *h_v-10+'A';
		} else {
			*h_v = *h_v+'0';
		}
		if (*l_v > 9) {
			*l_v = *l_v-10+'A';
		} else {
			*l_v = *l_v+'0';
		}
	}
}

static void app_comm_set_audio_cbk(unsigned char h_addr, unsigned char l_addr, unsigned char data)
{
	unsigned short addr = (h_addr<<8) | l_addr;
	unsigned char h_v = 0;
	unsigned char l_v = 0;

	l_to_c(&h_v, &l_v, data);

	if ((wr_e2[addr][0]==h_v) && (wr_e2[addr][1]==l_v)) {
		if (addr >= 0X03FF) { /* updata complete */
			isWriting = 0;
			vendor_issc_event_indcate(VENDOR_EV_AUDIO_PARAMETER_UPDATED, (void *) 0);
			LOG("Audio updata successed!");
		}
	} else {
		isWriting = 0;
		vendor_issc_event_indcate(VENDOR_EV_AUDIO_PARAMETER_UPDATED, (void *) 1);
		LOG("Audio updata failed!");
	}
}

void app_comm_play_ring(BOOL bStart, UINT8 id)
{
	unsigned char param_len = 2;
	unsigned char *param = NEW(param_len);
	unsigned char ocf = I8008_VENDOR_CMD_TONE_GENERATION;
	LOG("RingStart\r\n");
	param[0] = bStart;
	param[1] = id;
	app_gap_vendor_command(0,ocf,param,param_len);
	FREE(param);
}

static void vendor_issc_set_mic_gain(UINT8 volume)
{
	UINT8 param[4];
	UINT8 micGain = (volume >= 25) ? 25 : volume;

	param[0] = HIBYTE(I8008_MIC_GAIN_ADDR);
	param[1] = LOBYTE(I8008_MIC_GAIN_ADDR);
	param[2] = 0x01;
	param[3] = micGain&0xff;

	app_gap_vendor_command(0, I8008_VENDOR_CMD_WRITE_EEPROM, param, sizeof(param));
}

static void vendor_issc_get_mic_gain()
{
	UINT8 param[3];

	param[0] = HIBYTE(I8008_MIC_GAIN_ADDR);
	param[1] = LOBYTE(I8008_MIC_GAIN_ADDR);
	param[2] = 0x01;

	app_gap_vendor_command(0, I8008_VENDOR_CMD_READ_EEPROM, param, sizeof(param));
}

static void vendor_issc_mic_control(UINT8 flag)
{
	UINT8 param[3] = {0x00, 0x01, 0x00};
	UINT8 param_len = sizeof(param);

	if (flag == 1) {
		param[2] = 0x1C;
	} else {
		param[2] = 0x1B;
	}

	app_gap_vendor_command(0, 0x65, param, param_len);
}

static void vendor_issc_set_local_volume(UINT8 volume)
{
}

static int vendor_issc_get_local_volume()
{
	return 0;
}

static int vendor_issc_update_audio_param(void)
{
	int  fd = -1;
	char rd_buf[5120];
	char *line[MAX_LINE];
	char line_num[10];
	char byte[3];

	char *p;
	char *q;

	int i = 0;
	int j = 0;
	unsigned char mid_v = 0;
	int rd_num = 0;
	char version_param[2]; //usr to set fireware version param.
	unsigned char version_data[2];
	isWriting = 1;

	fd=open(BT_E2_FILE_PATH, O_RDONLY);

	if (fd <= 0) {
		close(fd);
		return -1;
	}

	rd_num = read(fd, rd_buf, sizeof(rd_buf));
	p = rd_buf;

	memset(wr_e2, 0, sizeof(wr_e2));
	for (i= 0; i<MAX_LINE; i++) { //write audio param
		memset(line_num, 0, sizeof(line_num));
		snprintf(line_num, sizeof(line_num), "0%02X0    ", i);
		p = strstr(p, line_num);
		p = p+strlen(line_num);
		q = strtok(p, "\n");
		p = p+strlen(q)+1;
		while((*q != ';') && isWriting) {
			if ((*q=='*') || ((*q>='0')&&(*q<='9')) || ((*q>='A')&&(*q<='Z'))) {
				wr_e2[j][0]=*q;
				wr_e2[j][1]=*(q+1);
				if ((j>=1023) && (wr_e2[j][0]=='*') && (wr_e2[j][1]=='*')) {
					isWriting = 0;
					vendor_issc_event_indcate(VENDOR_EV_AUDIO_PARAMETER_UPDATED, (void *) 0);
					LOG("Audio updata successed!");
					break;
				}
				if (((*q>='0')&&(*q<='9')) || ((*q>='A')&&(*q<='Z'))) {
					memset(byte, 0, sizeof(byte));
					snprintf(byte, sizeof(byte), "%c%c", wr_e2[j][0], wr_e2[j][1]);
					mid_v = (unsigned char)(strtol(byte, NULL, 16));
					set_audio_param(j, 1/* len */, &mid_v);
					usleep(200000); //Waiting for controller write
					get_audio_param(j, 1/* len */);
					usleep(200000);
				}
				q=q+2;
				j++;
				continue;
			}
			q=q+1;
		}
	}

	//write firewire version, firewire version addr is 0x0FF6, 0x0FF7.
	memset(line_num, 0, sizeof(line_num));
	memcpy(line_num, "0FF0    ", 8);
	if ((p=strstr(rd_buf, line_num))!=NULL) {
	} else {
		while(1) {
			rd_num = read(fd, rd_buf, sizeof(rd_buf));
			p = rd_buf;
			p = strstr(p, line_num);
			if (p==NULL) {
				if(rd_num < sizeof(rd_buf)) {
					return 0;
				} else {
					continue;
				}
			} else {
				break;
			}
		}
	}
	q = strtok(p, "\n");
	p=p+26; //addr: 0x0FF6
	memcpy(version_param, p, sizeof(version_param));
	version_data[0] = (unsigned int)(strtol(version_param, NULL, 16));
	p=p+3; //addr: 0x0FF7
	memcpy(version_param, p, sizeof(version_param));
	version_data[1] = (unsigned int)(strtol(version_param, NULL, 16));
	LOG("Version value: %04x", version_data);
	app_comm_write_eeprom(FLAG_ISSC_FIREWARE_VERSION_PARAM, 0xFF6, 2, version_data);

	return 0;
}

