/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	hid_bredr.h
Abstract:
Author:   
	Song Xiaowei
-----------------------------------------------------------------------------*/
#ifndef HID_BREDR_H
#define HID_BREDR_H

#include "hid_hlp.h"


#define HIDH_L2CAP_CONTROL_CHANNEL		0x1
#define HIDH_L2CAP_INTERRUPT_CHANNEL	0x2

/* HID Transaction header bytes len */
#define HIDH_L2CAP_THDR_LEN				0x1
#define HIDH_L2CAP_REPORT_ID_LEN		0x1
#define HIDH_L2CAP_REPORT_BUF_LEN		0x2
#define HIDH_L2CAP_IDLE_RATE_LEN		0x1
#define HIDH_L2CAP_PROT_LEN				0x01
#define HIDH_L2CAP_REQ_MASK				0xF0

/*reserved buff lenth*/
#define HIDH_L2CAP_BUF_RES_LEN			(HCI_RESERVE + L2CAP_CO_RESERVE)
/*define HID buf created Macro*/
#define HIDH_L2CAP_BUFFNEW(len, buf) 		(buf = BuffNew((WORD)(len + HIDH_L2CAP_BUF_RES_LEN), HIDH_L2CAP_BUF_RES_LEN))

struct HIDH_BREDR_ConnInfoStru {
	struct ChannelStru {
		WORD cid;
		WORD mtu;
	} ctrl_ch, int_ch;
};

void HID_BREDR_RegChannel();
void HID_BREDR_UnregChannel();
void HID_BREDR_ConnReq(struct HIDH_ConnInfoStru *session);
UINT8 HID_BREDR_ConnComplete(struct HIDH_BREDR_ConnInfoStru *conn_info, struct L2CAP_ConnectCompleteStru *conn_cfm, UINT16 *res);
void HID_BREDR_ConnInd(struct HIDH_ConnInfoStru *session, struct L2CAP_ConnectStru *conn_ind);
void HID_BREDR_DisconnReq(struct HIDH_BREDR_ConnInfoStru *conn_info);
UINT8 HID_BREDR_DisconnComplete(struct HIDH_ConnInfoStru *session, struct L2CAP_DisconnectCompleteStru *in);
void HID_BREDR_MsgReq(struct HIDH_BREDR_ConnInfoStru *conn_info, struct HIDH_MsgCommonStru *msg);
void *HID_BREDR_MsgDecode(struct BuffStru *buf, UINT8 *msg_type);

#endif /* HID_BREDR_H */
