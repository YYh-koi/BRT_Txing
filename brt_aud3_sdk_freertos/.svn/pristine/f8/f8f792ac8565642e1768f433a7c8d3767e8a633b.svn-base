/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    btinit.h
Abstract:

Author:
    He Gang
Revision History:
    1.  2000.02.10  Created
    2.  2011.03.16  (CR158)
---------------------------------------------------------------------------*/
#ifndef BTINIT_H
#define BTINIT_H

#include "btdatatype.h"

struct AppEvConn {
	WORD clsid;
	UCHAR bd[6];	/* Remote bd address */
	UCHAR* sub_hdl; /*this is for PAN, each PAN client connection's handle*/
};

struct AppEvConnEx {
	WORD    clsid;
	UCHAR   bd[6];	                    /* Remote bd address */
	UCHAR   *sub_hdl;                   /* this is for PAN, each PAN client connection's handle */
	DWORD   total_bandwidth;			/* Total radio bandwidth available. Kbits */
	DWORD   guaranteed_bandwidth;	    /* Max bandwidth assinged to this connection. Kbits */
	UINT8   radio_type;					/* Radio type e.g. BR/EDR, 802.11. It can be One of BT_RADIO_TYPE_BREDR, BT_RADIO_TYPE_80211, etc. */	
};

struct AppEvQoSIndParam {
	UCHAR *hdl;                         /* Connection handle */
	DWORD  total_bandwidth;				/* Total radio bandwidth available. Kbits */
	DWORD  guaranteed_bandwidth;	    /* Max bandwidth assinged to this connection. Kbits */
	UCHAR  radio_type;					/* Radio type e.g. BR/EDR, 802.11. It can be One of BT_RADIO_TYPE_BREDR, BT_RADIO_TYPE_80211, etc. */	
};

/* Possible radio type - AMP controller type, 0x03 - 0xEF Reserved, 0xF0 - 0xFF Vendor-Specific */
#define BT_RADIO_TYPE_BREDR         0x00	/* Bluetooth, HCI_DEVICE_TYPE_BREDR */
#define BT_RADIO_TYPE_80211         0x01	/* WIFI, AMP_CONTROLLER_TYPE_80211  */
#define BT_RADIO_TYPE_ECMA368       0x02	/* UWB, AMP_CONTROLLER_TYPE_ECMA368 */
#define BT_RADIO_TYPE_FAKEPAL_UDP   0xEF    /* UDP Fake PAL, AMP_CONTROLLER_TYPE_FAKEPAL_UDP */

#define IS_SERVEREV(a)	(a&0x8000)
#define APP_EVENT(a)	(a&0x7fff)
#define SERVER_EV(a)	(a|0x8000)
#define BTAPP_EV_CONN		0x01	/* Connection is setup. Param is a pointer to AppEvConn structure*/
#define BTAPP_EV_DISC		0x02	/* Connection is disconected. Param is not used */
#define BTAPP_EV_START		0x03	/* Start server.  Param is not used */
#define BTAPP_EV_STOP		0x04	/* Stop server. Param is not used */
#define BTAPP_EV_PREV_DISC	0x05
#define BTAPP_EV_SEND		0x05
#define BTAPP_EV_RECEIVE	0x06
#define BTAPP_EV_CONN_CFM	0x07	/* Confirmation to a connection request. Param is a pointer to AppEvConn structure */
#define BTAPP_EV_DISC_CFM   0x08	/* Confirmation to a disconnection request. Param is not used */
#define BTAPP_EV_EX_CONN	    0x09	/* Connection is setup. Param is a pointer to AppEvConnEx structure */
#define BTAPP_EV_EX_CONN_CFM	0x0A    /* Confirmation to a connection request. Param is a pointer to AppEvConnEx structure */
#define BTAPP_EV_QOS_CHANGED    0x0B    /* Qos setting for the connection is changed. Param is a pointer to AppEvQoSIndParam structure */

/* PAN specific event */
#define BTAPP_EV_PAN_BASE		0x100
#define BTAPP_EV_PAN_CONN		(BTAPP_EV_PAN_BASE+0)
#define BTAPP_EV_PAN_DISC		(BTAPP_EV_PAN_BASE+1)
/* decide if we are BRT NAP, param is not used, if we are not BRT NAP, return 0, else 1 */
#define BTAPP_EV_PAN_IS_BRTNAP	(BTAPP_EV_PAN_BASE+2) 
/* config indication for BRT NAP, param is a pointer to pan_nap_config structure, if we are not BRT nap, return 0, else 1 */
#define BTAPP_EV_PAN_NAP_CONFIG (BTAPP_EV_PAN_BASE+3) 
#define BTAPP_EV_PAN_CONNPARAM_CHANGE	(BTAPP_EV_PAN_BASE+4)

/* OPP specific event */
#define BTAPP_EV_OPP_BASE       0x200
#define BTAPP_EV_OPP_CONN		(BTAPP_EV_OPP_BASE+0)
#define BTAPP_EV_OPP_DISC		(BTAPP_EV_OPP_BASE+1)
#define BTAPP_EV_OPP_PULL		(BTAPP_EV_OPP_BASE+2)
#define BTAPP_EV_OPP_PUSH		(BTAPP_EV_OPP_BASE+3)
#define BTAPP_EV_OPP_PUSH_CARD	(BTAPP_EV_OPP_BASE+4)
#define BTAPP_EV_OPP_EXCHG		(BTAPP_EV_OPP_BASE+5)

/* FTP specific event */
#define BTAPP_EV_FTP_BASE		0x300
#define BTAPP_EV_FTP_PUT		(BTAPP_EV_FTP_BASE+0)
#define BTAPP_EV_FTP_GET        (BTAPP_EV_FTP_BASE+1)
#define BTAPP_EV_FTP_BROWSE		(BTAPP_EV_FTP_BASE+2)
#define BTAPP_EV_FTP_DEL_FILE   (BTAPP_EV_FTP_BASE+3)
#define BTAPP_EV_FTP_DEL_FODLER	(BTAPP_EV_FTP_BASE+4)	
#define BTAPP_EV_FTP_CREATE		(BTAPP_EV_FTP_BASE+5)
#define BTAPP_EV_FTP_SET_CUR	(BTAPP_EV_FTP_BASE+6)
#define BTAPP_EV_FTP_SET_ROOT	(BTAPP_EV_FTP_BASE+7)
#define BTAPP_EV_FTP_ABORT		(BTAPP_EV_FTP_BASE+8)
#define BTAPP_EV_FTP_BACK		(BTAPP_EV_FTP_BASE+9)
#define BTAPP_EV_FTP_CONN		(BTAPP_EV_FTP_BASE+10)

/* SYNC specific event */
#define BTAPP_EV_SYNC_BASE		0x400
#define BTAPP_EV_SYNC_START		(BTAPP_EV_SYNC_BASE+0)
#define BTAPP_EV_SYNC_CHALOG	(BTAPP_EV_SYNC_BASE+1)
#define BTAPP_EV_SYNC_DEVINFO	(BTAPP_EV_SYNC_BASE+2)
#define BTAPP_EV_SYNC_CC		(BTAPP_EV_SYNC_BASE+3)
#define BTAPP_EV_SYNC_INFOLOG	(BTAPP_EV_SYNC_BASE+4)
#define BTAPP_EV_SYNC_STREAM	(BTAPP_EV_SYNC_BASE+5)
#define BTAPP_EV_SYNC_LUID		(BTAPP_EV_SYNC_BASE+6)
#define BTAPP_EV_SYNC_OK		(BTAPP_EV_SYNC_BASE+7)
#define BTAPP_EV_SYNC_ADD		(BTAPP_EV_SYNC_BASE+8)
#define BTAPP_EV_SYNC_MODIFY	(BTAPP_EV_SYNC_BASE+9)
#define BTAPP_EV_SYNC_DELETE	(BTAPP_EV_SYNC_BASE+10)
#define BTAPP_EV_SYNC_PREV_CHALOG	(BTAPP_EV_SYNC_BASE+11)


/* HCRP specific event */
#define BTAPP_EV_HCRP_BASE			0x500
#define BTAPP_EV_HCRP_NOTIF			(BTAPP_EV_HCRP_BASE+1)
#define BTAPP_EV_HCRP_REG_NOTIF		(BTAPP_EV_HCRP_BASE+2)
#define BTAPP_EV_HCRP_GET_LPTSTAT   (BTAPP_EV_HCRP_BASE+3)
#define BTAPP_EV_HCRP_GET_1284ID    (BTAPP_EV_HCRP_BASE+4)
#define BTAPP_EV_HCRP_SOFT_RESET    (BTAPP_EV_HCRP_BASE+5)
#define BTAPP_EV_HCRP_HARD_RESET    (BTAPP_EV_HCRP_BASE+6)
#define BTAPP_EV_HCRP_DATA          (BTAPP_EV_HCRP_BASE+7)
#define BTAPP_EV_HCRP_RECEIVE_DATA  (BTAPP_EV_HCRP_BASE+8)
#define BTAPP_EV_HCRP_SEND_DATA     (BTAPP_EV_HCRP_BASE+9)


/* HID specific event */
#define BTAPP_EV_HID_BASE 			0x600
#define BTAPP_EV_HID_CONN		(BTAPP_EV_HID_BASE+1)
#define BTAPP_EV_HID_UNPLUG		(BTAPP_EV_HID_BASE+2)

/* BIP specific event */
#define BTAPP_EV_BIP_BASE			0x700
#define BTAPP_EV_BIP_START_ARCHIVE	(BTAPP_EV_BIP_BASE+0)
#define BTAPP_EV_BIP_ADV_PRINT		(BTAPP_EV_BIP_BASE+1)
#define BTAPP_EV_BIP_START_PRINT	(BTAPP_EV_BIP_BASE+2)
#define BTAPP_EV_BIP_GET_MON		(BTAPP_EV_BIP_BASE+3)
#define BTAPP_EV_BIP_STORE_IMG		(BTAPP_EV_BIP_BASE+4)
#define BTAPP_EV_BIP_REM_DISP		(BTAPP_EV_BIP_BASE+5)
#define BTAPP_EV_BIP_CONN			(BTAPP_EV_BIP_BASE+6)
#define BTAPP_EV_BIP_DISCONN		(BTAPP_EV_BIP_BASE+7)
#define BTAPP_EV_BIP_ABORT			(BTAPP_EV_BIP_BASE+8)
#define BTAPP_EV_BIP_NORMAL			(BTAPP_EV_BIP_BASE+9)

/*BPP specific event*/
#define BTAPP_EV_BPP_BASE			0x800
#define BTAPP_EV_BPP_CONN			(BTAPP_EV_BPP_BASE + 0)
#define BTAPP_EV_BPP_DISCONN		(BTAPP_EV_BPP_BASE + 1)
#define BTAPP_EV_BPP_PUT			(BTAPP_EV_BPP_BASE + 2)

/*MAP specific event*/
#define BTAPP_EV_MAP_BASE			0x900
#define BTAPP_EV_MAP_CONN			(BTAPP_EV_MAP_BASE+0)
#define BTAPP_EV_MAP_DISCONN		(BTAPP_EV_MAP_BASE+1)
#define BTAPP_EV_MAP_ABORT			(BTAPP_EV_MAP_BASE+2)
#define BTAPP_EV_MAP_ROOT_DIR		(BTAPP_EV_MAP_BASE+7)
#define BTAPP_EV_MAP_CUR_DIR		(BTAPP_EV_MAP_BASE+8)
#define BTAPP_EV_MAP_BACK_DIR		(BTAPP_EV_MAP_BASE+9)
#define BTAPP_EV_MAP_SEND_EVENT		(BTAPP_EV_MAP_BASE+11)
#define BTAPP_EV_MAP_SETNOTIFYREG	(BTAPP_EV_MAP_BASE+12)
#define BTAPP_EV_MAP_UPDATEINBOX	(BTAPP_EV_MAP_BASE+13)
#define BTAPP_EV_MAP_SETMSGSTATUS	(BTAPP_EV_MAP_BASE+14)
#define BTAPP_EV_MAP_PUSHMSG		(BTAPP_EV_MAP_BASE+15)

/*A2DP specific event*/
#define BTAPP_EV_A2DP_BASE					0xA00
#define	BTAPP_EV_A2DP_REGSEP				(BTAPP_EV_A2DP_BASE + 0x01)
#define	BTAPP_EV_A2DP_SEPDISCOVERY			(BTAPP_EV_A2DP_BASE + 0x02)
#define	BTAPP_EV_A2DP_GETCAPABILITIES		(BTAPP_EV_A2DP_BASE + 0x03)
#define	BTAPP_EV_A2DP_SETCONFIGURATION		(BTAPP_EV_A2DP_BASE + 0x04)
#define	BTAPP_EV_A2DP_GETCONFIGURATION		(BTAPP_EV_A2DP_BASE + 0x05)
#define	BTAPP_EV_A2DP_STREAMOPEN			(BTAPP_EV_A2DP_BASE + 0x06)
#define	BTAPP_EV_A2DP_STREAMSTART			(BTAPP_EV_A2DP_BASE + 0x07)
#define	BTAPP_EV_A2DP_STREAMSUSPEND			(BTAPP_EV_A2DP_BASE + 0x08)
#define	BTAPP_EV_A2DP_STREAMCLOSE			(BTAPP_EV_A2DP_BASE + 0x09)
#define	BTAPP_EV_A2DP_STREAMABORT			(BTAPP_EV_A2DP_BASE + 0x0a)
#define	BTAPP_EV_A2DP_RECONFIGURE			(BTAPP_EV_A2DP_BASE + 0x0b)/* this event cannot be used, use BTSDK_APP_EV_A2DP_SETCONFIGURATION, by lll  */
#define	BTAPP_EV_A2DP_DATAIND				(BTAPP_EV_A2DP_BASE + 0x0c)
#define	BTAPP_EV_A2DP_STREAMDONE			(BTAPP_EV_A2DP_BASE + 0x0d)
#define BTAPP_MAX_EVENT			10000

typedef UCHAR (StatusInd)(WORD event, UCHAR * hdl, UCHAR * param);

void BtInitProfile(void);
void BtDoneProfile(void);
void BtInitEnv(void);
void BtInitStack(void);
void BtDoneStack(void);

void BtInit(void);
void BtDone(void);

typedef UCHAR * HDLTYPE;

#ifdef CONFIG_GAP
#include "gap_ui.h"
#endif

#ifdef CONFIG_SDAP
#include "sdap_ui.h"
#endif

#ifdef CONFIG_SPP
//#include "spp_ui.h"
#endif

#ifdef CONFIG_DUN
//#include "dun_ui.h"
#endif

#ifdef CONFIG_FAX
#include "fax_ui.h"
#endif

#ifdef CONFIG_OPP
#include "opp_ui.h"
#endif

#ifdef CONFIG_FTP
#include "ftp_ui.h"
#endif

#ifdef CONFIG_IRMC
#include "sync_ui.h"
#endif

#ifdef CONFIG_BIP
#include "bip_ui.h"
#endif

#ifdef CONFIG_PAN
#include "pan_ui.h"
#endif

#ifdef CONFIG_HCRP
#include "hcrp_ui.h"
#endif

#ifdef CONFIG_HID
#include "hid_ui.h"
#endif

#ifdef CONFIG_AVCTP
#include "avctp_ui.h"
#endif

#ifdef CONFIG_AVRCP
#include "avrcp_ui.h"
#endif

#ifdef CONFIG_A2DP
#include "a2dp_ui.h"
#endif

#ifdef CONFIG_VAUDIO
#include "a2dputil_ui.h"
#endif

#ifdef CONFIG_HFP
#include "hfp_ui.h"
#endif

#ifdef CONFIG_VDP
#include "vdp_ui.h"
#endif

#ifdef CONFIG_SAP
#include "sap_ui.h"
#endif

#ifdef CONFIG_BPP
#include "bpp_ui.h"
#endif

#ifdef CONFIG_BIP
#include "bip_ui.h"
#endif

#ifdef CONFIG_PBAP
#include "pbap_ui.h"
#endif

#ifdef CONFIG_MAP
#include "map_ui.h"
#endif

#endif
