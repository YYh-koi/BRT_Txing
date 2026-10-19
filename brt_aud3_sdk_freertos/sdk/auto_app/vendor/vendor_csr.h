/*************************************************************************
    > File Name: auto_app/vendor/vendor_csr.h
    >
    >
    > Created Time: Wed 12 Oct 2016 03:57:03 PM CST
 ************************************************************************/

// FF 13 C2 01 00 09 00 00 00 5B 50 00 00 5F BA 00 00 00 00 00 00
typedef struct Vendor_RespStru {
	//UINT8 head;
	UINT8 pkg_length;
	UINT8 vendor_head;
	UINT16 type;
	UINT16 vendor_length;
	UINT16 Seqno;
	UINT16 Varid;
	UINT16 Status;
	UINT16 payload[1];
} VENDOR_RESP;


enum {
    DSP_CMD_DSP_REQ				 = 0x101c,
    DSP_MSG_FROM_OPR			 = 0x101a,

    DSP_GET_CHIP_VERSION         = 0x281a,

    DSP_SET_SCOCONFIG		     = 0x304d,

    DSP_ENABLE_SCOSTREAM		 = 0x4876,
    DSP_CLOSE_SRC				 = 0x486b,
    DSP_CLOSE_SNK				 = 0x486c,
    DSP_STREAM_TRANS_DISCONN     = 0x486d,
    DSP_STREAM_EXTERNAL_DETACH   = 0x487F,

    DSP_FASTPIPE_ENABLE			 = 0x5041,
    DSP_FASTPIPE_CREATE			 = 0x5042,
    DSP_FASTPIPE_RESIZE			 = 0x5057,

    DSP_STREAM_GETSRC			 = 0x505a,
    DSP_STREAM_GETSNK			 = 0x505b,
    DSP_STREAM_CFG				 = 0x505c,
    DSP_STREAM_ALIAS_SYNC		 = 0x505d,
    DSP_STREAM_CONNECT			 = 0x505e,
    DSP_STREAM_SYNC_SID		     = 0x5062,

    DSP_START_OPERATE			 = 0x5070,
    DSP_STOP_OPERATE			 = 0x5071,
    DSP_RESET_OPERATE			 = 0x5072,
    DSP_DESTROY_OPERATE			 = 0x5073,
    DSP_OPERATE_MSG				 = 0x5074,
    DSP_CREATE_OPERATE_C		 = 0x5075,

	PIO32_SET_PIN_FUNCTION		 = 0x5082,
    PIO32_HIGHLOW_CONTROL		 = 0x7084,
    PIO32_DIRECTION_CONTROL		 = 0x7083,
	CSR_PSKEY                    = 0x7003,
};

void vendor_save_project_version(char * project_ver);
