/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
	sdphlp.h
Abstract:
	This module declares some auxiliary functions.
Author:
	Wu Peng
	Ye Dongxiang
Revision History:
    1.  2000.01.20  Created
    2.  2011.03.16  (CR158)
    3.  2011.04.07  (CR160)
---------------------------------------------------------------------------*/
/* Auxiliary function declaration */
#ifndef _BRTSDP_HELP_H_
#define _BRTSDP_HELP_H_

struct DataElementSeq;
struct ContinuationState;

void wAddition(UCHAR *add1, const UCHAR *add2);
struct HandleItem* FindHandleItem(DWORD hdl);
UCHAR CompareUUIDx(struct SDP_UUID *uuid1, struct SDP_UUID *uuid2);
struct UUIDItem *LocateUUID(struct SDP_UUID *uuid);

DWORD DESize(UCHAR *de, UCHAR *addi, UCHAR flag);

void ErrorRsp(struct SDP_PDU *pack, WORD flag, struct SDP_PDU *out_pack);

void GetHandles(struct UUIDItem *uuid_item, struct ListStru *res);
void SelectHandles(struct SessionUserData *user_data, struct DataElementSeq *pattern, 
				   struct ListStru *hdl_buf, WORD max);

UCHAR CompareAttrID(WORD attr_id1, struct ServiceAttributeID *attr_id2);
UCHAR SelectAttributes(struct SessionUserData *user_data, DWORD hdl, struct DataElementSeq *id_list, 
					   struct ListStru *attr_buf, DWORD *byte_count);
UCHAR AttributeRsp(struct SessionUserData *user_data, struct SDP_PDU *pack, DWORD max,
				   struct ActionInfo *info, struct SDP_PDU *out_pack);
UCHAR AttributeRsp2(struct SessionUserData *user_data, struct SDP_PDU *pack, DWORD max, 
					struct ActionInfo *info, struct SDP_PDU *out_pack);

UCHAR ServiceSearchRsp(struct SessionUserData *user_data, struct SDP_PDU *pack, struct SDP_PDU *out_pack);
UCHAR ServiceAttributeRsp(struct SessionUserData *user_data, struct SDP_PDU *pack, 
						  struct SDP_PDU *out_pack);
UCHAR ServiceSearchAttributeRsp(struct SessionUserData *user_data, struct SDP_PDU *pack, 
								struct SDP_PDU *out_pack);

void AssemblyPattern(struct SDP_SearchReqStru *info, struct DataElementSeq *pattern);
UCHAR AssemblyAttrID(struct SDP_SearchReqStru *info, struct DataElementSeq *id_list, WORD *count);
void AssemblyAttrRsp(struct SDP_PDU *out_pack, struct ActionInfo *info);
void SetContinuationState(struct SessionUserData *user_data, struct ActionInfo *info,
						  struct ContinuationState *state, UCHAR con);

void FreeSession(struct FsmInst *fi);
void FreeAttrBuf(struct ListStru *lt);
void FreeHdlAttr(struct ListStru *lt);
void FreeHdlAttrBuf(struct ListStru *lt);
void FreePacket(struct SDP_PDU *pack);
void FreeOpenParaList(struct ListStru *oplist, WORD cid, WORD result);

UCHAR ServiceResponse(struct SessionUserData *user_data, UCHAR *pdu, WORD id, WORD size);
UCHAR SdpSendPacket(struct SDP_PDU *pack, WORD id);

UCHAR PostQuery(struct FsmInst *fi, UCHAR *pdu, WORD id, WORD size);
void OverlapState(struct ContinuationState *src, struct ContinuationState *dest, struct SDP_PDU *pack);
WORD PostSSQuery(struct FsmInst *fi, struct TransInfo *trans_info, struct SDP_PDU *out_pack, UCHAR *pdu);
WORD  PostSAQuery(struct FsmInst *fi, struct TransInfo *trans_info, struct SDP_PDU *out_pack, UCHAR *pdu);

struct TransInfo *NewTrans(struct SessionUserData *user_data, struct SDP_PDU *pack, WORD req_id);
void FreeTrans(struct SessionUserData *user_data, struct TransInfo *trans_info, WORD err);
void FreeActionBuf(struct ActionInfo *info);

void SDPRegister(void);

struct DataElement* MakeDEStru(UCHAR *src, UCHAR flag);

#ifndef	CONFIG_SDP_FIXED_DATABASE
struct SDP_UUID *GetUUID(UCHAR mode, UCHAR *data);
char RegisterUUID(struct HandleItem *hdl_item, struct SDP_UUID *uuid);
char UnregisterUUID(struct HandleItem *hdl_item, struct SDP_UUID *uuid);

typedef char (uuid_func)(struct HandleItem *hdl_item,struct SDP_UUID *uuid);
void UUIDsInDESeq(struct HandleItem *hdl, UCHAR *data, DWORD size, uuid_func *func);
void RemoveUUIDs(struct HandleItem *hdl, struct ServiceAttribute_I *attr);
void AddUUIDs(struct HandleItem *hdl, struct DataElement *attr);

UCHAR *SetAttrItem(WORD id, UCHAR desc, DWORD size, const char *data);
struct AttrItem *NewAttrItem(WORD id, UCHAR desc, DWORD size, const char *data);
void SetServiceDatabaseState(void);
#endif

/* For Debug only */
/* Possible values of SDP_DBG_Data::data_type */
#define SDP_DBG_DATA_PDU_MASK           0x80    /* The data is a standard SDP PDU */
#define SDP_DBG_DATA_PARTIAL_MASK       0x40    /* The data contains partial response */
#define SDP_DBG_DATA_TYPE_MASK          0x07    /* Type of the PDU */

#define SDP_DBG_DATA_PDU_ERROR_RSP      0x81    /* Error Response PDU */
#define SDP_DBG_DATA_PDU_SS_REQ         0x82    /* ServiceSearch Request PDU */
#define SDP_DBG_DATA_PDU_SS_RSP         0x83    /* ServiceSearch Response PDU */
#define SDP_DBG_DATA_PDU_SA_REQ         0x84    /* ServiceAttribute Request PDU */
#define SDP_DBG_DATA_PDU_SA_RSP         0x85    /* ServiceAttribute Response PDU */
#define SDP_DBG_DATA_PDU_SSA_REQ        0x86    /* ServiceSearchAttribute Request PDU */
#define SDP_DBG_DATA_PDU_SSA_RSP        0x87    /* ServiceSearchAttribute Response PDU */

#define SDP_DBG_DATA_SS_HANDLE_LIST     0x03    /* The data contains all service record handles */
#define SDP_DBG_DATA_SA_ATTR_LIST       0x05    /* The data contains complete AttributeList */
#define SDP_DBG_DATA_SSA_ATTR_LISTS     0x07    /* The data contains complete AttributeLists */

void SDP_DBG_Data(UINT8 *prompt, UINT8 *data, UINT16 len, UINT8 data_type);
void SDP_DBG_ServerFullHandleList(struct ListStru *handle_list);
void SDP_DBG_ServerFullAttrList(struct ActionInfo *info);
void SDP_DBG_ServerFullAttrLists(struct ActionInfo *info);

#endif




