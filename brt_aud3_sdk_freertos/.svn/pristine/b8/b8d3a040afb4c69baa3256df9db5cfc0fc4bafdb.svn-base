/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    g_sdp.h
Abstract:
	This module defines all the global variable used in SDP and the callback 
	functions for L2CAP.
Author:
    Wu Peng
Revision History:
    1.  2000.01.20  Created
    2.  2011.03.16  (CR158)
    3.  2012.05.31  (CR177)
---------------------------------------------------------------------------*/

/*	Global variable and function declaration */
#ifndef _BRTSDP_GLOBAL_H_
#define _BRTSDP_GLOBAL_H_

#include "global.h"

#define SDP_HANDLE_BASE		(((DWORD)PROT_SDP) << 24)
#define IDLE_TIMER			SDP_HANDLE_BASE + 0

#define DEFAULTMAXCOUNT		16
#define DEFAULTMAXBYTECOUNT	128
#define DEFAULTDURATION		10000

#define SDP_CLIENT			0
#define SDP_SERVER			1

#define ATTR_ID_DESC		0x09
#define TRANS_TYPE_MASK		0x0000000F
#define SDP_PDU_HEADER_MASK	0x29
#define TYPE_DESC(d)		(((d) & 0xF8) >> 3)
#define SIZE_DESC(d)		((d) & 0x7)

#undef Encode
#undef Decode
#define Encode(a,b,c)		EncodeNew(a,b,c,1)
#define Decode(a,b,c)		DecodeNew(a,b,c,1)

#define DBG_LEAVE_VOID		{DBG_LEAVE(PROT_SDP); return;}
#define DBG_LEAVE_SD(res)	{DBG_LEAVE(PROT_SDP); return (res);}

#define _DELENGTH(len)		(5 + (len ? len : 1)) /* 5 == sizeof(DataElement.size) + sizeof(DataElement.descriptor) */
#define DELENGTH(len)		(((_DELENGTH(len) + 3) >> 2) << 2)	
#define _ATTRLENGTH(len)	(4 + _DELENGTH(len)) /* 4 == sizeof(ServiceAttribute.attribute_id) + sizeof(ServiceAttribute.align_byte)*/
#define ATTRLENGTH(len)		(4 + DELENGTH(len))	

#define CONTROL_USER_DATA	((struct CtrlUserData *)(sdp_fsm_inst->user_data))
#ifndef CONFIG_SDP_FIXED_DATABASE
#define G_STATE				(CONTROL_USER_DATA->state)
#define G_STATE_LINK		(CONTROL_USER_DATA->state_link)
#else
#define G_STATE
#define G_STATE_LINK
#endif

#define ARG_UI_REQ			((struct SDP_UIReqParamStru *)arg)
#define ARG_REQ_ID			(ARG_UI_REQ->req_id)
#define ARG_BDADDR			(ARG_UI_REQ->in_param)
#define ARG_CID				((WORD *)ARG_UI_REQ->in_param)
#define ARG_SRCH			((struct SDP_SearchReqStru *)ARG_UI_REQ->in_param)
#define ARG_DATA			((struct BuffStru *)arg)
#define ARG_CONNCFMSTRU		((struct L2CAP_ConnectCompleteStru*)arg)

#define CFM_SRCHCFM			((struct SDP_SearchCfmStru *)cfm->out_param)
#define CFM_SRCHCFM_CNT		(CFM_SRCHCFM->count)
#define CFM_SRCHCFM_VAR		(CFM_SRCHCFM->var)

#define RSP_PDU_SS(pack)	((struct SSRspPack *)(pack->var))
#define RSP_PDU_SA(pack)	((struct SARspPack *)(pack->var))
#define REQ_PDU_SS(pack)	((struct SSReqPack *)(pack->var))	
#define REQ_PDU_SA(pack)	((struct SAReqPack *)(pack->var))	
#define REQ_PDU_SSA(pack)	((struct SSAReqPack *)(pack->var))	

#define UUID_128_MASK	0x15556B
#define UUID_64_MASK	0xF
#define UUID_32_MASK	0x3
#define UUID_16_MASK	0x2

#define INT_128_MASK	0x15556B
#define INT_64_MASK		0xF
#define INT_32_MASK		0x3
#define INT_16_MASK		0x2

#define STRU_16_16_MASK	0xA
#define STRU_32_16_MASK	0xB
#define STRU_32_32_MASK	0xF

#define SDP_PDU_ID_ERR_RSP	1
#define SDP_PDU_ID_SS_REQ	2
#define SDP_PDU_ID_SS_RSP	3
#define SDP_PDU_ID_SA_REQ	4
#define SDP_PDU_ID_SA_RSP	5
#define SDP_PDU_ID_SSA_REQ	6
#define SDP_PDU_ID_SSA_RSP	7

#define SDP_PDU_HEADER_LEN	5	

#define CONTROL_USER_DATA_FI(fi)	((struct CtrlUserData *)(fi->user_data))
#define SESSION_USER_DATA(fi)		((struct SessionUserData *)(fi->user_data))

#define BE_STREAM_TO_UINT16(p)		(((WORD)(p)[0] << 8) + (WORD)(p)[1])
#define BE_STREAM_TO_UINT32(p)		(((DWORD)(p)[0] << 24) + ((DWORD)(p)[1] << 16) + ((DWORD)(p)[2] << 8) + (DWORD)(p)[3])

#include "srvdb.h"

/*user data defined for the SDP control fsm instance*/
struct CtrlUserData {
#ifndef CONFIG_SDP_FIXED_DATABASE
	struct ListStru	hhead;				/*head of the handle table*/
	struct ListStru uhead;				/*head of the uuid table*/
	DWORD  state;						/*value of ServiceRecordState attribute.*/
	struct AttrItem *state_link;		/*pointer to the ServiceDatabaseState attribute item*/
#endif
	struct ListStru	session_info_list;	/*session information list*/
};

/*information regarding each SDP session*/
struct SessionInfo {
	WORD			chan_id;		/*channel ID*/
	UCHAR			rdev[BDLENGTH];	/*BD address of the remote device*/
	struct FsmInst  *fsm_inst;		/*FSM instance*/
	struct ListStru	open_para_list; /*Open search input parameter list*/
};

/*user data defined for the SDP session fsm instance*/
struct SessionUserData {
	UCHAR		  type;				/*session type: 0---Client; 1---Server*/
	WORD		  chan_id;			/*channel ID*/
	WORD		  conn_count;		/*connectin count*/
	WORD		  trans_id;			/*transaction id*/
	WORD		  out_mtu;			/*MTU for sending PDU*/
	struct ListStru action;		    /*internal buffer: TransInfo in Client side; ActionInfo in Server side*/	
};

/*buffer for each SDP transaction in the SDP server*/
struct ActionInfo {
	UCHAR	type;					/*SDP Transaction type*/
	UCHAR	flag;					/*1 - continue, 0 - first*/
	INT32	count;					/*Total number of items (handles or attributes) found*/
	DWORD	byte_count;				/*Total size of the item (handles or attributes) list in the response PDU*/
	struct ListStru buf;			/*Item list. For SS response, each list item will be a handle value;
									  for SA response, a struct ServiceAttribute_I type variable;
									  for SSA response, a struct SSAAttrBuf type variable.*/
};

/*buffer for attribute list of one service record in an SSA response*/
struct SSAAttrBuf {
	UCHAR		  flag;				/*1 - continue, 0 - first*/	
	DWORD		  byte_count;		/*Total size of the attributes list in the response PDU*/
	struct ListStru attr_buf;		/*Attributes list*/
};

/*buffer for each SDP transaction in the SDP client*/
struct TransInfo {
	WORD		   trans_id;		/*transaction id*/
	WORD		   cs_cur;			/*current number of search result*/	
	DWORD		   cs_total;		/*total number of search result*/
	struct SDP_UICfmParamStru *cfm;	/*search response from SDP server */
	struct SDP_PDU *pack;			/*SDP request PDU*/
	struct FsmTimer *trans_timer;   /*supervision timer of this transaction*/
};

#include "l2cap_ui.h"
#include "sdpui.h"
#include "sdppdu.h"
#include "asm.h"
#include "dasm.h"
#include "sdpfsm.h"
#include "sdphlp.h"


extern struct FsmInst * sdp_fsm_inst;
extern SDPUICfm *sdp_cb_func;

#ifdef CONFIG_SDP_FIXED_DATABASE
extern const UCHAR g_attribute_tables[];
extern struct HandleItem g_handle_list[];
extern const DWORD g_uuid_list[];
#endif

void InitDatabase(struct CtrlUserData *user_data,DWORD state);
#ifndef CONFIG_SDP_FIXED_DATABASE
void FreeDatabase(struct CtrlUserData *user_data);
#endif

void SDP_L2CAConnectInd(struct L2CAP_ConnectStru *ind);
void SDP_L2CAConnectCfm(struct L2CAP_ConnectCompleteStru *ind);
void SDP_L2CADisconnectInd(struct L2CAP_DisconnectCompleteStru *ind);
void SDP_L2CAQosViolationInd(UCHAR dev[]);
void SDP_L2CADataInd(struct BuffStru *data);
void SDP_L2CACallbackFunc(WORD msgid, void *arg);	
void SDP_L2CAP_SetConfigReq(struct L2CAP_ConfigOptionStru *in);

#endif




