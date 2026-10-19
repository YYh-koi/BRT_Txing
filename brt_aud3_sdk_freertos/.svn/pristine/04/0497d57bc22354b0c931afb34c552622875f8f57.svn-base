#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "porting.h"
#include "tl.h"
#include "bt_debug.h"
#include "userial_vendor.h"
#include "btdatatype.h"
#include "bt_hci_lib.h"
#include "bt_vendor_lib.h"
#include "bt_vendor_barrot.h"
#include "bt_vendor_barrot_fw.h"

#define TAG "vendor_barrot"

#define VND_DBG(fmt, ...) bse_print_va(TAG, fmt, ## __VA_ARGS__)
#define BTHWDBG VND_DBG
#define vs_read(...) read(__VA_ARGS__)

#define CONFIG_TL_H5

int CHIP_BARROT_H4_DONE = 0x02;

UCHAR s_rx_buffer[1024];
static UCHAR s_patch_buffer[HCI_CMD_MAX_LEN] = {0};

static uint32_t g_chip_version;
static uint32_t g_firmware_version;

// BARROT_BR8051A01_HCI_0000_202108021441_B
//static char fw_patchfile_path[256] = "/etc/backup/"; //read from brt_debug.h
static char fw_patchfile_path[256] = {0};
static char fw_patchfile_name[128] = {0};

//#define DBG_DUMP_DATA
//#define BTSNOOP_CAPTURE_PATCH

static void dump(UCHAR *out, int len);
static int do_write(unsigned char *buf, int len);
static int read_event(UCHAR *buffer);
static int read_event_ex(UCHAR *buffer, int *count);

static int read_chip_info();
static int download_patch_ex();
static uint8_t hw_config_reset(uint8_t mode);
static uint8_t hw_config_set_bdaddr();
static uint8_t hw_config_hci_reset();
static uint8_t hw_config_set_baud_rate(uint32_t baud);
static void set_uart_baudrate(uint32_t baud);

static fw_config_callback s_fwcfg_cb;
static void hlp_start_h5_init();

#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define STREAM_TO_UINT32(u32, p) {u32 = ((uint32_t)(*(p)) + (((uint32_t)(*((p) + 1))) << 8) + (((uint32_t)(*((p) + 2))) << 16) \
                                                    + (((uint32_t)(*((p) + 3))) << 24)); (p) += 4;}

#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32)>> 24);}

#define UART_TARGET_BAUD_RATE           921600

#undef CONFIG_RANDOM_ADDRESS

void bt_vendor_init()
{
	//TODO
}

void bt_vendor_done()
{
	//TODO
}

void barrot_firmware_config(void *param, fw_config_callback cb)
{
	int success = 0;
	s_fwcfg_cb = cb;

	//FIXME loadPatch reset is for what??

	/*1. read chip info*/
	if (read_chip_info() < 0) {
		VND_DBG("%s, read_chip_info failed!", __func__);
		goto err;
	}

	/*2. get patch file*/

	/*3. downlaod patch file*/
	if (download_patch_ex() < 0) {
		VND_DBG("%s, download firmware failed!", __func__);
		goto err;
	}

	/*4. patch reset*/
	if (hw_config_reset(BR_VENDORCMDPARA_RESET_TYPE_loadPatchReset) < 0) {
		VND_DBG("%s, loadPatchReset fail!", __func__);
		goto err;
	}

	/*4-1. read chip info again*/
	if (read_chip_info() < 0) {
		VND_DBG("%s, read_chip_info failed!", __func__);
		goto err;
	}

	/*5. set address*/
	if (hw_config_set_bdaddr() < 0) {
		VND_DBG("%s, set_address failed!", __func__);
		goto err;
	}

	/*6. hci reset*/
//	if (hw_config_hci_reset() < 0) {
//		VND_DBG("%s, hci_reset failed!", __func__);
//		goto err;
//	}

	/*7. set baudrate*/
	if (hw_config_set_baud_rate(UART_TARGET_BAUD_RATE) < 0) {
		VND_DBG("%s, set_baud_rate failed!", __func__);
		goto err;
	}

	/*8. software reset*/
	if (hw_config_reset(BR_VENDORCMDPARA_RESET_TYPE_softReset) < 0) {
		VND_DBG("%s, software reset failed!", __func__);
		goto err;
	}

	/*9. set local baudrate. This must after reset*/
	set_uart_baudrate(UART_TARGET_BAUD_RATE);

	//DO H5 init
#ifdef CONFIG_TL_H5
	cb(CHIP_BARROT_H4_DONE);//barrot start read proc
	hlp_start_h5_init();
	return;
#endif

	success = 1;
	VND_DBG("%s, successfully!", __func__);

err:
	VND_DBG("success = %d", success);
	if (cb != NULL) {
		cb(success ? 0 : 1);
	}
}

static void dump(UCHAR *out, int len)
{
#ifdef DBG_DUMP_DATA
	debug_print_data(TAG, out, len);
#endif
}

static int do_write(unsigned char *buf, int len)
{
	int ret = 0;
	int write_offset = 0;
	int write_len = len;
#ifdef BTSNOOP_CAPTURE_PATCH
	btsnoop_capture(buf, len, 0);
#endif
	dump(buf, len);
	do {
		ret = write_port(buf + write_offset, write_len);
		if (ret < 0) {
			VND_DBG("%s, write failed ret = %d", __func__, ret);
			return -1;
		} else if (ret == 0) {
			VND_DBG("%s, write failed with ret 0", __func__);
			return 0;
		} else {
			if (ret < write_len) {
				VND_DBG("%s, Write pending,do write ret = %d", __func__, ret);
				write_len = write_len - ret;
				write_offset = ret;
			} else {
				//VND_DBG("Write successful");
				break;
			}
		}
	} while (1);
	return len;
}

static int read_event(UCHAR *buffer)
{
	int i = 0;
	int len = 3;
	int count;
	int ret = -1;

	while ((count = read_port(&buffer[i], len)) < len) {
		if (count <= 0) {
			goto error;
		}
		i += count;
		len -= count;
	}

	i += count;
	len = buffer[2];

	while ((count = read_port(&buffer[i], len)) < len) {
		if (count <= 0) {
			goto error;
		}
		i += count;
		len -= count;
	}

	count += i;
#ifdef BTSNOOP_CAPTURE_PATCH
	btsnoop_capture(buffer, count, 1);
#endif
	dump(buffer, count);
	ret = 0;
error:
	return ret;
}
/*
static int read_event_ex(UCHAR *buffer, int *read_cnt)
{
	int i = 0;
	int len = 3;
	int count;
	int ret = -1;

	while ((count = read_port(&buffer[i], len)) < len) {
		if (count <= 0) {
			goto error;
		}
		i += count;
		len -= count;
	}

	i += count;
	len = buffer[2];

	while ((count = read_port(&buffer[i], len)) < len) {
		if (count <= 0) {
			goto error;
		}
		i += count;
		len -= count;
	}

	count += i;
#ifdef BTSNOOP_CAPTURE_PATCH
	btsnoop_capture(buffer, count, 1);
#endif
	dump(buffer, count);
	ret = 0;
error:
	*read_cnt = count;
	return ret;
}
*/

#define HCI_EVENT_MAX_LEN			 258
#define HCI_EVENT_MXA_RETRY_TIME   200
static int read_event_ex(UCHAR *buf, int *read_cnt)
{	
	int remain, r;
	int count = 0;
	int  retry_time = 0;

	while (count < 3) {
		r = read_port(buf, 3);
		if (r <= 0) {
			if(retry_time < HCI_EVENT_MXA_RETRY_TIME) {
				usleep(5 * 1000);
				retry_time++;
				continue;
			} else {
				BTHWDBG("%s, read head byte err = %d", __func__, r);
				return -1;
			}
		}
		count += r;
	}
	
	/* Now we read the parameters. */
	if (buf[2] < (HCI_EVENT_MAX_LEN - 3)) {
		remain = buf[2];
	} else {
		remain = HCI_EVENT_MAX_LEN - 3;
	}
	retry_time = 0;
	while ((count - 3) < remain) {
		r = read_port(buf + count, remain - (count - 3));
		if (r <= 0 ) {
			if(retry_time < HCI_EVENT_MXA_RETRY_TIME) {
				usleep(5 * 1000);
				retry_time++;
				continue;
			} else {
				BTHWDBG("%s, read date err = %d", __func__, r);
				return -1;
			}
		}
		count += r;
	}

	dump(buf, count);
	*read_cnt = count;
	return count;
}

static uint8_t check_chip_info(uint8_t *p_chip_info)
{
	uint8_t ret = 1;
	uint32_t chip_version, FW_version;
	STREAM_TO_UINT32(chip_version, p_chip_info);
	STREAM_TO_UINT32(FW_version, p_chip_info);

	VND_DBG("%s, chip_v = 0x%x, fw_v = 0x%x", __func__, chip_version, FW_version);

	if (chip_version == BARROT_CHIP_ID_8051A01) {
		g_chip_version     = chip_version;
		g_firmware_version = FW_version;
    } else if (chip_version == BARROT_CHIP_ID_8051A02) {
		g_chip_version     = chip_version;
		g_firmware_version = FW_version;
	} else {
		ret = 0;
		BTHWDBG("%s:patch file not match the chip", __func__);
	}

	return ret;
}

static UCHAR read_8051_chip_info[] = {0x01, 0x80, 0xFC, 0x02, 0x01, 0x00};
static int read_chip_info()
{
	do_write(read_8051_chip_info, sizeof(read_8051_chip_info));

	memset(s_rx_buffer, 0, 1024);//for get len
	int len = 0;
	int ret = read_event_ex(s_rx_buffer, &len);
	VND_DBG("%s, ret = %d, len = %d", __func__, ret, len);

	if (len != 31 && len != 37) {
		VND_DBG("Error len Not Info");
		if (len == 7 && (s_rx_buffer[0] = 0x04) && (s_rx_buffer[1] == 0x0F)) {
			VND_DBG("Pending Command. Waiting for info");
			int ret = read_event_ex(s_rx_buffer, &len);
			VND_DBG("%s, retry ret = %d, len = %d", __func__, ret, len);
			if (len != 31 && len != 37) {
				return -1;
			}
		} else {
			return -1;
		}
	}

	//check chip info
	uint8_t *p_info = s_rx_buffer + HCI_EVT_TAG + HCI_EVT_CMD_CMPL_LOCAL_CHIP_INFO;
	if (!check_chip_info(p_info)) {
		VND_DBG("%s, check_chip_info failed!", __func__);
		return -1;
	}

	return 0;
}

/*******************************************************************************
**
** Function		 hw_strncmp
**
** Description	  Used to compare two strings in caseless
**
** Returns		  0: match, otherwise: not match
**
*******************************************************************************/
static int hw_strncmp(const char *p_str1, const char *p_str2, const int len)
{
	int i;

	if (!p_str1 || !p_str2)
		return (1);

	for (i = 0; i < len; i++) {
		if (toupper(p_str1[i]) != toupper(p_str2[i]))
			return (i + 1);
	}

	return 0;
}


static int download_patch_ex()
{
	memset(s_patch_buffer, 0, HCI_CMD_MAX_LEN);
	uint8_t *p = s_patch_buffer;
	uint16_t offset = 0;

	//below is constant
	*p = 0x01;//hci cmd tag
	p++;
	UINT16_TO_STREAM(p, HCI_BRT_VS_CMD);//+2
	
	//for payload length
	uint8_t *size_pos = s_patch_buffer + 3;
	p++;
	
	//below is constant
	UINT16_TO_STREAM(p, BR_VENDORCMD_SUBID_WRITE_PATCH);//+2
	uint8_t *p_offset = s_patch_buffer + 6;
	p += 2;

	//loop read patch_file
	unsigned int wr_total_size = 0;
	unsigned int req_size = PATCH_DATA_ONE_PACKAGE_SIZE;
	while (wr_total_size < brt8051_fw_len) {

		//FIXME why UINT16_TO_STREAM will fail ???
		s_patch_buffer[6] = (uint8_t)offset;
		s_patch_buffer[7] = (uint8_t)(offset >> 8);

		req_size = (brt8051_fw_len - wr_total_size) > PATCH_DATA_ONE_PACKAGE_SIZE ? PATCH_DATA_ONE_PACKAGE_SIZE : brt8051_fw_len - wr_total_size; 
        memcpy(s_patch_buffer + 8, brt8051_fw + wr_total_size, req_size);

		*size_pos = 4 + req_size;
		offset += req_size;

		do_write(s_patch_buffer, req_size + 8);
		int len =0;
		if (read_event_ex(s_rx_buffer, &len) < 0) {
			VND_DBG("%s. read_event fail", __func__);
			return -1;
		}
		wr_total_size += req_size; 
		VND_DBG("%s. brt8051_fw_len:%d, wr_total_size:%d, req_size:%d", __func__, brt8051_fw_len, wr_total_size, req_size);
	}
	VND_DBG("Firmware downloaded!");
	return 0;
}

static UCHAR _8051_reset_cmd[7] = {0x01, 0x80, 0xFC, 0x03, 0x00, 0x00, 0x00};
static uint8_t hw_config_reset(uint8_t mode)
{
	int len =0;
	VND_DBG("hw_config_reset(%d).", mode);
	_8051_reset_cmd[6] = mode;
	do_write(_8051_reset_cmd, sizeof(_8051_reset_cmd));
	return read_event_ex(s_rx_buffer, &len);
}

static uint8_t vnd_local_bd_addr[6] = {0x00, 0x15, 0x83, 0xFF, 0xCC, 0x33};
static UCHAR _8051_set_address[12] = {0x01, 0x80, 0xfc, 0x08, 0x05, 0x00, 0x66, 0xCC, 0xFF, 0x14, 0x68, 0xFB};
#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 512
#endif
#define DEFAULT_ADDR_FILE "BDADDR.INI"

//format is 00:15:83:AA:BB:CC . Big endian, and is string format
static int is_bdaddr_available(char *bdaddr)
{
	if (bdaddr[0] == 0) {
		return 0;
	}

	if (bdaddr[2] != ':' || bdaddr[5] != ':' || bdaddr[8] != ':' || bdaddr[11] != ':' || bdaddr[14] != ':') {
		VND_DBG("inavailable address");
		return 0;
	}

	return 1;
}

//if address is not available, can not call this. Format 00:15:83:AA:BB:CC
static int parse_bdaddr(char *optarg)
{
	int bd_addr[6];
	int i;

	sscanf(optarg, "%02X:%02X:%02X:%02X:%02X:%02X",
	       &bd_addr[5], &bd_addr[4], &bd_addr[3],
	       &bd_addr[2], &bd_addr[1], &bd_addr[0]);
	for (i = 0; i < 6; i++) {
		_8051_set_address[6 + i] = bd_addr[i];
	}

	return 1;
}

static uint32_t crc32(const char* s){

    uint32_t crc = 0xffffffff;
    size_t i = 0;
    while (s[i] != '\0')
    {
        uint8_t byte = s[i];
        crc = crc ^ byte;
		uint8_t j = 0;
        for (j = 8; j > 0; --j)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }

        i++;
    }
    return crc ^ 0xffffffff;
}

#undef SYSTEM_INFO_PATH 
#ifdef SYSTEM_INFO_PATH
//if len is lower than 12, can use it for all address
#undef SYSTEM_INFO_PATH_LEN 17
static int hlp_assign_address_customnized()
{
	int ret = 1;//deafault as success

#ifndef SYSTEM_INFO_PATH
	return 0;
#else
	int fd, sz;
	uint8_t info[128] = {0};

	fd = open(SYSTEM_INFO_PATH, O_RDONLY);
	if (fd > 0) {
		int sz = read(fd, info, 128);
		VND_DBG("%s, read info = (%d)%s", __func__, sz, info);
		if (sz < 0
#ifdef SYSTEM_INFO_PATH_LEN
			 || sz != SYSTEM_INFO_PATH_LEN
#endif
			 ) {
			VND_DBG("%s, read info error", __func__);
			close(fd);
			return 0;
		}
		
		if (info[sz - 1] == '\r' || info[sz - 1] == '\n') {
			VND_DBG("reset info tail");
			info[sz - 1] = 0;
		}

//#if (SYSTEM_INFO_PATH_LEN < 12) TODO

		char bdaddr[18] = {0};
		uint32_t crc32_result = crc32(info);
		uint8_t bd[6] = {0};
		bd[0] = (uint8_t)(crc32_result & 0xFF);
		bd[1] = (uint8_t)(crc32_result >> 8 & 0xFF);
		bd[2] = (uint8_t)(crc32_result >> 16 & 0xFF);
		bd[3] = (uint8_t)(crc32_result >> 24 & 0xFF);
		bd[4] = 0x7F;
		bd[5] = 0x04;
		snprintf(bdaddr, 18, "%02X:%02X:%02X:%02X:%02X:%02X", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);

		VND_DBG("%s, %08X, read address = %s", __func__, crc32_result, bdaddr);
		if (is_bdaddr_available(bdaddr)) {
			parse_bdaddr(bdaddr);
		} else {
			close(fd);
			return 0;
		}
	} else {
		VND_DBG("%s, open file %s error(%s)", __func__, SYSTEM_INFO_PATH, strerror(errno));
		return 0;
	}
	close(fd);
#endif
	return ret;
}
#endif

#ifdef CONFIG_RANDOM_ADDRESS
static int hlp_assign_random_address()
{
	int ret = 1;//deafault as success

	int fd, sz;
	char path[PROPERTY_VALUE_MAX];
	char bdaddr[18] = {0};
	const char *config_dir = porting_get_configure_path(CONFIGURATION_TYPE_READONLY);
	memset(path, 0, PROPERTY_VALUE_MAX);
	snprintf(path, PROPERTY_VALUE_MAX, "%s/%s", config_dir, DEFAULT_ADDR_FILE);

	if (access(path, F_OK) == -1) {
		srand(time(NULL));
		uint8_t bd[6] = {0};
		bd[0] = rand() % 254 + 1;
		bd[1] = rand() % 254 + 1;
		bd[2] = rand() % 254 + 1;
		bd[3] = 0x0E;
		bd[4] = 0x7F;
		bd[5] = 0x04;
		snprintf(bdaddr, 18, "%02X:%02X:%02X:%02X:%02X:%02X", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);

		VND_DBG("%s, read address = %s", __func__, bdaddr);
		fd = open(path, O_RDWR | O_CREAT, 0777);
		if (fd > 0) {
			sz = write(fd, bdaddr, strlen(bdaddr));
			VND_DBG("%s, write ret = %d", __func__, sz);

			if (sz < 0 || sz != strlen(bdaddr)) {
				close(fd);
				return 0;
			}
		} else {
			VND_DBG("%s, Can not open %s, %s", __func__, path, strerror(errno));
			return;
		}
		close(fd);

		if (is_bdaddr_available(bdaddr)) {
			parse_bdaddr(bdaddr);
		} else {
			return 0;
		}
	} else {
		fd = open(path, O_RDONLY);
		if (fd < 0) {
			VND_DBG("%s ERROR (%s) %s", __func__, path, strerror(errno));
			return 0;
		} else {
			sz = read(fd, bdaddr, 17);
			VND_DBG("%s, sz = %d, read address = %s", __func__, sz, bdaddr);
			if (sz < 0 || sz != 17) {
				close(fd);
				return 0;
			}

			if (is_bdaddr_available(bdaddr)) {
				parse_bdaddr(bdaddr);
			} else {
				VND_DBG("%s, address not valid, remove the file(%s)", path);
				close(fd);
				remove(path);//TODO get random address Again?
				return 0;
			}
		}
		close(fd);
	}

	return ret;
}
#endif

static uint8_t hw_config_set_bdaddr()
{
	VND_DBG("%s", __func__);
	int address_assigned = 0;
#ifdef CONFIG_RANDOM_ADDRESS
	address_assigned = hlp_assign_random_address();
#else
#ifdef SYSTEM_INFO_PATH
	address_assigned = hlp_assign_address_customnized();
#endif
#endif
#ifdef SYSTEM_INFO_PATH
	if (address_assigned) {
		VND_DBG("address assigned");
	} else {//read address form TL.INI
		char path[PROPERTY_VALUE_MAX];
		char bdaddr[18];
		const char *config_dir = porting_get_configure_path(CONFIGURATION_TYPE_READONLY);
		memset(path, 0, PROPERTY_VALUE_MAX);
		snprintf(path, PROPERTY_VALUE_MAX, "%s/%s", config_dir, "TL.INI");

		int ret = get_private_profile_string("local", "bd_addr", " ", bdaddr, 18, path);
		VND_DBG("ret = %d, bd = %s", ret, bdaddr);

		if (ret > 0 && is_bdaddr_available(bdaddr)) {
			parse_bdaddr(bdaddr);
		}
	}
#endif
	do_write(_8051_set_address, sizeof(_8051_set_address));
	int len =0;
	return read_event_ex(s_rx_buffer, &len);
}

static UCHAR _hci_reset[] = {0x01, 0x03, 0x0C, 0x00};
static uint8_t hw_config_hci_reset()
{
	VND_DBG("%s", __func__);
	int len =0;
	do_write(_hci_reset, sizeof(_hci_reset));
	return read_event_ex(s_rx_buffer, &len);
}

static void set_uart_baudrate(uint32_t baud)
{
	userial_vendor_set_baud(baud);
}

static UCHAR _8051_set_baud_rate[24] = {0x01, 0x80, 0xfc, 0x14, 0x04, 0x00/*to fill 18*/};
static uint8_t hw_config_set_baud_rate(uint32_t baud)
{
	VND_DBG("%s, baud = %d", __func__, baud);
	uint8_t  para[18/*BR_VENDORCMD_PARA_LEN_setUart*/];
	int len =0;

	uint8_t  tl_mode     = BR_VENDORCMDPARA_TL_MODE_h4;
#ifdef CONFIG_TL_H5
	tl_mode     = BR_VENDORCMDPARA_TL_MODE_h5;
#endif
	uint8_t  word_length = BR_VENDORCMDPARA_UART_WORDLENGTH_8bits;
	uint8_t  stop_bits   = BR_VENDORCMDPARA_UART_STOPBITS_1bit;
	uint8_t  parity      = BR_VENDORCMDPARA_UART_PARITY_none;
	uint8_t  fc          = BR_VENDORCMDPARA_UART_FLOWCTRL_none;
	uint8_t  loopback    = BR_VENDORCMDPARA_UART_LOOPBACKMODE_none;
	uint16_t tx_buffer_size      = 0x800;
	uint16_t tx_buffer_threshold = 0x200;
	uint16_t Rx_buffer_size      = 0x800;
	uint16_t Rx_buffer_threshold = 0x200;

	uint8_t* p = &para[0];
	*p++  = tl_mode;
	UINT32_TO_STREAM(p, baud);
	*p++  = word_length;
	*p++  = stop_bits;
	*p++  = parity;
	*p++  = fc;
	*p++  = loopback;
	UINT16_TO_STREAM(p, tx_buffer_size);
	UINT16_TO_STREAM(p, tx_buffer_threshold);
	UINT16_TO_STREAM(p, Rx_buffer_size);
	UINT16_TO_STREAM(p, Rx_buffer_threshold);

	memcpy(_8051_set_baud_rate + 6, para, 18);

	do_write(_8051_set_baud_rate, sizeof(_8051_set_baud_rate));

	return read_event_ex(s_rx_buffer, &len);

}

#include "sysdep.h"
#define H5_NEW_(A,B,C) NEW(A)
#define H5_FREE_(A,B,C) FREE(A)

/* Message event ID passed from stack to vendor lib */
#define MSG_STACK_TO_HC_HCI_ACL        0x2100 /* eq. BT_EVT_TO_LM_HCI_ACL */
#define MSG_STACK_TO_HC_HCI_SCO        0x2200 /* eq. BT_EVT_TO_LM_HCI_SCO */
#define MSG_STACK_TO_HC_HCI_CMD        0x2000 /* eq. BT_EVT_TO_LM_HCI_CMD */

#define HCI_EVT_CMD_CMPL_OPCODE_OFFSET          (3)     //opcode's offset in COMMAND Completed Event
#define HCI_EVT_CMD_CMPL_STATUS_OFFSET          (5)     //status's offset in COMMAND Completed Event

#define BT_HC_HDR_SIZE (sizeof(HC_BT_HDR))
#define HCI_CMD_PREAMBLE_SIZE                   (3)

#define HCI_CMD_MAX_LEN             258

static bt_vendor_callbacks_t *bt_vendor_cbacks;
void brt_vendor_init(const bt_vendor_callbacks_t *p_cb)
{
    bt_vendor_cbacks = (bt_vendor_callbacks_t *) p_cb;
}

static void hw_config_cback(void *p_mem)
{
	VND_DBG("%s", __func__);
	HC_BT_HDR   *p_evt_buf = NULL;
	uint8_t     status = 0;
    uint16_t    opcode = 0;
	uint8_t     *p = NULL, *pp=NULL;
	if(p_mem != NULL)
    {
        p_evt_buf = (HC_BT_HDR *) p_mem;
        status = *((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_STATUS_OFFSET);
        p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_OPCODE_OFFSET;
        STREAM_TO_UINT16(opcode,p);
    }

	VND_DBG("%s, status = %d, opcode = %04X", __func__, status, opcode);

	if (status == 0) {
		s_fwcfg_cb(0);//0 is success
	} else {
		s_fwcfg_cb(1);
	}
}

static void hlp_start_h5_init()
{
	VND_DBG("%s", __func__);
	HC_BT_HDR  *p_buf = NULL;
	uint8_t     *p;

	p_buf = bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + HCI_CMD_PREAMBLE_SIZE);

	if (p_buf)
	{
		p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
		p_buf->offset = 0;
		p_buf->layer_specific = 0;
		p_buf->len = HCI_CMD_PREAMBLE_SIZE;

		p = (uint8_t *)(p_buf + 1);
		UINT16_TO_STREAM(p, HCI_VSC_H5_INIT);
		*p = 0; /* parameter length */
		//hw_cfg_cb.state = HW_CFG_H5_INIT;

		bt_vendor_cbacks->xmit_cb(HCI_VSC_H5_INIT, p_buf, hw_config_cback);
	}
	else
	{
		VND_DBG("%s buffer alloc fail!", __func__);
	}
}
