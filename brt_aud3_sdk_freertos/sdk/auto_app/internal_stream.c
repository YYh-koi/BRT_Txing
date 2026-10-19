#include "brt_trace.h"
#include "internal_stream.h"
#include "auto_gap.h"
#include "auto_stream.h"
#include "hfp_ui.h"
#include "brt_trace.h"

#define TAG "bse_stream"
#ifdef CONFIG_BT_CHIP_CSR8350
#define  SET_MICGAIN_VOLUME_DEFAULT  0x000C
#define  SET_SPKA2DPGAIN_VOLUME_DEFAULT  0x000F
#define  SET_SPKRINGGAIN_VOLUME_DEFAULT  0x000F
#define  SET_SPKSCOGAIN_VOLUME_DEFAULT   0x000F
#define  SET_DSPMIC_BASE_GAIN      0x00C0
#define  CONFIG_CVC_OPERATE			 1
//#define	 SCO_USE_2MIC				 1
#ifndef CONFIG_CVC_OPERATE
#undef SCO_USE_2MIC
#endif

SYSTEM_LOCK s_vendorCmd_resp_lock;
SYSTEM_LOCK s_vendorCmd_getreq_lock;
static struct FsmTimer *s_a2dp_fadeout_timer = NULL;
static struct FsmTimer *s_sco_fadeout_timer = NULL;
static int s_get_csr_pskey_wSeqNum = 0;
static int s_use_mica = 1;
//  patch v3
/*
static UINT8 s_patch_data[][228] = {"\xc3\x01\x00\x23\x00\x0d\x00\x1c\x10\x00\x00\x19\x00\x74\x26\x00\x00\x00\x00\x05\x00\x20\x01\xe8\x03\x39\xd1\x31\x00\x23\xd5\x04\x00\x00\xfd\x00\x00\xf0\xdd\x58\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x0e\x00\x1c\x10\x00\x00\x19\x00\x75\x0c\x00\x00\x00\x00\x26\x00\x18\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0b\x00\x58\xf1\xfd\xff\x05\x21\x92\x21\x10\xdd\x08\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x05\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x24\x03\x00\x00\x09\xd5\x2d\x00\x43\xd1\x08\x00\x53\xd1\x09\x00\x10\xdd\x05\x00\xe2\x91\x88\x00\x04\xd9\x00\x00\x60\xd9\x01\x00\xf0\xdd\x09\x00\xe2\x91\x08\x00\x55\x91\x7c\x00\x44\x91\x13\x00\x45\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x49\x00\x0f\x00\x1c\x10\x00\x00\x19\x00\x75\x0c\x00\x00\x00\x00\x26\x00\x0e\x00\xe1\x91\x7f\x00\x04\xd9\x00\x00\x60\xd9\x01\x00\x66\x91\x7c\x00\x43\xd1\x07\x00\x64\x9b\x00\x00\x46\x20\x8f\x00\x93\xd1\x04\x00\x29\xd1\x28\x00\x24\x03\x00\x00\x29\xd5\x28\x00\x22\x91\x78\x00\x00\xfd\x00\x00\xf0\xdd\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x4f\x00\x10\x00\x1c\x10\x00\x00\x19\x00\x64\x0c\x00\x00\x00\x00\x10\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0b\x00\x58\xf1\xfd\xff\x05\x21\x8a\x21\x10\xdd\x08\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x05\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x24\x03\x00\x00\x09\xd5\x2d\x00\xe2\x91\x88\x00\x00\xfd\x00\x00\xf0\xdd\xbe\x21\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x11\x00\x1c\x10\x00\x00\x19\x00\xe5\x1a\x00\x00\x00\x00\x30\x00\x18\x00\x38\xf1\xff\xff\x93\xd1\x09\x00\x09\xd1\x35\x00\x00\xdd\x25\x00\x29\xd1\x00\x00\x53\xd1\x07\x00\x00\xfd\x00\x00\xf0\xe1\x48\x81\x22\x8d\x01\x00\x85\x01\x06\x00\x02\x20\x8f\x00\x00\xfd\x00\x00\x40\xdd\x15\x1b\xc5\xe4\x2f\x00\xcc\x8d\x7f\x00\x29\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x1a\x81\x40\xf3\x00\x00\xc6\xf3\x00\x00\x40\x50\x2f\x00\xe0\x50\x3f\x00\x20\x01\x03\x00\x00\x03\xd1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x12\x00\x1c\x10\x00\x00\x19\x00\xe5\x1a\x00\x00\x00\x00\x30\x00\x18\x00\x00\x03\xa1\x00\x29\xd1\x38\x00\x39\xd1\x39\x00\x23\x00\x2f\x00\x29\xd5\x39\x00\x00\x03\xa1\x00\x20\x01\x00\x00\xf0\xe5\x02\x00\x00\x03\xa1\x00\x29\xd1\x00\x00\x30\x44\x4f\x00\x00\xfd\x00\x00\xf0\xe1\x84\x81\xe0\x51\x00\x00\x00\xfd\x00\x00\xf0\xdd\x15\x1b\x09\xd1\x27\x00\x00\xfd\x00\x00\x00\xdd\x08\x1a\xa9\xd1\x29\x00\x29\xd1\x28\x00\x22\x91\x78\x00\x00\xfd\x00\x00\xf0\xdd\xf3\x19\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x2f\x00\x13\x00\x1c\x10\x00\x00\x19\x00\x14\x1b\x00\x00\x00\x00\x08\x00\x09\xd1\x27\x00\x00\xfd\x00\x00\x00\xdd\xf7\x2e\x69\xd1\x29\x00\x29\xd1\x28\x00\x22\x91\x78\x00\x00\xfd\x00\x00\xf0\xdd\xe2\x2e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x14\x00\x1c\x10\x00\x00\x19\x00\xd5\x1b\x00\x00\x00\x00\x24\x00\x18\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0c\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x09\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x44\x8d\x08\x00\x69\xd1\x28\x00\x64\x23\x00\x00\x69\xd5\x28\x00\x09\xd5\x2d\x00\x09\xd5\x2b\x00\x63\xd1\x0c\x00\x43\xd1\x08\x00\x64\x9b\x00\x00\x43\xd1\x09\x00\x53\xd1\x0a\x00\x10\xdd\x05\x00\xe6\x91\x88\x00\x04\xd9\x00\x00\x50\xd9\x01\x00\xf0\xdd\x09\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x41\x00\x15\x00\x1c\x10\x00\x00\x19\x00\xd5\x1b\x00\x00\x00\x00\x24\x00\x0c\x00\xe6\x91\x08\x00\x55\x91\x7c\x00\x44\x91\x13\x00\x45\x03\x00\x00\xe1\x91\x7f\x00\x04\xd9\x00\x00\x50\xd9\x01\x00\x55\x91\x7c\x00\x22\x91\x08\x00\x25\x20\x2f\x00\x00\xfd\x00\x00\xf0\xdd\x78\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x16\x00\x1c\x10\x00\x00\x19\x00\x05\x1c\x00\x00\x00\x00\x1e\x00\x18\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0c\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x09\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x44\x8d\x08\x00\x69\xd1\x28\x00\x64\x03\x00\x00\x69\xd5\x28\x00\x09\xd5\x2d\x00\x09\xd5\x2b\x00\x63\xd1\x0c\x00\x66\x91\x08\x00\x43\xd1\x09\x00\x56\x98\x4f\x00\x43\xd1\x0a\x00\x00\xdd\x03\x00\x46\x94\x4f\x00\x54\x00\x5f\x00\xe5\x91\x80\x00\x43\xd1\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x29\x00\x17\x00\x1c\x10\x00\x00\x19\x00\x05\x1c\x00\x00\x00\x00\x1e\x00\x06\x00\x04\xd9\x00\x00\x50\xd9\x01\x00\x22\x91\x08\x00\x62\x20\x5f\x00\x00\xfd\x00\x00\xf0\xdd\x7d\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x18\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\x9a\xd1\x03\x00\x90\xf3\x00\x00\x70\xf3\x00\x00\x00\xf3\x00\x00\x29\xd1\x70\x01\x00\xfd\x00\x00\xf0\xe1\x93\xa5\x29\x00\x0f\x00\x98\xf1\xfd\xff\x02\x21\x06\x00\x10\xdd\xa2\x00\x09\xd1\x6d\x01\x00\xdd\x07\x00\x09\xd5\x6d\x01\x09\xd5\x6b\x01\x09\xd5\x6e\x01\x09\xd5\x31\x00\x09\xd5\x6a\x01\x07\xd5\x03\x00\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x0f\x81\x00\x50\x2f\x00\xc0\x50\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x19\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\xe0\x50\x3f\x00\x4c\xf1\xff\xff\xca\xf1\xff\xff\xaa\xf1\xff\xff\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x51\x81\xc2\x00\x0f\x00\x27\xd1\x03\x00\x02\x21\x02\x00\x00\xdd\x61\x00\x89\xd1\x6e\x01\x08\x21\x04\x00\x50\xdd\x5e\x00\x00\xf1\x00\x11\x01\xf3\x00\x00\xf0\xe1\xa1\x00\x05\xf3\x00\x00\x04\xf1\x00\x11\x40\x58\x0f\x00\x29\xd1\x31\x00\x28\x8b\x00\x00\x22\x81\x01\x00\x48\x00\x2f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x1a\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\x44\x8d\x7f\x00\xa4\x71\x01\x00\xc4\x22\x00\x32\xd0\xdd\x7c\x00\x00\x03\x00\x51\x02\x81\x01\x00\x50\x00\x00\x00\x55\x81\xff\x00\x55\x8d\x08\x00\x09\xd1\x31\x00\x00\xdd\x07\x00\x08\x81\x01\x00\x00\xdd\x05\x00\x00\x03\x00\x20\x22\x8d\x78\x00\x22\x81\xff\x00\x52\x87\x00\x00\x59\xd5\x6f\x01\xcc\x21\x02\x00\xd0\xdd\x6c\x00\xf0\xe1\x71\x00\x52\x00\x0f\x00\x69\xd1\x6b\x01\x10\xdd\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x1b\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\xf0\xe1\x6d\x00\x62\x00\x0f\x00\xf0\xdd\x08\x00\xf0\xe1\x6a\x00\x02\x20\x6f\x00\x00\xdd\x05\x00\x52\x00\x0f\x00\xcc\xe4\x3f\x00\xd0\xdd\x5c\x00\xf0\xdd\xfa\xff\x69\xd5\x6b\x01\x05\x21\xe8\x03\x50\xdd\x58\x00\x69\xd1\x31\x00\x45\x00\x0f\x00\x09\xd1\x69\x01\x00\xdd\x08\x00\x20\x01\x0d\x00\x45\x20\x2f\x00\x26\x03\x00\x00\x22\x8d\x7f\x00\xa0\x50\x2f\x00\xc2\x22\x00\x22\xd0\xdd\x50\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x1c\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\x36\x00\x5f\x00\x33\x81\x01\x00\x39\xd5\x31\x00\x24\x00\x8f\x00\x29\xd5\x6e\x01\x0d\xf1\xff\xff\x08\x21\x02\x00\x40\xdd\x04\x00\x00\x03\x31\x23\x0d\xf1\xff\xff\x00\x03\x00\xb1\x09\xd5\x6a\x01\x39\xd1\x6f\x01\x08\x81\x01\x00\x00\xdd\x06\x00\x00\x03\x00\x20\x22\x81\xff\x00\x23\x87\x00\x00\x00\x03\x00\xa1\xf0\xdd\x07\x00\x09\xd1\x69\x01\x00\xdd\x05\x00\x06\x81\x01\x00\x10\xdd\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x1d\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\x20\x44\x0f\x00\x29\xd5\x6a\x01\x09\xf1\xff\xff\x30\x44\x0f\x00\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x80\x81\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x51\x81\xc2\x00\x0f\x00\xf0\xe1\x46\x00\xf0\xe1\x3f\x00\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x0f\x81\x2c\xf1\xff\xff\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x51\x81\x22\x91\x7f\x00\x39\xd1\x6e\x01\x33\x91\x7e\x00\x03\x20\x2f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x1e\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\x32\x00\x0a\x00\x20\x01\x32\x00\x03\x21\x32\x00\x32\x00\x05\x00\xb9\x01\x33\x00\x3b\xd5\x35\x01\x00\xfd\x00\x00\xf0\xe1\x30\x1e\x20\x01\x00\x00\x37\xd1\x03\x00\x02\x20\x3f\x00\x10\xdd\x0e\x00\x78\xf1\xfe\xff\x98\xf1\xfd\xff\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x0f\x81\x48\xf1\xff\xff\x24\x23\x00\x00\x22\x00\x34\x00\x22\x91\x01\x00\x39\xd1\x6e\x01\x23\x20\x2f\x00\x20\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x1f\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\x29\xd5\x6e\x01\x03\xf1\xfd\xff\x00\xfd\x00\x00\xf0\xdd\xa9\x00\xf0\xe1\x17\x00\x00\xfd\x00\x00\xf0\xe1\x6d\x41\xf0\xe1\x14\x00\x03\xf1\xfd\xff\x20\xe4\x2f\x00\x27\xd5\x03\x00\x00\xfd\x00\x00\xf0\xdd\xa9\x00\x00\x03\x00\x21\x09\xd1\x31\x00\x00\xdd\x06\x00\x22\x81\xff\x00\x00\x03\x00\x30\x33\x81\x00\xff\x23\x87\x00\x00\x0d\xdc\x0f\x00\x32\x8d\x78\x00\x33\x81\xff\x00\x22\x81\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x20\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x18\x00\x22\x8d\x08\x00\x23\x87\x00\x00\x0d\xdc\x0f\x00\x00\xf3\x00\x00\xaa\xf1\xff\xff\xc6\xf3\x00\x00\xc0\x51\x00\x00\xe0\x51\x00\x00\x0d\xdc\x0f\x00\x29\xd1\x6a\x01\x0d\xdc\x00\x00\x89\xd1\x6e\x01\x88\x8d\x7f\x00\xa0\x50\x8f\x00\x00\x02\x00\x32\x30\x68\x2f\x00\x33\x44\xc4\x00\x0c\x20\x3f\x00\xc3\x00\x05\x00\x0c\x00\x0f\x00\x0d\xdc\x00\x00\xa0\x51\x01\x00\x00\x50\x2f\x00\x40\x50\x2f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x49\x00\x21\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe6\x00\x0e\x00\x50\x01\x08\x00\x00\xfd\x00\x00\x60\x01\x00\xff\x00\x03\x31\x00\x23\x8c\x5f\x00\xf0\xe5\x06\x00\x26\x83\x31\x00\x43\x80\x6f\x00\x44\x8d\x78\x00\x42\x87\x00\x00\x23\x8c\x5f\xc2\x20\x44\x0f\x00\x29\xd5\x6a\x01\x0d\xdc\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x23\x00\x22\x00\x1c\x10\x00\x00\x19\x00\x74\x07\x00\x00\x00\x00\x05\x00\x45\xd1\x03\x00\x44\x81\xff\xff\x40\x19\xff\x3f\x00\xfd\x00\x00\xf0\xe1\x1f\x5c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x71\x00\x23\x00\x1c\x10\x00\x00\x19\x00\x85\x0f\x00\x00\x00\x00\x29\x00\x18\x00\x40\x01\x71\x01\x60\x01\x00\x02\x00\xfd\x80\x00\x80\x01\x00\x01\x00\xfd\x00\x00\xf0\xe1\x66\x37\x09\xd5\x32\x00\x20\x01\x7b\x62\x29\xd5\x22\x00\x29\xd1\x2e\x00\x30\x01\xde\x2f\x32\xd5\x00\x00\x30\xe4\x2f\x00\x39\xd5\x6d\x01\xa0\xf3\x00\x00\x00\xfd\x00\x00\xf0\xe1\x35\x40\xa4\xf3\x00\x00\x20\x01\x05\x00\x00\xfd\x00\x00\xf0\xe1\x22\x64\x52\x00\x0f\x00\x59\xd5\x70\x01\x02\x00\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x55\x00\x24\x00\x1c\x10\x00\x00\x19\x00\x85\x0f\x00\x00\x00\x00\x29\x00\x11\x00\x00\xdd\x0f\x00\x90\xf3\x00\x00\x20\x01\x89\x27\x30\x01\x6c\x22\x4a\xd1\x04\x00\x00\xfd\x00\x00\xf0\xe1\x06\xa4\x94\xf3\x00\x00\x20\x09\xff\x3f\x00\xfd\x00\x00\x02\x21\x45\x83\x00\xfd\x00\x00\x10\xdd\xa9\x00\x59\xd1\x70\x01\x05\xd5\x01\x00\x00\xfd\x00\x00\xf0\xdd\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x33\x00\x25\x00\x1c\x10\x00\x00\x19\x00\xda\x03\x00\x00\x00\x00\x09\x00\xd0\xf3\x00\x00\x00\xfd\x00\x00\x40\x01\x32\x80\x50\x01\x00\x00\x60\x01\x10\x00\x00\xfd\x00\x00\xf0\xe1\x88\x81\x00\xfd\x00\x00\xf0\xdd\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
"\xc3\x01\x00\x33\x00\x26\x00\x1c\x10\x00\x00\x19\x00\x94\x31\x00\x00\x00\x00\x05\x00\x24\xf3\x00\x00\x90\x01\x06\x00\x02\xd1\x01\x00\x0d\xdc\x0f\x00"};
*/
// patch v4

static UINT8 s_patch_data[][228] = {"\xc3\x01\x00\x23\x00\x0d\x00\x1c\x10\x00\x00\x19\x00\x74\x26\x00\x00\x00\x00\x05\x00\x20\x01\xe8\x03\x39\xd1\x31\x00\x23\xd5\x04\x00\x00\xfd\x00\x00\xf0\xdd\x58\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x0e\x00\x1c\x10\x00\x00\x19\x00\x75\x0c\x00\x00\x00\x00\x26\x00\x18\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0b\x00\x58\xf1\xfd\xff\x05\x21\x92\x21\x10\xdd\x08\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x05\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x24\x03\x00\x00\x09\xd5\x2d\x00\x43\xd1\x08\x00\x53\xd1\x09\x00\x10\xdd\x05\x00\xe2\x91\x88\x00\x04\xd9\x00\x00\x60\xd9\x01\x00\xf0\xdd\x09\x00\xe2\x91\x08\x00\x55\x91\x7c\x00\x44\x91\x13\x00\x45\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x49\x00\x0f\x00\x1c\x10\x00\x00\x19\x00\x75\x0c\x00\x00\x00\x00\x26\x00\x0e\x00\xe1\x91\x7f\x00\x04\xd9\x00\x00\x60\xd9\x01\x00\x66\x91\x7c\x00\x43\xd1\x07\x00\x64\x9b\x00\x00\x46\x20\x8f\x00\x93\xd1\x04\x00\x29\xd1\x28\x00\x24\x03\x00\x00\x29\xd5\x28\x00\x22\x91\x78\x00\x00\xfd\x00\x00\xf0\xdd\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x4f\x00\x10\x00\x1c\x10\x00\x00\x19\x00\x64\x0c\x00\x00\x00\x00\x10\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0b\x00\x58\xf1\xfd\xff\x05\x21\x8a\x21\x10\xdd\x08\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x05\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x24\x03\x00\x00\x09\xd5\x2d\x00\xe2\x91\x88\x00\x00\xfd\x00\x00\xf0\xdd\xbe\x21\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x11\x00\x1c\x10\x00\x00\x19\x00\xe5\x1a\x00\x00\x00\x00\x30\x00\x18\x00\x38\xf1\xff\xff\x93\xd1\x09\x00\x09\xd1\x35\x00\x00\xdd\x25\x00\x29\xd1\x00\x00\x53\xd1\x07\x00\x00\xfd\x00\x00\xf0\xe1\x48\x81\x22\x8d\x01\x00\x85\x01\x06\x00\x02\x20\x8f\x00\x00\xfd\x00\x00\x40\xdd\x15\x1b\xc5\xe4\x2f\x00\xcc\x8d\x7f\x00\x29\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x1a\x81\x40\xf3\x00\x00\xc6\xf3\x00\x00\x40\x50\x2f\x00\xe0\x50\x3f\x00\x20\x01\x03\x00\x00\x03\xd1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x12\x00\x1c\x10\x00\x00\x19\x00\xe5\x1a\x00\x00\x00\x00\x30\x00\x18\x00\x00\x03\xa1\x00\x29\xd1\x38\x00\x39\xd1\x39\x00\x23\x00\x2f\x00\x29\xd5\x39\x00\x00\x03\xa1\x00\x20\x01\x00\x00\xf0\xe5\x02\x00\x00\x03\xa1\x00\x29\xd1\x00\x00\x30\x44\x4f\x00\x00\xfd\x00\x00\xf0\xe1\x84\x81\xe0\x51\x00\x00\x00\xfd\x00\x00\xf0\xdd\x15\x1b\x09\xd1\x27\x00\x00\xfd\x00\x00\x00\xdd\x08\x1a\xa9\xd1\x29\x00\x29\xd1\x28\x00\x22\x91\x78\x00\x00\xfd\x00\x00\xf0\xdd\xf3\x19\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x2f\x00\x13\x00\x1c\x10\x00\x00\x19\x00\x14\x1b\x00\x00\x00\x00\x08\x00\x09\xd1\x27\x00\x00\xfd\x00\x00\x00\xdd\xf7\x2e\x69\xd1\x29\x00\x29\xd1\x28\x00\x22\x91\x78\x00\x00\xfd\x00\x00\xf0\xdd\xe2\x2e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x14\x00\x1c\x10\x00\x00\x19\x00\xd5\x1b\x00\x00\x00\x00\x24\x00\x18\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0c\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x09\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x44\x8d\x08\x00\x69\xd1\x28\x00\x64\x23\x00\x00\x69\xd5\x28\x00\x09\xd5\x2d\x00\x09\xd5\x2b\x00\x63\xd1\x0c\x00\x43\xd1\x08\x00\x64\x9b\x00\x00\x43\xd1\x09\x00\x53\xd1\x0a\x00\x10\xdd\x05\x00\xe6\x91\x88\x00\x04\xd9\x00\x00\x50\xd9\x01\x00\xf0\xdd\x09\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x41\x00\x15\x00\x1c\x10\x00\x00\x19\x00\xd5\x1b\x00\x00\x00\x00\x24\x00\x0c\x00\xe6\x91\x08\x00\x55\x91\x7c\x00\x44\x91\x13\x00\x45\x03\x00\x00\xe1\x91\x7f\x00\x04\xd9\x00\x00\x50\xd9\x01\x00\x55\x91\x7c\x00\x22\x91\x08\x00\x25\x20\x2f\x00\x00\xfd\x00\x00\xf0\xdd\x78\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x16\x00\x1c\x10\x00\x00\x19\x00\x05\x1c\x00\x00\x00\x00\x1e\x00\x18\x00\x24\xf3\x00\x00\x22\x91\x7f\x00\x00\xdd\x0c\x00\x49\xd1\x2d\x00\x04\xd8\x0f\x00\x10\xdd\x09\x00\x00\xfd\x80\x00\x44\x89\x00\x00\x44\x8d\x08\x00\x69\xd1\x28\x00\x64\x03\x00\x00\x69\xd5\x28\x00\x09\xd5\x2d\x00\x09\xd5\x2b\x00\x63\xd1\x0c\x00\x66\x91\x08\x00\x43\xd1\x09\x00\x56\x98\x4f\x00\x43\xd1\x0a\x00\x00\xdd\x03\x00\x46\x94\x4f\x00\x54\x00\x5f\x00\xe5\x91\x80\x00\x43\xd1\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x29\x00\x17\x00\x1c\x10\x00\x00\x19\x00\x05\x1c\x00\x00\x00\x00\x1e\x00\x06\x00\x04\xd9\x00\x00\x50\xd9\x01\x00\x22\x91\x08\x00\x62\x20\x5f\x00\x00\xfd\x00\x00\xf0\xdd\x7d\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x18\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\x9a\xd1\x03\x00\x90\xf3\x00\x00\x70\xf3\x00\x00\x00\xf3\x00\x00\x29\xd1\x70\x01\x00\xfd\x00\x00\xf0\xe1\x93\xa5\x29\x00\x0f\x00\x98\xf1\xfd\xff\x02\x21\x06\x00\x10\xdd\xa0\x00\x09\xd1\x6d\x01\x00\xdd\x07\x00\x09\xd5\x6d\x01\x09\xd5\x6b\x01\x09\xd5\x6e\x01\x09\xd5\x31\x00\x09\xd5\x6a\x01\x07\xd5\x03\x00\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x0f\x81\x00\x50\x2f\x00\xc0\x50\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x19\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\xe0\x50\x3f\x00\x4c\xf1\xff\xff\xca\xf1\xff\xff\xaa\xf1\xff\xff\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x51\x81\xc2\x00\x0f\x00\x27\xd1\x03\x00\x02\x21\x02\x00\x00\xdd\x5F\x00\x89\xd1\x6e\x01\x08\x21\x04\x00\x50\xdd\x5C\x00\x00\xf1\x00\x11\x01\xf3\x00\x00\xf0\xe1\x9f\x00\x05\xf3\x00\x00\x04\xf1\x00\x11\x40\x58\x0f\x00\x29\xd1\x31\x00\x28\x8b\x00\x00\x22\x81\x01\x00\x48\x00\x2f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x1a\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\x44\x8d\x7f\x00\xa4\x71\x01\x00\xc4\x22\x00\x32\xd0\xdd\x7a\x00\x00\x03\x00\x51\x02\x81\x01\x00\x50\x00\x00\x00\x55\x81\xff\x00\x55\x8d\x08\x00\x09\xd1\x31\x00\x00\xdd\x05\x00\x08\x81\x01\x00\x00\xdd\x03\x00\x00\x03\x00\x20\x52\x81\x00\xff\x59\xd5\x6f\x01\xcc\x21\x02\x00\xd0\xdd\x6c\x00\xf0\xe1\x71\x00\x52\x00\x0f\x00\x69\xd1\x6b\x01\x10\xdd\x04\x00\xf0\xe1\x6d\x00\x62\x00\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x1b\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\xf0\xdd\x08\x00\xf0\xe1\x6a\x00\x02\x20\x6f\x00\x00\xdd\x05\x00\x52\x00\x0f\x00\xcc\xe4\x3f\x00\xd0\xdd\x5c\x00\xf0\xdd\xfa\xff\x69\xd5\x6b\x01\x05\x21\xe8\x03\x50\xdd\x58\x00\x69\xd1\x31\x00\x45\x00\x0f\x00\x09\xd1\x69\x01\x00\xdd\x08\x00\x20\x01\x0d\x00\x45\x20\x2f\x00\x26\x03\x00\x00\x22\x8d\x7f\x00\xa0\x50\x2f\x00\xc2\x22\x00\x22\xd0\xdd\x50\x00\x36\x00\x5f\x00\x33\x81\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x1c\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\x39\xd5\x31\x00\x24\x00\x8f\x00\x29\xd5\x6e\x01\x0d\xf1\xff\xff\x08\x21\x02\x00\x40\xdd\x04\x00\x00\x03\x31\x23\x0d\xf1\xff\xff\x00\x03\x00\xb1\x09\xd5\x6a\x01\x39\xd1\x6f\x01\x08\x81\x01\x00\x00\xdd\x06\x00\x00\x03\x00\x20\x22\x81\xff\x00\x23\x87\x00\x00\x00\x03\x00\xa1\xf0\xdd\x07\x00\x09\xd1\x69\x01\x00\xdd\x05\x00\x06\x81\x01\x00\x10\xdd\x03\x00\x20\x44\x0f\x00\x29\xd5\x6a\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x1d\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\x09\xf1\xff\xff\x30\x44\x0f\x00\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x80\x81\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x51\x81\xc2\x00\x0f\x00\xf0\xe1\x46\x00\xf0\xe1\x3f\x00\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x0f\x81\x2c\xf1\xff\xff\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x51\x81\x22\x91\x7f\x00\x39\xd1\x6e\x01\x33\x91\x7e\x00\x03\x20\x2f\x00\x32\x00\x0a\x00\x20\x01\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x1e\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\x03\x21\x32\x00\x32\x00\x05\x00\xb9\x01\x33\x00\x3b\xd5\x35\x01\x00\xfd\x00\x00\xf0\xe1\x30\x1e\x20\x01\x00\x00\x37\xd1\x03\x00\x02\x20\x3f\x00\x10\xdd\x0e\x00\x78\xf1\xfe\xff\x98\xf1\xfd\xff\x27\xd1\x00\x00\x00\xfd\x00\x00\xf0\xe1\x0f\x81\x48\xf1\xff\xff\x24\x23\x00\x00\x22\x00\x34\x00\x22\x91\x01\x00\x39\xd1\x6e\x01\x23\x20\x2f\x00\x20\x00\x04\x00\x29\xd5\x6e\x01\x03\xf1\xfd\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x1f\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\x00\xfd\x00\x00\xf0\xdd\xa9\x00\xf0\xe1\x17\x00\x00\xfd\x00\x00\xf0\xe1\x6d\x41\xf0\xe1\x14\x00\x03\xf1\xfd\xff\x20\xe4\x2f\x00\x27\xd5\x03\x00\x00\xfd\x00\x00\xf0\xdd\xa9\x00\x00\x03\x00\x21\x09\xd1\x31\x00\x00\xdd\x06\x00\x22\x81\xff\x00\x00\x03\x00\x30\x33\x81\x00\xff\x23\x87\x00\x00\x0d\xdc\x0f\x00\x32\x8d\x78\x00\x33\x81\xff\x00\x22\x81\xff\x00\x22\x8d\x08\x00\x23\x87\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x20\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x18\x00\x0d\xdc\x0f\x00\x00\xf3\x00\x00\xaa\xf1\xff\xff\xc6\xf3\x00\x00\xc0\x51\x00\x00\xe0\x51\x00\x00\x0d\xdc\x0f\x00\x29\xd1\x6a\x01\x0d\xdc\x00\x00\x89\xd1\x6e\x01\x88\x8d\x7f\x00\xa0\x50\x8f\x00\x00\x02\x00\x32\x30\x68\x2f\x00\x33\x44\xc4\x00\x0c\x20\x3f\x00\xc3\x00\x05\x00\x0c\x00\x0f\x00\x0d\xdc\x00\x00\xa0\x51\x01\x00\x00\x50\x2f\x00\x40\x50\x2f\x00\x50\x01\x08\x00\x00\xfd\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x49\x00\x21\x00\x1c\x10\x00\x00\x19\x00\x45\x10\x00\x00\x00\x00\xe4\x00\x0c\x00\x60\x01\x00\xff\x00\x03\x31\x00\x23\x8c\x5f\x00\xf0\xe5\x06\x00\x26\x83\x31\x00\x43\x80\x6f\x00\x44\x8d\x78\x00\x42\x87\x00\x00\x23\x8c\x5f\xc2\x20\x44\x0f\x00\x29\xd5\x6a\x01\x0d\xdc\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x23\x00\x22\x00\x1c\x10\x00\x00\x19\x00\x74\x07\x00\x00\x00\x00\x05\x00\x45\xd1\x03\x00\x44\x81\xff\xff\x40\x19\xff\x3f\x00\xfd\x00\x00\xf0\xe1\x1f\x5c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x71\x00\x23\x00\x1c\x10\x00\x00\x19\x00\x85\x0f\x00\x00\x00\x00\x29\x00\x18\x00\x40\x01\x71\x01\x60\x01\x00\x02\x00\xfd\x80\x00\x80\x01\x00\x01\x00\xfd\x00\x00\xf0\xe1\x66\x37\x09\xd5\x32\x00\x20\x01\x7b\x62\x29\xd5\x22\x00\x29\xd1\x2e\x00\x30\x01\xde\x2f\x32\xd5\x00\x00\x30\xe4\x2f\x00\x39\xd5\x6d\x01\xa0\xf3\x00\x00\x00\xfd\x00\x00\xf0\xe1\x35\x40\xa4\xf3\x00\x00\x20\x01\x05\x00\x00\xfd\x00\x00\xf0\xe1\x22\x64\x52\x00\x0f\x00\x59\xd5\x70\x01\x02\x00\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x55\x00\x24\x00\x1c\x10\x00\x00\x19\x00\x85\x0f\x00\x00\x00\x00\x29\x00\x11\x00\x00\xdd\x0f\x00\x90\xf3\x00\x00\x20\x01\x89\x27\x30\x01\x6c\x22\x4a\xd1\x04\x00\x00\xfd\x00\x00\xf0\xe1\x06\xa4\x94\xf3\x00\x00\x20\x09\xff\x3f\x00\xfd\x00\x00\x02\x21\x45\x83\x00\xfd\x00\x00\x10\xdd\xa9\x00\x59\xd1\x70\x01\x05\xd5\x01\x00\x00\xfd\x00\x00\xf0\xdd\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x33\x00\x25\x00\x1c\x10\x00\x00\x19\x00\xda\x03\x00\x00\x00\x00\x09\x00\xd0\xf3\x00\x00\x00\xfd\x00\x00\x40\x01\x32\x80\x50\x01\x00\x00\x60\x01\x10\x00\x00\xfd\x00\x00\xf0\xe1\x88\x81\x00\xfd\x00\x00\xf0\xdd\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                    "\xc3\x01\x00\x33\x00\x26\x00\x1c\x10\x00\x00\x19\x00\x94\x31\x00\x00\x00\x00\x05\x00\x24\xf3\x00\x00\x90\x01\x06\x00\x02\xd1\x01\x00\x0d\xdc\x0f\x00"
                                   };


//a2dp
static DSP_A2DP_MANAGER     s_dsp_a2dp_manager = {0};
// ringtone;
static DSP_RINGTONE_MANAGER s_dsp_ringtone_manager = {0};
//sco
static DSP_SCO_MANAGER		s_dsp_sco_manager = {0};
//Sync_sink_seqno
static DWORD s_startstream_Init_tramp = 0;
static UINT16 s_patch_index = 0;
static UINT8 s_stream_type = STREAM_IDLE;
static UINT16 s_wSeqNum = 0;
static WORD s_nDAC_Gain = 15;
static WORD s_n1stMic_Gain = 0x00CC;
static WORD s_n2ndMic_Gain = 0x00CC;
static BOOL s_bIsFirstCvc = TRUE;

#ifdef USE_RINGTONE_FASTPIPE
UINT8 * wavdata_buf = NULL;
int fastpipe_index = 0;
#endif

static int operator_message_audio( WORD sbc_opid, WORD message_id, WORD *message);
static int operator_message_cvc( WORD sbc_opid, WORD message_id, WORD *message);

extern UINT16 s_nFactoryMicGain;

#define DWORD_PTR DWORD
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))

/*++++++++++++++++++++++++++++++++++++++++*/
//[aug]trigger controller to request patch
//[param]
/*----------------------------------------*/
int hlp_audio_8350patch()
{
	create_operator_c(PATCH_AUDTO_CAP , 0x0000, 0x0000);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]assignment the parameter when operate
//      dsp use
//[param]cid acl_handle type
/*-------------------------------------------*/
int hlp_audio_open( UINT16 cid, UINT16 acl_handle, UINT8 type)
{
	bse_log_api(TAG, "[dsp][hlp_audio_open]type = %d, acl_handle = %04x, cid = %04x\r\n", type, acl_handle, cid);
	if ( type == STREAM_A2DP) {
		s_dsp_a2dp_manager.sbc_a2dp_cid = cid;
		s_dsp_a2dp_manager.acl_handle = acl_handle;
	} else if( type == STREAM_SCO) {
		s_dsp_sco_manager.sco_hdl = cid;
	} else if( type == STREAM_RINGTONE) {
	}
	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]use for send BCCMD to init PIO
//[param]
/*-------------------------------------------*/
int pio32_set_pin_function( WORD pin_number, WORD pio_function)
{
	WORD param[2] = {0};
	param[0] = pin_number;
	param[1] = pio_function;
	bse_log_info(TAG, "[dsp][cmd]pio32_set_pin_function(0x5082)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ,PIO32_SET_PIN_FUNCTION,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]due to the PIO(2-9) multiplexing PIO/PCM/SPI,should init before use them as PCM
//[param]pin_number-PIO Index, pio_function-role
/*-------------------------------------------*/
int pcm_init_pio32()
{

	//config PCM1,multiplex with SPI,so do not use this for I2S output
	/* 
	pio32_set_pin_function(PIO2,PCM_IN);
	pio32_set_pin_function(PIO3,PCM_OUT);
	pio32_set_pin_function(PIO4,PCM_SYNC);
	pio32_set_pin_function(PIO5,PCM_CLK);
	*/
	
	//config PCM2
	pio32_set_pin_function(PIO6,PCM_OUT);
	pio32_set_pin_function(PIO7,PCM_SYNC);
	pio32_set_pin_function(PIO8,PCM_IN);
	pio32_set_pin_function(PIO9,PCM_CLK);
	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]assignment the parameter when operate
//      dsp use
//[param]cid acl_handle type
/*-------------------------------------------*/
int hlp_audio_start( UINT8 type)
{
	if (type == STREAM_A2DP) {
		//use connection_property to get l2cap_src
#ifdef A2DP_SNK_I2S_OUTPUT
		pcm_init_pio32();
#endif
		s_dsp_a2dp_manager.a2dp_get_sbcsrc_SeqNum = s_wSeqNum;
		stream_get_source( L2CAP_TYPE, s_dsp_a2dp_manager.acl_handle, s_dsp_a2dp_manager.sbc_a2dp_cid);
	} else if( type == STREAM_SCO) {
		s_startstream_Init_tramp = GetCurrTime();

		s_dsp_sco_manager.create_scosend_SeqNum = s_wSeqNum;
		bse_log_info(TAG, "[dsp][audio_start] create_scosend_SeqNum: %d", s_dsp_sco_manager.create_scosend_SeqNum);
		s_dsp_sco_manager.sbc_sco_mic_gain = SET_MICGAIN_VOLUME_DEFAULT;
		s_dsp_sco_manager.sbc_sco_spk_gain = SET_SPKSCOGAIN_VOLUME_DEFAULT;
		if (get_codec_type() == HFP_CODECID_MSBC) {
			create_operator_c( WBS_SEND_CAP, 0x0000, 0x0000);

			s_dsp_sco_manager.create_scorecv_SeqNum = s_wSeqNum;
			create_operator_c( WBS_RECEIVE_CAP, 0x0000, 0x0000);

#ifdef CONFIG_CVC_OPERATE
			s_dsp_sco_manager.create_cvchfmic_send_SeqNum = s_wSeqNum;
#ifndef SCO_USE_2MIC
			create_operator_c( CVCHF1MIC_SEND_WB_CAP, 0x0000, 0x0000);
#else
			create_operator_c( CVCHF2MIC_SEND_WB_CAP, 0x0000, 0x0000);
#endif
			s_dsp_sco_manager.create_cvchf_receive_SeqNum = s_wSeqNum;
			create_operator_c( CVCHF_RECEIVE_WB_CAP, 0x0000, 0x0000);
#endif
		} else {
			create_operator_c( SCO_SEND_CAP, 0x0000, 0x0000);

			s_dsp_sco_manager.create_scorecv_SeqNum = s_wSeqNum;
			create_operator_c( SCO_RECEIVE_CAP, 0x0000, 0x0000);

#ifdef CONFIG_CVC_OPERATE
			s_dsp_sco_manager.create_cvchfmic_send_SeqNum = s_wSeqNum;
#ifndef SCO_USE_2MIC
			create_operator_c( CVCHF1MIC_SEND_NB_CAP, 0x0000, 0x0000);
#else
			create_operator_c( CVCHF2MIC_SEND_NB_CAP, 0x0000, 0x0000);
#endif
			s_dsp_sco_manager.create_cvchf_receive_SeqNum = s_wSeqNum;
			create_operator_c( CVCHF_RECEIVE_NB_CAP, 0x0000, 0x0000);
#endif
		}
	} else if( type == STREAM_RINGTONE) {
		WORD param = 0x0000;
		hlp_stream_set_type(STREAM_RINGTONE);
		s_startstream_Init_tramp = GetCurrTime();
		s_dsp_ringtone_manager.create_tingtone_SeqNum = s_wSeqNum;
#ifndef USE_RINGTONE_FASTPIPE
		create_operator_c( RINGTONE_CAP, 0x0000, 0x0000);
#else
		operator_message_audio( 0xc001, SET_TONE_VOLUME, &param);
		create_operator_c( BASIC_MONO_AUDIO_CAP, 0x0000, 0x0000);
		fastpipe_enable( 2048);
#endif
	} else {
		bse_log_error(TAG, " hlp_audio_start is error! type(%d)\r\n",type);
	}
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_audio_close  after destory_operate
//	   this func init variable to zero
//[param]
/*-------------------------------------------*/
int hlp_audio_close( UINT8 type)
{
	//use app_stream fsm, this func does not be used
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_audio_stop  use operator_message to
//	   trigger stop_process
//[param] stream type
/*-------------------------------------------*/
int hlp_audio_stop( UINT8 type)
{
	WORD param = 0x0000;
	bse_log_api(TAG, "[dsp] hlp_audio_stop type(%d) s_dsp_a2dp_manager.sbc_a2dp_opid: %d, s_dsp_sco_manager.sbc_sco_receive_opid: %d\r\n",  \
			                                                  type, s_dsp_a2dp_manager.sbc_a2dp_opid, s_dsp_sco_manager.sbc_sco_receive_opid);
	if(type == STREAM_A2DP && s_dsp_a2dp_manager.sbc_a2dp_opid) {
		s_dsp_a2dp_manager.a2dp_tostop_SeqNum = s_wSeqNum;
		operator_message_audio( s_dsp_a2dp_manager.sbc_a2dp_opid, AUDIO_ENABLE_FADE_OUT, &param);
		s_a2dp_fadeout_timer = FsmAddTimer(NULL, 500, 0, NULL, NULL, stream_a2dp_stop_timer); //The timer to resolve NO fadeout complete event from controller.
	} else if( type == STREAM_SCO && s_dsp_sco_manager.sbc_sco_receive_opid) {
		s_dsp_sco_manager.sco_opermsg_tostop_SeqNum = s_wSeqNum;
#ifdef CONFIG_CVC_OPERATE
		s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed = 0;
		s_dsp_sco_manager.sbc_sco_cvchf1mic_send_fadeout_completed = 0;
		operator_message_cvc(s_dsp_sco_manager.sbc_cvchf_receive_opid, CVC_ENABLE_FADE_OUT, &param);//[error]
		operator_message_cvc(s_dsp_sco_manager.sbc_cvchf1mic_send_opid, CVC_ENABLE_FADE_OUT, &param);//[error]
#ifdef SCO_USE_2MIC
		s_dsp_sco_manager.sbc_sco_cvchf2mic_send_fadeout_completed = 0;
		operator_message_cvc(s_dsp_sco_manager.sbc_cvchf2mic_send_opid, CVC_ENABLE_FADE_OUT, &param);//[error]
#endif
#else
		//need to see if the method have any value
		operator_message_audio(s_dsp_sco_manager.sbc_sco_send_opid, AUDIO_ENABLE_FADE_OUT, &param);
		operator_message_audio(s_dsp_sco_manager.sbc_sco_receive_opid, AUDIO_ENABLE_FADE_OUT, &param);
#endif
	} else if( type == STREAM_RINGTONE) {
		s_dsp_ringtone_manager.ringtone_stop_SeqNum = s_wSeqNum;
		operator_message_audio( s_dsp_ringtone_manager.sbc_ringtone_opid, RINGTONE_ENABLE_FADE_OUT, &param);
		/****fired the timer in hlp_operator_msg_resp****/
		//FsmAddTimer(NULL, 200, 0, NULL, NULL, stream_ringtone_stop_timer);
	} else {

	}
	return 0;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Get the cmd from event, and process the result
//0000: 04 FF 27 C3 02 00 13 00 26 00 1A 10 00 00 40 41
//0010: 06 10 0B 00 CF 00 CF 00 44 C0 49 01 96 F4 02 00
//0020: 0F 9A 00 00 00 00 00 00 E6 04
// decoding and handling event;
------------------------------------------------------------------*/
UINT8 hlp_audio_vendorCmd_getreq(void* cfm_par_event)
{
	UINT8 result = 0;
	WORD VendorId = 0;
	static VENDOR_RESP* cfm_par_vendor = NULL;
	ENTER_CRITICAL( s_vendorCmd_getreq_lock);
	if(cfm_par_event == NULL) {
		bse_log_error(TAG, "[dsp] [warning][hlp_stream][hlp_audio_vendorCmd_getreq] cfm_part == NULL");
		LEAVE_CRITICAL( s_vendorCmd_getreq_lock);
		return -1;
	}

	cfm_par_vendor = (VENDOR_RESP*)((UINT8*)cfm_par_event);
	bse_log_info(TAG, "[dsp] enter hlp_audio_vendorCmd_getreq  cfm_par_vendor: %p, cfm_par_event: %p", cfm_par_vendor, cfm_par_event);
	hlp_audio_vendorCmd_resp( cfm_par_vendor);
	LEAVE_CRITICAL( s_vendorCmd_getreq_lock);
	return result;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Get the cmd from event, and process the result,this cbk only
// use on send bccmd actively,
// FF 13 C2 01 00 09 00 00 00 5B 50 00 00 5F BA 00 00 00 00 00 00
// decoding and handling event;
------------------------------------------------------------------*/
UINT8 hlp_audio_vendorCmd_resp(void* cfm_par)
{
	UINT8 result = 0;
	WORD VendorId = 0;
	static VENDOR_RESP* cfm_par_vendor = NULL;

	if(cfm_par == NULL) {
		bse_log_error(TAG, "[dsp] [warning][hlp_stream][hlp_audio_vendor_cmd_resp] cfm_part == NULL");
		return -1;
	}
	ENTER_CRITICAL( s_vendorCmd_resp_lock);
	// the param data is not matches;
	if( ((UINT8*)cfm_par)[0] == 0xff)
		cfm_par_vendor = (VENDOR_RESP*)((UINT8*)cfm_par+1);
	else
		cfm_par_vendor = (VENDOR_RESP*)((UINT8*)cfm_par);

	VendorId = cfm_par_vendor->Varid;

	if(VendorId >= DSP_MSG_FROM_OPR && VendorId <= DSP_CREATE_OPERATE_C) {
		result = cfm_par_vendor->Status;
		bse_log_info(TAG, "[dsp][event]hlp_audio_event_cfm timetramp(%d) VendorId(0x%0x) result(0x%0x) param(0x%0x 0x%0x 0x%0x)\r\n",GetCurrTime(),VendorId,result,cfm_par_vendor->payload[0],cfm_par_vendor->payload[1],cfm_par_vendor->payload[2]);
		if (HCI_STATUS_OK != result) {
			hlp_vendor_cmd_resp_error(VendorId, cfm_par_vendor);
			LEAVE_CRITICAL( s_vendorCmd_resp_lock);
			return result;
		}
		switch (VendorId) {
		case DSP_STREAM_GETSNK: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STREAM_GETSNK\r\n",VendorId);
			hlp_get_sink_resp(cfm_par_vendor);
		}
		break;
		case DSP_STREAM_CFG: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STREAM_CFG\r\n",VendorId);
			hlp_sbc_config_resp(cfm_par_vendor);
		}
		break;
		case DSP_STREAM_ALIAS_SYNC: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STREAM_ALIAS_SYNC\r\n",VendorId);
			hlp_sco_alias_sink_resp(cfm_par_vendor);
		}
		break;
		case DSP_SET_SCOCONFIG: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_SET_SCOCONFIG do null\r\n",VendorId);
		}
		break;
		case DSP_STREAM_SYNC_SID: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STREAM_SYNC_SID\r\n",VendorId);
			hlp_sbc_sync_resp(cfm_par_vendor);
		}
		break;
		case DSP_CREATE_OPERATE_C: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- CREATE_OPERATE_C\r\n",VendorId);
			hlp_create_c_resp(cfm_par_vendor);
		}
		break;
		case DSP_OPERATE_MSG: {
				bse_log_info(TAG, "[dsp][event]cfm_par_vendor->payload[0]: (0x%0x), s_dsp_a2dp_manager.sbc_a2dp_opid: 0x%0x--- DSP_OPERATE_MSG\r\n", \
																				        cfm_par_vendor->payload[0], s_dsp_a2dp_manager.sbc_a2dp_opid);
				hlp_operator_msg_resp(cfm_par_vendor);
		}
		break;
		case DSP_STREAM_CONNECT: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STREAM_CONNECT\r\n",VendorId);
			hlp_sbc_connect_resp(cfm_par_vendor);
		}
		break;
		case DSP_STREAM_GETSRC: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STREAM_GETSRC\r\n",VendorId);
			hlp_get_source_resp(cfm_par_vendor);
		}
		break;
		case DSP_CMD_DSP_REQ:/*jump to next case,do the same as it*/
		case DSP_MSG_FROM_OPR: {
			//process MSG_FROM_OPR resp
			hlp_process_HQ_msg(cfm_par_vendor);
		}
		break;
		case DSP_STREAM_TRANS_DISCONN: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STREAM_TRANS_DISCONN\r\n",VendorId);
			hlp_sbc_disconnect_resp(cfm_par_vendor);
		}
		break;
		case DSP_START_OPERATE: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_START_OPERATE\r\n",VendorId);
			hlp_start_operator_resp(cfm_par_vendor);
		}
		break;
		case DSP_STOP_OPERATE: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_STOP_OPERATE\r\n",VendorId);
			hlp_stop_operator_resp(cfm_par_vendor);
		}
		break;
		case DSP_CLOSE_SNK: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_CLOSE_SNK\r\n",VendorId);
			hlp_close_sink_resp(cfm_par_vendor);
		}
		break;
		case DSP_CLOSE_SRC: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_CLOSE_SRC\r\n",VendorId);
			hlp_close_source_resp(cfm_par_vendor);
		}
		break;
		case DSP_RESET_OPERATE: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_RESET_OPERATE do null\r\n",VendorId);
		}
		break;
		case DSP_DESTROY_OPERATE: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_DESTROY_OPERATE\r\n",VendorId);
			hlp_destroy_operate_resp(cfm_par_vendor);
		}
		break;
#ifdef USE_RINGTONE_FASTPIPE
		case DSP_FASTPIPE_ENABLE: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_FASTPIPE_ENABLE do null\r\n",VendorId);
			hlp_fastpipe_enable_resp(cfm_par_vendor);
		}
		break;
		case DSP_FASTPIPE_RESIZE: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_FASTPIPE_RESIZE do null\r\n",VendorId);
			hlp_fastpipe_resize_resp(cfm_par_vendor);
		}
		break;
		case  DSP_FASTPIPE_CREATE: {
			bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_FASTPIPE_CREATE do null\r\n",VendorId);
			hlp_fastpipe_create_resp(cfm_par_vendor);
		}
		break;
#endif
		default:
			break;
		}
	} else if (VendorId == CSR_PSKEY) {
		bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- CSR_PSKEY_OPERATE\r\n",VendorId);
		hlp_csr_pskey_resp(cfm_par_vendor);
	}
	LEAVE_CRITICAL( s_vendorCmd_resp_lock);
	return result;
}

// pio32 confige
/*
ioIndex : pio  index  ;
value: 0~31;
direction: input /output;
value: 1,output   0,input;
*/
void Pio32_Set_Direction(WORD ioIndex, WORD direction)
{
	WORD param[6] = {0};
	UINT32 PIOValue = 0x1;
	PIOValue = PIOValue << ioIndex;
	if ( direction) {
		param[3] |= LOWORD(PIOValue);
		param[2] |= HIWORD(PIOValue);
		param[1] |= LOWORD(PIOValue);
		param[0] |= HIWORD(PIOValue);
	} else {
		param[3] &= LOWORD(PIOValue);
		param[2] &= HIWORD(PIOValue);
		param[1] |= LOWORD(PIOValue);
		param[0] |= HIWORD(PIOValue);
	}
	Dsp_manager_Bccmd(BCCMDN_SETREQ, PIO32_DIRECTION_CONTROL,sizeof(param)/sizeof(WORD),param);
}

/*
ioIndex : pio  index  ;
value: 0~31;
HighOrLow: pull-up /pull-down;
value: 1,pull-up   0,pull-down;
*/
void Pio32_Set_HighLow(WORD ioIndex, WORD HighOrLow)
{
	WORD param[6] = {0};
	UINT32 PIOValue = 0x1;
	UINT32 abPIOValue = 0xFFFF;

	PIOValue = PIOValue << ioIndex;
	abPIOValue &= (~PIOValue);
	if ( !HighOrLow) {
		param[3] &= LOWORD(PIOValue);
		param[2] &= HIWORD(PIOValue);
		param[1] |= LOWORD(PIOValue);
		param[0] |= HIWORD(PIOValue);
	} else {
		param[3] |= LOWORD(PIOValue);
		param[2] |= HIWORD(PIOValue);
		param[1] |= LOWORD(PIOValue);
		param[0] |= HIWORD(PIOValue);
	}
	Dsp_manager_Bccmd(BCCMDN_SETREQ, PIO32_HIGHLOW_CONTROL,sizeof(param)/sizeof(WORD),param);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// <sbc_source_id0> = <sbc_opid> + 0 + 0x2000 OPSRC(sbc_opid)
// <sbc_source_id1> = <sbc_opid> + 1 + 0x2000 OPSRCONE(sbc_opid)
// Max PDU length = Local MTU + 4
// <sbc_sinkid> = <sbc_opid> + 0 + 0xa000 OPSINK(sbc_opid)
//the same as set the enable_sco_stream pskey
-----------------------------------------------------------------*/
int enable_sco_streams( WORD sco_status)
{

	WORD param[1] = {sco_status};

	bse_log_api(TAG, "[dsp][cmd] enable_sco_streams(0x4876)+ param(0x%0x)\r\n",sco_status);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_ENABLE_SCOSTREAM,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stream_get_source   get dsp_src ,no need
//to create operate
//[param] src_type
/*-------------------------------------------*/
int stream_get_source( WORD src_type, WORD opt1, WORD opt2)
{
	WORD param[3] = {0};
	param[0] = src_type;
	param[1] = opt1;
	param[2] = opt2;
	bse_log_api(TAG, "[dsp][cmd]stream_get_source(0x505A)+ param(0x%0x,0x%0x,0x%0x)\r\n",src_type,opt1,opt2);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_GETSRC,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stream_get_sink   get dsp_sink ,no need
//to create operate
//[param]sink_type
/*-------------------------------------------*/
int stream_get_sink( WORD sink_type, WORD opt1, WORD opt2)
{
	WORD param[3] = {0};
	param[0] = sink_type;
	param[1] = opt1;
	param[2] = opt2;
	bse_log_api(TAG, "[dsp][cmd]stream_get_sink(0x505b)+ param(0x%0x,0x%0x,0x%0x)\r\n",param[0],param[1],param[2]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_GETSNK,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stream_close_source   release dsp_src
//[param] src_id
/*-------------------------------------------*/
int stream_close_source( WORD src_id)
{
	WORD param[1] = {0};
	if(src_id == 0)
		return -1;
	param[0] = src_id;
	bse_log_api(TAG, "[dsp][cmd]stream_close_source(0x486b)+ param(0x%0x)\r\n",src_id);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_CLOSE_SRC,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stream_close_sink, echo get_sink
//[param]sink_id
/*-------------------------------------------*/
int stream_close_sink( WORD sink_id)
{
	WORD param[1] = {0};
	param[0] = sink_id;

	bse_log_api(TAG, "[dsp][cmd]stream_close_sink(0x486c)+ param(0x%0x)\r\n",sink_id);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_CLOSE_SNK,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]when get dsp sink/src should config then
//[param]sid
/*-------------------------------------------*/
int stream_configure( WORD sid, WORD key, INT32 key_value)
{
	WORD param[4] = {0};
	param[0] = sid;
	param[1] = key;
	param[2] = (key_value&0xffff0000)>>16;
	param[3] =  key_value&0x0000ffff;

	bse_log_api(TAG, "[dsp][cmd]stream_configure(0x505C)+ param(0x%0x,0x%0x,0x%0x,0x%0x)\r\n",param[0],param[1] ,param[2] ,param[3]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_CFG,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stream_sync_sid sync codec or internal sids
//else will generate offset between two sids
//[param]sid
/*-------------------------------------------*/
int stream_sync_sid( WORD sid1, WORD sid2)
{
	WORD param[2] = {0};
	param[0] = sid1;
	param[1] = sid2;

	bse_log_api(TAG, "[dsp][cmd]stream_sync_sid(0x5062)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_SYNC_SID,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stream_alias_sink,if use two output should
//use this to alias and then sync
//[param]sid
/*-------------------------------------------*/
int stream_alias_sink( WORD sinkid1, WORD sinkid2)
{
	WORD param[2] = {0};
	param[0] = sinkid1;
	param[1] = sinkid2;

	bse_log_api(TAG, "[dsp][cmd] stream_alias_sink(0x505D)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_ALIAS_SYNC,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]create_operator_c,before use dsp,should
//create corresponding operator
//[param]sid
/*-------------------------------------------*/
int create_operator_c( WORD capability_id, WORD item_count, WORD skip_count)
{
	WORD param[3] = {0};
	param[0] = capability_id;
	param[1] = item_count;
	param[2] = skip_count;
	bse_log_api(TAG, "[dsp][cmd]create_operator_c(0x5075)+ param(0x%0x,0x%,0x%0x)\r\n",capability_id,item_count,skip_count);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_CREATE_OPERATE_C,sizeof(param)/sizeof(WORD),param);
	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug],get_chip_version, used to get the version
//of chip.
//[param]
/*-------------------------------------------*/
int get_chip_version(void)
{
	WORD param[3] = {0};
	param[0] = 0;
	param[1] = 0;
	param[2] = 0;
	bse_log_api(TAG, "[dsp][cmd]get_chip_version(0x281a)");
	Dsp_manager_Bccmd(BCCMD_GETREQ, DSP_GET_CHIP_VERSION, sizeof(param)/sizeof(WORD),param);
	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]operator_message
//[param]sid
/*-------------------------------------------*/
static int operator_message_cvc( WORD sbc_opid, WORD message_id, WORD *message)
{
	WORD param[8] = {0};
	int i = 0;
	param[0] = sbc_opid;

	if( message_id == CVC_VOLUME) {
		param[1] = message_id;
		param[4] = message[0];
	} else if (message_id == CVC_SET_MODE) {
		param[1] = message_id;
		param[2] = message[0];
		param[3] = 1;
		param[4] = 3;
	} else if (message_id == CVC_GET_PARAM) {
		param[1] = message_id;
		param[2] = 257;
	} else if (message_id == CVC_SET_PARAM) {
		param[1] = message_id;
		param[2] = 257;
		param[3] = 0;
		param[4] = 0x0080;
		param[5] = 1;
	} else {
		param[1] = message_id;
		param[2] = message[0];
	}
	bse_log_api(TAG, "[dsp][cmd] operator_message_cvc(0x5074)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_OPERATE_MSG,sizeof(param)/sizeof(WORD),param);
	return 0;
}

static int operator_message_audio( WORD sbc_opid, WORD message_id, WORD *message)
{
	WORD param[8] = {0};
	int i = 0;
	param[0] = sbc_opid;

	bse_log_info(TAG, "[dsp] operator_message_audio message_id: %x", message_id);

	if ( message_id == SET_RINGTONE_MUSIC) {
		for (i = 0; i<6; i++) {
			param[i+1] = message[i];
		}
	} else if( message_id == SET_TONE_VOLUME) {
		param[1] = message_id;
		param[2] = message[0];
		//param[3] = message[1];
	} else if( message_id == SET_SBC_PARAMS_MANUL) {
		param[1] = 1;//message_id;
		param[2] = message[0];
	} else {
		param[1] = message_id;
		param[2] = message[0];
	}
	bse_log_api(TAG, "[dsp][cmd] operator_message_audio(0x5074)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_OPERATE_MSG,sizeof(param)/sizeof(WORD),param);
	return 0;
}


/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stream_connect  stream_transform_disconnect
//after config and sync,should connect src with sink,
//[param]sid
/*-------------------------------------------*/
int stream_connect( WORD sbc_srcId, WORD sbc_sinkId)
{
	WORD param[2] = {0};
	param[0] = sbc_srcId;
	param[1] = sbc_sinkId;
	bse_log_api(TAG, "[dsp][cmd]stream_connect(0x505e)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_CONNECT,sizeof(param)/sizeof(WORD),param);
	return 0;
}
int stream_transform_disconnect( WORD tid)
{
	WORD param[1] = {0};
	param[0] = tid;
	bse_log_api(TAG, "[dsp][cmd] stream_transform_disconnect(0x486d)+ param(0x%0x)\r\n",tid);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_TRANS_DISCONN,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]start_operators
//this used to start streams
//[param]sid
/*-------------------------------------------*/
int start_operators( WORD count, WORD *list_ids)
{
	UINT8 stream_type = hlp_stream_get_type();
	WORD param[5] = {0};
	//bse_log_api(TAG, " start_operators+\r\n");

	param[0] = count;
	param[1] = list_ids[0];
	if (param[1] == s_dsp_sco_manager.sbc_sco_send_opid) {
		param[2] = list_ids[1];
		param[3] = list_ids[2];
		param[4] = list_ids[3];
	} else {
	}
	bse_log_api(TAG, "[dsp][cmd]start_operators(0x5070)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_START_OPERATE,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]stop_operators
//this used to stop streams
//[param]sid
/*-------------------------------------------*/
int stop_operators( WORD count, WORD *list_ids)
{
	WORD param[5] = {0};
	UINT8 stream_type = hlp_stream_get_type();
	bse_log_api(TAG, "[dsp][cmd]stop_operators(0x5071)+ streamtype(%d)\r\n",stream_type);

	param[0] = count;
	param[1] = list_ids[0];
	if(param[1] == s_dsp_a2dp_manager.sbc_a2dp_opid) {
		bse_log_info(TAG, "[dsp]stopoperate a2dp_list(0x%0x \r\n",param[1]);
	} else if(param[1] == s_dsp_sco_manager.sbc_sco_send_opid) {
		param[2] = list_ids[1];
		param[3] = list_ids[2];
		param[4] = list_ids[3];
		bse_log_info(TAG, "[dsp]stopoperate sco_list(0x%0x 0x%0x 0x%0x 0x%0x)\r\n",param[1],param[2],param[3],param[4]);
	} else if(param[1] == s_dsp_ringtone_manager.sbc_ringtone_opid) {
		bse_log_info(TAG, "[dsp]stopoperate ring_list(0x%0x) \r\n",param[1]);
	} else {
	}
	bse_log_info(TAG, "[dsp][cmd]stop_operators(0x5071)+ param(0x%0x,0x%0x)\r\n",param[0],param[1]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STOP_OPERATE,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++++
[argu]destroy_operators
----------------------------------------------*/
int destroy_operators(WORD count, WORD* list_ids)
{
	WORD param[5] = {0};
	UINT8 stream_type = hlp_stream_get_type();
	param[0] = count;
	param[1] = list_ids[0];

	//bse_log_api(TAG, "[dsp][cmd]destroy_operators(0x5073)+ stream_type(%d)\r\n",stream_type);
	if( list_ids[0] == 0x0000) {
		bse_log_error(TAG, "[dsp][cmd]destroy_operators(0x5073)+ opid is NULL param(0x%0x,0x%0x) stream_type(%d)\r\n",param[0],param[1],stream_type);
		return 1;
	}
	
	if (param[1] == s_dsp_sco_manager.sbc_sco_send_opid) {
		param[2] = list_ids[1];
		param[3] = list_ids[2];
		param[4] = list_ids[3];
	} else {
		
	}
	bse_log_info(TAG, "[dsp][cmd]destroy_operators(0x5073)+ param(0x%0x,0x%0x) stream_type(%d)\r\n",param[0],param[1],stream_type);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_DESTROY_OPERATE,sizeof(param)/sizeof(WORD),param);
	return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++++
[argu]reset_operators,
----------------------------------------------*/
int reset_operators( WORD count, WORD *list_ids)
{
	WORD param[5] = {0};
	UINT8 stream_type = hlp_stream_get_type();
	param[0] = count;
	param[1] = list_ids[0];

	if(stream_type == STREAM_A2DP) {
	} else if( stream_type == STREAM_SCO) {
		param[2] = list_ids[1];
		param[3] = list_ids[2];
		param[4] = list_ids[3];
	} else if( stream_type == STREAM_RINGTONE) {

	} else {

	}
	bse_log_api(TAG, "[dsp][cmd] reset_operators(0x5072)+ stream_type(%d)\r\n",stream_type);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_RESET_OPERATE,sizeof(param)/sizeof(WORD),param);
	return 0;
}

int stream_external_detach( WORD sid)
{
	WORD param[1] = {0};
	param[0] = sid;
	bse_log_api(TAG, "[dsp][cmd]stream_external_detach(0x487F)+ param(0x%0x)\r\n",sid);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_STREAM_EXTERNAL_DETACH,sizeof(param)/sizeof(WORD),param);
	return 0;
}

// dsp fastpipe confige;
int fastpipe_enable(WORD buffer_lenth)
{
	WORD param[3] = {0};
	param[1] = buffer_lenth;
	bse_log_api(TAG, "[dsp][cmd]fastpipe_enable(0x5041)+ param(0x%0x)\r\n",buffer_lenth);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_FASTPIPE_ENABLE, sizeof(param)/sizeof(WORD),param);
	return 0;
}

int fastpipe_resize(WORD buffer_lenth)
{
	WORD param[3] = {0};
	param[1] = buffer_lenth;
	bse_log_api(TAG, "[dsp][cmd]fastpipe_resize(0x5057)+ param(0x%0x)\r\n",buffer_lenth);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_FASTPIPE_RESIZE, sizeof(param)/sizeof(WORD),param);
	return 0;
}

int fastpipe_create(WORD *arg_list)
{
	WORD param[7] = {0};
	memcpy(param,arg_list,7);
	bse_log_api(TAG, "[dsp][cmd]fastpipe_create(0x5042)+ param(0x%0x,0x%0x,0x%0x,0x%0x,0x%0x,0x%0x,0x%0x)\r\n"
	            ,param[0],param[1],param[2],param[3],param[4],param[5],param[6]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_FASTPIPE_CREATE, sizeof(param)/sizeof(WORD),param);
	return 0;
}


/*+++++++++++++++++++++++++++++++++++++++++++++
[argu]dsp manager config, 25 patch
----------------------------------------------*/
int dspmanager_config_request( )
{
	UINT16 ocf = 0;
	UINT8 param_len = 0;
	UINT8 buftmp[256] = {0};
	static DWORD time_trampirst =0;
	static DWORD time_tramppre  =0;

	if (s_patch_index == 0) {
		time_trampirst = GetCurrTime();
		time_tramppre = time_trampirst;
	} else if (s_patch_index > 0) {

		bse_log_info(TAG, "[dsp]patch index(%d),interval(%d)",s_patch_index,GetCurrTime()-time_tramppre);
		time_tramppre = GetCurrTime();
	}
	if(s_patch_index >=25) {
		bse_log_info(TAG, "[dsp]have patched!!\r\n");
		return -1;
	}
	memset(buftmp,0,256);
	memcpy(buftmp,s_patch_data[s_patch_index],228);
	param_len = getbit(buftmp,4,3)*2+1;
	memset(buftmp+param_len,0,256-param_len);
	hlp_gap_excute_vendor_command(0,ocf,buftmp,param_len);
	s_patch_index ++;
	if (s_patch_index == 25) {
		bse_log_info(TAG, "[dsp]patchend index(%d),costtime(%d)",s_patch_index,abs(GetCurrTime()-time_trampirst));
	}
	return 0;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Get the cmd from event, and process the result
//0000: 04 FF// 27 C3 02 00 13 00 26 00 1A 10 00 00 40 41
//0010: 06 10 0B 00 CF 00 CF 00 44 C0 49 01 96 F4 02 00
//0020: 0F 9A 00 00 00 00 00 00 E6 04
// decoding and handling event;*/
int message_from_operator( void* param)
{
	UINT16 ocf = 0;
	UINT8 param_len = 0;
	UINT8 buftmp[256];

	UINT8 *z = (UINT8 *)param;

	param_len = getbit(z,5,4)*2+1;
	memset( buftmp, 0, 256);
	memcpy(buftmp, param, param_len+1);
	buftmp[2] = 0x01;

	hlp_gap_excute_vendor_command(0,ocf,buftmp+1,param_len);
	return 0;
}

WORD getbit(void * param, int nIndex, int nIndex2)
{
	WORD uHigh = (((UINT8*)param)[nIndex])<<8;
	WORD uLow = ((UINT8*)param)[nIndex2];
	return uHigh+uLow;
}
/*+++++++++++++++++++++++++++++++++++++++++++++
[argu]internal use for dsptest
----------------------------------------------*/
void hlp_set_sco_config()
{
	WORD param[1] = {0};
	param[0] = s_dsp_sco_manager.sco_hdl;
	bse_log_api(TAG, "[dsp][cmd]hlp_set_sco_config(0x304d) param(0x%0x)+\r\n",s_dsp_sco_manager.sco_hdl);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, DSP_SET_SCOCONFIG,sizeof(param)/sizeof(WORD),param);
}
/*+++++++++++++++++++++++++++++++++++++++++++++
[argu]operate response
----------------------------------------------*/
void hlp_get_sink_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;

	// 解析出当前的sink 通道;
	WORD sink_index = getbit( cfm_par->payload, 5, 4);
	// 解析出当前的sink ID,根据通道不同进行保存;
	WORD sink_id = cfm_par->payload[0];
	UINT16 wSeqNum = cfm_par->Seqno;

//a2dp
	if (wSeqNum == s_dsp_a2dp_manager.a2dp_getsink1_SeqNum) {
		int a2dpsnk_sample_rate = 0;

		s_dsp_a2dp_manager.sbc_a2dp_sink1 = sink_id;
		hlp_GetA2DPStream_AptxMode(&a2dpsnk_sample_rate);
#ifndef A2DP_SNK_I2S_OUTPUT
			stream_configure(s_dsp_a2dp_manager.sbc_a2dp_sink1,CODEC_OUTPUT_SAMPLE_RATE, a2dpsnk_sample_rate);
			stream_configure(s_dsp_a2dp_manager.sbc_a2dp_sink1,CODEC_OUTPUT_GAIN, SET_SPKA2DPGAIN_VOLUME_DEFAULT);
#else 
			//config the IIS ,need keep the same with mster/slave
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_MASTER_CLOCK_RATE, 0);			// bit clk
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_MASTER_MODE, 0);				// sync freq;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_SYNC_RATE, SAMPLE_RATE_44100);	// sample rate;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_JUSTIFY_FORMAT, 0);			// 0 left , 1 right;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_LEFT_JUSTIFY_DELAY, 1);		// 0 first clk cycle, 1,second clk cycle;
/* 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_CHANNEL_POLARITY, 1);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_AUDIO_ATTENUATION_ENABLE, 0);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_AUDIO_ATTENUATION, 0);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_JUSTIFY_RESOLUTION, 0);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_CROP_ENABLE, 1);
*/
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink1, I2S_BITS_PER_SAMPLE, 0x10);
#endif
	} else if(wSeqNum == s_dsp_a2dp_manager.a2dp_getsink2_SeqNum) {
		int a2dpsnk_sample_rate = 0;

		s_dsp_a2dp_manager.sbc_a2dp_sink2  = sink_id;
		hlp_GetA2DPStream_AptxMode(&a2dpsnk_sample_rate);
#ifndef A2DP_SNK_I2S_OUTPUT
			stream_configure(s_dsp_a2dp_manager.sbc_a2dp_sink2,CODEC_OUTPUT_SAMPLE_RATE, a2dpsnk_sample_rate);
			stream_configure(s_dsp_a2dp_manager.sbc_a2dp_sink2,CODEC_OUTPUT_GAIN, SET_SPKA2DPGAIN_VOLUME_DEFAULT);
#else
			//config the IIS ,need keep the same with mster/slave
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_MASTER_CLOCK_RATE, 0);			// bit clk
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_MASTER_MODE, 0);				// sync freq;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_SYNC_RATE, SAMPLE_RATE_44100);	// sample rate;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_JUSTIFY_FORMAT, 0);			// 0 left , 1 right;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_LEFT_JUSTIFY_DELAY, 1);		// 0 first clk cycle, 1,second clk cycle;
/* 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_CHANNEL_POLARITY, 1);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_AUDIO_ATTENUATION_ENABLE, 0);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_AUDIO_ATTENUATION, 0);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_JUSTIFY_RESOLUTION, 0);
 			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_CROP_ENABLE, 1);
*/
			s_dsp_a2dp_manager.a2dp_confige_SeqNum = s_wSeqNum;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_sink2, I2S_BITS_PER_SAMPLE, 0x10);
#endif
	}
//sco
	else if( wSeqNum == s_dsp_sco_manager.get_sco_sink_SeqNum) {
		s_dsp_sco_manager.sbc_sco_sinkid = sink_id;
		bse_log_info(TAG, "[dsp]sid getsink s_dsp_sco_manager.sbc_sco_sinkid(0x%0x)\r\n",s_dsp_sco_manager.sbc_sco_sinkid);
		s_dsp_sco_manager.get_codec_src_SeqNum = s_wSeqNum;
        if (s_use_mica) { //use micA or micB will be control by APP when sco connected!
            stream_get_source( CODEC_TYPE, 0x0000, 0x0000);
        } else {
            stream_get_source( CODEC_TYPE, 0x0000, 0x0001);
        }
#ifdef SCO_USE_2MIC
		s_dsp_sco_manager.get_codec_src2_SeqNum = s_wSeqNum;
        if (s_use_mica) {
		    stream_get_source( CODEC_TYPE, 0x0000, 0x0001);
        } else {
		    stream_get_source( CODEC_TYPE, 0x0000, 0x0000);
        }
#endif
		s_dsp_sco_manager.get_codec_sink1_SeqNum = s_wSeqNum;
		stream_get_sink( CODEC_TYPE, 0x0000, 0x0000);
		s_dsp_sco_manager.get_codec_sink2_SeqNum = s_wSeqNum;
		stream_get_sink( CODEC_TYPE, 0x0000, 0x0001);
	} else if ( wSeqNum == s_dsp_sco_manager.get_codec_sink1_SeqNum) {
		s_dsp_sco_manager.sbc_codec_sinkid1 = sink_id;
		bse_log_info(TAG, "[dsp]sid getsink s_dsp_sco_manager.s_dsp_sco_manager.sbc_codec_sinkid1 (0x%0x)\r\n",s_dsp_sco_manager.sbc_codec_sinkid1);

		if (get_codec_type() == HFP_CODECID_MSBC) {
			stream_configure( s_dsp_sco_manager.sbc_codec_sinkid1, CODEC_OUTPUT_SAMPLE_RATE, SAMPLE_RATE_16000);
		} else {
			stream_configure( s_dsp_sco_manager.sbc_codec_sinkid1, CODEC_OUTPUT_SAMPLE_RATE, SAMPLE_RATE_8000);
		}
		stream_configure( s_dsp_sco_manager.sbc_codec_sinkid1, CODEC_OUTPUT_GAIN, s_dsp_sco_manager.sbc_sco_spk_gain);
		s_nDAC_Gain = s_dsp_sco_manager.sbc_sco_spk_gain;
	} else if ( wSeqNum == s_dsp_sco_manager.get_codec_sink2_SeqNum) {
		s_dsp_sco_manager.sbc_codec_sinkid2 = sink_id;
		bse_log_info(TAG, "[dsp]sid getsink s_dsp_sco_manager.sbc_codec_sinkid2 (0x%0x)\r\n",s_dsp_sco_manager.sbc_codec_sinkid2);

		if (get_codec_type() == HFP_CODECID_MSBC) {
			stream_configure( s_dsp_sco_manager.sbc_codec_sinkid2, CODEC_OUTPUT_SAMPLE_RATE, SAMPLE_RATE_16000);
		} else {
			stream_configure( s_dsp_sco_manager.sbc_codec_sinkid2, CODEC_OUTPUT_SAMPLE_RATE, SAMPLE_RATE_8000);
		}

		stream_configure( s_dsp_sco_manager.sbc_codec_sinkid2, CODEC_OUTPUT_GAIN, s_dsp_sco_manager.sbc_sco_spk_gain);
	}
//ringtone
	else if(s_dsp_ringtone_manager.sink1_ringtone_SeqNum == wSeqNum) {
		s_dsp_ringtone_manager.sbc_ringtone_sinkid1 = sink_id;
		stream_configure( s_dsp_ringtone_manager.sbc_ringtone_sinkid1, CODEC_OUTPUT_SAMPLE_RATE, SAMPLE_RATE_44100);//SAMPLE_RATE_8000;SAMPLE_RATE_44100
		stream_configure( s_dsp_ringtone_manager.sbc_ringtone_sinkid1, CODEC_OUTPUT_GAIN, SET_SPKRINGGAIN_VOLUME_DEFAULT);
	} else if (s_dsp_ringtone_manager.sink2_ringtone_SeqNum == wSeqNum) {
		s_dsp_ringtone_manager.sbc_ringtone_sinkid2 = sink_id;
		stream_configure( s_dsp_ringtone_manager.sbc_ringtone_sinkid2, CODEC_OUTPUT_SAMPLE_RATE, SAMPLE_RATE_44100);
		stream_configure( s_dsp_ringtone_manager.sbc_ringtone_sinkid2, CODEC_OUTPUT_GAIN, SET_SPKRINGGAIN_VOLUME_DEFAULT);
	}
}
void hlp_sbc_config_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	WORD sid = cfm_par->payload[0];
	if (cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		//a2dp_src config does not rely on streamtype
		if(sid == s_dsp_a2dp_manager.sbc_a2dp_src_id) {
			int nbaud = 0;
			//Get a2dp_operate_id,and enable fadeout
			s_dsp_a2dp_manager.a2dp_create_sbcoper_SeqNum = s_wSeqNum;
			if (hlp_GetA2DPStream_AptxMode(&nbaud)) {
				create_operator_c( APTX_DECODEC_CAP, 0x0000, 0x0000);
			} else {
				create_operator_c( SBC_DECODEC_CAP, 0x0000, 0x0000);
			}
		} else if (sid == s_dsp_a2dp_manager.sbc_a2dp_sink2) {
			//start to sync codec sink
			if((s_dsp_a2dp_manager.sbc_a2dp_sink1 && s_dsp_a2dp_manager.sbc_a2dp_sink2)&& (cfm_par->payload[1] == CODEC_OUTPUT_GAIN))
				stream_sync_sid(s_dsp_a2dp_manager.sbc_a2dp_sink1, s_dsp_a2dp_manager.sbc_a2dp_sink2);
		} else if ((sid == s_dsp_sco_manager.sbc_codec_sinkid2)&& (cfm_par->payload[1] == CODEC_OUTPUT_GAIN) &&( s_dsp_sco_manager.change_sco_sink_SeqNum != cfm_par->Seqno)) {
			bse_log_info(TAG, "[dsp]sco stream_sync_sid conf_resp\r\n");
			stream_alias_sink( s_dsp_sco_manager.sbc_codec_sinkid1,s_dsp_sco_manager.sbc_codec_sinkid2);
		} else if ((sid == s_dsp_ringtone_manager.sbc_ringtone_sinkid2)&&(getbit( cfm_par->payload, 3, 2) == CODEC_OUTPUT_GAIN)) {
			if( s_dsp_ringtone_manager.sbc_ringtone_sinkid1 && s_dsp_ringtone_manager.sbc_ringtone_sinkid2) {
				bse_log_info(TAG, "[dsp]stream_alias_sink_ringtone ,sink_id I(0x%0x) II(0x%0x)\r\n",s_dsp_ringtone_manager.sbc_ringtone_sinkid1,s_dsp_ringtone_manager.sbc_ringtone_sinkid2);
				stream_alias_sink( s_dsp_ringtone_manager.sbc_ringtone_sinkid1,s_dsp_ringtone_manager.sbc_ringtone_sinkid2);
			}
		} else {

		}
	}
}

void hlp_sbc_sync_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	bse_log_info(TAG, "[dsp] enter hlp_sbc_sync_resp status: %d", cfm_par->Status);
	if (cfm_par->Status == HCI_STATUS_OK) {
		if(cfm_par->payload[0] == OPSRC(s_dsp_a2dp_manager.sbc_a2dp_opid)) {
			//bse_log_api(TAG, "[dsp]a2dp sync suc\r\n");
		} else if((cfm_par->payload[0] == s_dsp_sco_manager.sbc_codec_sinkid1) && (cfm_par->payload[1] == s_dsp_sco_manager.sbc_codec_sinkid2)) {
			bse_log_info(TAG, "[dsp]sco codec sync suc\r\n");
			s_dsp_sco_manager.sbc_connect_sco_src_SeqNum = s_wSeqNum;
			stream_connect( s_dsp_sco_manager.sbc_sco_srcid, OPSINK(s_dsp_sco_manager.sbc_sco_receive_opid));

		} else if ((cfm_par->payload[0] == s_dsp_a2dp_manager.sbc_a2dp_sink1)&&(cfm_par->payload[1] == s_dsp_a2dp_manager.sbc_a2dp_sink2)) {
			WORD param = 0x0000;

			operator_message_audio( s_dsp_a2dp_manager.sbc_a2dp_opid, AUDIO_DISABLE_FADE_OUT, &param);
			stream_connect( s_dsp_a2dp_manager.sbc_a2dp_src_id, OPSINK(s_dsp_a2dp_manager.sbc_a2dp_opid));
			stream_connect( OPSRCONE(s_dsp_a2dp_manager.sbc_a2dp_opid), s_dsp_a2dp_manager.sbc_a2dp_sink2);
			stream_connect( OPSRC(s_dsp_a2dp_manager.sbc_a2dp_opid), s_dsp_a2dp_manager.sbc_a2dp_sink1);
		} else if ((cfm_par->payload[0] == s_dsp_ringtone_manager.sbc_ringtone_sinkid1)&&(cfm_par->payload[1] == s_dsp_ringtone_manager.sbc_ringtone_sinkid2)) {
			WORD param[7] =  {0x0000,0x03e8,0x0020,0xac44,0x07d0,0x57c0,0x0001};
			stream_connect( OPSRC(s_dsp_ringtone_manager.sbc_ringtone_opid), s_dsp_ringtone_manager.sbc_ringtone_sinkid1);
			operator_message_audio( s_dsp_ringtone_manager.sbc_ringtone_opid, SET_RINGTONE_MUSIC, param);
			memset( param, 0, sizeof(param));
			operator_message_audio( s_dsp_ringtone_manager.sbc_ringtone_opid, RINGTONE_DISABLE_FADE_OUT, param);
			param[0] = 0x1f;
			param[1] = 0xffff;
			operator_message_audio( s_dsp_ringtone_manager.sbc_ringtone_opid, SET_TONE_VOLUME, param);
		}
	} else {
		bse_log_info(TAG, "[dsp] s_dsp_a2dp_manager.sbc_a2dp_opid(0x%04x) s_dsp_a2dp_manager.sbc_a2dp_src_id(0x%04x) s_dsp_a2dp_manager.sbc_a2dp_sink1(0x%04x) s_dsp_a2dp_manager.sbc_a2dp_sink2(0x%04x)\r\n",
		             s_dsp_a2dp_manager.sbc_a2dp_opid,s_dsp_a2dp_manager.sbc_a2dp_src_id, s_dsp_a2dp_manager.sbc_a2dp_sink1, s_dsp_a2dp_manager.sbc_a2dp_sink2);
	}
}

void hlp_create_c_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if (cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		bse_log_info(TAG, "[dsp][event]create_c_oper_resp ,stream_type(%d), Seqno(0x%02x), SeqNum(0x%02x)\r\n",stream_type,
		             cfm_par->Seqno, s_dsp_a2dp_manager.a2dp_create_sbcoper_SeqNum);
		//a2dp operate does not rely on streamtype
		if (cfm_par->Seqno == s_dsp_a2dp_manager.a2dp_create_sbcoper_SeqNum) {
			//create sbc operator suc
			WORD param = 0x0000;
			s_dsp_a2dp_manager.sbc_a2dp_opid = cfm_par->payload[0];

			s_startstream_Init_tramp = GetCurrTime();
			// get sink 0 stream;
			s_dsp_a2dp_manager.a2dp_getsink1_SeqNum = s_wSeqNum;
#ifndef A2DP_SNK_I2S_OUTPUT
			stream_get_sink(CODEC_TYPE,0,0);
#else
			stream_get_sink(IIS_TYPE, PCM2, 0); //PCM2, left. PCM1 used for SPI so choice PCM2
#endif
			// get sink 1 stream;
			s_dsp_a2dp_manager.a2dp_getsink2_SeqNum = s_wSeqNum;
#ifndef A2DP_SNK_I2S_OUTPUT
			stream_get_sink(CODEC_TYPE,0,1);
#else
			stream_get_sink(IIS_TYPE, PCM2, 1); //PCM2, right. PCM1 used for SPI so choice PCM2
#endif
		}
		WORD wSeqNum = cfm_par->Seqno;
		WORD opid = cfm_par->payload[0];
		bse_log_info(TAG, "[dsp][event][create] opid: 0x%04x", opid);
		if ( wSeqNum == s_dsp_sco_manager.create_scosend_SeqNum) {
			s_dsp_sco_manager.sbc_sco_send_opid = opid;
		} else if( wSeqNum == s_dsp_sco_manager.create_scorecv_SeqNum) {
			s_dsp_sco_manager.sbc_sco_receive_opid = opid;
			//with out cvc,do the process
#ifndef CONFIG_CVC_OPERATE
			s_dsp_sco_manager.get_sco_src_SeqNum = s_wSeqNum;
			stream_get_source( SCO_TYPE, s_dsp_sco_manager.sco_hdl, 0x0000);
			s_dsp_sco_manager.get_sco_sink_SeqNum = s_wSeqNum;
			stream_get_sink( SCO_TYPE, s_dsp_sco_manager.sco_hdl, 0x0000);
			s_dsp_sco_manager.get_codec_src_SeqNum = s_wSeqNum;
			stream_get_source( CODEC_TYPE, 0x0000, 0x0000);
			s_dsp_sco_manager.get_codec_sink1_SeqNum = s_wSeqNum;
			stream_get_sink( CODEC_TYPE, 0x0000, 0x0000);
			s_dsp_sco_manager.get_codec_sink2_SeqNum = s_wSeqNum;
			stream_get_sink( CODEC_TYPE, 0x0000, 0x0001);
#endif
		} else if( wSeqNum == s_dsp_sco_manager.create_cvchfmic_send_SeqNum) {
#ifndef SCO_USE_2MIC
			s_dsp_sco_manager.sbc_cvchf1mic_send_opid = opid;
#else
			s_dsp_sco_manager.sbc_cvchf2mic_send_opid = opid;
#endif
		} else if( wSeqNum == s_dsp_sco_manager.create_cvchf_receive_SeqNum) {
			//use cvc this operator is valible
			s_dsp_sco_manager.sbc_cvchf_receive_opid = opid;
			s_dsp_sco_manager.get_sco_src_SeqNum = s_wSeqNum;
			stream_get_source( SCO_TYPE, s_dsp_sco_manager.sco_hdl, 0x0000);
		} else if (wSeqNum == s_dsp_ringtone_manager.create_tingtone_SeqNum ) {
			s_dsp_ringtone_manager.sbc_ringtone_opid = cfm_par->payload[0];
			bse_log_info(TAG, "[dsp]ringtone createoperate(0x%0x)\r\n",s_dsp_ringtone_manager.sbc_ringtone_opid);
			s_dsp_ringtone_manager.sink1_ringtone_SeqNum = s_wSeqNum;
			stream_get_sink( CODEC_TYPE, 0x0000, 0x0000);
			s_dsp_ringtone_manager.sink2_ringtone_SeqNum = s_wSeqNum;
			stream_get_sink( CODEC_TYPE, 0x0000, 0x0001);
#ifdef USE_RINGTONE_FASTPIPE
			stream_get_source( FASTPIPE_TYPE, 0x0001, 0x0000);
#endif
		} else {
			bse_log_info(TAG, "[dsp][warning]createoperate suc,but in null state!!! type(%d)\r\n",stream_type);
			/*WORD list_ids[2] = {0};
			list_ids[0] = cfm_par->payload[0];
			stop_operators( 0x0001, list_ids);
			destroy_operators( 0x0001, list_ids);*/
		}
	}
}

void hlp_operator_msg_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if (cfm_par->Status == HCI_STATUS_OK) {
		int msgId = getbit( cfm_par, 15, 14);
		UINT8 stream_type = hlp_stream_get_type();

		UINT16 wSeqNum = cfm_par->Seqno;
		if(wSeqNum == s_dsp_a2dp_manager.a2dp_tostop_SeqNum) {
			//wait for fadeout complete timer
		} else if(wSeqNum == s_dsp_sco_manager.sco_opermsg_tostop_SeqNum) {
			//wait for fadeout complete timer
			//UART_Printf("[dsp][sco]timer to stop sco \r\n");
			//FsmAddTimer(NULL, 150, 0, NULL, NULL, stream_sco_stop_timer);
		} else if(wSeqNum == s_dsp_ringtone_manager.ringtone_stop_SeqNum) {
			if (msgId == RINGTONE_ENABLE_FADE_OUT) {
				//wait for fadeout complete timer
				bse_log_info(TAG, "[dsp]timer to  stop ringtone \r\n");
			//	FsmAddTimer(NULL, 150, 0, NULL, NULL, stream_ringtone_stop_timer); //We should wait ringtone fadeout complete event, don`t use timer to stop dsp.
			}
		} else {

		}
	}
}

void hlp_sbc_connect_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	// 确保connect 成功后,保存当前连接的tid;
	if (cfm_par->Status == HCI_STATUS_OK) {
		WORD tid = getbit( cfm_par, 15, 14);//payload[1]
		UINT8 stream_type = hlp_stream_get_type();
		int sbc_tid = cfm_par->payload[0];
		WORD list_id[1] = {0};

		//generator tid about a2dp
		if (tid == s_dsp_a2dp_manager.sbc_a2dp_sink1) {
			s_dsp_a2dp_manager.sbc_a2dp_tid1 = cfm_par->payload[0];
			bse_log_info(TAG, "[dsp]a2dp_audio_connect cmd s_dsp_a2dp_manager.sbc_a2dp_sink1(0x%04x) s_dsp_a2dp_manager.sbc_a2dp_tid1(0x%04x)",s_dsp_a2dp_manager.sbc_a2dp_sink1,s_dsp_a2dp_manager.sbc_a2dp_tid1);
			start_operators(0x0001,&s_dsp_a2dp_manager.sbc_a2dp_opid);
		} else if(tid == s_dsp_a2dp_manager.sbc_a2dp_sink2) {
			s_dsp_a2dp_manager.sbc_a2dp_tid2 = cfm_par->payload[0];
			bse_log_info(TAG, "[dsp]a2dp_audio_connect cmd s_dsp_a2dp_manager.sbc_a2dp_sink2(0x%04x) s_dsp_a2dp_manager.sbc_a2dp_tid2(0x%04x)",s_dsp_a2dp_manager.sbc_a2dp_sink2,s_dsp_a2dp_manager.sbc_a2dp_tid2);
		} else if(tid == OPSINK(s_dsp_a2dp_manager.sbc_a2dp_opid)) {
			s_dsp_a2dp_manager.sbc_a2dp_src_tid = cfm_par->payload[0];
		} else if (tid == s_dsp_sco_manager.sbc_codec_sinkid1) {
			WORD param = 0x0000;
			WORD list_id[2] = {0};
			s_dsp_sco_manager.sbc_codec_sink_tid = sbc_tid;
#ifdef CONFIG_CVC_OPERATE
			operator_message_cvc( s_dsp_sco_manager.sbc_cvchf_receive_opid, CVC_DISABLE_FADE_OUT, &param);
			operator_message_cvc( s_dsp_sco_manager.sbc_cvchf1mic_send_opid, CVC_DISABLE_FADE_OUT, &param);
#ifdef SCO_USE_2MIC
				operator_message_cvc( s_dsp_sco_manager.sbc_cvchf2mic_send_opid, CVC_DISABLE_FADE_OUT, &param);
#endif

#else
			operator_message_audio(s_dsp_sco_manager.sbc_sco_send_opid, AUDIO_DISABLE_FADE_OUT, &param);
			operator_message_audio(s_dsp_sco_manager.sbc_sco_receive_opid, AUDIO_DISABLE_FADE_OUT, &param);
			list_id[0] = s_dsp_sco_manager.sbc_sco_send_opid;
			list_id[1] = s_dsp_sco_manager.sbc_sco_receive_opid;
				start_operators( 0x0002, list_id);
#endif
		}
#ifndef SCO_USE_2MIC
		else if( tid == OPSINK(s_dsp_sco_manager.sbc_cvchf1mic_send_opid)) {
			s_dsp_sco_manager.sbc_cvc1hfmic_send_tid = sbc_tid;
		}
#else
		else if( tid == OPSINK(s_dsp_sco_manager.sbc_cvchf2mic_send_opid)) {
			s_dsp_sco_manager.sbc_cvc2hfmic_send_tid = sbc_tid;
		} else if(tid == OPSINKONE(s_dsp_sco_manager.sbc_cvchf2mic_send_opid)) {
			s_dsp_sco_manager.sbc_cvc2hfmic_send_tid2 = sbc_tid;
		}
#endif
		else if( tid == OPSINK(s_dsp_sco_manager.sbc_sco_receive_opid)) {
			s_dsp_sco_manager.sbc_sco_receive_tid = sbc_tid;
			s_dsp_sco_manager.sbc_connect_sco_sink_SeqNum = s_wSeqNum;
			stream_connect( OPSRC(s_dsp_sco_manager.sbc_sco_send_opid), s_dsp_sco_manager.sbc_sco_sinkid);
		} else if( tid == s_dsp_sco_manager.sbc_sco_sinkid) {
			s_dsp_sco_manager.sbc_sco_send_tid2 = sbc_tid;
#ifdef CONFIG_CVC_OPERATE
			stream_connect( OPSRC(s_dsp_sco_manager.sbc_cvchf_receive_opid), s_dsp_sco_manager.sbc_codec_sinkid1);
#ifndef SCO_USE_2MIC
			stream_connect( s_dsp_sco_manager.sbc_codec_srcid, OPSINK(s_dsp_sco_manager.sbc_cvchf1mic_send_opid));
			stream_connect( OPSRC(s_dsp_sco_manager.sbc_sco_receive_opid), OPSINK(s_dsp_sco_manager.sbc_cvchf_receive_opid));
			stream_connect( OPSRC(s_dsp_sco_manager.sbc_cvchf1mic_send_opid), OPSINK(s_dsp_sco_manager.sbc_sco_send_opid));
			stream_connect( OPSRCONE(s_dsp_sco_manager.sbc_cvchf_receive_opid), OPSINKONE(s_dsp_sco_manager.sbc_cvchf1mic_send_opid));
#else
			stream_connect( s_dsp_sco_manager.sbc_codec_srcid, OPSINK(s_dsp_sco_manager.sbc_cvchf2mic_send_opid));
			stream_connect( s_dsp_sco_manager.sbc_codec_src2id, OPSINKONE(s_dsp_sco_manager.sbc_cvchf2mic_send_opid));
			stream_connect( OPSRC(s_dsp_sco_manager.sbc_sco_receive_opid), OPSINK(s_dsp_sco_manager.sbc_cvchf_receive_opid));
			stream_connect( OPSRC(s_dsp_sco_manager.sbc_cvchf2mic_send_opid), OPSINK(s_dsp_sco_manager.sbc_sco_send_opid));
			stream_connect( OPSRCONE(s_dsp_sco_manager.sbc_cvchf_receive_opid), OPSINKONE(s_dsp_sco_manager.sbc_cvchf2mic_send_opid)+1);
#endif
#else
			//add with out cvc here
			stream_connect( s_dsp_sco_manager.sbc_codec_srcid, OPSINK(s_dsp_sco_manager.sbc_sco_send_opid));
			stream_connect( OPSRC(s_dsp_sco_manager.sbc_sco_receive_opid),s_dsp_sco_manager.sbc_codec_sinkid1);
#endif
		} else if( tid == OPSINK(s_dsp_sco_manager.sbc_cvchf_receive_opid)) {
			s_dsp_sco_manager.sbc_cvchf_receive_tid = sbc_tid;
		} else if(tid == OPSINK(s_dsp_sco_manager.sbc_sco_send_opid)) {
			s_dsp_sco_manager.sbc_sco_send_tid1 = sbc_tid;
		}
#ifndef SCO_USE_2MIC
		else if(tid == OPSINKONE(s_dsp_sco_manager.sbc_cvchf1mic_send_opid))
#else
		else if ( tid == OPSINKONE(s_dsp_sco_manager.sbc_cvchf2mic_send_opid)+1)
#endif
		{
			WORD list_ids[4] = {0};
#ifndef SCO_USE_2MIC
			s_dsp_sco_manager.sbc_cvchf1mic_tid = sbc_tid;
#else
			s_dsp_sco_manager.sbc_cvchf2mic_tid = sbc_tid;
#endif
			list_ids[0] = s_dsp_sco_manager.sbc_sco_send_opid;
			list_ids[1] = s_dsp_sco_manager.sbc_sco_receive_opid;
#ifndef SCO_USE_2MIC
			list_ids[2] = s_dsp_sco_manager.sbc_cvchf1mic_send_opid;
#else
			list_ids[2] = s_dsp_sco_manager.sbc_cvchf2mic_send_opid;
#endif
			list_ids[3] = s_dsp_sco_manager.sbc_cvchf_receive_opid;

			s_dsp_sco_manager.start_sco_SeqNum = s_wSeqNum;
#ifdef CONFIG_CVC_OPERATE
			start_operators( 0x0004, list_ids);
#else
			start_operators( 0x0002, list_ids);
#endif
			//re-config
			//s_dsp_sco_manager.sco_config_tofadeout_connect = s_wSeqNum;
			//stream_configure( s_dsp_sco_manager.sbc_codec_sinkid1, CODEC_OUTPUT_GAIN, 17);
		} else if( tid == s_dsp_sco_manager.sbc_sco_srcid) {
		
		} else if ( tid == s_dsp_ringtone_manager.sbc_ringtone_sinkid1) {
			list_id[0] = s_dsp_ringtone_manager.sbc_ringtone_opid;
			s_dsp_ringtone_manager.sbc_ringtone_tid = sbc_tid;
			start_operators( 0x0001, list_id);
		}
#ifdef USE_RINGTONE_FASTPIPE
		else if( tid == OPSINK(s_dsp_ringtone_manager.sbc_ringtone_opid)) {
			s_dsp_ringtone_manager.sbc_ringtone_fastpipe_tid = sbc_tid;
		}
#endif
		else {

		}
	}
}

void hlp_get_source_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if ( cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		WORD src_id = cfm_par->payload[0];

		//a2dp getsrc_id does not rely on streamtype
		if (s_dsp_a2dp_manager.a2dp_get_sbcsrc_SeqNum == cfm_par->Seqno) {
			s_dsp_a2dp_manager.sbc_a2dp_src_id = src_id;
			stream_configure( s_dsp_a2dp_manager.sbc_a2dp_src_id, L2CAP_MAXIMUM_PDU_LENGTH, L2CAP_DSP_PDU_DEFAULT);
		}
		WORD wSeqNum = cfm_par->Seqno;
		if( wSeqNum == s_dsp_sco_manager.get_sco_src_SeqNum) {
			s_dsp_sco_manager.sbc_sco_srcid = src_id;
			bse_log_info(TAG, "[dsp]sid sourceid s_dsp_sco_manager.sbc_sco_srcid(0x%0x)\r\n",s_dsp_sco_manager.sbc_sco_srcid);
			s_dsp_sco_manager.get_sco_sink_SeqNum = s_wSeqNum;
			stream_get_sink( SCO_TYPE, s_dsp_sco_manager.sco_hdl, 0x0000);
		} else if ( wSeqNum == s_dsp_sco_manager.get_codec_src_SeqNum) {
			s_dsp_sco_manager.sbc_codec_srcid = src_id;
			bse_log_info(TAG, "[dsp]sid sourceid s_dsp_sco_manager.sbc_codec_srcid(0x%0x)\r\n",s_dsp_sco_manager.sbc_codec_srcid);

			if (get_codec_type() == HFP_CODECID_MSBC) {
				stream_configure( s_dsp_sco_manager.sbc_codec_srcid, CODEC_INPUT_SAMPLE_RATE, SAMPLE_RATE_16000);
			} else {
				stream_configure( s_dsp_sco_manager.sbc_codec_srcid, CODEC_INPUT_SAMPLE_RATE, SAMPLE_RATE_8000);
			}

			stream_configure( s_dsp_sco_manager.sbc_codec_srcid, CODEC_INPUT_GAIN, s_dsp_sco_manager.sbc_sco_mic_gain);
			s_n1stMic_Gain = s_dsp_sco_manager.sbc_sco_mic_gain+SET_DSPMIC_BASE_GAIN;
		}
#ifdef SCO_USE_2MIC
		else if( wSeqNum == s_dsp_sco_manager.get_codec_src2_SeqNum) {
			s_dsp_sco_manager.sbc_codec_src2id = src_id;
			if (get_codec_type() == HFP_CODECID_MSBC) {
				stream_configure( s_dsp_sco_manager.sbc_codec_src2id, CODEC_INPUT_SAMPLE_RATE, SAMPLE_RATE_16000);
			} else {
				stream_configure( s_dsp_sco_manager.sbc_codec_src2id, CODEC_INPUT_SAMPLE_RATE, SAMPLE_RATE_8000);
			}
			stream_configure( s_dsp_sco_manager.sbc_codec_src2id, CODEC_INPUT_GAIN, s_dsp_sco_manager.sbc_sco_mic_gain);
			s_n2ndMic_Gain = s_dsp_sco_manager.sbc_sco_mic_gain+SET_DSPMIC_BASE_GAIN;
			if ( s_dsp_sco_manager.sbc_codec_srcid && s_dsp_sco_manager.sbc_codec_src2id) {
				stream_sync_sid( s_dsp_sco_manager.sbc_codec_srcid, s_dsp_sco_manager.sbc_codec_src2id);
			}
		}
#endif
		else if( stream_type == STREAM_RINGTONE) {
#ifdef USE_RINGTONE_FASTPIPE
			s_dsp_ringtone_manager.sbc_ringtone_src = src_id;
#endif
		} else if (wSeqNum == s_dsp_ringtone_manager.sink1_ringtone_SeqNum) {
#ifdef USE_RINGTONE_FASTPIPE
			s_dsp_ringtone_manager.sbc_ringtone_src = src_id;
#endif
		} else {

		}
	}
}

void hlp_dsp_config_resp( void* param)
{
	bse_log_api(TAG, "[dsp]patch request\r\n");
	dspmanager_config_request();
}

static void hlp_msg_from_operator_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if ( cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		if(cfm_par->payload[0] == s_dsp_a2dp_manager.sbc_a2dp_opid) {
			//message_from_operator( cfm_par);
		} else if( stream_type == STREAM_SCO) {
			message_from_operator( cfm_par);
		} else if( stream_type == STREAM_RINGTONE) {
			message_from_operator( cfm_par);
		} else {

		}

	}
}

void hlp_sbc_disconnect_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if ( cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		if(stream_type == STREAM_A2DP) {
			if(cfm_par->payload[0] == s_dsp_a2dp_manager.sbc_a2dp_tid2) {

			}
		} else if( stream_type == STREAM_SCO) {
			if(cfm_par->payload[0] == s_dsp_sco_manager.sbc_cvc1hfmic_send_tid) {

			}
		} else if( stream_type == STREAM_RINGTONE) {
			if (cfm_par->payload[0] == s_dsp_ringtone_manager.sbc_ringtone_tid) {

			}
		} else {

		}

	}
}

/*++++++++++++++++++++++++++++++++++++
[argu]
-------------------------------------*/
void timer_enable_ringtone_fade_out(void)
{
	WORD param[1] = {0};
	WORD param_temp[2]= {0x0000,0x0000};
	bse_log_api(TAG, "[dsp]ringtone fadeout_disable active timer\r\n");
	operator_message_audio(s_dsp_ringtone_manager.sbc_ringtone_opid, RINGTONE_ENABLE_FADE_OUT, param);
}

void hlp_start_operator_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if ( cfm_par->Status == HCI_STATUS_OK) {
		bse_log_info(TAG, "[dsp] hlp_start_operator_resp cfm_par->Seqno: %d, s_dsp_sco_manager.start_sco_SeqNum: %d", cfm_par->Seqno, s_dsp_sco_manager.start_sco_SeqNum);
		if(s_dsp_sco_manager.start_sco_SeqNum == cfm_par->Seqno) {
			WORD wValue = 0x0000;
#ifdef CONFIG_CVC_OPERATE
			operator_message_cvc( s_dsp_sco_manager.sbc_cvchf_receive_opid, CVC_DISABLE_FADE_OUT, &wValue);//[error]
#else
			//need to see if the method have any value
			operator_message_audio( s_dsp_sco_manager.sbc_sco_receive_opid, AUDIO_ENABLE_FADE_OUT, &wValue);
#endif
		}
		app_stream_event_cbk(APP_EV_STREAM_STARTED, NULL);
	}
}

void hlp_stop_operator_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	bse_log_api(TAG, "[dsp]hlp_stop_operator_resp type(%d)\r\n",hlp_stream_get_type());
}

void hlp_close_sink_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if ( cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		WORD sink_id = cfm_par->payload[0];
		bse_log_info(TAG, "[dsp]hlp_close_sink_resp streamtype(%d)\r\n",stream_type);
		if(stream_type == STREAM_A2DP) {

		} else if( stream_type == STREAM_SCO) {

		} else if( stream_type == STREAM_RINGTONE) {

		} else {
			bse_log_info(TAG, "[dsp]sink close type is not sco a2dp ringtone\r\n");
		}
	} else {

	}
}

int hlp_close_source_resp(VENDOR_RESP* param)
{
	bse_log_api(TAG, "----->hlp_close_source_resp");
	VENDOR_RESP* cfm_par = param;
	if ( cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		WORD src_id = cfm_par->payload[0];
		bse_log_info(TAG, "[dsp]hlp_close_source_resp streamtype(%d)\r\n",stream_type);
		if(src_id == s_dsp_a2dp_manager.sbc_a2dp_src_id) {

		} else if(src_id == s_dsp_sco_manager.sbc_codec_srcid) {
			s_dsp_sco_manager.sbc_codec_srcid = 0;
		} else if( stream_type == STREAM_RINGTONE) {

		} else {
			bse_log_info(TAG, "[dsp]sink close type is not sco a2dp ringtone\r\n");
		}
	} else {

	}

	return 0;
}

void hlp_stream_set_type( UINT8 stream_type)
{
	s_stream_type = stream_type;
}
UINT8 hlp_stream_get_type()
{
	return s_stream_type;
}

/*+++++++++++++++++++++++++++++++++++++++++++++
[argu]Set BCCMD func,define key and key_value
----------------------------------------------*/
SYSTEM_LOCK s_bccmd_lock;
void Dsp_manager_Bccmd(WORD msg_type, WORD wBccmdVarid,WORD wValueNum ,WORD *pValueSet)
{
	BCCMD *pBcCmd = NULL;
	BCCMD BcCmdStrct;
	UINT8 *param = NULL;
	UINT16 ocf = 0;
	UINT8 param_len = 0;
	WORD  actulnum = 0;
	ENTER_CRITICAL(s_bccmd_lock);
	actulnum = wValueNum;

	//BCCMD payload minimum is 4 uint16s, the bccmd msg minimum is 18 bytes
	if (wValueNum < 4) {
		wValueNum = 4;
	}
	param_len = 1+sizeof(BCCMD_HDR)+sizeof(WORD)*wValueNum;
	param = (UINT8*) NEW( param_len);
	if (param == NULL) {
		bse_log_error(TAG, "Malloc buffer fail\r\n");
		return;
	}
	memset(param,0,param_len);

	// CSR Payload descriptor for BCCMD command
	param[0]  = 0xC2;

	// Set BCCMD Header
	pBcCmd = (BCCMD *)NEW(sizeof(BCCMD) + wValueNum * sizeof(WORD));
	memset( pBcCmd, 0, sizeof(BCCMD) + wValueNum * sizeof(WORD));
	pBcCmd->BcCmdHdr.Type = msg_type;
	pBcCmd->BcCmdHdr.Length = sizeof(BCCMD_HDR)/sizeof(WORD) + wValueNum;
	pBcCmd->BcCmdHdr.Seqno = s_wSeqNum++;
	pBcCmd->BcCmdHdr.Varid = wBccmdVarid;
	pBcCmd->BcCmdHdr.Status = 0x0000;

	//Set BCCMD Param
	memcpy(pBcCmd->Payload, pValueSet, wValueNum * sizeof(WORD));
	memcpy(param+1, pBcCmd,sizeof(BCCMD_HDR)+ wValueNum * sizeof(WORD));
	if (wValueNum > actulnum) {
		//set excessed byte zero
		memset(param+1+sizeof(BCCMD_HDR)+sizeof(WORD)*actulnum,0,sizeof(WORD)*(wValueNum - actulnum));
	}
	// bse_log_api(TAG, "[dsp]BCCMDSend:\r\n");
	// AppDbgPrintData(param,sizeof(BCCMD_HDR)+ wValueNum * sizeof(WORD)+1,0);
	hlp_gap_excute_vendor_command(0,ocf, param, param_len);
	FREE(pBcCmd);
	FREE(param);
	LEAVE_CRITICAL(s_bccmd_lock);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++*/
//[arg]hlp_set_patchindex_0
//scaler for patch again in one btinit process
/*------------------------------------------------*/
void hlp_set_patchindex_0()
{
	s_patch_index = 0;
}
UINT16 hlp_get_patchindex()
{
	return s_patch_index;
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_destroy_operate_resp
//use to trigger audio_close
/*-------------------------------------------*/
void hlp_destroy_operate_resp( void* param)
{
	VENDOR_RESP* cfm_par = param;
	if (cfm_par->Status == HCI_STATUS_OK) {
		UINT8 stream_type = hlp_stream_get_type();
		bse_log_info(TAG, "[dsp][event]destroy to audio_close type(%d)(0x%08x)\r\n",stream_type,cfm_par->payload[1]);

		hlp_audio_close(stream_type);

		if (param != NULL) {
			VENDOR_RESP* cfm_par = param;
			if ((cfm_par->payload[1] == s_dsp_sco_manager.sbc_sco_send_opid) && (cfm_par->payload[2] == s_dsp_sco_manager.sbc_sco_receive_opid)) { 
				s_dsp_sco_manager.sbc_sco_send_opid = 0;
				s_dsp_sco_manager.sbc_sco_receive_opid = 0;
				s_dsp_sco_manager.sbc_cvchf1mic_send_opid = 0;
				s_dsp_sco_manager.sbc_cvchf2mic_send_opid = 0;
				s_dsp_sco_manager.sbc_cvchf_receive_opid = 0;
				
				s_dsp_sco_manager.sbc_sco_srcid = 0;
				s_dsp_sco_manager.sbc_sco_sinkid = 0;
				s_dsp_sco_manager.sbc_codec_srcid = 0;
				s_dsp_sco_manager.sbc_codec_src2id = 0;
				s_dsp_sco_manager.sbc_codec_sinkid1 = 0;
				s_dsp_sco_manager.sbc_codec_sinkid2 = 0;
				
				s_dsp_sco_manager.sbc_codec_sink_tid = 0;
				s_dsp_sco_manager.sbc_cvc1hfmic_send_tid = 0;
				s_dsp_sco_manager.sbc_cvc2hfmic_send_tid = 0;
				s_dsp_sco_manager.sbc_cvc2hfmic_send_tid2 = 0;
				s_dsp_sco_manager.sbc_sco_send_tid1 = 0;
				s_dsp_sco_manager.sbc_sco_send_tid2 = 0;
				s_dsp_sco_manager.sbc_sco_receive_tid = 0;
				s_dsp_sco_manager.sbc_cvchf1mic_tid = 0;
				s_dsp_sco_manager.sbc_cvchf2mic_tid = 0;
				s_dsp_sco_manager.sbc_cvchf_receive_tid = 0;
			} else if (cfm_par->payload[1] == s_dsp_a2dp_manager.sbc_a2dp_opid) {
				s_dsp_a2dp_manager.sbc_a2dp_opid = 0;
				s_dsp_a2dp_manager.sbc_a2dp_sink1 = 0;
				s_dsp_a2dp_manager.sbc_a2dp_sink2 = 0;
				s_dsp_a2dp_manager.sbc_a2dp_tid1 = 0;
				s_dsp_a2dp_manager.sbc_a2dp_tid2 = 0;
				s_dsp_a2dp_manager.sbc_a2dp_src_id = 0;
				s_dsp_a2dp_manager.sbc_a2dp_src_tid = 0;
			} else if (cfm_par->payload[1] = s_dsp_ringtone_manager.sbc_ringtone_opid) {
				s_dsp_ringtone_manager.sbc_ringtone_opid = 0;
				s_dsp_ringtone_manager.sbc_ringtone_sinkid1 = 0;
				s_dsp_ringtone_manager.sbc_ringtone_sinkid2= 0;
				s_dsp_ringtone_manager.sbc_ringtone_tid = 0;
			}
			bse_log_info(TAG, "[dsp][stoped] payload[0]: 0x%04x", cfm_par->payload[1]);
		}
		app_stream_event_cbk(APP_EV_STREAM_STOPPED, NULL);
	}
}
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_sco_alias_sink_resp
//use to trigger sync
/*-------------------------------------------*/
void hlp_sco_alias_sink_resp(void* param)
{
	VENDOR_RESP* cfm_par = param;
	if (cfm_par->Status == HCI_STATUS_OK) {
		//alias to sync codecsink
		if (cfm_par->payload[0] == s_dsp_sco_manager.sbc_codec_sinkid1) {
			stream_sync_sid( s_dsp_sco_manager.sbc_codec_sinkid1,s_dsp_sco_manager.sbc_codec_sinkid2);
		} else if (cfm_par->payload[0] == s_dsp_ringtone_manager.sbc_ringtone_sinkid1) {
			//ringtone
			stream_sync_sid(s_dsp_ringtone_manager.sbc_ringtone_sinkid1,s_dsp_ringtone_manager.sbc_ringtone_sinkid2);
		}
	}
}

#ifdef USE_RINGTONE_FASTPIPE
/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_fastpipe_enable_resp
//use to enable the fastpipe buffer
/*-------------------------------------------*/
void hlp_fastpipe_enable_resp(void* param)
{
	VENDOR_RESP* cfm_par = param;
	if (cfm_par->Status == HCI_STATUS_OK) {
		fastpipe_resize( 2048);
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_fastpipe_enable_resp
//use to enable the fastpipe buffer
/*-------------------------------------------*/
void hlp_fastpipe_resize_resp(void* param)
{
	VENDOR_RESP* cfm_par = param;
	if (cfm_par->Status == HCI_STATUS_OK) {
		WORD arg[7] = {0};
		arg[0] = 0;
		arg[1] = 100;
		arg[2] = 256;
		arg[3] = 63;
		arg[4] = 63;
		arg[5] = 63;
		arg[6] = 63;
		fastpipe_create( arg);

		arg[0] = 1;
		arg[1] = 100;
		arg[2] = 256;
		arg[3] = 63;
		arg[4] = 63;
		arg[5] = 200;
		arg[6] = 896;
		fastpipe_create( arg);

		s_dsp_ringtone_manager.fastpipe_first_sendlength = 63+896;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_fastpipe_enable_resp
//use to enable the fastpipe buffer
/*-------------------------------------------*/
void hlp_fastpipe_create_resp(void* param)
{
	VENDOR_RESP* cfm_par = param;
	if (cfm_par->Status == HCI_STATUS_OK) {
		s_dsp_ringtone_manager.sbc_fastpipe_handle = cfm_par->payload[2];
	}
}
#endif

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[aug]hlp_vendor_cmd_resp_error
//process unusual situation,now only notify,
//convenient to check,may be modify later
/*-------------------------------------------*/
void hlp_vendor_cmd_resp_error(WORD cmd,VENDOR_RESP* pVendresp)
{
	WORD VendorId = cmd;
	int stream_tpye = hlp_stream_get_type();
	switch (VendorId) {
	case DSP_CMD_DSP_REQ:
		bse_log_info(TAG, "[dsp][error][event]DSP_START_OPERATE(0x%0x)\r\n",VendorId);
		//APP_CSR_WarmReset();
		break;
	case DSP_CREATE_OPERATE_C:
		bse_log_info(TAG, "[dsp][error][event]DSP_CREATE_OPERATE_C(0x%0x)\r\n",VendorId);
		break;
	case DSP_START_OPERATE:
		bse_log_info(TAG, "[dsp][error][event]DSP_START_OPERATE(0x%0x)\r\n",VendorId);
		break;
	case DSP_DESTROY_OPERATE:
		bse_log_info(TAG, "[dsp][error][event]DSP_DESTROY_OPERATE(0x%0x)\r\n",VendorId);
		break;
	case DSP_OPERATE_MSG:
		bse_log_info(TAG, "[dsp][error][event]DSP_OPERATE_MSG(0x%0x)\r\n",VendorId);
		break;
	case DSP_RESET_OPERATE:
		bse_log_info(TAG, "[dsp][error][event]DSP_RESET_OPERATE(0x%0x)\r\n",VendorId);
		break;
	case DSP_STREAM_ALIAS_SYNC:
		bse_log_info(TAG, "[dsp][error][event]DSP_STREAM_ALIAS_SYNC(0x%0x)\r\n",VendorId);
		break;
	case DSP_CLOSE_SRC:
		bse_log_info(TAG, "[dsp][error][event]DSP_CLOSE_SRC(0x%0x)\r\n",VendorId);
		break;
	case DSP_CLOSE_SNK:
		bse_log_info(TAG, "[dsp][error][event]DSP_CLOSE_SNK(0x%0x)\r\n",VendorId);
		break;
	case DSP_STREAM_TRANS_DISCONN:
		bse_log_info(TAG, "[dsp][error][event]DSP_STREAM_TRANS_DISCONN(0x%0x)\r\n",VendorId);
		break;
	case DSP_STOP_OPERATE:
		bse_log_info(TAG, "[dsp][error][event]DSP_STOP_OPERATE(0x%0x)\r\n",VendorId);
		break;
	case DSP_STREAM_CONNECT:
		bse_log_info(TAG, "[dsp][error][event]DSP_STREAM_CONNECT(0x%0x)\r\n",VendorId);
		break;
	case DSP_STREAM_SYNC_SID:
		bse_log_info(TAG, "[dsp][error][event]DSP_STREAM_SYNC_SID(0x%0x)\r\n",VendorId);
		break;
	default:
		break;
	}
	// 0x94: The host attempted to create an operator with a capability that does not exist. patch use;
	if ( VendorId == DSP_STOP_OPERATE || VendorId == DSP_STREAM_TRANS_DISCONN || VendorId == DSP_CLOSE_SNK
	     || VendorId == DSP_CLOSE_SRC || VendorId == DSP_DESTROY_OPERATE || VendorId == DSP_OPERATE_MSG
	     ||VendorId == DSP_STREAM_SYNC_SID || ( VendorId == DSP_CREATE_OPERATE_C && pVendresp->payload[0] == 0x94)) {
		if (( VendorId == DSP_CREATE_OPERATE_C && pVendresp->payload[0] == 0x94) ||(VendorId == DSP_OPERATE_MSG)||VendorId == DSP_STREAM_SYNC_SID) {
			if ( VendorId == DSP_CREATE_OPERATE_C && pVendresp->payload[0] == 0x94) {
				app_stream_event_cbk(APP_EV_STREAM_PATCH_COMPLETE, NULL);
			}
			bse_log_info(TAG, "[dsp][event]warning not care. vendorId(0x%0x)\r\n",VendorId);
		} else {
			bse_log_info(TAG, "[dsp][event][error]stop_oper fail vendorid(0x%0x)\r\n",VendorId);
			app_stream_event_cbk(APP_EV_STREAM_STOP_FAIL, NULL);
		}
	} else if( stream_tpye == STREAM_SCO) {
		WORD list_ids[4] = {0};
		bse_log_info(TAG, "[dsp][error][event]start fail.sco type vendorId(0x%0x)\r\n",VendorId);
		app_stream_event_cbk(APP_EV_STREAM_START_FAIL, NULL);

		list_ids[0] = s_dsp_sco_manager.sbc_sco_send_opid;
		list_ids[1] = s_dsp_sco_manager.sbc_sco_receive_opid;
#ifndef SCO_USE_2MIC
		list_ids[2] = s_dsp_sco_manager.sbc_cvchf1mic_send_opid;
#else
		list_ids[2] = s_dsp_sco_manager.sbc_cvchf2mic_send_opid;
#endif
		list_ids[3] = s_dsp_sco_manager.sbc_cvchf_receive_opid;
#ifdef CONFIG_CVC_OPERATE
		stop_operators( 0x0004, list_ids);
#else
		stop_operators( 0x0002, list_ids);
#endif
		if( VendorId == DSP_START_OPERATE) {
			stream_transform_disconnect( s_dsp_sco_manager.sbc_codec_sink_tid);
#ifndef SCO_USE_2MIC
			stream_transform_disconnect( s_dsp_sco_manager.sbc_cvc1hfmic_send_tid);
			stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf_receive_tid);
			stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf1mic_tid);
#else
			stream_transform_disconnect( s_dsp_sco_manager.sbc_cvc2hfmic_send_tid);
			stream_transform_disconnect( s_dsp_sco_manager.sbc_cvc2hfmic_send_tid2);
			stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf_receive_tid);
			stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf2mic_tid);
#endif
			stream_transform_disconnect( s_dsp_sco_manager.sbc_sco_send_tid1);
		}
		//sco_sync

		if ( pVendresp->Seqno == s_dsp_sco_manager.sbc_connect_sco_sink_SeqNum ||
		     pVendresp->Seqno == s_dsp_sco_manager.sbc_connect_sco_src_SeqNum ||
		     VendorId == DSP_START_OPERATE) {
			stream_sync_sid( s_dsp_sco_manager.sbc_codec_sinkid1, 0x0000);
			stream_sync_sid( s_dsp_sco_manager.sbc_codec_sinkid2, 0x0000);
#ifdef SCO_USE_2MIC
			stream_close_source( s_dsp_sco_manager.sbc_codec_src2id);
#endif
			stream_close_source( s_dsp_sco_manager.sbc_codec_srcid);
			stream_close_sink( s_dsp_sco_manager.sbc_codec_sinkid1);
			stream_close_sink( s_dsp_sco_manager.sbc_codec_sinkid2);
		}
#ifdef CONFIG_CVC_OPERATE
		destroy_operators( 0x0004, list_ids);
#else 
		destroy_operators( 0x0002, list_ids);
#endif
	} else if ( stream_tpye == STREAM_A2DP) {
		WORD list_ids[1] = {0};
		app_stream_event_cbk(APP_EV_STREAM_START_FAIL, NULL);
		bse_log_info(TAG, "[dsp][error][event]start fail.a2dp type.vendorId(0x%0x)\r\n",VendorId);

		list_ids[0] = s_dsp_a2dp_manager.sbc_a2dp_opid;
		stop_operators( 0x0001, list_ids);
		//stream disconn
		stream_transform_disconnect( s_dsp_a2dp_manager.sbc_a2dp_src_tid);
		stream_transform_disconnect( s_dsp_a2dp_manager.sbc_a2dp_tid1);
		stream_transform_disconnect( s_dsp_a2dp_manager.sbc_a2dp_tid2);
		//stream unsync
		stream_sync_sid( s_dsp_a2dp_manager.sbc_a2dp_sink1, 0x0000);
		stream_sync_sid( s_dsp_a2dp_manager.sbc_a2dp_sink2, 0x0000);

		//close sink
		stream_close_sink( s_dsp_a2dp_manager.sbc_a2dp_sink1);
		stream_close_sink( s_dsp_a2dp_manager.sbc_a2dp_sink2);

		stream_close_source( s_dsp_a2dp_manager.sbc_a2dp_src_id);
		destroy_operators( 1, &s_dsp_a2dp_manager.sbc_a2dp_opid);
	} else if( stream_tpye == STREAM_RINGTONE) {
		WORD list_ids[1] = {0};
		app_stream_event_cbk(APP_EV_STREAM_START_FAIL, NULL);
		bse_log_info(TAG, "[dsp][error][event]start fail.ringtone type.vendorId(0x%0x)\r\n",VendorId);

		list_ids[0] = s_dsp_ringtone_manager.sbc_ringtone_opid;
		stop_operators( 0x0001, list_ids);
		stream_transform_disconnect( s_dsp_ringtone_manager.sbc_ringtone_tid);

		stream_sync_sid( s_dsp_ringtone_manager.sbc_ringtone_sinkid1, 0x0000);
		stream_close_sink( s_dsp_ringtone_manager.sbc_ringtone_sinkid1);
		stream_close_sink( s_dsp_ringtone_manager.sbc_ringtone_sinkid2);
		destroy_operators( 0x0001, list_ids);
	}
}

#ifdef USE_RINGTONE_FASTPIPE
void hlp_fastpipe_sendwavfile( UINT8* filename)
{
	FILE* fp = NULL;
	long file_length = 0;

	// open wav file;
	fp = fopen( filename, "rt");
	if ( fp == NULL) {
		bse_log_error(TAG, "[dsp][fastpipe] hlp_fastpipe_sendwavfile open file fail!\r\n");
		return;
	}

	// read wav file data to catch buffer;
	file_length = ftell(fp);
	wavdata_buf = (UINT8 *)malloc(file_length+1);
	memset( wavdata_buf, 0, file_length);
	fread( wavdata_buf, 1, file_length, fp);

	// send data ;
	hlp_fastpipe_data( NULL);
	fclose(fp);
}

void hlp_fastpipe_data( UINT8* data)
{
	UINT8 * fastpipe_buf = NULL;
	if ( data == NULL) {
		// data is null, first send wav file ;
		fastpipe_index = 0;
		fastpipe_buf = (UINT8*)malloc( s_dsp_ringtone_manager.fastpipe_first_sendlength + 6);
		memset( fastpipe_buf,0, s_dsp_ringtone_manager.fastpipe_first_sendlength + 6);
		sprintf( fastpipe_buf, "\x03%04x%04x", s_dsp_ringtone_manager.sbc_fastpipe_handle, s_dsp_ringtone_manager.fastpipe_first_sendlength);
		memcpy( fastpipe_buf+5, wavdata_buf, s_dsp_ringtone_manager.fastpipe_first_sendlength);
		UART_TL_WriteBuf( fastpipe_buf, s_dsp_ringtone_manager.fastpipe_first_sendlength+6);
		fastpipe_index = s_dsp_ringtone_manager.fastpipe_first_sendlength;
	} else {
		// parse the data, send aggign length data;

	}
}
#endif

/*+++++++++++++++++++++++++++++++++++++++++++*/
//[argu]codec type(1-cvsd 2-msbc) about
/*-------------------------------------------*/
static unsigned char s_ncodectype = 1;
unsigned char get_codec_type(void)
{
	return s_ncodectype;
}
void set_codec_type(unsigned char nType)
{
	s_ncodectype = nType;
}

STREAM_TYPE_NAME stream_array[5] = {
	{STREAM_SCO,"sco"},
	{STREAM_A2DP,"a2dp"},
	{STREAM_RINGTONE,"ringtone"},
	{STREAM_IDLE,"null"}
};
char * hlp_findname(UINT8 type)
{
	int i = 0;
	for (i = 0; i<(sizeof(stream_array)/sizeof(STREAM_TYPE_NAME)); i++) {
		if (type == stream_array[i].stream_type) {
			return stream_array[i].name;
		}
	}
	return NULL;
}

int hlp_GetLocalMicGain()
{
	return s_dsp_sco_manager.sbc_sco_mic_gain;
}

void hlp_SetLocalMicGain(UINT16 nMicGain)
{
	s_dsp_sco_manager.sbc_sco_mic_gain = nMicGain;
    bse_log_info(TAG, "SET MIC GAIN, sbc_codec_srcid: %x, sbc_codec_src2id: %x", s_dsp_sco_manager.sbc_codec_srcid, s_dsp_sco_manager.sbc_codec_src2id);
	if (s_dsp_sco_manager.sbc_codec_srcid) {
		stream_configure( s_dsp_sco_manager.sbc_codec_srcid, CODEC_INPUT_GAIN, s_dsp_sco_manager.sbc_sco_mic_gain);
	}
#ifdef SCO_USE_2MIC
	if (s_dsp_sco_manager.sbc_codec_src2id) {
		stream_configure( s_dsp_sco_manager.sbc_codec_src2id, CODEC_INPUT_GAIN, s_dsp_sco_manager.sbc_sco_mic_gain);
	}
#endif
}

/*
General										Key			Supported Values
Audio channel mute enable ****				0x0700		0: Disable/1: Enable-mute
*/
static unsigned int s_MicChannelMute = 0;
void hlp_SetChannelMuteMic(unsigned int flag)
{
	if(hlp_stream_get_type() == STREAM_SCO) {
		UINT16 Chn_val= flag & 0xffff;
		s_MicChannelMute = Chn_val;
		if (s_dsp_sco_manager.sbc_codec_srcid) {
			stream_configure( s_dsp_sco_manager.sbc_codec_srcid, AUDIO_CHANNEL_MUTE_ENABLE, Chn_val);
		}
#ifdef SCO_USE_2MIC
		if(s_dsp_sco_manager.sbc_codec_src2id) {
			stream_configure( s_dsp_sco_manager.sbc_codec_src2id, AUDIO_CHANNEL_MUTE_ENABLE, Chn_val);
		}
#endif
	}
}

unsigned int hlp_getChannelMuteState()
{
	return s_MicChannelMute;
}

/*+++++++++++++++++++++++++++++++++++++++++
[Argu]hlp_SetCVCMode,cvc operator set
[param]0-standby 1-HFK 2-ASR 3-Left 4-right
------------------------------------------*/
void hlp_SetCVCMode(UINT nmode)
{
	WORD param = 0x0000;
	if (s_dsp_sco_manager.sbc_cvchf2mic_send_opid) {
		operator_message_cvc(s_dsp_sco_manager.sbc_cvchf2mic_send_opid, CVC_GET_PARAM, &param);
		operator_message_cvc(s_dsp_sco_manager.sbc_cvchf2mic_send_opid, CVC_SET_PARAM, &param);
	}
}
/*+++++++++++++++++++++++++++++++++++++++++
[Argu]hlp_Set A2DP APT-X or SBC Mode
[param]
------------------------------------------*/
static BOOL s_bAPTX = FALSE;
static int s_nSampleRate = 44100;
void hlp_SetA2DPStream_AptxMode(BOOL baptx,int nsamplerate)
{
	s_bAPTX = baptx;
	if (nsamplerate) {
		s_nSampleRate = nsamplerate;
	}
}
BOOL hlp_GetA2DPStream_AptxMode(int* pnRate)
{
	if (pnRate) {
		*pnRate = s_nSampleRate;
	}
	return s_bAPTX;
}

int hlp_GetLocalSpkGain()
{
	return s_dsp_sco_manager.sbc_sco_spk_gain;
}

void hlp_SetLocalSpkGain(UINT16 nSpkGain)
{
	s_dsp_sco_manager.sbc_sco_spk_gain = nSpkGain;
	if(hlp_stream_get_type() == STREAM_SCO) {
		if (s_dsp_sco_manager.sbc_codec_sinkid1) {
			stream_configure( s_dsp_sco_manager.sbc_codec_sinkid1, CODEC_OUTPUT_GAIN, s_dsp_sco_manager.sbc_sco_spk_gain);
		}
		if (s_dsp_sco_manager.sbc_codec_sinkid2) {
			s_dsp_sco_manager.change_sco_sink_SeqNum = s_wSeqNum;
			stream_configure( s_dsp_sco_manager.sbc_codec_sinkid2, CODEC_OUTPUT_GAIN, s_dsp_sco_manager.sbc_sco_spk_gain);
		}
	}
}
/***************************************************
description of HQ(C3) Message
***************************************************/
UINT8 hlp_process_HQ_msg(void* cfm_par_event)
{
	UINT8 result = 0;
	WORD VendorId = 0;
	static VENDOR_RESP* cfm_par_vendor = NULL;
	if(cfm_par_event == NULL) {
		return -1;
	}

	cfm_par_vendor = (VENDOR_RESP*)((UINT8*)cfm_par_event);
	VendorId = cfm_par_vendor->Varid;
	result = cfm_par_vendor->Status;
	if (HCI_STATUS_OK != result) {
		hlp_vendor_cmd_resp_error(VendorId, cfm_par_vendor);
		return result;
	}
	if ( VendorId == DSP_MSG_FROM_OPR) {
		// 00 fc 27 c3 01 00 13 00 28 00 1a 10 00 00 00 41    06 10    07 00   cf 00   cf 00    ff ff 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
		hlp_msg_from_operator_resp(cfm_par_vendor);
		switch (cfm_par_vendor->payload[1]) {
		case CVC_CODEC_MSG: {
#ifdef CONFIG_CVC_OPERATE
			WORD nDAC_Gain = cfm_par_vendor->payload[2];
			WORD n1st_Micgain = cfm_par_vendor->payload[3];
			WORD n2nd_Micgain = cfm_par_vendor->payload[4];
			WORD cvc_src2id = 0;
			WORD cvc_opid = 0;
			WORD cvc_srcid = 0;


			UINT8 stream_type = hlp_stream_get_type();

			if ( stream_type == STREAM_SCO ) {
				cvc_opid = s_dsp_sco_manager.sbc_cvchf_receive_opid;
				cvc_srcid = s_dsp_sco_manager.sbc_codec_srcid;
#ifdef SCO_USE_2MIC
				cvc_src2id = s_dsp_sco_manager.sbc_codec_src2id;
#endif
			} else {
				//ignore this event
				break;
			}
			bse_log_info(TAG, "[dsp][event] cvc_codec  Dac_gain(0x%0x) n1st_micgain(0x%0x) n2nd_MicGain(0x%0x) \r\    nLocal(s_nDacGain-0x%0x s_n1stGain-0x%0x)\r\n",
			             nDAC_Gain, n1st_Micgain, n2nd_Micgain,s_nDAC_Gain,s_n1stMic_Gain);
			if ( s_bIsFirstCvc) {
				s_bIsFirstCvc = FALSE;
				s_nFactoryMicGain = n1st_Micgain - SET_DSPMIC_BASE_GAIN;
			}

			if ( nDAC_Gain != s_nDAC_Gain) {
				operator_message_cvc( cvc_opid, CVC_VOLUME, &nDAC_Gain);
				s_nDAC_Gain = nDAC_Gain;
			}
			if ( (n1st_Micgain) != s_n1stMic_Gain) {
				// n1st_Micgain 0x00CF 16bit 15: pre-amp, +24db(0x0011 Scale stall );  0~8:mic gain( 0xC0 == 0x0000);
				stream_configure( cvc_srcid, CODEC_INPUT_GAIN,  (n1st_Micgain)-SET_DSPMIC_BASE_GAIN);
				s_n1stMic_Gain =  (n1st_Micgain);
			}
#ifdef SCO_USE_2MIC
			if ( (n2nd_Micgain) != s_n2ndMic_Gain ) {
				stream_configure( cvc_src2id, CODEC_INPUT_GAIN, (n2nd_Micgain)-SET_DSPMIC_BASE_GAIN);
				s_n2ndMic_Gain = (n2nd_Micgain);
			}
#endif
#endif //end if CONFIG_CVC_OPERATE
		}
		break;
		case FADE_OUT_COMPLETE_MSG: {
				//no param to show enable or disable, success or fail.only to show which id fadeout_complete
				bse_log_info(TAG, "[dsp][event][fadeout] completed! cfm_par_vendor->payload[0]: %x, s_dsp_a2dp_manager.sbc_a2dp_opid: %x, s_dsp_sco_manager.sbc_cvchf1mic_send_opid: %x, s_dsp_sco_manager.sbc_cvchf_receive_opid: %x, s_dsp_sco_manager.sbc_cvchf2mic_send_opid: %x, s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed: %x, s_dsp_sco_manager.sbc_sco_cvchf1mic_send_fadeout_completed: %x, s_dsp_sco_manager.sbc_sco_cvchf2mic_send_fadeout_completed: %x\r\n", cfm_par_vendor->payload[0], s_dsp_a2dp_manager.sbc_a2dp_opid, s_dsp_sco_manager.sbc_cvchf1mic_send_opid, s_dsp_sco_manager.sbc_cvchf_receive_opid, s_dsp_sco_manager.sbc_cvchf2mic_send_opid, s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed, s_dsp_sco_manager.sbc_sco_cvchf1mic_send_fadeout_completed, s_dsp_sco_manager.sbc_sco_cvchf2mic_send_fadeout_completed);
				if (cfm_par_vendor->payload[0] == s_dsp_a2dp_manager.sbc_a2dp_opid) {
					if (s_a2dp_fadeout_timer) {
						FSM_DelTimer(&s_a2dp_fadeout_timer);
						s_a2dp_fadeout_timer = NULL;
					}
					stream_a2dp_stop_timer();
				}
#ifdef CONFIG_CVC_OPERATE
				else if (cfm_par_vendor->payload[0] == s_dsp_sco_manager.sbc_cvchf1mic_send_opid) {
					if (s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed == 1) {
						s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed = 0;
						s_dsp_sco_manager.sbc_sco_cvchf1mic_send_fadeout_completed = 0;
						stream_sco_stop_timer();
					} else {
						s_dsp_sco_manager.sbc_sco_cvchf1mic_send_fadeout_completed = 1;
					}
				} else if (cfm_par_vendor->payload[0] == s_dsp_sco_manager.sbc_cvchf_receive_opid) {
#ifdef SCO_USE_2MIC
					if (s_dsp_sco_manager.sbc_sco_cvchf2mic_send_fadeout_completed == 1)
#else
					if (s_dsp_sco_manager.sbc_sco_cvchf1mic_send_fadeout_completed == 1)
#endif
					{
#ifdef SCO_USE_2MIC
						s_dsp_sco_manager.sbc_sco_cvchf2mic_send_fadeout_completed = 0;
#else
						s_dsp_sco_manager.sbc_sco_cvchf1mic_send_fadeout_completed = 0;
#endif 
						s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed = 0;
						stream_sco_stop_timer();
					} else {
						s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed = 1;
					}
				} else if (cfm_par_vendor->payload[0] == s_dsp_ringtone_manager.sbc_ringtone_opid) {
                    bse_log_info(TAG, "[dsp] -----> [opid: %04x ], ring tone fadeout completed!", s_dsp_ringtone_manager.sbc_ringtone_opid);
                    stream_ringtone_stop_timer();
                }
#ifdef SCO_USE_2MIC
				else if (cfm_par_vendor->payload[0] == s_dsp_sco_manager.sbc_cvchf2mic_send_opid) {
					if (s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed == 1) {
						s_dsp_sco_manager.sbc_sco_cvchf_receive_fadeout_completed = 0;
						s_dsp_sco_manager.sbc_sco_cvchf2mic_send_fadeout_completed = 0;
						stream_sco_stop_timer();
					} else {
						s_dsp_sco_manager.sbc_sco_cvchf2mic_send_fadeout_completed = 1;
					}
				}
#endif
#else
				else if (cfm_par_vendor->payload[0] == s_dsp_sco_manager.sbc_sco_send_opid) {
					if (s_dsp_sco_manager.sbc_sco_receive_fadeout_completed == 1) {
						s_dsp_sco_manager.sbc_sco_receive_fadeout_completed = 0;
						s_dsp_sco_manager.sbc_sco_send_fadeout_completed = 0;
						stream_sco_stop_timer();
					} else {
						s_dsp_sco_manager.sbc_sco_send_fadeout_completed = 1;
					}
				} else if (cfm_par_vendor->payload[0] == s_dsp_sco_manager.sbc_sco_receive_opid) {
					if (s_dsp_sco_manager.sbc_sco_send_fadeout_completed == 1) {
						s_dsp_sco_manager.sbc_sco_receive_fadeout_completed = 0;
						s_dsp_sco_manager.sbc_sco_send_fadeout_completed = 0;
						stream_sco_stop_timer();
					} else {
						s_dsp_sco_manager.sbc_sco_recevice_fadeout_completed = 1;
					}
				}
#endif
		}
		break;
		case SECURITY_PASSED_MSG: {
			bse_log_info(TAG, "[dsp][event][license]SECURITY_PASSED_MSG--- ok!\r\n");
		}
		break;
		case SECURITY_FAILED_MSG: {
			bse_log_info(TAG, "[dsp][event][license]SECURITY_FAILED_MSG--- fail!\r\n");
		}
		break;
		default:
			bse_log_info(TAG, "[dsp][event][ignor]VendorId(0x%0x)--- DSP_MSG_FROM_OPR\r\n",cfm_par_vendor->payload[1]);
			break;
		}
	} else if(VendorId == DSP_CMD_DSP_REQ) {
		//fillin dsp patch
		bse_log_info(TAG, "[dsp][event]VendorId(0x%0x)--- DSP_REQ_PATCH\r\n",VendorId);
		hlp_dsp_config_resp(cfm_par_vendor);
	}
	return 0;
}

void stream_a2dp_stop_timer()
{
	bse_log_info(TAG, "[dsp] enter stream_a2dp_stop_timer");
	WORD list_ids[1] = {0};
	list_ids[0] = s_dsp_a2dp_manager.sbc_a2dp_opid;
	stop_operators( 0x0001, list_ids);

	//stream disconn
	stream_transform_disconnect( s_dsp_a2dp_manager.sbc_a2dp_src_tid);
	stream_transform_disconnect( s_dsp_a2dp_manager.sbc_a2dp_tid1);
	stream_transform_disconnect( s_dsp_a2dp_manager.sbc_a2dp_tid2);
	//stream unsync
	stream_sync_sid( s_dsp_a2dp_manager.sbc_a2dp_sink1, 0x0000);
	stream_sync_sid( s_dsp_a2dp_manager.sbc_a2dp_sink2, 0x0000);

	//close sink
	stream_close_sink( s_dsp_a2dp_manager.sbc_a2dp_sink1);
	stream_close_sink( s_dsp_a2dp_manager.sbc_a2dp_sink2);

	bse_log_info(TAG, "[dsp]release a2dp resouce, in process of audio_stop\r\n");
	stream_close_source( s_dsp_a2dp_manager.sbc_a2dp_src_id);
	destroy_operators( 1, &s_dsp_a2dp_manager.sbc_a2dp_opid);
	s_a2dp_fadeout_timer = NULL;
}

void stream_sco_stop_timer()
{
	bse_log_info(TAG, "[dsp] enter stream_sco_stop_timer");
#ifdef CONFIG_CVC_OPERATE
	WORD list_ids[4] = {0};
	list_ids[0] = s_dsp_sco_manager.sbc_sco_send_opid;
	list_ids[1] = s_dsp_sco_manager.sbc_sco_receive_opid;

#ifndef SCO_USE_2MIC
	list_ids[2] = s_dsp_sco_manager.sbc_cvchf1mic_send_opid;
#else
	list_ids[2] = s_dsp_sco_manager.sbc_cvchf2mic_send_opid;
#endif
	list_ids[3] = s_dsp_sco_manager.sbc_cvchf_receive_opid;


	if(hlp_getChannelMuteState()) {
		hlp_SetChannelMuteMic(0);
	}
	stop_operators( 0x0004, list_ids);

	stream_transform_disconnect( s_dsp_sco_manager.sbc_codec_sink_tid);
#ifndef SCO_USE_2MIC
	stream_transform_disconnect( s_dsp_sco_manager.sbc_cvc1hfmic_send_tid);
	stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf_receive_tid);
	stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf1mic_tid);
#else
	stream_transform_disconnect( s_dsp_sco_manager.sbc_cvc2hfmic_send_tid);
	stream_transform_disconnect( s_dsp_sco_manager.sbc_cvc2hfmic_send_tid2);
	stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf_receive_tid);
	stream_transform_disconnect( s_dsp_sco_manager.sbc_cvchf2mic_tid);
#endif
	stream_transform_disconnect( s_dsp_sco_manager.sbc_sco_send_tid1);
	//stream_transform_disconnect( s_dsp_sco_manager.sbc_sco_receive_tid);
	//stream_transform_disconnect( s_dsp_sco_manager.sbc_sco_send_tid2);

	//sco_sync
	stream_sync_sid( s_dsp_sco_manager.sbc_codec_sinkid1, 0x0000);
	stream_sync_sid( s_dsp_sco_manager.sbc_codec_sinkid2, 0x0000);
	stream_close_source( s_dsp_sco_manager.sbc_codec_srcid);
	stream_close_sink( s_dsp_sco_manager.sbc_codec_sinkid1);
	stream_close_sink( s_dsp_sco_manager.sbc_codec_sinkid2);
#ifdef SCO_USE_2MIC
	stream_close_source( s_dsp_sco_manager.sbc_codec_src2id);
#endif
	destroy_operators( 0x0004, list_ids);

#else
	WORD list_opid[2] = {0};
	list_opid[0] = s_dsp_sco_manager.sbc_sco_send_opid;
	list_opid[1] = s_dsp_sco_manager.sbc_sco_receive_opid;

	stop_operators( 0x0002, list_opid);
	stream_transform_disconnect( s_dsp_sco_manager.sbc_codec_sink_tid);
	stream_transform_disconnect( s_dsp_sco_manager.sbc_sco_send_tid1);

	//sco_sync
	stream_sync_sid( s_dsp_sco_manager.sbc_codec_sinkid1, 0x0000);
	stream_sync_sid( s_dsp_sco_manager.sbc_codec_sinkid2, 0x0000);
	stream_close_source( s_dsp_sco_manager.sbc_codec_srcid);
	stream_close_sink( s_dsp_sco_manager.sbc_codec_sinkid1);
	stream_close_sink( s_dsp_sco_manager.sbc_codec_sinkid2);
	destroy_operators( 0x0002, list_opid);
#endif
}

void hlp_csr_set_pskey(WORD key, UINT32 value_num, WORD *key_value)
{
	WORD *param = (WORD *)malloc( (value_num+3)*sizeof(WORD));
	memset(param, 0, (value_num+3)*sizeof(WORD));
	param[0] = key;
	param[1] = value_num&0xff;
	param[2] = value_num >> 8;
	memcpy(param+3, key_value, value_num*sizeof(WORD));
	bse_log_info(TAG, "[dsp][cmd]hlp_csr_set_pskey(0x7003)+ param(0x%0x,0x%0x,0x%0x)\r\n",param[0],param[1],param[2]);
	Dsp_manager_Bccmd(BCCMDN_SETREQ, CSR_PSKEY,value_num+3,param);
	free(param);
}

void hlp_csr_get_pskey(WORD key, UINT32 value_num)                                                                                                                         
{
	WORD *param = (WORD *)malloc( (value_num+3)*sizeof(WORD));
	memset(param, 0, (value_num+3)*sizeof(WORD));

	param[0] = key;
	param[1] = value_num&0xff;
	param[2] = value_num >> 8;
	
	s_get_csr_pskey_wSeqNum = s_wSeqNum;
	bse_log_info(TAG, "[dsp][cmd]hlp_csr_get_pskey(0x7003)+ param(0x%0x,0x%0x,0x%0x)\r\n", param[0],param[1],param[2]);
	Dsp_manager_Bccmd(BCCMD_GETREQ, CSR_PSKEY, value_num+3, param);
	free(param);
}

/*+++++++++++++++++++++++++++++++++++++++++++

--------------------------------------------*/
void stream_ringtone_stop_timer()
{
	WORD list_ids[1] = {0};
	list_ids[0] = s_dsp_ringtone_manager.sbc_ringtone_opid;
	stop_operators( 0x0001, list_ids);
	stream_transform_disconnect( s_dsp_ringtone_manager.sbc_ringtone_tid);

	stream_sync_sid( s_dsp_ringtone_manager.sbc_ringtone_sinkid1, 0x0000);
	stream_close_sink( s_dsp_ringtone_manager.sbc_ringtone_sinkid1);
	stream_close_sink( s_dsp_ringtone_manager.sbc_ringtone_sinkid2);
	destroy_operators( 0x0001, list_ids);
}
void hlp_stream_fadeout_ringtone(void)
{
	FsmAddTimer(NULL, 1200, 0, NULL, NULL, timer_enable_ringtone_fade_out);//make up ringtone
}

void hlp_csr_pskey_resp(void *param)
{
	VENDOR_RESP* cfm_par = (VENDOR_RESP*) param;

	if (cfm_par && (cfm_par->Status == HCI_STATUS_OK)) {
		UINT16 wSeqNum = cfm_par->Seqno;
		if (s_get_csr_pskey_wSeqNum == wSeqNum) {
			// get pskey event update;
			
			if (cfm_par->payload[0] == 0x02ae) {
				char firmware_version[20];
				int i = 0;

				memset(firmware_version, 0, sizeof(firmware_version));
				bse_log_info(TAG, "[dsp][res] hlp_csr_pskey_resp pld[0]: %x, pld[1]: %x, pld[2]: %x, pld[3]: %x, pld[4]: %x", \
						 cfm_par->payload[0], cfm_par->payload[1], cfm_par->payload[2], cfm_par->payload[3], cfm_par->payload[4]);
				for ( ; i < cfm_par->payload[1]; i++) {
					firmware_version[i*2] = cfm_par->payload[i+3] & 0xff;
					firmware_version[i*2+1] = cfm_par->payload[i+3] >> 8;
				}
				bse_log_info(TAG, "[dsp] get firmware_version : %s\r\n", firmware_version);
				//bt_event_indicate(BT_EV_OTHER_GET_PSKEY, firmware_version);
				vendor_save_frimware_version(firmware_version);
			}
		}
	} else {
		
	}

	return;
}

/**
 * @oaram flag 1: mica, 0: micB
 * 
 * 
 * */
void hlp_csr_8350_call_use_mic(int flag)
{
    bse_log_info(TAG, "set mic: %d", s_use_mica);
    s_use_mica = flag;
}

#endif
