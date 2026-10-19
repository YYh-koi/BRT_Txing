
#ifndef APP_RFCOMM_SDP_H
#define APP_RFCOMM_SDP_H

#include "btdatatype.h"
#include "sdap_uistru.h"
/****************************************************/

typedef enum {
	/* private result code */
	ER_RFC_SUCCESS,				    /* Success */
	
	ER_RFC_REPEATED_CONNECTION,	    /* A connection to the same service exists */
	ER_RFC_SDP_CONNECTION_FAILURE,	/* Fail to create SDP connection */
	ER_RFC_SERVICE_NOT_AVAILABLE,	/* Can't find the specified service record or the service record is labeled as unavailable */
	ER_RFC_REMOTE_SDP_TRANSACTION_ERROR,	/* Error occur during SDP transaction */

	ER_RFC_INVALID_HANDLE,			/* The specified handle is not a valid  instance handle */
	ER_RFC_NO_CONNECTION,			/* The connection is not ready to execute the operation requested */
	ER_RFC_LOCAL_BUSY,				/* Another operation is incomplete */
	ER_RFC_LOCAL_ABORT,			    /* Operation aborted by local APP */
	ER_RFC_LOCAL_CLOSE,			    /* The  Stack is being closed. */
	ER_RFC_TOO_MORE_SERVICES,       /* Local services of specified type have reached the limit. */

} tRFC_SdpResult;

typedef enum { 
	RFC_SDP_EVT_SERVICE_IND,
	RFC_SDP_EVT_DISCOVER_COMPLETE,
}tRFC_SDP_Event_Type;


typedef void (tRfcSdp_EventCbk)(void *handle, void *context, tRFC_SDP_Event_Type event, void *param);

void hlp_uuid_trans_a_to_b(UINT8 *srv_uuid, struct SDAP_UUIDStru* uuidStru);
void RFC_Discover(UINT8 *bd, UINT8 uuid[16], void *context,  tRfcSdp_EventCbk *cbk);

#endif