/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    pan_ui.h
Abstract:
    This file exports PAN user interface.
Author:
    Zeng Yongping
Revision History:
    1.  2001.12.10  Created
    2.  2003.03.15	adapted for SOC stack
    3.  2011.03.16  (CR158)
    4.  2011.06.20  (CR164)
---------------------------------------------------------------------------*/
#ifndef __PAN_UI_H__
#define __PAN_UI_H__

#define MAX_PAN_USER_NUM 7
#define MAX_NET_TYPE_NUM 16

#define PAN_ETHERNET						0x00 
#define PAN_GENERAL_ETHERNET				0x01 
#define PAN_COMPRESSED_ETHERNET				0x02 
#define PAN_ETHERNET_SOURCE_ONLY			0x03 
#define PAN_ETHERNET_DEST_ONLY				0x04

#define MAC_ADDR_LEN        			 6

struct PAN_EthFrameStru {
	WORD	reserved1;
	WORD	len;			/* the length of the ethernet frame */
	void	*reserved2;
	UCHAR	frame[1];		/* the body of the ethernet frame */
};

struct PAN_ServiceStru {
	WORD	cbSize;

	DWORD	svc_hdl;			/* service handle */
	WORD	svcUUID; 	/* uuid  NAP or GN */

	WORD    secu_desc;   /* security description */
	WORD    net_access_type; /* net access type */
	DWORD   max_access_rate; /* maximum possible network access data rate */

	WORD	type_num;   /* number of supported network packet type */
	WORD	type_list[MAX_NET_TYPE_NUM];  /* list of supported network packet type */
};

struct PAN_ClntLocalAttr{
	WORD	cbSize;
	WORD	local_UUID;
	UINT32 total_bandwidth;					/* out param, Kbits */
	UINT32 max_guaranteed_bandwidth;		/* out param, Kbits */
	UINT8  tl_type;							/* out param, etc: HCI_DEVICE_TYPE_BREDR */	
};

struct PAN_SvrConnInd {
	WORD clsid;	
	UCHAR bd_addr[BDLENGTH];
	UCHAR *hdl;
	UINT32 total_bandwidth;					/* Kbits */
	UINT32 max_guaranteed_bandwidth;		/* Kbits */
	UINT8  tl_type;							/* etc: HCI_DEVICE_TYPE_BREDR */	
};

struct PAN_ConnParamChangeInd {
	UCHAR *hdl;
	UINT32 total_bandwidth;					/* Kbits */
	UINT32 max_guaranteed_bandwidth;		/* Kbits */
	UINT8  tl_type;							/* etc: HCI_DEVICE_TYPE_BREDR */	
};

struct PAN_SetupReqStru {
	UCHAR	uuid_size;					/* the size of the uuid in the setup control packet to send */
	UCHAR	reserved1;
	WORD	local_uuid;					/* the local and remote uuid, it is one of the following: */
	WORD	remote_uuid;				/* BNEP_PANU_UUID, BNEP_NAP_UUID, BNEP_GN_UUID */
	UINT32 total_bandwidth;					/* out param, Kbits */
	UINT32 max_guaranteed_bandwidth;		/* out param, Kbits */
	UINT8  tl_type;							/* out param, etc: HCI_DEVICE_TYPE_BREDR */	
};

struct PAN_NptReqStru {
	WORD	ranges_num;					/* the number of the ranges array */
	WORD	reserved1;
	struct NptRangeStru {
		WORD	start;					/* networking protocol type start */
		WORD	end;					/* networking protocol type end */
	}ranges[1]; /* networking protocol type range structure */
};

struct PAN_MultiAddrReqStru {
	WORD	ranges_num;					/* the number of the ranges array */	
	WORD	reserved1;
	struct MultiAddrStru {
		UCHAR	start[MAC_ADDR_LEN];	/* multicast address start */
		UCHAR	end[MAC_ADDR_LEN];		/* multicast address end */
	}ranges[1];/* multicast address range structure */
};

struct PAN_ReservedCtrlReqStru {
	UCHAR	control_type;				/* the type of BNEP control message, it is not defined in the current BNEP Spec */
	UCHAR	length;						/* the length of the unknown control payload */
	UCHAR	payload[1];	
};

/* the common functions for PAN */
DWORD PAN_Init(void);
void PAN_Done(void);
const char* PAN_GetVersion(void);
void PANAPP_RegCbk(UCHAR *handle, UCHAR* event_cbk);
DWORD PAN_Disconn(UCHAR *handle);

WORD PAN_Setup_Connection(UCHAR *handle, struct PAN_SetupReqStru *in);
WORD PAN_Set_NPT_Filter(UCHAR *handle, struct PAN_NptReqStru *in);
WORD PAN_Set_MAddr_Filter(UCHAR *handle, struct PAN_MultiAddrReqStru *in);
WORD PAN_SendEthFrm(UCHAR type, struct PAN_EthFrameStru *pFrm, BOOL need_free);

/* the functions for PAN client */
#ifdef CONFIG_PAN_CLIENT
UCHAR* PAN_ClntStart(UCHAR *bd, UCHAR *sdp_attrib, UCHAR *local_attrib);
#define PAN_ClntStop(h) PAN_Disconn(h)
#else
#define PAN_ClntStart(a, b, c) 0
#define PAN_ClntStop(h) 0
#endif

/* the functions for PAN server */
#ifdef CONFIG_PAN_SERVER
UCHAR* PAN_SvrStart(UCHAR *local_attrib);
DWORD PAN_SvrStop(UCHAR *handle);
#define PAN_DiscClient(h)	PAN_Disconn(h)
#else
//#define PAN_SvrStart(a) 0
//#define PAN_SvrStop(h) 0
//#define PAN_DiscClient(h) 0
#endif

/* the ethernet frame Tx function of the port, every port must povide this function when register */
typedef BOOL (SENDFRMCBK)(struct PAN_EthFrameStru *pFrm, HANDLE handle);

/* the functions for PAN block */
HANDLE PAN_RegExternPort(SENDFRMCBK *func);
void PAN_UnregExternPort(HANDLE hPort);

/* the functions defined by the application */
#ifdef CONFIG_BNEP_MTU_CONFIGUABLE
/* Returns the maximum payload that is to send to the BNEP. */
extern DWORD PAN_Ext_GetConfiguredMTU(void);
#endif

#ifdef CONFIG_OS_WIN32 
#include "btwin32/nat_win32_ui.h"
#endif

#endif
