/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    sdpfsm.h
Abstract:
	This module defines all the SDP FSM routines.
Author:
    Wu Peng
    Ye Dongxiang
Revision History:
    1.  2000.01.20  Created
    2.  2000.09.05
    	new FSM routines
    		AbortQuery		dispatch violation indication to session FSM
    		ReceivedData	dispatch data indication to session FSM
    		QuitQuery		dispatch disconnection indication to session FSM
    	the old timer routine AbortQuery renamed as QueryTimeOut
    3.  2002.12.04  Support fixed service record database and dynamic registering
        service record.
    4.  2003.01.08  Use asynchrous structure. Add events and functions to handle
        connection setup procedure.
    5.  2003.02.15	Remove operation on local service database from the FSM table.
    6.  2003.05.20 	Add a new default FSM event handler to the control FSM.
---------------------------------------------------------------------------*/
/* Header file for protocol FSM */
#ifndef _BRTSDP_FSM_H_
#define _BRTSDP_FSM_H_

enum {
	ST_SDP_READY
};

#define SDP_CONTROL_STATE_COUNT (ST_SDP_READY + 1)

enum {
	ST_SDP_CONNECT
};

#define SDP_SESSION_STATE_COUNT (ST_SDP_CONNECT + 1)

enum {
	EV_SDP_SD_OPENSEARCH = 1,
	EV_SDP_SD_CONNREQ,
	EV_SDP_SD_CONNECTCFM,
	EV_SDP_SD_CLOSESEARCH,
	EV_SDP_SD_SERVICEQUERY,
	EV_SDP_SD_VIOLATION,
	EV_SDP_SD_DATA,
	EV_SDP_SD_DISCONNECT,

	EV_SDP_SD_DONE
};

#define SDP_CONTROL_EVENT_COUNT (EV_SDP_SD_DONE)

enum {
	EV_SDP_SERVICESEARCHFOR = 1,
	EV_SDP_SERVICEATTRIBUTEFOR,
	EV_SDP_SERVICESEARCHATTRIBUTEFOR,
	EV_SDP_DATAIND,
	EV_SDP_TIMEOUT,
	EV_SDP_CLOSESEARCH
};

#define SDP_SESSION_EVENT_COUNT (EV_SDP_CLOSESEARCH)
#define SDP_CONTROL_FN_COUNT (sizeof(sdp_control_fn_list)/sizeof(struct FsmNode))
#define SDP_SESSION_FN_COUNT (sizeof(sdp_session_fn_list)/sizeof(struct FsmNode))

void BeginQuery(struct FsmInst *fi, UCHAR event, void *arg);
void ConnectCfm(struct FsmInst *fi, UCHAR event, void *arg);
void ConnReqIn(struct FsmInst *fi, UCHAR event, void *arg);
void EndQuery(struct FsmInst *fi, UCHAR event, void *arg);
void AbortQuery(struct FsmInst *fi, UCHAR event, void *arg);
void QuitQuery(struct FsmInst *fi, UCHAR event, void *arg);
void ServiceQuery(struct FsmInst *fi, UCHAR event, void *arg);
void EndSession(struct FsmInst *fi, UCHAR event, void *arg);
void AbortSession(struct FsmInst *fi, UCHAR event, void *arg);
void ServiceSearchQuery(struct FsmInst *fi, UCHAR event, void *arg);
void ServiceAttributeQuery(struct FsmInst *fi, UCHAR event, void *arg);
void ServiceSearchAttributeQuery(struct FsmInst *fi, UCHAR event, void *arg);
void QueryTimeOut(struct FsmInst *fi, UCHAR event, void *arg);
void ReceiveData_sdp(struct FsmInst *fi, UCHAR event, void *arg);
void DataReady(struct FsmInst *fi, UCHAR event, void *arg);

WORD RegisterService(struct ServiceRegStru *reg);
#ifndef CONFIG_SDP_FIXED_DATABASE
WORD UnregisterService(DWORD handle);
WORD AllocateSvcHdl(DWORD *handle);
#endif

void ControlDown(struct FsmInst *fi, UCHAR event, void *arg);
#endif




