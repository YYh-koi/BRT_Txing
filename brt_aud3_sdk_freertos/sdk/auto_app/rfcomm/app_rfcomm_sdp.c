#include "btdatatype.h"
#include "autoconf_bt.h"
#include "app_rfcomm_sdp.h"
#include "sdap_uimacro.h"
#include "sdap_uistru.h"
#include "sdap_uifunc.h"
#include "sysdep.h"
#include "sdpui.h"

#include "btlist.h"
#include "buff.h"
#include "brt_trace.h"

#define MODULE_TAG  "BRTRFCSDP"
#define BRT_DBG_RFCSDP(fmt,...)   bse_log_print(MODULE_TAG,"\r", fmt, ##__VA_ARGS__)

#define RFC_FREE(m)				{if (m != NULL) FREE(m);}
#define MAX_RFC_RECORD_COUNT	(12)

/* Context structure for  */
typedef struct {
	UINT8			bd[BD_ADDR_LEN];/* Bluetooth device address of the  server */
	void				*context;       /* Application defined context data */
	tRfcSdp_EventCbk	*cbk;  			/* Application defined callback function */
	UINT16			trans_hdl;		/* Transaction handle used to identify the SDP connection. */
	UINT8			svc_hdl_count;  /* Total number of service record handles */
	UINT8			svc_hdl_idx;   	/* Service record handle index of current querying record */
	UINT32			*svc_hdls;     	/* Array of service record handles */
	UINT8			service_uuid[16];
} tRFC_DiscoverContextStru;

static void RFC_SearchSAReq(tRFC_DiscoverContextStru *in_context);

/*---------------------------------------------------------------------------
Description:
	This function sends MAP_EVT_DISCOVER_MAS_COMPLETE event with an error to the APP.
---------------------------------------------------------------------------*/
static void RFC_SendDiscoverCompleteEv(tRFC_DiscoverContextStru *in_context, UINT16 status)
{
	if (status == ERROR_SDAP_SUCCESS)
	{
	    status = ER_RFC_SUCCESS;
	} 
	else if ((status > ERROR_SDAP_INSUFFICIENT_RESOURCE) && (status < ERROR_SDAP_INVALID_UUID)) 
	{
		status = ER_RFC_SDP_CONNECTION_FAILURE;
	} 
	else if ((status == ERROR_SDAP_FOUND_NO_SERVICE) || (status == ERROR_SDAP_SERVICE_UNAVAILABLE)) 
	{
		status = ER_RFC_SERVICE_NOT_AVAILABLE;
	} 
	else 
	{
		status = ER_RFC_REMOTE_SDP_TRANSACTION_ERROR;
	}
	
	(*in_context->cbk)(in_context->bd, in_context->context, RFC_SDP_EVT_DISCOVER_COMPLETE, &status);
	RFC_FREE(in_context->svc_hdls);
	FREE(in_context);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
	This function is called to deal with the confirmation to the ServiceSearch 
	procedure initiated by MAC_OpenSearchCfm.
---------------------------------------------------------------------------*/
static void RFC_SearchSACfm(tRFC_DiscoverContextStru *in_context, UINT16 status, 
                        struct SDP_SearchCfmStru *sa_cfm)
{
	UINT8 is_complete = 0;
	UINT8 tmp;
	struct SDAP_SPPInfoStru *mas_in;

    //BRT_DBG_RFCSDP("search sa cfm--status=%d, sa_cfm->count(bytes)", status);
	if (ERROR_SDAP_SUCCESS == status) 
	{
		
		#if 0
			char str[400] = {0};
			int i = 0, index = 0;
			int len = sizeof(str);
			
			BRT_DBG_RFCSDP("Count = %d", sa_cfm->count);
			for(i = 0; i < sa_cfm->count; i++)
			{
				index = strlen(str);
				snprintf(&(str[index]), len - index, "%02x-", sa_cfm->var[i]);
			}
			BRT_DBG_RFCSDP("SA DATA = %s", str);
		#endif
		
		if (0 != sa_cfm->count) 
		{
			/* Decode service attributes */
            mas_in = (struct SDAP_SPPInfoStru *)SDAP_CreateSvcInfoStru(sa_cfm->count, sa_cfm->var, 0, CLS_SERIAL_PORT);
			if (mas_in != NULL) 
			{
				(*in_context->cbk)(in_context->bd, in_context->context, RFC_SDP_EVT_SERVICE_IND, mas_in);
				FREE(mas_in);
			}

		}
		
		in_context->svc_hdl_idx++;
		if (in_context->svc_hdl_idx < in_context->svc_hdl_count) 
		{
			RFC_SearchSAReq(in_context);
		} 
		else 
		{
			is_complete = 1;
		}

	}
	
	if (is_complete) 
	{
		SDAP_CloseSearchReq(in_context->trans_hdl, NULL, NULL);
		/* Report the result to the application */
		RFC_SendDiscoverCompleteEv(in_context, status);
	}
	
	if (sa_cfm->var != NULL) 
	{
	    FREE(sa_cfm->var);
	}
	
	FREE(sa_cfm);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
	This function is called to initiate SA for a MAS service.
---------------------------------------------------------------------------*/
static void RFC_SearchSAReq(tRFC_DiscoverContextStru *in_context)
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
	sa_req->attribute_id_list[0].id = PROT_DESCRIPTION_LIST;
	
	sa_req->max_count = CONFIG_SDAP_MAX_BYTES_COUNT;
	
	SDAP_SearchReq(sa_req, in_context, (SDAP_SearchRsp_Func *)RFC_SearchSACfm);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
	This function is called to deal with the confirmation to the ServiceSearch 
	procedure initiated by MAC_OpenSearchCfm.
---------------------------------------------------------------------------*/
static void RFC_SearchSSCfm(tRFC_DiscoverContextStru *in_context, UINT16 status, 
                        struct SDP_SearchCfmStru *ss_cfm)
{
	UINT8 tmp;
	UINT8 *hdls;

    //BRT_DBG_RFCSDP("search ss cfm--%d, cnt(bytes)=%d", status, ss_cfm ? ss_cfm->count : 0);

    if (ERROR_SDAP_SUCCESS == status && ss_cfm)
	{
		if (0 == ss_cfm->count) 
		{
			status = ERROR_SDAP_FOUND_NO_SERVICE;
		} 
		else if ((ss_cfm->count <= (MAX_RFC_RECORD_COUNT << 2)) && 
			((ss_cfm->count & 3) == 0))
		{
			in_context->svc_hdls = (UINT32 *)ss_cfm->var;
			hdls = ss_cfm->var;
			
			in_context->svc_hdl_count = (ss_cfm->count)/4;
			in_context->svc_hdl_idx = 0;

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
			/* in_context->svc_hdl_count = ss_cfm->count;		*/
#endif

			RFC_SearchSAReq(in_context);
			ss_cfm->var = NULL;
		} 
		else 
		{
			/* Invalid data size */
			status = ERROR_SDAP_INVALID_RESPONSE;
		}
	}
	if (status != ERROR_SDAP_SUCCESS) {
		SDAP_CloseSearchReq(in_context->trans_hdl, NULL, NULL);
		/* Report the result to the application */
		RFC_SendDiscoverCompleteEv(in_context, status);
	}
    if (ss_cfm)
    {
        if (ss_cfm->var != NULL)
        {
            FREE(ss_cfm->var);
        }

        FREE(ss_cfm);
    }
}


static void RFC_OpenSearchCfm(tRFC_DiscoverContextStru *in_context, UINT8 *bd, UINT16 status, UINT16 trans_hdl)
{
	BRT_DBG_RFCSDP("sdp open cfm, status=%d", status);
	
	if (ERROR_SDAP_SUCCESS == status) 
	{
		struct SDP_SearchReqStru *ss_req;

		in_context->trans_hdl = trans_hdl;
		ss_req = (struct SDP_SearchReqStru *)NEW(sizeof(struct SDP_SearchReqStru));
		memset(ss_req, 0, sizeof(struct SDP_SearchReqStru));
		ss_req->trans_handle = trans_hdl;
		/* Set SS request parameters */
		ss_req->mask = TRANS_SS | STOP_MAXCOUNT;
		ss_req->pattern_count = 1;
		ss_req->service_search_pattern = (struct SDP_UUID *)NEW(sizeof(struct SDP_UUID));
		ss_req->service_search_pattern[0].mask = SD_UUID_128;
		memcpy(ss_req->service_search_pattern[0].var, in_context->service_uuid, 16);
		#if 0
		{
			int i = 0;
			for(i = 0; i < 16; i++)
			{
				ss_req->service_search_pattern[0].var[i] = in_context->service_uuid[15 -i];
			}
		}
		#endif

		ss_req->max_count = MAX_RFC_RECORD_COUNT;
		SDAP_SearchReq(ss_req, in_context, (SDAP_SearchRsp_Func *)RFC_SearchSSCfm);

	}
	else 
	{
		RFC_SendDiscoverCompleteEv(in_context, status);
	}

}

void RFC_Discover(UINT8 *bd, UINT8 uuid[16], void *context,  tRfcSdp_EventCbk *cbk)
{
	tRFC_DiscoverContextStru *in_context;

	if (cbk != NULL) {
		in_context = NEW(sizeof(tRFC_DiscoverContextStru));
		memset(in_context, 0, sizeof(tRFC_DiscoverContextStru));
		memcpy(in_context->bd, bd, BD_ADDR_LEN);
		memcpy(in_context->service_uuid, uuid, 16);

		in_context->context = context;
		in_context->cbk = cbk;
		SDAP_OpenSearchReq(bd, in_context, (SDAP_OpenSearchRsp_Func *)RFC_OpenSearchCfm);
	}
}



