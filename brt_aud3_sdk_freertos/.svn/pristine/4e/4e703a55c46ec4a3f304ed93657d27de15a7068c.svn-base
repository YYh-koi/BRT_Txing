#ifndef __VND_BARROT_H__
#define __VND_BARROT_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "br8051_vs_hci.h"

#define BARROT_CHIP_ID_8051A01 0x08051A01
#define BARROT_CHIP_ID_8051A02 0x08051A02
#define FW_PATCHFILE_EXTENSION	  ".brbin"
#define FW_PATCHFILE_EXTENSION_LEN 6
#define FW_PATCHFILE_PATH_MAXLEN	248
#define HCI_CMD_MAX_LEN			 258

#define PATCH_DATA_ONE_PACKAGE_SIZE		128
#define PATCH_MAX_SIZE					(16*1024)


#define HCI_EVT_TAG                          1
#define HCI_EVT_CMD_CMPL_OPCODE				 3
#define HCI_EVT_CMD_CMPL_LOCAL_CHIP_INFO	12

#define HCI_RESET							   0x0C03
#define HCI_BRT_VS_CMD 0xFC80


#define HCI_VSC_H5_INIT                         0xFCEE
#endif

typedef void (*fw_config_callback)(int result);
void barrot_firmware_config(void *param, fw_config_callback cb);
