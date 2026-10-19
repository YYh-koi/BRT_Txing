#include<stdio.h>

#include "auto_gap.h"
#include "gap_ui.h"
#include "internal_gap.h"
#include "hci_ui.h"
#include "auto_commfunc.h"
#include "internal_manager.h"
#include "bt_power_manager.h"
#include "hci_stru.h"
#include "sdap_uistru.h"
#include "sdpuistru.h"
#include "sdpuimacro.h"
#include "sdap_uimacro.h"
#include "sdap_uifunc.h"

extern UINT32 CheckServiceTypeExt(UINT8 *attr, UINT32 len, struct SDAP_UUIDStru *puuid);

#define MAX_RECORD_COUNT	32
typedef void (SvcSchCbkFunc)(UINT8 *bd, UINT32 number, UINT16 *services);
typedef struct {
	UINT8  			bd[6];
	SvcSchCbkFunc	*cbk;

	UINT16 			trans_hdl;
	UINT8 			svc_hdl_count;
	UINT8 			svc_hdl_idx;
	UINT32 			*svc_hdls;

	UINT32			services_idx;
	UINT16			*services;
} tGetServicesContextStru;

void GetServices_SearchSACfm(tGetServicesContextStru *in_context, UINT16 status, struct SDP_SearchCfmStru *sa_cfm);
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
---------------------------------------------------------------------------*/
void GetServiceClassReq(tGetServicesContextStru *in_context)
{
	struct SDP_SearchReqStru *sa_req;

	/* Initiate ServiceAttribute procedure */
	sa_req = (struct SDP_SearchReqStru*)NEW(sizeof(struct SDP_SearchReqStru));
	memset(sa_req, 0, sizeof(struct SDP_SearchReqStru));
	sa_req->trans_handle = in_context->trans_hdl;
	sa_req->mask = TRANS_SA | STOP_MAXCOUNT;
	sa_req->handle = in_context->svc_hdls[in_context->svc_hdl_idx];
	sa_req->id_count = 1;
	sa_req->attribute_id_list = (struct ServiceAttributeID *)NEW(sizeof(struct ServiceAttributeID));
	sa_req->attribute_id_list[0].mask = SD_ATTRID_SINGLE;
	sa_req->attribute_id_list[0].id = 0x0001;
	sa_req->attribute_id_list[0].end_id = 0;
	sa_req->max_count = 100;
	SDAP_SearchReq(sa_req, in_context, (SDAP_SearchRsp_Func *)GetServices_SearchSACfm);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
---------------------------------------------------------------------------*/
void GetServices_SearchSACfm(tGetServicesContextStru *in_context, UINT16 status, struct SDP_SearchCfmStru *sa_cfm)
{
	UINT8 is_complete = 0;

	if (ERROR_SDAP_SUCCESS == status) {
		if (0 != sa_cfm->count) {
			/* Decode service attributes */
			in_context->services[in_context->services_idx] = (UINT16)CheckServiceTypeExt(sa_cfm->var, sa_cfm->count, NULL);
			if (in_context->services[in_context->services_idx]) {
				in_context->services_idx++;
			}
			in_context->svc_hdl_idx++;
			if (in_context->svc_hdl_idx < in_context->svc_hdl_count) {
				GetServiceClassReq(in_context);
			} else {
				is_complete = 1;
			}
		}
	}
	if (is_complete) {
		SDAP_CloseSearchReq(in_context->trans_hdl, NULL, NULL);
		/* Report the result to the application */
		(*in_context->cbk)(in_context->bd, in_context->services_idx, in_context->services);
		FREE(in_context->svc_hdls);
		FREE(in_context->services);
		FREE(in_context);
	}
	FREE(sa_cfm->var);
	FREE(sa_cfm);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
---------------------------------------------------------------------------*/
void GetServices_SearchSSCfm(tGetServicesContextStru *in_context, UINT16 status, struct SDP_SearchCfmStru *ss_cfm)
{
	UINT8 tmp;
	UINT8 *hdls;

	if (ERROR_SDAP_SUCCESS == status) {
		if (0 == ss_cfm->count) {
			status = ERROR_SDAP_FOUND_NO_SERVICE;
		} else if ((ss_cfm->count <= (MAX_RECORD_COUNT << 2)) && ((ss_cfm->count & 3) == 0)) {
			in_context->svc_hdls = (UINT32 *)ss_cfm->var;
			in_context->svc_hdl_count = (UINT8)(ss_cfm->count >> 2);
			in_context->services = NEW(in_context->svc_hdl_count * sizeof(UINT16));
			hdls = ss_cfm->var;
#ifdef CONFIG_LITTLE_ENDIAN
			/* Decode service record handles */
			while (ss_cfm->count != 0) {
				tmp = hdls[0];
				hdls[0] = hdls[3];
				hdls[3] = tmp;
				tmp = hdls[1];
				hdls[1] = hdls[2];
				hdls[2] = tmp;
				ss_cfm->count -= 4;
				hdls += 4;
			}
#endif
			GetServiceClassReq(in_context);
			ss_cfm->var = NULL;
		} else {
			/* Invalid data size */
			status = ERROR_SDAP_INVALID_RESPONSE;
		}
	}
	if (status!= ERROR_SDAP_SUCCESS) {
		SDAP_CloseSearchReq(in_context->trans_hdl, NULL, NULL);
		/* Report the result to the application */
		(*in_context->cbk)(in_context->bd, 0, NULL);
		FREE(in_context);
	}
	if (ss_cfm->var != NULL) {
		FREE(ss_cfm->var);
	}
	FREE(ss_cfm);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
---------------------------------------------------------------------------*/
void GetServices_OpenSearchCfm(tGetServicesContextStru *in_context, UINT8 *bd, UINT16 status, UINT16 trans_hdl)
{
	if (ERROR_SDAP_SUCCESS == status) {
		struct SDP_SearchReqStru *ss_req;
		UINT32 svc_type = 0x100;

		in_context->trans_hdl = trans_hdl;
		ss_req = NEW(sizeof(struct SDP_SearchReqStru));
		memset(ss_req, 0, sizeof(struct SDP_SearchReqStru));
		ss_req->trans_handle = trans_hdl;
		/* Set SS request parameters */
		ss_req->mask = TRANS_SS | STOP_MAXCOUNT;
		ss_req->pattern_count = 1;
		ss_req->service_search_pattern = (struct SDP_UUID *)NEW(sizeof(struct SDP_UUID));
		ss_req->service_search_pattern[0].mask = SD_UUID_16;
		memcpy(ss_req->service_search_pattern[0].var, (UCHAR*)&svc_type, sizeof(UINT32));
		ss_req->max_count = MAX_RECORD_COUNT;
		SDAP_SearchReq(ss_req, in_context, (SDAP_SearchRsp_Func *)GetServices_SearchSSCfm);
	} else {
		(*in_context->cbk)(bd, 0, NULL);
		FREE(in_context);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
---------------------------------------------------------------------------*/
void GetRemoteServiceListReq(UINT8 *bd, SvcSchCbkFunc *cbk)
{
	tGetServicesContextStru *in_context;

	in_context = NEW(sizeof(tGetServicesContextStru));
	memset(in_context, 0, sizeof(tGetServicesContextStru));
	memcpy(in_context->bd, bd, 6);
	in_context->cbk = cbk;
	SDAP_OpenSearchReq(bd, in_context, (SDAP_OpenSearchRsp_Func *)GetServices_OpenSearchCfm);
}
