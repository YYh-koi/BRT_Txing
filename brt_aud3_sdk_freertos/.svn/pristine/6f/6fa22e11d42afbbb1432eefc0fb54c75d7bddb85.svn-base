/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
	sdap_uifunc.h
Abstract:
	Export UI functions.
Author:
	Ye Dongxiang
Revision History:
	1.  2002.01.27	Created by D.X. Ye
	2.  2002.01.10  Add SDAP_Search and SDAP_SearchEx.
    				Redefine SDAP_OpenSearch, SDAP_CloseSearch, SDAP_ServiceBrowse, 
    				SDAP_ServiceSearch, SDAP_GetServiceAttibute.
	3.  2007.05.21  Support Basic Print Profile. By D.X.Ye
	4.  2007.09.07  Support Phone Book Access Profile. By D.X.Ye.
	5.  2009.12.10  (CR148)
	6.  2010.06.25  (CR151)
-------------------------------------------------------------------------------------------------*/
#ifndef _BT_SDAP_UIFUNC_H
#define _BT_SDAP_UIFUNC_H
#include "btdatatype.h"
#include "sdpuistru.h"

#define SDAPSVCATTRSTRULEN			9	/*sizeof(attr_id) + sizeof(align_byte) + sizeof(attr_val.size) + sizeof(attr_val.descriptor)*/
#define _SDAPSVCATTRSTRULEN(attr)		((WORD)(SDAPSVCATTRSTRULEN + ((attr)->attr_val.size ? (attr)->attr_val.size : 1) + (attr)->align_byte))

/*++++++++++++++++++++ SDAP Service Register/Unregister UI Definition ++++++++++++++++++++*/
#ifdef CONFIG_SDAP
DWORD	SDAP_Init(void);
void	SDAP_Done(void);
const char* SDAP_GetVersion(void);
UCHAR* SDAP_MallocMemory(int size);
void SDAP_FreeMemory(UCHAR *buffer);
#endif

#ifdef CONFIG_SDAP_SERVER
WORD SDAP_AllocateSvcHdl(DWORD *handle);
WORD SDAP_RegisterAttribute(DWORD *svc_hdl,struct SDAP_SvcAttrStru *attr);
WORD SDAP_ServiceUnregister(DWORD svc_hdl);

struct SDAP_SvcAttrStru     *MakeUIntA(WORD id, UCHAR mode, const UCHAR *val);
struct SDAP_DataEleStru   	*MakeUIntDE(UCHAR mode, const UCHAR *val);
struct SDAP_SvcAttrStru  	*MakeIntA(WORD id, UCHAR mode, const UCHAR *val);
struct SDAP_DataEleStru	    *MakeIntDE(UCHAR mode, const UCHAR *val);
struct SDAP_SvcAttrStru  	*MakeUUIDA(WORD id, UCHAR mode, const struct SDAP_UUIDStru *val);
struct SDAP_DataEleStru      *MakeUUIDDE(UCHAR mode, const struct SDAP_UUIDStru *val);
struct SDAP_SvcAttrStru  	*MakeBoolA(WORD id, UCHAR mode, const UCHAR *val);
struct SDAP_DataEleStru      *MakeBoolDE(UCHAR mode, const UCHAR *val);
struct SDAP_SvcAttrStru  	*MakeStringA(WORD id, const UCHAR *src);
struct SDAP_DataEleStru      *MakeStringDE(const UCHAR *src);
struct SDAP_SvcAttrStru  	*MakeURLA(WORD id, const UCHAR *src);
struct SDAP_DataEleStru      *MakeURLDE(const UCHAR *src);
struct SDAP_SvcAttrStru    	*MakeDESeqA(WORD id, const char *szTypes,...);
struct SDAP_DataEleStru      *MakeDESeqDE(const char *szTypes,...);
struct SDAP_SvcAttrStru  	*MakeDEAltA(WORD id, const char *szTypes,...);
struct SDAP_DataEleStru      *MakeDEAltDE(const char *szTypes,...);


WORD SDAP_ModifyStringAttrib(DWORD svc_hdl, UCHAR *str_attrib, WORD attrib_id);
WORD SDAP_ModifySvcAvailAttrib(DWORD svc_hdl, UCHAR cur_clis, UCHAR max_clis);
WORD SDAP_RegisterGroup(DWORD *svc_hdl, DWORD mask, struct SDAP_UUIDStru *grp_id);

#ifdef CONFIG_TCS_GW
WORD SDAP_RegisterCTPService(UCHAR *svc_name, struct SDAP_CTPInfoStru *reg_info);
#else
#define SDAP_RegisterCTPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_CTP_ICP
WORD SDAP_RegisterICPService(UCHAR *svc_name, struct SDAP_ICPInfoStru *reg_info);	
#else 
#define SDAP_RegisterICPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_SPP_SERVER
WORD SDAP_RegisterSPPService(UCHAR *svc_name, struct SDAP_SPPInfoStru *reg_info);
#else
#define SDAP_RegisterSPPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_SAP_SERVER
WORD SDAP_RegisterSAPService(UCHAR *svc_name, struct SDAP_SAPInfoStru *reg_info);
#else
#define SDAP_RegisterSAPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#if (defined(CONFIG_HEP_HS) || defined(CONFIG_HFP_HF))
WORD SDAP_RegisterHEPHSService(UCHAR *svc_name, struct SDAP_HEPHSInfoStru *reg_info);
#else
#define SDAP_RegisterHEPHSService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#define SDAP_RegisterHEPService(name, reg)			SDAP_RegisterHEPHSService(name, reg)
#if (defined(CONFIG_HEP_AG) || defined(CONFIG_HFP_AG))
WORD SDAP_RegisterHEPAGService(UCHAR *svc_name, struct SDAP_HEPAGInfoStru *reg_info);
#else
#define SDAP_RegisterHEPAGService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_HFP_HF
WORD SDAP_RegisterHFPHFService(UCHAR *svc_name, struct SDAP_HFPHFInfoStru *reg_info);
#else
#define SDAP_RegisterHFPHFService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#define SDAP_RegisterHFPService(name, reg)			SDAP_RegisterHFPHFService(name, reg)
#ifdef CONFIG_HFP_AG
WORD SDAP_RegisterHFPAGService(UCHAR *svc_name, struct SDAP_HFPAGInfoStru *reg_info);
#else
#define SDAP_RegisterHFPAGService(name, reg)	 	(ERROR_SDAP_API_UNAVAILABLE)	
#endif
#ifdef CONFIG_DUN_SERVER
WORD SDAP_RegisterDUNService(UCHAR *svc_name, struct SDAP_DUNInfoStru *reg_info);
#else
#define SDAP_RegisterDUNService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_FAX_SERVER
WORD SDAP_RegisterFaxService(UCHAR *svc_name, struct SDAP_FaxInfoStru *reg_info);
#else
#define SDAP_RegisterFaxService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_LAP_SERVER
WORD SDAP_RegisterLAPService(UCHAR *svc_name, struct SDAP_LAPInfoStru *reg_info);
#else
#define SDAP_RegisterLAPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_OPP_SERVER
WORD SDAP_RegisterOPPService(UCHAR *svc_name, struct SDAP_OPPInfoStru *reg_info);
#else
#define SDAP_RegisterOPPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_FTP_SERVER
WORD SDAP_RegisterFTPService(UCHAR *svc_name, struct SDAP_FTPInfoStru *reg_info);
#else
#define SDAP_RegisterFTPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_IRMC_SERVER
WORD SDAP_RegisterSyncService(UCHAR *svc_name, struct SDAP_SyncInfoStru *reg_info);
#else
#define SDAP_RegisterSyncService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_IRMC_CLIENT
WORD SDAP_RegisterSyncCmdService(UCHAR *svc_name, struct SDAP_SyncCmdInfoStru *reg_info);
#else
#define SDAP_RegisterSyncCmdService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_BIP_SERVER
WORD SDAP_RegisterBIPImgService(UCHAR *svc_name, struct SDAP_BIPImgInfoStru *reg_info);
#else
#define SDAP_RegisterBIPImgService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_BIP_CLIENT
WORD SDAP_RegisterBIPObjService(UCHAR *svc_name, struct SDAP_BIPObjInfoStru *reg_info);
#else
#define SDAP_RegisterBIPObjService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_PAN_SERVER
WORD SDAP_RegisterPANService(UCHAR *svc_name, struct SDAP_PANInfoStru *reg_info);
#define SDAP_RegisterGNService(name, reg)		SDAP_RegisterPANService(name, reg); 
#define SDAP_RegisterNAPService(name, reg)		SDAP_RegisterPANService(name, reg);
#define SDAP_RegisterPANUService(name, reg)		SDAP_RegisterPANService(name, reg); 
#else
#define SDAP_RegisterPANService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterNAPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterGNService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterPANUService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)	
#endif
#ifdef CONFIG_AVRCP
WORD SDAP_RegisterAVRCPService(UCHAR *svc_name, struct SDAP_AVRCPInfoStru *reg_info);
#define SDAP_RegisterAVCTService(name, reg)		SDAP_RegisterAVRCPService(name, reg); 
#define SDAP_RegisterAVTGService(name, reg)		SDAP_RegisterAVRCPService(name, reg); 
#else
#define SDAP_RegisterAVRCPService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterAVCTService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterAVTGService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)	
#endif
#ifdef CONFIG_A2DP
WORD SDAP_RegisterA2DPService(UCHAR *svc_name, struct SDAP_A2DPInfoStru *reg_info);
#define SDAP_RegisterA2DPSrcService(name, reg)		SDAP_RegisterA2DPService(name, reg); 
#define SDAP_RegisterA2DPSnkService(name, reg)		SDAP_RegisterA2DPService(name, reg); 
#else
#define SDAP_RegisterA2DPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterA2DPSrcService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterA2DPSnkService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)	
#endif
#ifdef CONFIG_HCRP
WORD SDAP_RegisterHCRPService(UCHAR *svc_name, struct SDAP_HCRPInfoStru *reg_info);
#define SDAP_RegisterHCRPCtrlService(name, reg)		SDAP_RegisterHCRPService(name, reg); 
#define SDAP_RegisterHCRPNotifyService(name, reg)	SDAP_RegisterHCRPService(name, reg); 
#else
#define SDAP_RegisterHCRPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterHCRPCtrlService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterHCRPNotifyService(name, reg)	(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_HID_SERVER
WORD SDAP_ModifyHIDDescListAttr(DWORD svc_hdl, UCHAR *desc_list, DWORD list_size);
WORD SDAP_RegisterHIDService(UCHAR *svc_name, struct SDAP_HIDInfoStru *reg_info);
#else
#define SDAP_RegisterHIDService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_ModifyHIDDescListAttr					(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_DI_SERVER
WORD SDAP_RegisterDIService(UCHAR *svc_name, struct SDAP_DIInfoStru *reg_info);
#else
#define SDAP_RegisterDIService(name, reg)				(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_VDP
WORD SDAP_RegisterVDPService(UCHAR *svc_name, struct SDAP_VDPInfoStru *reg_info);
#define SDAP_RegisterVDPSrcService(name, reg)		SDAP_RegisterVDPService(name, reg); 
#define SDAP_RegisterVDPSnkService(name, reg)		SDAP_RegisterVDPService(name, reg); 
#else
#define SDAP_RegisterVDPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterVDPSrcService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterVDPSnkService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)	
#endif

#ifdef CONFIG_BPP_PRINTER
WORD SDAP_RegisterBPPPrinterService(UCHAR *svc_name, struct SDAP_BPPPrinterInfoStru *reg_info);
WORD SDAP_RegisterBPPRUIService(UCHAR *svc_name, struct SDAP_BPPRUIInfoStru *reg_info);
#else
#define SDAP_RegisterBPPPrinterService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterBPPRUIService(name, reg)				(ERROR_SDAP_API_UNAVAILABLE)	
#endif

#ifdef CONFIG_BPP_SENDER
WORD SDAP_RegisterBPPObjService(UCHAR *svc_name, struct SDAP_BPPObjInfoStru *reg_info);
#else
#define SDAP_RegisterBPPObjService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_PBAP_CLIENT
WORD SDAP_RegisterPBAPPCEService(UCHAR *svc_name, struct SDAP_PBAPPCEInfoStru *reg_info);
#else
#define SDAP_RegisterPBAPPCEService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_PBAP_SERVER
WORD SDAP_RegisterPBAPPSEService(UCHAR *svc_name, struct SDAP_PBAPPSEInfoStru *reg_info);
#else
#define SDAP_RegisterPBAPPSEService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_HDP_SERVER
struct SDAP_HDPInfoStru *SDAP_EncodeHDPSvcInfoStru(struct SDAP_HDPInfoHlpStru *src_info);
WORD SDAP_ModifyHDPSupportedFeaturesAttr(DWORD svc_hdl, WORD mdep_entry_num, UCHAR *mdep_entry_list);
WORD SDAP_RegisterHDPService(UCHAR *svc_name, struct SDAP_HDPInfoStru *reg_info);
#else
#define SDAP_EncodeHDPSvcInfoStru(src)              (NULL)
#define SDAP_ModifyHDPSupportedFeaturesAttr(hdl, num, list)    (ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterHDPService(name, reg)		    (ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_MAP_MSE
WORD SDAP_RegisterMAPMASService(UCHAR* svc_name, struct SDAP_MAPMASInfoStru* reg_info);
#else 
#define SDAP_RegisterMAPMASService(name, reg)	(ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_MAP_MCE
WORD SDAP_RegisterMAPMNSService(UCHAR* svc_name, struct SDAP_MAPMNSInfoStru* reg_info);
#else
#define SDAP_RegisterMAPMNSService(name, reg)	(ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_ATT
WORD SDAP_RegisterGATTService(UCHAR *svc_name, struct SDAP_GATTInfoStru *reg_info);
#else
#define SDAP_RegisterGATTService(name, reg)	    (ERROR_SDAP_API_UNAVAILABLE)
#endif

#endif

/*++++++++++++++++++++++++ SDAP Service Search UI Definition +++++++++++++++++++++++++*/
#ifdef CONFIG_SDAP_CLIENT
WORD SDAP_OpenSearch(UCHAR* bd, WORD *trans_hdl);
WORD SDAP_CloseSearch(WORD trans_hdl);
WORD SDAP_Search(struct SDP_SearchReqStru *sreq, struct SDP_SearchCfmStru *scfm);
WORD SDAP_SearchEx(struct SDP_SearchReqStru *sreq, struct SDP_SearchCfmStru *scfm);
WORD SDAP_ServiceBrowse(struct SDAP_BrowseInfoStru *info, WORD *size, DWORD *hdl_buf);
WORD SDAP_GetServiceAttribute(WORD trans_hdl, DWORD svc_hdl, WORD id, struct SDAP_DataEleStru *value);
WORD SDAP_ServiceSearch(struct SDAP_SearchInfoStru *info, DWORD svc_hdl, DWORD *size, UCHAR *attr_buf);
WORD SDAP_GetStringAttribute(WORD trans_hdl, DWORD svc_hdl, WORD id, DWORD *size, UCHAR *str);
WORD SDAP_GetServiceInfo(WORD trans_hdl, DWORD *uuid, struct SDAP_GeneralInfoStru *sdx_info);
struct SDAP_GeneralInfoStru *SDAP_GetServiceInfoEx(WORD trans_hdl, DWORD svc_hdl, DWORD *svc_type);
WORD SDAP_GetServiceInfo2(WORD trans_hdl, WORD svc_type, struct SDAP_UUIDStru *uuid, struct SDAP_GeneralInfoStru *sdx_info);
WORD SDAP_GetAllAttributes(WORD trans_hdl, DWORD svc_hdl, struct SDP_SearchCfmStru *sCfm);

struct SDAP_GeneralInfoStru *SDAP_CreateSvcInfoStru(DWORD buf_size, UCHAR *buf, DWORD svc_hdl, DWORD svc_type);

/*
	app_context: application defined context data used to identify the request.
    bd:     Device address of the SDP server.
    status: Status of the connection setup procedure, it can be one of,
            ERROR_SDAP_OPEN_PENDING - The connection request is sent and waiting
            for response. The application can cancel the connection request in
            this state.
            ERROR_SDAP_SUCCESS - The connection with the server is created successfully.
            Other error codes - Fail to create a connection with the server.
    trans_hdl: Transaction handle used to identify the connection.
*/
typedef void (SDAP_OpenSearchRsp_Func)(void *app_context, UCHAR *bd, WORD status, WORD trans_hdl);

/*
	app_context: application defined context data used to identify the request.
    trans_hdl: transaction handle of the service discovery session to be closed.
    status: Status of the disconnection procedure, it can be one of,
            ERROR_SDAP_SUCCESS - Remove one reference or release the connection
                                 successfully.
            ERROR_SDP_INVALID_TRANSHANDLE - trans_hdl is an illegal transation handle.
*/
typedef void (SDAP_CloseSearchRsp_Func)(void *app_context, WORD trans_hdl, WORD status);

/*
	app_context: application defined context data used to identify the request.
    status: Status of the disconnection procedure, it can be one of,
            ERROR_SDAP_SUCCESS - Remove one reference or release the connection
                                 successfully.
            ERROR_SDP_INVALID_TRANSHANDLE - trans_hdl is an illegal transation handle.
            Other error codes - Fail to retrieve the service handles or attribute values.
	scfm	Pointer to the structure used to return the result. 
 	        scfm->var points to the buffer contain the attribute data and 
 	        scfm->size is set to the actual size of the data stored in scfm->var.

			The callback function shall	free the buffer pointed by scfm and scfm->var
			by calling function SDAP_FreeMemory() after handling the searching result.

			If an SS search is required, the result contained in scfm->var is the copy of
			ServiceRecordHandleList field of the response PDU.

			If an SA (or SSA) is required, the result contained in scfm->var is the copy of
			AttributeList(s) field of the response PDU.
*/
typedef void (SDAP_SearchRsp_Func)(void *app_context, WORD status, struct SDP_SearchCfmStru *scfm);

/*
	app_context: application defined context data used to identify the request.
    trans_hdl: Transaction handle used to identify the connection.
    status: Status of the attribute search procedure, it can be one of,
            ERROR_SDAP_SUCCESS - Retrieve attribute values successfully and the
                                 service is available.
            ERROR_SDAP_SERVICE_UNAVAILABLE - Retrieve attribute values successfully
                                 but the specified service is not accepting new clients.
            Other error codes - Fail to retrieve attribute values.
	svc_type: a 16bit UUID specifies the type of the service record retrieved.
	sdx_info: Pointer to a variable of struct SDAP_GeneralInfoStru type that returns
              the attribute values.
*/
typedef void (SDAP_GetServiceInfoRsp_Func)(void *app_context, WORD trans_hdl, WORD status, WORD svc_type, struct SDAP_GeneralInfoStru *sdx_info);

/*
	app_context: application defined context data used to identify the request.
    bd:     Device address of the SDP server.
    status: Status of the attribute search procedure, it can be one of,
            ERROR_SDAP_SUCCESS - Retrieve attribute values successfully and the
                                 service is available.
            ERROR_SDAP_SERVICE_UNAVAILABLE - Retrieve attribute values successfully
                                 but the specified service is not accepting new clients.
            Other error codes - Fail to retrieve attribute values.
	svc_type: a 16bit UUID specifies the type of the service record retrieved.
	sdx_info: Pointer to a variable of struct SDAP_GeneralInfoStru type that returns
              the attribute values.
*/
typedef void (SDAP_GetServiceInfoBDRsp_Func)(void *app_context, UCHAR *bd, WORD status, WORD svc_type, struct SDAP_GeneralInfoStru *sdx_info);

void SDAP_OpenSearchReq(UCHAR *bd, void *app_context, SDAP_OpenSearchRsp_Func *rsp_func);
void SDAP_CloseSearchReq(WORD trans_hdl, void *app_context, SDAP_CloseSearchRsp_Func *rsp_func);
void SDAP_SearchReq(struct SDP_SearchReqStru *sreq, void *app_context, SDAP_SearchRsp_Func *rsp_func);
void SDAP_GetServiceInfoReq(WORD trans_hdl, WORD svc_type, struct SDAP_UUIDStru *uuid, 
                            DWORD svc_hdl, void *app_context, SDAP_GetServiceInfoRsp_Func *rsp_func);
void SDAP_GetServiceInfoBDReq(UCHAR *bd, WORD svc_type, struct SDAP_UUIDStru *uuid, 
                            DWORD svc_hdl, void *app_context, SDAP_GetServiceInfoBDRsp_Func *rsp_func);

#ifdef CONFIG_HDP_CLIENT
void SDAP_DecodeHDPSvcInfoStru(struct SDAP_HDPInfoStru *src, struct SDAP_HDPInfoHlpStru *dest);
#else
#define SDAP_DecodeHDPSvcInfoStru(src, dest)
#endif

#define SDAP_GetCTPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_CORDLESS_TELE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetICPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_INTERCOM, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetSPPSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_SERIAL_PORT, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetSAPSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_SIM_ACCESS, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHEPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HEADSET, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHEPHSSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HEADSET, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHEPAGSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HEADSET_AG, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHFPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HANDSFREE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHFPHFSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HANDSFREE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHFPAGSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HANDSFREE_AG, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetDUNSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_DIALUP_NET, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetFaxSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_FAX, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetLAPSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_LAN_ACCESS, NULL, (struct SDAP_GeneralInfoStru*)info) 
#define SDAP_GetFTPSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_OBEX_FILE_TRANS, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetOPPSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_OBEX_OBJ_PUSH, NULL, (struct SDAP_GeneralInfoStru*)info); 
#define SDAP_GetSyncSvcInfo(hdl, info)  	SDAP_GetServiceInfo2(hdl, CLS_IRMC_SYNC, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetSyncCmdSvcInfo(hdl, info)  	SDAP_GetServiceInfo2(hdl, CLS_IRMC_SYNC_CMD, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetBIPImgSvcInfo(hdl, info)  	SDAP_GetServiceInfo2(hdl, CLS_IMAG_RESPONDER, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetBIPRefObjSvcInfo(hdl, info)	SDAP_GetServiceInfo2(hdl, CLS_IMAG_REF_OBJ, &((info)->svc_id), (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetBIPArchSvcInfo(hdl, info)  	SDAP_GetServiceInfo2(hdl, CLS_IMAG_AUTO_ARCH, &((info)->svc_id), (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetNAPSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_PAN_NAP, NULL, (struct SDAP_GeneralInfoStru*)info) 
#define SDAP_GetGNSvcInfo(hdl, info)   		SDAP_GetServiceInfo2(hdl, CLS_PAN_GN, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetPANUSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_PAN_PANU, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetAVRCPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, (info)->svc_type, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetAVRCTInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AVRCP_CT, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetAVRTGInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AVRCP_TG, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetA2DPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_ADV_AUDIO_DISTRIB, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetA2DPSrcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AUDIO_SOURCE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetA2DPSnkInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AUDIO_SINK, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHCRPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, (info)->conn_type, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHCRPCtrlSvcInfo(hdl, info) 	SDAP_GetServiceInfo2(hdl, PROTOCOL_HCR_CONTROL, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHCRPNotifySvcInfo(hdl, info)   SDAP_GetServiceInfo2(hdl, PROTOCOL_HCR_NOTIFY, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHIDSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HID, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetDISvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_PNP_INFO, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetVDPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_VIDEO_DISTRIBUTION, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetVDPSrcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_VIDEO_SOURCE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetVDPSnkInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_VIDEO_SINK, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetBPPPrinterInfo(hdl, info)	SDAP_GetServiceInfo2(hdl, CLS_DIRECT_PRINT, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetBPPRUIInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_REFLECTED_UI, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetBPPObjInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_DPS_REF_OBJ, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetPBAPPSEInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_PBAP_PSE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetPBAPPCEInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_PBAP_PCE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHDPSrcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HDP_SOURCE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHDPSnkInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HDP_SINK, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetMAPMASInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_MAP_MAS, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetMAPMNSInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_MAP_MNS, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetGATTInfo(hdl, info)		    SDAP_GetServiceInfo2(hdl, CLS_GENERIC_ATT, NULL, (struct SDAP_GeneralInfoStru*)info)
#endif

#endif
