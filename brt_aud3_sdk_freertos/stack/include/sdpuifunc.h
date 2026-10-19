/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    sdpuifunc.h
Abstract:
	This files exports SDP API functions.
Author:
    Wu Peng
Revision History:
    1.  2000.01.20  Created
---------------------------------------------------------------------------*/
#ifndef _BRTSDP_UI_FUNC_H_
#define _BRTSDP_UI_FUNC_H_

#include "sdpuimacro.h"

#define  FUNC_EXPORT 

typedef void (SDPUICfm)(struct SDP_UICfmParamStru *cfm);

UCHAR SDPInit(void);
void SDPDone(void);
DWORD DESize(UCHAR *de, UCHAR *addi, UCHAR flag);

FUNC_EXPORT WORD sdRegisterService(struct ServiceRegStru *service);
FUNC_EXPORT WORD sdUnregisterService(DWORD handle);
FUNC_EXPORT WORD sdAllocateSvcHdl(DWORD *handle);

FUNC_EXPORT WORD sdOpenSearchReq(struct SDP_UIReqParamStru *req);
FUNC_EXPORT WORD sdCloseSearchReq(struct SDP_UIReqParamStru *req);
FUNC_EXPORT WORD sdxSearchReq(struct SDP_UIReqParamStru *req);

FUNC_EXPORT void  sdRegisterCB(SDPUICfm *cb);

FUNC_EXPORT UCHAR* sdMallocMemory(int size);
FUNC_EXPORT void  sdFreeMemory(UCHAR *buf);


#endif

