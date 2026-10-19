#ifndef _IAP2_UI_H
#define _IAP2_UI_H

#include "btdatatype.h"
#include "buff.h"

typedef enum _IAP2_EventEnum {
	IAP2_EV_CONNECTED = 1,
	IAP2_EV_DISCONNECTED,
	IAP2_EV_HIDSTARTED,
	IAP2_EV_HIDSTOPED,
	IAP2_EV_EASTARTED,
	IAP2_EV_EASTOPPED,
	IAP2_EV_FCON,
	IAP2_EV_FCOFF
} IAP2_EventEnum;

/* IAP2 Result Code */
typedef enum _IAP2_ResCodeEnum {
	IAP2_SUCCESSFUL = 0,

	IAP2_RES_FAILED,
	IAP2_RES_PARAMETER_INVALID,

	IAP2_RES_LINK_NOT_EXIST,
	IAP2_RES_LINK_TL_FAILED,
	IAP2_RES_LINK_EXISTS,
	IAP2_RES_SYNC_FAILED,
	IAP2_RES_AUTH_FAILED,
	IAP2_RES_ID_REJECTED,
} IAP2_ResCodeEnum;

/* Event IAP2_EV_STARTED parameter structure. */
typedef struct _IAP2_ConnectResStru {
	IAP2_ResCodeEnum res;		/* The IAP2 link established successful or failed. */
} IAP2_ConnectResStru;

/* Event IAP2_EV_MSG_CFM parameter structure. */
typedef struct _IAP2_SendCfmStru {
	IAP2_ResCodeEnum res;			/* Result of sending message, identifies the Receiver received
					 				   and accepted the sent Message successfully or not. */
	UINT8		seq_num;			/* Sequence Number of the sent Message which be acknowledged; */
} IAP2_SendCfmStru;

void EXT_IAP2_EventInd(void *param, UINT8 event);
void EXT_IAP2_GetLocInfo(UINT8 loc_bd[BD_ADDR_LEN], UINT8 loc_name[64]);

/******************************************************************************/
/*                               Timer functions                              */
/******************************************************************************/
/*-----------------------------------------------------------------------------
Description:
	Callback function for timer expires.
Arguments:
	param:		[in] Parameter;
	event:		[in] Event.
-----------------------------------------------------------------------------*/
typedef void (CBK_Timer_Expire)(UINT8 event, void *param);

/*-----------------------------------------------------------------------------
Description:
	Add a timer.
Arguments:
	timeout:	[in] Timeout value in milliseconds;
	cbk:		[in] Callback function for timer expires;
	param:		[in] Parameter;
	event:		[in] Timer event.
Return:
	Timer Handle.
-----------------------------------------------------------------------------*/
UINT32 EXT_Timer_Add(UINT32 timeout, CBK_Timer_Expire *cbk, void *param, UINT8 event);

/*-----------------------------------------------------------------------------
Description:
	Kill a timer.
Arguments:
	timer_hdl:	[in] Timer Handle.
-----------------------------------------------------------------------------*/
void EXT_Timer_Kill(UINT32 timer_hdl);

/*****************************************************************************/
/*                         Transport Layer functions                         */
/*****************************************************************************/
void IAP2_TL_ConnComplete(UINT8 *bd, UINT8 res);
void IAP2_TL_DisconnComplete(UINT8 *bd);
void IAP2_TL_DataReceive(UINT8 *data, UINT16 data_len);
void IAP2_TL_DataCfm(UINT16 offset, UINT8 out_index);

/*-----------------------------------------------------------------------------
Description:
	Start Tranport Layer, register local Device Name.
Remarks:
	If the Transport Layer is Bluttooth SPP, register SPP Serivce with the MFI
	specified Accessory UUID.
-----------------------------------------------------------------------------*/
void EXT_TL_Start();

/*-----------------------------------------------------------------------------
Description:
	Stop Tranport Layer.
-----------------------------------------------------------------------------*/
void EXT_TL_Stop();

/*-----------------------------------------------------------------------------
Description:
	Establish a TL connection.
Arguments:
	bd:		Bluetooth address.
Remarks:
	If the Transport Layer is Bluetooth SPP, connect SPP with the MFI specified
	Device UUID.
-----------------------------------------------------------------------------*/
void EXT_TL_Connect(UINT8 *bd);

/*-----------------------------------------------------------------------------
Description:
	Terminate a TL connection.
Arguments:
	conn_hdl:	[in] connection handle.
-----------------------------------------------------------------------------*/
void EXT_TL_Disconnect();

extern struct BuffStru *EXT_TL_DataNew(UINT16 size);

/*-----------------------------------------------------------------------------
Description:
	Send data on transport layer.
Arguments:
	conn_hdl:	[in] connection handle;
	data:		[in] the Packet will be sent.
	data_len:	[in] Length of the packet will be sent;
-----------------------------------------------------------------------------*/
void EXT_TL_DataReq(struct BuffStru *data, UINT8 out_index);

/*-------------------------------------------------------------------------------- 
Description:
    Data is consumed, and notify remote device to send more data if necessary.
---------------------------------------------------------------------------------*/
void EXT_TL_DataConsumed();

/*****************************************************************************/
/*                          Authentication functions                         */
/*****************************************************************************/
/*-----------------------------------------------------------------------------
Description:
	Indicates Certificate Data total length.
-----------------------------------------------------------------------------*/
void IAP2_Auth_CertDataLen(UINT16 length);

/*-----------------------------------------------------------------------------
Description:
	Indicates the Authentication Certificate data is ready.
Arguments:
	data_len:	[in] the length of data filled in buffer.
-----------------------------------------------------------------------------*/
void IAP2_Auth_CertData(UINT16 data_len);

/*-----------------------------------------------------------------------------
Description:
	Indicate Authentication Response data total length.
-----------------------------------------------------------------------------*/
void IAP2_Auth_ChRspDataLen(UINT16 length);

/*-----------------------------------------------------------------------------
Description:
	Requests authentication challenge response.
Arguments:
	data_len:	[in] the length of data filled in buffer.
-----------------------------------------------------------------------------*/
void IAP2_Auth_ChRspData(UINT16 data_len);

/*-----------------------------------------------------------------------------
Description:
	Callback function to get Accessory Certificate data total length.
-----------------------------------------------------------------------------*/
void EXT_Auth_GetCertDataLen();

/*-----------------------------------------------------------------------------
Description:
	Callback function to get Authentication Certificate data.

Arguments:
	buff:		[in] Specifies the buffer to return Authentication Certificate data;
	buff_len:	[in] Buffer length.
-----------------------------------------------------------------------------*/
void EXT_Auth_GetCertData(UINT8 *buff, UINT16 buff_len);

/*-----------------------------------------------------------------------------
Description:
	Callback function to compute a Response to the Authentication Challenge.
Arguments:
	data:		[in] Authentication Challenge;
	len:		[in] Authentication Challenge length.
-----------------------------------------------------------------------------*/
void EXT_Auth_ChData(UINT8 *data, UINT16 data_len);

/*-----------------------------------------------------------------------------
Description:
	Callback function to get Authentication Challenge Response data.
Arguments:
	buff:		[in] Response data buffer. Fill the Data in BUFDATA(data).
	buff_len:	[in] Authentication Challenge data offset.
-----------------------------------------------------------------------------*/
void EXT_Auth_GetChRspData(UINT8 *buff, UINT16 buff_len);

typedef struct _IAP2_PortingFuncStru {
	void (*EXT_IAP2_EventInd_t)(void *param, UINT8 event);
	void (*EXT_IAP2_GetLocInfo_t)(UINT8 loc_bd[BD_ADDR_LEN], UINT8 loc_name[64]);
	UINT32 (*EXT_Timer_Add_t)(UINT32 timeout, CBK_Timer_Expire *cbk, void *param, UINT8 event);
	void (*EXT_Timer_Kill_t)(UINT32 timer_hdl);
	void (*EXT_TL_Start_t)();
	void (*EXT_TL_Stop_t)();
	void (*EXT_TL_Connect_t)(UINT8 *bd);
	void (*EXT_TL_Disconnect_t)();
	struct BuffStru *(*EXT_TL_DataNew_t)(UINT16 size);
	void (*EXT_TL_DataReq_t)(struct BuffStru *data, UINT8 out_index);
	void (*EXT_TL_DataConsumed_t)();
	void (*EXT_Auth_GetCertDataLen_t)();
	void (*EXT_Auth_GetCertData_t)(UINT8 *buff, UINT16 buff_len);
	void (*EXT_Auth_ChData_t)(UINT8 *data, UINT16 data_len);
	void (*EXT_Auth_GetChRspData_t)(UINT8 *buff, UINT16 buff_len);
} IAP2_PortingFuncStru;

void IAP2_Start(IAP2_PortingFuncStru *p_func);
void IAP2_ConnectReq(UINT8 *bd);
void IAP2_DisconnectReq();
void IAP2_HID_Start(void);
void IAP2_HID_Stop(void);
void IAP2_HidReport(UINT8 *rpt, UINT16 rpt_len);


#endif /* _IAP2_UI_H */
